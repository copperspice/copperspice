/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QNONCONTIGUOUSBYTEDEVICE_P_H
#define QNONCONTIGUOUSBYTEDEVICE_P_H

#include <QtCore/qobject.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qbuffer.h>
#include <QtCore/qiodevice.h>
#include <QtCore/QSharedPointer>
#include "qringbuffer_p.h"

QT_BEGIN_NAMESPACE

class Q_CORE_EXPORT QNonContiguousByteDevice : public QObject
{
   CS_OBJECT(QNonContiguousByteDevice)
 public:
   virtual const char *readPointer(qint64 maximumLength, qint64 &len) = 0;
   virtual bool advanceReadPointer(qint64 amount) = 0;
   virtual bool atEnd() = 0;
   virtual bool reset() = 0;
   void disableReset();
   bool isResetDisabled() {
      return resetDisabled;
   }
   virtual qint64 size() = 0;

   virtual ~QNonContiguousByteDevice();

 protected:
   QNonContiguousByteDevice();


   bool resetDisabled;
 public:
   CORE_CS_SIGNAL_1(Public, void readyRead())
   CORE_CS_SIGNAL_2(readyRead)
   CORE_CS_SIGNAL_1(Public, void readProgress(qint64 current, qint64 total))
   CORE_CS_SIGNAL_2(readProgress, current, total)
};

class Q_CORE_EXPORT QNonContiguousByteDeviceFactory
{
 public:
   static QNonContiguousByteDevice *create(QIODevice *device);
   static QNonContiguousByteDevice *create(QByteArray *byteArray);
   static QNonContiguousByteDevice *create(QSharedPointer<QRingBuffer> ringBuffer);
   static QIODevice *wrap(QNonContiguousByteDevice *byteDevice);
};

// the actual implementations
//

class QNonContiguousByteDeviceByteArrayImpl : public QNonContiguousByteDevice
{
 public:
   QNonContiguousByteDeviceByteArrayImpl(QByteArray *ba);
   ~QNonContiguousByteDeviceByteArrayImpl();
   const char *readPointer(qint64 maximumLength, qint64 &len);
   bool advanceReadPointer(qint64 amount);
   bool atEnd();
   bool reset();
   qint64 size();
 protected:
   QByteArray *byteArray;
   qint64 currentPosition;
};

class QNonContiguousByteDeviceRingBufferImpl : public QNonContiguousByteDevice
{
 public:
   QNonContiguousByteDeviceRingBufferImpl(QSharedPointer<QRingBuffer> rb);
   ~QNonContiguousByteDeviceRingBufferImpl();
   const char *readPointer(qint64 maximumLength, qint64 &len);
   bool advanceReadPointer(qint64 amount);
   bool atEnd();
   bool reset();
   qint64 size();
 protected:
   QSharedPointer<QRingBuffer> ringBuffer;
   qint64 currentPosition;
};


class QNonContiguousByteDeviceIoDeviceImpl : public QNonContiguousByteDevice
{
   CS_OBJECT(QNonContiguousByteDeviceIoDeviceImpl)
 public:
   QNonContiguousByteDeviceIoDeviceImpl(QIODevice *d);
   ~QNonContiguousByteDeviceIoDeviceImpl();
   const char *readPointer(qint64 maximumLength, qint64 &len);
   bool advanceReadPointer(qint64 amount);
   bool atEnd();
   bool reset();
   qint64 size();
 protected:
   QIODevice *device;
   QByteArray *currentReadBuffer;
   qint64 currentReadBufferSize;
   qint64 currentReadBufferAmount;
   qint64 currentReadBufferPosition;
   qint64 totalAdvancements; //progress counter used for emitting the readProgress signal
   bool eof;
   qint64 initialPosition;
};

class QNonContiguousByteDeviceBufferImpl : public QNonContiguousByteDevice
{
   CS_OBJECT(QNonContiguousByteDeviceBufferImpl)
 public:
   QNonContiguousByteDeviceBufferImpl(QBuffer *b);
   ~QNonContiguousByteDeviceBufferImpl();
   const char *readPointer(qint64 maximumLength, qint64 &len);
   bool advanceReadPointer(qint64 amount);
   bool atEnd();
   bool reset();
   qint64 size();
 protected:
   QBuffer *buffer;
   QByteArray byteArray;
   QNonContiguousByteDeviceByteArrayImpl *arrayImpl;
};

// ... and the reverse thing
class QByteDeviceWrappingIoDevice : public QIODevice
{
 public:
   QByteDeviceWrappingIoDevice (QNonContiguousByteDevice *bd);
   ~QByteDeviceWrappingIoDevice ();
   virtual bool isSequential () const;
   virtual bool atEnd () const;
   virtual bool reset ();
   virtual qint64 size () const;
 protected:
   virtual qint64 readData ( char *data, qint64 maxSize );
   virtual qint64 writeData ( const char *data, qint64 maxSize );

   QNonContiguousByteDevice *byteDevice;
};

QT_END_NAMESPACE

#endif
