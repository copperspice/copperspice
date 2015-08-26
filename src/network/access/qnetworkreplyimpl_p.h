/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QNETWORKREPLYIMPL_P_H
#define QNETWORKREPLYIMPL_P_H

#include <qnetworkreply.h>
#include <qnetworkreply_p.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkproxy.h>
#include <QtCore/qmap.h>
#include <QtCore/qqueue.h>
#include <QtCore/qbuffer.h>
#include <qringbuffer_p.h>
#include <qbytedata_p.h>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE

class QAbstractNetworkCache;
class QNetworkAccessBackend;
class QNetworkReplyImplPrivate;

class QNetworkReplyImpl: public QNetworkReply
{
   NET_CS_OBJECT(QNetworkReplyImpl)

 public:
   QNetworkReplyImpl(QObject *parent = 0);
   ~QNetworkReplyImpl();
   virtual void abort();

   // reimplemented from QNetworkReply / QIODevice
   virtual void close();
   virtual qint64 bytesAvailable() const;
   virtual void setReadBufferSize(qint64 size);
   virtual bool canReadLine () const;

   virtual qint64 readData(char *data, qint64 maxlen);
   virtual bool event(QEvent *);

#ifndef QT_NO_OPENSSL
   virtual void ignoreSslErrors();

   virtual QSslConfiguration sslConfigurationImplementation() const override;
   virtual void setSslConfigurationImplementation(const QSslConfiguration &configuration) override;
   virtual void ignoreSslErrorsImplementation(const QList<QSslError> &errors) override;
#endif

   Q_DECLARE_PRIVATE(QNetworkReplyImpl)

   NET_CS_SLOT_1(Private, void _q_startOperation())
   NET_CS_SLOT_2(_q_startOperation)

   NET_CS_SLOT_1(Private, void _q_copyReadyRead())
   NET_CS_SLOT_2(_q_copyReadyRead)

   NET_CS_SLOT_1(Private, void _q_copyReadChannelFinished())
   NET_CS_SLOT_2(_q_copyReadChannelFinished)

   NET_CS_SLOT_1(Private, void _q_bufferOutgoingData())
   NET_CS_SLOT_2(_q_bufferOutgoingData)

   NET_CS_SLOT_1(Private, void _q_bufferOutgoingDataFinished())
   NET_CS_SLOT_2(_q_bufferOutgoingDataFinished)

#ifndef QT_NO_BEARERMANAGEMENT
   NET_CS_SLOT_1(Private, void _q_networkSessionConnected())
   NET_CS_SLOT_2(_q_networkSessionConnected)

   NET_CS_SLOT_1(Private, void _q_networkSessionFailed())
   NET_CS_SLOT_2(_q_networkSessionFailed)
#endif

   NET_CS_SLOT_1(Private, void _q_cacheDestroyed())
   NET_CS_SLOT_2(_q_cacheDestroyed)

   NET_CS_SLOT_1(Private, void _q_cacheSaveDeviceAboutToClose())
   NET_CS_SLOT_2(_q_cacheSaveDeviceAboutToClose)
};

class QNetworkReplyImplPrivate: public QNetworkReplyPrivate
{
 public:
   enum InternalNotifications {
      NotifyDownstreamReadyWrite,
      NotifyCloseDownstreamChannel,
      NotifyCopyFinished
   };

   enum State {
      Idle,               // The reply is idle.
      Buffering,          // The reply is buffering outgoing data.
      Working,            // The reply is uploading/downloading data.
      Finished,           // The reply has finished.
      Aborted,            // The reply has been aborted.
      WaitingForSession,  // The reply is waiting for the session to open before connecting.
      Reconnecting        // The reply will reconnect to once roaming has completed.
   };

   typedef QQueue<InternalNotifications> NotificationQueue;

   QNetworkReplyImplPrivate();

   void _q_startOperation();
   void _q_sourceReadyRead();
   void _q_sourceReadChannelFinished();
   void _q_copyReadyRead();
   void _q_copyReadChannelFinished();
   void _q_bufferOutgoingData();
   void _q_bufferOutgoingDataFinished();

#ifndef QT_NO_BEARERMANAGEMENT
   void _q_networkSessionConnected();
   void _q_networkSessionFailed();
#endif

   void _q_cacheDestroyed();
   void _q_cacheSaveDeviceAboutToClose();

   void setup(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData);

   void pauseNotificationHandling();
   void resumeNotificationHandling();
   void backendNotify(InternalNotifications notification);
   void handleNotifications();
   void createCache();
   void completeCacheSave();

   // callbacks from the backend (through the manager):
   void setCachingEnabled(bool enable);
   bool isCachingEnabled() const;
   void consume(qint64 count);
   void emitUploadProgress(qint64 bytesSent, qint64 bytesTotal);
   qint64 nextDownstreamBlockSize() const;

   void initCacheSaveDevice();
   void appendDownstreamDataSignalEmissions();
   void appendDownstreamData(QByteDataBuffer &data);
   void appendDownstreamData(QIODevice *data);
   void appendDownstreamData(const QByteArray &data);

   void setDownloadBuffer(QSharedPointer<char> sp, qint64 size);
   char *getDownloadBuffer(qint64 size);
   void appendDownstreamDataDownloadBuffer(qint64, qint64);

   void finished();
   void error(QNetworkReply::NetworkError code, const QString &errorString);
   void metaDataChanged();
   void redirectionRequested(const QUrl &target);
   void sslErrors(const QList<QSslError> &errors);

   QNetworkAccessBackend *backend;
   QIODevice *outgoingData;
   QSharedPointer<QRingBuffer> outgoingDataBuffer;
   QIODevice *copyDevice;
   QAbstractNetworkCache *networkCache() const;

   bool migrateBackend();

   bool cacheEnabled;
   QIODevice *cacheSaveDevice;

   NotificationQueue pendingNotifications;
   bool notificationHandlingPaused;

   QUrl urlForLastAuthentication;

#ifndef QT_NO_NETWORKPROXY
   QNetworkProxy lastProxyAuthentication;
   QList<QNetworkProxy> proxyList;
#endif

   // Used for normal downloading. For "zero copy" the downloadBuffer is used
   QByteDataBuffer readBuffer;
   qint64 bytesDownloaded;
   qint64 lastBytesDownloaded;
   qint64 bytesUploaded;
   qint64 preMigrationDownloaded;

   QString httpReasonPhrase;
   int httpStatusCode;

   State state;

   // only used when the "zero copy" style is used. Else readBuffer is used.
   // Please note that the whole "zero copy" download buffer API is private right now. Do not use it.
   qint64 downloadBufferReadPosition;
   qint64 downloadBufferCurrentSize;
   qint64 downloadBufferMaximumSize;
   QSharedPointer<char> downloadBufferPointer;
   char *downloadBuffer;

   Q_DECLARE_PUBLIC(QNetworkReplyImpl)
};

#ifndef QT_NO_BEARERMANAGEMENT
class QDisabledNetworkReply : public QNetworkReply
{
   NET_CS_OBJECT(QDisabledNetworkReply)

 public:
   QDisabledNetworkReply(QObject *parent, const QNetworkRequest &req, QNetworkAccessManager::Operation op);
   ~QDisabledNetworkReply();

   void abort()
   { }

 protected:
   qint64 readData(char *, qint64) {
      return -1;
   }
};
#endif

QT_END_NAMESPACE

#endif
