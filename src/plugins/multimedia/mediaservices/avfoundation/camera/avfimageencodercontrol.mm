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

#include "avfcameraviewfindersettingscontrol.h"
#include "avfimageencodercontrol.h"
#include "avfimagecapturecontrol.h"
#include "avfcamerautility.h"
#include "avfcamerasession.h"
#include "avfcameraservice.h"
#include "avfcameradebug.h"
#include "avfcameracontrol.h"
#include <qmediaencodersettings.h>
#include <qsysinfo.h>
#include <qdebug.h>

#include <AVFoundation/AVFoundation.h>

AVFImageEncoderControl::AVFImageEncoderControl(AVFCameraService *service)
    : m_service(service)
{
    Q_ASSERT(service);
}

QStringList AVFImageEncoderControl::supportedImageCodecs() const
{
    return QStringList() << "jpeg";
}

QString AVFImageEncoderControl::imageCodecDescription(const QString &codecName) const
{
    if (codecName == "jpeg")
        return tr("JPEG image");

    return QString();
}

QList<QSize> AVFImageEncoderControl::supportedResolutions(const QImageEncoderSettings &settings,
      bool *continuous) const
{
    (void) settings;

    QList<QSize> resolutions;

    if (!videoCaptureDeviceIsValid())
        return resolutions;



        AVCaptureDevice *captureDevice = m_service->session()->videoCaptureDevice();
        const QVector<AVCaptureDeviceFormat *> formats(qt_unique_device_formats(captureDevice,
                                                       m_service->session()->defaultCodec()));

        for (int i = 0; i < formats.size(); ++i) {
            AVCaptureDeviceFormat *format = formats[i];

            const QSize res(qt_device_format_resolution(format));
            if (!res.isNull() && res.isValid())
                resolutions << res;
        }


    if (continuous)
        *continuous = false;

    return resolutions;
}

QImageEncoderSettings AVFImageEncoderControl::requestedSettings() const
{
    return m_settings;
}

QImageEncoderSettings AVFImageEncoderControl::imageSettings() const
{
    QImageEncoderSettings settings;

    if (!videoCaptureDeviceIsValid())
        return settings;



        AVCaptureDevice *captureDevice = m_service->session()->videoCaptureDevice();
        if (!captureDevice.activeFormat) {
            qDebugCamera() << Q_FUNC_INFO << "no active format";
            return settings;
        }

        QSize res(qt_device_format_resolution(captureDevice.activeFormat));

        if (res.isNull() || !res.isValid()) {
            qDebugCamera() << Q_FUNC_INFO << "failed to exctract the image resolution";
            return settings;
        }

        settings.setResolution(res);

    settings.setCodec(QLatin1String("jpeg"));

    return settings;
}

void AVFImageEncoderControl::setImageSettings(const QImageEncoderSettings &settings)
{
    if (m_settings == settings)
        return;

    m_settings = settings;
    applySettings();
}

bool AVFImageEncoderControl::applySettings()
{
    if (!videoCaptureDeviceIsValid())
        return false;

    AVFCameraSession *session = m_service->session();
    if (!session || (session->state() != QCamera::ActiveState
        && session->state() != QCamera::LoadedState)
            || !m_service->cameraControl()->captureMode().testFlag(QCamera::CaptureStillImage)) {
        return false;
    }

    if (!m_service->imageCaptureControl()
        || !m_service->imageCaptureControl()->stillImageOutput()) {
        qDebugCamera() << Q_FUNC_INFO << "no still image output";
        return false;
    }

    if (m_settings.codec().size()
        && m_settings.codec() != QLatin1String("jpeg")) {
        qDebugCamera() << Q_FUNC_INFO << "unsupported codec:" << m_settings.codec();
        return false;
    }

    QSize res(m_settings.resolution());
    if (res.isNull()) {
        qDebugCamera() << Q_FUNC_INFO << "invalid resolution:" << res;
        return false;
    }

    if (!res.isValid()) {
        // Invalid == default value.
        // Here we could choose the best format available, but
        // activeFormat is already equal to 'preset high' by default,
        // which is good enough, otherwise we can end in some format with low framerates.
        return false;
    }

    bool activeFormatChanged = false;

        AVCaptureDevice *captureDevice = m_service->session()->videoCaptureDevice();
        AVCaptureDeviceFormat *match = qt_find_best_resolution_match(captureDevice, res,
                                                                     m_service->session()->defaultCodec());

        if (!match) {
            qDebugCamera() << Q_FUNC_INFO << "unsupported resolution:" << res;
            return false;
        }

        activeFormatChanged = qt_set_active_format(captureDevice, match, true);

    return activeFormatChanged;
}

bool AVFImageEncoderControl::videoCaptureDeviceIsValid() const
{
    if (!m_service->session() || !m_service->session()->videoCaptureDevice())
        return false;

    AVCaptureDevice *captureDevice = m_service->session()->videoCaptureDevice();
    if (!captureDevice.formats || !captureDevice.formats.count)
        return false;

    return true;
}
