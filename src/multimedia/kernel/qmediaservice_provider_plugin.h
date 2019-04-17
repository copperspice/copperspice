/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QMEDIASERVICE_PROVIDER_PLUGIN_H
#define QMEDIASERVICE_PROVIDER_PLUGIN_H

#include <QtCore/qstringlist.h>
#include <QtCore/qplugin.h>
#include <qmultimedia.h>

// emerald #include <QtMultimedia/qcamera.h>

class QMediaService;
class QMediaServiceProviderHintPrivate;

class Q_MULTIMEDIA_EXPORT QMediaServiceProviderHint
{
public:
    enum Type { Null, ContentType, Device, SupportedFeatures, CameraPosition };

    enum Feature {
        LowLatencyPlayback = 0x01,
        RecordingSupport = 0x02,
        StreamPlayback = 0x04,
        VideoSurface = 0x08
    };
    Q_DECLARE_FLAGS(Features, Feature)

    QMediaServiceProviderHint();
    QMediaServiceProviderHint(const QString &mimeType, const QStringList& codecs);
    QMediaServiceProviderHint(const QByteArray &device);
// emerald     QMediaServiceProviderHint(QCamera::Position position);
    QMediaServiceProviderHint(Features features);
    QMediaServiceProviderHint(const QMediaServiceProviderHint &other);
    ~QMediaServiceProviderHint();

    QMediaServiceProviderHint& operator=(const QMediaServiceProviderHint &other);

    bool operator == (const QMediaServiceProviderHint &other) const;
    bool operator != (const QMediaServiceProviderHint &other) const;

    bool isNull() const;

    Type type() const;

    QString mimeType() const;
    QStringList codecs() const;

    QByteArray device() const;
// emerald     QCamera::Position cameraPosition() const;

    Features features() const;

private:
    QSharedDataPointer<QMediaServiceProviderHintPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMediaServiceProviderHint::Features)

struct Q_MULTIMEDIA_EXPORT QMediaServiceProviderFactoryInterface
{
    virtual QMediaService* create(QString const& key) = 0;
    virtual void release(QMediaService *service) = 0;
    virtual ~QMediaServiceProviderFactoryInterface();
};

#define QMediaServiceProviderFactoryInterface_iid \
    "com.copperspice.cs.mediaserviceproviderfactory/1.0"

CS_DECLARE_INTERFACE(QMediaServiceProviderFactoryInterface, QMediaServiceProviderFactoryInterface_iid)

struct Q_MULTIMEDIA_EXPORT QMediaServiceSupportedFormatsInterface
{
    virtual ~QMediaServiceSupportedFormatsInterface() {}
    virtual QMultimedia::SupportEstimate hasSupport(const QString &mimeType, const QStringList& codecs) const = 0;
    virtual QStringList supportedMimeTypes() const = 0;
};

#define QMediaServiceSupportedFormatsInterface_iid \
    "com.copperspice.cs.mediaservicesupportedformats/1.0"

CS_DECLARE_INTERFACE(QMediaServiceSupportedFormatsInterface, QMediaServiceSupportedFormatsInterface_iid)

struct Q_MULTIMEDIA_EXPORT QMediaServiceSupportedDevicesInterface
{
    virtual ~QMediaServiceSupportedDevicesInterface() {}
    virtual QList<QByteArray> devices(const QByteArray &service) const = 0;
    virtual QString deviceDescription(const QByteArray &service, const QByteArray &device) = 0;
};

#define QMediaServiceSupportedDevicesInterface_iid \
    "com.copperspice.cs.mediaservicesupporteddevices/1.0"

CS_DECLARE_INTERFACE(QMediaServiceSupportedDevicesInterface, QMediaServiceSupportedDevicesInterface_iid)

// emerald - review
// This should be part of QMediaServiceSupportedDevicesInterface but it can't in order
// to preserve binary compatibility with 5.2 and earlier.

// The whole media service plugin API shouldn't even be public in the first place. It should
// be made private in Qt 6.0 and QMediaServiceDefaultDeviceInterface should be merged with
// QMediaServiceSupportedDevicesInterface

struct Q_MULTIMEDIA_EXPORT QMediaServiceDefaultDeviceInterface
{
    virtual ~QMediaServiceDefaultDeviceInterface() {}
    virtual QByteArray defaultDevice(const QByteArray &service) const = 0;
};

