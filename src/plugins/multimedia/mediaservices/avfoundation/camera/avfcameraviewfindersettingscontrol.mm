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

#include <avfcameraviewfindersettingscontrol.h>
#include <avfcamerarenderercontrol.h>
#include <avfcamerautility.h>
#include <avfcamerasession.h>
#include <avfcameraservice.h>
#include <avfcameradebug.h>
#include <qabstractvideosurface.h>
#include <qvariant.h>
#include <qsysinfo.h>
#include <qvector.h>
#include <qdebug.h>
#include <qlist.h>

#include <qmultimediautils_p.h>

#include <algorithm>

#include <AVFoundation/AVFoundation.h>

namespace {

bool qt_framerates_sane(const QCameraViewfinderSettings &settings)
{
    const qreal minFPS = settings.minimumFrameRate();
    const qreal maxFPS = settings.maximumFrameRate();

    if (minFPS < 0. || maxFPS < 0.)
        return false;

    return !maxFPS || maxFPS >= minFPS;
}

} // namespace

AVFCameraViewfinderSettingsControl2::AVFCameraViewfinderSettingsControl2(AVFCameraService *service)
    : m_service(service)
{
    Q_ASSERT(service);
}

QList<QCameraViewfinderSettings> AVFCameraViewfinderSettingsControl2::supportedViewfinderSettings() const
{
    QList<QCameraViewfinderSettings> supportedSettings;

    AVCaptureDevice *captureDevice = m_service->session()->videoCaptureDevice();
    if (!captureDevice) {
        qDebugCamera() << Q_FUNC_INFO << "no capture device found";
        return supportedSettings;
    }

    QVector<AVFPSRange> framerates;

    QVector<QVideoFrame::PixelFormat> pixelFormats(viewfinderPixelFormats());

    if (!pixelFormats.size())
        pixelFormats << QVideoFrame::Format_Invalid; // The default value.

        if (!captureDevice.formats || !captureDevice.formats.count) {
            qDebugCamera() << Q_FUNC_INFO << "no capture device formats found";
            return supportedSettings;
        }

        const QVector<AVCaptureDeviceFormat *> formats(qt_unique_device_formats(captureDevice,
                                                       m_service->session()->defaultCodec()));
        for (int i = 0; i < formats.size(); ++i) {
            AVCaptureDeviceFormat *format = formats[i];

            const QSize res(qt_device_format_resolution(format));
            if (res.isNull() || !res.isValid())
                continue;
            const QSize par(qt_device_format_pixel_aspect_ratio(format));
            if (par.isNull() || !par.isValid())
                continue;

            framerates = qt_device_format_framerates(format);
            if (!framerates.size())
                framerates << AVFPSRange(); // The default value.

            for (int i = 0; i < pixelFormats.size(); ++i) {
                for (int j = 0; j < framerates.size(); ++j) {
                    QCameraViewfinderSettings newSet;
                    newSet.setResolution(res);
                    newSet.setPixelAspectRatio(par);
                    newSet.setPixelFormat(pixelFormats[i]);
                    newSet.setMinimumFrameRate(framerates[j].first);
                    newSet.setMaximumFrameRate(framerates[j].second);
                    supportedSettings << newSet;
                }
            }
        }

    return supportedSettings;
}

