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

#include <qftp_p.h>
#include <qabstractsocket.h>

#ifndef QT_NO_FTP

#include <qtcpsocket.h>
#include <qurlinfo_p.h>
#include <qtcpserver.h>

#include <qcoreapplication.h>
#include <qfileinfo.h>
#include <qhash.h>
#include <qregularexpression.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qlocale.h>

class QFtpPI;

/*
    The QFtpDTP (DTP = Data Transfer Process) controls all client side
    data transfer between the client and server.
*/
class QFtpDTP : public QObject
{
   NET_CS_OBJECT(QFtpDTP)

 public:
   enum ConnectState {
      CsHostFound,
      CsConnected,
      CsClosed,
      CsHostNotFound,
      CsConnectionRefused
   };

   QFtpDTP(QFtpPI *p, QObject *parent = nullptr);

   void setData(QByteArray *);
   void setDevice(QIODevice *);
   void writeData();
   void setBytesTotal(qint64 bytes);

   bool hasError() const;
   QString errorMessage() const;
   void clearError();

   void connectToHost(const QString &host, quint16 port);
   int setupListener(const QHostAddress &address);
   void waitForConnection();

   QTcpSocket::SocketState state() const;
   qint64 bytesAvailable() const;
   qint64 read(char *data, qint64 maxlen);
   QByteArray readAll();

   void abortConnection();

   static bool parseDir(const QByteArray &buffer, const QString &userName, QUrlInfo *info);

   NET_CS_SIGNAL_1(Public, void listInfo(const QUrlInfo &urlInfo))
   NET_CS_SIGNAL_2(listInfo, urlInfo)

   NET_CS_SIGNAL_1(Public, void readyRead())
   NET_CS_SIGNAL_2(readyRead)

   NET_CS_SIGNAL_1(Public, void dataTransferProgress(qint64 bytesDone, qint64 bytesTotal))
   NET_CS_SIGNAL_2(dataTransferProgress, bytesDone, bytesTotal)

   NET_CS_SIGNAL_1(Public, void connectState(int connectState))
   NET_CS_SIGNAL_2(connectState, connectState)

   NET_CS_SLOT_1(Public, void dataReadyRead())
   NET_CS_SLOT_2(dataReadyRead)

 private:
   NET_CS_SLOT_1(Private, void socketConnected())
   NET_CS_SLOT_2(socketConnected)

   NET_CS_SLOT_1(Private, void socketReadyRead())
   NET_CS_SLOT_2(socketReadyRead)

   NET_CS_SLOT_1(Private, void socketError(QAbstractSocket::SocketError errorCode))
   NET_CS_SLOT_2(socketError)

   NET_CS_SLOT_1(Private, void socketConnectionClosed())
   NET_CS_SLOT_2(socketConnectionClosed)

   NET_CS_SLOT_1(Private, void socketBytesWritten(qint64 bytes))
   NET_CS_SLOT_2(socketBytesWritten)

   NET_CS_SLOT_1(Private, void setupSocket())
   NET_CS_SLOT_2(setupSocket)

   void clearData();

   QTcpSocket *socket;
   QTcpServer listener;

   QFtpPI *pi;
   QString err;
   qint64 m_bytesDone;
   qint64 m_bytesTotal;
   bool callWriteData;

   // If is_ba is true, ba is used; ba is never 0.
   // Otherwise dev is used; dev can be 0 or not.
   union {
      QByteArray *ba;
      QIODevice *dev;
   } data;

   bool is_ba;

   QByteArray bytesFromSocket;

};

/**********************************************************************
 *
 * QFtpPI - Protocol Interpreter
 *
 *********************************************************************/

class QFtpPI : public QObject
{
   NET_CS_OBJECT(QFtpPI)

 public:
   QFtpPI(QObject *parent = nullptr);

   void connectToHost(const QString &host, quint16 port);

   bool sendCommands(const QStringList &cmds);
   bool sendCommand(const QString &cmd) {
      return sendCommands(QStringList(cmd));
   }

   void clearPendingCommands();
   void abort();

   QString currentCommand() const {
      return currentCmd;
   }

   bool rawCommand;
   bool transferConnectionExtended;

   QFtpDTP dtp;

   // PI has a DTP which is not the design of RFC 959, but it
   // makes the design simpler

   NET_CS_SIGNAL_1(Public, void connectState(int connectState))
   NET_CS_SIGNAL_2(connectState, connectState)

   NET_CS_SIGNAL_1(Public, void finished(const QString &textMsg))
   NET_CS_SIGNAL_2(finished, textMsg)

   NET_CS_SIGNAL_1(Public, void error(int errorCode, const QString &textMsg))
   NET_CS_SIGNAL_OVERLOAD(error, (int, const QString &), errorCode, textMsg)

   NET_CS_SIGNAL_1(Public, void rawFtpReply(int replyCode, const QString &replyText))
   NET_CS_SIGNAL_2(rawFtpReply, replyCode, replyText)

 private:
   NET_CS_SLOT_1(Private, void hostFound())
   NET_CS_SLOT_2(hostFound)

   NET_CS_SLOT_1(Private, void connected())
   NET_CS_SLOT_2(connected)

   NET_CS_SLOT_1(Private, void connectionClosed())
   NET_CS_SLOT_2(connectionClosed)

   NET_CS_SLOT_1(Private, void delayedCloseFinished())
   NET_CS_SLOT_2(delayedCloseFinished)

   NET_CS_SLOT_1(Private, void readyRead())
   NET_CS_SLOT_2(readyRead)

   NET_CS_SLOT_1(Private, void error(QAbstractSocket::SocketError errorCode))
   NET_CS_SLOT_OVERLOAD(error, (QAbstractSocket::SocketError))

   NET_CS_SLOT_1(Private, void dtpConnectState(int state))
   NET_CS_SLOT_2(dtpConnectState)

   // the states are modelled after the generalized state diagram of RFC 959, page 58
   enum State {
      Begin,
      Idle,
      Waiting,
      Success,
      Failure
   };

   enum AbortState {
      None,
      AbortStarted,
      WaitForAbortToFinish
   };

   bool processReply();
   bool startNextCmd();

   QTcpSocket commandSocket;
   QString m_replyText;
   char m_replyCode[3];
   State state;
   AbortState abortState;
   QStringList pendingCommands;
   QString currentCmd;

   bool waitForDtpToConnect;
   bool waitForDtpToClose;

   QByteArray bytesFromSocket;

   friend class QFtpDTP;
};

/**********************************************************************
 *
 * QFtpCommand implemenatation
 *
 *********************************************************************/
class QFtpCommand
{

 public:
   QFtpCommand(QFtp::Command cmd, const QStringList &raw, const QByteArray &ba);
   QFtpCommand(QFtp::Command cmd, const QStringList &raw, QIODevice *dev = nullptr);
   ~QFtpCommand();

   int id;
   QFtp::Command command;
   QStringList rawCmds;

   // If is_ba is true, ba is used; ba is never 0.
   // Otherwise dev is used; dev can be 0 or not.
   union {
      QByteArray *ba;
      QIODevice *dev;
   } data;

