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

#ifndef QPACKETPROTOCOL_P_H
#define QPACKETPROTOCOL_P_H

#include <QtCore/qobject.h>
#include <QtCore/qdatastream.h>
#include <qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

class QIODevice;
class QBuffer;
class QPacket;
class QPacketAutoSend;
class QPacketProtocolPrivate;

class Q_DECLARATIVE_EXPORT QPacketProtocol : public QObject
{
   DECL_CS_OBJECT(QPacketProtocol)
 public:
   explicit QPacketProtocol(QIODevice *dev, QObject *parent = nullptr);
   virtual ~QPacketProtocol();

   qint32 maximumPacketSize() const;
   qint32 setMaximumPacketSize(qint32);

   QPacketAutoSend send();
   void send(const QPacket &);

   qint64 packetsAvailable() const;
   QPacket read();

   bool waitForReadyRead(int msecs = 3000);

   void clear();

   QIODevice *device();

 public:
   DECL_CS_SIGNAL_1(Public, void readyRead())
   DECL_CS_SIGNAL_2(readyRead)
   DECL_CS_SIGNAL_1(Public, void invalidPacket())
   DECL_CS_SIGNAL_2(invalidPacket)
   DECL_CS_SIGNAL_1(Public, void packetWritten())
   DECL_CS_SIGNAL_2(packetWritten)

 private:
   QPacketProtocolPrivate *d;
};


class Q_DECLARATIVE_EXPORT QPacket : public QDataStream
{
 public:
   QPacket();
   QPacket(const QPacket &);
   virtual ~QPacket();

   void clear();
   bool isEmpty() const;
   QByteArray data() const;

 protected:
   friend class QPacketProtocol;
   QPacket(const QByteArray &ba);
   QByteArray b;
   QBuffer *buf;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QPacketAutoSend : public QPacket
{
 public:
   virtual ~QPacketAutoSend();

 private:
   friend class QPacketProtocol;
   QPacketAutoSend(QPacketProtocol *);
   QPacketProtocol *p;
};

QT_END_NAMESPACE

#endif
