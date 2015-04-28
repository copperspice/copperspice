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

#ifndef QHTTPTHREADDELEGATE_P_H
#define QHTTPTHREADDELEGATE_P_H

#include <QObject>
#include <QThreadStorage>
#include <QNetworkProxy>
#include <QSslConfiguration>
#include <QSslError>
#include <QList>
#include <QNetworkReply>
#include <qhttpnetworkrequest_p.h>
#include <qhttpnetworkconnection_p.h>
#include <QSharedPointer>
#include <qsslconfiguration.h>
#include <qnoncontiguousbytedevice_p.h>
#include <qnetworkaccessauthenticationmanager_p.h>

#ifndef QT_NO_HTTP

QT_BEGIN_NAMESPACE

class QAuthenticator;
class QHttpNetworkReply;
class QEventLoop;
class QNetworkAccessCache;
class QNetworkAccessCachedHttpConnection;

class QHttpThreadDelegate : public QObject
{
   CS_OBJECT(QHttpThreadDelegate)

 public:
   explicit QHttpThreadDelegate(QObject *parent = 0);

   ~QHttpThreadDelegate();

   // incoming
   bool ssl;

#ifndef QT_NO_OPENSSL
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
   qint64 incomingContentLength;
   QNetworkReply::NetworkError incomingErrorCode;
   QString incomingErrorDetail;

#ifndef QT_NO_BEARERMANAGEMENT
   QSharedPointer<QNetworkSession> networkSession;
#endif

 protected:
   // The zerocopy download buffer, if used:
   QSharedPointer<char> downloadBuffer;
   // The QHttpNetworkConnection that is used
   QNetworkAccessCachedHttpConnection *httpConnection;
   QByteArray cacheKey;
   QHttpNetworkReply *httpReply;

   // Used for implementing the synchronous HTTP, see startRequestSynchronously()
   QEventLoop *synchronousRequestLoop;

 public:
   NET_CS_SIGNAL_1(Public, void authenticationRequired(const QHttpNetworkRequest &request, QAuthenticator *un_named_arg2))
   NET_CS_SIGNAL_2(authenticationRequired, request, un_named_arg2)

#ifndef QT_NO_NETWORKPROXY
   NET_CS_SIGNAL_1(Public, void proxyAuthenticationRequired(const QNetworkProxy &un_named_arg1,
                   QAuthenticator *un_named_arg2))
   NET_CS_SIGNAL_2(proxyAuthenticationRequired, un_named_arg1, un_named_arg2)
#endif

#ifndef QT_NO_OPENSSL
   NET_CS_SIGNAL_1(Public, void sslErrors(const QList <QSslError> &un_named_arg1, bool *un_named_arg2,
                                          QList <QSslError> *un_named_arg3))
   NET_CS_SIGNAL_2(sslErrors, un_named_arg1, un_named_arg2, un_named_arg3)

   NET_CS_SIGNAL_1(Public, void sslConfigurationChanged(const QSslConfiguration &un_named_arg1))
   NET_CS_SIGNAL_2(sslConfigurationChanged, un_named_arg1)
#endif

   NET_CS_SIGNAL_1(Public, void downloadMetaData(const QList <QPair <QByteArray, QByteArray>> &un_named_arg1,
                   int un_named_arg2, const QString &un_named_arg3, bool un_named_arg4, 
                   QSharedPointer <char> un_named_arg5, qint64 un_named_arg6))

   NET_CS_SIGNAL_2(downloadMetaData, un_named_arg1, un_named_arg2, un_named_arg3, un_named_arg4, un_named_arg5,
                   un_named_arg6)

   NET_CS_SIGNAL_1(Public, void downloadProgress(qint64 un_named_arg1, qint64 un_named_arg2))
   NET_CS_SIGNAL_2(downloadProgress, un_named_arg1, un_named_arg2)

   NET_CS_SIGNAL_1(Public, void downloadData(const QByteArray &un_named_arg1))
   NET_CS_SIGNAL_2(downloadData, un_named_arg1)

