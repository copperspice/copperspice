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

#ifndef QTCPSOCKET_H
#define QTCPSOCKET_H

#include <qabstractsocket.h>
#include <qvariant.h>

class QTcpSocketPrivate;

class Q_NETWORK_EXPORT QTcpSocket : public QAbstractSocket
{
   NET_CS_OBJECT(QTcpSocket)

 public:
   explicit QTcpSocket(QObject *parent = nullptr);

   QTcpSocket(const QTcpSocket &) = delete;
   QTcpSocket &operator=(const QTcpSocket &) = delete;

   virtual ~QTcpSocket();

 protected:
   QTcpSocket(QTcpSocketPrivate &dd, QObject *parent = nullptr);
   QTcpSocket(QAbstractSocket::SocketType socketType, QTcpSocketPrivate &dd, QObject *parent = nullptr);

 private:
   Q_DECLARE_PRIVATE(QTcpSocket)
};

#endif
