/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qnetworkreplyimpl_p.h>
#include <qnetworkaccessbackend_p.h>
#include <qnetworkcookie.h>
#include <qnetworkcookiejar.h>
#include <qabstractnetworkcache.h>
#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qsslconfiguration.h>
#include <qnetworksession.h>
#include <qnetworkaccessmanager_p.h>

#include <QCoreApplication>

inline QNetworkReplyImplPrivate::QNetworkReplyImplPrivate()
   : backend(0), outgoingData(0),
     copyDevice(0),
     cacheEnabled(false), cacheSaveDevice(0),
     notificationHandlingPaused(false),
     bytesDownloaded(0), lastBytesDownloaded(-1), bytesUploaded(-1), preMigrationDownloaded(-1),
     httpStatusCode(0),
     state(ReplyState::Idle),
     downloadBufferReadPosition(0),
     downloadBufferCurrentSize(0),
     downloadBufferMaximumSize(0),
     downloadBuffer(0)
{
   if (request.attribute(QNetworkRequest::EmitAllUploadProgressSignalsAttribute).toBool() == true) {
      emitAllUploadProgressSignals = true;
   }
}

void QNetworkReplyImplPrivate::_q_startOperation()
{
   // ensure this function is only being called once
   if (state == ReplyState::Working || state == ReplyState::Finished) {
      qDebug("QNetworkReplyImpl::_q_startOperation was called more than once");
      return;
   }
   state = ReplyState::Working;

   // note: if that method is called directly, it cannot happen that the backend is 0,
   // because we just checked via a qobject_cast that we got a http backend (see
   // QNetworkReplyImplPrivate::setup())
   if (!backend) {
      error(QNetworkReplyImpl::ProtocolUnknownError,
            QCoreApplication::translate("QNetworkReply", "Protocol \"%1\" is unknown").formatArg(url.scheme())); // not really true!;
      finished();
      return;
   }

#ifndef QT_NO_BEARERMANAGEMENT
   Q_Q(QNetworkReplyImpl);
   QSharedPointer<QNetworkSession> session(manager->d_func()->getNetworkSession());
   QVariant isBackground = backend->request().attribute(QNetworkRequest::BackgroundRequestAttribute, QVariant::fromValue(false));
   if (isBackground.toBool() && session && session->usagePolicies().testFlag(QNetworkSession::NoBackgroundTrafficPolicy)) {
      error(QNetworkReply::BackgroundRequestNotAllowedError,
            QCoreApplication::translate("QNetworkReply", "Background request not allowed."));
      finished();
      return;
   }
#endif
   if (!backend->start()) {

#ifndef QT_NO_BEARERMANAGEMENT
      // backend failed to start because the session state is not Connected.
      // QNetworkAccessManager will call _q_startOperation again for us when the session
      // state changes.
      state = ReplyState::WaitingForSession;

      if (session) {

         QObject::connect(session.data(), SIGNAL(error(QNetworkSession::SessionError)),
                          q, SLOT(_q_networkSessionFailed()));

         if (!session->isOpen()) {
            session->setSessionProperty("ConnectInBackground", isBackground);
            session->open();
         }

      } else {
         qWarning("Backend is waiting for QNetworkSession to connect, but there is none!");
         state = ReplyState::Working;

         error(QNetworkReplyImpl::NetworkSessionFailedError,
               QCoreApplication::translate("QNetworkReply", "Network session error."));
         finished();
      }
#else
      qWarning("Backend start failed");
      state = ReplyState::Working;
      error(QNetworkReplyImpl::UnknownNetworkError,
            QCoreApplication::translate("QNetworkReply", "backend start error."));
      finished();
#endif
      return;

   } else {

#ifndef QT_NO_BEARERMANAGEMENT
      if (session) {
         QObject::connect(session.data(), SIGNAL(stateChanged(QNetworkSession::State)),
                          q, SLOT(_q_networkSessionStateChanged(QNetworkSession::State)), Qt::QueuedConnection);
      }
#endif
   }

#ifndef QT_NO_BEARERMANAGEMENT
   if (session) {
      QObject::connect(session.data(), SIGNAL(usagePoliciesChanged(QNetworkSession::UsagePolicies)),
                       q, SLOT(_q_networkSessionUsagePoliciesChanged(QNetworkSession::UsagePolicies)));
   }
#endif
   downloadProgressSignalChoke.start();
   uploadProgressSignalChoke.invalidate();
   if (backend && backend->isSynchronous()) {
      state = ReplyState::Finished;
      q_func()->setFinished(true);

   } else {
      if (state != ReplyState::Finished) {
         if (operation == QNetworkAccessManager::GetOperation) {
            pendingNotifications.append(NotifyDownstreamReadyWrite);
         }

         handleNotifications();
      }
   }
}

