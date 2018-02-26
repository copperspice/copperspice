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

#include <qssl_p.h>
#include <qsslsocket.h>
#include <qsslcipher.h>

#ifdef QT_OPENSSL
#include <qsslsocket_openssl_p.h>
#endif

#ifdef QT_SECURETRANSPORT
#include "qsslsocket_mac_p.h"
#endif

#include <qsslconfiguration_p.h>

#include <qdebug.h>
#include <qdir.h>
#include <qmutex.h>
#include <qelapsedtimer.h>
#include <qhostaddress.h>
#include <qhostinfo.h>

class QSslSocketGlobalData
{
 public:
   QSslSocketGlobalData() : config(new QSslConfigurationPrivate)
   {   }

   QMutex mutex;
   QList<QSslCipher> supportedCiphers;
   QVector<QSslEllipticCurve> supportedEllipticCurves;
   QExplicitlySharedDataPointer<QSslConfigurationPrivate> config;
};
Q_GLOBAL_STATIC(QSslSocketGlobalData, globalData)

QSslSocket::QSslSocket(QObject *parent)
   : QTcpSocket(*new QSslSocketBackendPrivate, parent)
{
   Q_D(QSslSocket);

#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::QSslSocket(" << parent << "), this =" << (void *)this;
#endif

   d->q_ptr = this;
   d->init();
}

QSslSocket::~QSslSocket()
{
   Q_D(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::~QSslSocket(), this =" << (void *)this;
#endif
   delete d->plainSocket;
   d->plainSocket = 0;
}

void QSslSocket::resume()
{
   // continuing might emit signals, rather do this through the event loop
   QMetaObject::invokeMethod(this, "_q_resumeImplementation", Qt::QueuedConnection);
}

void QSslSocket::connectToHostEncrypted(const QString &hostName, quint16 port, OpenMode mode, NetworkLayerProtocol protocol)
{
   Q_D(QSslSocket);

   if (d->state == ConnectedState || d->state == ConnectingState) {
      qWarning("QSslSocket::connectToHostEncrypted() called when already connecting/connected");
      return;
   }

   d->init();
   d->autoStartHandshake = true;
   d->initialized = true;

   // Note: When connecting to localhost, some platforms (e.g., HP-UX and some BSDs)
   // establish the connection immediately (i.e., first attempt).
   connectToHost(hostName, port, mode, protocol);
}

void QSslSocket::connectToHostEncrypted(const QString &hostName, quint16 port,
                                        const QString &sslPeerName, OpenMode mode, NetworkLayerProtocol protocol)
{
   Q_D(QSslSocket);
   if (d->state == ConnectedState || d->state == ConnectingState) {
      qWarning("QSslSocket::connectToHostEncrypted() called when already connecting/connected");
      return;
   }

   d->init();
   d->autoStartHandshake = true;
   d->initialized = true;
   d->verificationPeerName = sslPeerName;

   // Note: When connecting to localhost, some platforms (e.g., HP-UX and some BSDs)
   // establish the connection immediately (i.e., first attempt).
   connectToHost(hostName, port, mode, protocol);
}



bool QSslSocket::setSocketDescriptor(qintptr socketDescriptor, SocketState state, OpenMode openMode)
{
   Q_D(QSslSocket);

#ifdef QSSLSOCKET_DEBUG
   qDebug()     << "QSslSocket::setSocketDescriptor(" << socketDescriptor << ','
                << state << ',' << openMode << ')';
#endif
   if (!d->plainSocket) {
      d->createPlainSocket(openMode);
   }

   bool retVal = d->plainSocket->setSocketDescriptor(socketDescriptor, state, openMode);
   d->cachedSocketDescriptor = d->plainSocket->socketDescriptor();
   setSocketError(d->plainSocket->error());
   setSocketState(state);
   setOpenMode(openMode);
   setLocalPort(d->plainSocket->localPort());
   setLocalAddress(d->plainSocket->localAddress());
   setPeerPort(d->plainSocket->peerPort());
   setPeerAddress(d->plainSocket->peerAddress());
   setPeerName(d->plainSocket->peerName());
   return retVal;
}

void QSslSocket::setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value)
{
   Q_D(QSslSocket);
   if (d->plainSocket) {
      d->plainSocket->setSocketOption(option, value);
   }
}

QVariant QSslSocket::socketOption(QAbstractSocket::SocketOption option)
{
   Q_D(QSslSocket);
   if (d->plainSocket) {
      return d->plainSocket->socketOption(option);
   } else {
      return QVariant();
   }
}

QSslSocket::SslMode QSslSocket::mode() const
{
   Q_D(const QSslSocket);
   return d->mode;
}

bool QSslSocket::isEncrypted() const
{
   Q_D(const QSslSocket);
   return d->connectionEncrypted;
}

QSsl::SslProtocol QSslSocket::protocol() const
{
   Q_D(const QSslSocket);
   return d->configuration.protocol;
}

void QSslSocket::setProtocol(QSsl::SslProtocol protocol)
{
   Q_D(QSslSocket);
   d->configuration.protocol = protocol;
}

QSslSocket::PeerVerifyMode QSslSocket::peerVerifyMode() const
{
   Q_D(const QSslSocket);
   return d->configuration.peerVerifyMode;
}

void QSslSocket::setPeerVerifyMode(QSslSocket::PeerVerifyMode mode)
{
   Q_D(QSslSocket);
   d->configuration.peerVerifyMode = mode;
}

int QSslSocket::peerVerifyDepth() const
{
   Q_D(const QSslSocket);
   return d->configuration.peerVerifyDepth;
}

void QSslSocket::setPeerVerifyDepth(int depth)
{
   Q_D(QSslSocket);
   if (depth < 0) {
      qWarning("QSslSocket::setPeerVerifyDepth: cannot set negative depth of %d", depth);
      return;
   }
   d->configuration.peerVerifyDepth = depth;
}

