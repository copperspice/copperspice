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

#ifndef QSHAREDNETWORK_SESSION_P_H
#define QSHAREDNETWORK_SESSION_P_H

#include <qhash.h>
#include <qmutex.h>
#include <qnetworkconfiguration.h>
#include <qnetworksession.h>
#include <qsharedpointer.h>
#include <qweakpointer.h>

#ifndef QT_NO_BEARERMANAGEMENT

uint qHash(const QNetworkConfiguration &config);

class QSharedNetworkSessionManager
{
 public:
   static QSharedPointer<QNetworkSession> getSession(QNetworkConfiguration config);
   static void setSession(QNetworkConfiguration config, QSharedPointer<QNetworkSession> session);

 private:
   QHash<QNetworkConfiguration, QWeakPointer<QNetworkSession> > sessions;
};

#endif // QT_NO_BEARERMANAGEMENT

#endif

