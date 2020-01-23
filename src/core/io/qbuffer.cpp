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

#include <qbuffer.h>
#include <qiodevice_p.h>

/** QBufferPrivate **/
class QBufferPrivate : public QIODevicePrivate
{
   Q_DECLARE_PUBLIC(QBuffer)

 public:
   QBufferPrivate()
      : buf(0), writtenSinceLastEmit(0), signalConnectionCount(0), signalsEmitted(false)

   { }
   ~QBufferPrivate() { }

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

/*!
    \class QBuffer
    \reentrant
    \brief The QBuffer class provides a QIODevice interface for a QByteArray.

    \ingroup io

    QBuffer allows you to access a QByteArray using the QIODevice
    interface. The QByteArray is treated just as a standard random-accessed
    file. Example:

    \snippet doc/src/snippets/buffer/buffer.cpp 0

    By default, an internal QByteArray buffer is created for you when
    you create a QBuffer. You can access this buffer directly by
    calling buffer(). You can also use QBuffer with an existing
    QByteArray by calling setBuffer(), or by passing your array to
    QBuffer's constructor.

    Call open() to open the buffer. Then call write() or
    putChar() to write to the buffer, and read(), readLine(),
    readAll(), or getChar() to read from it. size() returns the
    current size of the buffer, and you can seek to arbitrary
    positions in the buffer by calling seek(). When you are done with
    accessing the buffer, call close().

    The following code snippet shows how to write data to a
    QByteArray using QDataStream and QBuffer:

    \snippet doc/src/snippets/buffer/buffer.cpp 1

    Effectively, we convert the application's QPalette into a byte
    array. Here's how to read the data from the QByteArray:

    \snippet doc/src/snippets/buffer/buffer.cpp 2

    QTextStream and QDataStream also provide convenience constructors
    that take a QByteArray and that create a QBuffer behind the
    scenes.

    QBuffer emits readyRead() when new data has arrived in the
    buffer. By connecting to this signal, you can use QBuffer to
    store temporary data before processing it. For example, you can
    pass the buffer to QFtp when downloading a file from an FTP
    server. Whenever a new payload of data has been downloaded,
    readyRead() is emitted, and you can process the data that just
    arrived. QBuffer also emits bytesWritten() every time new data
    has been written to the buffer.

    \sa QFile, QDataStream, QTextStream, QByteArray
*/

/*!
    Constructs an empty buffer with the given \a parent. You can call
    setData() to fill the buffer with data, or you can open it in
    write mode and use write().

    \sa open()
*/
QBuffer::QBuffer(QObject *parent)
   : QIODevice(*new QBufferPrivate, parent)
{
   Q_D(QBuffer);

   d->buf = &d->defaultBuf;
   d->ioIndex = 0;
}

/*!
    Constructs a QBuffer that uses the QByteArray pointed to by \a
    byteArray as its internal buffer, and with the given \a parent.
    The caller is responsible for ensuring that \a byteArray remains
    valid until the QBuffer is destroyed, or until setBuffer() is
    called to change the buffer. QBuffer doesn't take ownership of
    the QByteArray.

    If you open the buffer in write-only mode or read-write mode and
    write something into the QBuffer, \a byteArray will be modified.

    Example:

    \snippet doc/src/snippets/buffer/buffer.cpp 3

    \sa open(), setBuffer(), setData()
*/
QBuffer::QBuffer(QByteArray *byteArray, QObject *parent)
   : QIODevice(*new QBufferPrivate, parent)
{
   Q_D(QBuffer);
   d->buf = byteArray ? byteArray : &d->defaultBuf;
   d->defaultBuf.clear();
   d->ioIndex = 0;
}


/*!
    Destroys the buffer.
*/

QBuffer::~QBuffer()
{
}

/*!
    Makes QBuffer uses the QByteArray pointed to by \a
    byteArray as its internal buffer. The caller is responsible for
    ensuring that \a byteArray remains valid until the QBuffer is
    destroyed, or until setBuffer() is called to change the buffer.
    QBuffer doesn't take ownership of the QByteArray.

    Does nothing if isOpen() is true.

    If you open the buffer in write-only mode or read-write mode and
    write something into the QBuffer, \a byteArray will be modified.

    Example:

    \snippet doc/src/snippets/buffer/buffer.cpp 4

    If \a byteArray is 0, the buffer creates its own internal
    QByteArray to work on. This byte array is initially empty.

    \sa buffer(), setData(), open()
*/

void QBuffer::setBuffer(QByteArray *byteArray)
{
   Q_D(QBuffer);
   if (isOpen()) {
      qWarning("QBuffer::setBuffer: Buffer is open");
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

/*!
    Returns a reference to the QBuffer's internal buffer. You can use
    it to modify the QByteArray behind the QBuffer's back.

    \sa setBuffer(), data()
*/

QByteArray &QBuffer::buffer()
{
   Q_D(QBuffer);
   return *d->buf;
}

/*!
    \overload

    This is the same as data().
*/

const QByteArray &QBuffer::buffer() const
{
   Q_D(const QBuffer);
   return *d->buf;
}


/*!
    Returns the data contained in the buffer.

    This is the same as buffer().

    \sa setData(), setBuffer()
*/

const QByteArray &QBuffer::data() const
{
   Q_D(const QBuffer);
   return *d->buf;
}

/*!
    Sets the contents of the internal buffer to be \a data. This is
    the same as assigning \a data to buffer().

    Does nothing if isOpen() is true.

    \sa setBuffer()
*/
void QBuffer::setData(const QByteArray &data)
{
   Q_D(QBuffer);
   if (isOpen()) {
      qWarning("QBuffer::setData: Buffer is open");
      return;
   }
   *d->buf = data;
   d->ioIndex = 0;
}

/*!
    \fn void QBuffer::setData(const char *data, int size)

    \overload

    Sets the contents of the internal buffer to be the first \a size
    bytes of \a data.
*/

/*!
   \reimp
*/
bool QBuffer::open(OpenMode flags)
{
   Q_D(QBuffer);

   if ((flags & (Append | Truncate)) != 0) {
      flags |= WriteOnly;
   }
   if ((flags & (ReadOnly | WriteOnly)) == 0) {
      qWarning("QBuffer::open: Buffer access not specified");
      return false;
   }

   if ((flags & Truncate) == Truncate) {
      d->buf->resize(0);
   }
   d->ioIndex = (flags & Append) == Append ? d->buf->size() : 0;

   return QIODevice::open(flags);
}

/*!
    \reimp
*/
void QBuffer::close()
{
   QIODevice::close();
}

/*!
    \reimp
*/
qint64 QBuffer::pos() const
{
   return QIODevice::pos();
}

/*!
    \reimp
*/
qint64 QBuffer::size() const
{
   Q_D(const QBuffer);
   return qint64(d->buf->size());
}

/*!
    \reimp
*/
bool QBuffer::seek(qint64 pos)
{
   Q_D(QBuffer);
   if (pos > d->buf->size() && isWritable()) {
      if (seek(d->buf->size())) {
         const qint64 gapSize = pos - d->buf->size();
         if (write(QByteArray(gapSize, 0)) != gapSize) {
            qWarning("QBuffer::seek: Unable to fill gap");
            return false;
         }
      } else {
         return false;
      }
   } else if (pos > d->buf->size() || pos < 0) {
      qWarning("QBuffer::seek: Invalid pos: %d", int(pos));
      return false;
   }
   d->ioIndex = int(pos);
   return QIODevice::seek(pos);
}

/*!
    \reimp
*/
bool QBuffer::atEnd() const
{
   return QIODevice::atEnd();
}

/*!
   \reimp
*/
bool QBuffer::canReadLine() const
{
   Q_D(const QBuffer);
   if (!isOpen()) {
      return false;
   }

   return d->buf->indexOf('\n', int(pos())) != -1 || QIODevice::canReadLine();
}

/*!
    \reimp
*/
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

/*!
    \reimp
*/
qint64 QBuffer::writeData(const char *data, qint64 len)
{
   Q_D(QBuffer);
   int extraBytes = d->ioIndex + len - d->buf->size();
   if (extraBytes > 0) { // overflow
      int newSize = d->buf->size() + extraBytes;
      d->buf->resize(newSize);
      if (d->buf->size() != newSize) { // could not resize
         qWarning("QBuffer::writeData: Memory allocation error");
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

