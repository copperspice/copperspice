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

#include <qdebug.h>
#include <qmap.h>
#include <qmediaservice.h>
#include <qmediaplayer.h>
#include <qmediaplayercontrol.h>

#include <qfactoryloader_p.h>
#include <qmediaserviceprovider_p.h>
#include <qmediaservice_provider_plugin.h>

QMediaServiceProviderPlugin::~QMediaServiceProviderPlugin()
{
}

class QMediaServiceProviderHintPrivate : public QSharedData
{
 public:
   QMediaServiceProviderHintPrivate(QMediaServiceProviderHint::Type type)
      : type(type), cameraPosition(QCamera::UnspecifiedPosition), features(Qt::EmptyFlag) {
   }

   QMediaServiceProviderHintPrivate(const QMediaServiceProviderHintPrivate &other)
      : QSharedData(other), type(other.type), device(other.device),
        cameraPosition(other.cameraPosition), mimeType(other.mimeType), codecs(other.codecs),
        features(other.features) {
   }

   ~QMediaServiceProviderHintPrivate() {
   }

   QMediaServiceProviderHint::Type type;
   QString device;
   QCamera::Position cameraPosition;
   QString mimeType;
   QStringList codecs;
   QMediaServiceProviderHint::Features features;
};

QMediaServiceProviderHint::QMediaServiceProviderHint()
   : d(new QMediaServiceProviderHintPrivate(Null))
{
}

QMediaServiceProviderHint::QMediaServiceProviderHint(const QString &type, const QStringList &codecs)
   : d(new QMediaServiceProviderHintPrivate(ContentType))
{
   d->mimeType = type;
   d->codecs = codecs;
}

QMediaServiceProviderHint::QMediaServiceProviderHint(const QString &device)
   : d(new QMediaServiceProviderHintPrivate(Device))
{
   d->device = device;
}

QMediaServiceProviderHint::QMediaServiceProviderHint(QCamera::Position position)
    :d(new QMediaServiceProviderHintPrivate(CameraPosition))
{
    d->cameraPosition = position;
}

QMediaServiceProviderHint::QMediaServiceProviderHint(QMediaServiceProviderHint::Features features)
   : d(new QMediaServiceProviderHintPrivate(SupportedFeatures))
{
   d->features = features;
}

QMediaServiceProviderHint::QMediaServiceProviderHint(const QMediaServiceProviderHint &other)
   : d(other.d)
{
}

QMediaServiceProviderHint::~QMediaServiceProviderHint()
{
}

QMediaServiceProviderHint &QMediaServiceProviderHint::operator=(const QMediaServiceProviderHint &other)
{
   d = other.d;
   return *this;
}

bool QMediaServiceProviderHint::operator == (const QMediaServiceProviderHint &other) const
{
   return (d == other.d) ||
      (d->type == other.d->type &&
         d->device == other.d->device &&
         d->cameraPosition == other.d->cameraPosition &&
         d->mimeType == other.d->mimeType &&
         d->codecs == other.d->codecs &&
         d->features == other.d->features);
}

bool QMediaServiceProviderHint::operator != (const QMediaServiceProviderHint &other) const
{
   return !(*this == other);
}

bool QMediaServiceProviderHint::isNull() const
{
   return d->type == Null;
}

QMediaServiceProviderHint::Type QMediaServiceProviderHint::type() const
{
   return d->type;
}

QString QMediaServiceProviderHint::mimeType() const
{
   return d->mimeType;
}

QStringList QMediaServiceProviderHint::codecs() const
{
   return d->codecs;
}

QString QMediaServiceProviderHint::device() const
{
   return d->device;
}

QCamera::Position QMediaServiceProviderHint::cameraPosition() const
{
   return d->cameraPosition;
}

QMediaServiceProviderHint::Features QMediaServiceProviderHint::features() const
{
   return d->features;
}

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QMediaServiceProviderInterface_ID, "/mediaservices", Qt::CaseInsensitive);
   return &retval;
}

