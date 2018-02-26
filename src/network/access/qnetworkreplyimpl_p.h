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

#ifndef QNETWORKREPLYIMPL_P_H
#define QNETWORKREPLYIMPL_P_H

#include <qnetworkreply.h>
#include <qnetworkreply_p.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkproxy.h>
#include <qnetworksession.h>

#include <qbuffer.h>
#include <qbytedata_p.h>
#include <qmap.h>
#include <qqueue.h>
#include <qringbuffer_p.h>

#include <QSharedPointer>

class QAbstractNetworkCache;
class QNetworkAccessBackend;
class QNetworkReplyImplPrivate;

class QNetworkReplyImpl: public QNetworkReply
{
   NET_CS_OBJECT(QNetworkReplyImpl)

 public:
   QNetworkReplyImpl(QObject *parent = nullptr);
   ~QNetworkReplyImpl();

   virtual void abort() override;

   // reimplemented from QNetworkReply or QIODevice
   void close() override;
   qint64 bytesAvailable() const override;
   void setReadBufferSize(qint64 size) override;
   bool canReadLine () const override;

   qint64 readData(char *data, qint64 maxlen) override;
   bool event(QEvent *) override;

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

   NET_CS_SLOT_1(Private, void _q_networkSessionStateChanged(QNetworkSession::State))
   NET_CS_SLOT_2(_q_networkSessionStateChanged)

   NET_CS_SLOT_1(Private, void _q_networkSessionUsagePoliciesChanged(QNetworkSession::UsagePolicies))
   NET_CS_SLOT_2(_q_networkSessionUsagePoliciesChanged)
#endif

#ifdef QT_SSL
protected:
   virtual void ignoreSslErrors();
   virtual void ignoreSslErrorsImplementation(const QList<QSslError> &errors) override;

   void sslConfigurationImplementation(QSslConfiguration &configuration) const override;
   void setSslConfigurationImplementation(const QSslConfiguration &configuration) override;
#endif

};

class QNetworkReplyImplPrivate: public QNetworkReplyPrivate
{
 public:
   enum InternalNotifications {
      NotifyDownstreamReadyWrite,
      NotifyCloseDownstreamChannel,
      NotifyCopyFinished
   };

   typedef QQueue<InternalNotifications> NotificationQueue;

   QNetworkReplyImplPrivate();

   void _q_startOperation();

   void _q_copyReadyRead();
   void _q_copyReadChannelFinished();
   void _q_bufferOutgoingData();
   void _q_bufferOutgoingDataFinished();

#ifndef QT_NO_BEARERMANAGEMENT
   void _q_networkSessionConnected();
   void _q_networkSessionFailed();
   void _q_networkSessionStateChanged(QNetworkSession::State);
   void _q_networkSessionUsagePoliciesChanged(QNetworkSession::UsagePolicies);
#endif

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
   void encrypted();
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

   ReplyState state;

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

   void abort() override { }

 protected:
   qint64 readData(char *, qint64) override {
      return -1;
   }
};
#endif

Q_DECLARE_METATYPE(QSharedPointer<char>)

#endif
