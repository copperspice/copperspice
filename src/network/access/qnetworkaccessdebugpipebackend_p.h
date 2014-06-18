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

#ifndef QNETWORKACCESSDEBUGPIPEBACKEND_P_H
#define QNETWORKACCESSDEBUGPIPEBACKEND_P_H

#include "qnetworkaccessbackend_p.h"
#include "qnetworkrequest.h"
#include "qnetworkreply.h"
#include "qtcpsocket.h"

QT_BEGIN_NAMESPACE

#ifdef QT_BUILD_INTERNAL

class QNetworkAccessDebugPipeBackend: public QNetworkAccessBackend
{
   CS_OBJECT(QNetworkAccessDebugPipeBackend)

 public:
   QNetworkAccessDebugPipeBackend();
   virtual ~QNetworkAccessDebugPipeBackend();

   virtual void open();
   virtual void closeDownstreamChannel();

   virtual void downstreamReadyWrite();

 protected:
   void pushFromSocketToDownstream();
   void pushFromUpstreamToSocket();
   void possiblyFinish();
   QNonContiguousByteDevice *uploadByteDevice;

 private :
   NET_CS_SLOT_1(Private, void uploadReadyReadSlot())
   NET_CS_SLOT_2(uploadReadyReadSlot)
   NET_CS_SLOT_1(Private, void socketReadyRead())
   NET_CS_SLOT_2(socketReadyRead)
   NET_CS_SLOT_1(Private, void socketBytesWritten(qint64 bytes))
   NET_CS_SLOT_2(socketBytesWritten)
   NET_CS_SLOT_1(Private, void socketError())
   NET_CS_SLOT_2(socketError)
   NET_CS_SLOT_1(Private, void socketDisconnected())
   NET_CS_SLOT_2(socketDisconnected)
   NET_CS_SLOT_1(Private, void socketConnected())
   NET_CS_SLOT_2(socketConnected)

   QTcpSocket socket;
   bool bareProtocol;
   bool hasUploadFinished;
   bool hasDownloadFinished;
   bool hasEverythingFinished;

   qint64 bytesDownloaded;
   qint64 bytesUploaded;
};

class QNetworkAccessDebugPipeBackendFactory: public QNetworkAccessBackendFactory
{
 public:
   virtual QNetworkAccessBackend *create(QNetworkAccessManager::Operation op,
                                         const QNetworkRequest &request) const;
};

#endif  // QT_BUILD_INTERNAL

QT_END_NAMESPACE

#endif
