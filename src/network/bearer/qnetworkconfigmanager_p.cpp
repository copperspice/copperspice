/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qnetworkconfigmanager_p.h>
#include <qbearerplugin_p.h>
#include <qfactoryloader_p.h>
#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qthread.h>
#include <qcoreapplication_p.h>

#ifndef QT_NO_BEARERMANAGEMENT

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QBearerEngineFactoryInterface_iid, QLatin1String("/bearer")))

QNetworkConfigurationManagerPrivate::QNetworkConfigurationManagerPrivate()
   : QObject(), pollTimer(0), bearerThread(0),  mutex(QMutex::Recursive), forcedPolling(0), firstUpdate(true)
{
   qRegisterMetaType<QNetworkConfiguration>();
   qRegisterMetaType<QNetworkConfigurationPrivatePointer>();
}

void QNetworkConfigurationManagerPrivate::initialize()
{
   //Two stage construction, because we only want to do this heavyweight work for the winner of the Q_GLOBAL_STATIC race.
   bearerThread = new QThread();
   bearerThread->setObjectName("Network Bearer Thread");
   bearerThread->moveToThread(QCoreApplicationPrivate::mainThread());    // because cleanup() is called in main thread context.
   moveToThread(bearerThread);
   bearerThread->start();
   updateConfigurations();
}

