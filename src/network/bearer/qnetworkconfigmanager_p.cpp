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

#include <qnetworkconfigmanager_p.h>

#include <qalgorithms.h>
#include <qdebug.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qthread.h>

#include <qcoreapplication_p.h>
#include <qbearerplugin_p.h>
#include <qfactoryloader_p.h>

#ifndef QT_NO_BEARERMANAGEMENT

static QFactoryLoader *loader()
{
    static QFactoryLoader retval(QBearerEngineInterface_ID, "/bearer");
    return &retval;
}

QNetworkConfigurationManagerPrivate::QNetworkConfigurationManagerPrivate()
   : QObject(), pollTimer(nullptr), bearerThread(nullptr), forcedPolling(0), firstUpdate(true)
{
}

void QNetworkConfigurationManagerPrivate::initialize()
{
   bearerThread = new QThread();
   bearerThread->setObjectName("Network Bearer Thread");

   // because cleanup() is called in main thread context.
   bearerThread->moveToThread(QCoreApplicationPrivate::mainThread());

   moveToThread(bearerThread);
   bearerThread->start();
   updateConfigurations();
}

QNetworkConfigurationManagerPrivate::~QNetworkConfigurationManagerPrivate()
{
   QRecursiveMutexLocker locker(&mutex);

   qDeleteAll(sessionEngines);
   sessionEngines.clear();

   if (bearerThread) {
      bearerThread->quit();
   }
}

void QNetworkConfigurationManagerPrivate::cleanup()
{
   QThread *thread = bearerThread;
   deleteLater();

   if (thread->wait(5000)) {
      delete thread;
   }
}

QNetworkConfiguration QNetworkConfigurationManagerPrivate::defaultConfiguration() const
{
   QRecursiveMutexLocker locker(&mutex);

   for (QBearerEngine *engine : sessionEngines) {
      QNetworkConfigurationPrivatePointer ptr = engine->defaultConfiguration();

      if (ptr) {
         QNetworkConfiguration config;
         config.d = ptr;
         return config;
      }
   }

   // Engines don't have a default configuration.

   // Return first active snap
   QNetworkConfigurationPrivatePointer defaultConfiguration;

   for (QBearerEngine *engine : sessionEngines) {
      QHash<QString, QNetworkConfigurationPrivatePointer>::iterator it;
      QHash<QString, QNetworkConfigurationPrivatePointer>::iterator end;

      QRecursiveMutexLocker locker(&engine->mutex);

      for (it = engine->snapConfigurations.begin(),
            end = engine->snapConfigurations.end(); it != end; ++it) {
         QNetworkConfigurationPrivatePointer ptr = it.value();

         QRecursiveMutexLocker configLocker(&ptr->mutex);

         if ((ptr->state & QNetworkConfiguration::Active) == QNetworkConfiguration::Active) {
            QNetworkConfiguration config;
            config.d = ptr;
            return config;

         } else if (!defaultConfiguration) {
            if ((ptr->state & QNetworkConfiguration::Discovered) == QNetworkConfiguration::Discovered) {
               defaultConfiguration = ptr;
            }
         }
      }
   }

   // No Active SNAPs return first Discovered SNAP.
   if (defaultConfiguration) {
      QNetworkConfiguration config;
      config.d = defaultConfiguration;
      return config;
   }

   /*
       No Active or Discovered SNAPs, find the perferred access point.
       The following priority order is used:

           1. Active Ethernet
           2. Active WLAN
           3. Active Other
           4. Discovered Ethernet
           5. Discovered WLAN
           6. Discovered Other
   */

   for (QBearerEngine *engine : sessionEngines) {
      QHash<QString, QNetworkConfigurationPrivatePointer>::iterator it;
      QHash<QString, QNetworkConfigurationPrivatePointer>::iterator end;

      QRecursiveMutexLocker locker(&engine->mutex);

      for (it = engine->accessPointConfigurations.begin(),
            end = engine->accessPointConfigurations.end(); it != end; ++it) {
         QNetworkConfigurationPrivatePointer ptr = it.value();

         QRecursiveMutexLocker configLocker(&ptr->mutex);
         QNetworkConfiguration::BearerType bearerType = ptr->bearerType;

         if ((ptr->state & QNetworkConfiguration::Discovered) == QNetworkConfiguration::Discovered) {
            if (! defaultConfiguration) {
               defaultConfiguration = ptr;

            } else {
               QRecursiveMutexLocker defaultConfigLocker(&defaultConfiguration->mutex);

               if (defaultConfiguration->state == ptr->state) {
                  switch (defaultConfiguration->bearerType) {
                     case QNetworkConfiguration::BearerEthernet:
                        // do nothing
                        break;

                     case QNetworkConfiguration::BearerWLAN:
                        // Ethernet beats WLAN
                        defaultConfiguration = ptr;
                        break;

                     default:
                        // Ethernet and WLAN beats other
                        if (bearerType == QNetworkConfiguration::BearerEthernet ||
                              bearerType == QNetworkConfiguration::BearerWLAN) {
                           defaultConfiguration = ptr;
                        }
                  }

               } else {
                  // active beats discovered
                  if ((defaultConfiguration->state & QNetworkConfiguration::Active) != QNetworkConfiguration::Active) {
                     defaultConfiguration = ptr;
                  }
               }
            }
         }
      }
   }

   // No Active InternetAccessPoint return first Discovered InternetAccessPoint.
   if (defaultConfiguration) {
      QNetworkConfiguration config;
      config.d = defaultConfiguration;
      return config;
   }

   return QNetworkConfiguration();
}

