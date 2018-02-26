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

#ifndef QNETWORKREPLY_P_H
#define QNETWORKREPLY_P_H

#include <qnetworkrequest.h>
#include <qnetworkrequest_p.h>
#include <qnetworkreply.h>
#include <QtCore/qpointer.h>
#include <QtCore/QElapsedTimer>
#include <qiodevice_p.h>


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
   QNetworkReply::NetworkError errorCode;
   bool isFinished;

   static inline void setManager(QNetworkReply *reply, QNetworkAccessManager *manager) {
      reply->d_func()->manager = manager;
   }

   Q_DECLARE_PUBLIC(QNetworkReply)
};



#endif
