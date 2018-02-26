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

#ifndef QNETWORKREPLYHTTPIMPL_P_H
#define QNETWORKREPLYHTTPIMPL_P_H

#include "qnetworkrequest.h"
#include "qnetworkreply.h"

#include "qpointer.h"
#include "qdatetime.h"
#include "qsharedpointer.h"
#include "qatomic.h"

#include <QNetworkCacheMetaData>
#include <qhttpnetworkrequest_p.h>
#include <qbytedata_p.h>
#include <qnetworkreply_p.h>
#include <QNetworkProxy>
#include <QNetworkSession>

#ifdef QT_SSL
#include <QSslConfiguration>
#endif

class QIODevice;
class QNetworkReplyHttpImplPrivate;

class QNetworkReplyHttpImpl: public QNetworkReply
{
    NET_CS_OBJECT(QNetworkReplyHttpImpl)

public:
    QNetworkReplyHttpImpl(QNetworkAccessManager* const, const QNetworkRequest&, QNetworkAccessManager::Operation&,
                  QIODevice* outgoingData);

    virtual ~QNetworkReplyHttpImpl();

    void close() override;
    void abort() override;
    qint64 bytesAvailable() const override;
    bool isSequential () const override;
    qint64 size() const override;
    qint64 readData(char*, qint64) override;
    void setReadBufferSize(qint64 size) override;
    bool canReadLine () const override;

    NET_CS_SLOT_1(Private, void _q_startOperation())
    NET_CS_SLOT_2(_q_startOperation)

    NET_CS_SLOT_1(Private, bool start(const QNetworkRequest & un_named_arg1))
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

    NET_CS_SLOT_1(Private, void _q_networkSessionStateChanged(QNetworkSession::State un_named_arg1))
    NET_CS_SLOT_2(_q_networkSessionStateChanged)

    NET_CS_SLOT_1(Private, void _q_networkSessionUsagePoliciesChanged(QNetworkSession::UsagePolicies un_named_arg1))
    NET_CS_SLOT_2(_q_networkSessionUsagePoliciesChanged)
#endif

    NET_CS_SLOT_1(Private, void _q_finished())
    NET_CS_SLOT_2(_q_finished)

    NET_CS_SLOT_1(Private, void _q_error(QNetworkReply::NetworkError un_named_arg1,const QString & un_named_arg2))
    NET_CS_SLOT_2(_q_error)

    // From reply
    NET_CS_SLOT_1(Private, void replyDownloadData(QByteArray un_named_arg1))
    NET_CS_SLOT_2(replyDownloadData)

    NET_CS_SLOT_1(Private, void replyFinished())
    NET_CS_SLOT_2(replyFinished)

    NET_CS_SLOT_1(Private, void replyDownloadMetaData(QList <QPair <QByteArray,QByteArray>> un_named_arg1,int un_named_arg2,
                  QString un_named_arg3,bool un_named_arg4,QSharedPointer <char> un_named_arg5,qint64 un_named_arg6,bool un_named_arg7))
    NET_CS_SLOT_2(replyDownloadMetaData)

    NET_CS_SLOT_1(Private, void replyDownloadProgressSlot(qint64 un_named_arg1,qint64 un_named_arg2))
    NET_CS_SLOT_2(replyDownloadProgressSlot)

    NET_CS_SLOT_1(Private, void httpAuthenticationRequired(QHttpNetworkRequest un_named_arg1,QAuthenticator * un_named_arg2))
    NET_CS_SLOT_2(httpAuthenticationRequired)

    NET_CS_SLOT_1(Private, void httpError(QNetworkReply::NetworkError un_named_arg1, QString un_named_arg2))
    NET_CS_SLOT_2(httpError)

#ifdef QT_SSL
    NET_CS_SLOT_1(Private, void replyEncrypted())
    NET_CS_SLOT_2(replyEncrypted)

    NET_CS_SLOT_1(Private, void replySslErrors(const QList <QSslError> & un_named_arg1, bool * un_named_arg2,
                  QList <QSslError> * un_named_arg3))
    NET_CS_SLOT_2(replySslErrors)

    NET_CS_SLOT_1(Private, void replySslConfigurationChanged(QSslConfiguration un_named_arg1))
    NET_CS_SLOT_2(replySslConfigurationChanged)

    NET_CS_SLOT_1(Private, void replyPreSharedKeyAuthenticationRequiredSlot(QSslPreSharedKeyAuthenticator * un_named_arg1))
    NET_CS_SLOT_2(replyPreSharedKeyAuthenticationRequiredSlot)
#endif

#ifndef QT_NO_NETWORKPROXY
    NET_CS_SLOT_1(Private, void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator * auth))
    NET_CS_SLOT_2(proxyAuthenticationRequired)