   bool is_ba;

   static QAtomicInt idCounter;
};

QAtomicInt QFtpCommand::idCounter = 1;

QFtpCommand::QFtpCommand(QFtp::Command cmd, const QStringList &raw, const QByteArray &ba)
   : command(cmd), rawCmds(raw), is_ba(true)
{
   id = idCounter.fetchAndAddRelaxed(1);
   data.ba = new QByteArray(ba);
}

QFtpCommand::QFtpCommand(QFtp::Command cmd, const QStringList &raw, QIODevice *dev)
   : command(cmd), rawCmds(raw), is_ba(false)
{
   id = idCounter.fetchAndAddRelaxed(1);
   data.dev = dev;
}

QFtpCommand::~QFtpCommand()
{
   if (is_ba) {
      delete data.ba;
   }
}

/**********************************************************************
 *
 * QFtpDTP implemenatation
 *
 *********************************************************************/
QFtpDTP::QFtpDTP(QFtpPI *p, QObject *parent)
   : QObject(parent), socket(nullptr), listener(this), pi(p), callWriteData(false)
{
   clearData();
   listener.setObjectName("QFtpDTP active state server");

   connect(&listener, &QTcpServer::newConnection, this, &QFtpDTP::setupSocket);
}

void QFtpDTP::setData(QByteArray *ba)
{
   is_ba = true;
   data.ba = ba;
}

void QFtpDTP::setDevice(QIODevice *dev)
{
   is_ba = false;
   data.dev = dev;
}

void QFtpDTP::setBytesTotal(qint64 bytes)
{
   m_bytesTotal = bytes;
   m_bytesDone = 0;
   emit dataTransferProgress(m_bytesDone, m_bytesTotal);
}

void QFtpDTP::connectToHost(const QString &host, quint16 port)
{
   bytesFromSocket.clear();

   if (socket) {
      delete socket;
      socket = nullptr;
   }
   socket = new QTcpSocket(this);

#ifndef QT_NO_BEARERMANAGEMENT
   // copy network session down to the socket
   socket->setProperty("_q_networksession", property("_q_networksession"));
#endif

   socket->setObjectName("QFtpDTP Passive state socket");

   connect(socket, &QTcpSocket::connected,    this, &QFtpDTP::socketConnected);
   connect(socket, &QTcpSocket::readyRead,    this, &QFtpDTP::socketReadyRead);
   connect(socket, &QTcpSocket::error,        this, &QFtpDTP::socketError);
   connect(socket, &QTcpSocket::disconnected, this, &QFtpDTP::socketConnectionClosed);
   connect(socket, &QTcpSocket::bytesWritten, this, &QFtpDTP::socketBytesWritten);

   socket->connectToHost(host, port);
}

int QFtpDTP::setupListener(const QHostAddress &address)
{

#ifndef QT_NO_BEARERMANAGEMENT
   // copy network session down to the socket
   listener.setProperty("_q_networksession", property("_q_networksession"));
#endif

   if (! listener.isListening() && ! listener.listen(address, 0)) {
      return -1;
   }

   return listener.serverPort();
}

void QFtpDTP::waitForConnection()
{
   // This function is only interesting in Active transfer mode; it works
   // around a limitation in QFtp's design by blocking, waiting for an
   // incoming connection. For the default Passive mode, it does nothing.
   if (listener.isListening()) {
      listener.waitForNewConnection();
   }
}

QTcpSocket::SocketState QFtpDTP::state() const
{
   return socket ? socket->state() : QTcpSocket::UnconnectedState;
}

qint64 QFtpDTP::bytesAvailable() const
{
   if (! socket || socket->state() != QTcpSocket::ConnectedState) {
      return (qint64) bytesFromSocket.size();
   }

   return socket->bytesAvailable();
}

qint64 QFtpDTP::read(char *data, qint64 maxlen)
{
   qint64 read;

   if (socket && socket->state() == QTcpSocket::ConnectedState) {
      read = socket->read(data, maxlen);
   } else {
      read = qMin(maxlen, qint64(bytesFromSocket.size()));
      memcpy(data, bytesFromSocket.data(), read);
      bytesFromSocket.remove(0, read);
   }

   m_bytesDone += read;

   return read;
}

QByteArray QFtpDTP::readAll()
{
   QByteArray tmp;
   if (socket && socket->state() == QTcpSocket::ConnectedState) {
      tmp = socket->readAll();
      m_bytesDone += tmp.size();
   } else {
      tmp = bytesFromSocket;
      bytesFromSocket.clear();
   }

   return tmp;
}

void QFtpDTP::writeData()
{
   if (! socket) {
      return;
   }

   if (is_ba) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QFtpDTP::writeData: write %d bytes", data.ba->size());
#endif

      if (data.ba->size() == 0) {
         emit dataTransferProgress(0, m_bytesTotal);
      } else {
         socket->write(data.ba->data(), data.ba->size());
      }

      socket->close();

      clearData();

   } else if (data.dev) {
      callWriteData = false;
      const qint64 blockSize = 16 * 1024;
      char buf[16 * 1024];
      qint64 read = data.dev->read(buf, blockSize);

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QFtpDTP::writeData: write() of size %lli bytes", read);
#endif

      if (read > 0) {
         socket->write(buf, read);
      } else if (read == -1 || (!data.dev->isSequential() && data.dev->atEnd())) {
         // error or EOF
         if (m_bytesDone == 0 && socket->bytesToWrite() == 0) {
            emit dataTransferProgress(0, m_bytesTotal);
         }
         socket->close();
         clearData();
      }

      // do we continue uploading?
      callWriteData = data.dev != nullptr;
   }
}

void QFtpDTP::dataReadyRead()
{
   writeData();
}

inline bool QFtpDTP::hasError() const
{
   return ! err.isEmpty();
}

inline QString QFtpDTP::errorMessage() const
{
   return err;
}

inline void QFtpDTP::clearError()
{
   err.clear();
}

void QFtpDTP::abortConnection()
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QFtpDTP::abortConnection, bytesAvailable == %lli", socket ? socket->bytesAvailable() : (qint64) 0);
#endif

   callWriteData = false;
   clearData();

   if (socket) {
      socket->abort();
   }
}

static void _q_fixupDateTime(QDateTime *dateTime)
{
   // Adjust for future tolerance.
   const int futureTolerance = 86400;

   if (dateTime->secsTo(QDateTime::currentDateTime()) < -futureTolerance) {
      QDate d = dateTime->date();

      d.setDate(d.year() - 1, d.month(), d.day());
      dateTime->setDate(d);
   }
}