QList<QNetworkConfiguration> QNetworkConfigurationManagerPrivate::allConfigurations(QNetworkConfiguration::StateFlags filter) const
{
   QList<QNetworkConfiguration> result;

   QRecursiveMutexLocker locker(&mutex);

   for (QBearerEngine *engine : sessionEngines) {
      QHash<QString, QNetworkConfigurationPrivatePointer>::iterator it;
      QHash<QString, QNetworkConfigurationPrivatePointer>::iterator end;

      QRecursiveMutexLocker locker(&engine->mutex);

      //find all InternetAccessPoints
      for (it = engine->accessPointConfigurations.begin(),
            end = engine->accessPointConfigurations.end(); it != end; ++it) {
         QNetworkConfigurationPrivatePointer ptr = it.value();

         QRecursiveMutexLocker configLocker(&ptr->mutex);

         if ((ptr->state & filter) == filter) {
            QNetworkConfiguration pt;
            pt.d = ptr;
            result << pt;
         }
      }

      //find all service networks
      for (it = engine->snapConfigurations.begin(),
            end = engine->snapConfigurations.end(); it != end; ++it) {
         QNetworkConfigurationPrivatePointer ptr = it.value();

         QRecursiveMutexLocker configLocker(&ptr->mutex);

         if ((ptr->state & filter) == filter) {
            QNetworkConfiguration pt;
            pt.d = ptr;
            result << pt;
         }
      }
   }

   return result;
}

QNetworkConfiguration QNetworkConfigurationManagerPrivate::configurationFromIdentifier(const QString &identifier) const
{
   QNetworkConfiguration item;

   QRecursiveMutexLocker locker(&mutex);

   for (QBearerEngine *engine : sessionEngines) {
      QRecursiveMutexLocker locker(&engine->mutex);

      if (engine->accessPointConfigurations.contains(identifier)) {
         item.d = engine->accessPointConfigurations[identifier];

      } else if (engine->snapConfigurations.contains(identifier)) {
         item.d = engine->snapConfigurations[identifier];

      } else if (engine->userChoiceConfigurations.contains(identifier)) {
         item.d = engine->userChoiceConfigurations[identifier];

      } else {
         continue;
      }

      return item;
   }

   return item;
}