QCameraViewfinderSettings AVFCameraViewfinderSettingsControl2::viewfinderSettings() const
{
    QCameraViewfinderSettings settings = m_settings;

    AVCaptureDevice *captureDevice = m_service->session()->videoCaptureDevice();
    if (! captureDevice) {
        qDebugCamera() << Q_FUNC_INFO << "no capture device found";
        return settings;
    }

    if (m_service->session()->state() != QCamera::LoadedState &&
        m_service->session()->state() != QCamera::ActiveState) {
        return settings;
    }

     if (!captureDevice.activeFormat) {
         qDebugCamera() << Q_FUNC_INFO << "no active capture device format";
         return settings;
     }

     const QSize res(qt_device_format_resolution(captureDevice.activeFormat));
     const QSize par(qt_device_format_pixel_aspect_ratio(captureDevice.activeFormat));
     if (res.isNull() || !res.isValid() || par.isNull() || !par.isValid()) {
         qDebugCamera() << Q_FUNC_INFO << "failed to obtain resolution/pixel aspect ratio";
         return settings;
     }

     settings.setResolution(res);
     settings.setPixelAspectRatio(par);

    // TODO: resolution and PAR before 7.0.
    const AVFPSRange fps = qt_current_framerates(captureDevice, videoConnection());
    settings.setMinimumFrameRate(fps.first);
    settings.setMaximumFrameRate(fps.second);

    AVCaptureVideoDataOutput *videoOutput = m_service->videoOutput() ? m_service->videoOutput()->videoDataOutput() : nullptr;
    if (videoOutput) {
        NSObject *obj = [videoOutput.videoSettings objectForKey:(id)kCVPixelBufferPixelFormatTypeKey];
        if (obj && [obj isKindOfClass:[NSNumber class]]) {
            NSNumber *nsNum = static_cast<NSNumber *>(obj);
            settings.setPixelFormat(QtPixelFormatFromCVFormat([nsNum unsignedIntValue]));
        }
    }

    return settings;
}

void AVFCameraViewfinderSettingsControl2::setViewfinderSettings(const QCameraViewfinderSettings &settings)
{
    if (m_settings == settings)
        return;

    m_settings = settings;
    applySettings(m_settings);
}

QVideoFrame::PixelFormat AVFCameraViewfinderSettingsControl2::QtPixelFormatFromCVFormat(unsigned avPixelFormat)
{
    // BGRA <-> ARGB "swap" is intentional:
    // to work correctly with GL_RGBA, color swap shaders
    // (in QSG node renderer etc.).
    switch (avPixelFormat) {
    case kCVPixelFormatType_32ARGB:
        return QVideoFrame::Format_BGRA32;
    case kCVPixelFormatType_32BGRA:
        return QVideoFrame::Format_ARGB32;
    case kCVPixelFormatType_24RGB:
        return QVideoFrame::Format_RGB24;
    case kCVPixelFormatType_24BGR:
        return QVideoFrame::Format_BGR24;
    case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
    case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
        return QVideoFrame::Format_NV12;
    default:
        return QVideoFrame::Format_Invalid;
    }
}

bool AVFCameraViewfinderSettingsControl2::CVPixelFormatFromQtFormat(QVideoFrame::PixelFormat qtFormat, unsigned &conv)
{
    // BGRA <-> ARGB "swap" is intentional:
    // to work correctly with GL_RGBA, color swap shaders
    // (in QSG node renderer etc.).
    switch (qtFormat) {
    case QVideoFrame::Format_ARGB32:
        conv = kCVPixelFormatType_32BGRA;
        break;
    case QVideoFrame::Format_BGRA32:
        conv = kCVPixelFormatType_32ARGB;
        break;
    case QVideoFrame::Format_NV12:
        conv = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
        break;

    // These two formats below are not supported
    // by QSGVideoNodeFactory_RGB, so for now I have to
    // disable them.
    /*
    case QVideoFrame::Format_RGB24:
        conv = kCVPixelFormatType_24RGB;
        break;
    case QVideoFrame::Format_BGR24:
        conv = kCVPixelFormatType_24BGR;
        break;
    */

    default:
        return false;
    }

    return true;
}

