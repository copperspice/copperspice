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

#ifndef QHTTP_THREADDELEGATE_P_H
#define QHTTP_THREADDELEGATE_P_H

#include <qobject.h>
#include <qlist.h>
#include <qsharedpointer.h>
#include <qthreadstorage.h>
#include <qnetworkproxy.h>
#include <qsslconfiguration.h>
#include <qsslerror.h>
#include <qnetwork_reply.h>
#include <qsslconfiguration.h>

#include <qhttp_networkrequest_p.h>
#include <qhttp_networkconnection_p.h>
#include <qnoncontiguousbytedevice_p.h>
#include <qnetaccess_authenticationmanager_p.h>

class QAuthenticator;
class QHttpNetworkReply;
class QEventLoop;
class QNetworkAccessCache;
class QNetworkAccessCachedHttpConnection;

class QHttpThreadDelegate : public QObject
{
   NET_CS_OBJECT(QHttpThreadDelegate)

 public:
   explicit QHttpThreadDelegate(QObject *parent = nullptr);

   ~QHttpThreadDelegate();

   // incoming
   bool ssl;

#ifdef QT_SSL
   QSslConfiguration incomingSslConfiguration;
#endif

   QHttpNetworkRequest httpRequest;
   qint64 downloadBufferMaximumSize;
   qint64 readBufferMaxSize;
   qint64 bytesEmitted;

   // From backend, modified by us for signal compression
   QSharedPointer<QAtomicInt> pendingDownloadData;
   QSharedPointer<QAtomicInt> pendingDownloadProgress;

#ifndef QT_NO_NETWORKPROXY
   QNetworkProxy cacheProxy;
   QNetworkProxy transparentProxy;
#endif

   QSharedPointer<QNetworkAccessAuthenticationManager> authenticationManager;
   bool synchronous;

   // outgoing, Retrieved in the synchronous HTTP case
   QByteArray synchronousDownloadData;
   QList<QPair<QByteArray, QByteArray> > incomingHeaders;
   int incomingStatusCode;
   QString incomingReasonPhrase;
   bool isPipeliningUsed;
   bool isSpdyUsed;
   qint64 incomingContentLength;
   QNetworkReply::NetworkError incomingErrorCode;
   QString incomingErrorDetail;

#ifndef QT_NO_BEARERMANAGEMENT
   QSharedPointer<QNetworkSession> networkSession;
#endif

   NET_CS_SIGNAL_1(Public, void authenticationRequired(const QHttpNetworkRequest &request, QAuthenticator *authenticator))
   NET_CS_SIGNAL_2(authenticationRequired, request, authenticator)

#ifndef QT_NO_NETWORKPROXY
   NET_CS_SIGNAL_1(Public, void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator))
   NET_CS_SIGNAL_2(proxyAuthenticationRequired, proxy, authenticator)
#endif

#ifdef QT_SSL
   NET_CS_SIGNAL_1(Public, void encrypted())
   NET_CS_SIGNAL_2(encrypted)

   NET_CS_SIGNAL_1(Public, void sslErrors(const QList <QSslError> &errorList, bool *ignoreAll, QList <QSslError> *toBeIgnored))
   NET_CS_SIGNAL_2(sslErrors, errorList, ignoreAll, toBeIgnored)

   NET_CS_SIGNAL_1(Public, void sslConfigurationChanged(const QSslConfiguration &sslConfig))
   NET_CS_SIGNAL_2(sslConfigurationChanged, sslConfig)

   NET_CS_SIGNAL_1(Public, void preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *data))
   NET_CS_SIGNAL_2(preSharedKeyAuthenticationRequired, data)
