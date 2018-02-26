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

#ifndef QNETWORKREPLYDATAIMPL_P_H
#define QNETWORKREPLYDATAIMPL_P_H

#include <qnetworkreply.h>
#include <qnetworkreply_p.h>
#include <qnetworkaccessmanager.h>
#include <QBuffer>



class QNetworkReplyDataImplPrivate;

class QNetworkReplyDataImpl: public QNetworkReply
{
   NET_CS_OBJECT(QNetworkReplyDataImpl)

 public:
   QNetworkReplyDataImpl(QObject *parent, const QNetworkRequest &req, const QNetworkAccessManager::Operation op);
   ~QNetworkReplyDataImpl();

   void abort() override;

   // reimplemented from QNetworkReply
   void close() override;
   qint64 bytesAvailable() const override;
   bool isSequential () const override;
   qint64 size() const override;

   qint64 readData(char *data, qint64 maxlen) override;

   Q_DECLARE_PRIVATE(QNetworkReplyDataImpl)
};

class QNetworkReplyDataImplPrivate: public QNetworkReplyPrivate
{
 public:
   QNetworkReplyDataImplPrivate();
   ~QNetworkReplyDataImplPrivate();

   QBuffer decodedData;

   Q_DECLARE_PUBLIC(QNetworkReplyDataImpl)
};


#endif // QNETWORKREPLYDATAIMPL_H