bool QNetworkConfigurationManagerPrivate::isOnline() const
{
   QRecursiveMutexLocker locker(&mutex);

   return !allConfigurations(QNetworkConfiguration::Active).isEmpty();
}

QNetworkConfigurationManager::Capabilities QNetworkConfigurationManagerPrivate::capabilities() const
{
   QRecursiveMutexLocker locker(&mutex);

   QNetworkConfigurationManager::Capabilities capFlags;

   for (QBearerEngine *engine : sessionEngines) {
      capFlags |= engine->capabilities();
   }

   return capFlags;
}

void QNetworkConfigurationManagerPrivate::configurationAdded(QNetworkConfigurationPrivatePointer ptr)
{
   QRecursiveMutexLocker locker(&mutex);

   if (! firstUpdate) {
      QNetworkConfiguration item;
      item.d = ptr;
      emit configurationAdded(item);
   }

   ptr->mutex.lock();
   if (ptr->state == QNetworkConfiguration::Active) {
      ptr->mutex.unlock();
      onlineConfigurations.insert(ptr->id);
      if (!firstUpdate && onlineConfigurations.count() == 1) {
         emit onlineStateChanged(true);
      }

   } else {
      ptr->mutex.unlock();
   }
}

void QNetworkConfigurationManagerPrivate::configurationRemoved(QNetworkConfigurationPrivatePointer ptr)
{
   QRecursiveMutexLocker locker(&mutex);

   ptr->mutex.lock();
   ptr->isValid = false;
   ptr->mutex.unlock();

   if (!firstUpdate) {
      QNetworkConfiguration item;
      item.d = ptr;
      emit configurationRemoved(item);
   }

   onlineConfigurations.remove(ptr->id);
   if (!firstUpdate && onlineConfigurations.isEmpty()) {
      emit onlineStateChanged(false);
   }
}

void QNetworkConfigurationManagerPrivate::configurationChanged(QNetworkConfigurationPrivatePointer ptr)
{
   QRecursiveMutexLocker locker(&mutex);

   if (!firstUpdate) {
      QNetworkConfiguration item;
      item.d = ptr;
      emit configurationChanged(item);
   }

   bool previous = !onlineConfigurations.isEmpty();

   ptr->mutex.lock();
   if (ptr->state == QNetworkConfiguration::Active) {
      onlineConfigurations.insert(ptr->id);
   } else {
      onlineConfigurations.remove(ptr->id);
   }
   ptr->mutex.unlock();

   bool online = !onlineConfigurations.isEmpty();

   if (!firstUpdate && online != previous) {
      emit onlineStateChanged(online);
   }
}

