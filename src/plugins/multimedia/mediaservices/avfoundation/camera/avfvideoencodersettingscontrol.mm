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

#include "avfvideoencodersettingscontrol.h"

#include "avfcameraservice.h"
#include "avfcamerautility.h"
#include "avfcamerasession.h"
#include "avfcamerarenderercontrol.h"

#include <AVFoundation/AVFoundation.h>

static QStringList *supportedCodecs() {
   static QStringList retval = {"avc1", "jpeg", "ap4h", "apcn"};
   return &retval;
}

static bool format_supports_framerate(AVCaptureDeviceFormat *format, qreal fps)
{
    if (format && fps > qreal(0)) {
        const qreal epsilon = 0.1;

        for (AVFrameRateRange *range in format.videoSupportedFrameRateRanges) {
            if (range.maxFrameRate - range.minFrameRate < epsilon) {
                if (qAbs(fps - range.maxFrameRate) < epsilon)
                    return true;
            }

            if (fps >= range.minFrameRate && fps <= range.maxFrameRate)
                return true;
        }
    }

    return false;
}

static bool real_list_contains(const QList<qreal> &list, qreal value)
{
    for (qreal r : list) {
        if (qFuzzyCompare(r, value))
            return true;
    }

    return false;
}

AVFVideoEncoderSettingsControl::AVFVideoEncoderSettingsControl(AVFCameraService *service)
    : QVideoEncoderSettingsControl(), m_service(service), m_restoreFormat(nil)
{
}

QList<QSize> AVFVideoEncoderSettingsControl::supportedResolutions(const QVideoEncoderSettings &settings,
                                                                  bool *continuous) const
{
    (void) settings;

    if (continuous)
        *continuous = true;

    // AVFoundation seems to support any resolution for recording, with the following limitations:
    // - The recording resolution can't be higher than the camera's active resolution
    // - On OS X, the recording resolution is automatically adjusted to have the same aspect ratio as
    //   the camera's active resolution
    QList<QSize> resolutions;
    resolutions.append(QSize(32, 32));


        AVCaptureDevice *device = m_service->session()->videoCaptureDevice();
        if (device) {
            int maximumWidth = 0;
            const QVector<AVCaptureDeviceFormat *> formats(qt_unique_device_formats(device,
                                                                                    m_service->session()->defaultCodec()));
            for (int i = 0; i < formats.size(); ++i) {
                const QSize res(qt_device_format_resolution(formats[i]));
                if (res.width() > maximumWidth)
                    maximumWidth = res.width();
            }

            if (maximumWidth > 0)
                resolutions.append(QSize(maximumWidth, maximumWidth));
        }



    if (resolutions.count() == 1)
        resolutions.append(QSize(3840, 3840));

    return resolutions;
}

QList<qreal> AVFVideoEncoderSettingsControl::supportedFrameRates(const QVideoEncoderSettings &settings, bool *continuous) const
{
    QList<qreal> uniqueFrameRates;


        AVCaptureDevice *device = m_service->session()->videoCaptureDevice();
        if (!device)
            return uniqueFrameRates;

        if (continuous)
            *continuous = false;

        QVector<AVFPSRange> allRates;

        if (!settings.resolution().isValid()) {
            const QVector<AVCaptureDeviceFormat *> formats(qt_unique_device_formats(device, 0));
            for (int i = 0; i < formats.size(); ++i) {
                AVCaptureDeviceFormat *format = formats.at(i);
                allRates += qt_device_format_framerates(format);
            }
        } else {
            AVCaptureDeviceFormat *format = qt_find_best_resolution_match(device,
                                                                          settings.resolution(),
                                                                          m_service->session()->defaultCodec());
            if (format)
                allRates = qt_device_format_framerates(format);
        }

        for (int j = 0; j < allRates.size(); ++j) {
            if (!real_list_contains(uniqueFrameRates, allRates[j].first))
                uniqueFrameRates.append(allRates[j].first);

            if (!real_list_contains(uniqueFrameRates, allRates[j].second))
                uniqueFrameRates.append(allRates[j].second);
        }


    return uniqueFrameRates;
}