   NET_CS_SIGNAL_1(Public, void error(QNetworkReply::NetworkError un_named_arg1, const QString &un_named_arg2))
   NET_CS_SIGNAL_2(error, un_named_arg1, un_named_arg2)

   NET_CS_SIGNAL_1(Public, void downloadFinished())
   NET_CS_SIGNAL_2(downloadFinished)

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

 protected :
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
   NET_CS_SLOT_1(Protected, void dataReadProgressSlot(int done, int total))
   NET_CS_SLOT_2(dataReadProgressSlot)
   NET_CS_SLOT_1(Protected, void cacheCredentialsSlot(const QHttpNetworkRequest &request, QAuthenticator *authenticator))
   NET_CS_SLOT_2(cacheCredentialsSlot)
#ifndef QT_NO_OPENSSL
   NET_CS_SLOT_1(Protected, void sslErrorsSlot(const QList <QSslError> &errors))
   NET_CS_SLOT_2(sslErrorsSlot)
#endif

   NET_CS_SLOT_1(Protected, void synchronousAuthenticationRequiredSlot(const QHttpNetworkRequest &request,
                 QAuthenticator  *un_named_arg2))
   NET_CS_SLOT_2(synchronousAuthenticationRequiredSlot)
#ifndef QT_NO_NETWORKPROXY
   NET_CS_SLOT_1(Protected, void synchronousProxyAuthenticationRequiredSlot(const QNetworkProxy &un_named_arg1,
                 QAuthenticator *un_named_arg2))
   NET_CS_SLOT_2(synchronousProxyAuthenticationRequiredSlot)
#endif

 protected:
   // Cache for all the QHttpNetworkConnection objects.
   // This is per thread.
   static QThreadStorage<QNetworkAccessCache *> connections;
};

// This QNonContiguousByteDevice is connected to the QNetworkAccessHttpBackend
// and represents the PUT/POST data.
class QNonContiguousByteDeviceThreadForwardImpl : public QNonContiguousByteDevice
{
   CS_OBJECT(QNonContiguousByteDeviceThreadForwardImpl)

 protected:
   bool wantDataPending;
   qint64 m_amount;
   char *m_data;
   QByteArray m_dataArray;
   bool m_atEnd;
   qint64 m_size;
   qint64 m_pos;

 public:
   QNonContiguousByteDeviceThreadForwardImpl(bool aE, qint64 s)
      : QNonContiguousByteDevice(),
        wantDataPending(false),
        m_amount(0),
        m_data(0),
        m_atEnd(aE),
        m_size(s),
        m_pos(0) {
   }

   ~QNonContiguousByteDeviceThreadForwardImpl() {
   }

   qint64 pos() {
      return m_pos;
   }

   const char *readPointer(qint64 maximumLength, qint64 &len) {
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
      return 0;
   }

   bool advanceReadPointer(qint64 a) {
      if (m_data == 0) {
         return false;
      }

      m_amount -= a;
      m_data += a;
      m_pos += a;

      // To main thread to inform about our state. The m_pos will be sent as a sanity check.
      emit processedData(m_pos, a);

      return true;
   }

   bool atEnd() {
      if (m_amount > 0) {
         return false;
      } else {
         return m_atEnd;
      }
   }

   bool reset() {
      m_amount = 0;
      m_data = 0;
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

   qint64 size() {
      return m_size;
   }

 public :
   // From user thread
   NET_CS_SLOT_1(Public, void haveDataSlot(qint64 pos, const QByteArray &dataArray, bool dataAtEnd, qint64 dataSize))
   NET_CS_SLOT_2(haveDataSlot)

   // to main thread
   NET_CS_SIGNAL_1(Public, void wantData(qint64 un_named_arg1))
   NET_CS_SIGNAL_2(wantData, un_named_arg1)

   NET_CS_SIGNAL_1(Public, void processedData(qint64 pos, qint64 amount))
   NET_CS_SIGNAL_2(processedData, pos, amount)

   NET_CS_SIGNAL_1(Public, void resetData(bool *b))
   NET_CS_SIGNAL_2(resetData, b)
};

QT_END_NAMESPACE

#endif // QT_NO_HTTP

#endif // QHTTPTHREADDELEGATE_H