void QNetworkConfigurationManagerPrivate::updateConfigurations()
{
   QRecursiveMutexLocker locker(&mutex);

   if (firstUpdate) {
      if (dynamic_cast<QBearerEngine *>(sender())) {
         return;
      }

      updating    = false;
      bool envOK  = false;

      const int skipGeneric  = qgetenv("QT_EXCLUDE_GENERIC_BEARER").toInt(&envOK);
      QBearerEngine *generic = nullptr;

      QFactoryLoader *pluginLoader = loader();

      auto keySet = pluginLoader->keySet();

      for (auto item : keySet) {
         if (QBearerEngine *engine = cs_load_plugin<QBearerEngine, QBearerEnginePlugin>(pluginLoader, item)) {

            if (item == "generic") {
               generic = engine;
            } else {
               sessionEngines.append(engine);
            }

            engine->moveToThread(bearerThread);

            connect(engine, &QBearerEngine::updateCompleted,
                    this, &QNetworkConfigurationManagerPrivate::updateConfigurations, Qt::QueuedConnection);

            connect(engine, &QBearerEngine::configurationAdded,
                    this, cs_mp_cast<QNetworkConfigurationPrivatePointer>(&QNetworkConfigurationManagerPrivate::configurationAdded), Qt::QueuedConnection);

            connect(engine, &QBearerEngine::configurationRemoved,
                    this, cs_mp_cast<QNetworkConfigurationPrivatePointer>(&QNetworkConfigurationManagerPrivate::configurationRemoved), Qt::QueuedConnection);

            connect(engine, &QBearerEngine::configurationChanged,
                    this, cs_mp_cast<QNetworkConfigurationPrivatePointer>(&QNetworkConfigurationManagerPrivate::configurationChanged), Qt::QueuedConnection);
         }
      }

      if (generic) {
         if (! envOK || skipGeneric <= 0) {
            sessionEngines.append(generic);
         } else {
            delete generic;
         }
      }
   }

   QBearerEngine *engine = dynamic_cast<QBearerEngine *>(sender());

   if (engine && ! updatingEngines.isEmpty()) {
      updatingEngines.remove(engine);
   }

   if (updating && updatingEngines.isEmpty()) {
      updating = false;
      emit configurationUpdateComplete();
   }

   if (engine && !pollingEngines.isEmpty()) {
      pollingEngines.remove(engine);
      if (pollingEngines.isEmpty()) {
         startPolling();
      }
   }

   if (firstUpdate) {
      firstUpdate = false;

      //shallow copy the list in case it is modified when we unlock mutex
      QList<QBearerEngine *> enginesToInitialize = sessionEngines;
      locker.unlock();

      for (QBearerEngine *engine : enginesToInitialize) {
         QMetaObject::invokeMethod(engine, "initialize", Qt::BlockingQueuedConnection);
      }
   }
}

void QNetworkConfigurationManagerPrivate::performAsyncConfigurationUpdate()
{
   QRecursiveMutexLocker locker(&mutex);

   if (sessionEngines.isEmpty()) {
      emit configurationUpdateComplete();
      return;
   }

   updating = true;

   for (QBearerEngine *engine : sessionEngines) {
      updatingEngines.insert(engine);
      QMetaObject::invokeMethod(engine, "requestUpdate");
   }
}

QList<QBearerEngine *> QNetworkConfigurationManagerPrivate::engines() const
{
   QRecursiveMutexLocker locker(&mutex);

   return sessionEngines;
}

void QNetworkConfigurationManagerPrivate::startPolling()
{
   QRecursiveMutexLocker locker(&mutex);

   if (! pollTimer) {
      pollTimer = new QTimer(this);

      bool ok;
      int interval = qgetenv("QT_BEARER_POLL_TIMEOUT").toInt(&ok);

      if (! ok) {
         interval = 10000;//default 10 seconds
      }

      pollTimer->setInterval(interval);
      pollTimer->setSingleShot(true);
      connect(pollTimer, &QTimer::timeout, this, &QNetworkConfigurationManagerPrivate::pollEngines);
   }

   if (pollTimer->isActive()) {
      return;
   }

   for (QBearerEngine *engine : sessionEngines) {
      if (engine->requiresPolling() && (forcedPolling || engine->configurationsInUse())) {
         pollTimer->start();
         break;
      }
   }

   performAsyncConfigurationUpdate();
}

void QNetworkConfigurationManagerPrivate::pollEngines()
{
   QRecursiveMutexLocker locker(&mutex);

   for (QBearerEngine *engine : sessionEngines) {
      if (engine->requiresPolling() && (forcedPolling || engine->configurationsInUse())) {
         pollingEngines.insert(engine);
         QMetaObject::invokeMethod(engine, "requestUpdate");
      }
   }
}

void QNetworkConfigurationManagerPrivate::enablePolling()
{
   QRecursiveMutexLocker locker(&mutex);

   ++forcedPolling;

   if (forcedPolling == 1) {
      QMetaObject::invokeMethod(this, "startPolling");
   }
}

void QNetworkConfigurationManagerPrivate::disablePolling()
{
   QRecursiveMutexLocker locker(&mutex);

   --forcedPolling;
}

#endif // QT_NO_BEARERMANAGEMENT