#endif

   NET_CS_SIGNAL_1(Public, void downloadMetaData(const QList <QPair <QByteArray,QByteArray>> &headers, int statusCode,
         const QString &reason, bool isPipelined, QSharedPointer <char> downloadBuffer, qint64 contentLength, bool isSpdy))
   NET_CS_SIGNAL_2(downloadMetaData, headers, statusCode, reason, isPipelined, downloadBuffer, contentLength, isSpdy)

   NET_CS_SIGNAL_1(Public, void downloadProgress(qint64 bytesReceived, qint64 bytesTotal))
   NET_CS_SIGNAL_2(downloadProgress, bytesReceived, bytesTotal)

   NET_CS_SIGNAL_1(Public, void downloadData(const QByteArray &sizeEmitted))
   NET_CS_SIGNAL_2(downloadData, sizeEmitted)

   NET_CS_SIGNAL_1(Public, void error(QNetworkReply::NetworkError errorCode, const QString &errorMsg))
   NET_CS_SIGNAL_2(error, errorCode, errorMsg)

   NET_CS_SIGNAL_1(Public, void downloadFinished())
   NET_CS_SIGNAL_2(downloadFinished)

   NET_CS_SIGNAL_1(Public, void redirected(const QUrl &url, int httpStatus, int maxRedirectsRemaining))
   NET_CS_SIGNAL_2(redirected, url, httpStatus, maxRedirectsRemaining)

   // This are called via QueuedConnection from user thread
   NET_CS_SLOT_1(Public, void startRequest())
   NET_CS_SLOT_2(startRequest)

   NET_CS_SLOT_1(Public, void abortRequest())
   NET_CS_SLOT_2(abortRequest)

   NET_CS_SLOT_1(Public, void readBufferSizeChanged(qint64 size))
   NET_CS_SLOT_2(readBufferSizeChanged)

   NET_CS_SLOT_1(Public, void readBufferFreed(qint64 size))
   NET_CS_SLOT_2(readBufferFreed)

   // This is called with a BlockingQueuedConnection from user thread
   NET_CS_SLOT_1(Public, void startRequestSynchronously())
   NET_CS_SLOT_2(startRequestSynchronously)

 protected:
   // The zerocopy download buffer, if used:
   QSharedPointer<char> downloadBuffer;

   // The QHttpNetworkConnection that is used
   QNetworkAccessCachedHttpConnection *httpConnection;
   QByteArray cacheKey;
   QHttpNetworkReply *httpReply;

   // Used for implementing the synchronous HTTP, see startRequestSynchronously()
   QEventLoop *synchronousRequestLoop;

   // Cache for all the QHttpNetworkConnection objects. This is per thread.
   static QThreadStorage<QNetworkAccessCache *> connections;

   // From QHttp*
   NET_CS_SLOT_1(Protected, void readyReadSlot())
   NET_CS_SLOT_2(readyReadSlot)

   NET_CS_SLOT_1(Protected, void finishedSlot())
   NET_CS_SLOT_2(finishedSlot)

   NET_CS_SLOT_1(Protected, void finishedWithErrorSlot(QNetworkReply::NetworkError errorCode,
                  const QString &detail = QString()))
   NET_CS_SLOT_2(finishedWithErrorSlot)

   NET_CS_SLOT_1(Protected, void synchronousFinishedSlot())
   NET_CS_SLOT_2(synchronousFinishedSlot)

   NET_CS_SLOT_1(Protected, void synchronousFinishedWithErrorSlot(QNetworkReply::NetworkError errorCode,
                  const QString &detail = QString()))
   NET_CS_SLOT_2(synchronousFinishedWithErrorSlot)

   NET_CS_SLOT_1(Protected, void headerChangedSlot())
   NET_CS_SLOT_2(headerChangedSlot)

   NET_CS_SLOT_1(Protected, void synchronousHeaderChangedSlot())
   NET_CS_SLOT_2(synchronousHeaderChangedSlot)

   NET_CS_SLOT_1(Protected, void dataReadProgressSlot(qint64 done, qint64 total))
   NET_CS_SLOT_2(dataReadProgressSlot)

   NET_CS_SLOT_1(Protected, void cacheCredentialsSlot(const QHttpNetworkRequest &request, QAuthenticator *authenticator))
   NET_CS_SLOT_2(cacheCredentialsSlot)

