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

#include <qdebug.h>
#include <qmap.h>

#include <qmediaservice.h>
#include <qmediaplayer.h>
#include <qmediaplayercontrol.h>

#include <qfactoryloader_p.h>
#include <qmediaserviceprovider_p.h>
#include <qmediaservice_provider_plugin.h>

class QMediaServiceProviderHintPrivate : public QSharedData
{
 public:
   QMediaServiceProviderHintPrivate(QMediaServiceProviderHint::Type type)
      : type(type),
        // emerald    cameraPosition(QCamera::UnspecifiedPosition),
        features(0) {
   }

   QMediaServiceProviderHintPrivate(const QMediaServiceProviderHintPrivate &other)
      : QSharedData(other),
        type(other.type),
        device(other.device),
        // emerald    cameraPosition(other.cameraPosition),
        mimeType(other.mimeType),
        codecs(other.codecs),
        features(other.features) {
   }

   ~QMediaServiceProviderHintPrivate() {
   }

   QMediaServiceProviderHint::Type type;
   QString device;
   // emerald        QCamera::Position cameraPosition;
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


/* emerald

QMediaServiceProviderHint::QMediaServiceProviderHint(QCamera::Position position)
    :d(new QMediaServiceProviderHintPrivate(CameraPosition))
{
    d->cameraPosition = position;
}

*/


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

/*!
    Identifies if \a other is of equal value to a media service provider hint.

    Returns true if the hints are equal, and false if they are not.
*/
bool QMediaServiceProviderHint::operator == (const QMediaServiceProviderHint &other) const
{
   return (d == other.d) ||
      (d->type == other.d->type &&
         d->device == other.d->device &&
         // emerald       d->cameraPosition == other.d->cameraPosition &&
         d->mimeType == other.d->mimeType &&
         d->codecs == other.d->codecs &&
         d->features == other.d->features);
}

/*!
    Identifies if \a other is not of equal value to a media service provider hint.

    Returns true if the hints are not equal, and false if they are.
*/
bool QMediaServiceProviderHint::operator != (const QMediaServiceProviderHint &other) const
{
   return !(*this == other);
}

/*!
    Returns true if a media service provider is null.
*/
bool QMediaServiceProviderHint::isNull() const
{
   return d->type == Null;
}

/*!
    Returns the type of a media service provider hint.
*/
QMediaServiceProviderHint::Type QMediaServiceProviderHint::type() const
{
   return d->type;
}

/*!
    Returns the mime type of the media a service is expected to be able play.
*/
QString QMediaServiceProviderHint::mimeType() const
{
   return d->mimeType;
}

/*!
    Returns a list of codes a media service is expected to be able to decode.
*/
QStringList QMediaServiceProviderHint::codecs() const
{
   return d->codecs;
}

QString QMediaServiceProviderHint::device() const
{
   return d->device;
}


/* emerald

QCamera::Position QMediaServiceProviderHint::cameraPosition() const
{
    return d->cameraPosition;
}

*/

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

