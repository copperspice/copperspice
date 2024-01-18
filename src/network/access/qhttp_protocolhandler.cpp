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

#include <qhttp_protocolhandler_p.h>

#include <qnoncontiguousbytedevice_p.h>
#include <qhttp_networkconnectionchannel_p.h>

QHttpProtocolHandler::QHttpProtocolHandler(QHttpNetworkConnectionChannel *channel)
   : QAbstractProtocolHandler(channel)
{
}

void QHttpProtocolHandler::_q_receiveReply()
{
   Q_ASSERT(m_socket);

   if (! m_reply) {
      if (m_socket->bytesAvailable() > 0)
         qWarning("QHttpProtocolHandler::_q_receiveReply() Called without QHttpNetworkReply, %lld bytes "
               " are pending", m_socket->bytesAvailable());

      m_channel->close();

      return;
   }

   // only run when the QHttpNetworkConnection is not currently being destructed, e.g.
   // this function is called from _q_disconnected which is called because
   // of ~QHttpNetworkConnectionPrivate
   if (! dynamic_cast<QHttpNetworkConnection *>(m_connection)) {
      return;
   }

   QAbstractSocket::SocketState socketState = m_socket->state();

   // connection might be closed to signal the end of data
   if (socketState == QAbstractSocket::UnconnectedState) {
      if (m_socket->bytesAvailable() <= 0) {
         if (m_reply->d_func()->state == QHttpNetworkReplyPrivate::ReadingDataState) {
            // finish this reply. this case happens when the server did not send a content length
            m_reply->d_func()->state = QHttpNetworkReplyPrivate::AllDoneState;
            m_channel->allDone();
            return;

         } else {
            m_channel->handleUnexpectedEOF();
            return;
         }

      } else {
         // socket not connected but still bytes for reading.. just continue in this function
      }
   }

   // read loop for the response
   qint64 bytes = 0;
   qint64 lastBytes = bytes;

   do {
      lastBytes = bytes;

      QHttpNetworkReplyPrivate::ReplyState state = m_reply->d_func()->state;

      switch (state) {
         case QHttpNetworkReplyPrivate::NothingDoneState:
            m_reply->d_func()->state = QHttpNetworkReplyPrivate::ReadingStatusState;

            [[fallthrough]];

         case QHttpNetworkReplyPrivate::ReadingStatusState: {
            qint64 statusBytes = m_reply->d_func()->readStatus(m_socket);

            if (statusBytes == -1) {
               // connection broke while reading status. also handled if later _q_disconnected is called
               m_channel->handleUnexpectedEOF();
               return;
            }

            bytes += statusBytes;
            m_channel->lastStatus = m_reply->d_func()->statusCode;
            break;
         }

         case QHttpNetworkReplyPrivate::ReadingHeaderState: {
            QHttpNetworkReplyPrivate *replyPrivate = m_reply->d_func();
            qint64 headerBytes = replyPrivate->readHeader(m_socket);

            if (headerBytes == -1) {
               // connection broke while reading headers. also handled if later _q_disconnected is called
               m_channel->handleUnexpectedEOF();
               return;
            }

            bytes += headerBytes;

            // If headers were parsed successfully now it is the ReadingDataState
            if (replyPrivate->state == QHttpNetworkReplyPrivate::ReadingDataState) {
               if (replyPrivate->isCompressed() && replyPrivate->autoDecompress) {
                  // remove the Content-Length from header
                  replyPrivate->removeAutoDecompressHeader();
               } else {
                  replyPrivate->autoDecompress = false;
               }

               if (replyPrivate->statusCode == 100) {
                  replyPrivate->clearHttpLayerInformation();
                  replyPrivate->state = QHttpNetworkReplyPrivate::ReadingStatusState;
                  break; // ignore
               }

               if (replyPrivate->shouldEmitSignals()) {
                  emit m_reply->headerChanged();
               }

               // After headerChanged had been emitted
               // we can suddenly have a replyPrivate->userProvidedDownloadBuffer
               // this is handled in the ReadingDataState however

               if (!replyPrivate->expectContent()) {
                  replyPrivate->state = QHttpNetworkReplyPrivate::AllDoneState;
                  m_channel->allDone();
                  break;
               }
            }
            break;
         }

         case QHttpNetworkReplyPrivate::ReadingDataState: {
            QHttpNetworkReplyPrivate *replyPrivate = m_reply->d_func();

            if (m_socket->state() == QAbstractSocket::ConnectedState &&
                  replyPrivate->downstreamLimited && !replyPrivate->responseData.isEmpty() && replyPrivate->shouldEmitSignals()) {

               // only do the following when connected, not after a disconnect when there is still data

               // have some HTTP body data, do not read more from the socket until fetched by
               // QHttpNetworkAccessHttpBackend. if we read more, we could not limit read buffer usage

               // only do this when shouldEmitSignals == true because HTTP parsing needs the 401/407 replies
               // these replies do not observe the read buffer maximum size, but they should be small

               return;
            }

            if (replyPrivate->userProvidedDownloadBuffer) {
               // user provided a direct buffer where all the data should go

               // only works when we tell the user the content length so they can allocate
               // a buffer with that size, this call will read only from the buffered data

               qint64 haveRead = replyPrivate->readBodyVeryFast(m_socket,
                     replyPrivate->userProvidedDownloadBuffer + replyPrivate->totalProgress);

               if (haveRead > 0) {
                  bytes += haveRead;
                  replyPrivate->totalProgress += haveRead;
                  // the user will get notified of it via progress signal
                  emit m_reply->dataReadProgress(replyPrivate->totalProgress, replyPrivate->bodyLength);

               } else if (haveRead == 0) {
                  // happens since this called in a loop. Currently no bytes available.

               } else if (haveRead < 0) {
                  m_connection->d_func()->emitReplyError(m_socket, m_reply, QNetworkReply::RemoteHostClosedError);
                  break;
               }

            } else if (!replyPrivate->isChunked() && !replyPrivate->autoDecompress
                  && replyPrivate->bodyLength > 0) {

               // bulk files like images should fulfill these properties and
               // we can therefore save on memory copying

               qint64 haveRead = replyPrivate->readBodyFast(m_socket, &replyPrivate->responseData);
               bytes += haveRead;
               replyPrivate->totalProgress += haveRead;

               if (replyPrivate->shouldEmitSignals()) {
                  emit m_reply->readyRead();
                  emit m_reply->dataReadProgress(replyPrivate->totalProgress, replyPrivate->bodyLength);
               }

            } else {
               // use the traditional slower reading (for compressed encoding, chunked encoding,
               // no content-length etc)
               qint64 haveRead = replyPrivate->readBody(m_socket, &replyPrivate->responseData);

               if (haveRead > 0) {
                  bytes += haveRead;
                  replyPrivate->totalProgress += haveRead;

                  if (replyPrivate->shouldEmitSignals()) {
                     emit m_reply->readyRead();
                     emit m_reply->dataReadProgress(replyPrivate->totalProgress, replyPrivate->bodyLength);
                  }

               } else if (haveRead == -1) {
                  // Some error occurred
                  m_connection->d_func()->emitReplyError(m_socket, m_reply, QNetworkReply::ProtocolFailure);
                  break;
               }
            }

            // still in ReadingDataState? This function will be called again by the socket's readyRead
            if (replyPrivate->state == QHttpNetworkReplyPrivate::ReadingDataState) {
               break;
            }

            // everything done
         }
         [[fallthrough]];

         case QHttpNetworkReplyPrivate::AllDoneState:
            m_channel->allDone();
            break;

         default:
            break;
      }

   } while (bytes != lastBytes && m_reply);
}