void QNetworkReplyImplPrivate::_q_copyReadyRead()
{
   Q_Q(QNetworkReplyImpl);

   if (state != ReplyState::Working) {
      return;
   }

   if (!copyDevice || !q->isOpen()) {
      return;
   }

   // FIXME Optimize to use download buffer if it is a QBuffer.
   // Needs to be done where sendCacheContents() (?) of HTTP is emitting
   // metaDataChanged ?

   forever {
      qint64 bytesToRead = nextDownstreamBlockSize();
      if (bytesToRead == 0) {
         // we'll be called again, eventually
         break;
      }

      bytesToRead = qBound(1, bytesToRead, copyDevice->bytesAvailable());

      QByteArray byteData;
      byteData.resize(bytesToRead);
      qint64 bytesActuallyRead = copyDevice->read(byteData.data(), byteData.size());

      if (bytesActuallyRead == -1)  {
         byteData.clear();
         backendNotify(NotifyCopyFinished);
         break;
      }

      byteData.resize(bytesActuallyRead);
      readBuffer.append(byteData);

      if (!copyDevice->isSequential() && copyDevice->atEnd()) {
         backendNotify(NotifyCopyFinished);
         bytesDownloaded += bytesActuallyRead;
         break;
      }

      bytesDownloaded += bytesActuallyRead;
   }

   if (bytesDownloaded == lastBytesDownloaded) {
      // we didn't read anything
      return;
   }

   lastBytesDownloaded = bytesDownloaded;
   QVariant totalSize = cookedHeaders.value(QNetworkRequest::ContentLengthHeader);

   if (preMigrationDownloaded != Q_INT64_C(-1)) {
      totalSize = totalSize.toLongLong() + preMigrationDownloaded;
   }

   pauseNotificationHandling();

   // emit readyRead before downloadProgress incase this will cause events to be
   // processed and we get into a recursive call (as in QProgressDialog).
   emit q->readyRead();

   if (downloadProgressSignalChoke.elapsed() >= progressSignalInterval) {
      downloadProgressSignalChoke.restart();

      emit q->downloadProgress(bytesDownloaded,
                               totalSize.isNull() ? Q_INT64_C(-1) : totalSize.toLongLong());
   }

   resumeNotificationHandling();
}

void QNetworkReplyImplPrivate::_q_copyReadChannelFinished()
{
   _q_copyReadyRead();
}

void QNetworkReplyImplPrivate::_q_bufferOutgoingDataFinished()
{
   Q_Q(QNetworkReplyImpl);

   // make sure this is only called once, ever.
   //_q_bufferOutgoingData may call it or the readChannelFinished emission
   if (state != ReplyState::Buffering) {
      return;
   }

   // disconnect signals
   QObject::disconnect(outgoingData, SIGNAL(readyRead()), q, SLOT(_q_bufferOutgoingData()));
   QObject::disconnect(outgoingData, SIGNAL(readChannelFinished()), q, SLOT(_q_bufferOutgoingDataFinished()));

   // finally, start the request
   QMetaObject::invokeMethod(q, "_q_startOperation", Qt::QueuedConnection);
}

void QNetworkReplyImplPrivate::_q_bufferOutgoingData()
{
   Q_Q(QNetworkReplyImpl);

   if (!outgoingDataBuffer) {
      // first call, create our buffer
      outgoingDataBuffer = QSharedPointer<QRingBuffer>::create();

      QObject::connect(outgoingData, SIGNAL(readyRead()), q, SLOT(_q_bufferOutgoingData()));
      QObject::connect(outgoingData, SIGNAL(readChannelFinished()), q, SLOT(_q_bufferOutgoingDataFinished()));
   }

   qint64 bytesBuffered = 0;
   qint64 bytesToBuffer = 0;

   // read data into our buffer
   forever {
      bytesToBuffer = outgoingData->bytesAvailable();

      // unknown? just try 2 kB, this also ensures we always try to read the EOF
      if (bytesToBuffer <= 0)    {
         bytesToBuffer = 2 * 1024;
      }

      char *dst = outgoingDataBuffer->reserve(bytesToBuffer);
      bytesBuffered = outgoingData->read(dst, bytesToBuffer);

      if (bytesBuffered == -1) {
         // EOF has been reached.
         outgoingDataBuffer->chop(bytesToBuffer);

         _q_bufferOutgoingDataFinished();
         break;

      } else if (bytesBuffered == 0) {
         // nothing read right now, just wait until we get called again
         outgoingDataBuffer->chop(bytesToBuffer);

         break;
      } else {
         // don't break, try to read() again
         outgoingDataBuffer->chop(bytesToBuffer - bytesBuffered);
      }
   }
}

#ifndef QT_NO_BEARERMANAGEMENT
void QNetworkReplyImplPrivate::_q_networkSessionConnected()
{
   Q_Q(QNetworkReplyImpl);

   if (manager.isNull()) {
      return;
   }

   QSharedPointer<QNetworkSession> session = manager->d_func()->getNetworkSession();
   if (! session) {
      return;
   }

   if (session->state() != QNetworkSession::Connected) {
      return;
   }

   switch (state) {
      case ReplyState::Buffering:
      case ReplyState::Working:
      case ReplyState::Reconnecting:
         // Migrate existing downloads to new network connection.
         migrateBackend();
         break;

      case ReplyState::WaitingForSession:
         // Start waiting requests.
         QMetaObject::invokeMethod(q, "_q_startOperation", Qt::QueuedConnection);
         break;

      default:
         ;
   }
}