static void _q_parseUnixDir(const QStringList &tokens, const QString &userName, QUrlInfo *info)
{
   // Unix style, 7 + 1 entries
   // -rw-r--r--    1 ftp      ftp      17358091 Aug 10  2004 qt-x11-free-3.3.3.tar.gz
   // drwxr-xr-x    3 ftp      ftp          4096 Apr 14  2000 compiled-examples
   // lrwxrwxrwx    1 ftp      ftp             9 Oct 29  2005 qtscape -> qtmozilla
   if (tokens.size() != 8) {
      return;
   }

   char first = tokens.at(1).at(0).toLatin1();
   if (first == 'd') {
      info->setDir(true);
      info->setFile(false);
      info->setSymLink(false);
   } else if (first == '-') {
      info->setDir(false);
      info->setFile(true);
      info->setSymLink(false);
   } else if (first == 'l') {
      info->setDir(true);
      info->setFile(false);
      info->setSymLink(true);
   }

   // Resolve filename
   QString name = tokens.at(7);
   if (info->isSymLink()) {
      int linkPos = name.indexOf(QLatin1String(" ->"));
      if (linkPos != -1) {
         name.resize(linkPos);
      }
   }
   info->setName(name);

   // Resolve owner & group
   info->setOwner(tokens.at(3));
   info->setGroup(tokens.at(4));

   // Resolve size
   info->setSize(tokens.at(5).toInteger<qint64>());

   QStringList formats;
   formats << QLatin1String("MMM dd  yyyy") << QLatin1String("MMM dd hh:mm") << QLatin1String("MMM  d  yyyy")
           << QLatin1String("MMM  d hh:mm") << QLatin1String("MMM  d yyyy") << QLatin1String("MMM dd yyyy");

   QString dateString = tokens.at(6);
   dateString.replace(0, 1,dateString[0].toUpper());

   // Resolve the modification date by parsing all possible formats
   QDateTime dateTime;
   int n = 0;

   do {
      dateTime = QLocale::c().toDateTime(dateString, formats.at(n++));
   }  while (n < formats.size() && (!dateTime.isValid()));

   if (n == 2 || n == 4) {
      // Guess the year
      dateTime.setDate(QDate(QDate::currentDate().year(), dateTime.date().month(), dateTime.date().day()));
      _q_fixupDateTime(&dateTime);
   }

   if (dateTime.isValid()) {
      info->setLastModified(dateTime);
   }

   // Resolve permissions
   int permissions = 0;
   QString p = tokens.at(2);
   permissions |= (p[0] == QChar('r') ? QUrlInfo::ReadOwner : 0);
   permissions |= (p[1] == QChar('w') ? QUrlInfo::WriteOwner : 0);
   permissions |= (p[2] == QChar('x') ? QUrlInfo::ExeOwner : 0);
   permissions |= (p[3] == QChar('r') ? QUrlInfo::ReadGroup : 0);
   permissions |= (p[4] == QChar('w') ? QUrlInfo::WriteGroup : 0);
   permissions |= (p[5] == QChar('x') ? QUrlInfo::ExeGroup : 0);
   permissions |= (p[6] == QChar('r') ? QUrlInfo::ReadOther : 0);
   permissions |= (p[7] == QChar('w') ? QUrlInfo::WriteOther : 0);
   permissions |= (p[8] == QChar('x') ? QUrlInfo::ExeOther : 0);
   info->setPermissions(permissions);

   bool isOwner = info->owner() == userName;
   info->setReadable((permissions & QUrlInfo::ReadOther) || ((permissions & QUrlInfo::ReadOwner) && isOwner));
   info->setWritable((permissions & QUrlInfo::WriteOther) || ((permissions & QUrlInfo::WriteOwner) && isOwner));
}

static void _q_parseDosDir(const QStringList &tokens, const QString &userName, QUrlInfo *info)
{
   // DOS style, 3 + 1 entries
   // 01-16-02  11:14AM       <DIR>          epsgroup
   // 06-05-03  03:19PM                 1973 readme.txt
   if (tokens.size() != 4) {
      return;
   }

   (void) userName;

   QString name = tokens.at(3);
   info->setName(name);
   info->setSymLink(name.toLower().endsWith(".lnk"));

   if (tokens.at(2) == QLatin1String("<DIR>")) {
      info->setFile(false);
      info->setDir(true);

   } else {
      info->setFile(true);
      info->setDir(false);
      info->setSize(tokens.at(2).toInteger<qint64>());
   }

   // Note: We cannot use QFileInfo; permissions are for the server-side
   // machine, and QFileInfo's behavior depends on the local platform.

   int permissions = QUrlInfo::ReadOwner | QUrlInfo::WriteOwner
                     | QUrlInfo::ReadGroup | QUrlInfo::WriteGroup
                     | QUrlInfo::ReadOther | QUrlInfo::WriteOther;
   QString ext;
   int extIndex = name.lastIndexOf(QChar('.'));

   if (extIndex != -1) {
      ext = name.mid(extIndex + 1);
   }
   if (ext == QLatin1String("exe") || ext == QLatin1String("bat") || ext == QLatin1String("com")) {
      permissions |= QUrlInfo::ExeOwner | QUrlInfo::ExeGroup | QUrlInfo::ExeOther;
   }

   info->setPermissions(permissions);

   info->setReadable(true);
   info->setWritable(info->isFile());

   QDateTime dateTime = QLocale::c().toDateTime(tokens.at(1), QLatin1String("MM-dd-yy  hh:mmAP"));

   if (dateTime.date().year() < 1971) {
      dateTime.setDate(QDate(dateTime.date().year() + 100, dateTime.date().month(), dateTime.date().day()));
   }

   info->setLastModified(dateTime);
}

bool QFtpDTP::parseDir(const QByteArray &buffer, const QString &userName, QUrlInfo *info)
{
   if (buffer.isEmpty()) {
      return false;
   }

   QString bufferStr = QString::fromUtf8(buffer).trimmed();

   // Unix style FTP servers
   QRegularExpression unixPattern("^([\\-dl])([a-zA-Z\\-]{9,9})\\s+\\d+\\s+(\\S*)\\s+(\\S*)\\s+(\\d+)\\s+(\\S+\\s+\\S+\\s+\\S+)\\s+(\\S.*)");
   QRegularExpressionMatch unixMatch = unixPattern.match(bufferStr);

   if (unixMatch.capturedStart(0) == bufferStr.cbegin()) {
      _q_parseUnixDir(unixMatch.capturedTexts(), userName, info);
      return true;
   }

   // DOS style FTP servers
   QRegularExpression dosPattern("^(\\d\\d-\\d\\d-\\d\\d\\ \\ \\d\\d:\\d\\d[AP]M)\\s+(<DIR>|\\d+)\\s+(\\S.*)$");
   QRegularExpressionMatch dosMatch = dosPattern.match(bufferStr);

   if (dosMatch.capturedStart(0) == bufferStr.cbegin()) {
      _q_parseDosDir(dosMatch.capturedTexts(), userName, info);
      return true;
   }

   // unsupported
   return false;
}

void QFtpDTP::socketConnected()
{
   m_bytesDone = 0;

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QFtpDTP::connectState(CsConnected)");
#endif

   emit connectState(QFtpDTP::CsConnected);
}

