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

#include <qnoncontiguousbytedevice_p.h>
#include <qbuffer.h>
#include <qdebug.h>
#include <qfile.h>

QT_BEGIN_NAMESPACE

QNonContiguousByteDevice::QNonContiguousByteDevice() : QObject((QObject *)0), resetDisabled(false)
{
}

QNonContiguousByteDevice::~QNonContiguousByteDevice()
{
}

void QNonContiguousByteDevice::disableReset()
{
   resetDisabled = true;
}

// FIXME we should scrap this whole implementation and instead change the ByteArrayImpl to be able to cope with sub-arrays?
QNonContiguousByteDeviceBufferImpl::QNonContiguousByteDeviceBufferImpl(QBuffer *b) : QNonContiguousByteDevice()
{
   buffer = b;
   byteArray = QByteArray::fromRawData(buffer->buffer().constData() + buffer->pos(), buffer->size() - buffer->pos());
   arrayImpl = new QNonContiguousByteDeviceByteArrayImpl(&byteArray);
   arrayImpl->setParent(this);

   connect(arrayImpl, SIGNAL(readyRead()), this, SLOT(readyRead()));
   connect(arrayImpl, SIGNAL(readProgress(qint64, qint64)), this, SLOT(readProgress(qint64, qint64)));
}

QNonContiguousByteDeviceBufferImpl::~QNonContiguousByteDeviceBufferImpl()
{
}

const char *QNonContiguousByteDeviceBufferImpl::readPointer(qint64 maximumLength, qint64 &len)
{
   return arrayImpl->readPointer(maximumLength, len);
}

bool QNonContiguousByteDeviceBufferImpl::advanceReadPointer(qint64 amount)
{
   return arrayImpl->advanceReadPointer(amount);
}

bool QNonContiguousByteDeviceBufferImpl::atEnd()
{
   return arrayImpl->atEnd();
}

bool QNonContiguousByteDeviceBufferImpl::reset()
{
   if (resetDisabled) {
      return false;
   }
   return arrayImpl->reset();
}

qint64 QNonContiguousByteDeviceBufferImpl::size()
{
   return arrayImpl->size();
}

QNonContiguousByteDeviceByteArrayImpl::QNonContiguousByteDeviceByteArrayImpl(QByteArray *ba) :
   QNonContiguousByteDevice(), currentPosition(0)
{
   byteArray = ba;
}

QNonContiguousByteDeviceByteArrayImpl::~QNonContiguousByteDeviceByteArrayImpl()
{
}

const char *QNonContiguousByteDeviceByteArrayImpl::readPointer(qint64 maximumLength, qint64 &len)
{
   if (atEnd()) {
      len = -1;
      return 0;
   }

   if (maximumLength != -1) {
      len = qMin(maximumLength, size() - currentPosition);
   } else {
      len = size() - currentPosition;
   }

   return byteArray->constData() + currentPosition;
}

bool QNonContiguousByteDeviceByteArrayImpl::advanceReadPointer(qint64 amount)
{
   currentPosition += amount;
   emit readProgress(currentPosition, size());
   return true;
}

bool QNonContiguousByteDeviceByteArrayImpl::atEnd()
{
   return currentPosition >= size();
}

bool QNonContiguousByteDeviceByteArrayImpl::reset()
{
   if (resetDisabled) {
      return false;
   }

   currentPosition = 0;
   return true;
}

qint64 QNonContiguousByteDeviceByteArrayImpl::size()
{
   return byteArray->size();
}

qint64 QNonContiguousByteDeviceByteArrayImpl::pos()
{
   return currentPosition;
}

QNonContiguousByteDeviceRingBufferImpl::QNonContiguousByteDeviceRingBufferImpl(QSharedPointer<QRingBuffer> rb)
   : QNonContiguousByteDevice(), currentPosition(0)
{
   ringBuffer = rb;
}

QNonContiguousByteDeviceRingBufferImpl::~QNonContiguousByteDeviceRingBufferImpl()
{
}