class QPluginServiceProvider : public QMediaServiceProvider
{
   struct MediaServiceData {
      QString type;
      QMediaServiceProviderPlugin *plugin;

      MediaServiceData()
         : plugin(nullptr)
      {
      }
   };

   QMap<const QMediaService *, MediaServiceData> mediaServiceData;

 public:
   QMediaService *requestService(const QString &key, const QMediaServiceProviderHint &hint) override {

      QList<QMediaServiceProviderPlugin *> plugins;
      QFactoryLoader *factoryObj = loader();

      // what keys are available
      const QSet<QString> keySet = factoryObj->keySet();

      for (QString tmpKey : keySet)  {
         // may want to optimize, only load the plugin when the keys match

         if (tmpKey == key) {

            for (QLibraryHandle *handle : factoryObj->librarySet(tmpKey) )  {
               QObject *obj = factoryObj->instance(handle);

               QMediaServiceProviderPlugin *plugin = dynamic_cast<QMediaServiceProviderPlugin *>(obj);

               if (plugin) {
                  plugins.append(plugin);
               }
            }
         }

      }

      if (! plugins.isEmpty()) {
         QMediaServiceProviderPlugin *plugin = nullptr;

         switch (hint.type()) {

            case QMediaServiceProviderHint::Null:
               plugin = plugins[0];

               // special case for media player, if low latency was not asked,
               // prefer services not offering it, since they are likely to support more formats

               if (key == Q_MEDIASERVICE_MEDIAPLAYER) {
                  for (QMediaServiceProviderPlugin *currentPlugin : plugins) {

                     QMediaServiceFeaturesInterface *iface = dynamic_cast<QMediaServiceFeaturesInterface *>(currentPlugin);

                     if (! iface || ! (iface->supportedFeatures(key) & QMediaServiceProviderHint::LowLatencyPlayback)) {
                        plugin = currentPlugin;
                        break;
                     }
                  }
               }
               break;

            case QMediaServiceProviderHint::SupportedFeatures:
               plugin = plugins[0];

               for (QMediaServiceProviderPlugin *currentPlugin : plugins) {
                  QMediaServiceFeaturesInterface *iface = dynamic_cast<QMediaServiceFeaturesInterface *>(currentPlugin);

                  if (iface) {
                     if ((iface->supportedFeatures(key) & hint.features()) == hint.features()) {
                        plugin = currentPlugin;
                        break;
                     }
                  }
               }
               break;

            case QMediaServiceProviderHint::Device: {
               plugin = plugins[0];

               for (QMediaServiceProviderPlugin *currentPlugin : plugins) {
                  QMediaServiceSupportedDevicesInterface *iface =
                     dynamic_cast<QMediaServiceSupportedDevicesInterface *>(currentPlugin);

                  if (iface && iface->devices(key).contains(hint.device())) {
                     plugin = currentPlugin;
                     break;
                  }
               }
            }
            break;

            case QMediaServiceProviderHint::CameraPosition: {
               plugin = plugins[0];

               if (key == Q_MEDIASERVICE_CAMERA && hint.cameraPosition() != QCamera::UnspecifiedPosition) {
                  for (QMediaServiceProviderPlugin *currentPlugin : plugins) {
                     const QMediaServiceSupportedDevicesInterface *deviceIface =
                        dynamic_cast<QMediaServiceSupportedDevicesInterface*>(currentPlugin);

                     const QMediaServiceCameraInfoInterface *cameraIface =
                        dynamic_cast<QMediaServiceCameraInfoInterface*>(currentPlugin);

                     if (deviceIface && cameraIface) {
                        const QList<QString> cameras = deviceIface->devices(key);

                        for (const QString &camera : cameras) {
                           if (cameraIface->cameraPosition(camera) == hint.cameraPosition()) {
                              plugin = currentPlugin;
                              break;
                           }
                        }
                     }
                  }
               }
            }
            break;


            case QMediaServiceProviderHint::ContentType: {
               QMultimedia::SupportEstimate estimate = QMultimedia::NotSupported;

               for (QMediaServiceProviderPlugin *currentPlugin : plugins) {
                  QMultimedia::SupportEstimate currentEstimate = QMultimedia::MaybeSupported;

                  QMediaServiceSupportedFormatsInterface *iface =
                     dynamic_cast<QMediaServiceSupportedFormatsInterface *>(currentPlugin);

                  if (iface) {
                     currentEstimate = iface->hasSupport(hint.mimeType(), hint.codecs());
                  }

                  if (currentEstimate > estimate) {
                     estimate = currentEstimate;
                     plugin   = currentPlugin;

                     if (currentEstimate == QMultimedia::PreferredService) {
                        break;
                     }
                  }
               }
            }
            break;
         }

         if (plugin != nullptr) {
            QMediaService *service = plugin->create(key);

            if (service != nullptr) {
               MediaServiceData d;
               d.type   = key;
               d.plugin = plugin;

               mediaServiceData.insert(service, d);
            }

            return service;
         }
      }

      qWarning() << "QPluginServiceProvider::requestService(): No plugin was found for QMediaService with the key of: " << key;
      return nullptr;
   }