QString QSslSocket::peerVerifyName() const
{
   Q_D(const QSslSocket);
   return d->verificationPeerName;
}

void QSslSocket::setPeerVerifyName(const QString &hostName)
{
   Q_D(QSslSocket);
   d->verificationPeerName = hostName;
}

qint64 QSslSocket::bytesAvailable() const
{
   Q_D(const QSslSocket);
   if (d->mode == UnencryptedMode) {
      return QIODevice::bytesAvailable() + (d->plainSocket ? d->plainSocket->bytesAvailable() : 0);
   }
   return QIODevice::bytesAvailable();
}

qint64 QSslSocket::bytesToWrite() const
{
   Q_D(const QSslSocket);
   if (d->mode == UnencryptedMode) {
      return d->plainSocket ? d->plainSocket->bytesToWrite() : 0;
   }
   return d->writeBuffer.size();
}

qint64 QSslSocket::encryptedBytesAvailable() const
{
   Q_D(const QSslSocket);
   if (d->mode == UnencryptedMode) {
      return 0;
   }
   return d->plainSocket->bytesAvailable();
}

qint64 QSslSocket::encryptedBytesToWrite() const
{
   Q_D(const QSslSocket);
   if (d->mode == UnencryptedMode) {
      return 0;
   }
   return d->plainSocket->bytesToWrite();
}

bool QSslSocket::canReadLine() const
{
   Q_D(const QSslSocket);
   if (d->mode == UnencryptedMode) {
      return QIODevice::canReadLine() || (d->plainSocket && d->plainSocket->canReadLine());
   }

   return QIODevice::canReadLine();
}

void QSslSocket::close()
{
#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::close()";
#endif

   Q_D(QSslSocket);
   if (encryptedBytesToWrite() || ! d->writeBuffer.isEmpty()) {
      flush();
   }

   if (d->plainSocket) {
      d->plainSocket->close();
   }

   QTcpSocket::close();

   // must be cleared, reading/writing not possible on closed socket:
   d->buffer.clear();
   d->writeBuffer.clear();
}

bool QSslSocket::atEnd() const
{
   Q_D(const QSslSocket);
   if (d->mode == UnencryptedMode) {
      return QIODevice::atEnd() && (!d->plainSocket || d->plainSocket->atEnd());
   }

   return QIODevice::atEnd();
}

