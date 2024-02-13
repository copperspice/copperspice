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

#ifndef QNONCONTIGUOUSBYTEDEVICE_P_H
#define QNONCONTIGUOUSBYTEDEVICE_P_H

#include <qbuffer.h>
#include <qbytearray.h>
#include <qiodevice.h>
#include <qobject.h>
#include <qsharedpointer.h>

#include <qringbuffer_p.h>

class Q_CORE_EXPORT QNonContiguousByteDevice : public QObject
{
   CORE_CS_OBJECT(QNonContiguousByteDevice)

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
   virtual qint64 pos() {
      return -1;
   }

   virtual ~QNonContiguousByteDevice();

   CORE_CS_SIGNAL_1(Public, void readyRead())
   CORE_CS_SIGNAL_2(readyRead)
   CORE_CS_SIGNAL_1(Public, void readProgress(qint64 current, qint64 total))
   CORE_CS_SIGNAL_2(readProgress, current, total)

 protected:
   QNonContiguousByteDevice();

   bool resetDisabled;
};

class Q_CORE_EXPORT QNonContiguousByteDeviceFactory
{
 public:
   static QNonContiguousByteDevice *create(QIODevice *device);
   static QNonContiguousByteDevice *create(QByteArray *byteArray);
   static QNonContiguousByteDevice *create(QSharedPointer<QRingBuffer> ringBuffer);

   static QSharedPointer<QNonContiguousByteDevice> createShared(QIODevice *device);
   static QSharedPointer<QNonContiguousByteDevice> createShared(QByteArray *byteArray);
   static QSharedPointer<QNonContiguousByteDevice> createShared(QSharedPointer<QRingBuffer> ringBuffer);

   static QIODevice *wrap(QNonContiguousByteDevice *byteDevice);
};

class QNonContiguousByteDeviceByteArrayImpl : public QNonContiguousByteDevice
{
 public:
   QNonContiguousByteDeviceByteArrayImpl(QByteArray *ba);
   ~QNonContiguousByteDeviceByteArrayImpl();

   const char *readPointer(qint64 maximumLength, qint64 &len) override;
   bool advanceReadPointer(qint64 amount) override;
   bool atEnd() override;
   bool reset() override;
   qint64 size() override;
   qint64 pos() override;

 protected:
   QByteArray *byteArray;
   qint64 currentPosition;
};

class QNonContiguousByteDeviceRingBufferImpl : public QNonContiguousByteDevice
{
 public:
   QNonContiguousByteDeviceRingBufferImpl(QSharedPointer<QRingBuffer> rb);
   ~QNonContiguousByteDeviceRingBufferImpl();

   const char *readPointer(qint64 maximumLength, qint64 &len) override;
   bool advanceReadPointer(qint64 amount) override;
   bool atEnd() override;
   bool reset() override;
   qint64 size() override;
   qint64 pos() override;

 protected:
   QSharedPointer<QRingBuffer> ringBuffer;
   qint64 currentPosition;
};

class QNonContiguousByteDeviceIoDeviceImpl : public QNonContiguousByteDevice
{
   CORE_CS_OBJECT(QNonContiguousByteDeviceIoDeviceImpl)

 public:
   QNonContiguousByteDeviceIoDeviceImpl(QIODevice *d);
   ~QNonContiguousByteDeviceIoDeviceImpl();

   const char *readPointer(qint64 maximumLength, qint64 &len) override;
   bool advanceReadPointer(qint64 amount) override;
   bool atEnd() override;
   bool reset() override;
   qint64 size() override;
   qint64 pos() override;

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
   CORE_CS_OBJECT(QNonContiguousByteDeviceBufferImpl)

 public:
   QNonContiguousByteDeviceBufferImpl(QBuffer *b);
   ~QNonContiguousByteDeviceBufferImpl();

   const char *readPointer(qint64 maximumLength, qint64 &len) override;
   bool advanceReadPointer(qint64 amount) override;
   bool atEnd() override;
   bool reset() override;
   qint64 size() override;

 protected:
   QBuffer *buffer;
   QByteArray byteArray;
   QNonContiguousByteDeviceByteArrayImpl *arrayImpl;
};

class QByteDeviceWrappingIoDevice : public QIODevice
{
 public:
   QByteDeviceWrappingIoDevice (QNonContiguousByteDevice *bd);
   ~QByteDeviceWrappingIoDevice ();

   bool isSequential () const override;
   bool atEnd () const override;
   bool reset () override;
   qint64 size () const override;

 protected:
   qint64 readData ( char *data, qint64 maxSize ) override;
   qint64 writeData ( const char *data, qint64 maxSize ) override;

   QNonContiguousByteDevice *byteDevice;
};

#endif