void QNetworkReplyImplPrivate::_q_networkSessionStateChanged(QNetworkSession::State sessionState)
{
   if (sessionState == QNetworkSession::Disconnected
         && state != ReplyState::Idle && state != ReplyState::Reconnecting) {

      error(QNetworkReplyImpl::NetworkSessionFailedError,
            QCoreApplication::translate("QNetworkReply", "Network session error."));

      finished();
   }
}
void QNetworkReplyImplPrivate::_q_networkSessionFailed()
{
   // Abort waiting and working replies.
   if (state == ReplyState::WaitingForSession || state == ReplyState::Working) {
      state = ReplyState::Working;

      QSharedPointer<QNetworkSession> session(manager->d_func()->getNetworkSession());
      QString errorStr;

      if (session) {
         errorStr = session->errorString();
      } else {
         errorStr = QCoreApplication::translate("QNetworkReply", "Network session error.");
      }

      error(QNetworkReplyImpl::NetworkSessionFailedError, errorStr);
      finished();
   }
}
void QNetworkReplyImplPrivate::_q_networkSessionUsagePoliciesChanged(QNetworkSession::UsagePolicies newPolicies)
{
   if (backend->request().attribute(QNetworkRequest::BackgroundRequestAttribute).toBool()) {
      if (newPolicies & QNetworkSession::NoBackgroundTrafficPolicy) {

         if (state == ReplyState::WaitingForSession || state == ReplyState::Working) {
            state = ReplyState::Working;

            error(QNetworkReply::BackgroundRequestNotAllowedError,
                  QCoreApplication::translate("QNetworkReply", "Background request not allowed."));

            finished();
         }
      }
   }
}
#endif

void QNetworkReplyImplPrivate::setup(QNetworkAccessManager::Operation op, const QNetworkRequest &req,
                                     QIODevice *data)
{
   Q_Q(QNetworkReplyImpl);

   outgoingData = data;
   request = req;
   url = request.url();
   operation = op;

   q->QIODevice::open(QIODevice::ReadOnly);
   // Internal code that does a HTTP reply for the synchronous Ajax
   // in QtWebKit.
   QVariant synchronousHttpAttribute = req.attribute(
                                          static_cast<QNetworkRequest::Attribute>(QNetworkRequest::SynchronousRequestAttribute));
   // The synchronous HTTP is a corner case, we will put all upload data in one big QByteArray in the outgoingDataBuffer.
   // Yes, this is not the most efficient thing to do, but on the other hand synchronous XHR needs to die anyway.
   if (synchronousHttpAttribute.toBool() && outgoingData) {
      outgoingDataBuffer = QSharedPointer<QRingBuffer>::create();
      qint64 previousDataSize = 0;

      do {
         previousDataSize = outgoingDataBuffer->size();
         outgoingDataBuffer->append(outgoingData->readAll());
      } while (outgoingDataBuffer->size() != previousDataSize);
   }

   if (backend) {
      backend->setSynchronous(synchronousHttpAttribute.toBool());
   }


   if (outgoingData && backend && !backend->isSynchronous()) {
      // there is data to be uploaded, e.g. HTTP POST.

      if (!backend->needsResetableUploadData() || !outgoingData->isSequential()) {
         // backend does not need upload buffering or
         // fixed size non-sequential
         // just start the operation
         QMetaObject::invokeMethod(q, "_q_startOperation", Qt::QueuedConnection);
      } else {
         bool bufferingDisallowed =
            req.attribute(QNetworkRequest::DoNotBufferUploadDataAttribute,
                          false).toBool();

         if (bufferingDisallowed) {
            // if a valid content-length header for the request was supplied, we can disable buffering
            // if not, we will buffer anyway
            if (req.header(QNetworkRequest::ContentLengthHeader).isValid()) {
               QMetaObject::invokeMethod(q, "_q_startOperation", Qt::QueuedConnection);
            } else {
               state = ReplyState::Buffering;
               QMetaObject::invokeMethod(q, "_q_bufferOutgoingData", Qt::QueuedConnection);
            }

         } else {
            // _q_startOperation will be called when the buffering has finished.
            state = ReplyState::Buffering;
            QMetaObject::invokeMethod(q, "_q_bufferOutgoingData", Qt::QueuedConnection);
         }
      }
   } else {
      // for HTTP, we want to send out the request as fast as possible to the network, without
      // invoking methods in a QueuedConnection

      if (backend && backend->isSynchronous()) {
         _q_startOperation();
      } else {
         QMetaObject::invokeMethod(q, "_q_startOperation", Qt::QueuedConnection);
      }

   }
}

void QNetworkReplyImplPrivate::backendNotify(InternalNotifications notification)
{
   Q_Q(QNetworkReplyImpl);
   if (!pendingNotifications.contains(notification)) {
      pendingNotifications.enqueue(notification);
   }

   if (pendingNotifications.size() == 1) {
      QCoreApplication::postEvent(q, new QEvent(QEvent::NetworkReplyUpdated));
   }
}