// Note! docs copied from QAbstractSocket::flush()
bool QSslSocket::flush()
{
   Q_D(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::flush()";
#endif
   if (d->mode != UnencryptedMode)
      // encrypt any unencrypted bytes in our buffer
   {
      d->transmit();
   }

   return d->plainSocket ? d->plainSocket->flush() : false;
}

void QSslSocket::setReadBufferSize(qint64 size)
{
   Q_D(QSslSocket);
   d->readBufferMaxSize = size;

   if (d->plainSocket) {
      d->plainSocket->setReadBufferSize(size);
   }
}

void QSslSocket::abort()
{
   Q_D(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::abort()";
#endif
   if (d->plainSocket) {
      d->plainSocket->abort();
   }
   close();
}

QSslConfiguration QSslSocket::sslConfiguration() const
{
   Q_D(const QSslSocket);

   // create a deep copy of our configuration
   QSslConfigurationPrivate *copy = new QSslConfigurationPrivate(d->configuration);
   copy->ref.store(0);              // the QSslConfiguration constructor refs up
   copy->sessionCipher = d->sessionCipher();
   copy->sessionProtocol = d->sessionProtocol();

   return QSslConfiguration(copy);
}

void QSslSocket::setSslConfiguration(const QSslConfiguration &configuration)
{
   Q_D(QSslSocket);
   d->configuration.localCertificateChain = configuration.localCertificateChain();
   d->configuration.privateKey = configuration.privateKey();
   d->configuration.ciphers = configuration.ciphers();
   d->configuration.ellipticCurves = configuration.ellipticCurves();
   d->configuration.caCertificates = configuration.caCertificates();
   d->configuration.peerVerifyDepth = configuration.peerVerifyDepth();
   d->configuration.peerVerifyMode = configuration.peerVerifyMode();
   d->configuration.protocol = configuration.protocol();
   d->configuration.sslOptions = configuration.d->sslOptions;
   d->configuration.sslSession = configuration.sessionTicket();
   d->configuration.sslSessionTicketLifeTimeHint = configuration.sessionTicketLifeTimeHint();
   d->configuration.nextAllowedProtocols = configuration.allowedNextProtocols();
   d->configuration.nextNegotiatedProtocol = configuration.nextNegotiatedProtocol();
   d->configuration.nextProtocolNegotiationStatus = configuration.nextProtocolNegotiationStatus();

   // if the CA certificates were set explicitly (either via
   // QSslConfiguration::setCaCertificates() or QSslSocket::setCaCertificates(),
   // we cannot load the certificates on demand
   if (!configuration.d->allowRootCertOnDemandLoading) {
      d->allowRootCertOnDemandLoading = false;
   }
}

void QSslSocket::setLocalCertificateChain(const QList<QSslCertificate> &localChain)
{
   Q_D(QSslSocket);
   d->configuration.localCertificateChain = localChain;
}

QList<QSslCertificate> QSslSocket::localCertificateChain() const
{
   Q_D(const QSslSocket);
   return d->configuration.localCertificateChain;
}

void QSslSocket::setLocalCertificate(const QSslCertificate &certificate)
{
   Q_D(QSslSocket);
   d->configuration.localCertificateChain = QList<QSslCertificate>();
   d->configuration.localCertificateChain += certificate;
}

void QSslSocket::setLocalCertificate(const QString &path, QSsl::EncodingFormat format)
{
   QFile file(path);

   if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      setLocalCertificate(QSslCertificate(file.readAll(), format));
   }
}

QSslCertificate QSslSocket::localCertificate() const
{
   Q_D(const QSslSocket);

   if (d->configuration.localCertificateChain.isEmpty()) {
      return QSslCertificate();
   }

   return d->configuration.localCertificateChain[0];
}

QSslCertificate QSslSocket::peerCertificate() const
{
   Q_D(const QSslSocket);
   return d->configuration.peerCertificate;
}

QList<QSslCertificate> QSslSocket::peerCertificateChain() const
{
   Q_D(const QSslSocket);
   return d->configuration.peerCertificateChain;
}

QSslCipher QSslSocket::sessionCipher() const
{
   Q_D(const QSslSocket);
   return d->sessionCipher();
}

QSsl::SslProtocol QSslSocket::sessionProtocol() const
{
   Q_D(const QSslSocket);
   return d->sessionProtocol();
}

void QSslSocket::setPrivateKey(const QSslKey &key)
{
   Q_D(QSslSocket);
   d->configuration.privateKey = key;
}

void QSslSocket::setPrivateKey(const QString &fileName, QSsl::KeyAlgorithm algorithm,
                               QSsl::EncodingFormat format, const QByteArray &passPhrase)
{
   Q_D(QSslSocket);
   QFile file(fileName);

   if (file.open(QIODevice::ReadOnly)) {
      d->configuration.privateKey = QSslKey(file.readAll(), algorithm, format, QSsl::PrivateKey, passPhrase);
   }
}

QSslKey QSslSocket::privateKey() const
{
   Q_D(const QSslSocket);
   return d->configuration.privateKey;
}

bool QSslSocket::addCaCertificates(const QString &path, QSsl::EncodingFormat format,
                                   QRegExp::PatternSyntax syntax)
{
   Q_D(QSslSocket);
   QList<QSslCertificate> certs = QSslCertificate::fromPath(path, format, syntax);

   if (certs.isEmpty()) {
      return false;
   }

   d->configuration.caCertificates += certs;
   return true;
}

void QSslSocket::addCaCertificate(const QSslCertificate &certificate)
{
   Q_D(QSslSocket);
   d->configuration.caCertificates += certificate;
}

void QSslSocket::addCaCertificates(const QList<QSslCertificate> &certificates)
{
   Q_D(QSslSocket);
   d->configuration.caCertificates += certificates;
}

bool QSslSocket::addDefaultCaCertificates(const QString &path, QSsl::EncodingFormat encoding,
      QRegExp::PatternSyntax syntax)
{
   return QSslSocketPrivate::addDefaultCaCertificates(path, encoding, syntax);
}

void QSslSocket::addDefaultCaCertificate(const QSslCertificate &certificate)
{
   QSslSocketPrivate::addDefaultCaCertificate(certificate);
}

void QSslSocket::addDefaultCaCertificates(const QList<QSslCertificate> &certificates)
{
   QSslSocketPrivate::addDefaultCaCertificates(certificates);
}

bool QSslSocket::waitForConnected(int msecs)
{
   Q_D(QSslSocket);

   if (!d->plainSocket) {
      return false;
   }

   bool retVal = d->plainSocket->waitForConnected(msecs);

   if (!retVal) {
      setSocketState(d->plainSocket->state());
      setSocketError(d->plainSocket->error());
      setErrorString(d->plainSocket->errorString());
   }

   return retVal;
}

bool QSslSocket::waitForEncrypted(int msecs)
{
   Q_D(QSslSocket);
   if (!d->plainSocket || d->connectionEncrypted) {
      return false;
   }

   if (d->mode == UnencryptedMode && !d->autoStartHandshake) {
      return false;
   }

   QElapsedTimer stopWatch;
   stopWatch.start();

   if (d->plainSocket->state() != QAbstractSocket::ConnectedState) {
      // Wait until we've entered connected state.
      if (!d->plainSocket->waitForConnected(msecs)) {
         return false;
      }
   }

   while (!d->connectionEncrypted) {
      // Start the handshake, if this hasn't been started yet.
      if (d->mode == UnencryptedMode) {
         startClientEncryption();
      }

      // Loop, waiting until the connection has been encrypted or an error
      // occurs.
      if (!d->plainSocket->waitForReadyRead(qt_subtract_from_timeout(msecs, stopWatch.elapsed()))) {
         return false;
      }
   }

   return d->connectionEncrypted;
}

bool QSslSocket::waitForReadyRead(int msecs)
{
   Q_D(QSslSocket);
   if (!d->plainSocket) {
      return false;
   }

   if (d->mode == UnencryptedMode && !d->autoStartHandshake) {
      return d->plainSocket->waitForReadyRead(msecs);
   }

   // This function must return true if and only if readyRead() *was* emitted.
   // So we initialize "readyReadEmitted" to false and check if it was set to true.
   // waitForReadyRead() could be called recursively, so we can't use the same variable
   // (the inner waitForReadyRead() may fail, but the outer one still succeeded)
   bool readyReadEmitted = false;
   bool *previousReadyReadEmittedPointer = d->readyReadEmittedPointer;
   d->readyReadEmittedPointer = &readyReadEmitted;

   QElapsedTimer stopWatch;
   stopWatch.start();

   if (!d->connectionEncrypted) {
      // Wait until we've entered encrypted mode, or until a failure occurs.
      if (!waitForEncrypted(msecs)) {
         d->readyReadEmittedPointer = previousReadyReadEmittedPointer;
         return false;
      }
   }

   if (!d->writeBuffer.isEmpty()) {
      // empty our cleartext write buffer first
      d->transmit();
   }

   // test readyReadEmitted first because either operation above
   // (waitForEncrypted or transmit) may have set it
   while (!readyReadEmitted &&
          d->plainSocket->waitForReadyRead(qt_subtract_from_timeout(msecs, stopWatch.elapsed()))) {
   }

   d->readyReadEmittedPointer = previousReadyReadEmittedPointer;
   return readyReadEmitted;
}

bool QSslSocket::waitForBytesWritten(int msecs)
{
   Q_D(QSslSocket);
   if (!d->plainSocket) {
      return false;
   }

   if (d->mode == UnencryptedMode) {
      return d->plainSocket->waitForBytesWritten(msecs);
   }

   QElapsedTimer stopWatch;
   stopWatch.start();

   if (!d->connectionEncrypted) {
      // Wait until we've entered encrypted mode, or until a failure occurs.
      if (!waitForEncrypted(msecs)) {
         return false;
      }
   }
   if (!d->writeBuffer.isEmpty()) {
      // empty our cleartext write buffer first
      d->transmit();
   }

   return d->plainSocket->waitForBytesWritten(qt_subtract_from_timeout(msecs, stopWatch.elapsed()));
}


bool QSslSocket::waitForDisconnected(int msecs)
{
   Q_D(QSslSocket);

   // require calling connectToHost() before waitForDisconnected()
   if (state() == UnconnectedState) {
      qWarning("QSslSocket::waitForDisconnected() is not allowed in UnconnectedState");
      return false;
   }

   if (!d->plainSocket) {
      return false;
   }
   if (d->mode == UnencryptedMode) {
      return d->plainSocket->waitForDisconnected(msecs);
   }

   QElapsedTimer stopWatch;
   stopWatch.start();

   if (!d->connectionEncrypted) {
      // Wait until we've entered encrypted mode, or until a failure occurs.
      if (!waitForEncrypted(msecs)) {
         return false;
      }
   }

   bool retVal = d->plainSocket->waitForDisconnected(qt_subtract_from_timeout(msecs, stopWatch.elapsed()));
   if (!retVal) {
      setSocketState(d->plainSocket->state());
      setSocketError(d->plainSocket->error());
      setErrorString(d->plainSocket->errorString());
   }
   return retVal;
}

QList<QSslError> QSslSocket::sslErrors() const
{
   Q_D(const QSslSocket);
   return d->sslErrors;
}

bool QSslSocket::supportsSsl()
{
   return QSslSocketPrivate::supportsSsl();
}

long QSslSocket::sslLibraryVersionNumber()
{
   return QSslSocketPrivate::sslLibraryVersionNumber();
}

QString QSslSocket::sslLibraryVersionString()
{
   return QSslSocketPrivate::sslLibraryVersionString();
}

long QSslSocket::sslLibraryBuildVersionNumber()
{
   return QSslSocketPrivate::sslLibraryBuildVersionNumber();
}

QString QSslSocket::sslLibraryBuildVersionString()
{
   return QSslSocketPrivate::sslLibraryBuildVersionString();
}

void QSslSocket::startClientEncryption()
{
   Q_D(QSslSocket);
   if (d->mode != UnencryptedMode) {
      qWarning("QSslSocket::startClientEncryption: cannot start handshake on non-plain connection");
      return;
   }

   if (state() != ConnectedState) {
      qWarning("QSslSocket::startClientEncryption: cannot start handshake when not connected");
      return;
   }

#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::startClientEncryption()";
#endif

   d->mode = SslClientMode;
   emit modeChanged(d->mode);
   d->startClientEncryption();
}


void QSslSocket::startServerEncryption()
{
   Q_D(QSslSocket);
   if (d->mode != UnencryptedMode) {
      qWarning("QSslSocket::startServerEncryption: cannot start handshake on non-plain connection");
      return;
   }

#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::startServerEncryption()";
#endif

   d->mode = SslServerMode;
   emit modeChanged(d->mode);
   d->startServerEncryption();
}

void QSslSocket::ignoreSslErrors()
{
   Q_D(QSslSocket);
   d->ignoreAllSslErrors = true;
}

void QSslSocket::ignoreSslErrors(const QList<QSslError> &errors)
{
   Q_D(QSslSocket);
   d->ignoreErrorsList = errors;
}

/*!
    \internal
*/
void QSslSocket::connectToHost(const QString &hostName, quint16 port, OpenMode openMode, NetworkLayerProtocol protocol)
{
   Q_D(QSslSocket);

   d->preferredNetworkLayerProtocol = protocol;
   if (!d->initialized) {
      d->init();
   }
   d->initialized = false;

#ifdef QSSLSOCKET_DEBUG
   qDebug()     << "QSslSocket::connectToHost("
                << hostName << ',' << port << ',' << openMode << ')';
#endif

   if (!d->plainSocket) {

#ifdef QSSLSOCKET_DEBUG
      qDebug() << "\tcreating internal plain socket";
#endif
      d->createPlainSocket(openMode);
   }

#ifndef QT_NO_NETWORKPROXY
   d->plainSocket->setProxy(proxy());
#endif
   QIODevice::open(openMode);
   d->plainSocket->connectToHost(hostName, port, openMode);
   d->cachedSocketDescriptor = d->plainSocket->socketDescriptor();
}

/*!
    \internal
*/
void QSslSocket::disconnectFromHost()
{
   Q_D(QSslSocket);

#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::disconnectFromHost()";
#endif
   if (!d->plainSocket) {
      return;
   }

   if (d->state == UnconnectedState) {
      return;
   }
   if (d->mode == UnencryptedMode && !d->autoStartHandshake) {
      d->plainSocket->disconnectFromHost();
      return;
   }
   if (d->state <= ConnectingState) {
      d->pendingClose = true;
      return;
   }

   // Perhaps emit closing()
   if (d->state != ClosingState) {
      d->state = ClosingState;
      emit stateChanged(d->state);
   }

   if (!d->writeBuffer.isEmpty()) {
      d->pendingClose = true;
      return;
   }

   if (d->mode == UnencryptedMode) {
      d->plainSocket->disconnectFromHost();
   } else {
      d->disconnectFromHost();
   }
}

qint64 QSslSocket::readData(char *data, qint64 maxlen)
{
   Q_D(QSslSocket);
   qint64 readBytes = 0;

   if (d->mode == UnencryptedMode && !d->autoStartHandshake) {
      readBytes = d->plainSocket->read(data, maxlen);

#ifdef QSSLSOCKET_DEBUG
      qDebug() << "QSslSocket::readData(" << (void *)data << ',' << maxlen << ") ==" << readBytes;
#endif

   } else {
      // possibly trigger another transmit() to decrypt more data from the socket
      if (d->plainSocket->bytesAvailable()) {
         QMetaObject::invokeMethod(this, "_q_flushReadBuffer", Qt::QueuedConnection);
      }
   }

   return readBytes;
}

qint64 QSslSocket::writeData(const char *data, qint64 len)
{
   Q_D(QSslSocket);

#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::writeData(" << (void *)data << ',' << len << ')';
#endif

   if (d->mode == UnencryptedMode && ! d->autoStartHandshake) {
      return d->plainSocket->write(data, len);
   }

   char *writePtr = d->writeBuffer.reserve(len);
   ::memcpy(writePtr, data, len);

   // make sure we flush to the plain socket's buffer
   QMetaObject::invokeMethod(this, "_q_flushWriteBuffer", Qt::QueuedConnection);

   return len;
}

/*!
    \internal
*/
QSslSocketPrivate::QSslSocketPrivate()
   : initialized(false)
   , mode(QSslSocket::UnencryptedMode)
   , autoStartHandshake(false)
   , connectionEncrypted(false)
   , shutdown(false)
   , ignoreAllSslErrors(false)
   , readyReadEmittedPointer(0)
   , allowRootCertOnDemandLoading(true)
   , plainSocket(0)
   , paused(false)
{
   QSslConfigurationPrivate::deepCopyDefaultConfiguration(&configuration);
}

/*!
    \internal
*/
QSslSocketPrivate::~QSslSocketPrivate()
{
}

/*!
    \internal
*/
void QSslSocketPrivate::init()
{
   mode = QSslSocket::UnencryptedMode;
   autoStartHandshake = false;
   connectionEncrypted = false;
   ignoreAllSslErrors = false;
   shutdown = false;
   pendingClose = false;

   // we don't want to clear the ignoreErrorsList, so
   // that it is possible setting it before connecting
   //    ignoreErrorsList.clear();

   buffer.clear();
   writeBuffer.clear();
   configuration.peerCertificate.clear();
   configuration.peerCertificateChain.clear();
}

/*!
    \internal
*/
QList<QSslCipher> QSslSocketPrivate::defaultCiphers()
{
   QSslSocketPrivate::ensureInitialized();
   QMutexLocker locker(&globalData()->mutex);
   return globalData()->config->ciphers;
}

/*!
    \internal
*/
QList<QSslCipher> QSslSocketPrivate::supportedCiphers()
{
   QSslSocketPrivate::ensureInitialized();
   QMutexLocker locker(&globalData()->mutex);
   return globalData()->supportedCiphers;
}

/*!
    \internal
*/
void QSslSocketPrivate::setDefaultCiphers(const QList<QSslCipher> &ciphers)
{
   QMutexLocker locker(&globalData()->mutex);
   globalData()->config.detach();
   globalData()->config->ciphers = ciphers;
}

/*!
    \internal
*/
void QSslSocketPrivate::setDefaultSupportedCiphers(const QList<QSslCipher> &ciphers)
{
   QMutexLocker locker(&globalData()->mutex);
   globalData()->config.detach();
   globalData()->supportedCiphers = ciphers;
}

/*!
    \internal
*/
QVector<QSslEllipticCurve> QSslSocketPrivate::supportedEllipticCurves()
{
   QSslSocketPrivate::ensureInitialized();
   const QMutexLocker locker(&globalData()->mutex);
   return globalData()->supportedEllipticCurves;
}
void QSslSocketPrivate::setDefaultSupportedEllipticCurves(const QVector<QSslEllipticCurve> &curves)
{
   const QMutexLocker locker(&globalData()->mutex);
   globalData()->config.detach();
   globalData()->supportedEllipticCurves = curves;
}
QList<QSslCertificate> QSslSocketPrivate::defaultCaCertificates()
{
   QSslSocketPrivate::ensureInitialized();
   QMutexLocker locker(&globalData()->mutex);
   return globalData()->config->caCertificates;
}

/*!
    \internal
*/
void QSslSocketPrivate::setDefaultCaCertificates(const QList<QSslCertificate> &certs)
{
   QSslSocketPrivate::ensureInitialized();
   QMutexLocker locker(&globalData()->mutex);
   globalData()->config.detach();
   globalData()->config->caCertificates = certs;
   // when the certificates are set explicitly, we do not want to
   // load the system certificates on demand
   s_loadRootCertsOnDemand = false;
}

/*!
    \internal
*/
bool QSslSocketPrivate::addDefaultCaCertificates(const QString &path, QSsl::EncodingFormat format,
      QRegExp::PatternSyntax syntax)
{
   QSslSocketPrivate::ensureInitialized();
   QList<QSslCertificate> certs = QSslCertificate::fromPath(path, format, syntax);
   if (certs.isEmpty()) {
      return false;
   }

   QMutexLocker locker(&globalData()->mutex);
   globalData()->config.detach();
   globalData()->config->caCertificates += certs;
   return true;
}

/*!
    \internal
*/
void QSslSocketPrivate::addDefaultCaCertificate(const QSslCertificate &cert)
{
   QSslSocketPrivate::ensureInitialized();
   QMutexLocker locker(&globalData()->mutex);
   globalData()->config.detach();
   globalData()->config->caCertificates += cert;
}

/*!
    \internal
*/
void QSslSocketPrivate::addDefaultCaCertificates(const QList<QSslCertificate> &certs)
{
   QSslSocketPrivate::ensureInitialized();
   QMutexLocker locker(&globalData()->mutex);
   globalData()->config.detach();
   globalData()->config->caCertificates += certs;
}

/*!
    \internal
*/
QSslConfiguration QSslConfigurationPrivate::defaultConfiguration()
{
   QSslSocketPrivate::ensureInitialized();
   QMutexLocker locker(&globalData()->mutex);
   return QSslConfiguration(globalData()->config.data());
}

/*!
    \internal
*/
void QSslConfigurationPrivate::setDefaultConfiguration(const QSslConfiguration &configuration)
{
   QSslSocketPrivate::ensureInitialized();
   QMutexLocker locker(&globalData()->mutex);
   if (globalData()->config == configuration.d) {
      return;   // nothing to do
   }

   globalData()->config = const_cast<QSslConfigurationPrivate *>(configuration.d.constData());
}


/*!
    \internal
*/
void QSslConfigurationPrivate::deepCopyDefaultConfiguration(QSslConfigurationPrivate *ptr)
{
   QSslSocketPrivate::ensureInitialized();
   QMutexLocker locker(&globalData()->mutex);
   const QSslConfigurationPrivate *global = globalData()->config.constData();

   if (! global) {
      return;
   }

   ptr->ref.store(1);
   ptr->peerCertificate       = global->peerCertificate;
   ptr->peerCertificateChain  = global->peerCertificateChain;
   ptr->localCertificateChain = global->localCertificateChain;
   ptr->privateKey = global->privateKey;
   ptr->sessionCipher = global->sessionCipher;
   ptr->sessionProtocol = global->sessionProtocol;
   ptr->ciphers = global->ciphers;
   ptr->caCertificates = global->caCertificates;
   ptr->protocol = global->protocol;
   ptr->peerVerifyMode = global->peerVerifyMode;
   ptr->peerVerifyDepth = global->peerVerifyDepth;
   ptr->sslOptions = global->sslOptions;
   ptr->ellipticCurves = global->ellipticCurves;
}

/*!
    \internal
*/
void QSslSocketPrivate::createPlainSocket(QIODevice::OpenMode openMode)
{
   Q_Q(QSslSocket);
   q->setOpenMode(openMode); // <- from QIODevice
   q->setSocketState(QAbstractSocket::UnconnectedState);
   q->setSocketError(QAbstractSocket::UnknownSocketError);
   q->setLocalPort(0);
   q->setLocalAddress(QHostAddress());
   q->setPeerPort(0);
   q->setPeerAddress(QHostAddress());
   q->setPeerName(QString());

   plainSocket = new QTcpSocket(q);

#ifndef QT_NO_BEARERMANAGEMENT
   //copy network session down to the plain socket (if it has been set)
   plainSocket->setProperty("_q_networksession", q->property("_q_networksession"));
#endif

   q->connect(plainSocket, SIGNAL(connected()), q, SLOT(_q_connectedSlot()), Qt::DirectConnection);

   q->connect(plainSocket, SIGNAL(hostFound()), q, SLOT(_q_hostFoundSlot()), Qt::DirectConnection);

   q->connect(plainSocket, SIGNAL(disconnected()), q, SLOT(_q_disconnectedSlot()), Qt::DirectConnection);

   q->connect(plainSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
              q, SLOT(_q_stateChangedSlot(QAbstractSocket::SocketState)), Qt::DirectConnection);

   q->connect(plainSocket, SIGNAL(error(QAbstractSocket::SocketError)),
              q, SLOT(_q_errorSlot(QAbstractSocket::SocketError)), Qt::DirectConnection);

   q->connect(plainSocket, SIGNAL(readyRead()), q, SLOT(_q_readyReadSlot()), Qt::DirectConnection);

   q->connect(plainSocket, SIGNAL(bytesWritten(qint64)), q, SLOT(_q_bytesWrittenSlot(qint64)), Qt::DirectConnection);

#ifndef QT_NO_NETWORKPROXY
   q->connect(plainSocket, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *)),
              q, SLOT(proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *)));