void QFtpDTP::socketReadyRead()
{
   if (! socket) {
      return;
   }

   if (pi->currentCommand().isEmpty()) {
      socket->close();

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QFtpDTP::connectState(CsClosed)");
#endif

      emit connectState(QFtpDTP::CsClosed);
      return;
   }

   if (pi->abortState != QFtpPI::None) {
      // discard data
      socket->readAll();
      return;
   }

   if (pi->currentCommand().startsWith(QLatin1String("LIST"))) {
      while (socket->canReadLine()) {
         QUrlInfo i;
         QByteArray line = socket->readLine();

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QFtpDTP read (list): '%s'", line.constData());
#endif

         if (parseDir(line, QLatin1String(""), &i)) {
            emit listInfo(i);

         } else {
            // some FTP servers don't return a 550 if the file or directory
            // does not exist, but rather write a text to the data socket
            // -- try to catch these cases
            if (line.endsWith("No such file or directory\r\n")) {
               err = QString::fromUtf8(line);
            }
         }
      }

   } else {
      if (! is_ba && data.dev) {
         do {
            QByteArray ba;
            ba.resize(socket->bytesAvailable());
            qint64 bytesRead = socket->read(ba.data(), ba.size());

            if (bytesRead < 0) {
               // a read following a readyRead() signal will
               // never fail.
               return;
            }

            ba.resize(bytesRead);
            m_bytesDone += bytesRead;

#if defined(CS_SHOW_DEBUG_NETWORK)
            qDebug("QFtpDTP read: %lli bytes (total %lli bytes)", bytesRead, m_bytesDone);
#endif
            if (data.dev) {     // make sure it wasn't deleted in the slot
               data.dev->write(ba);
            }
            emit dataTransferProgress(m_bytesDone, m_bytesTotal);

            // Need to loop; dataTransferProgress is often connected to
            // slots that update the GUI (e.g., progress bar values), and
            // if events are processed, more data may have arrived.
         } while (socket->bytesAvailable());

      } else {
#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QFtpDTP readyRead: %lli bytes available (total %lli bytes read)", bytesAvailable(), m_bytesDone);
#endif

         emit dataTransferProgress(m_bytesDone + socket->bytesAvailable(), m_bytesTotal);
         emit readyRead();
      }
   }
}

void QFtpDTP::socketError(QAbstractSocket::SocketError errorCode)
{
   if (errorCode == QTcpSocket::HostNotFoundError) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QFtpDTP::connectState(CsHostNotFound)");
#endif

      emit connectState(QFtpDTP::CsHostNotFound);

   } else if (errorCode == QTcpSocket::ConnectionRefusedError) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QFtpDTP::connectState(CsConnectionRefused)");
#endif

      emit connectState(QFtpDTP::CsConnectionRefused);
   }
}

