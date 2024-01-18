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

#include "avfcamerazoomcontrol.h"
#include "avfcameraservice.h"
#include "avfcamerautility.h"
#include "avfcamerasession.h"
#include "avfcameracontrol.h"
#include "avfcameradebug.h"
#include <qsysinfo.h>
#include <qglobal.h>
#include <qdebug.h>

AVFCameraZoomControl::AVFCameraZoomControl(AVFCameraService *service)
    : m_session(service->session()), m_maxZoomFactor(1.), m_zoomFactor(1.), m_requestedZoomFactor(1.)
{
    Q_ASSERT(m_session);
    connect(m_session, SIGNAL(stateChanged(QCamera::State)), this, SLOT(cameraStateChanged()));
}

qreal AVFCameraZoomControl::maximumOpticalZoom() const
{
    // not supported
    return 1.;
}

qreal AVFCameraZoomControl::maximumDigitalZoom() const
{
    return m_maxZoomFactor;
}

qreal AVFCameraZoomControl::requestedOpticalZoom() const
{
    // Not supported.
    return 1;
}

qreal AVFCameraZoomControl::requestedDigitalZoom() const
{
    return m_requestedZoomFactor;
}

qreal AVFCameraZoomControl::currentOpticalZoom() const
{
    // Not supported.
    return 1.;
}

qreal AVFCameraZoomControl::currentDigitalZoom() const
{
    return m_zoomFactor;
}

void AVFCameraZoomControl::zoomTo(qreal optical, qreal digital)
{
    (void) optical;
    (void) digital;

#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_7_0)
    if (QSysInfo::MacintoshVersion < QSysInfo::MV_IOS_7_0)
        return;

    if (qFuzzyCompare(CGFloat(digital), m_requestedZoomFactor))
        return;

    m_requestedZoomFactor = digital;
    emit requestedDigitalZoomChanged(digital);

    zoomToRequestedDigital();
#endif
}

void AVFCameraZoomControl::cameraStateChanged()
{
#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_7_0)
    if (QSysInfo::MacintoshVersion < QSysInfo::MV_IOS_7_0)
        return;

    const QCamera::State state = m_session->state();
    if (state != QCamera::ActiveState) {
        if (state == QCamera::UnloadedState && m_maxZoomFactor > 1.) {
            m_maxZoomFactor = 1.;
            emit maximumDigitalZoomChanged(1.);
        }
        return;
    }

    AVCaptureDevice *captureDevice = m_session->videoCaptureDevice();
    if (! captureDevice || !captureDevice.activeFormat) {
        qDebugCamera() << Q_FUNC_INFO << "camera state is active, but"
                       << "video capture device and/or active format is nil";
        return;
    }

    if (captureDevice.activeFormat.videoMaxZoomFactor > 1.) {
        if (!qFuzzyCompare(m_maxZoomFactor, captureDevice.activeFormat.videoMaxZoomFactor)) {
            m_maxZoomFactor = captureDevice.activeFormat.videoMaxZoomFactor;
            Q_EMIT maximumDigitalZoomChanged(m_maxZoomFactor);
        }
    } else if (!qFuzzyCompare(m_maxZoomFactor, CGFloat(1.))) {
        m_maxZoomFactor = 1.;

        emit maximumDigitalZoomChanged(1.);
    }

    zoomToRequestedDigital();
#endif
}

void AVFCameraZoomControl::zoomToRequestedDigital()
{
#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_7_0)
    if (QSysInfo::MacintoshVersion < QSysInfo::MV_IOS_7_0)
        return;

    AVCaptureDevice *captureDevice = m_session->videoCaptureDevice();
    if (! captureDevice || !captureDevice.activeFormat)
        return;

    if (qFuzzyCompare(captureDevice.activeFormat.videoMaxZoomFactor, CGFloat(1.)))
        return;

    const CGFloat clampedZoom = qBound(CGFloat(1.), m_requestedZoomFactor,
                                       captureDevice.activeFormat.videoMaxZoomFactor);
    const CGFloat deviceZoom = captureDevice.videoZoomFactor;
    if (qFuzzyCompare(clampedZoom, deviceZoom)) {
        // Nothing to set, but check if a signal must be emitted:
        if (! qFuzzyCompare(m_zoomFactor, deviceZoom)) {
            m_zoomFactor = deviceZoom;
            emit currentDigitalZoomChanged(deviceZoom);
        }

        return;
    }

    const AVFConfigurationLock lock(captureDevice);
    if (! lock) {
        qDebugCamera() << Q_FUNC_INFO << "failed to lock for configuration";
        return;
    }

    captureDevice.videoZoomFactor = clampedZoom;

    if (!qFuzzyCompare(clampedZoom, m_zoomFactor)) {
        m_zoomFactor = clampedZoom;
        emit currentDigitalZoomChanged(clampedZoom);
    }
#endif

}