const char *QNonContiguousByteDeviceRingBufferImpl::readPointer(qint64 maximumLength, qint64 &len)
{
   if (atEnd()) {
      len = -1;
      return 0;
   }

   const char *returnValue = ringBuffer->readPointerAtPosition(currentPosition, len);

   if (maximumLength != -1) {
      len = qMin(len, maximumLength);
   }

   return returnValue;
}

bool QNonContiguousByteDeviceRingBufferImpl::advanceReadPointer(qint64 amount)
{
   currentPosition += amount;
   emit readProgress(currentPosition, size());
   return true;
}

bool QNonContiguousByteDeviceRingBufferImpl::atEnd()
{
   return currentPosition >= size();
}

bool QNonContiguousByteDeviceRingBufferImpl::reset()
{
   if (resetDisabled) {
      return false;
   }

   currentPosition = 0;
   return true;
}

qint64 QNonContiguousByteDeviceRingBufferImpl::size()
{
   return ringBuffer->size();
}

qint64 QNonContiguousByteDeviceRingBufferImpl::pos()
{
   return currentPosition;
}

QNonContiguousByteDeviceIoDeviceImpl::QNonContiguousByteDeviceIoDeviceImpl(QIODevice *d)
   : QNonContiguousByteDevice(),
     currentReadBuffer(0), currentReadBufferSize(16 * 1024),
     currentReadBufferAmount(0), currentReadBufferPosition(0), totalAdvancements(0),
     eof(false)
{
   device = d;
   initialPosition = d->pos();
   connect(device, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::QueuedConnection);
   connect(device, SIGNAL(readChannelFinished()), this, SLOT(readyRead()), Qt::QueuedConnection);
}

QNonContiguousByteDeviceIoDeviceImpl::~QNonContiguousByteDeviceIoDeviceImpl()
{
   delete currentReadBuffer;
}

const char *QNonContiguousByteDeviceIoDeviceImpl::readPointer(qint64 maximumLength, qint64 &len)
{
   if (eof == true) {
      len = -1;
      return 0;
   }

   if (currentReadBuffer == 0) {
      currentReadBuffer = new QByteArray(currentReadBufferSize, '\0');   // lazy alloc
   }

   if (maximumLength == -1) {
      maximumLength = currentReadBufferSize;
   }

   if (currentReadBufferAmount - currentReadBufferPosition > 0) {
      len = currentReadBufferAmount - currentReadBufferPosition;
      return currentReadBuffer->data() + currentReadBufferPosition;
   }

   qint64 haveRead = device->read(currentReadBuffer->data(), qMin(maximumLength, currentReadBufferSize));

   if ((haveRead == -1) || (haveRead == 0 && device->atEnd() && !device->isSequential())) {
      eof = true;
      len = -1;
      // size was unknown before, emit a readProgress with the final size
      if (size() == -1) {
         emit readProgress(totalAdvancements, totalAdvancements);
      }
      return 0;
   }

   currentReadBufferAmount = haveRead;
   currentReadBufferPosition = 0;

   len = haveRead;
   return currentReadBuffer->data();
}

bool QNonContiguousByteDeviceIoDeviceImpl::advanceReadPointer(qint64 amount)
{
   totalAdvancements += amount;

   // normal advancement
   currentReadBufferPosition += amount;

   if (size() == -1) {
      emit readProgress(totalAdvancements, totalAdvancements);
   } else {
      emit readProgress(totalAdvancements, size());
   }

   // advancing over that what has actually been read before
   if (currentReadBufferPosition > currentReadBufferAmount) {
      qint64 i = currentReadBufferPosition - currentReadBufferAmount;
      while (i > 0) {
         if (device->getChar(0) == false) {
            emit readProgress(totalAdvancements - i, size());
            return false; // ### FIXME handle eof
         }
         i--;
      }

      currentReadBufferPosition = 0;
      currentReadBufferAmount = 0;
   }


   return true;
}

bool QNonContiguousByteDeviceIoDeviceImpl::atEnd()
{
   return eof == true;
}

