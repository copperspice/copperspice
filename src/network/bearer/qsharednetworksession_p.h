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

#ifndef QSHAREDNETWORKSESSION_P_H
#define QSHAREDNETWORKSESSION_P_H

#include <qnetworksession.h>
#include <qnetworkconfiguration.h>
#include <QHash>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QMutex>

#ifndef QT_NO_BEARERMANAGEMENT

QT_BEGIN_NAMESPACE

uint qHash(const QNetworkConfiguration &config);

class QSharedNetworkSessionManager
{
 public:
   static QSharedPointer<QNetworkSession> getSession(QNetworkConfiguration config);
   static void setSession(QNetworkConfiguration config, QSharedPointer<QNetworkSession> session);

 private:
   QHash<QNetworkConfiguration, QWeakPointer<QNetworkSession> > sessions;
};

QT_END_NAMESPACE

#endif // QT_NO_BEARERMANAGEMENT

#endif //QSHAREDNETWORKSESSIONPRIVATE_H

