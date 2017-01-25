/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QNETWORKACCESSHTTPBACKEND_P_H
#define QNETWORKACCESSHTTPBACKEND_P_H

#include <qhttpnetworkconnection_p.h>
#include <qnetworkaccessbackend_p.h>
#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include <qabstractsocket.h>
#include <QtCore/qpointer.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qsharedpointer.h>
#include <qatomic.h>

#ifndef QT_NO_HTTP

QT_BEGIN_NAMESPACE

class QNetworkAccessCachedHttpConnection;
class QNetworkAccessHttpBackendIODevice;

class QNetworkAccessHttpBackend: public QNetworkAccessBackend
{
   NET_CS_OBJECT(QNetworkAccessHttpBackend)

 public:
   QNetworkAccessHttpBackend();
   virtual ~QNetworkAccessHttpBackend();

   void open() override;
   void closeDownstreamChannel() override;

   void downstreamReadyWrite() override;
   void setDownstreamLimited(bool b) override;
   void setReadBufferSize(qint64 size) override;
   void emitReadBufferFreed(qint64 size) override;

   void copyFinished(QIODevice *) override;

#ifndef QT_NO_OPENSSL
   virtual void ignoreSslErrors();
   virtual void ignoreSslErrors(const QList<QSslError> &errors);

   virtual void fetchSslConfiguration(QSslConfiguration &configuration) const;
   virtual void setSslConfiguration(const QSslConfiguration &configuration);
#endif

   QNetworkCacheMetaData fetchCacheMetaData(const QNetworkCacheMetaData &metaData) const override;

   // we return true since HTTP needs to send PUT/POST data again after having authenticated
   bool needsResetableUploadData() override  {
      return true;
   }

   bool canResume() const override;
   void setResumeOffset(quint64 offset) override;

   // To HTTP thread:
   NET_CS_SIGNAL_1(Public, void startHttpRequest())
   NET_CS_SIGNAL_2(startHttpRequest)
   NET_CS_SIGNAL_1(Public, void abortHttpRequest())
   NET_CS_SIGNAL_2(abortHttpRequest)
   NET_CS_SIGNAL_1(Public, void readBufferSizeChanged(qint64 size))
   NET_CS_SIGNAL_2(readBufferSizeChanged, size)
   NET_CS_SIGNAL_1(Public, void readBufferFreed(qint64 size))
   NET_CS_SIGNAL_2(readBufferFreed, size)

   NET_CS_SIGNAL_1(Public, void startHttpRequestSynchronously())
   NET_CS_SIGNAL_2(startHttpRequestSynchronously)

   NET_CS_SIGNAL_1(Public, void haveUploadData(const qint64 pos, const QByteArray &dataArray, bool dataAtEnd, qint64 dataSize))
   NET_CS_SIGNAL_2(haveUploadData, pos, dataArray, dataAtEnd, dataSize)

 private :
   // From HTTP thread:
   NET_CS_SLOT_1(Private, void replyDownloadData(const QByteArray &un_named_arg1))
   NET_CS_SLOT_2(replyDownloadData)

   NET_CS_SLOT_1(Private, void replyFinished())
   NET_CS_SLOT_2(replyFinished)

   NET_CS_SLOT_1(Private, void replyDownloadMetaData(
                    const QList <QPair <QByteArray, QByteArray>> &un_named_arg1, int un_named_arg2, const QString &un_named_arg3,
                    bool un_named_arg4, QSharedPointer <char> un_named_arg5, qint64 un_named_arg6))

   NET_CS_SLOT_2(replyDownloadMetaData)
   NET_CS_SLOT_1(Private, void replyDownloadProgressSlot(qint64 un_named_arg1, qint64 un_named_arg2))
   NET_CS_SLOT_2(replyDownloadProgressSlot)
   NET_CS_SLOT_1(Private, void httpAuthenticationRequired(const QHttpNetworkRequest &request, QAuthenticator *auth))
   NET_CS_SLOT_2(httpAuthenticationRequired)
   NET_CS_SLOT_1(Private, void httpError(QNetworkReply::NetworkError error, const QString &errorString))
   NET_CS_SLOT_2(httpError)

#ifndef QT_NO_OPENSSL
   NET_CS_SLOT_1(Private, void replySslErrors(const QList <QSslError> &un_named_arg1, bool *un_named_arg2,
                 QList <QSslError> *un_named_arg3))

   NET_CS_SLOT_2(replySslErrors)
   NET_CS_SLOT_1(Private, void replySslConfigurationChanged(const QSslConfiguration &un_named_arg1))
   NET_CS_SLOT_2(replySslConfigurationChanged)
#endif

   // From QNonContiguousByteDeviceThreadForwardImpl in HTTP thread:
   NET_CS_SLOT_1(Private, void resetUploadDataSlot(bool *r))
   NET_CS_SLOT_2(resetUploadDataSlot)
   NET_CS_SLOT_1(Private, void wantUploadDataSlot(qint64 un_named_arg1))
   NET_CS_SLOT_2(wantUploadDataSlot)
   NET_CS_SLOT_1(Private, void sentUploadDataSlot(qint64 un_named_arg1, qint64 un_named_arg2))
   NET_CS_SLOT_2(sentUploadDataSlot)

   NET_CS_SLOT_1(Private, bool sendCacheContents(const QNetworkCacheMetaData &metaData))
   NET_CS_SLOT_2(sendCacheContents)

   QHttpNetworkRequest httpRequest; // There is also a copy in the HTTP thread
   int statusCode;
   qint64 uploadByteDevicePosition;
   QString reasonPhrase;

   // Will be increased by HTTP thread:
   QSharedPointer<QAtomicInt> pendingDownloadDataEmissions;
   QSharedPointer<QAtomicInt> pendingDownloadProgressEmissions;
   bool loadingFromCache;
   QByteDataBuffer pendingDownloadData;
   bool usingZerocopyDownloadBuffer;

#ifndef QT_NO_OPENSSL
   QSslConfiguration *pendingSslConfiguration;
   bool pendingIgnoreAllSslErrors;
   QList<QSslError> pendingIgnoreSslErrorsList;
#endif
   quint64 resumeOffset;

   bool loadFromCacheIfAllowed(QHttpNetworkRequest &httpRequest);
   void invalidateCache();
   void postRequest();
   void readFromHttp();
   void checkForRedirect(const int statusCode);
};

class QNetworkAccessHttpBackendFactory : public QNetworkAccessBackendFactory
{
 public:
   virtual QNetworkAccessBackend *create(QNetworkAccessManager::Operation op, const QNetworkRequest &request) const override;
};

QT_END_NAMESPACE

#endif // QT_NO_HTTP

#endif
