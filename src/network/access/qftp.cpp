/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

//#define QFTPPI_DEBUG
//#define QFTPDTP_DEBUG

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

   NET_CS_SIGNAL_1(Public, void listInfo(const QUrlInfo &un_named_arg1))
   NET_CS_SIGNAL_2(listInfo, un_named_arg1)

   NET_CS_SIGNAL_1(Public, void readyRead())
   NET_CS_SIGNAL_2(readyRead)

   NET_CS_SIGNAL_1(Public, void dataTransferProgress(qint64 un_named_arg1, qint64 un_named_arg2))
   NET_CS_SIGNAL_2(dataTransferProgress, un_named_arg1, un_named_arg2)

   NET_CS_SIGNAL_1(Public, void connectState(int un_named_arg1))
   NET_CS_SIGNAL_2(connectState, un_named_arg1)

 private :
   NET_CS_SLOT_1(Private, void socketConnected())
   NET_CS_SLOT_2(socketConnected)

   NET_CS_SLOT_1(Private, void socketReadyRead())
   NET_CS_SLOT_2(socketReadyRead)

   NET_CS_SLOT_1(Private, void socketError(QAbstractSocket::SocketError un_named_arg1))
   NET_CS_SLOT_2(socketError)

   NET_CS_SLOT_1(Private, void socketConnectionClosed())
   NET_CS_SLOT_2(socketConnectionClosed)

   NET_CS_SLOT_1(Private, void socketBytesWritten(qint64 un_named_arg1))
   NET_CS_SLOT_2(socketBytesWritten)

   NET_CS_SLOT_1(Private, void setupSocket())
   NET_CS_SLOT_2(setupSocket)

   NET_CS_SLOT_1(Private, void dataReadyRead())
   NET_CS_SLOT_2(dataReadyRead)

   void clearData();

   QTcpSocket *socket;
   QTcpServer listener;

   QFtpPI *pi;
   QString err;
   qint64 bytesDone;
   qint64 bytesTotal;
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

   QFtpDTP dtp; // the PI has a DTP which is not the design of RFC 959, but it
   // makes the design simpler this way

   NET_CS_SIGNAL_1(Public, void connectState(int un_named_arg1))
   NET_CS_SIGNAL_2(connectState, un_named_arg1)

   NET_CS_SIGNAL_1(Public, void finished(const QString &un_named_arg1))
   NET_CS_SIGNAL_2(finished, un_named_arg1)

   NET_CS_SIGNAL_1(Public, void error(int un_named_arg1, const QString &un_named_arg2))
   NET_CS_SIGNAL_OVERLOAD(error, (int, const QString &), un_named_arg1, un_named_arg2)

   NET_CS_SIGNAL_1(Public, void rawFtpReply(int un_named_arg1, const QString &un_named_arg2))
   NET_CS_SIGNAL_2(rawFtpReply, un_named_arg1, un_named_arg2)

 private :
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

   NET_CS_SLOT_1(Private, void error(QAbstractSocket::SocketError un_named_arg1))
   NET_CS_SLOT_OVERLOAD(error, (QAbstractSocket::SocketError))

   NET_CS_SLOT_1(Private, void dtpConnectState(int un_named_arg1))
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
   QString replyText;
   char replyCode[3];
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
   QFtpCommand(QFtp::Command cmd, const QStringList &raw, QIODevice *dev = 0);
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
   : QObject(parent), socket(0), listener(this), pi(p), callWriteData(false)
{
   clearData();
   listener.setObjectName(QLatin1String("QFtpDTP active state server"));
   connect(&listener, SIGNAL(newConnection()), this, SLOT(setupSocket()));
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
   bytesTotal = bytes;
   bytesDone = 0;
   emit dataTransferProgress(bytesDone, bytesTotal);
}

void QFtpDTP::connectToHost(const QString &host, quint16 port)
{
   bytesFromSocket.clear();

   if (socket) {
      delete socket;
      socket = 0;
   }
   socket = new QTcpSocket(this);

#ifndef QT_NO_BEARERMANAGEMENT
   // copy network session down to the socket
   socket->setProperty("_q_networksession", property("_q_networksession"));
#endif

   socket->setObjectName("QFtpDTP Passive state socket");

   connect(socket, SIGNAL(connected()),          this,   SLOT(socketConnected()));
   connect(socket, SIGNAL(readyRead()),          this,   SLOT(socketReadyRead()));

   connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this,   SLOT(socketError(QAbstractSocket::SocketError)));

   connect(socket, SIGNAL(disconnected()),       this,   SLOT(socketConnectionClosed()));
   connect(socket, SIGNAL(bytesWritten(qint64)), this,   SLOT(socketBytesWritten(qint64)));

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

   bytesDone += read;
   return read;
}

QByteArray QFtpDTP::readAll()
{
   QByteArray tmp;
   if (socket && socket->state() == QTcpSocket::ConnectedState) {
      tmp = socket->readAll();
      bytesDone += tmp.size();
   } else {
      tmp = bytesFromSocket;
      bytesFromSocket.clear();
   }
   return tmp;
}