   void releaseService(QMediaService *service) override {
      if (service != nullptr) {
         MediaServiceData d = mediaServiceData.take(service);

         if (d.plugin != nullptr) {
            d.plugin->release(service);
         }
      }
   }

   QMediaServiceProviderHint::Features supportedFeatures(const QMediaService *service) const override {
      if (service) {
         MediaServiceData d = mediaServiceData.value(service);

         if (d.plugin) {
            QMediaServiceFeaturesInterface *iface = dynamic_cast<QMediaServiceFeaturesInterface *>(d.plugin);

            if (iface) {
               return iface->supportedFeatures(d.type);
            }
         }
      }

      return QMediaServiceProviderHint::Features();
   }

   QMultimedia::SupportEstimate hasSupport(const QString &serviceType, const QString &mimeType,
         const QStringList &codecs, int flags) const  override {

      QList<QObject *> plugins;
      QFactoryLoader *factoryObj = loader();

      for (QLibraryHandle *handle : factoryObj->librarySet(serviceType) )  {
         QObject *obj = factoryObj->instance(handle);

         if (obj) {
            plugins.append(obj);
         }
      }

      if (plugins.isEmpty()) {
         return QMultimedia::NotSupported;
      }

      bool allServicesProvideInterface = true;
      QMultimedia::SupportEstimate supportEstimate = QMultimedia::NotSupported;

      for (QObject *obj : plugins) {
         QMediaServiceSupportedFormatsInterface *iface = dynamic_cast<QMediaServiceSupportedFormatsInterface *>(obj);

         if (flags) {
            QMediaServiceFeaturesInterface *iface = dynamic_cast<QMediaServiceFeaturesInterface *>(obj);

            if (iface) {
               QMediaServiceProviderHint::Features features = iface->supportedFeatures(serviceType);

               //if low latency playback was asked, skip services known
               //not to provide low latency playback
               if ((flags & QMediaPlayer::LowLatency) &&
                  !(features & QMediaServiceProviderHint::LowLatencyPlayback)) {
                  continue;
               }

               //the same for QIODevice based streams support
               if ((flags & QMediaPlayer::StreamPlayback) &&
                  !(features & QMediaServiceProviderHint::StreamPlayback)) {
                  continue;
               }
            }
         }

         if (iface) {
            supportEstimate = qMax(supportEstimate, iface->hasSupport(mimeType, codecs));
         } else {
            allServicesProvideInterface = false;
         }
      }

      // do not return PreferredService
      supportEstimate = qMin(supportEstimate, QMultimedia::ProbablySupported);

      // Return NotSupported only if no services are available of serviceType
      // or all the services returned NotSupported, otherwise return at least MaybeSupported
      if (! allServicesProvideInterface) {
         supportEstimate = qMax(QMultimedia::MaybeSupported, supportEstimate);
      }

      return supportEstimate;
   }