QStringList AVFVideoEncoderSettingsControl::supportedVideoCodecs() const
{
    return *supportedCodecs();
}

QString AVFVideoEncoderSettingsControl::videoCodecDescription(const QString &codecName) const
{
    if (codecName == "avc1")
        return QString("H.264");

    else if (codecName == "jpeg")
        return QString("M-JPEG");

    else if (codecName == "ap4h")
        return QString("Apple ProRes 4444");

    else if (codecName == "apcn")
        return QString("Apple ProRes 422 Standard Definition");

    return QString();
}

QVideoEncoderSettings AVFVideoEncoderSettingsControl::videoSettings() const
{
    return m_actualSettings;
}

void AVFVideoEncoderSettingsControl::setVideoSettings(const QVideoEncoderSettings &settings)
{
    if (m_requestedSettings == settings)
        return;

    m_requestedSettings = m_actualSettings = settings;
}

NSDictionary *AVFVideoEncoderSettingsControl::applySettings(AVCaptureConnection *connection)
{
    if (m_service->session()->state() != QCamera::LoadedState &&
        m_service->session()->state() != QCamera::ActiveState) {
        return nil;
    }

    AVCaptureDevice *device = m_service->session()->videoCaptureDevice();
    if (!device)
        return nil;

    AVFPSRange currentFps = qt_current_framerates(device, connection);
    const bool needFpsChange = m_requestedSettings.frameRate() > 0
                               && m_requestedSettings.frameRate() != currentFps.second;

    NSMutableDictionary *videoSettings = [NSMutableDictionary dictionary];

    // -- Codec

    // AVVideoCodecKey is the only mandatory key
    QString codec = m_requestedSettings.codec().isEmpty() ? supportedCodecs()->first() : m_requestedSettings.codec();

    if (! supportedCodecs()->contains(codec)) {
        qWarning("Unsupported codec: '%s'", csPrintable(codec));
        codec = supportedCodecs()->first();
    }
    [videoSettings setObject:codec.toNSString() forKey:AVVideoCodecKey];
    m_actualSettings.setCodec(codec);

    // -- Resolution

    int w = m_requestedSettings.resolution().width();
    int h = m_requestedSettings.resolution().height();

    if (AVCaptureDeviceFormat *currentFormat = device.activeFormat) {
        CMFormatDescriptionRef formatDesc = currentFormat.formatDescription;
        CMVideoDimensions dim = CMVideoFormatDescriptionGetDimensions(formatDesc);

        // We have to change the device's activeFormat in 3 cases:
        // - the requested recording resolution is higher than the current device resolution
        // - the requested recording resolution has a different aspect ratio than the current device aspect ratio
        // - the requested frame rate is not available for the current device format
        AVCaptureDeviceFormat *newFormat = nil;
        if ((w <= 0 || h <= 0)
                && m_requestedSettings.frameRate() > 0
                && !format_supports_framerate(currentFormat, m_requestedSettings.frameRate())) {

            newFormat = qt_find_best_framerate_match(device,
                                                     m_service->session()->defaultCodec(),
                                                     m_requestedSettings.frameRate());

        } else if (w > 0 && h > 0) {
            AVCaptureDeviceFormat *f = qt_find_best_resolution_match(device,
                                                                     m_requestedSettings.resolution(),
                                                                     m_service->session()->defaultCodec());

            if (f) {
                CMVideoDimensions d = CMVideoFormatDescriptionGetDimensions(f.formatDescription);
                qreal fAspectRatio = qreal(d.width) / d.height;

                if (w > dim.width || h > dim.height
                        || qAbs((qreal(dim.width) / dim.height) - fAspectRatio) > 0.01) {
                    newFormat = f;
                }
            }
        }

        if (qt_set_active_format(device, newFormat, !needFpsChange)) {
            m_restoreFormat = [currentFormat retain];
            formatDesc = newFormat.formatDescription;
            dim = CMVideoFormatDescriptionGetDimensions(formatDesc);
        }

        if (w > 0 && h > 0) {
            // Make sure the recording resolution has the same aspect ratio as the device's
            // current resolution
            qreal deviceAspectRatio = qreal(dim.width) / dim.height;
            qreal recAspectRatio = qreal(w) / h;
            if (qAbs(deviceAspectRatio - recAspectRatio) > 0.01) {
                if (recAspectRatio > deviceAspectRatio)
                    w = qRound(h * deviceAspectRatio);
                else
                    h = qRound(w / deviceAspectRatio);
            }

            // recording resolution can't be higher than the device's active resolution
            w = qMin(w, dim.width);
            h = qMin(h, dim.height);
        }
    }

    if (w > 0 && h > 0) {
        // Width and height must be divisible by 2
        w += w & 1;
        h += h & 1;

        [videoSettings setObject:[NSNumber numberWithInt:w] forKey:AVVideoWidthKey];
        [videoSettings setObject:[NSNumber numberWithInt:h] forKey:AVVideoHeightKey];
        m_actualSettings.setResolution(w, h);
    } else {
        m_actualSettings.setResolution(qt_device_format_resolution(device.activeFormat));
    }

    // -- FPS

    if (needFpsChange) {
        m_restoreFps = currentFps;
        const qreal fps = m_requestedSettings.frameRate();
        qt_set_framerate_limits(device, connection, fps, fps);
    }
    m_actualSettings.setFrameRate(qt_current_framerates(device, connection).second);

    // -- Codec Settings

    NSMutableDictionary *codecProperties = [NSMutableDictionary dictionary];
    int bitrate = -1;
    float quality = -1.f;

    if (m_requestedSettings.encodingMode() == QMultimedia::ConstantQualityEncoding) {
        if (m_requestedSettings.quality() != QMultimedia::NormalQuality) {
            if (codec != QLatin1String("jpeg")) {
                qWarning("ConstantQualityEncoding is not supported for codec: '%s'", csPrintable(codec));
            } else {
                switch (m_requestedSettings.quality()) {
                case QMultimedia::VeryLowQuality:
                    quality = 0.f;
                    break;
                case QMultimedia::LowQuality:
                    quality = 0.25f;
                    break;
                case QMultimedia::HighQuality:
                    quality = 0.75f;
                    break;
                case QMultimedia::VeryHighQuality:
                    quality = 1.f;
                    break;
                default:
                    quality = -1.f; // NormalQuality, let the system decide
                    break;
                }
            }
        }
    } else if (m_requestedSettings.encodingMode() == QMultimedia::AverageBitRateEncoding){
        if (codec != QLatin1String("avc1"))
            qWarning("AverageBitRateEncoding is not supported for codec: '%s'", csPrintable(codec));
        else
            bitrate = m_requestedSettings.bitRate();
    } else {
        qWarning("Encoding mode is not supported");
    }

    if (bitrate != -1)
        [codecProperties setObject:[NSNumber numberWithInt:bitrate] forKey:AVVideoAverageBitRateKey];
    if (quality != -1.f)
        [codecProperties setObject:[NSNumber numberWithFloat:quality] forKey:AVVideoQualityKey];

    [videoSettings setObject:codecProperties forKey:AVVideoCompressionPropertiesKey];

    return videoSettings;
}

void AVFVideoEncoderSettingsControl::unapplySettings(AVCaptureConnection *connection)
{
    m_actualSettings = m_requestedSettings;

    AVCaptureDevice *device = m_service->session()->videoCaptureDevice();
    if (!device)
        return;

    const bool needFpsChanged = m_restoreFps.first || m_restoreFps.second;

    if (m_restoreFormat) {
        qt_set_active_format(device, m_restoreFormat, !needFpsChanged);
        [m_restoreFormat release];
        m_restoreFormat = nil;
    }


    if (needFpsChanged) {
        qt_set_framerate_limits(device, connection, m_restoreFps.first, m_restoreFps.second);
        m_restoreFps = AVFPSRange();
    }
}