      MediaServiceData() : plugin(0) { }
   };

   QMap<const QMediaService *, MediaServiceData> mediaServiceData;

 public:
   QMediaService *requestService(const QString &key, const QMediaServiceProviderHint &hint) {

      QList<QMediaServiceProviderPlugin *>plugins;
      QFactoryLoader *factoryObj = loader();

      // what keys are available
      const QSet<QString> keySet = factoryObj->keySet();

      for (QString tmpKey : keySet)  {

         for (QLibraryHandle *handle : factoryObj->librarySet(tmpKey) )  {
            QObject *obj = factoryObj->instance(handle);

            QMediaServiceProviderPlugin *plugin = dynamic_cast<QMediaServiceProviderPlugin *>(obj);

            if (plugin) {
               plugins.append(plugin);
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

               if (type == Q_MEDIASERVICE_MEDIAPLAYER) {
                  for (QMediaServiceProviderPlugin *currentPlugin : plugins) {

                     QMediaServiceFeaturesInterface *iface = dynamic_cast<QMediaServiceFeaturesInterface *>(currentPlugin);

                     if (! iface || ! (iface->supportedFeatures(type) & QMediaServiceProviderHint::LowLatencyPlayback)) {
                        plugin = currentPlugin;
                        break;
                     }
                  }
               }
               break;

            case QMediaServiceProviderHint::SupportedFeatures:
               plugin = plugins[0];
               foreach (QMediaServiceProviderPlugin *currentPlugin, plugins) {
                  QMediaServiceFeaturesInterface *iface = qobject_cast<QMediaServiceFeaturesInterface *>(currentPlugin);

                  if (iface) {
                     if ((iface->supportedFeatures(type) & hint.features()) == hint.features()) {
                        plugin = currentPlugin;
                        break;
                     }
                  }
               }
               break;

            case QMediaServiceProviderHint::Device: {
               plugin = plugins[0];
               foreach (QMediaServiceProviderPlugin *currentPlugin, plugins) {
                  QMediaServiceSupportedDevicesInterface *iface =
                     qobject_cast<QMediaServiceSupportedDevicesInterface *>(currentPlugin);

                  if (iface && iface->devices(type).contains(hint.device())) {
                     plugin = currentPlugin;
                     break;
                  }
               }
            }
            break;


            /* emerald

                        case QMediaServiceProviderHint::CameraPosition: {
                                plugin = plugins[0];
                                if (type == QByteArray(Q_MEDIASERVICE_CAMERA)
                                        && hint.cameraPosition() != QCamera::UnspecifiedPosition) {
                                    foreach (QMediaServiceProviderPlugin *currentPlugin, plugins) {
                                        const QMediaServiceSupportedDevicesInterface *deviceIface =
                                                qobject_cast<QMediaServiceSupportedDevicesInterface*>(currentPlugin);
                                        const QMediaServiceCameraInfoInterface *cameraIface =
                                                qobject_cast<QMediaServiceCameraInfoInterface*>(currentPlugin);

                                        if (deviceIface && cameraIface) {
                                            const QList<QByteArray> cameras = deviceIface->devices(type);
                                            foreach (const QByteArray &camera, cameras) {
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
            */

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
                     plugin = currentPlugin;

                     if (currentEstimate == QMultimedia::PreferredService) {
                        break;
                     }
                  }
               }
            }
            break;
         }

         if (plugin != 0) {
            QMediaService *service = plugin->create(key);

            if (service != 0) {
               MediaServiceData d;
               d.type = type;
               d.plugin = plugin;
               mediaServiceData.insert(service, d);
            }

            return service;
         }
      }

      qWarning() << "QPluginServiceProvider::requestService(): No plugin was found for QMediaService with the key of: " << key;
      return 0;
   }

   void releaseService(QMediaService *service) {
      if (service != 0) {
         MediaServiceData d = mediaServiceData.take(service);

         if (d.plugin != 0) {
            d.plugin->release(service);
         }
      }
   }

   QMediaServiceProviderHint::Features supportedFeatures(const QMediaService *service) const {
      if (service) {
         MediaServiceData d = mediaServiceData.value(service);

         if (d.plugin) {
            QMediaServiceFeaturesInterface *iface =
               qobject_cast<QMediaServiceFeaturesInterface *>(d.plugin);

            if (iface) {
               return iface->supportedFeatures(d.type);
            }
         }
      }

      return QMediaServiceProviderHint::Features();
   }

   QMultimedia::SupportEstimate hasSupport(const QString &serviceType, const QString &mimeType,
         const QStringList &codecs, int flags) const {

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

   QStringList supportedMimeTypes(const QString &serviceType, int flags) const {
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

   QString defaultDevice(const QString &serviceType) const {
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

   QList<QString> devices(const QString &serviceType) const {
      QFactoryLoader *factoryObj = loader();
      QList<QString> list;

      for (QLibraryHandle *handle : factoryObj->librarySet(serviceType))  {
         QObject *obj = factoryObj->instance(handle);

         if (obj == nullptr) {
            continue;
         }

         QMediaServiceSupportedDevicesInterface *iface = qobject_cast<QMediaServiceSupportedDevicesInterface *>(obj);

         if (iface) {
            list.append(iface->devices(serviceType));
         }
      }

      return list;
   }

   QString deviceDescription(const QString &serviceType, const QString &device) {
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


   /* emerald

       QCamera::Position cameraPosition(const QString &device) const
       {
           const QString serviceType(Q_MEDIASERVICE_CAMERA);

           for (QObject *obj : loader()->instances(serviceType)) {
               const QMediaServiceSupportedDevicesInterface *deviceIface =
                       dynamic_cast<QMediaServiceSupportedDevicesInterface*>(obj);

               const QMediaServiceCameraInfoInterface *cameraIface =
                       dynamic_cast<QMediaServiceCameraInfoInterface*>(obj);

               if (cameraIface) {
                   if (deviceIface && !deviceIface->devices(serviceType).contains(device)) {
                      continue;
                   }

                   return cameraIface->cameraPosition(device);
               }
           }

           return QCamera::UnspecifiedPosition;
       }

       int cameraOrientation(const QString &device) const
       {
           const QString serviceType(Q_MEDIASERVICE_CAMERA);

           for (QObject *obj : loader()->instances(serviceType)) {
               const QMediaServiceSupportedDevicesInterface *deviceIface =
                       dynamic_cast<QMediaServiceSupportedDevicesInterface*>(obj);

               const QMediaServiceCameraInfoInterface *cameraIface =
                       dynamic_cast<QMediaServiceCameraInfoInterface*>(obj);

               if (cameraIface) {
                  if (deviceIface && !deviceIface->devices(serviceType).contains(device)) {
                     continue;
                  }

                  return cameraIface->cameraOrientation(device);
               }
           }

           return 0;
       }
   */

};

Q_GLOBAL_STATIC(QPluginServiceProvider, pluginProvider);

QMediaServiceProviderHint::Features QMediaServiceProvider::supportedFeatures(const QMediaService *service) const
{
   return QMediaServiceProviderHint::Features(0);
}

QMultimedia::SupportEstimate QMediaServiceProvider::hasSupport(const QString &serviceType,
   const QString &mimeType, const QStringList &codecs, int flags) const
{
   return QMultimedia::MaybeSupported;
}

QStringList QMediaServiceProvider::supportedMimeTypes(const QString &serviceType, int flags) const
{
   Q_UNUSED(serviceType);
   Q_UNUSED(flags);

   return QStringList();
}

QString QMediaServiceProvider::defaultDevice(const QString &serviceType) const
{
   return QString();
}

QList<QString> QMediaServiceProvider::devices(const QString &service) const
{
   return QList<QString>();
}

QString QMediaServiceProvider::deviceDescription(const QString &serviceType, const QString &device)
{
   Q_UNUSED(serviceType);
   Q_UNUSED(device);
   return QString();
}


/* emerald

QCamera::Position QMediaServiceProvider::cameraPosition(const QString &device) const
{
    return QCamera::UnspecifiedPosition;
}

int QMediaServiceProvider::cameraOrientation(const QString &device) const
{
    return 0;
}

*/

static QMediaServiceProvider *qt_defaultMediaServiceProvider = 0;

void QMediaServiceProvider::setDefaultServiceProvider(QMediaServiceProvider *provider)
{
   qt_defaultMediaServiceProvider = provider;
}

QMediaServiceProvider *QMediaServiceProvider::defaultServiceProvider()
{
   return qt_defaultMediaServiceProvider != 0
      ? qt_defaultMediaServiceProvider
      : static_cast<QMediaServiceProvider *>(pluginProvider());
}