   QStringList supportedMimeTypes(const QString &serviceType, int flags) const override {
      QFactoryLoader *factoryObj = loader();
      QStringList supportedTypes;

      for (QLibraryHandle *handle : factoryObj->librarySet(serviceType))  {
         QObject *obj = factoryObj->instance(handle);

         if (obj == nullptr) {
            continue;
         }

         QMediaServiceSupportedFormatsInterface *iface = dynamic_cast<QMediaServiceSupportedFormatsInterface *>(obj);

         if (flags) {
            QMediaServiceFeaturesInterface *iface = dynamic_cast<QMediaServiceFeaturesInterface *>(obj);

            if (iface) {
               QMediaServiceProviderHint::Features features = iface->supportedFeatures(serviceType);

               // If low latency playback was asked for, skip MIME types from services known
               // not to provide low latency playback
               if ((flags & QMediaPlayer::LowLatency) &&
                  !(features & QMediaServiceProviderHint::LowLatencyPlayback)) {
                  continue;
               }

               //the same for QIODevice based streams support
               if ((flags & QMediaPlayer::StreamPlayback) &&
                  !(features & QMediaServiceProviderHint::StreamPlayback)) {
                  continue;
               }

               //the same for QAbstractVideoSurface support
               if ((flags & QMediaPlayer::VideoSurface) &&
                  !(features & QMediaServiceProviderHint::VideoSurface)) {
                  continue;
               }
            }
         }

         if (iface) {
            supportedTypes << iface->supportedMimeTypes();
         }
      }

      // Multiple services may support the same MIME type
      supportedTypes.removeDuplicates();

      return supportedTypes;
   }

   QString defaultDevice(const QString &serviceType) const override {
      QFactoryLoader *factoryObj = loader();

      for (QLibraryHandle *handle : factoryObj->librarySet(serviceType))  {
         QObject *obj = factoryObj->instance(handle);

         if (obj == nullptr) {
            continue;
         }

         const QMediaServiceDefaultDeviceInterface *iface = dynamic_cast<QMediaServiceDefaultDeviceInterface *>(obj);

         if (iface) {
            return iface->defaultDevice(serviceType);
         }
      }

      // if QMediaServiceDefaultDeviceInterface is not implemented, return the first available device
      QList<QString> devs = devices(serviceType);

      if (! devs.isEmpty()) {
         return devs.first();
      }

      return QString();
   }

   QList<QString> devices(const QString &serviceType) const override {
      QFactoryLoader *factoryObj = loader();
      QList<QString> list;

      for (QLibraryHandle *handle : factoryObj->librarySet(serviceType))  {
         QObject *obj = factoryObj->instance(handle);

         if (obj == nullptr) {
            continue;
         }

         QMediaServiceSupportedDevicesInterface *iface = dynamic_cast<QMediaServiceSupportedDevicesInterface *>(obj);

         if (iface) {
            list.append(iface->devices(serviceType));
         }
      }

      return list;
   }

   QString deviceDescription(const QString &serviceType, const QString &device) override {
      QFactoryLoader *factoryObj = loader();

      for (QLibraryHandle *handle : factoryObj->librarySet(serviceType) )  {
         QObject *obj = factoryObj->instance(handle);

         if (obj == nullptr) {
            continue;
         }

         QMediaServiceSupportedDevicesInterface *iface = dynamic_cast<QMediaServiceSupportedDevicesInterface *>(obj);

         if (iface) {
            if (iface->devices(serviceType).contains(device)) {
               return iface->deviceDescription(serviceType, device);
            }
         }
      }

      return QString();
   }

