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

#ifndef QNETWORK_REPLYHTTPIMPL_P_H
#define QNETWORK_REPLYHTTPIMPL_P_H

#include <qnetwork_reply.h>

#include <qnetwork_request.h>
#include <qpointer.h>
#include <qdatetime.h>
#include <qsharedpointer.h>
#include <qatomic.h>
#include <qnetwork_cachemetadata.h>
#include <qnetworkproxy.h>
#include <qnetworksession.h>

#include <qhttp_networkrequest_p.h>
#include <qbytedata_p.h>
#include <qnetwork_reply_p.h>

#ifdef QT_SSL
#include <qsslconfiguration.h>
#endif

class QIODevice;
class QNetworkReplyHttpImplPrivate;

class QNetworkReplyHttpImpl: public QNetworkReply
{
    NET_CS_OBJECT(QNetworkReplyHttpImpl)

public:
    QNetworkReplyHttpImpl(QNetworkAccessManager * const, const QNetworkRequest &,
            QNetworkAccessManager::Operation &, QIODevice *outgoingData);

    virtual ~QNetworkReplyHttpImpl();

    void close() override;
    void abort() override;
    qint64 bytesAvailable() const override;
    bool isSequential () const override;
    qint64 size() const override;
    qint64 readData(char *, qint64) override;
    void setReadBufferSize(qint64 size) override;
    bool canReadLine () const override;

    NET_CS_SLOT_1(Private, void _q_startOperation())
    NET_CS_SLOT_2(_q_startOperation)

    NET_CS_SLOT_1(Private, bool start(const QNetworkRequest &newHttpRequest))
    NET_CS_SLOT_2(start)

    NET_CS_SLOT_1(Private, void _q_cacheLoadReadyRead())
    NET_CS_SLOT_2(_q_cacheLoadReadyRead)

    NET_CS_SLOT_1(Private, void _q_bufferOutgoingData())
    NET_CS_SLOT_2(_q_bufferOutgoingData)

    NET_CS_SLOT_1(Private, void _q_bufferOutgoingDataFinished())
    NET_CS_SLOT_2(_q_bufferOutgoingDataFinished)

#ifndef QT_NO_BEARERMANAGEMENT
    NET_CS_SLOT_1(Private, void _q_networkSessionConnected())
    NET_CS_SLOT_2(_q_networkSessionConnected)

    NET_CS_SLOT_1(Private, void _q_networkSessionFailed())
    NET_CS_SLOT_2(_q_networkSessionFailed)

    NET_CS_SLOT_1(Private, void _q_networkSessionStateChanged(QNetworkSession::State sessionState))
    NET_CS_SLOT_2(_q_networkSessionStateChanged)

    NET_CS_SLOT_1(Private, void _q_networkSessionUsagePoliciesChanged(QNetworkSession::UsagePolicies newPolicies))
    NET_CS_SLOT_2(_q_networkSessionUsagePoliciesChanged)
#endif

    NET_CS_SLOT_1(Private, void _q_finished())
    NET_CS_SLOT_2(_q_finished)

    NET_CS_SLOT_1(Private, void _q_error(QNetworkReply::NetworkError errorCode, const QString &errorMsg))
    NET_CS_SLOT_2(_q_error)

    // From reply
    NET_CS_SLOT_1(Private, void replyDownloadData(const QByteArray &data))
    NET_CS_SLOT_2(replyDownloadData)

    NET_CS_SLOT_1(Private, void replyFinished())
    NET_CS_SLOT_2(replyFinished)

    NET_CS_SLOT_1(Private, void replyDownloadMetaData(const QList <QPair <QByteArray,QByteArray>> &headers, int statusCode,
          const QString &reason, bool isPipelined, QSharedPointer <char> downloadBuffer, qint64 contentLength, bool isSpdy))
    NET_CS_SLOT_2(replyDownloadMetaData)

    NET_CS_SLOT_1(Private, void replyDownloadProgressSlot(qint64 bytesReceived, qint64 bytesTotal))
    NET_CS_SLOT_2(replyDownloadProgressSlot)

    NET_CS_SLOT_1(Private, void httpAuthenticationRequired(const QHttpNetworkRequest &request, QAuthenticator *auth))
    NET_CS_SLOT_2(httpAuthenticationRequired)

    NET_CS_SLOT_1(Private, void httpError(QNetworkReply::NetworkError errorCode, const QString &errorMsg))
    NET_CS_SLOT_2(httpError)

#ifdef QT_SSL
    NET_CS_SLOT_1(Private, void replyEncrypted())
    NET_CS_SLOT_2(replyEncrypted)

    NET_CS_SLOT_1(Private, void replySslErrors(const QList<QSslError> &errorList, bool *ignoreAll, QList<QSslError> *toBeIgnored))
    NET_CS_SLOT_2(replySslErrors)