AVCaptureDeviceFormat *AVFCameraViewfinderSettingsControl2::findBestFormatMatch(const QCameraViewfinderSettings &settings) const
{
    AVCaptureDevice *captureDevice = m_service->session()->videoCaptureDevice();
    if (! captureDevice || settings.isNull())
        return nil;

        const QSize &resolution = settings.resolution();
        if (!resolution.isNull() && resolution.isValid()) {
            // Either the exact match (including high resolution for images on iOS)
            // or a format with a resolution close to the requested one.
            return qt_find_best_resolution_match(captureDevice, resolution,
                                                 m_service->session()->defaultCodec());
        }

        // No resolution requested, what about framerates?
        if (!qt_framerates_sane(settings)) {
            qDebugCamera() << Q_FUNC_INFO << "invalid framerate requested (min/max):"
                           << settings.minimumFrameRate() << settings.maximumFrameRate();
            return nil;
        }

        const qreal minFPS(settings.minimumFrameRate());
        const qreal maxFPS(settings.maximumFrameRate());
        if (minFPS || maxFPS)
            return qt_find_best_framerate_match(captureDevice,
                                                m_service->session()->defaultCodec(),
                                                maxFPS ? maxFPS : minFPS);
        // Ignore PAR for the moment (PAR without resolution can
        // pick a format with really bad resolution).
        // No need to test pixel format, just return settings.


    return nil;
}

QVector<QVideoFrame::PixelFormat> AVFCameraViewfinderSettingsControl2::viewfinderPixelFormats() const
{
    QVector<QVideoFrame::PixelFormat> qtFormats;

    AVCaptureVideoDataOutput *videoOutput = m_service->videoOutput() ? m_service->videoOutput()->videoDataOutput() : nullptr;
    if (!videoOutput) {
        qDebugCamera() << Q_FUNC_INFO << "no video output found";
        return qtFormats;
    }

    NSArray *pixelFormats = [videoOutput availableVideoCVPixelFormatTypes];

    for (NSObject *obj in pixelFormats) {
        if (![obj isKindOfClass:[NSNumber class]])
            continue;

        NSNumber *formatAsNSNumber = static_cast<NSNumber *>(obj);
        // It's actually FourCharCode (== UInt32):
        const QVideoFrame::PixelFormat qtFormat(QtPixelFormatFromCVFormat([formatAsNSNumber unsignedIntValue]));
        if (qtFormat != QVideoFrame::Format_Invalid
                && !qtFormats.contains(qtFormat)) { // Can happen, for example, with 8BiPlanar existing in video/full range.
            qtFormats << qtFormat;
        }
    }

    return qtFormats;
}

bool AVFCameraViewfinderSettingsControl2::convertPixelFormatIfSupported(QVideoFrame::PixelFormat qtFormat, unsigned &avfFormat)const
{
    AVCaptureVideoDataOutput *videoOutput = m_service->videoOutput() ? m_service->videoOutput()->videoDataOutput() : nullptr;
    if (!videoOutput)
        return false;

    unsigned conv = 0;
    if (!CVPixelFormatFromQtFormat(qtFormat, conv))
        return false;

    NSArray *formats = [videoOutput availableVideoCVPixelFormatTypes];
    if (!formats || !formats.count)
        return false;

    if (m_service->videoOutput()->surface()) {
        const QAbstractVideoSurface *surface = m_service->videoOutput()->surface();
        if (!surface->supportedPixelFormats().contains(qtFormat))
            return false;
    }

    bool found = false;
    for (NSObject *obj in formats) {
        if (![obj isKindOfClass:[NSNumber class]])
            continue;

        NSNumber *nsNum = static_cast<NSNumber *>(obj);
        if ([nsNum unsignedIntValue] == conv) {
            avfFormat = conv;
            found = true;
        }
    }

    return found;
}

