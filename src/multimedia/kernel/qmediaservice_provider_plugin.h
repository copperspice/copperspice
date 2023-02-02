/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#ifndef QMEDIASERVICE_PROVIDER_PLUGIN_H
#define QMEDIASERVICE_PROVIDER_PLUGIN_H

#include <qstringlist.h>
#include <qplugin.h>
#include <qmultimedia.h>
#include <qcamera.h>

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
   using Features = QFlags<Feature>;

   QMediaServiceProviderHint();
   QMediaServiceProviderHint(const QString &mimeType, const QStringList &codecs);
   QMediaServiceProviderHint(const QString &device);
   QMediaServiceProviderHint(QCamera::Position position);
   QMediaServiceProviderHint(Features features);
   QMediaServiceProviderHint(const QMediaServiceProviderHint &other);

   ~QMediaServiceProviderHint();

   QMediaServiceProviderHint &operator=(const QMediaServiceProviderHint &other);

   bool operator == (const QMediaServiceProviderHint &other) const;
   bool operator != (const QMediaServiceProviderHint &other) const;

   bool isNull() const;

   Type type() const;

   QString mimeType() const;
   QStringList codecs() const;

   QString device() const;
   QCamera::Position cameraPosition() const;

   Features features() const;

 private:
   QSharedDataPointer<QMediaServiceProviderHintPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMediaServiceProviderHint::Features)

struct Q_MULTIMEDIA_EXPORT QMediaServiceProviderFactoryInterface {
   virtual ~QMediaServiceProviderFactoryInterface() {}

   virtual QMediaService *create(QString const &key) = 0;
   virtual void release(QMediaService *service) = 0;
};

#define QMediaServiceProviderInterface_ID  "com.copperspice.CS.mediaServiceProvider/1.0"
CS_DECLARE_INTERFACE(QMediaServiceProviderFactoryInterface, QMediaServiceProviderInterface_ID)

struct Q_MULTIMEDIA_EXPORT QMediaServiceSupportedFormatsInterface {
   virtual ~QMediaServiceSupportedFormatsInterface() {}

   virtual QMultimedia::SupportEstimate hasSupport(const QString &mimeType, const QStringList &codecs) const = 0;
   virtual QStringList supportedMimeTypes() const = 0;
};

#define QMediaServiceSupportedFormatsInterface_ID "com.copperspice.CS.mediaServiceSupportedFormats/1.0"
CS_DECLARE_INTERFACE(QMediaServiceSupportedFormatsInterface, QMediaServiceSupportedFormatsInterface_ID)

struct Q_MULTIMEDIA_EXPORT QMediaServiceSupportedDevicesInterface {
   virtual ~QMediaServiceSupportedDevicesInterface() {}

   virtual QList<QString> devices(const QString &service) const = 0;
   virtual QString deviceDescription(const QString &service, const QString &device) = 0;
};

#define QMediaServiceSupportedDevicesInterface_ID "com.copperspice.CS.mediaServiceSupportedDevices/1.0"
CS_DECLARE_INTERFACE(QMediaServiceSupportedDevicesInterface, QMediaServiceSupportedDevicesInterface_ID)

struct Q_MULTIMEDIA_EXPORT QMediaServiceDefaultDeviceInterface {
   virtual ~QMediaServiceDefaultDeviceInterface() {}
   virtual QString defaultDevice(const QString &service) const = 0;
};

#define QMediaServiceDefaultDeviceInterface_ID  "com.copperspice.CS.mediaServiceDefaultDevice/1.0"
CS_DECLARE_INTERFACE(QMediaServiceDefaultDeviceInterface, QMediaServiceDefaultDeviceInterface_ID)


struct Q_MULTIMEDIA_EXPORT QMediaServiceCameraInfoInterface
{
   virtual ~QMediaServiceCameraInfoInterface() {}
   virtual QCamera::Position cameraPosition(const QString &device) const = 0;
   virtual int cameraOrientation(const QString &device) const = 0;
};

#define QMediaServiceCameraInfoInterface_ID "com.copperspice.CS.mediaServiceCameraInfo/1.0"
CS_DECLARE_INTERFACE(QMediaServiceCameraInfoInterface, QMediaServiceCameraInfoInterface_ID)


struct Q_MULTIMEDIA_EXPORT QMediaServiceFeaturesInterface {
   virtual ~QMediaServiceFeaturesInterface() {}
   virtual QMediaServiceProviderHint::Features supportedFeatures(const QString &service) const = 0;
};

#define QMediaServiceFeaturesInterface_ID "com.copperspice.CS.mediaServiceFeatures/1.0"
CS_DECLARE_INTERFACE(QMediaServiceFeaturesInterface, QMediaServiceFeaturesInterface_ID)

class Q_MULTIMEDIA_EXPORT QMediaServiceProviderPlugin : public QObject, public QMediaServiceProviderFactoryInterface
{
   MULTI_CS_OBJECT_MULTIPLE(QMediaServiceProviderPlugin, QObject)
   CS_INTERFACES(QMediaServiceProviderFactoryInterface)

 public:
   virtual ~QMediaServiceProviderPlugin();

   virtual QMediaService *create(const QString &key) override = 0 ;
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
#define Q_MEDIASERVICE_MEDIAPLAYER "com.copperspice.CS.mediaPlayer"

/*!
   Service with support for recording from audio sources
   Required Controls: QAudioDeviceControl
   Recording Controls (QMediaRecorder):
                        Required: QMediaRecorderControl
                        Recommended: QAudioEncoderSettingsControl
                        Optional: QMediaContainerControl
*/
#define Q_MEDIASERVICE_AUDIOSOURCE "com.copperspice.CS.audioSource"

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
#define Q_MEDIASERVICE_CAMERA "com.copperspice.CS.camera"

/*!
    Service with support for radio tuning.
    Required Controls: QRadioTunerControl
    Recording Controls (Optional, used by QMediaRecorder):
                        Required: QMediaRecorderControl
                        Recommended: QAudioEncoderSettingsControl
                        Optional: QMediaContainerControl
*/
#define Q_MEDIASERVICE_RADIO "com.copperspice.CS.radio"

/*!
    Service with support for decoding audio.
    Required Controls: QAudioDecoderControl
    Optional: that streams control
*/
#define Q_MEDIASERVICE_AUDIODECODER "com.copperspice.CS.audioDecoder"


#endif