   QCamera::Position cameraPosition(const QString &device) const override {
     const QString serviceType(Q_MEDIASERVICE_CAMERA);

     QFactoryLoader *factoryObj = loader();

     for (QLibraryHandle *handle : factoryObj->librarySet(serviceType))  {
         QObject *obj = factoryObj->instance(handle);

         if (obj == nullptr) {
            continue;
         }

         const QMediaServiceSupportedDevicesInterface *deviceIface =
               dynamic_cast<QMediaServiceSupportedDevicesInterface*>(obj);

         const QMediaServiceCameraInfoInterface *cameraIface = dynamic_cast<QMediaServiceCameraInfoInterface*>(obj);

         if (cameraIface) {
             if (deviceIface && ! deviceIface->devices(serviceType).contains(device)) {
                continue;
             }

             return cameraIface->cameraPosition(device);
         }
     }

     return QCamera::UnspecifiedPosition;
   }

   int cameraOrientation(const QString &device) const override {
     const QString serviceType(Q_MEDIASERVICE_CAMERA);

     QFactoryLoader *factoryObj = loader();

     for (QLibraryHandle *handle : factoryObj->librarySet(serviceType)) {
         QObject *obj = factoryObj->instance(handle);

         if (obj == nullptr) {
            continue;
         }

         const QMediaServiceSupportedDevicesInterface *deviceIface =
               dynamic_cast<QMediaServiceSupportedDevicesInterface*>(obj);

         const QMediaServiceCameraInfoInterface *cameraIface = dynamic_cast<QMediaServiceCameraInfoInterface*>(obj);

         if (cameraIface) {
            if (deviceIface && ! deviceIface->devices(serviceType).contains(device)) {
               continue;
            }

            return cameraIface->cameraOrientation(device);
         }
     }

     return 0;
   }
};

QPluginServiceProvider *pluginProvider() {
   static QPluginServiceProvider retval;
   return &retval;
}

QMediaServiceProviderHint::Features QMediaServiceProvider::supportedFeatures(const QMediaService *service) const
{
   (void) service;

   return Qt::EmptyFlag;
}

QMultimedia::SupportEstimate QMediaServiceProvider::hasSupport(const QString &serviceType,
   const QString &mimeType, const QStringList &codecs, int flags) const
{
   (void) serviceType;
   (void) mimeType;
   (void) codecs;
   (void) flags;

   return QMultimedia::MaybeSupported;
}

QStringList QMediaServiceProvider::supportedMimeTypes(const QString &serviceType, int flags) const
{
   (void) serviceType;
   (void) flags;

   return QStringList();
}

QString QMediaServiceProvider::defaultDevice(const QString &serviceType) const
{
   (void) serviceType;

   return QString();
}

QList<QString> QMediaServiceProvider::devices(const QString &service) const
{
   (void) service;

   return QList<QString>();
}

QString QMediaServiceProvider::deviceDescription(const QString &serviceType, const QString &device)
{
   (void) serviceType;
   (void) device;

   return QString();
}

QCamera::Position QMediaServiceProvider::cameraPosition(const QString &device) const
{
   (void) device;

   return QCamera::UnspecifiedPosition;
}

int QMediaServiceProvider::cameraOrientation(const QString &device) const
{
   (void) device;

    return 0;
}

static QMediaServiceProvider *qt_defaultMediaServiceProvider = nullptr;

void QMediaServiceProvider::setDefaultServiceProvider(QMediaServiceProvider *provider)
{
   qt_defaultMediaServiceProvider = provider;
}

QMediaServiceProvider *QMediaServiceProvider::defaultServiceProvider()
{
   if (qt_defaultMediaServiceProvider != nullptr) {
      return qt_defaultMediaServiceProvider;

   } else {
      return static_cast<QMediaServiceProvider *>(pluginProvider());

   }
}