bool AVFCameraViewfinderSettingsControl2::applySettings(const QCameraViewfinderSettings &settings)
{
    if (m_service->session()->state() != QCamera::LoadedState &&
        m_service->session()->state() != QCamera::ActiveState) {
        return false;
    }

    AVCaptureDevice *captureDevice = m_service->session()->videoCaptureDevice();
    if (!captureDevice)
        return false;

    bool activeFormatChanged = false;

    AVCaptureDeviceFormat *match = findBestFormatMatch(settings);

    if (match) {
        activeFormatChanged = qt_set_active_format(captureDevice, match, false);
    } else {
        qDebugCamera() << Q_FUNC_INFO << "matching device format not found";
        // We still can update the pixel format at least.
    }

    AVCaptureVideoDataOutput *videoOutput = m_service->videoOutput() ? m_service->videoOutput()->videoDataOutput() : nullptr;
    if (videoOutput) {
        unsigned avfPixelFormat = 0;
        if (!convertPixelFormatIfSupported(settings.pixelFormat(), avfPixelFormat)) {
            // If the the pixel format is not specified or invalid, pick the preferred video surface
            // format, or if no surface is set, the preferred capture device format

            const QVector<QVideoFrame::PixelFormat> deviceFormats = viewfinderPixelFormats();
            QVideoFrame::PixelFormat pickedFormat = deviceFormats.first();

            QAbstractVideoSurface *surface = m_service->videoOutput()->surface();
            if (surface) {
                if (m_service->videoOutput()->supportsTextures()) {
                    pickedFormat = QVideoFrame::Format_ARGB32;
                } else {
                    QList<QVideoFrame::PixelFormat> surfaceFormats = surface->supportedPixelFormats();

                    for (int i = 0; i < surfaceFormats.count(); ++i) {
                        const QVideoFrame::PixelFormat surfaceFormat = surfaceFormats.at(i);
                        if (deviceFormats.contains(surfaceFormat)) {
                            pickedFormat = surfaceFormat;
                            break;
                        }
                    }
                }
            }

            CVPixelFormatFromQtFormat(pickedFormat, avfPixelFormat);
        }

        if (avfPixelFormat != 0) {
            NSMutableDictionary *videoSettings = [NSMutableDictionary dictionaryWithCapacity:1];
            [videoSettings setObject:[NSNumber numberWithUnsignedInt:avfPixelFormat]
                                      forKey:(id)kCVPixelBufferPixelFormatTypeKey];

            videoOutput.videoSettings = videoSettings;
        }
    }

    qt_set_framerate_limits(captureDevice, videoConnection(), settings.minimumFrameRate(), settings.maximumFrameRate());

    return activeFormatChanged;
}

QCameraViewfinderSettings AVFCameraViewfinderSettingsControl2::requestedSettings() const
{
    return m_settings;
}

AVCaptureConnection *AVFCameraViewfinderSettingsControl2::videoConnection() const
{
    if (!m_service->videoOutput() || !m_service->videoOutput()->videoDataOutput())
        return nil;

    return [m_service->videoOutput()->videoDataOutput() connectionWithMediaType:AVMediaTypeVideo];
}

AVFCameraViewfinderSettingsControl::AVFCameraViewfinderSettingsControl(AVFCameraService *service)
    : m_service(service)
{
    // Legacy viewfinder settings control.
    Q_ASSERT(service);
    initSettingsControl();
}

bool AVFCameraViewfinderSettingsControl::isViewfinderParameterSupported(ViewfinderParameter parameter) const
{
    return parameter == Resolution
           || parameter == PixelAspectRatio
           || parameter == MinimumFrameRate
           || parameter == MaximumFrameRate
           || parameter == PixelFormat;
}

QVariant AVFCameraViewfinderSettingsControl::viewfinderParameter(ViewfinderParameter parameter) const
{
    if (!isViewfinderParameterSupported(parameter)) {
        qDebugCamera() << Q_FUNC_INFO << "parameter is not supported";
        return QVariant();
    }

    if (!initSettingsControl()) {
        qDebugCamera() << Q_FUNC_INFO << "initialization failed";
        return QVariant();
    }

    const QCameraViewfinderSettings settings(m_settingsControl->viewfinderSettings());
    if (parameter == Resolution)
        return settings.resolution();
    if (parameter == PixelAspectRatio)
        return settings.pixelAspectRatio();
    if (parameter == MinimumFrameRate)
        return settings.minimumFrameRate();
    if (parameter == MaximumFrameRate)
        return settings.maximumFrameRate();
    if (parameter == PixelFormat)
        return QVariant::fromValue(settings.pixelFormat());

    return QVariant();
}

