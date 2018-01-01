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

#ifndef QWSUTILS_QWS_H
#define QWSUTILS_QWS_H

#include <QtCore/QIODevice>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SXE
#define QWS_SOCK_BASE QUnixSocket
#define QWS_SOCK_SERVER_BASE QUnixSocketServer
class QUnixSocket;
class QUnixSocketServer;

#else
#define QWS_SOCK_BASE QTcpSocket
#define QWS_SOCK_SERVER_BASE QTcpServer
class QTcpSocket;
class QTcpServer;
#endif

class QWSSocket;
class QWSServerSocket;

/********************************************************************
 *
 * Convenient socket functions
 *
 ********************************************************************/
#ifndef QT_NO_QWS_MULTIPROCESS
inline int qws_read_uint(QIODevice *socket)
{
   if (!socket || socket->bytesAvailable() < (int)sizeof(int)) {
      return -1;
   }

   int i;
   socket->read(reinterpret_cast<char *>(&i), sizeof(i));

   return i;
}

inline void qws_write_uint(QIODevice *socket, int i)
{
   if (!socket) {
      return;
   }

   socket->write(reinterpret_cast<char *>(&i), sizeof(i));
}

#endif // QT_NO_QWS_MULTIPROCESS

QT_END_NAMESPACE

#endif // QWSUTILS_QWS_H