void QFtpDTP::writeData()
{
   if (!socket) {
      return;
   }

   if (is_ba) {

#if defined(QFTPDTP_DEBUG)
      qDebug("QFtpDTP::writeData: write %d bytes", data.ba->size());
#endif

      if (data.ba->size() == 0) {
         emit dataTransferProgress(0, bytesTotal);
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

#if defined(QFTPDTP_DEBUG)
      qDebug("QFtpDTP::writeData: write() of size %lli bytes", read);
#endif

      if (read > 0) {
         socket->write(buf, read);
      } else if (read == -1 || (!data.dev->isSequential() && data.dev->atEnd())) {
         // error or EOF
         if (bytesDone == 0 && socket->bytesToWrite() == 0) {
            emit dataTransferProgress(0, bytesTotal);
         }
         socket->close();
         clearData();
      }

      // do we continue uploading?
      callWriteData = data.dev != 0;
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
#if defined(QFTPDTP_DEBUG)
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

#ifndef QT_NO_DATESTRING
   do {
      dateTime = QLocale::c().toDateTime(dateString, formats.at(n++));
   }  while (n < formats.size() && (!dateTime.isValid()));
#endif

   if (n == 2 || n == 4) {
      // Guess the year.
      dateTime.setDate(QDate(QDate::currentDate().year(),
                             dateTime.date().month(),
                             dateTime.date().day()));
      _q_fixupDateTime(&dateTime);
   }

   if (dateTime.isValid()) {
      info->setLastModified(dateTime);
   }

   // Resolve permissions
   int permissions = 0;
   QString p = tokens.at(2);
   permissions |= (p[0] == QLatin1Char('r') ? QUrlInfo::ReadOwner : 0);
   permissions |= (p[1] == QLatin1Char('w') ? QUrlInfo::WriteOwner : 0);
   permissions |= (p[2] == QLatin1Char('x') ? QUrlInfo::ExeOwner : 0);
   permissions |= (p[3] == QLatin1Char('r') ? QUrlInfo::ReadGroup : 0);
   permissions |= (p[4] == QLatin1Char('w') ? QUrlInfo::WriteGroup : 0);
   permissions |= (p[5] == QLatin1Char('x') ? QUrlInfo::ExeGroup : 0);
   permissions |= (p[6] == QLatin1Char('r') ? QUrlInfo::ReadOther : 0);
   permissions |= (p[7] == QLatin1Char('w') ? QUrlInfo::WriteOther : 0);
   permissions |= (p[8] == QLatin1Char('x') ? QUrlInfo::ExeOther : 0);
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

   Q_UNUSED(userName);

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
   int extIndex = name.lastIndexOf(QLatin1Char('.'));

   if (extIndex != -1) {
      ext = name.mid(extIndex + 1);
   }
   if (ext == QLatin1String("exe") || ext == QLatin1String("bat") || ext == QLatin1String("com")) {
      permissions |= QUrlInfo::ExeOwner | QUrlInfo::ExeGroup | QUrlInfo::ExeOther;
   }

   info->setPermissions(permissions);

   info->setReadable(true);
   info->setWritable(info->isFile());

   QDateTime dateTime;
#ifndef QT_NO_DATESTRING
   dateTime = QLocale::c().toDateTime(tokens.at(1), QLatin1String("MM-dd-yy  hh:mmAP"));
   if (dateTime.date().year() < 1971) {
      dateTime.setDate(QDate(dateTime.date().year() + 100,
                             dateTime.date().month(),
                             dateTime.date().day()));
   }
#endif

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

   if (unixMatch.capturedStart(0) == bufferStr.begin()) {
      _q_parseUnixDir(unixMatch.capturedTexts(), userName, info);
      return true;
   }

   // DOS style FTP servers
   QRegularExpression dosPattern("^(\\d\\d-\\d\\d-\\d\\d\\ \\ \\d\\d:\\d\\d[AP]M)\\s+(<DIR>|\\d+)\\s+(\\S.*)$");
   QRegularExpressionMatch dosMatch = dosPattern.match(bufferStr);

   if (dosMatch.capturedStart(0) == bufferStr.begin()) {
      _q_parseDosDir(dosMatch.capturedTexts(), userName, info);
      return true;
   }

   // unsupported
   return false;
}

void QFtpDTP::socketConnected()
{
   bytesDone = 0;

#if defined(QFTPDTP_DEBUG)
   qDebug("QFtpDTP::connectState(CsConnected)");
#endif

   emit connectState(QFtpDTP::CsConnected);
}

void QFtpDTP::socketReadyRead()
{
   if (!socket) {
      return;
   }

   if (pi->currentCommand().isEmpty()) {
      socket->close();

#if defined(QFTPDTP_DEBUG)
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
#if defined(QFTPDTP_DEBUG)
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
      if (!is_ba && data.dev) {
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
            bytesDone += bytesRead;
#if defined(QFTPDTP_DEBUG)
            qDebug("QFtpDTP read: %lli bytes (total %lli bytes)", bytesRead, bytesDone);
#endif
            if (data.dev) {     // make sure it wasn't deleted in the slot
               data.dev->write(ba);
            }
            emit dataTransferProgress(bytesDone, bytesTotal);

            // Need to loop; dataTransferProgress is often connected to
            // slots that update the GUI (e.g., progress bar values), and
            // if events are processed, more data may have arrived.
         } while (socket->bytesAvailable());
      } else {
#if defined(QFTPDTP_DEBUG)
         qDebug("QFtpDTP readyRead: %lli bytes available (total %lli bytes read)",
                bytesAvailable(), bytesDone);
#endif
         emit dataTransferProgress(bytesDone + socket->bytesAvailable(), bytesTotal);
         emit readyRead();
      }
   }
}

void QFtpDTP::socketError(QAbstractSocket::SocketError e)
{
   if (e == QTcpSocket::HostNotFoundError) {
#if defined(QFTPDTP_DEBUG)
      qDebug("QFtpDTP::connectState(CsHostNotFound)");
#endif
      emit connectState(QFtpDTP::CsHostNotFound);
   } else if (e == QTcpSocket::ConnectionRefusedError) {
#if defined(QFTPDTP_DEBUG)
      qDebug("QFtpDTP::connectState(CsConnectionRefused)");
#endif
      emit connectState(QFtpDTP::CsConnectionRefused);
   }
}

void QFtpDTP::socketConnectionClosed()
{
   if (!is_ba && data.dev) {
      clearData();
   }

   if (socket->isOpen()) {
      bytesFromSocket = socket->readAll();
   } else {
      bytesFromSocket.clear();
   }

#if defined(QFTPDTP_DEBUG)
   qDebug("QFtpDTP::connectState(CsClosed)");
#endif
   emit connectState(QFtpDTP::CsClosed);
}

void QFtpDTP::socketBytesWritten(qint64 bytes)
{
   bytesDone += bytes;

#if defined(QFTPDTP_DEBUG)
   qDebug("QFtpDTP::bytesWritten(%lli)", bytesDone);
#endif

   emit dataTransferProgress(bytesDone, bytesTotal);
   if (callWriteData) {
      writeData();
   }
}

void QFtpDTP::setupSocket()
{
   socket = listener.nextPendingConnection();
   socket->setObjectName(QLatin1String("QFtpDTP Active state socket"));

   connect(socket, SIGNAL(connected()),          this,  SLOT(socketConnected()));
   connect(socket, SIGNAL(readyRead()),          this,  SLOT(socketReadyRead()));

   connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this,    SLOT(socketError(QAbstractSocket::SocketError)));

   connect(socket, SIGNAL(disconnected()),       this,  SLOT(socketConnectionClosed()));
   connect(socket, SIGNAL(bytesWritten(qint64)), this,  SLOT(socketBytesWritten(qint64)));

   listener.close();
}

