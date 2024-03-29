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

#ifndef QNETWORK_REPLYDATA_P_H
#define QNETWORK_REPLYDATA_P_H

#include <qbuffer.h>
#include <qnetaccess_manager.h>
#include <qnetwork_reply.h>

#include <qnetwork_reply_p.h>

class QNetworkReplyDataImplPrivate;

class QNetworkReplyDataImpl: public QNetworkReply
{
   NET_CS_OBJECT(QNetworkReplyDataImpl)

 public:
   QNetworkReplyDataImpl(QObject *parent, const QNetworkRequest &req, const QNetworkAccessManager::Operation op);
   ~QNetworkReplyDataImpl();

   void abort() override;

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

#endif
