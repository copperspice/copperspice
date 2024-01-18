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

#include <qsharednetworksession_p.h>

#include <qthreadstorage.h>

#include <qbearerengine_p.h>

#ifndef QT_NO_BEARERMANAGEMENT

QThreadStorage<QSharedNetworkSessionManager *> tls;

inline QSharedNetworkSessionManager *sharedNetworkSessionManager()
{
   QSharedNetworkSessionManager *rv = tls.localData();
   if (!rv) {
      rv = new QSharedNetworkSessionManager;
      tls.setLocalData(rv);
   }
   return rv;
}

static void doDeleteLater(QObject *obj)
{
   obj->deleteLater();
}

QSharedPointer<QNetworkSession> QSharedNetworkSessionManager::getSession(QNetworkConfiguration config)
{
   QSharedNetworkSessionManager *m(sharedNetworkSessionManager());
   //if already have a session, return it
   if (m->sessions.contains(config)) {
      QSharedPointer<QNetworkSession> p = m->sessions.value(config).toStrongRef();
      if (!p.isNull()) {
         return p;
      }
   }

   //otherwise make one
   QSharedPointer<QNetworkSession> session(new QNetworkSession(config), doDeleteLater);
   m->sessions[config] = session;
   return session;
}

void QSharedNetworkSessionManager::setSession(QNetworkConfiguration config, QSharedPointer<QNetworkSession> session)
{
   QSharedNetworkSessionManager *m(sharedNetworkSessionManager());
   m->sessions[config] = session;
}

uint qHash(const QNetworkConfiguration &config)
{
   return ((uint)config.type()) | (((uint)config.bearerType()) << 8) | (((uint)config.purpose()) << 16);
}

#endif // QT_NO_BEARERMANAGEMENT