void AVFCameraViewfinderSettingsControl::setViewfinderParameter(ViewfinderParameter parameter, const QVariant &value)
{
    if (! isViewfinderParameterSupported(parameter)) {
        qDebugCamera() << Q_FUNC_INFO << "parameter is not supported";
        return;
    }

    if (parameter == Resolution)
        setResolution(value);
    if (parameter == PixelAspectRatio)
        setAspectRatio(value);
    if (parameter == MinimumFrameRate)
        setFrameRate(value, false);
    if (parameter == MaximumFrameRate)
        setFrameRate(value, true);
    if (parameter == PixelFormat)
        setPixelFormat(value);
}

void AVFCameraViewfinderSettingsControl::setResolution(const QVariant &newValue)
{
    if (!newValue.canConvert<QSize>()) {
        qDebugCamera() << Q_FUNC_INFO << "QSize type expected";
        return;
    }

    if (!initSettingsControl()) {
        qDebugCamera() << Q_FUNC_INFO << "initialization failed";
        return;
    }

    const QSize res(newValue.toSize());
    if (res.isNull() || !res.isValid()) {
        qDebugCamera() << Q_FUNC_INFO << "invalid resolution:" << res;
        return;
    }

    QCameraViewfinderSettings settings(m_settingsControl->viewfinderSettings());
    settings.setResolution(res);
    m_settingsControl->setViewfinderSettings(settings);
}

void AVFCameraViewfinderSettingsControl::setAspectRatio(const QVariant &newValue)
{
    if (!newValue.canConvert<QSize>()) {
        qDebugCamera() << Q_FUNC_INFO << "QSize type expected";
        return;
    }

    if (!initSettingsControl()) {
        qDebugCamera() << Q_FUNC_INFO << "initialization failed";
        return;
    }

    const QSize par(newValue.value<QSize>());
    if (par.isNull() || !par.isValid()) {
        qDebugCamera() << Q_FUNC_INFO << "invalid pixel aspect ratio:" << par;
        return;
    }

    QCameraViewfinderSettings settings(m_settingsControl->viewfinderSettings());
    settings.setPixelAspectRatio(par);
    m_settingsControl->setViewfinderSettings(settings);
}

void AVFCameraViewfinderSettingsControl::setFrameRate(const QVariant &newValue, bool max)
{
    if (!newValue.canConvert<qreal>()) {
        qDebugCamera() << Q_FUNC_INFO << "qreal type expected";
        return;
    }

    if (!initSettingsControl()) {
        qDebugCamera() << Q_FUNC_INFO << "initialization failed";
        return;
    }

    const qreal fps(newValue.toReal());
    QCameraViewfinderSettings settings(m_settingsControl->viewfinderSettings());
    max ? settings.setMaximumFrameRate(fps) : settings.setMinimumFrameRate(fps);
    m_settingsControl->setViewfinderSettings(settings);
}

void AVFCameraViewfinderSettingsControl::setPixelFormat(const QVariant &newValue)
{
    if (!newValue.canConvert<QVideoFrame::PixelFormat>()) {
        qDebugCamera() << Q_FUNC_INFO
                       << "QVideoFrame::PixelFormat type expected";
        return;
    }

    if (! initSettingsControl()) {
        qDebugCamera() << Q_FUNC_INFO << "initialization failed";
        return;
    }

    QCameraViewfinderSettings settings(m_settingsControl->viewfinderSettings());
    settings.setPixelFormat(newValue.value<QVideoFrame::PixelFormat>());
    m_settingsControl->setViewfinderSettings(settings);
}

bool AVFCameraViewfinderSettingsControl::initSettingsControl()const
{
    if (! m_settingsControl)
        m_settingsControl = m_service->viewfinderSettingsControl2();

    return !m_settingsControl.isNull();
}