bool QNonContiguousByteDeviceIoDeviceImpl::reset()
{
   if (resetDisabled) {
      return false;
   }
   bool reset = (initialPosition == 0) ? device->reset() : device->seek(initialPosition);
   if (reset) {
      eof = false; // assume eof is false, it will be true after a read has been attempted
      totalAdvancements = 0; //reset the progress counter
      if (currentReadBuffer) {
         delete currentReadBuffer;
         currentReadBuffer = 0;
      }
      currentReadBufferAmount = 0;
      currentReadBufferPosition = 0;
      return true;
   }

   return false;
}

qint64 QNonContiguousByteDeviceIoDeviceImpl::size()
{
   // note that this is different from the size() implementation of QIODevice!

   if (device->isSequential()) {
      return -1;
   }

   return device->size() - initialPosition;
}

qint64 QNonContiguousByteDeviceIoDeviceImpl::pos()
{
   if (device->isSequential()) {
      return -1;
   }

   return device->pos();
}

QByteDeviceWrappingIoDevice::QByteDeviceWrappingIoDevice(QNonContiguousByteDevice *bd) : QIODevice((QObject *)0)
{
   byteDevice = bd;
   connect(bd, SIGNAL(readyRead()), this, SLOT(readyRead()));

   open(ReadOnly);
}

QByteDeviceWrappingIoDevice::~QByteDeviceWrappingIoDevice()
{

}

bool QByteDeviceWrappingIoDevice::isSequential() const
{
   return (byteDevice->size() == -1);
}

bool QByteDeviceWrappingIoDevice::atEnd() const
{
   return byteDevice->atEnd();
}

bool QByteDeviceWrappingIoDevice::reset()
{
   return byteDevice->reset();
}

qint64 QByteDeviceWrappingIoDevice::size() const
{
   if (isSequential()) {
      return 0;
   }

   return byteDevice->size();
}

qint64 QByteDeviceWrappingIoDevice::readData( char *data, qint64 maxSize)
{
   qint64 len;
   const char *readPointer = byteDevice->readPointer(maxSize, len);
   if (len == -1) {
      return -1;
   }

   memcpy(data, readPointer, len);
   byteDevice->advanceReadPointer(len);
   return len;
}

qint64 QByteDeviceWrappingIoDevice::writeData( const char *data, qint64 maxSize)
{
   Q_UNUSED(data);
   Q_UNUSED(maxSize);
   return -1;
}

QNonContiguousByteDevice *QNonContiguousByteDeviceFactory::create(QIODevice *device)
{
   // shortcut if it is a QBuffer
   if (QBuffer *buffer = qobject_cast<QBuffer *>(device)) {
      return new QNonContiguousByteDeviceBufferImpl(buffer);
   }

   // ### FIXME special case if device is a QFile that supports map()
   // then we can actually deal with the file without using read/peek

   // generic QIODevice
   return new QNonContiguousByteDeviceIoDeviceImpl(device); // FIXME
}

QNonContiguousByteDevice *QNonContiguousByteDeviceFactory::create(QByteArray *byteArray)
{
   return new QNonContiguousByteDeviceByteArrayImpl(byteArray);
}


QNonContiguousByteDevice *QNonContiguousByteDeviceFactory::create(QSharedPointer<QRingBuffer> ringBuffer)
{
   return new QNonContiguousByteDeviceRingBufferImpl(ringBuffer);
}

QSharedPointer<QNonContiguousByteDevice> QNonContiguousByteDeviceFactory::createShared(QIODevice *device)
{
   return QSharedPointer<QNonContiguousByteDevice>(create(device));
}

QSharedPointer<QNonContiguousByteDevice> QNonContiguousByteDeviceFactory::createShared(QByteArray *byteArray)
{
   return QSharedPointer<QNonContiguousByteDevice>(create(byteArray));
}

QSharedPointer<QNonContiguousByteDevice> QNonContiguousByteDeviceFactory::createShared(QSharedPointer<QRingBuffer> ringBuffer)
{
   return QSharedPointer<QNonContiguousByteDevice>(create(ringBuffer));
}

QIODevice *QNonContiguousByteDeviceFactory::wrap(QNonContiguousByteDevice *byteDevice)
{
   // ### FIXME if the parameter inherits from QIoDevice, we could just return it
   // needed for FTP backend

   return new QByteDeviceWrappingIoDevice(byteDevice);
}

QT_END_NAMESPACE