void QHttpProtocolHandler::_q_readyRead()
{
   if (m_socket->state() == QAbstractSocket::ConnectedState && m_socket->bytesAvailable() == 0) {
      // received a readyRead but no bytes are available, happens for the Unbuffered QTcpSocket
      // Also check if socket is in ConnectedState since this method cam also be invoked via the event loop

      char c;
      qint64  ret = m_socket->peek(&c, 1);

      if (ret < 0) {
         m_channel->_q_error(m_socket->error());

         // still need to handle the reply so it emits its signals etc.
         if (m_reply) {
            _q_receiveReply();
         }

         return;
      }
   }

   if (m_channel->isSocketWaiting() || m_channel->isSocketReading()) {
      if (m_socket->bytesAvailable()) {
         // might get a spurious call from readMoreLater()
         // call of the QHttpNetworkConnection even while the socket is disconnecting.
         // Therefore check if there is actually bytes available before changing the channel state.
         m_channel->state = QHttpNetworkConnectionChannel::ReadingState;
      }

      if (m_reply) {
         _q_receiveReply();
      }
   }
}

bool QHttpProtocolHandler::sendRequest()
{
   m_reply = m_channel->reply;

   if (! m_reply) {
      // how should that happen
      qWarning("QHttpProtocolHandler::sendRequest() No QHttpNetworkReply available");
      return false;
   }

   switch (m_channel->state) {
      case QHttpNetworkConnectionChannel::IdleState: {
         // write the header

         if (!m_channel->ensureConnection()) {
            // wait for the connection (and encryption) to be done
            // sendRequest will be called again from either
            // _q_connected or _q_encrypted
            return false;
         }

         QString scheme = m_channel->request.url().scheme();

         if (scheme == "preconnect-http" || scheme == "preconnect-https") {
            m_channel->state = QHttpNetworkConnectionChannel::IdleState;
            m_reply->d_func()->state = QHttpNetworkReplyPrivate::AllDoneState;
            m_channel->allDone();
            m_connection->preConnectFinished(); // will only decrease the counter
            m_reply = nullptr;      // so we can reuse this channel
            return true;            // we have a working connection and are done
         }

         m_channel->written = 0;    // excluding the header
         m_channel->bytesTotal = 0;

         QHttpNetworkReplyPrivate *replyPrivate = m_reply->d_func();
         replyPrivate->clear();
         replyPrivate->connection = m_connection;
         replyPrivate->connectionChannel = m_channel;
         replyPrivate->autoDecompress = m_channel->request.d->autoDecompress;
         replyPrivate->pipeliningUsed = false;

         // if the url contains authentication parameters, use the new ones
         // both channels will use the new authentication parameters
         if (! m_channel->request.url().userInfo().isEmpty() && m_channel->request.withCredentials()) {
            QUrl url = m_channel->request.url();
            QAuthenticator &auth = m_channel->authenticator;

            if (url.userName() != auth.user()
                  || (!url.password().isEmpty() && url.password() != auth.password())) {
               auth.setUser(url.userName());
               auth.setPassword(url.password());
               m_connection->d_func()->copyCredentials(m_connection->d_func()->indexOf(m_socket), &auth, false);
            }

            // clear the userinfo,  since we use the same request for resending
            // userinfo in url can conflict with the one in the authenticator
            url.setUserInfo(QString());
            m_channel->request.setUrl(url);
         }

         // Will only be false if Qt WebKit is performing a cross-origin XMLHttpRequest
         // and withCredentials has not been set to true.
         if (m_channel->request.withCredentials()) {
            m_connection->d_func()->createAuthorization(m_socket, m_channel->request);
         }

#ifndef QT_NO_NETWORKPROXY
         QByteArray header = QHttpNetworkRequestPrivate::header(m_channel->request,
               (m_connection->d_func()->networkProxy.type() != QNetworkProxy::NoProxy));
#else
         QByteArray header = QHttpNetworkRequestPrivate::header(m_channel->request, false);
#endif

         m_socket->write(header);
         // flushing is dangerous (QSslSocket calls transmit which might read or error)
         // m_socket->flush();
         QNonContiguousByteDevice *uploadByteDevice = m_channel->request.uploadByteDevice();

         if (uploadByteDevice) {
            // connect the signals so this function gets called again
            QObject::connect(uploadByteDevice, &QNonContiguousByteDevice::readyRead,
                  m_channel, &QHttpNetworkConnectionChannel::_q_uploadDataReadyRead);

            m_channel->bytesTotal = m_channel->request.contentLength();

            m_channel->state = QHttpNetworkConnectionChannel::WritingState; // start writing data
            sendRequest(); //recurse

         } else {
            m_channel->state = QHttpNetworkConnectionChannel::WaitingState; // now wait for response
            sendRequest(); //recurse
         }

         break;
      }

      case QHttpNetworkConnectionChannel::WritingState: {
         // write the data
         QNonContiguousByteDevice *uploadByteDevice = m_channel->request.uploadByteDevice();

         if (!uploadByteDevice || m_channel->bytesTotal == m_channel->written) {
            if (uploadByteDevice) {
               emit m_reply->dataSendProgress(m_channel->written, m_channel->bytesTotal);
            }

            m_channel->state = QHttpNetworkConnectionChannel::WaitingState; // now wait for response
            sendRequest(); // recurse
            break;
         }

         // only feed the QTcpSocket buffer when there is less than 32 kB in it
         const qint64 socketBufferFill   = 32 * 1024;
         const qint64 socketWriteMaxSize = 16 * 1024;

#ifdef QT_SSL
         QSslSocket *sslSocket = dynamic_cast<QSslSocket *>(m_socket);
         // if it is really an ssl socket, check more than just bytesToWrite()
         while ((m_socket->bytesToWrite() + (sslSocket ? sslSocket->encryptedBytesToWrite() : 0))
                <= socketBufferFill && m_channel->bytesTotal != m_channel->written)
#else
         while (m_socket->bytesToWrite() <= socketBufferFill && m_channel->bytesTotal != m_channel->written)
#endif
         {
            // get pointer to upload data
            qint64 currentReadSize = 0;
            qint64 desiredReadSize = qMin(socketWriteMaxSize, m_channel->bytesTotal - m_channel->written);
            const char *readPointer = uploadByteDevice->readPointer(desiredReadSize, currentReadSize);

            if (currentReadSize == -1) {
               // premature eof happened
               m_connection->d_func()->emitReplyError(m_socket, m_reply, QNetworkReply::UnknownNetworkError);
               return false;

            } else if (readPointer == nullptr || currentReadSize == 0) {
               // nothing to read currently, break the loop
               break;

            } else {
               if (m_channel->written != uploadByteDevice->pos()) {
                  // useful in tracking down an upload corruption

                  qWarning("QHttpProtocolHandler::sendRequest() Internal error in sendRequest, expected to write at position "
                     "%lld, however device is at %lld", m_channel->written, uploadByteDevice->pos());

                  Q_ASSERT(m_channel->written == uploadByteDevice->pos());
                  m_connection->d_func()->emitReplyError(m_socket, m_reply, QNetworkReply::ProtocolFailure);

                  return false;
               }

               qint64 currentWriteSize = m_socket->write(readPointer, currentReadSize);
               if (currentWriteSize == -1 || currentWriteSize != currentReadSize) {
                  // socket broke down
                  m_connection->d_func()->emitReplyError(m_socket, m_reply, QNetworkReply::UnknownNetworkError);
                  return false;

               } else {
                  m_channel->written += currentWriteSize;
                  uploadByteDevice->advanceReadPointer(currentWriteSize);

                  emit m_reply->dataSendProgress(m_channel->written, m_channel->bytesTotal);

                  if (m_channel->written == m_channel->bytesTotal) {
                     // make sure this function is called once again
                     m_channel->state = QHttpNetworkConnectionChannel::WaitingState;
                     sendRequest();
                     break;
                  }
               }
            }
         }

         break;
      }

      case QHttpNetworkConnectionChannel::WaitingState: {
         QNonContiguousByteDevice *uploadByteDevice = m_channel->request.uploadByteDevice();

         if (uploadByteDevice) {
            QObject::disconnect(uploadByteDevice, &QNonContiguousByteDevice::readyRead,
               m_channel, &QHttpNetworkConnectionChannel::_q_uploadDataReadyRead);
         }

         // HTTP pipelining
         // m_connection->d_func()->fillPipeline(m_socket);
         // m_socket->flush();

         // ensure we try to receive a reply in all cases, even if _q_readyRead_ hat not been called
         // this is needed if the sends an reply before we have finished sending the request. In that
         // case receiveReply had been called before but ignored the server reply
         if (m_socket->bytesAvailable()) {
            QMetaObject::invokeMethod(m_channel, "_q_receiveReply", Qt::QueuedConnection);
         }

         break;
      }

      case QHttpNetworkConnectionChannel::ReadingState:
         // ignore _q_bytesWritten in these states
         [[fallthrough]];

      default:
         break;
   }

   return true;
}
