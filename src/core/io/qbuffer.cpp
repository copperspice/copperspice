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

#include <qbuffer.h>

#include <qiodevice_p.h>

class QBufferPrivate : public QIODevicePrivate
{
   Q_DECLARE_PUBLIC(QBuffer)

 public:
   QBufferPrivate()
      : buf(nullptr), writtenSinceLastEmit(0), signalConnectionCount(0), signalsEmitted(false)
   { }

   ~QBufferPrivate()
   { }

   QByteArray *buf;
   QByteArray defaultBuf;
   int ioIndex;

   qint64 peek(char *data, qint64 maxSize) override;
   QByteArray peek(qint64 maxSize) override;

   void _q_emitSignals();

   qint64 writtenSinceLastEmit;
   mutable int signalConnectionCount;
   bool signalsEmitted;
};

void QBufferPrivate::_q_emitSignals()
{
   Q_Q(QBuffer);

   emit q->bytesWritten(writtenSinceLastEmit);
   writtenSinceLastEmit = 0;

   emit q->readyRead();
   signalsEmitted = false;
}

void QBuffer::_q_emitSignals()
{
   Q_D(QBuffer);
   d->_q_emitSignals();
}

qint64 QBufferPrivate::peek(char *data, qint64 maxSize)
{
   qint64 readBytes = qMin(maxSize, static_cast<qint64>(buf->size()) - pos);
   memcpy(data, buf->constData() + pos, readBytes);
   return readBytes;
}

QByteArray QBufferPrivate::peek(qint64 maxSize)
{
   qint64 readBytes = qMin(maxSize, static_cast<qint64>(buf->size()) - pos);

   if (pos == 0 && maxSize >= buf->size()) {
      return *buf;
   }

   return QByteArray(buf->constData() + pos, readBytes);
}

QBuffer::QBuffer(QObject *parent)
   : QIODevice(*new QBufferPrivate, parent)
{
   Q_D(QBuffer);

   d->buf = &d->defaultBuf;
   d->ioIndex = 0;
}

QBuffer::QBuffer(QByteArray *byteArray, QObject *parent)
   : QIODevice(*new QBufferPrivate, parent)
{
   Q_D(QBuffer);
   d->buf = byteArray ? byteArray : &d->defaultBuf;
   d->defaultBuf.clear();
   d->ioIndex = 0;
}

QBuffer::~QBuffer()
{
}

void QBuffer::setBuffer(QByteArray *byteArray)
{
   Q_D(QBuffer);

   if (isOpen()) {
      qWarning("QBuffer::setBuffer() Buffer is open");
      return;
   }

   if (byteArray) {
      d->buf = byteArray;
   } else {
      d->buf = &d->defaultBuf;
   }

   d->defaultBuf.clear();
   d->ioIndex = 0;
}

QByteArray &QBuffer::buffer()
{
   Q_D(QBuffer);
   return *d->buf;
}

const QByteArray &QBuffer::buffer() const
{
   Q_D(const QBuffer);
   return *d->buf;
}

const QByteArray &QBuffer::data() const
{
   Q_D(const QBuffer);
   return *d->buf;
}

void QBuffer::setData(const QByteArray &data)
{
   Q_D(QBuffer);

   if (isOpen()) {
      qWarning("QBuffer::setData() Buffer is open");
      return;
   }

   *d->buf = data;
   d->ioIndex = 0;
}

bool QBuffer::open(OpenMode flags)
{
   Q_D(QBuffer);

   if ((flags & (Append | Truncate)) != 0) {
      flags |= WriteOnly;
   }

   if ((flags & (ReadOnly | WriteOnly)) == 0) {
      qWarning("QBuffer::open() Buffer access not specified");
      return false;
   }

   if ((flags & Truncate) == Truncate) {
      d->buf->resize(0);
   }

   d->ioIndex = (flags & Append) == Append ? d->buf->size() : 0;

   return QIODevice::open(flags);
}

void QBuffer::close()
{
   QIODevice::close();
}

qint64 QBuffer::pos() const
{
   return QIODevice::pos();
}

qint64 QBuffer::size() const
{
   Q_D(const QBuffer);
   return qint64(d->buf->size());
}

bool QBuffer::seek(qint64 pos)
{
   Q_D(QBuffer);

   if (pos > d->buf->size() && isWritable()) {
      if (seek(d->buf->size())) {
         const qint64 gapSize = pos - d->buf->size();

         if (write(QByteArray(gapSize, 0)) != gapSize) {
            qWarning("QBuffer::seek() Unable to fill gap");
            return false;
         }

      } else {
         return false;
      }

   } else if (pos > d->buf->size() || pos < 0) {
      qWarning("QBuffer::seek() Invalid position, %d", int(pos));
      return false;
   }

   d->ioIndex = int(pos);

   return QIODevice::seek(pos);
}

bool QBuffer::atEnd() const
{
   return QIODevice::atEnd();
}

bool QBuffer::canReadLine() const
{
   Q_D(const QBuffer);

   if (!isOpen()) {
      return false;
   }

   return d->buf->indexOf('\n', int(pos())) != -1 || QIODevice::canReadLine();
}

qint64 QBuffer::readData(char *data, qint64 len)
{
   Q_D(QBuffer);

   if ((len = qMin(len, qint64(d->buf->size()) - d->ioIndex)) <= 0) {
      return qint64(0);
   }

   memcpy(data, d->buf->constData() + d->ioIndex, len);
   d->ioIndex += int(len);
   return len;
}

qint64 QBuffer::writeData(const char *data, qint64 len)
{
   Q_D(QBuffer);

   int extraBytes = d->ioIndex + len - d->buf->size();

   if (extraBytes > 0) { // overflow
      int newSize = d->buf->size() + extraBytes;
      d->buf->resize(newSize);

      if (d->buf->size() != newSize) { // could not resize
         qWarning("QBuffer::writeData() Memory allocation error");
         return -1;
      }
   }

   memcpy(d->buf->data() + d->ioIndex, (uchar *)data, int(len));
   d->ioIndex += int(len);

   d->writtenSinceLastEmit += len;

   if (d->signalConnectionCount && !d->signalsEmitted && !signalsBlocked()) {
      d->signalsEmitted = true;
      QMetaObject::invokeMethod(this, "_q_emitSignals", Qt::QueuedConnection);
   }

   return len;
}

// internal
void QBuffer::connectNotify(const QMetaMethod &signalMethod) const
{
   if (signalMethod.name() == "readyRead" || signalMethod.name() == "bytesWritten") {
      d_func()->signalConnectionCount++;
   }
}

// internal
void QBuffer::disconnectNotify(const QMetaMethod &signalMethod) const
{
   if (signalMethod.name() == "readyRead" || signalMethod.name() == "bytesWritten") {
      d_func()->signalConnectionCount--;
   }
}