void QFtpDTP::clearData()
{
   is_ba = false;
   data.dev = 0;
}

/**********************************************************************
 *
 * QFtpPI implemenatation
 *
 *********************************************************************/
QFtpPI::QFtpPI(QObject *parent) :
   QObject(parent),
   rawCommand(false),
   transferConnectionExtended(true),
   dtp(this),
   commandSocket(0),
   state(Begin), abortState(None),
   currentCmd(QString()),
   waitForDtpToConnect(false),
   waitForDtpToClose(false)
{
   commandSocket.setObjectName("QFtpPI_socket");
   connect(&commandSocket, SIGNAL(hostFound()),     this, SLOT(hostFound()));
   connect(&commandSocket, SIGNAL(connected()),     this, SLOT(connected()));
   connect(&commandSocket, SIGNAL(disconnected()),  this, SLOT(connectionClosed()));
   connect(&commandSocket, SIGNAL(readyRead()),     this, SLOT(readyRead()));

   connect(&commandSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
           SLOT(error(QAbstractSocket::SocketError)));

   connect(&dtp, SIGNAL(connectState(int)),          this, SLOT(dtpConnectState(int)));
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

/*
  Sends the sequence of commands \a cmds to the FTP server. When the commands
  are all done the finished() signal is emitted. When an error occurs, the
  error() signal is emitted.

  If there are pending commands in the queue this functions returns false and
  the \a cmds are not added to the queue; otherwise it returns true.
*/
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

   if (currentCmd.startsWith(QLatin1String("STOR "))) {
      abortState = AbortStarted;
#if defined(QFTPPI_DEBUG)
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

#if defined(QFTPPI_DEBUG)
   //    qDebug("QFtpPI state: %d [connected()]", state);
#endif

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

void QFtpPI::error(QAbstractSocket::SocketError e)
{
   if (e == QTcpSocket::HostNotFoundError) {
      emit connectState(QFtp::Unconnected);
      emit error(QFtp::HostNotFound, QFtp::tr("Host %1 not found").formatArg(commandSocket.peerName()));

   } else if (e == QTcpSocket::ConnectionRefusedError) {
      emit connectState(QFtp::Unconnected);
      emit error(QFtp::ConnectionRefused, QFtp::tr("Connection refused to host %1").formatArg(commandSocket.peerName()));

   } else if (e == QTcpSocket::SocketTimeoutError) {
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

      if (replyText.isEmpty()) {
         if (line.length() < 3) {
            // protocol error
            return;
         }

         const int lowerLimit[3] = {1, 0, 0};
         const int upperLimit[3] = {5, 5, 9};

         for (int i = 0; i < 3; i++) {
            replyCode[i] = line[i].digitValue();

            if (replyCode[i] < lowerLimit[i] || replyCode[i] > upperLimit[i]) {
               // protocol error
               return;
            }
         }
      }

      QString endOfMultiLine;
      endOfMultiLine.append('0' + replyCode[0]);
      endOfMultiLine.append('0' + replyCode[1]);
      endOfMultiLine.append('0' + replyCode[2]);
      endOfMultiLine.append(' ');

      QString lineCont = endOfMultiLine.left(3);
      lineCont.append('-');

      QString lineLeft4 = line.left(4);

      while (lineLeft4 != endOfMultiLine) {
         if (lineLeft4 == lineCont) {
            replyText += line.mid(4);   // strip 'xyz-'

         } else {
            replyText += line;
         }

         if (! commandSocket.canReadLine()) {
            return;
         }

         line = QString::fromUtf8(commandSocket.readLine());
         lineLeft4 = line.left(4);
      }

      replyText += line.mid(4); // strip reply code 'xyz '
      if (replyText.endsWith("\r\n")) {
         replyText.chop(2);
      }

      if (processReply()) {
         replyText = "";
      }
   }
}

/*
  Process a reply from the FTP server.

  Returns true if the reply was processed or false if the reply has to be
  processed at a later point.
*/
bool QFtpPI::processReply()
{
#if defined(QFTPPI_DEBUG)
   //    qDebug("QFtpPI state: %d [processReply() begin]", state);
   if (replyText.length() < 400) {
      qDebug("QFtpPI recv: %d %s", 100 * replyCode[0] + 10 * replyCode[1] + replyCode[2], replyText.toLatin1().constData());
   } else {
      qDebug("QFtpPI recv: %d (text skipped)", 100 * replyCode[0] + 10 * replyCode[1] + replyCode[2]);
   }
#endif

   int replyCodeInt = 100 * replyCode[0] + 10 * replyCode[1] + replyCode[2];

   // process 226 replies ("Closing Data Connection") only when the data
   // connection is really closed to avoid short reads of the DTP
   if (replyCodeInt == 226 || (replyCodeInt == 250 && currentCmd.startsWith(QLatin1String("RETR")))) {
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
         if (replyCode[0] == 1) {
            return true;

         } else if (replyCode[0] == 2) {
            state = Idle;
            emit finished(QFtp::tr("Connected to host %1").formatArg(commandSocket.peerName()));
            break;
         }

         // reply codes not starting with 1 or 2 are not handled.
         return true;

      case Waiting:
         if (static_cast<signed char>(replyCode[0]) < 0 || replyCode[0] > 5) {
            state = Failure;
         } else

            if (replyCodeInt == 202) {
               state = Failure;
            } else {
               state = table[replyCode[0] - 1];
            }

         break;

      default:
         // ignore unrequested message
         return true;
   }
#if defined(QFTPPI_DEBUG)
   //    qDebug("QFtpPI state: %d [processReply() intermediate]", state);
#endif

   // special actions on certain replies
   emit rawFtpReply(replyCodeInt, replyText);

   if (rawCommand) {
      rawCommand = false;

   } else if (replyCodeInt == 227) {
      // 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
      // rfc959 does not define this response precisely, and gives
      // both examples where the parenthesis are used, and where
      // they are missing. We need to scan for the address and host info.

      QRegularExpression addrPortPattern("(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)");
      QRegularExpressionMatch match = addrPortPattern.match(replyText);

      if (! match.hasMatch()) {
         // this error should be reported

#if defined(QFTPPI_DEBUG)
         qDebug("QFtp: bad 227 response -- address and port information missing");
#endif

      } else {
         QStringList list = match.capturedTexts();

         QString host = list[1] + '.' + list[2] + '.' + list[3] + '.' + list[4];
         quint16 port = (list[5].toInteger<uint>() << 8) + list[6].toInteger<uint>();

         waitForDtpToConnect = true;
         dtp.connectToHost(host, port);
      }

   } else if (replyCodeInt == 229) {
      // 229 Extended Passive mode OK (|||10982|)
      int portPos = replyText.indexOf('(');

      if (portPos == -1) {

#if defined(QFTPPI_DEBUG)
         qDebug("QFtp: bad 229 response -- port information missing");
#endif
         // this error should be reported

      } else {
         ++portPos;
         QChar delimiter = replyText.at(portPos);
         QStringList epsvParameters = replyText.mid(portPos).split(delimiter);

         waitForDtpToConnect = true;
         dtp.connectToHost(commandSocket.peerAddress().toString(), epsvParameters.at(3).toInteger<int>());
      }

   } else if (replyCodeInt == 230) {
      if (currentCmd.startsWith(QLatin1String("USER ")) && pendingCommands.count() > 0 &&
            pendingCommands.first().startsWith(QLatin1String("PASS "))) {
         // no need to send the PASS -- we are already logged in
         pendingCommands.pop_front();
      }
      // 230 User logged in, proceed.
      emit connectState(QFtp::LoggedIn);

   } else if (replyCodeInt == 213) {
      // 213 File status.
      if (currentCmd.startsWith(QLatin1String("SIZE "))) {
         dtp.setBytesTotal(replyText.simplified().toInteger<qint64>());
      }

   } else if (replyCode[0] == 1 && currentCmd.startsWith(QLatin1String("STOR "))) {
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
         // the old PASV and PORT instead and try again.
         if (currentCmd.startsWith(QLatin1String("EPSV"))) {
            transferConnectionExtended = false;
            pendingCommands.prepend(QLatin1String("PASV\r\n"));

         } else if (currentCmd.startsWith(QLatin1String("EPRT"))) {
            transferConnectionExtended = false;
            pendingCommands.prepend(QLatin1String("PORT\r\n"));

         } else {
            emit error(QFtp::UnknownError, replyText);
         }

         if (state != Waiting) {
            state = Idle;
            startNextCmd();
         }

         break;
   }

#if defined(QFTPPI_DEBUG)
   //    qDebug("QFtpPI state: %d [processReply() end]", state);
#endif

   return true;
}

/*
  Starts next pending command. Returns false if there are no pending commands,
  otherwise it returns true.
*/
bool QFtpPI::startNextCmd()
{
   if (waitForDtpToConnect) {
      // don't process any new commands until we are connected
      return true;
   }

#if defined(QFTPPI_DEBUG)
   if (state != Idle) {
      qDebug("QFtpPI startNextCmd: Internal error, QFtpPI called in non-Idle state %d", state);
   }
#endif

   if (pendingCommands.isEmpty()) {
      currentCmd.clear();
      emit finished(replyText);
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
         currentCmd = QLatin1String("EPRT |");
         currentCmd += (address.protocol() == QTcpSocket::IPv4Protocol) ? QLatin1Char('1') : QLatin1Char('2');
         currentCmd += QLatin1Char('|') + address.toString() + QLatin1Char('|') + QString::number(port);
         currentCmd += QLatin1Char('|');

      } else if (address.protocol() == QTcpSocket::IPv4Protocol) {
         int port = dtp.setupListener(address);
         QString portArg;
         quint32 ip = address.toIPv4Address();
         portArg += QString::number((ip & 0xff000000) >> 24);
         portArg += QLatin1Char(',') + QString::number((ip & 0xff0000) >> 16);
         portArg += QLatin1Char(',') + QString::number((ip & 0xff00) >> 8);
         portArg += QLatin1Char(',') + QString::number(ip & 0xff);
         portArg += QLatin1Char(',') + QString::number((port & 0xff00) >> 8);
         portArg += QLatin1Char(',') + QString::number(port & 0xff);

         currentCmd = QLatin1String("PORT ");
         currentCmd += portArg;

      } else {
         // No IPv6 connection can be set up with the PORT command
         return false;
      }

      currentCmd += QLatin1String("\r\n");

   } else if (currentCmd.startsWith(QLatin1String("PASV"))) {
      if ((address.protocol() == QTcpSocket::IPv6Protocol) && transferConnectionExtended) {
         currentCmd = QLatin1String("EPSV\r\n");
      }
   }

   pendingCommands.pop_front();

#if defined(QFTPPI_DEBUG)
   qDebug("QFtpPI send: %s", currentCmd.left(currentCmd.length() - 2).toLatin1().constData());
#endif

   state = Waiting;
   commandSocket.write(currentCmd.toLatin1());
   return true;
}

void QFtpPI::dtpConnectState(int s)
{
   switch (s) {
      case QFtpDTP::CsClosed:
         if (waitForDtpToClose) {
            // there is an unprocessed reply
            if (processReply()) {
               replyText = QLatin1String("");
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
         emit error(QFtp::ConnectionRefused,
                    QFtp::tr("Data connection refused"));
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
   void _q_piFinished(const QString &);
   void _q_piError(int, const QString &);
   void _q_piConnectState(int);
   void _q_piFtpReply(int, const QString &);

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


/*!
    \class QFtp
    \brief The QFtp class provides an implementation of the client side of FTP protocol.

    \ingroup network
    \inmodule QtNetwork


    This class provides a direct interface to FTP that allows you to
    have more control over the requests. However, for new
    applications, it is recommended to use QNetworkAccessManager and
    QNetworkReply, as those classes possess a simpler, yet more
    powerful API.

    The class works asynchronously, so there are no blocking
    functions. If an operation cannot be executed immediately, the
    function will still return straight away and the operation will be
    scheduled for later execution. The results of scheduled operations
    are reported via signals. This approach depends on the event loop
    being in operation.

    The operations that can be scheduled (they are called "commands"
    in the rest of the documentation) are the following:
    connectToHost(), login(), close(), list(), cd(), get(), put(),
    remove(), mkdir(), rmdir(), rename() and rawCommand().

    All of these commands return a unique identifier that allows you
    to keep track of the command that is currently being executed.
    When the execution of a command starts, the commandStarted()
    signal with the command's identifier is emitted. When the command
    is finished, the commandFinished() signal is emitted with the
    command's identifier and a bool that indicates whether the command
    finished with an error.

    In some cases, you might want to execute a sequence of commands,
    e.g. if you want to connect and login to a FTP server. This is
    simply achieved:

    \snippet doc/src/snippets/code/src_network_access_qftp.cpp 0

    In this case two FTP commands have been scheduled. When the last
    scheduled command has finished, a done() signal is emitted with
    a bool argument that tells you whether the sequence finished with
    an error.

    If an error occurs during the execution of one of the commands in
    a sequence of commands, all the pending commands (i.e. scheduled,
    but not yet executed commands) are cleared and no signals are
    emitted for them.

    Some commands, e.g. list(), emit additional signals to report
    their results.

    Example: If you want to download the INSTALL file from the Qt
    FTP server, you would write this:

    \snippet doc/src/snippets/code/src_network_access_qftp.cpp 1

    For this example the following sequence of signals is emitted
    (with small variations, depending on network traffic, etc.):

    \snippet doc/src/snippets/code/src_network_access_qftp.cpp 2

    The dataTransferProgress() signal in the above example is useful
    if you want to show a \link QProgressBar progress bar \endlink to
    inform the user about the progress of the download. The
    readyRead() signal tells you that there is data ready to be read.
    The amount of data can be queried then with the bytesAvailable()
    function and it can be read with the read() or readAll()
    function.

    If the login fails for the above example, the signals would look
    like this:

    \snippet doc/src/snippets/code/src_network_access_qftp.cpp 3

    You can then get details about the error with the error() and
    errorString() functions.

    For file transfer, QFtp can use both active or passive mode, and
    it uses passive file transfer mode by default; see the
    documentation for setTransferMode() for more details about this.

    Call setProxy() to make QFtp connect via an FTP proxy server.

    The functions currentId() and currentCommand() provide more
    information about the currently executing command.

    The functions hasPendingCommands() and clearPendingCommands()
    allow you to query and clear the list of pending commands.

    If you are an experienced network programmer and want to have
    complete control you can use rawCommand() to execute arbitrary FTP
    commands.

    \warning The current version of QFtp doesn't fully support
    non-Unix FTP servers.

    \sa QNetworkAccessManager, QNetworkRequest, QNetworkReply,
        {FTP Example}
*/


/*!
    Constructs a QFtp object with the given \a parent.
*/
QFtp::QFtp(QObject *parent)
   : QObject(parent), d_ptr(new QFtpPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QFtp);

   d->errorString = tr("Unknown error");

   connect(&d->pi, SIGNAL(connectState(int)),      this, SLOT(_q_piConnectState(int)));
   connect(&d->pi, SIGNAL(finished(QString )),     this, SLOT(_q_piFinished(QString)));
   connect(&d->pi, SIGNAL(error(int, QString)),    this, SLOT(_q_piError(int, QString)));
   connect(&d->pi, SIGNAL(rawFtpReply(int, QString )), this, SLOT(_q_piFtpReply(int, QString)));
   connect(&d->pi.dtp, SIGNAL(readyRead()),            this, SLOT(readyRead()));

   connect(&d->pi.dtp, SIGNAL(dataTransferProgress(qint64, qint64)), this,
           SLOT(dataTransferProgress(qint64, qint64)));

   connect(&d->pi.dtp, SIGNAL(listInfo(QUrlInfo)),     this, SLOT(listInfo(QUrlInfo)));
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

/*!
    Logs in to the FTP server with the username \a user and the
    password \a password.

    The stateChanged() signal is emitted when the state of the
    connecting process changes, e.g. to \c LoggedIn.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::login(const QString &user, const QString &password)
{
   QStringList cmds;
   cmds << (QLatin1String("USER ") + (user.isEmpty() ? QLatin1String("anonymous") : user) + QLatin1String("\r\n"));
   cmds << (QLatin1String("PASS ") + (password.isEmpty() ? QLatin1String("anonymous@") : password) + QLatin1String("\r\n"));
   return d_func()->addCommand(new QFtpCommand(Login, cmds));
}

/*!
    Closes the connection to the FTP server.

    The stateChanged() signal is emitted when the state of the
    connecting process changes, e.g. to \c Closing, then \c
    Unconnected.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa stateChanged() commandStarted() commandFinished()
*/
int QFtp::close()
{
   return d_func()->addCommand(new QFtpCommand(Close, QStringList(QLatin1String("QUIT\r\n"))));
}

/*!
    Sets the current FTP transfer mode to \a mode. The default is QFtp::Passive.

    \sa QFtp::TransferMode
*/
int QFtp::setTransferMode(TransferMode mode)
{
   int id = d_func()->addCommand(new QFtpCommand(SetTransferMode, QStringList()));
   d_func()->pi.transferConnectionExtended = true;
   d_func()->transferMode = mode;
   return id;
}

/*!
    Enables use of the FTP proxy on host \a host and port \a
    port. Calling this function with \a host empty disables proxying.

    QFtp does not support FTP-over-HTTP proxy servers. Use
    QNetworkAccessManager for this.
*/
int QFtp::setProxy(const QString &host, quint16 port)
{
   QStringList args;
   args << host << QString::number(port);
   return d_func()->addCommand(new QFtpCommand(SetProxy, args));
}

/*!
    Lists the contents of directory \a dir on the FTP server. If \a
    dir is empty, it lists the contents of the current directory.

    The listInfo() signal is emitted for each directory entry found.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa listInfo() commandStarted() commandFinished()
*/
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

/*!
    Changes the working directory of the server to \a dir.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::cd(const QString &dir)
{
   return d_func()->addCommand(new QFtpCommand(Cd, QStringList(QLatin1String("CWD ") + dir + QLatin1String("\r\n"))));
}

/*!
    Downloads the file \a file from the server.

    If \a dev is 0, then the readyRead() signal is emitted when there
    is data available to read. You can then read the data with the
    read() or readAll() functions.

    If \a dev is not 0, the data is written directly to the device \a
    dev. Make sure that the \a dev pointer is valid for the duration
    of the operation (it is safe to delete it when the
    commandFinished() signal is emitted). In this case the readyRead()
    signal is \e not emitted and you cannot read data with the
    read() or readAll() functions.

    If you don't read the data immediately it becomes available, i.e.
    when the readyRead() signal is emitted, it is still available
    until the next command is started.

    For example, if you want to present the data to the user as soon
    as there is something available, connect to the readyRead() signal
    and read the data immediately. On the other hand, if you only want
    to work with the complete data, you can connect to the
    commandFinished() signal and read the data when the get() command
    is finished.

    The data is transferred as Binary or Ascii depending on the value
    of \a type.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa readyRead() dataTransferProgress() commandStarted()
    commandFinished()
*/
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

/*!
    \overload

    Writes a copy of the given \a data to the file called \a file on
    the server. The progress of the upload is reported by the
    dataTransferProgress() signal.

    The data is transferred as Binary or Ascii depending on the value
    of \a type.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    Since this function takes a copy of the \a data, you can discard
    your own copy when this function returns.

    \sa dataTransferProgress() commandStarted() commandFinished()
*/
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

/*!
    Reads the data from the IO device \a dev, and writes it to the
    file called \a file on the server. The data is read in chunks from
    the IO device, so this overload allows you to transmit large
    amounts of data without the need to read all the data into memory
    at once.

    The data is transferred as Binary or Ascii depending on the value
    of \a type.

    Make sure that the \a dev pointer is valid for the duration of the
    operation (it is safe to delete it when the commandFinished() is
    emitted).
*/
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

/*!
    Deletes the file called \a file from the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::remove(const QString &file)
{
   return d_func()->addCommand(new QFtpCommand(Remove, QStringList(QLatin1String("DELE ") + file + QLatin1String("\r\n"))));
}

/*!
    Creates a directory called \a dir on the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::mkdir(const QString &dir)
{
   return d_func()->addCommand(new QFtpCommand(Mkdir, QStringList(QLatin1String("MKD ") + dir + QLatin1String("\r\n"))));
}

/*!
    Removes the directory called \a dir from the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::rmdir(const QString &dir)
{
   return d_func()->addCommand(new QFtpCommand(Rmdir, QStringList(QLatin1String("RMD ") + dir + QLatin1String("\r\n"))));
}

/*!
    Renames the file called \a oldname to \a newname on the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::rename(const QString &oldname, const QString &newname)
{
   QStringList cmds;
   cmds << QLatin1String("RNFR ") + oldname + QLatin1String("\r\n");
   cmds << QLatin1String("RNTO ") + newname + QLatin1String("\r\n");
   return d_func()->addCommand(new QFtpCommand(Rename, cmds));
}

/*!
    Sends the raw FTP command \a command to the FTP server. This is
    useful for low-level FTP access. If the operation you wish to
    perform has an equivalent QFtp function, we recommend using the
    function instead of raw FTP commands since the functions are
    easier and safer.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa rawCommandReply() commandStarted() commandFinished()
*/
int QFtp::rawCommand(const QString &command)
{
   QString cmd = command.trimmed() + QLatin1String("\r\n");
   return d_func()->addCommand(new QFtpCommand(RawCommand, QStringList(cmd)));
}

/*!
    Returns the number of bytes that can be read from the data socket
    at the moment.

    \sa get() readyRead() read() readAll()
*/
qint64 QFtp::bytesAvailable() const
{
   return d_func()->pi.dtp.bytesAvailable();
}

/*! \fn qint64 QFtp::readBlock(char *data, quint64 maxlen)

    Use read() instead.
*/

/*!
    Reads \a maxlen bytes from the data socket into \a data and
    returns the number of bytes read. Returns -1 if an error occurred.

    \sa get() readyRead() bytesAvailable() readAll()
*/
qint64 QFtp::read(char *data, qint64 maxlen)
{
   return d_func()->pi.dtp.read(data, maxlen);
}

/*!
    Reads all the bytes available from the data socket and returns
    them.

    \sa get() readyRead() bytesAvailable() read()
*/
QByteArray QFtp::readAll()
{
   return d_func()->pi.dtp.readAll();
}

/*!
    Aborts the current command and deletes all scheduled commands.

    If there is an unfinished command (i.e. a command for which the
    commandStarted() signal has been emitted, but for which the
    commandFinished() signal has not been emitted), this function
    sends an \c ABORT command to the server. When the server replies
    that the command is aborted, the commandFinished() signal with the
    \c error argument set to \c true is emitted for the command. Due
    to timing issues, it is possible that the command had already
    finished before the abort request reached the server, in which
    case, the commandFinished() signal is emitted with the \c error
    argument set to \c false.

    For all other commands that are affected by the abort(), no
    signals are emitted.

    If you don't start further FTP commands directly after the
    abort(), there won't be any scheduled commands and the done()
    signal is emitted.

    \warning Some FTP servers, for example the BSD FTP daemon (version
    0.3), wrongly return a positive reply even when an abort has
    occurred. For these servers the commandFinished() signal has its
    error flag set to \c false, even though the command did not
    complete successfully.

    \sa clearPendingCommands()
*/
void QFtp::abort()
{
   if (d_func()->pending.isEmpty()) {
      return;
   }

   clearPendingCommands();
   d_func()->pi.abort();
}

/*!
    Returns the identifier of the FTP command that is being executed
    or 0 if there is no command being executed.

    \sa currentCommand()
*/
int QFtp::currentId() const
{
   if (d_func()->pending.isEmpty()) {
      return 0;
   }
   return d_func()->pending.first()->id;
}

/*!
    Returns the command type of the FTP command being executed or \c
    None if there is no command being executed.

    \sa currentId()
*/
QFtp::Command QFtp::currentCommand() const
{
   if (d_func()->pending.isEmpty()) {
      return None;
   }
   return d_func()->pending.first()->command;
}

/*!
    Returns the QIODevice pointer that is used by the FTP command to read data
    from or store data to. If there is no current FTP command being executed or
    if the command does not use an IO device, this function returns 0.

    This function can be used to delete the QIODevice in the slot connected to
    the commandFinished() signal.

    \sa get() put()
*/
QIODevice *QFtp::currentDevice() const
{
   if (d_func()->pending.isEmpty()) {
      return 0;
   }
   QFtpCommand *c = d_func()->pending.first();
   if (c->is_ba) {
      return 0;
   }
   return c->data.dev;
}

/*!
    Returns true if there are any commands scheduled that have not yet
    been executed; otherwise returns false.

    The command that is being executed is \e not considered as a
    scheduled command.

    \sa clearPendingCommands() currentId() currentCommand()
*/
bool QFtp::hasPendingCommands() const
{
   return d_func()->pending.count() > 1;
}

/*!
    Deletes all pending commands from the list of scheduled commands.
    This does not affect the command that is being executed. If you
    want to stop this as well, use abort().

    \sa hasPendingCommands() abort()
*/
void QFtp::clearPendingCommands()
{
   // delete all entires except the first one
   while (d_func()->pending.count() > 1) {
      delete d_func()->pending.takeLast();
   }
}

/*!
    Returns the current state of the object. When the state changes,
    the stateChanged() signal is emitted.

    \sa State stateChanged()
*/
QFtp::State QFtp::state() const
{
   return d_func()->state;
}

/*!
    Returns the last error that occurred. This is useful to find out
    what went wrong when receiving a commandFinished() or a done()
    signal with the \c error argument set to \c true.

    If you start a new command, the error status is reset to \c NoError.
*/
QFtp::Error QFtp::error() const
{
   return d_func()->error;
}

/*!
    Returns a human-readable description of the last error that
    occurred. This is useful for presenting a error message to the
    user when receiving a commandFinished() or a done() signal with
    the \c error argument set to \c true.

    The error string is often (but not always) the reply from the
    server, so it is not always possible to translate the string. If
    the message comes from Qt, the string has already passed through
    tr().
*/
QString QFtp::errorString() const
{
   return d_func()->errorString;
}

/*! \internal
*/
void QFtpPrivate::_q_startNextCommand()
{
   Q_Q(QFtp);
   if (pending.isEmpty()) {
      return;
   }
   QFtpCommand *c = pending.first();

   error = QFtp::NoError;
   errorString = QT_TRANSLATE_NOOP(QFtp, QLatin1String("Unknown error"));

   if (q->bytesAvailable()) {
      q->readAll();   // clear the data
   }
   emit q->commandStarted(c->id);

   // Proxy support, replace the Login argument in place, then fall through.
   if (c->command == QFtp::Login && ! proxyHost.isEmpty()) {
      QString loginString = c->rawCmds.first().trimmed();
      loginString += QChar('@') + host;

      if (port && port != 21) {
         loginString += QLatin1Char(':') + QString::number(port);
      }

      loginString.append("\r\n");
      c->rawCmds[0] = loginString;
   }

   if (c->command == QFtp::SetTransferMode) {
      _q_piFinished(QLatin1String("Transfer mode set"));

   } else if (c->command == QFtp::SetProxy) {
      proxyHost = c->rawCmds[0];
      proxyPort = c->rawCmds[1].toInteger<uint>();

      c->rawCmds.clear();
      _q_piFinished(QLatin1String("Proxy set to ") + proxyHost + QLatin1Char(':') + QString::number(proxyPort));

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

               pi.dtp.connect(c->data.dev, SIGNAL(readyRead()),           &pi.dtp, SLOT(dataReadyRead()));
               pi.dtp.connect(c->data.dev, SIGNAL(readChannelFinished()), &pi.dtp, SLOT(dataReadyRead()));

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

/*! \internal
*/
void QFtpPrivate::_q_piFinished(const QString &)
{
   if (pending.isEmpty()) {
      return;
   }
   QFtpCommand *c = pending.first();

   if (c->command == QFtp::Close) {
      // The order of in which the slots are called is arbitrary, so
      // disconnect the SIGNAL-SIGNAL temporary to make sure that we
      // don't get the commandFinished() signal before the stateChanged()
      // signal.
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

/*! \internal
*/
void QFtpPrivate::_q_piError(int errorCode, const QString &text)
{
   Q_Q(QFtp);

   if (pending.isEmpty()) {
      qWarning("QFtpPrivate::_q_piError was called without pending command!");
      return;
   }

   QFtpCommand *c = pending.first();

   // non-fatal errors
   if (c->command == QFtp::Get && pi.currentCommand().startsWith(QLatin1String("SIZE "))) {
      pi.dtp.setBytesTotal(0);
      return;
   } else if (c->command == QFtp::Put && pi.currentCommand().startsWith(QLatin1String("ALLO "))) {
      return;
   }

   error = QFtp::Error(errorCode);
   switch (q->currentCommand()) {
      case QFtp::ConnectToHost:
         errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Connecting to host failed:\n%1")).formatArg(text);
         break;

      case QFtp::Login:
         errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Login failed:\n%1")).formatArg(text);
         break;

      case QFtp::List:
         errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Listing directory failed:\n%1")).formatArg(text);
         break;

      case QFtp::Cd:
         errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Changing directory failed:\n%1")).formatArg(text);
         break;

      case QFtp::Get:
         errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Downloading file failed:\n%1")).formatArg(text);
         break;

      case QFtp::Put:
         errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Uploading file failed:\n%1")).formatArg(text);
         break;

      case QFtp::Remove:
         errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Removing file failed:\n%1")).formatArg(text);
         break;

      case QFtp::Mkdir:
         errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Creating directory failed:\n%1")).formatArg(text);
         break;

      case QFtp::Rmdir:
         errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Removing directory failed:\n%1")).formatArg(text);
         break;

      default:
         errorString = text;
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

/*! \internal
*/
void QFtpPrivate::_q_piConnectState(int connectState)
{
   state = QFtp::State(connectState);
   emit q_func()->stateChanged(state);
   if (close_waitForStateChange) {
      close_waitForStateChange = false;
      _q_piFinished(QLatin1String(QT_TRANSLATE_NOOP("QFtp", "Connection closed")));
   }
}

/*! \internal
*/
void QFtpPrivate::_q_piFtpReply(int code, const QString &text)
{
   if (q_func()->currentCommand() == QFtp::RawCommand) {
      pi.rawCommand = true;
      emit q_func()->rawCommandReply(code, text);
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

void QFtp::_q_piFinished(const QString &un_named_arg1)
{
   Q_D(QFtp);
   d->_q_piFinished(un_named_arg1);
}

void QFtp::_q_piError(int un_named_arg1, const QString &un_named_arg2)
{
   Q_D(QFtp);
   d->_q_piError(un_named_arg1, un_named_arg2);
}

void QFtp::_q_piConnectState(int un_named_arg1)
{
   Q_D(QFtp);
   d->_q_piConnectState(un_named_arg1);
}

void QFtp::_q_piFtpReply(int un_named_arg1, const QString &un_named_arg2)
{
   Q_D(QFtp);
   d->_q_piFtpReply(un_named_arg1, un_named_arg2);
}

#endif // QT_NO_FTP