QNetworkConfigurationManagerPrivate::~QNetworkConfigurationManagerPrivate()
{
   QMutexLocker locker(&mutex);

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
   QMutexLocker locker(&mutex);

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
      QHash<QString, QNetworkConfigurationPrivatePointer>::Iterator it;
      QHash<QString, QNetworkConfigurationPrivatePointer>::Iterator end;

      QMutexLocker locker(&engine->mutex);

      for (it = engine->snapConfigurations.begin(),
            end = engine->snapConfigurations.end(); it != end; ++it) {
         QNetworkConfigurationPrivatePointer ptr = it.value();

         QMutexLocker configLocker(&ptr->mutex);

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
      QHash<QString, QNetworkConfigurationPrivatePointer>::Iterator it;
      QHash<QString, QNetworkConfigurationPrivatePointer>::Iterator end;

      QMutexLocker locker(&engine->mutex);

      for (it = engine->accessPointConfigurations.begin(),
            end = engine->accessPointConfigurations.end(); it != end; ++it) {
         QNetworkConfigurationPrivatePointer ptr = it.value();

         QMutexLocker configLocker(&ptr->mutex);
         QNetworkConfiguration::BearerType bearerType = ptr->bearerType;

         if ((ptr->state & QNetworkConfiguration::Discovered) == QNetworkConfiguration::Discovered) {
            if (!defaultConfiguration) {
               defaultConfiguration = ptr;
            } else {
               QMutexLocker defaultConfigLocker(&defaultConfiguration->mutex);

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
                  if ((defaultConfiguration->state & QNetworkConfiguration::Active) !=
                        QNetworkConfiguration::Active) {
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

   QMutexLocker locker(&mutex);

   for (QBearerEngine *engine : sessionEngines) {
      QHash<QString, QNetworkConfigurationPrivatePointer>::Iterator it;
      QHash<QString, QNetworkConfigurationPrivatePointer>::Iterator end;

      QMutexLocker locker(&engine->mutex);

      //find all InternetAccessPoints
      for (it = engine->accessPointConfigurations.begin(),
            end = engine->accessPointConfigurations.end(); it != end; ++it) {
         QNetworkConfigurationPrivatePointer ptr = it.value();

         QMutexLocker configLocker(&ptr->mutex);

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

         QMutexLocker configLocker(&ptr->mutex);

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

   QMutexLocker locker(&mutex);

   for (QBearerEngine *engine : sessionEngines) {
      QMutexLocker locker(&engine->mutex);

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
   QMutexLocker locker(&mutex);

   return !allConfigurations(QNetworkConfiguration::Active).isEmpty();
}

QNetworkConfigurationManager::Capabilities QNetworkConfigurationManagerPrivate::capabilities() const
{
   QMutexLocker locker(&mutex);

   QNetworkConfigurationManager::Capabilities capFlags;

   for (QBearerEngine *engine : sessionEngines) {
      capFlags |= engine->capabilities();
   }

   return capFlags;
}

void QNetworkConfigurationManagerPrivate::configurationAdded(QNetworkConfigurationPrivatePointer ptr)
{
   QMutexLocker locker(&mutex);

   if (!firstUpdate) {
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
   QMutexLocker locker(&mutex);

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
   QMutexLocker locker(&mutex);

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
   // typedef QMultiMap<int, QString> PluginKeyMap;
   // typedef PluginKeyMap::const_iterator PluginKeyMapConstIterator;

   QMutexLocker locker(&mutex);

   if (firstUpdate) {
      if (qobject_cast<QBearerEngine *>(sender())) {
         return;
      }

      updating = false;

      QBearerEngine *generic = nullptr;
      QFactoryLoader *l = loader();

      /*
            // const PluginKeyMap keyMap = l->keyMap();
            // const PluginKeyMapConstIterator cend = keyMap.constEnd();
            // QStringList addedEngines;

            for (PluginKeyMapConstIterator it = keyMap.constBegin(); it != cend; ++it) {
               const QString &key = it.value();

               if (addedEngines.contains(key))  {
                  continue;
               }

               addedEngines.append(key);
               if (QBearerEngine *engine = qLoadPlugin<QBearerEngine, QBearerEnginePlugin>(l, key)) {
               }
      */

      for (const QString &key : l->keys())  {
         QBearerEnginePlugin *plugin = qobject_cast<QBearerEnginePlugin *>(l->instance(key));

         if (plugin) {
            QBearerEngine *engine = plugin->create(key);

            if (! engine) {
               continue;
            }

            if (key == "generic") {
               generic = engine;
            } else {
               sessionEngines.append(engine);
            }

            engine->moveToThread(bearerThread);

            connect(engine, SIGNAL(updateCompleted()),
                    this, SLOT(updateConfigurations()), Qt::QueuedConnection);

            connect(engine, SIGNAL(configurationAdded(QNetworkConfigurationPrivatePointer)),
                    this, SLOT(configurationAdded(QNetworkConfigurationPrivatePointer)), Qt::QueuedConnection);

            connect(engine, SIGNAL(configurationRemoved(QNetworkConfigurationPrivatePointer)),
                    this, SLOT(configurationRemoved(QNetworkConfigurationPrivatePointer)), Qt::QueuedConnection);

            connect(engine, SIGNAL(configurationChanged(QNetworkConfigurationPrivatePointer)),
                    this, SLOT(configurationChanged(QNetworkConfigurationPrivatePointer)), Qt::QueuedConnection);
         }
      }

      if (generic) {
         sessionEngines.append(generic);
      }
   }

   QBearerEngine *engine = qobject_cast<QBearerEngine *>(sender());

   if (engine && !updatingEngines.isEmpty()) {
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
   QMutexLocker locker(&mutex);

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
   QMutexLocker locker(&mutex);

   return sessionEngines;
}

void QNetworkConfigurationManagerPrivate::startPolling()
{
   QMutexLocker locker(&mutex);

   if (!pollTimer) {
      pollTimer = new QTimer(this);
      bool ok;
      int interval = qgetenv("QT_BEARER_POLL_TIMEOUT").toInt(&ok);

      if (! ok) {
         interval = 10000;//default 10 seconds
      }

      pollTimer->setInterval(interval);
      pollTimer->setSingleShot(true);
      connect(pollTimer, SIGNAL(timeout()), this, SLOT(pollEngines()));
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
   QMutexLocker locker(&mutex);

   for (QBearerEngine *engine : sessionEngines) {
      if (engine->requiresPolling() && (forcedPolling || engine->configurationsInUse())) {
         pollingEngines.insert(engine);
         QMetaObject::invokeMethod(engine, "requestUpdate");
      }
   }
}

void QNetworkConfigurationManagerPrivate::enablePolling()
{
   QMutexLocker locker(&mutex);

   ++forcedPolling;

   if (forcedPolling == 1) {
      QMetaObject::invokeMethod(this, "startPolling");
   }
}

void QNetworkConfigurationManagerPrivate::disablePolling()
{
   QMutexLocker locker(&mutex);

   --forcedPolling;
}


#endif // QT_NO_BEARERMANAGEMENT
