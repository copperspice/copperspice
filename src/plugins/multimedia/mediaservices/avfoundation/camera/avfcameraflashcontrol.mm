/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include "avfcameraflashcontrol.h"
#include "avfcamerautility.h"
#include "avfcamerasession.h"
#include "avfcameraservice.h"
#include "avfcameradebug.h"

#include <qdebug.h>

#include <AVFoundation/AVFoundation.h>

AVFCameraFlashControl::AVFCameraFlashControl(AVFCameraService *service)
    : m_service(service), m_session(nullptr), m_supportedModes(QCameraExposure::FlashOff),
      m_flashMode(QCameraExposure::FlashOff)
{
    Q_ASSERT(service);
    m_session = m_service->session();
    Q_ASSERT(m_session);

    connect(m_session, SIGNAL(stateChanged(QCamera::State)), SLOT(cameraStateChanged(QCamera::State)));
}

QCameraExposure::FlashModes AVFCameraFlashControl::flashMode() const
{
    return m_flashMode;
}

void AVFCameraFlashControl::setFlashMode(QCameraExposure::FlashModes mode)
{
    if (m_flashMode == mode)
        return;

    if (m_session->state() == QCamera::ActiveState && !isFlashModeSupported(mode)) {
        qDebugCamera() << Q_FUNC_INFO << "unsupported mode" << mode;
        return;
    }

    m_flashMode = mode;

    if (m_session->state() != QCamera::ActiveState)
        return;

    applyFlashSettings();
}

bool AVFCameraFlashControl::isFlashModeSupported(QCameraExposure::FlashModes mode) const
{
    // From what QCameraExposure has, we can support only these:
    //  FlashAuto = 0x1,
    //  FlashOff = 0x2,
    //  FlashOn = 0x4,
    // AVCaptureDevice has these flash modes:
    //  AVCaptureFlashModeAuto
    //  AVCaptureFlashModeOff
    //  AVCaptureFlashModeOn
    // QCameraExposure also has:
    //  FlashTorch = 0x20,       --> "Constant light source."
    //  FlashVideoLight = 0x40.  --> "Constant light source."
    // AVCaptureDevice:
    //  AVCaptureTorchModeOff (no mapping)
    //  AVCaptureTorchModeOn     --> FlashVideoLight
    //  AVCaptureTorchModeAuto (no mapping)

    return m_supportedModes & mode;
}

bool AVFCameraFlashControl::isFlashReady() const
{
    if (m_session->state() != QCamera::ActiveState)
        return false;

    AVCaptureDevice *captureDevice = m_session->videoCaptureDevice();
    if (!captureDevice)
        return false;

    if (!captureDevice.hasFlash && !captureDevice.hasTorch)
        return false;

    if (!isFlashModeSupported(m_flashMode))
        return false;

#ifdef Q_OS_IOS
    // AVCaptureDevice's docs:
    // "The flash may become unavailable if, for example,
    //  the device overheats and needs to cool off."
    if (m_flashMode != QCameraExposure::FlashVideoLight)
        return [captureDevice isFlashAvailable];

    return [captureDevice isTorchAvailable];
#endif

    return true;
}

void AVFCameraFlashControl::cameraStateChanged(QCamera::State newState)
{
    if (newState == QCamera::UnloadedState) {
        m_supportedModes = QCameraExposure::FlashOff;
        Q_EMIT flashReady(false);
    } else if (newState == QCamera::ActiveState) {
        m_supportedModes = QCameraExposure::FlashOff;
        AVCaptureDevice *captureDevice = m_session->videoCaptureDevice();
        if (!captureDevice) {
            qDebugCamera() << Q_FUNC_INFO << "no capture device in 'Active' state";
            Q_EMIT flashReady(false);
            return;
        }

        if (captureDevice.hasFlash) {
            if ([captureDevice isFlashModeSupported:AVCaptureFlashModeOn])
                m_supportedModes |= QCameraExposure::FlashOn;
            if ([captureDevice isFlashModeSupported:AVCaptureFlashModeAuto])
                m_supportedModes |= QCameraExposure::FlashAuto;
        }

        if (captureDevice.hasTorch && [captureDevice isTorchModeSupported:AVCaptureTorchModeOn])
            m_supportedModes |= QCameraExposure::FlashVideoLight;

        Q_EMIT flashReady(applyFlashSettings());
    }
}

bool AVFCameraFlashControl::applyFlashSettings()
{
    Q_ASSERT(m_session->requestedState() == QCamera::ActiveState);

    AVCaptureDevice *captureDevice = m_session->videoCaptureDevice();
    if (!captureDevice) {
        qDebugCamera() << Q_FUNC_INFO << "no capture device found";
        return false;
    }

    if (!isFlashModeSupported(m_flashMode)) {
        qDebugCamera() << Q_FUNC_INFO << "unsupported mode" << m_flashMode;
        return false;
    }

    if (!captureDevice.hasFlash && !captureDevice.hasTorch) {
        // FlashOff is the only mode we support.
        // Return false - flash is not ready.
        return false;
    }

    const AVFConfigurationLock lock(captureDevice);

    if (m_flashMode != QCameraExposure::FlashVideoLight) {
        if (captureDevice.torchMode != AVCaptureTorchModeOff) {
#ifdef Q_OS_IOS
            if (![captureDevice isTorchAvailable]) {
                qDebugCamera() << Q_FUNC_INFO << "torch is not available at the moment";
                return false;
            }
#endif
            captureDevice.torchMode = AVCaptureTorchModeOff;
        }
#ifdef Q_OS_IOS
        if (![captureDevice isFlashAvailable]) {
            // We'd like to switch flash (into some mode), but it's not available:
            qDebugCamera() << Q_FUNC_INFO << "flash is not available at the moment";
            return false;
        }
#endif
    } else {
        if (captureDevice.flashMode != AVCaptureFlashModeOff) {
#ifdef Q_OS_IOS
            if (![captureDevice isFlashAvailable]) {
                qDebugCamera() << Q_FUNC_INFO << "flash is not available at the moment";
                return false;
            }
#endif
            captureDevice.flashMode = AVCaptureFlashModeOff;
        }

#ifdef Q_OS_IOS
        if (![captureDevice isTorchAvailable]) {
            qDebugCamera() << Q_FUNC_INFO << "torch is not available at the moment";
            return false;
        }
#endif
    }

    if (m_flashMode == QCameraExposure::FlashOff)
        captureDevice.flashMode = AVCaptureFlashModeOff;
    else if (m_flashMode == QCameraExposure::FlashOn)
        captureDevice.flashMode = AVCaptureFlashModeOn;
    else if (m_flashMode == QCameraExposure::FlashAuto)
        captureDevice.flashMode = AVCaptureFlashModeAuto;
    else if (m_flashMode == QCameraExposure::FlashVideoLight)
        captureDevice.torchMode = AVCaptureTorchModeOn;

    return true;
}