void QNetworkReplyImplPrivate::handleNotifications()
{
   if (notificationHandlingPaused) {
      return;
   }

   NotificationQueue current = pendingNotifications;
   pendingNotifications.clear();

   if (state != ReplyState::Working) {
      return;
   }

   while (state == ReplyState::Working && ! current.isEmpty()) {
      InternalNotifications notification = current.dequeue();

      switch (notification) {
         case NotifyDownstreamReadyWrite:
            if (copyDevice) {
               _q_copyReadyRead();
            } else {
               backend->downstreamReadyWrite();
            }
            break;

         case NotifyCloseDownstreamChannel:
            backend->closeDownstreamChannel();
            break;

         case NotifyCopyFinished: {
            QIODevice *dev = copyDevice;
            copyDevice = 0;
            backend->copyFinished(dev);
            break;
         }
      }
   }
}

// Do not handle the notifications while we are emitting downloadProgress
// or readyRead
void QNetworkReplyImplPrivate::pauseNotificationHandling()
{
   notificationHandlingPaused = true;
}

// Resume notification handling
void QNetworkReplyImplPrivate::resumeNotificationHandling()
{
   Q_Q(QNetworkReplyImpl);
   notificationHandlingPaused = false;
   if (pendingNotifications.size() >= 1) {
      QCoreApplication::postEvent(q, new QEvent(QEvent::NetworkReplyUpdated));
   }
}

QAbstractNetworkCache *QNetworkReplyImplPrivate::networkCache() const
{
   if (!backend) {
      return 0;
   }
   return backend->networkCache();
}

void QNetworkReplyImplPrivate::createCache()
{
   // check if we can save and if we're allowed to
   if (!networkCache() || !request.attribute(QNetworkRequest::CacheSaveControlAttribute, true).toBool())  {
      return;
   }
   cacheEnabled = true;
}

bool QNetworkReplyImplPrivate::isCachingEnabled() const
{
   return (cacheEnabled && networkCache() != 0);
}

void QNetworkReplyImplPrivate::setCachingEnabled(bool enable)
{
   if (!enable && !cacheEnabled) {
      return;   // nothing to do
   }

   if (enable && cacheEnabled) {
      return;   // nothing to do either!
   }

   if (enable) {
      if (bytesDownloaded) {
         // refuse to enable in this case
         qCritical("QNetworkReplyImpl: backend error: caching was enabled after some bytes had been written");
         return;
      }

      createCache();

   } else {
      // someone told us to turn on, then back off?
      // ok... but you should make up your mind
      qDebug("QNetworkReplyImpl: setCachingEnabled(true) called after setCachingEnabled(false) -- "
             "backend %s probably needs to be fixed", csPrintable(backend->metaObject()->className())) ;

      networkCache()->remove(url);
      cacheSaveDevice = 0;
      cacheEnabled = false;

   }
}

void QNetworkReplyImplPrivate::completeCacheSave()
{
   if (cacheEnabled) {
      if (errorCode != QNetworkReplyImpl::NoError) {
         networkCache()->remove(url);

      } else if (cacheSaveDevice) {
         networkCache()->insert(cacheSaveDevice);
      }
   }

   cacheSaveDevice = 0;
   cacheEnabled = false;
}

void QNetworkReplyImplPrivate::emitUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
   Q_Q(QNetworkReplyImpl);
   bytesUploaded = bytesSent;
   if (!emitAllUploadProgressSignals) {
      if (uploadProgressSignalChoke.isValid()) {
         if (bytesSent != bytesTotal && uploadProgressSignalChoke.elapsed() < progressSignalInterval) {
            return;
         }
         uploadProgressSignalChoke.restart();
      } else {
         uploadProgressSignalChoke.start();
      }
   }
   pauseNotificationHandling();
   emit q->uploadProgress(bytesSent, bytesTotal);
   resumeNotificationHandling();
}


qint64 QNetworkReplyImplPrivate::nextDownstreamBlockSize() const
{
   enum { DesiredBufferSize = 32 * 1024 };
   if (readBufferMaxSize == 0) {
      return DesiredBufferSize;
   }

   return qMax(0, readBufferMaxSize - readBuffer.byteAmount());
}

void QNetworkReplyImplPrivate::initCacheSaveDevice()
{
   Q_Q(QNetworkReplyImpl);

   // The disk cache does not support partial content, so don't even try to
   // save any such content into the cache.
   if (q->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 206) {
      cacheEnabled = false;
      return;
   }

   // save the meta data
   QNetworkCacheMetaData metaData;
   metaData.setUrl(url);
   metaData = backend->fetchCacheMetaData(metaData);

   // save the redirect request also in the cache
   QVariant redirectionTarget = q->attribute(QNetworkRequest::RedirectionTargetAttribute);
   if (redirectionTarget.isValid()) {
      QNetworkCacheMetaData::AttributesMap attributes = metaData.attributes();
      attributes.insert(QNetworkRequest::RedirectionTargetAttribute, redirectionTarget);
      metaData.setAttributes(attributes);
   }

   cacheSaveDevice = networkCache()->prepare(metaData);

   if (! cacheSaveDevice || (cacheSaveDevice && !cacheSaveDevice->isOpen())) {
      if (cacheSaveDevice && !cacheSaveDevice->isOpen())
         qCritical("QNetworkReplyImpl::initCacheSaveDevice() Network cache returned a device that is not open -- "
                   "class %s probably needs to be fixed", csPrintable(networkCache()->metaObject()->className()));

      networkCache()->remove(url);
      cacheSaveDevice = 0;
      cacheEnabled = false;
   }
}