    NET_CS_SLOT_1(Private, void replySslConfigurationChanged(const QSslConfiguration &sslConfig))
    NET_CS_SLOT_2(replySslConfigurationChanged)

    NET_CS_SLOT_1(Private, void replyPreSharedKeyAuthenticationRequiredSlot(QSslPreSharedKeyAuthenticator *authenticator))
    NET_CS_SLOT_2(replyPreSharedKeyAuthenticationRequiredSlot)
#endif

#ifndef QT_NO_NETWORKPROXY
    NET_CS_SLOT_1(Private, void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth))
    NET_CS_SLOT_2(proxyAuthenticationRequired)
#endif

    NET_CS_SLOT_1(Private, void resetUploadDataSlot(bool *r))
    NET_CS_SLOT_2(resetUploadDataSlot)

    NET_CS_SLOT_1(Private, void wantUploadDataSlot(qint64 maxSize))
    NET_CS_SLOT_2(wantUploadDataSlot)

    NET_CS_SLOT_1(Private, void sentUploadDataSlot(qint64 pos, qint64 amount))
    NET_CS_SLOT_2(sentUploadDataSlot)

    NET_CS_SLOT_1(Private, void uploadByteDeviceReadyReadSlot())
    NET_CS_SLOT_2(uploadByteDeviceReadyReadSlot)

    NET_CS_SLOT_1(Private, void emitReplyUploadProgress(qint64 bytesSent, qint64 bytesTotal))
    NET_CS_SLOT_2(emitReplyUploadProgress)

    NET_CS_SLOT_1(Private, void _q_cacheSaveDeviceAboutToClose())
    NET_CS_SLOT_2(_q_cacheSaveDeviceAboutToClose)

    NET_CS_SLOT_1(Private, void _q_metaDataChanged())
    NET_CS_SLOT_2(_q_metaDataChanged)

    NET_CS_SLOT_1(Private, void onRedirected(const QUrl &redirectUrl, int httpStatus, int maxRedirectsRemaining))
    NET_CS_SLOT_2(onRedirected)

    // To HTTP thread:
    NET_CS_SIGNAL_1(Public, void startHttpRequest())
    NET_CS_SIGNAL_2(startHttpRequest)

    NET_CS_SIGNAL_1(Public, void abortHttpRequest())
    NET_CS_SIGNAL_2(abortHttpRequest)

    NET_CS_SIGNAL_1(Public, void readBufferSizeChanged(qint64 size))
    NET_CS_SIGNAL_2(readBufferSizeChanged,size)

    NET_CS_SIGNAL_1(Public, void readBufferFreed(qint64 size))
    NET_CS_SIGNAL_2(readBufferFreed,size)

    NET_CS_SIGNAL_1(Public, void startHttpRequestSynchronously())
    NET_CS_SIGNAL_2(startHttpRequestSynchronously)

    NET_CS_SIGNAL_1(Public, void haveUploadData(qint64 pos, const QByteArray &dataArray, bool dataAtEnd, qint64 dataSize))
    NET_CS_SIGNAL_2(haveUploadData,pos,dataArray,dataAtEnd,dataSize)

    Q_DECLARE_PRIVATE(QNetworkReplyHttpImpl)

#ifdef QT_SSL

protected:
    void ignoreSslErrors() override;
    void ignoreSslErrorsImplementation(const QList<QSslError> &errors) override;
    void setSslConfigurationImplementation(const QSslConfiguration &configuration) override;
    void sslConfigurationImplementation(QSslConfiguration &configuration) const override;
#endif

};

class QNetworkReplyHttpImplPrivate: public QNetworkReplyPrivate
{
public:
    static QHttpNetworkRequest::Priority convert(const QNetworkRequest::Priority &priority);

    QNetworkReplyHttpImplPrivate();
    ~QNetworkReplyHttpImplPrivate();

    bool start(const QNetworkRequest &newHttpRequest);
    void _q_startOperation();

    void _q_cacheLoadReadyRead();

    void _q_bufferOutgoingData();
    void _q_bufferOutgoingDataFinished();

    void _q_cacheSaveDeviceAboutToClose();

#ifndef QT_NO_BEARERMANAGEMENT
    void _q_networkSessionConnected();
    void _q_networkSessionFailed();
    void _q_networkSessionStateChanged(QNetworkSession::State sessionState);
    void _q_networkSessionUsagePoliciesChanged(QNetworkSession::UsagePolicies);
#endif

    void _q_finished();

    void finished();
    void error(QNetworkReply::NetworkError code, const QString &errorString);
    void _q_error(QNetworkReply::NetworkError errorCode, const QString &errorMsg);
    void _q_metaDataChanged();

    void checkForRedirect(const int statusCode);