void QFtpDTP::socketConnectionClosed()
{
   if (! is_ba && data.dev) {
      clearData();
   }

   if (socket->isOpen()) {
      bytesFromSocket = socket->readAll();
   } else {
      bytesFromSocket.clear();
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QFtpDTP::connectState(CsClosed)");
#endif

   emit connectState(QFtpDTP::CsClosed);
}

void QFtpDTP::socketBytesWritten(qint64 bytes)
{
   m_bytesDone += bytes;

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QFtpDTP::bytesWritten(%lli)", m_bytesDone);
#endif

   emit dataTransferProgress(m_bytesDone, m_bytesTotal);
   if (callWriteData) {
      writeData();
   }
}

void QFtpDTP::setupSocket()
{
   socket = listener.nextPendingConnection();
   socket->setObjectName(QLatin1String("QFtpDTP Active state socket"));

   connect(socket, &QTcpSocket::connected,    this, &QFtpDTP::socketConnected);
   connect(socket, &QTcpSocket::readyRead,    this, &QFtpDTP::socketReadyRead);
   connect(socket, &QTcpSocket::error,        this, &QFtpDTP::socketError);
   connect(socket, &QTcpSocket::disconnected, this, &QFtpDTP::socketConnectionClosed);
   connect(socket, &QTcpSocket::bytesWritten, this, &QFtpDTP::socketBytesWritten);

   listener.close();
}

void QFtpDTP::clearData()
{
   is_ba    = false;
   data.dev = nullptr;
}

/**********************************************************************
 *
 * QFtpPI implemenatation
 *
 *********************************************************************/
QFtpPI::QFtpPI(QObject *parent)
   : QObject(parent), rawCommand(false), transferConnectionExtended(true), dtp(this),
     commandSocket(nullptr), state(Begin), abortState(None), currentCmd(QString()),
     waitForDtpToConnect(false), waitForDtpToClose(false)
{
   commandSocket.setObjectName("QFtpPI_socket");

   connect(&commandSocket, &QTcpSocket::hostFound,    this, &QFtpPI::hostFound);
   connect(&commandSocket, &QTcpSocket::connected,    this, &QFtpPI::connected);
   connect(&commandSocket, &QTcpSocket::disconnected, this, &QFtpPI::connectionClosed);
   connect(&commandSocket, &QTcpSocket::readyRead,    this, &QFtpPI::readyRead);

   connect(&commandSocket, cs_mp_cast<QAbstractSocket::SocketError>(&QTcpSocket::error),
            this, cs_mp_cast<QAbstractSocket::SocketError>(&QFtpPI::error));

   connect(&dtp,           &QFtpDTP::connectState,    this, &QFtpPI::dtpConnectState);
}

void QFtpPI::connectToHost(const QString &host, quint16 port)
{
   emit connectState(QFtp::HostLookup);

#ifndef QT_NO_BEARERMANAGEMENT
   //copy network session down to the socket & DTP
   commandSocket.setProperty("_q_networksession", property("_q_networksession"));
   dtp.setProperty("_q_networksession", property("_q_networksession"));
#endif

   commandSocket.connectToHost(host, port);
}

bool QFtpPI::sendCommands(const QStringList &cmds)
{
   if (!pendingCommands.isEmpty()) {
      return false;
   }

   if (commandSocket.state() != QTcpSocket::ConnectedState || state != Idle) {
      emit error(QFtp::NotConnected, QFtp::tr("Not connected"));
      return true; // there are no pending commands
   }

   pendingCommands = cmds;
   startNextCmd();
   return true;
}

void QFtpPI::clearPendingCommands()
{
   pendingCommands.clear();
   dtp.abortConnection();
   currentCmd.clear();
   state = Idle;
}

void QFtpPI::abort()
{
   pendingCommands.clear();

   if (abortState != None) {
      // already sent
      return;
   }

   if (currentCmd.isEmpty()) {
      return;   //no command in progress
   }

   if (currentCmd.startsWith("STOR ")) {
      abortState = AbortStarted;

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QFtpPI send: ABOR");
#endif

      commandSocket.write("ABOR\r\n", 6);
      dtp.abortConnection();

   } else {
      //Deviation from RFC 959:
      //Most FTP servers do not support ABOR, or require the telnet
      //IP & synch sequence (TCP urgent data) which is not supported by QTcpSocket.
      //Following what most FTP clients do, just reset the data connection and wait for 426

      abortState = WaitForAbortToFinish;
      dtp.abortConnection();
   }
}

void QFtpPI::hostFound()
{
   emit connectState(QFtp::Connecting);
}

void QFtpPI::connected()
{
   state = Begin;

   // try to improve performance by setting TCP_NODELAY
   commandSocket.setSocketOption(QAbstractSocket::LowDelayOption, 1);

   emit connectState(QFtp::Connected);
}

void QFtpPI::connectionClosed()
{
   commandSocket.close();
   emit connectState(QFtp::Unconnected);
}

void QFtpPI::delayedCloseFinished()
{
   emit connectState(QFtp::Unconnected);
}

void QFtpPI::error(QAbstractSocket::SocketError errorCode)
{
   if (errorCode == QTcpSocket::HostNotFoundError) {
      emit connectState(QFtp::Unconnected);
      emit error(QFtp::HostNotFound, QFtp::tr("Host %1 not found").formatArg(commandSocket.peerName()));

   } else if (errorCode == QTcpSocket::ConnectionRefusedError) {
      emit connectState(QFtp::Unconnected);
      emit error(QFtp::ConnectionRefused, QFtp::tr("Connection refused to host %1").formatArg(commandSocket.peerName()));

   } else if (errorCode == QTcpSocket::SocketTimeoutError) {
      emit connectState(QFtp::Unconnected);
      emit error(QFtp::ConnectionRefused, QFtp::tr("Connection timed out to host %1").formatArg(commandSocket.peerName()));
   }
}

void QFtpPI::readyRead()
{
   if (waitForDtpToClose) {
      return;
   }

   while (commandSocket.canReadLine()) {
      // read line with respect to line continuation
      QString line = QString::fromUtf8(commandSocket.readLine());

      if (m_replyText.isEmpty()) {
         if (line.length() < 3) {
            // protocol error
            return;
         }

         const int lowerLimit[3] = {1, 0, 0};
         const int upperLimit[3] = {5, 5, 9};

         for (int i = 0; i < 3; i++) {
            m_replyCode[i] = line[i].digitValue();

            if (m_replyCode[i] < lowerLimit[i] || m_replyCode[i] > upperLimit[i]) {
               // protocol error
               return;
            }
         }
      }

      QString endOfMultiLine;
      endOfMultiLine.append('0' + m_replyCode[0]);
      endOfMultiLine.append('0' + m_replyCode[1]);
      endOfMultiLine.append('0' + m_replyCode[2]);
      endOfMultiLine.append(' ');

      QString lineCont = endOfMultiLine.left(3);
      lineCont.append('-');

      QString lineLeft4 = line.left(4);

      while (lineLeft4 != endOfMultiLine) {
         if (lineLeft4 == lineCont) {
            m_replyText += line.mid(4);   // strip 'xyz-'

         } else {
            m_replyText += line;
         }

         if (! commandSocket.canReadLine()) {
            return;
         }

         line = QString::fromUtf8(commandSocket.readLine());
         lineLeft4 = line.left(4);
      }

      m_replyText += line.mid(4); // strip reply code 'xyz '
      if (m_replyText.endsWith("\r\n")) {
         m_replyText.chop(2);
      }

      if (processReply()) {
         m_replyText = "";
      }
   }
}

bool QFtpPI::processReply()
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   if (m_replyText.length() < 400) {
      qDebug("QFtpPI recv: %d %s", 100 * m_replyCode[0] + 10 * m_replyCode[1] + m_replyCode[2],
            m_replyText.toLatin1().constData());

   } else {
      qDebug("QFtpPI recv: %d (text skipped)", 100 * m_replyCode[0] + 10 * m_replyCode[1] + m_replyCode[2]);
   }
#endif

   int replyCodeX = 100 * m_replyCode[0] + 10 * m_replyCode[1] + m_replyCode[2];

   // process 226 replies ("Closing Data Connection") only when the data
   // connection is really closed to avoid short reads of the DTP
   if (replyCodeX == 226 || (replyCodeX == 250 && currentCmd.startsWith("RETR"))) {
      if (dtp.state() != QTcpSocket::UnconnectedState) {
         waitForDtpToClose = true;
         return false;
      }
   }

   switch (abortState) {
      case AbortStarted:
         abortState = WaitForAbortToFinish;
         break;

      case WaitForAbortToFinish:
         abortState = None;
         return true;

      default:
         break;
   }

   // get new state
   static const State table[5] = {
      /* 1yz   2yz      3yz   4yz      5yz */
      Waiting, Success, Idle, Failure, Failure
   };

   switch (state) {
      case Begin:
         if (m_replyCode[0] == 1) {
            return true;

         } else if (m_replyCode[0] == 2) {
            state = Idle;
            emit finished(QFtp::tr("Connected to host %1").formatArg(commandSocket.peerName()));
            break;
         }

         // reply codes not starting with 1 or 2 are not handled.
         return true;

      case Waiting:
         if (static_cast<signed char>(m_replyCode[0]) < 0 || m_replyCode[0] > 5) {
            state = Failure;
         } else

            if (replyCodeX == 202) {
               state = Failure;
            } else {
               state = table[m_replyCode[0] - 1];
            }

         break;

      default:
         // ignore unrequested message
         return true;
   }

   // special actions on certain replies
   emit rawFtpReply(replyCodeX, m_replyText);

   if (rawCommand) {
      rawCommand = false;

   } else if (replyCodeX == 227) {
      // 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
      // rfc959 does not define this response precisely, and gives
      // both examples where the parenthesis are used, and where
      // they are missing. We need to scan for the address and host info.

      QRegularExpression addrPortPattern("(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)");
      QRegularExpressionMatch match = addrPortPattern.match(m_replyText);

      if (! match.hasMatch()) {
         // this error should be reported

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QFtp: bad 227 response -- address and port information missing");
#endif

      } else {
         QStringList list = match.capturedTexts();

         QString host = list[1] + '.' + list[2] + '.' + list[3] + '.' + list[4];
         quint16 port = (list[5].toInteger<uint>() << 8) + list[6].toInteger<uint>();

         waitForDtpToConnect = true;
         dtp.connectToHost(host, port);
      }

   } else if (replyCodeX == 229) {
      // 229 Extended Passive mode OK (|||10982|)
      int portPos = m_replyText.indexOf('(');

      if (portPos == -1) {

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QFtp: bad 229 response, port information missing");
#endif
         // this error should be reported

      } else {
         ++portPos;
         QChar delimiter = m_replyText.at(portPos);
         QStringList epsvParameters = m_replyText.mid(portPos).split(delimiter);

         waitForDtpToConnect = true;
         dtp.connectToHost(commandSocket.peerAddress().toString(), epsvParameters.at(3).toInteger<int>());
      }

   } else if (replyCodeX == 230) {
      if (currentCmd.startsWith(QLatin1String("USER ")) && pendingCommands.count() > 0 &&
            pendingCommands.first().startsWith(QLatin1String("PASS "))) {
         // no need to send the PASS -- we are already logged in
         pendingCommands.pop_front();
      }

      // 230 User logged in, proceed.
      emit connectState(QFtp::LoggedIn);

   } else if (replyCodeX == 213) {
      // 213 File status.
      if (currentCmd.startsWith(QLatin1String("SIZE "))) {
         dtp.setBytesTotal(m_replyText.simplified().toInteger<qint64>());
      }

   } else if (m_replyCode[0] == 1 && currentCmd.startsWith("STOR ")) {
      dtp.waitForConnection();
      dtp.writeData();
   }

   // react on new state
   switch (state) {
      case Begin:
         // should never happen
         break;

      case Success:
         // success handling
         state = Idle;
         [[fallthrough]];

      case Idle:
         if (dtp.hasError()) {
            emit error(QFtp::UnknownError, dtp.errorMessage());
            dtp.clearError();
         }
         startNextCmd();
         break;

      case Waiting:
         break;

      case Failure:
         // If the EPSV or EPRT commands fail, replace them with
         // the old PASV and PORT instead and try again

         if (currentCmd.startsWith("EPSV")) {
            transferConnectionExtended = false;
            pendingCommands.prepend("PASV\r\n");

         } else if (currentCmd.startsWith("EPRT")) {
            transferConnectionExtended = false;
            pendingCommands.prepend("PORT\r\n");

         } else {
            emit error(QFtp::UnknownError, m_replyText);

         }

         if (state != Waiting) {
            state = Idle;
            startNextCmd();
         }

         break;
   }

   return true;
}