// we received downstream data and send this to the cache
// and to our readBuffer (which in turn gets read by the user of QNetworkReply)
void QNetworkReplyImplPrivate::appendDownstreamData(QByteDataBuffer &data)
{
   Q_Q(QNetworkReplyImpl);
   if (!q->isOpen()) {
      return;
   }

   if (cacheEnabled && !cacheSaveDevice) {
      initCacheSaveDevice();
   }

   qint64 bytesWritten = 0;
   for (int i = 0; i < data.bufferCount(); i++) {
      QByteArray const &item = data[i];

      if (cacheSaveDevice) {
         cacheSaveDevice->write(item.constData(), item.size());
      }
      readBuffer.append(item);

      bytesWritten += item.size();
   }
   data.clear();

   bytesDownloaded += bytesWritten;
   lastBytesDownloaded = bytesDownloaded;

   appendDownstreamDataSignalEmissions();
}

void QNetworkReplyImplPrivate::appendDownstreamDataSignalEmissions()
{
   Q_Q(QNetworkReplyImpl);

   QVariant totalSize = cookedHeaders.value(QNetworkRequest::ContentLengthHeader);
   if (preMigrationDownloaded != Q_INT64_C(-1)) {
      totalSize = totalSize.toLongLong() + preMigrationDownloaded;
   }

   pauseNotificationHandling();
   // important: At the point of this readyRead(), the data parameter list must be empty,
   // else implicit sharing will trigger memcpy when the user is reading data!
   emit q->readyRead();
   // emit readyRead before downloadProgress incase this will cause events to be
   // processed and we get into a recursive call (as in QProgressDialog).

   if (downloadProgressSignalChoke.elapsed() >= progressSignalInterval) {
      downloadProgressSignalChoke.restart();

      emit q->downloadProgress(bytesDownloaded,
                               totalSize.isNull() ? Q_INT64_C(-1) : totalSize.toLongLong());
   }

   resumeNotificationHandling();

   // do we still have room in the buffer?
   if (nextDownstreamBlockSize() > 0) {
      backendNotify(QNetworkReplyImplPrivate::NotifyDownstreamReadyWrite);
   }
}

// this is used when it was fetched from the cache, right?
void QNetworkReplyImplPrivate::appendDownstreamData(QIODevice *data)
{
   Q_Q(QNetworkReplyImpl);

   if (!q->isOpen()) {
      return;
   }

   // read until EOF from data
   if (copyDevice) {
      qCritical("QNetworkReplyImpl: copy from QIODevice already in progress -- "
                "backend probly needs to be fixed");
      return;
   }

   copyDevice = data;
   q->connect(copyDevice, SIGNAL(readyRead()), q, SLOT(_q_copyReadyRead()));
   q->connect(copyDevice, SIGNAL(readChannelFinished()), q, SLOT(_q_copyReadChannelFinished()));

   // start the copy:
   _q_copyReadyRead();
}

void QNetworkReplyImplPrivate::appendDownstreamData(const QByteArray &data)
{
   Q_UNUSED(data)
   // TODO implement

   // TODO call

   qFatal("QNetworkReplyImplPrivate::appendDownstreamData not implemented");
}

static void downloadBufferDeleter(char *ptr)
{
   delete[] ptr;
}

char *QNetworkReplyImplPrivate::getDownloadBuffer(qint64 size)
{
   Q_Q(QNetworkReplyImpl);

   if (!downloadBuffer) {
      // We are requested to create it
      // Check attribute() if allocating a buffer of that size can be allowed
      QVariant bufferAllocationPolicy = request.attribute(QNetworkRequest::MaximumDownloadBufferSizeAttribute);

      if (bufferAllocationPolicy.isValid() && bufferAllocationPolicy.toLongLong() >= size) {
         downloadBufferCurrentSize = 0;
         downloadBufferMaximumSize = size;
         downloadBuffer = new char[downloadBufferMaximumSize]; // throws if allocation fails
         downloadBufferPointer = QSharedPointer<char>(downloadBuffer, downloadBufferDeleter);

         q->setAttribute(QNetworkRequest::DownloadBufferAttribute,
                         QVariant::fromValue<QSharedPointer<char> >(downloadBufferPointer));
      }
   }

   return downloadBuffer;
}

void QNetworkReplyImplPrivate::setDownloadBuffer(QSharedPointer<char> sp, qint64 size)
{
   Q_Q(QNetworkReplyImpl);

   downloadBufferPointer = sp;
   downloadBuffer = downloadBufferPointer.data();
   downloadBufferCurrentSize = 0;
   downloadBufferMaximumSize = size;

   q->setAttribute(QNetworkRequest::DownloadBufferAttribute,
                   QVariant::fromValue<QSharedPointer<char> > (downloadBufferPointer));
}