#endif

   buffer.clear();
   writeBuffer.clear();
   connectionEncrypted = false;
   configuration.peerCertificate.clear();
   configuration.peerCertificateChain.clear();
   mode = QSslSocket::UnencryptedMode;
   q->setReadBufferSize(readBufferMaxSize);
}

void QSslSocketPrivate::pauseSocketNotifiers(QSslSocket *socket)
{
   if (! socket->d_func()->plainSocket) {
      return;
   }
   QAbstractSocketPrivate::pauseSocketNotifiers(socket->d_func()->plainSocket);
}

void QSslSocketPrivate::resumeSocketNotifiers(QSslSocket *socket)
{
   if (!socket->d_func()->plainSocket) {
      return;
   }
   QAbstractSocketPrivate::resumeSocketNotifiers(socket->d_func()->plainSocket);
}

bool QSslSocketPrivate::isPaused() const
{
   return paused;
}
bool QSslSocketPrivate::bind(const QHostAddress &address, quint16 port, QAbstractSocket::BindMode mode)
{
   // this function is called from QAbstractSocket::bind
   if (!initialized) {
      init();
   }
   initialized = false;

#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::bind(" << address << ',' << port << ',' << mode << ')';
#endif

   if (! plainSocket) {
#ifdef QSSLSOCKET_DEBUG
      qDebug() << ", Creating internal plain socket";
#endif

      createPlainSocket(QIODevice::ReadWrite);
   }
   bool ret = plainSocket->bind(address, port, mode);
   localPort = plainSocket->localPort();
   localAddress = plainSocket->localAddress();
   cachedSocketDescriptor = plainSocket->socketDescriptor();
   return ret;
}
/*!
    \internal
*/
void QSslSocketPrivate::_q_connectedSlot()
{
   Q_Q(QSslSocket);
   q->setLocalPort(plainSocket->localPort());
   q->setLocalAddress(plainSocket->localAddress());
   q->setPeerPort(plainSocket->peerPort());
   q->setPeerAddress(plainSocket->peerAddress());
   q->setPeerName(plainSocket->peerName());
   cachedSocketDescriptor = plainSocket->socketDescriptor();

#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::_q_connectedSlot()";
   qDebug() << "\tstate =" << q->state();
   qDebug() << "\tpeer =" << q->peerName() << q->peerAddress() << q->peerPort();
   qDebug()     << "\tlocal =" << QHostInfo::fromName(q->localAddress().toString()).hostName()
                << q->localAddress() << q->localPort();
#endif
   if (autoStartHandshake) {
      q->startClientEncryption();
   }
   emit q->connected();


   if (pendingClose && !autoStartHandshake) {
      pendingClose = false;
      q->disconnectFromHost();
   }
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_hostFoundSlot()
{
   Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::_q_hostFoundSlot()";
   qDebug() << "\tstate =" << q->state();
#endif
   emit q->hostFound();
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_disconnectedSlot()
{
   Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::_q_disconnectedSlot()";
   qDebug() << "\tstate =" << q->state();
#endif
   disconnected();
   emit q->disconnected();
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_stateChangedSlot(QAbstractSocket::SocketState state)
{
   Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::_q_stateChangedSlot(" << state << ')';
#endif
   q->setSocketState(state);
   emit q->stateChanged(state);
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_errorSlot(QAbstractSocket::SocketError error)
{
   Q_Q(QSslSocket);

#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::_q_errorSlot(" << error << ')';
   qDebug() << "\tstate =" << q->state();
   qDebug() << "\terrorString =" << q->errorString();
#endif


   // this moves encrypted bytes from plain socket into our buffer
   if (plainSocket->bytesAvailable()) {
      qint64 tmpReadBufferMaxSize = readBufferMaxSize;
      readBufferMaxSize = 0; // reset temporarily so the plain sockets completely drained drained
      transmit();
      readBufferMaxSize = tmpReadBufferMaxSize;
   }

   q->setSocketError(plainSocket->error());
   q->setErrorString(plainSocket->errorString());
   emit q->error(error);
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_readyReadSlot()
{
   Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::_q_readyReadSlot() -" << plainSocket->bytesAvailable() << "bytes available";
#endif
   if (mode == QSslSocket::UnencryptedMode) {
      if (readyReadEmittedPointer) {
         *readyReadEmittedPointer = true;
      }
      emit q->readyRead();
      return;
   }

   transmit();
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_bytesWrittenSlot(qint64 written)
{
   Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
   qDebug() << "QSslSocket::_q_bytesWrittenSlot(" << written << ')';
#endif

   if (mode == QSslSocket::UnencryptedMode) {
      emit q->bytesWritten(written);
   } else {
      emit q->encryptedBytesWritten(written);
   }

   if (state == QAbstractSocket::ClosingState && writeBuffer.isEmpty()) {
      q->disconnectFromHost();
   }
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_flushWriteBuffer()
{
   Q_Q(QSslSocket);
   if (!writeBuffer.isEmpty()) {
      q->flush();
   }
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_flushReadBuffer()
{
   // trigger a read from the plainSocket into SSL
   if (mode != QSslSocket::UnencryptedMode) {
      transmit();
   }
}

void QSslSocketPrivate::_q_resumeImplementation()
{
   if (plainSocket) {
      plainSocket->resume();
   }
   paused = false;
   if (!connectionEncrypted) {
      if (verifyErrorsHaveBeenIgnored()) {
         continueHandshake();
      } else {
         setErrorAndEmit(QAbstractSocket::SslHandshakeFailedError, sslErrors.first().errorString());
         plainSocket->disconnectFromHost();
         return;
      }
   }
   transmit();
}
bool QSslSocketPrivate::verifyErrorsHaveBeenIgnored()
{
   bool doEmitSslError;
   if (!ignoreErrorsList.empty()) {
      // check whether the errors we got are all in the list of expected errors
      // (applies only if the method QSslSocket::ignoreSslErrors(const QList<QSslError> &errors)
      // was called)
      doEmitSslError = false;
      for (int a = 0; a < sslErrors.count(); a++) {
         if (!ignoreErrorsList.contains(sslErrors.at(a))) {
            doEmitSslError = true;
            break;
         }
      }
   } else {
      // if QSslSocket::ignoreSslErrors(const QList<QSslError> &errors) was not called and
      // we get an SSL error, emit a signal unless we ignored all errors (by calling
      // QSslSocket::ignoreSslErrors() )
      doEmitSslError = !ignoreAllSslErrors;
   }
   return !doEmitSslError;
}

/*!
    \internal
*/
qint64 QSslSocketPrivate::peek(char *data, qint64 maxSize)
{
   if (mode == QSslSocket::UnencryptedMode && !autoStartHandshake) {
      //unencrypted mode - do not use QIODevice::peek, as it reads ahead data from the plain socket
      //peek at data already in the QIODevice buffer (from a previous read)
      qint64 r = buffer.peek(data, maxSize);
      if (r == maxSize) {
         return r;
      }
      data += r;
      //peek at data in the plain socket
      if (plainSocket) {
         qint64 r2 = plainSocket->peek(data, maxSize - r);
         if (r2 < 0) {
            return (r > 0 ? r : r2);
         }
         return r + r2;
      } else {
         return -1;
      }
   } else {
      //encrypted mode - the socket engine will read and decrypt data into the QIODevice buffer
      return QTcpSocketPrivate::peek(data, maxSize);
   }
}

/*!
    \internal
*/
QByteArray QSslSocketPrivate::peek(qint64 maxSize)
{
   if (mode == QSslSocket::UnencryptedMode && !autoStartHandshake) {
      //unencrypted mode - do not use QIODevice::peek, as it reads ahead data from the plain socket
      //peek at data already in the QIODevice buffer (from a previous read)
      QByteArray ret;
      ret.reserve(maxSize);
      ret.resize(buffer.peek(ret.data(), maxSize));
      if (ret.length() == maxSize) {
         return ret;
      }
      //peek at data in the plain socket
      if (plainSocket) {
         return ret + plainSocket->peek(maxSize - ret.length());
      } else {
         return QByteArray();
      }
   } else {
      //encrypted mode - the socket engine will read and decrypt data into the QIODevice buffer
      return QTcpSocketPrivate::peek(maxSize);
   }
}

/*!
    \internal
*/
bool QSslSocketPrivate::rootCertOnDemandLoadingSupported()
{
   return s_loadRootCertsOnDemand;
}

/*!
    \internal
*/
QList<QByteArray> QSslSocketPrivate::unixRootCertDirectories()
{
   return QList<QByteArray>()   <<  "/etc/ssl/certs/"// (K)ubuntu, OpenSUSE, Mandriva, MeeGo ...
          << "/usr/lib/ssl/certs/" // Gentoo, Mandrake
          << "/usr/share/ssl/" // Centos, Redhat, SuSE
          << "/usr/local/ssl/" // Normal OpenSSL Tarball
          << "/var/ssl/certs/" // AIX
          << "/usr/local/ssl/certs/" // Solaris
          << "/opt/openssl/certs/"; // HP-UX
}

void QSslSocketPrivate::checkSettingSslContext(QSslSocket *socket, QSharedPointer<QSslContext> sslContext)
{
   if (socket->d_func()->sslContextPointer.isNull()) {
      socket->d_func()->sslContextPointer = sslContext;
   }
}

QSharedPointer<QSslContext> QSslSocketPrivate::sslContext(QSslSocket *socket)
{
   return (socket) ? socket->d_func()->sslContextPointer : QSharedPointer<QSslContext>();
}

bool QSslSocketPrivate::isMatchingHostname(const QSslCertificate &cert, const QString &peerName)
{
   QStringList commonNameList = cert.subjectInfo(QSslCertificate::CommonName);

   for (const QString &commonName : commonNameList) {
      if (isMatchingHostname(commonName.toLower(), peerName.toLower())) {
         return true;
      }
   }

   for (const QString &altName : cert.subjectAlternativeNames().values(QSsl::DnsEntry)) {
      if (isMatchingHostname(altName.toLower(), peerName.toLower())) {
         return true;
      }
   }

   return false;
}

bool QSslSocketPrivate::isMatchingHostname(const QString &cn, const QString &hostname)
{
   int wildcard = cn.indexOf(QLatin1Char('*'));

   // Check this is a wildcard cert, if not then just compare the strings
   if (wildcard < 0) {
      return cn == hostname;
   }

   int firstCnDot = cn.indexOf(QLatin1Char('.'));
   int secondCnDot = cn.indexOf(QLatin1Char('.'), firstCnDot + 1);

   // Check at least 3 components
   if ((-1 == secondCnDot) || (secondCnDot + 1 >= cn.length())) {
      return false;
   }

   // Check * is last character of 1st component (ie. there's a following .)
   if (wildcard + 1 != firstCnDot) {
      return false;
   }

   // Check only one star
   if (cn.lastIndexOf(QLatin1Char('*')) != wildcard) {
      return false;
   }

   // Check characters preceding * (if any) match
   if (wildcard && (hostname.leftRef(wildcard) != cn.leftRef(wildcard))) {
      return false;
   }

   // Check characters following first . match
   if (hostname.midRef(hostname.indexOf(QLatin1Char('.'))) != cn.midRef(firstCnDot)) {
      return false;
   }

   // Check if the hostname is an IP address, if so then wildcards are not allowed
   QHostAddress addr(hostname);
   if (!addr.isNull()) {
      return false;
   }

   // Ok, I guess this was a wildcard CN and the hostname matches.
   return true;
}

void QSslSocket::_q_connectedSlot()
{
   Q_D(QSslSocket);
   d->_q_connectedSlot();
}

void QSslSocket::_q_hostFoundSlot()
{
   Q_D(QSslSocket);
   d->_q_hostFoundSlot();
}

void QSslSocket::_q_disconnectedSlot()
{
   Q_D(QSslSocket);
   d->_q_disconnectedSlot();
}

void QSslSocket::_q_stateChangedSlot(QAbstractSocket::SocketState un_named_arg1)
{
   Q_D(QSslSocket);
   d->_q_stateChangedSlot(un_named_arg1);
}

void QSslSocket::_q_errorSlot(QAbstractSocket::SocketError un_named_arg1)
{
   Q_D(QSslSocket);
   d->_q_errorSlot(un_named_arg1);
}

void QSslSocket::_q_readyReadSlot()
{
   Q_D(QSslSocket);
   d->_q_readyReadSlot();
}

void QSslSocket::_q_bytesWrittenSlot(qint64 un_named_arg1)
{
   Q_D(QSslSocket);
   d->_q_bytesWrittenSlot(un_named_arg1);
}

void QSslSocket::_q_flushWriteBuffer()
{
   Q_D(QSslSocket);
   d->_q_flushWriteBuffer();
}

void QSslSocket::_q_flushReadBuffer()
{
   Q_D(QSslSocket);
   d->_q_flushReadBuffer();
}

void QSslSocket::_q_resumeImplementation()
{
   Q_D(QSslSocket);
   d->_q_resumeImplementation();
}

#if defined(Q_OS_WIN)
void QSslSocket::_q_caRootLoaded(QSslCertificate arg1, QSslCertificate arg2)
{
   Q_D(QSslSocket);
   d->_q_caRootLoaded(arg1, arg2);
}
#endif