bool QFtpPI::startNextCmd()
{
   if (waitForDtpToConnect) {
      // do not process any new commands until we are connected
      return true;
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   if (state != Idle) {
      qDebug("QFtpPI startNextCmd() Internal error, QFtpPI called in non-Idle state %d", state);
   }
#endif

   if (pendingCommands.isEmpty()) {
      currentCmd.clear();
      emit finished(m_replyText);
      return false;
   }

   currentCmd = pendingCommands.first();

   // PORT and PASV are edited in-place, depending on whether we
   // should try the extended transfer connection commands EPRT and
   // EPSV. The PORT command also triggers setting up a listener, and
   // the address/port arguments are edited in.
   QHostAddress address = commandSocket.localAddress();

   if (currentCmd.startsWith(QLatin1String("PORT"))) {
      if ((address.protocol() == QTcpSocket::IPv6Protocol) && transferConnectionExtended) {
         int port = dtp.setupListener(address);
         currentCmd = "EPRT |";
         currentCmd += (address.protocol() == QTcpSocket::IPv4Protocol) ? QChar('1') : QChar('2');
         currentCmd += QChar('|') + address.toString() + QChar('|') + QString::number(port);
         currentCmd += QChar('|');

      } else if (address.protocol() == QTcpSocket::IPv4Protocol) {
         int port = dtp.setupListener(address);

         QString portArg;
         quint32 ip = address.toIPv4Address();

         portArg += QString::number((ip & 0xff000000) >> 24);
         portArg += QChar(',') + QString::number((ip & 0xff0000) >> 16);
         portArg += QChar(',') + QString::number((ip & 0xff00) >> 8);
         portArg += QChar(',') + QString::number(ip & 0xff);
         portArg += QChar(',') + QString::number((port & 0xff00) >> 8);
         portArg += QChar(',') + QString::number(port & 0xff);

         currentCmd = QLatin1String("PORT ");
         currentCmd += portArg;

      } else {
         // No IPv6 connection can be set up with the PORT command
         return false;
      }

      currentCmd += "\r\n";

   } else if (currentCmd.startsWith("PASV")) {
      if ((address.protocol() == QTcpSocket::IPv6Protocol) && transferConnectionExtended) {
         currentCmd = "EPSV\r\n";
      }
   }

   pendingCommands.pop_front();

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QFtpPI send: %s", currentCmd.left(currentCmd.length() - 2).toLatin1().constData());
#endif

   state = Waiting;
   commandSocket.write(currentCmd.toLatin1());
   return true;
}

void QFtpPI::dtpConnectState(int state)
{
   switch (state) {
      case QFtpDTP::CsClosed:
         if (waitForDtpToClose) {
            // there is an unprocessed reply
            if (processReply()) {
               m_replyText = QString("");
            } else {
               return;
            }
         }
         waitForDtpToClose = false;
         readyRead();
         return;

      case QFtpDTP::CsConnected:
         waitForDtpToConnect = false;
         startNextCmd();
         return;

      case QFtpDTP::CsHostNotFound:
      case QFtpDTP::CsConnectionRefused:
         emit error(QFtp::ConnectionRefused, QFtp::tr("Data connection refused"));
         startNextCmd();
         return;

      default:
         return;
   }
}

class QFtpPrivate
{
   Q_DECLARE_PUBLIC(QFtp)

 public:

   inline QFtpPrivate() : close_waitForStateChange(false), state(QFtp::Unconnected),
      transferMode(QFtp::Passive), error(QFtp::NoError) {
   }

   virtual ~QFtpPrivate() {
      while (!pending.isEmpty()) {
         delete pending.takeFirst();
      }
   }

   // private slots
   void _q_startNextCommand();
   void _q_piFinished(const QString &textMsg);
   void _q_piError(int errorCode, const QString &textMsg);
   void _q_piConnectState(int connectState);
   void _q_piFtpReply(int code, const QString &textMsg);

   int addCommand(QFtpCommand *cmd);

   QFtpPI pi;
   QList<QFtpCommand *> pending;
   bool close_waitForStateChange;
   QFtp::State state;
   QFtp::TransferMode transferMode;
   QFtp::Error error;
   QString errorString;

   QString host;
   quint16 port;
   QString proxyHost;
   quint16 proxyPort;

 protected:
   QFtp *q_ptr;

};

int QFtpPrivate::addCommand(QFtpCommand *cmd)
{
   pending.append(cmd);

   if (pending.count() == 1) {
      // don't emit the commandStarted() signal before the ID is returned
      QTimer::singleShot(0, q_func(), SLOT(_q_startNextCommand()));
   }
   return cmd->id;
}

QFtp::QFtp(QObject *parent)
   : QObject(parent), d_ptr(new QFtpPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QFtp);

   d->errorString = tr("Unknown error");

   connect(&d->pi,     &QFtpPI::connectState,          this, &QFtp::_q_piConnectState);
   connect(&d->pi,     &QFtpPI::finished,              this, &QFtp::_q_piFinished);

   connect(&d->pi,     cs_mp_cast<int, const QString &>(&QFtpPI::error),
         this, cs_mp_cast<int, const QString &>(&QFtp::_q_piError));

   connect(&d->pi,     &QFtpPI::rawFtpReply,           this, &QFtp::_q_piFtpReply);
   connect(&d->pi.dtp, &QFtpDTP::readyRead,            this, &QFtp::readyRead);
   connect(&d->pi.dtp, &QFtpDTP::dataTransferProgress, this, &QFtp::dataTransferProgress);
   connect(&d->pi.dtp, &QFtpDTP::listInfo,             this, &QFtp::listInfo);
}

int QFtp::connectToHost(const QString &host, quint16 port)
{
   QStringList cmds;
   cmds << host;
   cmds << QString::number((uint)port);

   int id = d_func()->addCommand(new QFtpCommand(ConnectToHost, cmds));
   d_func()->pi.transferConnectionExtended = true;

   return id;
}