void QNetworkReplyImplPrivate::appendDownstreamDataDownloadBuffer(qint64 bytesReceived, qint64 bytesTotal)
{
   Q_Q(QNetworkReplyImpl);
   if (! q->isOpen()) {
      return;
   }

   if (cacheEnabled && ! cacheSaveDevice) {
      initCacheSaveDevice();
   }

   if (cacheSaveDevice && bytesReceived == bytesTotal) {
      //        if (lastBytesDownloaded == -1)
      //            lastBytesDownloaded = 0;
      //        cacheSaveDevice->write(downloadBuffer + lastBytesDownloaded, bytesReceived - lastBytesDownloaded);

      // Write everything in one go if we use a download buffer. might be more performant.
      cacheSaveDevice->write(downloadBuffer, bytesTotal);
   }

   bytesDownloaded = bytesReceived;
   lastBytesDownloaded = bytesReceived;

   downloadBufferCurrentSize = bytesReceived;

   // Only emit readyRead when actual data is there
   // emit readyRead before downloadProgress incase this will cause events to be
   // processed and we get into a recursive call (as in QProgressDialog).
   if (bytesDownloaded > 0) {
      emit q->readyRead();
   }

   if (downloadProgressSignalChoke.elapsed() >= progressSignalInterval) {
      downloadProgressSignalChoke.restart();
      emit q->downloadProgress(bytesDownloaded, bytesTotal);
   }
}

void QNetworkReplyImplPrivate::finished()
{
   Q_Q(QNetworkReplyImpl);

   if (state == ReplyState::Finished || state == ReplyState::Aborted || state == ReplyState::WaitingForSession) {
      return;
   }

   pauseNotificationHandling();
   QVariant totalSize = cookedHeaders.value(QNetworkRequest::ContentLengthHeader);
   if (preMigrationDownloaded != Q_INT64_C(-1)) {
      totalSize = totalSize.toLongLong() + preMigrationDownloaded;
   }

   if (!manager.isNull()) {

#ifndef QT_NO_BEARERMANAGEMENT
      QSharedPointer<QNetworkSession> session (manager->d_func()->getNetworkSession());

      if (session && session->state() == QNetworkSession::Roaming &&
            state == ReplyState::Working && errorCode != QNetworkReply::OperationCanceledError) {

         // only content with a known size will fail with a temporary network failure error
         if (!totalSize.isNull()) {

            if (bytesDownloaded != totalSize) {

               if (migrateBackend()) {
                  // either we are migrating or the request is finished/aborted
                  if (state == ReplyState::Reconnecting || state == ReplyState::WaitingForSession) {
                     resumeNotificationHandling();
                     return; // exit early if we are migrating.
                  }
               } else {
                  error(QNetworkReply::TemporaryNetworkFailureError,
                        QNetworkReply::tr("Temporary network failure."));
               }
            }
         }
      }
#endif
   }
   resumeNotificationHandling();

   state = ReplyState::Finished;
   q->setFinished(true);

   pendingNotifications.clear();

   pauseNotificationHandling();
   if (totalSize.isNull() || totalSize == -1) {
      emit q->downloadProgress(bytesDownloaded, bytesDownloaded);
   } else {
      emit q->downloadProgress(bytesDownloaded, totalSize.toLongLong());
   }

   if (bytesUploaded == -1 && (outgoingData || outgoingDataBuffer)) {
      emit q->uploadProgress(0, 0);
   }
   resumeNotificationHandling();

   // if we don't know the total size of or we received everything save the cache
   if (totalSize.isNull() || totalSize == -1 || bytesDownloaded == totalSize) {
      completeCacheSave();
   }

   // note: might not be a good idea, since users could decide to delete us
   // which would delete the backend too...
   // maybe we should protect the backend
   pauseNotificationHandling();
   emit q->readChannelFinished();
   emit q->finished();
   resumeNotificationHandling();
}

void QNetworkReplyImplPrivate::error(QNetworkReplyImpl::NetworkError code, const QString &errorMessage)
{
   Q_Q(QNetworkReplyImpl);
   // Can't set and emit multiple errors.
   if (errorCode != QNetworkReply::NoError) {
      qWarning("QNetworkReplyImplPrivate::error: Internal problem, this method must only be called once.");
      return;
   }

   errorCode = code;
   q->setErrorString(errorMessage);

   // note: might not be a good idea, since users could decide to delete us
   // which would delete the backend too...
   // maybe we should protect the backend
   emit q->error(code);
}

void QNetworkReplyImplPrivate::metaDataChanged()
{
   Q_Q(QNetworkReplyImpl);
   // 1. do we have cookies?
   // 2. are we allowed to set them?

   if (cookedHeaders.contains(QNetworkRequest::SetCookieHeader) && !manager.isNull()
         && (static_cast<QNetworkRequest::LoadControl>
             (request.attribute(QNetworkRequest::CookieSaveControlAttribute,
                                QNetworkRequest::Automatic).toInt()) == QNetworkRequest::Automatic)) {
      QList<QNetworkCookie> cookies =
         qvariant_cast<QList<QNetworkCookie> >(cookedHeaders.value(QNetworkRequest::SetCookieHeader));
      QNetworkCookieJar *jar = manager->cookieJar();
      if (jar) {
         jar->setCookiesFromUrl(cookies, url);
      }
   }
   emit q->metaDataChanged();
}