#ifdef QT_SSL
   NET_CS_SLOT_1(Protected, void sslErrorsSlot(const QList <QSslError> &errors))
   NET_CS_SLOT_2(sslErrorsSlot)

   NET_CS_SLOT_1(Protected, void encryptedSlot())
   NET_CS_SLOT_2(encryptedSlot)

   NET_CS_SLOT_1(Protected, void preSharedKeyAuthenticationRequiredSlot(QSslPreSharedKeyAuthenticator *authenticator))
   NET_CS_SLOT_2(preSharedKeyAuthenticationRequiredSlot)
#endif

   NET_CS_SLOT_1(Protected, void synchronousAuthenticationRequiredSlot(const QHttpNetworkRequest &request,
         QAuthenticator *authenticator))
   NET_CS_SLOT_2(synchronousAuthenticationRequiredSlot)

#ifndef QT_NO_NETWORKPROXY
   NET_CS_SLOT_1(Protected, void synchronousProxyAuthenticationRequiredSlot(const QNetworkProxy &proxy,
         QAuthenticator *authenticator))
   NET_CS_SLOT_2(synchronousProxyAuthenticationRequiredSlot)
#endif

};

// This QNonContiguousByteDevice is connected to the QNetworkAccessHttpBackend
// and represents the PUT/POST data.
class QNonContiguousByteDeviceThreadForwardImpl : public QNonContiguousByteDevice
{
   NET_CS_OBJECT(QNonContiguousByteDeviceThreadForwardImpl)

 public:
   QNonContiguousByteDeviceThreadForwardImpl(bool aE, qint64 s)
      : QNonContiguousByteDevice(), wantDataPending(false), m_amount(0), m_data(nullptr),
        m_atEnd(aE), m_size(s), m_pos(0)
   {
   }

   ~QNonContiguousByteDeviceThreadForwardImpl() {
   }

   qint64 pos() override {
      return m_pos;
   }

   const char *readPointer(qint64 maximumLength, qint64 &len) override {
      if (m_amount > 0) {
         len = m_amount;
         return m_data;
      }

      if (m_atEnd) {
         len = -1;

      } else if (!wantDataPending) {
         len = 0;
         wantDataPending = true;
         emit wantData(maximumLength);

      } else {
         // Do nothing, we already sent a wantData signal and wait for results
         len = 0;
      }

      return nullptr;
   }

   bool advanceReadPointer(qint64 a) override {
      if (m_data == nullptr) {
         return false;
      }

      m_amount -= a;
      m_data   += a;
      m_pos    += a;

      // To main thread to inform about our state. The m_pos will be sent as a sanity check.
      emit processedData(m_pos, a);

      return true;
   }

   bool atEnd() override {
      if (m_amount > 0) {
         return false;
      } else {
         return m_atEnd;
      }
   }

   bool reset() override {
      m_amount = 0;
      m_data   = nullptr;
      m_dataArray.clear();

      if (wantDataPending) {
         // had requested the user thread to send some data (only 1 in-flight at any moment)
         wantDataPending = false;
      }

      // Communicate as BlockingQueuedConnection
      bool b = false;
      emit resetData(&b);

      if (b) {
	      // the reset succeeded, we're at pos 0 again
         m_pos = 0;

         // the HTTP code will anyway abort the request if !b.
      }

      return b;
   }

   qint64 size() override {
      return m_size;
   }

   // From user thread
   NET_CS_SLOT_1(Public, void haveDataSlot(qint64 pos, const QByteArray &dataArray, bool dataAtEnd, qint64 dataSize))
   NET_CS_SLOT_2(haveDataSlot)

   // to main thread
   NET_CS_SIGNAL_1(Public, void wantData(qint64 length))
   NET_CS_SIGNAL_2(wantData, length)

   NET_CS_SIGNAL_1(Public, void processedData(qint64 pos, qint64 amount))
   NET_CS_SIGNAL_2(processedData, pos, amount)

   NET_CS_SIGNAL_1(Public, void resetData(bool *b))
   NET_CS_SIGNAL_2(resetData, b)

 protected:
   bool wantDataPending;
   qint64 m_amount;
   char *m_data;
   QByteArray m_dataArray;
   bool m_atEnd;
   qint64 m_size;
   qint64 m_pos;
};

#endif
