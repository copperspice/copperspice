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

#ifndef QNETWORK_REPLYFILE_P_H
#define QNETWORK_REPLYFILE_P_H

#include <qnetwork_reply.h>

#include <qabstractfileengine.h>
#include <qfile.h>
#include <qnetaccess_manager.h>

#include <qnetwork_reply_p.h>

class QNetworkReplyFileImplPrivate;

class QNetworkReplyFileImpl: public QNetworkReply
{
   NET_CS_OBJECT(QNetworkReplyFileImpl)

 public:
   QNetworkReplyFileImpl(QObject *parent, const QNetworkRequest &req, const QNetworkAccessManager::Operation op);
   ~QNetworkReplyFileImpl();

   void abort() override;
   void close() override;

   qint64 bytesAvailable() const override;
   bool isSequential () const override;
   qint64 size() const override;

   qint64 readData(char *data, qint64 maxlen) override;

   Q_DECLARE_PRIVATE(QNetworkReplyFileImpl)
};

class QNetworkReplyFileImplPrivate: public QNetworkReplyPrivate
{
 public:
   QNetworkReplyFileImplPrivate();

   QFile realFile;
   qint64 realFileSize;

   Q_DECLARE_PUBLIC(QNetworkReplyFileImpl)
};

#endif