void QNetworkReplyImplPrivate::redirectionRequested(const QUrl &target)
{
   attributes.insert(QNetworkRequest::RedirectionTargetAttribute, target);
}

void QNetworkReplyImplPrivate::encrypted()
{
#ifdef QT_SSL
   Q_Q(QNetworkReplyImpl);
   emit q->encrypted();
#endif
}
void QNetworkReplyImplPrivate::sslErrors(const QList<QSslError> &errors)
{
#ifdef QT_SSL
   Q_Q(QNetworkReplyImpl);
   emit q->sslErrors(errors);
#else
   Q_UNUSED(errors);
#endif
}

QNetworkReplyImpl::QNetworkReplyImpl(QObject *parent)
   : QNetworkReply(*new QNetworkReplyImplPrivate, parent)
{
}

QNetworkReplyImpl::~QNetworkReplyImpl()
{
   Q_D(QNetworkReplyImpl);

   // This code removes the data from the cache if it was prematurely aborted.
   // See QNetworkReplyImplPrivate::completeCacheSave(), we disable caching there after the cache
   // save had been properly finished. So if it is still enabled it means we got deleted/aborted.
   if (d->isCachingEnabled()) {
      d->networkCache()->remove(url());
   }
}

void QNetworkReplyImpl::abort()
{
   Q_D(QNetworkReplyImpl);

   if (d->state == QNetworkReplyImplPrivate::ReplyState::Finished || d->state == QNetworkReplyImplPrivate::ReplyState::Aborted) {
      return;
   }

   // stop both upload and download
   if (d->outgoingData) {
      disconnect(d->outgoingData, 0, this, 0);
   }

   if (d->copyDevice) {
      disconnect(d->copyDevice, 0, this, 0);
   }

   QNetworkReply::close();

   // call finished which will emit signals
   d->error(OperationCanceledError, tr("Operation canceled"));

   if (d->state == QNetworkReplyImplPrivate::ReplyState::WaitingForSession) {
      d->state = QNetworkReplyImplPrivate::ReplyState::Working;
   }
   d->finished();


   d->state = QNetworkReplyImplPrivate::ReplyState::Aborted;

   // finished may access the backend
   if (d->backend) {
      d->backend->deleteLater();
      d->backend = 0;
   }
}

void QNetworkReplyImpl::close()
{
   Q_D(QNetworkReplyImpl);
   if (d->state == QNetworkReplyImplPrivate::ReplyState::Aborted ||
         d->state == QNetworkReplyImplPrivate::ReplyState::Finished) {
      return;
   }

   // stop the download
   if (d->backend) {
      d->backend->closeDownstreamChannel();
   }

   if (d->copyDevice) {
      disconnect(d->copyDevice, 0, this, 0);
   }

   QNetworkReply::close();

   // call finished which will emit signals
   d->error(OperationCanceledError, tr("Operation canceled"));
   d->finished();
}

bool QNetworkReplyImpl::canReadLine () const
{
   Q_D(const QNetworkReplyImpl);
   return QNetworkReply::canReadLine() || d->readBuffer.canReadLine();
}


/*!
    Returns the number of bytes available for reading with
    QIODevice::read(). The number of bytes available may grow until
    the finished() signal is emitted.
*/
qint64 QNetworkReplyImpl::bytesAvailable() const
{
   // Special case for the "zero copy" download buffer
   Q_D(const QNetworkReplyImpl);
   if (d->downloadBuffer) {
      qint64 maxAvail = d->downloadBufferCurrentSize - d->downloadBufferReadPosition;
      return QNetworkReply::bytesAvailable() + maxAvail;
   }

   return QNetworkReply::bytesAvailable() + d_func()->readBuffer.byteAmount();
}

void QNetworkReplyImpl::setReadBufferSize(qint64 size)
{
   Q_D(QNetworkReplyImpl);

   if (size > d->readBufferMaxSize && size > d->readBuffer.byteAmount()) {
      d->backendNotify(QNetworkReplyImplPrivate::NotifyDownstreamReadyWrite);
   }

   QNetworkReply::setReadBufferSize(size);

   if (d->backend) {
      d->backend->setDownstreamLimited(d->readBufferMaxSize > 0);
   }
}

#ifdef QT_SSL
void QNetworkReplyImpl::sslConfigurationImplementation(QSslConfiguration &configuration) const
{
   Q_D(const QNetworkReplyImpl);

   if (d->backend) {
      d->backend->fetchSslConfiguration(configuration);
   }
}

void QNetworkReplyImpl::setSslConfigurationImplementation(const QSslConfiguration &config)
{
   Q_D(QNetworkReplyImpl);

   if (d->backend && ! config.isNull()) {
      d->backend->setSslConfiguration(config);
   }
}

void QNetworkReplyImpl::ignoreSslErrors()
{
   Q_D(QNetworkReplyImpl);

   if (d->backend) {
      d->backend->ignoreSslErrors();
   }
}

void QNetworkReplyImpl::ignoreSslErrorsImplementation(const QList<QSslError> &errors)
{
   Q_D(QNetworkReplyImpl);

   if (d->backend) {
      d->backend->ignoreSslErrors(errors);
   }
}
#endif