#endif

    NET_CS_SLOT_1(Private, void resetUploadDataSlot(bool *r))
    NET_CS_SLOT_2(resetUploadDataSlot)

    NET_CS_SLOT_1(Private, void wantUploadDataSlot(qint64 un_named_arg1))
    NET_CS_SLOT_2(wantUploadDataSlot)

    NET_CS_SLOT_1(Private, void sentUploadDataSlot(qint64 un_named_arg1, qint64 un_named_arg2))
    NET_CS_SLOT_2(sentUploadDataSlot)

    NET_CS_SLOT_1(Private, void uploadByteDeviceReadyReadSlot())
    NET_CS_SLOT_2(uploadByteDeviceReadyReadSlot)

    NET_CS_SLOT_1(Private, void emitReplyUploadProgress(qint64 un_named_arg1, qint64 un_named_arg2))
    NET_CS_SLOT_2(emitReplyUploadProgress)

    NET_CS_SLOT_1(Private, void _q_cacheSaveDeviceAboutToClose())
    NET_CS_SLOT_2(_q_cacheSaveDeviceAboutToClose)

    NET_CS_SLOT_1(Private, void _q_metaDataChanged())
    NET_CS_SLOT_2(_q_metaDataChanged)

    NET_CS_SLOT_1(Private, void onRedirected(QUrl un_named_arg1, int un_named_arg2, int un_named_arg3))
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

    NET_CS_SIGNAL_1(Public, void haveUploadData(qint64 pos, QByteArray dataArray, bool dataAtEnd, qint64 dataSize))
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
    static QHttpNetworkRequest::Priority convert(const QNetworkRequest::Priority& prio);

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
    void _q_networkSessionStateChanged(QNetworkSession::State);
    void _q_networkSessionUsagePoliciesChanged(QNetworkSession::UsagePolicies);
#endif

    void _q_finished();

    void finished();
    void error(QNetworkReply::NetworkError code, const QString &errorString);
    void _q_error(QNetworkReply::NetworkError code, const QString &errorString);
    void _q_metaDataChanged();

    void checkForRedirect(const int statusCode);

    // incoming from user
    QNetworkAccessManager *manager;
    QNetworkAccessManagerPrivate *managerPrivate;
    QHttpNetworkRequest httpRequest; // There is also a copy in the HTTP thread
    bool synchronous;

    ReplyState state;

    // from http thread
    int statusCode;
    QString reasonPhrase;

    // upload
    QNonContiguousByteDevice* createUploadByteDevice();
    QSharedPointer<QNonContiguousByteDevice> uploadByteDevice;
    qint64 uploadByteDevicePosition;
    bool uploadDeviceChoking; // if we couldn't readPointer() any data at the moment
    QIODevice *outgoingData;
    QSharedPointer<QRingBuffer> outgoingDataBuffer;

    void emitReplyUploadProgress(qint64 bytesSent, qint64 bytesTotal); // dup?
    void onRedirected(QUrl redirectUrl, int httpStatus, int maxRedirectsRemainig);

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
    char* downloadZerocopyBuffer;

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

    void postRequest(const QNetworkRequest& newHttpRequest);
    QNetworkAccessManager::Operation getRedirectOperation(QNetworkAccessManager::Operation currentOp, int httpStatus);
    QNetworkRequest createRedirectRequest(const QNetworkRequest &originalRequests, const QUrl &url, int maxRedirectsRemainig);
    bool isHttpRedirectResponse() const;

    // From HTTP thread:
    void replyDownloadData(QByteArray);
    void replyFinished();
    void replyDownloadMetaData(QList<QPair<QByteArray,QByteArray> >, int, QString, bool, QSharedPointer<char>, qint64, bool);
    void replyDownloadProgressSlot(qint64,qint64);
    void httpAuthenticationRequired(const QHttpNetworkRequest &request, QAuthenticator *auth);
    void httpError(QNetworkReply::NetworkError error, QString errorString);

#ifdef QT_SSL
    void replyEncrypted();
    void replySslErrors(const QList<QSslError> &, bool *, QList<QSslError> *);
    void replySslConfigurationChanged(QSslConfiguration);
    void replyPreSharedKeyAuthenticationRequiredSlot(QSslPreSharedKeyAuthenticator *);
#endif

#ifndef QT_NO_NETWORKPROXY
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth);
#endif

    // From QNonContiguousByteDeviceThreadForwardImpl in HTTP thread:
    void resetUploadDataSlot(bool *r);
    void wantUploadDataSlot(qint64);
    void sentUploadDataSlot(qint64, qint64);

    // From user's QNonContiguousByteDevice
    void uploadByteDeviceReadyReadSlot();

    Q_DECLARE_PUBLIC(QNetworkReplyHttpImpl)
};

#endif