int QFtp::login(const QString &user, const QString &password)
{
   QStringList cmds;
   cmds << (QLatin1String("USER ") + (user.isEmpty() ? QLatin1String("anonymous") : user) + QLatin1String("\r\n"));
   cmds << (QLatin1String("PASS ") + (password.isEmpty() ? QLatin1String("anonymous@") : password) + QLatin1String("\r\n"));
   return d_func()->addCommand(new QFtpCommand(Login, cmds));
}

int QFtp::close()
{
   return d_func()->addCommand(new QFtpCommand(Close, QStringList(QLatin1String("QUIT\r\n"))));
}

int QFtp::setTransferMode(TransferMode mode)
{
   int id = d_func()->addCommand(new QFtpCommand(SetTransferMode, QStringList()));
   d_func()->pi.transferConnectionExtended = true;
   d_func()->transferMode = mode;
   return id;
}

int QFtp::setProxy(const QString &host, quint16 port)
{
   QStringList args;
   args << host << QString::number(port);
   return d_func()->addCommand(new QFtpCommand(SetProxy, args));
}

int QFtp::list(const QString &dir)
{
   QStringList cmds;
   cmds << QLatin1String("TYPE A\r\n");
   cmds << QLatin1String(d_func()->transferMode == Passive ? "PASV\r\n" : "PORT\r\n");
   if (dir.isEmpty()) {
      cmds << QLatin1String("LIST\r\n");
   } else {
      cmds << (QLatin1String("LIST ") + dir + QLatin1String("\r\n"));
   }
   return d_func()->addCommand(new QFtpCommand(List, cmds));
}

int QFtp::cd(const QString &dir)
{
   return d_func()->addCommand(new QFtpCommand(Cd, QStringList(QLatin1String("CWD ") + dir + QLatin1String("\r\n"))));
}

int QFtp::get(const QString &file, QIODevice *dev, TransferType type)
{
   QStringList cmds;
   if (type == Binary) {
      cmds << QLatin1String("TYPE I\r\n");
   } else {
      cmds << QLatin1String("TYPE A\r\n");
   }
   cmds << QLatin1String("SIZE ") + file + QLatin1String("\r\n");
   cmds << QLatin1String(d_func()->transferMode == Passive ? "PASV\r\n" : "PORT\r\n");
   cmds << QLatin1String("RETR ") + file + QLatin1String("\r\n");
   return d_func()->addCommand(new QFtpCommand(Get, cmds, dev));
}

int QFtp::put(const QByteArray &data, const QString &file, TransferType type)
{
   QStringList cmds;
   if (type == Binary) {
      cmds << QLatin1String("TYPE I\r\n");
   } else {
      cmds << QLatin1String("TYPE A\r\n");
   }
   cmds << QLatin1String(d_func()->transferMode == Passive ? "PASV\r\n" : "PORT\r\n");
   cmds << QLatin1String("ALLO ") + QString::number(data.size()) + QLatin1String("\r\n");
   cmds << QLatin1String("STOR ") + file + QLatin1String("\r\n");
   return d_func()->addCommand(new QFtpCommand(Put, cmds, data));
}

int QFtp::put(QIODevice *dev, const QString &file, TransferType type)
{
   QStringList cmds;
   if (type == Binary) {
      cmds << QLatin1String("TYPE I\r\n");
   } else {
      cmds << QLatin1String("TYPE A\r\n");
   }
   cmds << QLatin1String(d_func()->transferMode == Passive ? "PASV\r\n" : "PORT\r\n");
   if (!dev->isSequential()) {
      cmds << QLatin1String("ALLO ") + QString::number(dev->size()) + QLatin1String("\r\n");
   }
   cmds << QLatin1String("STOR ") + file + QLatin1String("\r\n");
   return d_func()->addCommand(new QFtpCommand(Put, cmds, dev));
}

int QFtp::remove(const QString &file)
{
   return d_func()->addCommand(new QFtpCommand(Remove, QStringList(QLatin1String("DELE ") + file + QLatin1String("\r\n"))));
}

int QFtp::mkdir(const QString &dir)
{
   return d_func()->addCommand(new QFtpCommand(Mkdir, QStringList(QLatin1String("MKD ") + dir + QLatin1String("\r\n"))));
}

int QFtp::rmdir(const QString &dir)
{
   return d_func()->addCommand(new QFtpCommand(Rmdir, QStringList(QLatin1String("RMD ") + dir + QLatin1String("\r\n"))));
}

int QFtp::rename(const QString &oldname, const QString &newname)
{
   QStringList cmds;
   cmds << QLatin1String("RNFR ") + oldname + QLatin1String("\r\n");
   cmds << QLatin1String("RNTO ") + newname + QLatin1String("\r\n");
   return d_func()->addCommand(new QFtpCommand(Rename, cmds));
}

int QFtp::rawCommand(const QString &command)
{
   QString cmd = command.trimmed() + QLatin1String("\r\n");
   return d_func()->addCommand(new QFtpCommand(RawCommand, QStringList(cmd)));
}

qint64 QFtp::bytesAvailable() const
{
   return d_func()->pi.dtp.bytesAvailable();
}

qint64 QFtp::read(char *data, qint64 maxlen)
{
   return d_func()->pi.dtp.read(data, maxlen);
}

QByteArray QFtp::readAll()
{
   return d_func()->pi.dtp.readAll();
}

void QFtp::abort()
{
   if (d_func()->pending.isEmpty()) {
      return;
   }

   clearPendingCommands();
   d_func()->pi.abort();
}

int QFtp::currentId() const
{
   if (d_func()->pending.isEmpty()) {
      return 0;
   }
   return d_func()->pending.first()->id;
}

QFtp::Command QFtp::currentCommand() const
{
   if (d_func()->pending.isEmpty()) {
      return None;
   }

   return d_func()->pending.first()->command;
}

QIODevice *QFtp::currentDevice() const
{
   if (d_func()->pending.isEmpty()) {
      return nullptr;
   }

   QFtpCommand *c = d_func()->pending.first();
   if (c->is_ba) {
      return nullptr;
   }

   return c->data.dev;
}

bool QFtp::hasPendingCommands() const
{
   return d_func()->pending.count() > 1;
}

void QFtp::clearPendingCommands()
{
   // delete all entires except the first one
   while (d_func()->pending.count() > 1) {
      delete d_func()->pending.takeLast();
   }
}

QFtp::State QFtp::state() const
{
   return d_func()->state;
}

QFtp::Error QFtp::error() const
{
   return d_func()->error;
}

QString QFtp::errorString() const
{
   return d_func()->errorString;
}

