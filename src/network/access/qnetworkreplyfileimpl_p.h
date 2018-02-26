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

#ifndef QNETWORKREPLYFILEIMPL_P_H
#define QNETWORKREPLYFILEIMPL_P_H

#include <qnetworkreply.h>
#include <qnetworkreply_p.h>
#include <qnetworkaccessmanager.h>
#include <QFile>
#include <QAbstractFileEngine>

QT_BEGIN_NAMESPACE

class QNetworkReplyFileImplPrivate;

class QNetworkReplyFileImpl: public QNetworkReply
{
   NET_CS_OBJECT(QNetworkReplyFileImpl)

 public:
   QNetworkReplyFileImpl(QObject *parent, const QNetworkRequest &req, const QNetworkAccessManager::Operation op);
   ~QNetworkReplyFileImpl();

   void abort() override;

   // reimplemented from QNetworkReply
   void close() override;
   qint64 bytesAvailable() const override;
   bool isSequential () const override;
   qint64 size() const override;

   virtual qint64 readData(char *data, qint64 maxlen) override;

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

QT_END_NAMESPACE

#endif // QNETWORKREPLYFILEIMPL_H
