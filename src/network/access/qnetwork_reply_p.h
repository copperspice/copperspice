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

#ifndef QNETWORK_REPLY_P_H
#define QNETWORK_REPLY_P_H

#include <qnetwork_reply.h>

#include <qnetwork_request.h>
#include <qpointer.h>
#include <qelapsedtimer.h>

#include <qiodevice_p.h>
#include <qnetwork_request_p.h>

class QNetworkReplyPrivate: public QIODevicePrivate, public QNetworkHeadersPrivate
{
 public:
    enum ReplyState {
        Idle,               // The reply is idle.
        Buffering,          // The reply is buffering outgoing data.
        Working,            // The reply is uploading/downloading data.
        Finished,           // The reply has finished.
        Aborted,            // The reply has been aborted.
        WaitingForSession,  // The reply is waiting for the session to open before connecting.
        Reconnecting        // The reply will reconnect to once roaming has completed.
   };

   QNetworkReplyPrivate();

   static void setManager(QNetworkReply *reply, QNetworkAccessManager *manager) {
      reply->d_func()->manager = manager;
   }

   QNetworkRequest request;
   QNetworkRequest originalRequest;
   QUrl url;
   QPointer<QNetworkAccessManager> manager;
   qint64 readBufferMaxSize;

   QElapsedTimer downloadProgressSignalChoke;
   QElapsedTimer uploadProgressSignalChoke;

   bool emitAllUploadProgressSignals;
   const static int progressSignalInterval;
   QNetworkAccessManager::Operation operation;
   QNetworkReply::NetworkError m_errorCode;
   bool isFinished;

   Q_DECLARE_PUBLIC(QNetworkReply)
};

#endif