/*!
    \internal
*/
qint64 QNetworkReplyImpl::readData(char *data, qint64 maxlen)
{
   Q_D(QNetworkReplyImpl);

   // Special case code if we have the "zero copy" download buffer
   if (d->downloadBuffer) {
      qint64 maxAvail = qMin(d->downloadBufferCurrentSize - d->downloadBufferReadPosition, maxlen);

      if (maxAvail == 0) {
         return d->state == QNetworkReplyImplPrivate::ReplyState::Finished ? -1 : 0;
      }

      // FIXME what about "Aborted" state?
      memcpy(data, d->downloadBuffer + d->downloadBufferReadPosition, maxAvail);
      d->downloadBufferReadPosition += maxAvail;
      return maxAvail;
   }

   if (d->readBuffer.isEmpty()) {
      return d->state == QNetworkReplyImplPrivate::ReplyState::Finished ? -1 : 0;
   }
   // FIXME what about "Aborted" state?

   d->backendNotify(QNetworkReplyImplPrivate::NotifyDownstreamReadyWrite);
   if (maxlen == 1) {
      // optimization for getChar()
      *data = d->readBuffer.getChar();
      return 1;
   }

   maxlen = qMin(maxlen, d->readBuffer.byteAmount());
   return d->readBuffer.read(data, maxlen);
}

/*!
   \internal Reimplemented for internal purposes
*/
bool QNetworkReplyImpl::event(QEvent *e)
{
   if (e->type() == QEvent::NetworkReplyUpdated) {
      d_func()->handleNotifications();
      return true;
   }

   return QObject::event(e);
}

/*
    Migrates the backend of the QNetworkReply to a new network connection if required.  Returns
    true if the reply is migrated or it is not required; otherwise returns false.
*/
bool QNetworkReplyImplPrivate::migrateBackend()
{
   Q_Q(QNetworkReplyImpl);

   // Network reply is already finished or aborted, don't need to migrate.
   if (state == ReplyState::Finished || state == ReplyState::Aborted) {
      return true;
   }

   // Request has outgoing data, not migrating.
   if (outgoingData) {
      return false;
   }

   // Request is serviced from the cache, don't need to migrate.
   if (copyDevice) {
      return true;
   }

   // Backend does not support resuming download.
   if (backend && !backend->canResume()) {
      return false;
   }

   state = ReplyState::Reconnecting;
   cookedHeaders.clear();
   rawHeaders.clear();

   preMigrationDownloaded = bytesDownloaded;

   delete backend;
   backend = manager->d_func()->findBackend(operation, request);

   if (backend) {
      backend->setParent(q);
      backend->reply = this;
      backend->setResumeOffset(bytesDownloaded);
   }

   QMetaObject::invokeMethod(q, "_q_startOperation", Qt::QueuedConnection);

   return true;
}

#ifndef QT_NO_BEARERMANAGEMENT
QDisabledNetworkReply::QDisabledNetworkReply(QObject *parent,
      const QNetworkRequest &req, QNetworkAccessManager::Operation op)
   :   QNetworkReply(parent)
{
   setRequest(req);
   setUrl(req.url());
   setOperation(op);

   QString msg = QCoreApplication::translate("QNetworkAccessManager", "Network access is disabled.");

   setError(UnknownNetworkError, msg);

   QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection, Q_ARG(QNetworkReply::NetworkError, UnknownNetworkError));
   QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
}

QDisabledNetworkReply::~QDisabledNetworkReply()
{
}
#endif


void QNetworkReplyImpl::_q_startOperation()
{
   Q_D(QNetworkReplyImpl);
   d->_q_startOperation();
}

void QNetworkReplyImpl::_q_copyReadyRead()
{
   Q_D(QNetworkReplyImpl);
   d->_q_copyReadyRead();
}

void QNetworkReplyImpl::_q_copyReadChannelFinished()
{
   Q_D(QNetworkReplyImpl);
   d->_q_copyReadChannelFinished();
}

void QNetworkReplyImpl::_q_bufferOutgoingData()
{
   Q_D(QNetworkReplyImpl);
   d->_q_bufferOutgoingData();
}

void QNetworkReplyImpl::_q_bufferOutgoingDataFinished()
{
   Q_D(QNetworkReplyImpl);
   d->_q_bufferOutgoingDataFinished();
}

#ifndef QT_NO_BEARERMANAGEMENT
void QNetworkReplyImpl::_q_networkSessionConnected()
{
   Q_D(QNetworkReplyImpl);
   d->_q_networkSessionConnected();
}

void QNetworkReplyImpl::_q_networkSessionFailed()
{
   Q_D(QNetworkReplyImpl);
   d->_q_networkSessionFailed();
}

void QNetworkReplyImpl::_q_networkSessionStateChanged(QNetworkSession::State data)
{
   Q_D(QNetworkReplyImpl);
   d->_q_networkSessionStateChanged(data);
}

void QNetworkReplyImpl::_q_networkSessionUsagePoliciesChanged(QNetworkSession::UsagePolicies data)
{
   Q_D(QNetworkReplyImpl);
   d->_q_networkSessionUsagePoliciesChanged(data);
}
#endif


