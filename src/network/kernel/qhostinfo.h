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

#ifndef QHOSTINFO_H
#define QHOSTINFO_H

#include <QtCore/qlist.h>
#include <QtCore/qscopedpointer.h>
#include <QtNetwork/qhostaddress.h>

QT_BEGIN_NAMESPACE

class QObject;
class QHostInfoPrivate;

class Q_NETWORK_EXPORT QHostInfo
{

 public:
   enum HostInfoError {
      NoError,
      HostNotFound,
      UnknownError
   };

   explicit QHostInfo(int lookupId = -1);
   QHostInfo(const QHostInfo &d);
   QHostInfo &operator=(const QHostInfo &d);
   ~QHostInfo();

   QString hostName() const;
   void setHostName(const QString &name);

   QList<QHostAddress> addresses() const;
   void setAddresses(const QList<QHostAddress> &addresses);

   HostInfoError error() const;
   void setError(HostInfoError error);

   QString errorString() const;
   void setErrorString(const QString &errorString);

   void setLookupId(int id);
   int lookupId() const;

   static int lookupHost(const QString &name, QObject *receiver, const char *member);
   static void abortHostLookup(int lookupId);

   static QHostInfo fromName(const QString &name);
   static QString localHostName();
   static QString localDomainName();

 private:
   QScopedPointer<QHostInfoPrivate> d;
};

QT_END_NAMESPACE

#endif // QHOSTINFO_H