#define QMediaServiceDefaultDeviceInterface_iid \
    "com.copperspice.cs.mediaservicedefaultdevice/1.0"

CS_DECLARE_INTERFACE(QMediaServiceDefaultDeviceInterface, QMediaServiceDefaultDeviceInterface_iid)


/* emerald

struct Q_MULTIMEDIA_EXPORT QMediaServiceCameraInfoInterface
{
    virtual ~QMediaServiceCameraInfoInterface() {}
    virtual QCamera::Position cameraPosition(const QByteArray &device) const = 0;
    virtual int cameraOrientation(const QByteArray &device) const = 0;
};

#define QMediaServiceCameraInfoInterface_iid \
    "com.copperspice.cs.mediaservicecamerainfo/1.0"

CS_DECLARE_INTERFACE(QMediaServiceCameraInfoInterface, QMediaServiceCameraInfoInterface_iid)

*/


struct Q_MULTIMEDIA_EXPORT QMediaServiceFeaturesInterface
{
    virtual ~QMediaServiceFeaturesInterface() {}
    virtual QMediaServiceProviderHint::Features supportedFeatures(const QByteArray &service) const = 0;
};

#define QMediaServiceFeaturesInterface_iid \
    "com.copperspice.cs.mediaservicefeatures/1.0"

CS_DECLARE_INTERFACE(QMediaServiceFeaturesInterface, QMediaServiceFeaturesInterface_iid)

class Q_MULTIMEDIA_EXPORT QMediaServiceProviderPlugin : public QObject, public QMediaServiceProviderFactoryInterface
{
    MULTI_CS_OBJECT_MULTIPLE(QMediaServiceProviderPlugin, QObject)
    CS_INTERFACES(QMediaServiceProviderFactoryInterface)

public:
    virtual QMediaService* create(const QString& key) override = 0 ;
    virtual void release(QMediaService *service) override = 0;
};

/*!
    Service with support for media playback
    Required Controls: QMediaPlayerControl
    Optional Controls: QMediaPlaylistControl, QAudioDeviceControl
    Video Output Controls (used by QWideoWidget and QGraphicsVideoItem):
                        Required: QVideoOutputControl
                        Optional: QVideoWindowControl, QVideoRendererControl, QVideoWidgetControl
*/
#define Q_MEDIASERVICE_MEDIAPLAYER "com.copperspice.cs.mediaplayer"

/*!
   Service with support for recording from audio sources
   Required Controls: QAudioDeviceControl
   Recording Controls (QMediaRecorder):
                        Required: QMediaRecorderControl
                        Recommended: QAudioEncoderSettingsControl
                        Optional: QMediaContainerControl
*/
#define Q_MEDIASERVICE_AUDIOSOURCE "com.copperspice.cs.audiosource"

/*!
    Service with support for camera use.
    Required Controls: QCameraControl
    Optional Controls: QCameraExposureControl, QCameraFocusControl, QCameraImageProcessingControl
    Still Capture Controls: QCameraImageCaptureControl
    Video Capture Controls (QMediaRecorder):
                        Required: QMediaRecorderControl
                        Recommended: QAudioEncoderSettingsControl, QVideoEncoderSettingsControl, QMediaContainerControl
    Viewfinder Video Output Controls (used by QCameraViewfinder and QGraphicsVideoItem):
                        Required: QVideoOutputControl
                        Optional: QVideoWindowControl, QVideoRendererControl, QVideoWidgetControl
*/
#define Q_MEDIASERVICE_CAMERA "com.copperspice.cs.camera"

/*!
    Service with support for radio tuning.
    Required Controls: QRadioTunerControl
    Recording Controls (Optional, used by QMediaRecorder):
                        Required: QMediaRecorderControl
                        Recommended: QAudioEncoderSettingsControl
                        Optional: QMediaContainerControl
*/
#define Q_MEDIASERVICE_RADIO "com.copperspice.cs.radio"

/*!
    Service with support for decoding audio.
    Required Controls: QAudioDecoderControl
    Optional: that streams control
*/
#define Q_MEDIASERVICE_AUDIODECODER "com.copperspice.cs.audiodecode"


#endif