void QFtpPrivate::_q_startNextCommand()
{
   Q_Q(QFtp);
   if (pending.isEmpty()) {
      return;
   }
   QFtpCommand *c = pending.first();

   error = QFtp::NoError;
   errorString = cs_mark_tr("QFtp", "Unknown error");

   if (q->bytesAvailable()) {
      q->readAll();   // clear the data
   }
   emit q->commandStarted(c->id);

   // Proxy support, replace the Login argument in place
   if (c->command == QFtp::Login && ! proxyHost.isEmpty()) {
      QString loginString = c->rawCmds.first().trimmed();
      loginString += QChar('@') + host;

      if (port && port != 21) {
         loginString += QChar(':') + QString::number(port);
      }

      loginString.append("\r\n");
      c->rawCmds[0] = loginString;
   }

   if (c->command == QFtp::SetTransferMode) {
      _q_piFinished("Transfer mode set");

   } else if (c->command == QFtp::SetProxy) {
      proxyHost = c->rawCmds[0];
      proxyPort = c->rawCmds[1].toInteger<uint>();

      c->rawCmds.clear();
      _q_piFinished("Proxy set to " + proxyHost + ':' + QString::number(proxyPort));

   } else if (c->command == QFtp::ConnectToHost) {

#ifndef QT_NO_BEARERMANAGEMENT
      //copy network session down to the PI
      pi.setProperty("_q_networksession", q->property("_q_networksession"));
#endif

      if (!proxyHost.isEmpty()) {
         host = c->rawCmds[0];
         port = c->rawCmds[1].toInteger<uint>();
         pi.connectToHost(proxyHost, proxyPort);

      } else {
         pi.connectToHost(c->rawCmds[0], c->rawCmds[1].toInteger<uint>());
      }

   } else {
      if (c->command == QFtp::Put) {
         if (c->is_ba) {
            pi.dtp.setData(c->data.ba);
            pi.dtp.setBytesTotal(c->data.ba->size());

         } else if (c->data.dev && (c->data.dev->isOpen() || c->data.dev->open(QIODevice::ReadOnly))) {
            pi.dtp.setDevice(c->data.dev);

            if (c->data.dev->isSequential()) {
               pi.dtp.setBytesTotal(0);

               pi.dtp.connect(c->data.dev, &QIODevice::readyRead,           &pi.dtp, &QFtpDTP::dataReadyRead);
               pi.dtp.connect(c->data.dev, &QIODevice::readChannelFinished, &pi.dtp, &QFtpDTP::dataReadyRead);

            } else {
               pi.dtp.setBytesTotal(c->data.dev->size());
            }
         }

      } else if (c->command == QFtp::Get) {
         if (!c->is_ba && c->data.dev) {
            pi.dtp.setDevice(c->data.dev);
         }

      } else if (c->command == QFtp::Close) {
         state = QFtp::Closing;
         emit q->stateChanged(state);
      }
      pi.sendCommands(c->rawCmds);
   }
}

void QFtpPrivate::_q_piFinished(const QString &textMsg)
{
   (void) textMsg;

   if (pending.isEmpty()) {
      return;
   }

   QFtpCommand *c = pending.first();

   if (c->command == QFtp::Close) {
      // The order of in which the slots are called is arbitrary, so
      // disconnect the SIGNAL-SIGNAL temporary to make sure that we
      // don't get the commandFinished() signal before the stateChanged() signal

      if (state != QFtp::Unconnected) {
         close_waitForStateChange = true;
         return;
      }
   }

   emit q_func()->commandFinished(c->id, false);
   pending.removeFirst();

   delete c;

   if (pending.isEmpty()) {
      emit q_func()->done(false);
   } else {
      _q_startNextCommand();
   }
}

void QFtpPrivate::_q_piError(int errorCode, const QString &textMsg)
{
   Q_Q(QFtp);

   if (pending.isEmpty()) {
      qWarning("QFtp::_q_piError() Called without pending command");
      return;
   }

   QFtpCommand *c = pending.first();

   // non-fatal errors
   if (c->command == QFtp::Get && pi.currentCommand().startsWith("SIZE ")) {
      pi.dtp.setBytesTotal(0);
      return;

   } else if (c->command == QFtp::Put && pi.currentCommand().startsWith("ALLO ")) {
      return;
   }

   error = QFtp::Error(errorCode);

   switch (q->currentCommand()) {
      case QFtp::ConnectToHost:
         errorString = QString::fromLatin1(cs_mark_tr("QFtp", "Connecting to host failed:\n%1")).formatArg(textMsg);
         break;

      case QFtp::Login:
         errorString = QString::fromLatin1(cs_mark_tr("QFtp", "Login failed:\n%1")).formatArg(textMsg);
         break;

      case QFtp::List:
         errorString = QString::fromLatin1(cs_mark_tr("QFtp", "Listing directory failed:\n%1")).formatArg(textMsg);
         break;

      case QFtp::Cd:
         errorString = QString::fromLatin1(cs_mark_tr("QFtp", "Changing directory failed:\n%1")).formatArg(textMsg);
         break;

      case QFtp::Get:
         errorString = QString::fromLatin1(cs_mark_tr("QFtp", "Downloading file failed:\n%1")).formatArg(textMsg);
         break;

      case QFtp::Put:
         errorString = QString::fromLatin1(cs_mark_tr("QFtp", "Uploading file failed:\n%1")).formatArg(textMsg);
         break;

      case QFtp::Remove:
         errorString = QString::fromLatin1(cs_mark_tr("QFtp", "Removing file failed:\n%1")).formatArg(textMsg);
         break;

      case QFtp::Mkdir:
         errorString = QString::fromLatin1(cs_mark_tr("QFtp", "Creating directory failed:\n%1")).formatArg(textMsg);
         break;

      case QFtp::Rmdir:
         errorString = QString::fromLatin1(cs_mark_tr("QFtp", "Removing directory failed:\n%1")).formatArg(textMsg);
         break;

      default:
         errorString = textMsg;
         break;
   }

   pi.clearPendingCommands();
   q->clearPendingCommands();
   emit q->commandFinished(c->id, true);

   pending.removeFirst();
   delete c;

   if (pending.isEmpty()) {
      emit q->done(true);
   } else {
      _q_startNextCommand();
   }
}

void QFtpPrivate::_q_piConnectState(int connectState)
{
   state = QFtp::State(connectState);
   emit q_func()->stateChanged(state);

   if (close_waitForStateChange) {
      close_waitForStateChange = false;
      _q_piFinished(QString::fromLatin1(cs_mark_tr("QFtp", "Connection closed")));
   }
}

void QFtpPrivate::_q_piFtpReply(int code, const QString &textMsg)
{
   if (q_func()->currentCommand() == QFtp::RawCommand) {
      pi.rawCommand = true;
      emit q_func()->rawCommandReply(code, textMsg);
   }
}

QFtp::~QFtp()
{
   abort();
   close();
}

void QFtp::_q_startNextCommand()
{
   Q_D(QFtp);
   d->_q_startNextCommand();
}

void QFtp::_q_piFinished(const QString &textMsg)
{
   Q_D(QFtp);
   d->_q_piFinished(textMsg);
}

void QFtp::_q_piError(int errorCode, const QString &textMsg)
{
   Q_D(QFtp);
   d->_q_piError(errorCode, textMsg);
}

void QFtp::_q_piConnectState(int connectState)
{
   Q_D(QFtp);
   d->_q_piConnectState(connectState);
}

void QFtp::_q_piFtpReply(int code, const QString &textMsg)
{
   Q_D(QFtp);
   d->_q_piFtpReply(code, textMsg);
}

#endif // QT_NO_FTP
