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

#include <qnetworkconfigmanager.h>

#include <qbearerengine_p.h>
#include <qstringlist.h>
#include <qcoreapplication.h>
#include <qmutex.h>
#include <qthread.h>

#include <qcoreapplication_p.h>
#include <qnetworkconfigmanager_p.h>

#ifndef QT_NO_BEARERMANAGEMENT

static QAtomicPointer<QNetworkConfigurationManagerPrivate> connManager_ptr;
static QAtomicInt appShutdown;

static void connManager_prepare()
{
   int shutdown = appShutdown.fetchAndStoreAcquire(0);

   Q_ASSERT(shutdown == 0 || shutdown == 1);
   (void) shutdown;
}

static void connManager_cleanup()
{
   // this is not atomic or thread-safe!
   int shutdown = appShutdown.fetchAndStoreAcquire(1);
   Q_ASSERT(shutdown == 0);

   QNetworkConfigurationManagerPrivate *cmp = connManager_ptr.fetchAndStoreAcquire(nullptr);

   if (cmp) {
      cmp->cleanup();
   }
}

void QNetworkConfigurationManagerPrivate::addPreAndPostRoutine()
{
   qAddPreRoutine(connManager_prepare);
   qAddPostRoutine(connManager_cleanup);
}

QNetworkConfigurationManagerPrivate *qNetworkConfigurationManagerPrivate()
{
   QNetworkConfigurationManagerPrivate *ptr = connManager_ptr.loadAcquire();
   int shutdown = appShutdown.loadAcquire();

   if (! ptr && !shutdown) {
      static QMutex connManager_mutex;

      QMutexLocker locker(&connManager_mutex);

      if (! (ptr = connManager_ptr.loadAcquire())) {
         ptr = new QNetworkConfigurationManagerPrivate;

         if (QCoreApplicationPrivate::mainThread() == QThread::currentThread()) {
            // right thread or no main thread yet
            ptr->addPreAndPostRoutine();
            ptr->initialize();

         } else {
            // wrong thread, we need to make the main thread do this
            QObject *obj = new QObject;

            QObject::connect(obj, &QObject::destroyed,
                  ptr, &QNetworkConfigurationManagerPrivate::addPreAndPostRoutine, Qt::DirectConnection);

            ptr->initialize();       // moves this object to the right thread
            obj->moveToThread(QCoreApplicationPrivate::mainThread());
            obj->deleteLater();
         }

         connManager_ptr.storeRelease(ptr);
      }
   }

   return ptr;
}

QNetworkConfigurationManager::QNetworkConfigurationManager(QObject *parent)
   : QObject(parent)
{
   QNetworkConfigurationManagerPrivate *priv = qNetworkConfigurationManagerPrivate();

   connect(priv, cs_mp_cast<const QNetworkConfiguration &>(&QNetworkConfigurationManagerPrivate::configurationAdded),
           this, &QNetworkConfigurationManager::configurationAdded);

   connect(priv, cs_mp_cast<const QNetworkConfiguration &>(&QNetworkConfigurationManagerPrivate::configurationRemoved),
           this, &QNetworkConfigurationManager::configurationRemoved);

   connect(priv, cs_mp_cast<const QNetworkConfiguration &>(&QNetworkConfigurationManagerPrivate::configurationChanged),
           this, &QNetworkConfigurationManager::configurationChanged);

   connect(priv, &QNetworkConfigurationManagerPrivate::onlineStateChanged,
           this, &QNetworkConfigurationManager::onlineStateChanged);

   connect(priv, &QNetworkConfigurationManagerPrivate::configurationUpdateComplete,
           this, &QNetworkConfigurationManager::updateCompleted);

   priv->enablePolling();
}

QNetworkConfigurationManager::~QNetworkConfigurationManager()
{
   QNetworkConfigurationManagerPrivate *priv = qNetworkConfigurationManagerPrivate();

   if (priv) {
      priv->disablePolling();
   }
}

QNetworkConfiguration QNetworkConfigurationManager::defaultConfiguration() const
{
   QNetworkConfigurationManagerPrivate *priv = qNetworkConfigurationManagerPrivate();

   if (priv) {
      return priv->defaultConfiguration();
   }

   return QNetworkConfiguration();
}

QList<QNetworkConfiguration> QNetworkConfigurationManager::allConfigurations(QNetworkConfiguration::StateFlags filter) const
{
   QNetworkConfigurationManagerPrivate *priv = qNetworkConfigurationManagerPrivate();

   if (priv) {
      return priv->allConfigurations(filter);
   }

   return QList<QNetworkConfiguration>();
}

QNetworkConfiguration QNetworkConfigurationManager::configurationFromIdentifier(const QString &identifier) const
{
   QNetworkConfigurationManagerPrivate *priv = qNetworkConfigurationManagerPrivate();

   if (priv) {
      return priv->configurationFromIdentifier(identifier);
   }

   return QNetworkConfiguration();
}

bool QNetworkConfigurationManager::isOnline() const
{
   QNetworkConfigurationManagerPrivate *priv = qNetworkConfigurationManagerPrivate();

   if (priv) {
      return priv->isOnline();
   }

   return false;
}

QNetworkConfigurationManager::Capabilities QNetworkConfigurationManager::capabilities() const
{
   QNetworkConfigurationManagerPrivate *priv = qNetworkConfigurationManagerPrivate();

   if (priv) {
      return priv->capabilities();
   }

   return QNetworkConfigurationManager::Capabilities(Qt::EmptyFlag);
}

void QNetworkConfigurationManager::updateConfigurations()
{
   QNetworkConfigurationManagerPrivate *priv = qNetworkConfigurationManagerPrivate();

   if (priv) {
      priv->performAsyncConfigurationUpdate();
   }
}

#endif // QT_NO_BEARERMANAGEMENT