    // incoming from user
    QNetworkAccessManager *manager;
    QNetworkAccessManagerPrivate *managerPrivate;
    QHttpNetworkRequest httpRequest;    // also a copy in the HTTP thread
    bool synchronous;

    ReplyState state;

    // from http thread
    int statusCode;
    QString reasonPhrase;

    // upload
    QNonContiguousByteDevice *createUploadByteDevice();
    QSharedPointer<QNonContiguousByteDevice> uploadByteDevice;
    qint64 uploadByteDevicePosition;
    bool uploadDeviceChoking;          // if we could not readPointer() any data at the moment
    QIODevice *outgoingData;
    QSharedPointer<QRingBuffer> outgoingDataBuffer;

    void emitReplyUploadProgress(qint64 bytesSent, qint64 bytesTotal);    // dup?
    void onRedirected(const QUrl &redirectUrl, int httpStatus, int maxRedirectsRemaining);

    qint64 bytesUploaded;

    // cache
    void createCache();
    void completeCacheSave();
    void setCachingEnabled(bool enable);
    bool isCachingEnabled() const;
    bool isCachingAllowed() const;
    void initCacheSaveDevice();
    QIODevice *cacheLoadDevice;
    bool loadingFromCache;

    QIODevice *cacheSaveDevice;
    bool cacheEnabled; // is this for saving?

    QUrl urlForLastAuthentication;

#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy lastProxyAuthentication;
#endif

    bool migrateBackend();
    bool canResume() const;
    void setResumeOffset(quint64 offset);
    quint64 resumeOffset;
    qint64 preMigrationDownloaded;

    // Used for normal downloading. For "zero copy" the downloadZerocopyBuffer is used
    QByteDataBuffer downloadMultiBuffer;
    QByteDataBuffer pendingDownloadData; // For signal compression
    qint64 bytesDownloaded;

    // only used when the "zero copy" style is used. Else downloadMultiBuffer is used.
    // Please note that the whole "zero copy" download buffer API is private right now. Do not use it.
    qint64 downloadBufferReadPosition;
    qint64 downloadBufferCurrentSize;
    QSharedPointer<char> downloadBufferPointer;
    char *downloadZerocopyBuffer;

    // Will be increased by HTTP thread:
    QSharedPointer<QAtomicInt> pendingDownloadDataEmissions;
    QSharedPointer<QAtomicInt> pendingDownloadProgressEmissions;

#ifdef QT_SSL
    QSslConfiguration sslConfiguration;
    bool pendingIgnoreAllSslErrors;
    QList<QSslError> pendingIgnoreSslErrorsList;
#endif

    bool loadFromCacheIfAllowed(QHttpNetworkRequest &httpRequest);
    void invalidateCache();
    bool sendCacheContents(const QNetworkCacheMetaData &metaData);
    QNetworkCacheMetaData fetchCacheMetaData(const QNetworkCacheMetaData &metaData) const;

    void postRequest(const QNetworkRequest &newHttpRequest);
    QNetworkAccessManager::Operation getRedirectOperation(QNetworkAccessManager::Operation currentOp, int httpStatus);
    QNetworkRequest createRedirectRequest(const QNetworkRequest &originalRequests, const QUrl &url, int maxRedirectsRemaining);
    bool isHttpRedirectResponse() const;

    // From HTTP thread:
    void replyDownloadData(const QByteArray &data);
    void replyFinished();
    void replyDownloadMetaData(QList<QPair<QByteArray,QByteArray> >, int, QString, bool, QSharedPointer<char>, qint64, bool);
    void replyDownloadProgressSlot(qint64 bytesReceived, qint64 bytesTotal);
    void httpAuthenticationRequired(const QHttpNetworkRequest &request, QAuthenticator *auth);
    void httpError(QNetworkReply::NetworkError errorCode, const QString &errorMsg);

#ifdef QT_SSL
    void replyEncrypted();
    void replySslErrors(const QList<QSslError> &errorList, bool *ignoreAll, QList<QSslError> *toBeIgnored);
    void replySslConfigurationChanged(QSslConfiguration sslConfig);
    void replyPreSharedKeyAuthenticationRequiredSlot(QSslPreSharedKeyAuthenticator *authenticator);
#endif

#ifndef QT_NO_NETWORKPROXY
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth);
#endif

    // From QNonContiguousByteDeviceThreadForwardImpl in HTTP thread:
    void resetUploadDataSlot(bool *r);
    void wantUploadDataSlot(qint64 maxSize);
    void sentUploadDataSlot(qint64 pos, qint64 amount);

    // From user's QNonContiguousByteDevice
    void uploadByteDeviceReadyReadSlot();

    Q_DECLARE_PUBLIC(QNetworkReplyHttpImpl)
};

#endif
