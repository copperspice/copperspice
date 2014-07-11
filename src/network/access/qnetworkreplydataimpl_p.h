/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QNETWORKREPLYDATAIMPL_P_H
#define QNETWORKREPLYDATAIMPL_P_H

#include <qnetworkreply.h>
#include <qnetworkreply_p.h>
#include <qnetworkaccessmanager.h>
#include <QBuffer>

QT_BEGIN_NAMESPACE

class QNetworkReplyDataImplPrivate;

class QNetworkReplyDataImpl: public QNetworkReply
{
   CS_OBJECT(QNetworkReplyDataImpl)

 public:
   QNetworkReplyDataImpl(QObject *parent, const QNetworkRequest &req, const QNetworkAccessManager::Operation op);
   ~QNetworkReplyDataImpl();
   virtual void abort();

   // reimplemented from QNetworkReply
   virtual void close();
   virtual qint64 bytesAvailable() const;
   virtual bool isSequential () const;
   qint64 size() const;

   virtual qint64 readData(char *data, qint64 maxlen);

   Q_DECLARE_PRIVATE(QNetworkReplyDataImpl)
};

class QNetworkReplyDataImplPrivate: public QNetworkReplyPrivate
{
 public:
   QNetworkReplyDataImplPrivate();
   ~QNetworkReplyDataImplPrivate();

   QPair<QString, QByteArray> decodeDataUrlResult;
   QBuffer decodedData;

   Q_DECLARE_PUBLIC(QNetworkReplyDataImpl)
};

QT_END_NAMESPACE

#endif // QNETWORKREPLYDATAIMPL_H
