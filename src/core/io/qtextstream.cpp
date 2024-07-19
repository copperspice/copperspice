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

#include <qtextstream.h>

#include <qbuffer.h>
#include <qfile.h>
#include <qnumeric.h>

#ifndef QT_NO_TEXTCODEC
#include <qtextcodec.h>
#endif

#include <qlocale_p.h>

#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <new>
#include <stdlib.h>

static constexpr const int QTEXTSTREAM_BUFFERSIZE = 16384;

#define CHECK_VALID_STREAM(x) do { \
      if (! d->m_string && ! d->device) { \
         qWarning("QTextStream() No device was available"); \
         return x; \
      } } while (false)

// Base implementations of operator>> for ints and reals
#define IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(type) do { \
      Q_D(QTextStream); \
      CHECK_VALID_STREAM(*this); \
      quint64 tmp; \
      switch (d->getNumber(&tmp)) { \
         case QTextStreamPrivate::npsOk: \
            i = (type)tmp; \
            break; \
         case QTextStreamPrivate::npsMissingDigit: \
         case QTextStreamPrivate::npsInvalidPrefix: \
            i = (type)0; \
            setStatus(atEnd() ? QTextStream::ReadPastEnd : QTextStream::ReadCorruptData); \
            break; \
      } \
      return *this; } while (false)

#define IMPLEMENT_STREAM_RIGHT_REAL_OPERATOR(type) do { \
      Q_D(QTextStream); \
      CHECK_VALID_STREAM(*this); \
      double tmp; \
      if (d->getReal(&tmp)) { \
         f = (type)tmp; \
      } else { \
         f = (type)0; \
         setStatus(atEnd() ? QTextStream::ReadPastEnd : QTextStream::ReadCorruptData); \
      } \
      return *this; } while (false)

class QDeviceClosedNotifier : public QObject
{
   CORE_CS_OBJECT(QDeviceClosedNotifier)

 public:
   QDeviceClosedNotifier()
   { }

   void setupDevice(QTextStream *newStream, QIODevice *device) {
      disconnect();

      if (device != nullptr) {
         connect(device, &QIODevice::aboutToClose, this, &QDeviceClosedNotifier::flushStream);
      }

      m_stream = newStream;
   }

   CORE_CS_SLOT_1(Public, void flushStream())
   CORE_CS_SLOT_2(flushStream)

 private:
   QTextStream *m_stream;
};

void  QDeviceClosedNotifier::flushStream()
{
   m_stream->flush();
}

class QTextStreamPrivate
{
   Q_DECLARE_PUBLIC(QTextStream)

 public:
   enum TokenDelimiter {
      Space,
      NotSpace,
      EndOfLine
   };

   QTextStreamPrivate(QTextStream *stream);
   ~QTextStreamPrivate();

   void reset();

   QIODevice *device;

   QDeviceClosedNotifier deviceClosedNotifier;
   bool deleteDevice;

   QString *m_string;

   int stringOffset;
   QIODevice::OpenMode stringOpenMode;

#ifndef QT_NO_TEXTCODEC
   QTextCodec *codec;
   QTextCodec::ConverterState readConverterState;
   QTextCodec::ConverterState writeConverterState;
   QTextCodec::ConverterState *readConverterSavedState;
   bool autoDetectUnicode;
#endif

   QString read(int maxlen);
   bool scan(QString *newToken, int maxlen, TokenDelimiter delimiter);

   inline QString::const_iterator readPtr() const;
   inline void consumeLastToken();
   inline void consume(int nchars);

   void saveConverterState(qint64 newPos);
   void restoreToSavedConverterState();
   int lastTokenSize;

   // Return value type for getNumber()
   enum NumberParsingStatus {
      npsOk,
      npsMissingDigit,
      npsInvalidPrefix
   };

   inline bool getChar(QChar *ch);
   inline void ungetChar(const QChar &ch);
   NumberParsingStatus getNumber(quint64 *l);
   bool getReal(double *f);

   inline void write(const QString &data);
   inline void putString(const QString &ch, bool number = false);
   void putNumber(quint64 number, bool negative);

   bool fillReadBuffer(qint64 maxBytes = -1);
   void resetReadBuffer();
   void flushWriteBuffer();

   QString writeBuffer;
   QString readBuffer;

   int readBufferOffset;
   int readConverterSavedStateOffset; //the offset between readBufferStartDevicePos and that start of the buffer
   qint64 readBufferStartDevicePos;

   // parameters
   int realNumberPrecision;
   int integerBase;
   int fieldWidth;
   QChar padChar;
   QTextStream::FieldAlignment fieldAlignment;
   QTextStream::RealNumberNotation realNumberNotation;
   QTextStream::NumberFlags numberFlags;

   QTextStream::Status status;
   QLocale locale;
   QTextStream *q_ptr;
};

QTextStreamPrivate::QTextStreamPrivate(QTextStream *stream)

#ifndef QT_NO_TEXTCODEC
   :  readConverterSavedState(nullptr), readConverterSavedStateOffset(0), locale(QLocale::c())
# else
   : readConverterSavedStateOffset(0), locale(QLocale::c())
#endif

{
   this->q_ptr = stream;
   reset();
}

QTextStreamPrivate::~QTextStreamPrivate()
{
   if (deleteDevice) {
      device->blockSignals(true);
      delete device;
   }

#ifndef QT_NO_TEXTCODEC
   delete readConverterSavedState;
#endif
}

#ifndef QT_NO_TEXTCODEC
static void resetCodecConverterStateHelper(QTextCodec::ConverterState *state)
{
   state->~ConverterState();
   new (state) QTextCodec::ConverterState;
}

static void copyConverterStateHelper(QTextCodec::ConverterState *dest, const QTextCodec::ConverterState *src)
{
   // copies the structure manually
   Q_ASSERT(! src->m_data);

   dest->m_flags       = src->m_flags;
   dest->invalidChars  = src->invalidChars;
   dest->state_data[0] = src->state_data[0];
   dest->state_data[1] = src->state_data[1];
   dest->state_data[2] = src->state_data[2];
}
#endif

QTextStream::Params QTextStream::getParams() const
{
   Q_D(const QTextStream);

   Params retval;

   retval.p_realNumberPrecision = d->realNumberPrecision;
   retval.p_integerBase         = d->integerBase;
   retval.p_fieldWidth          = d->fieldWidth;
   retval.p_padChar             = d->padChar;
   retval.p_fieldAlignment      = d->fieldAlignment;
   retval.p_realNumberNotation  = d->realNumberNotation;
   retval.p_numberFlags         = d->numberFlags;

   return retval;
}

void QTextStream::setParams(const Params &data)
{
   Q_D(QTextStream);

   d->realNumberPrecision = data.p_realNumberPrecision;
   d->integerBase         = data.p_integerBase;
   d->fieldWidth          = data.p_fieldWidth;
   d->padChar             = data.p_padChar;
   d->fieldAlignment      = data.p_fieldAlignment;
   d->realNumberNotation  = data.p_realNumberNotation;
   d->numberFlags         = data.p_numberFlags;
}

void QTextStreamPrivate::reset()
{
   realNumberPrecision = 6;
   integerBase         = 0;
   fieldWidth          = 0;
   padChar             = ' ';
   fieldAlignment      = QTextStream::AlignRight;
   realNumberNotation  = QTextStream::SmartNotation;
   numberFlags         = Qt::EmptyFlag;

   device       = nullptr;
   deleteDevice = false;
   m_string     = nullptr;
   stringOffset = 0;
   stringOpenMode = QIODevice::NotOpen;

   readBufferOffset = 0;
   readBufferStartDevicePos = 0;
   lastTokenSize = 0;

#ifndef QT_NO_TEXTCODEC
   codec = QTextCodec::codecForLocale();
   resetCodecConverterStateHelper(&readConverterState);
   resetCodecConverterStateHelper(&writeConverterState);

   delete readConverterSavedState;

   readConverterSavedState = nullptr;
   writeConverterState.m_flags |= QTextCodec::IgnoreHeader;
   autoDetectUnicode = true;
#endif
}

bool QTextStreamPrivate::fillReadBuffer(qint64 maxBytes)
{
   // no buffer next to the QString itself; this function should only be called internally, for devices.
   Q_ASSERT(! m_string);
   Q_ASSERT(device);

   // handle text translation and bypass the Text flag in the device.
   bool textModeEnabled = device->isTextModeEnabled();

   if (textModeEnabled) {
      device->setTextModeEnabled(false);
   }

   // read raw data into a temporary buffer
   char buf[QTEXTSTREAM_BUFFERSIZE];
   qint64 bytesRead = 0;

#if defined(Q_OS_WIN)
   // On Windows there is no non-blocking stdin so fall back to reading lines
   // If there is no QOBJECT then read lines for all sequential devices, otherwise, we read lines only for stdin

   QFile *file = nullptr;

   if (device->isSequential() && (file = dynamic_cast<QFile *>(device)) && file->handle() == 0 ) {
      if (maxBytes != -1) {
         bytesRead = device->readLine(buf, qMin(static_cast<qint64>(sizeof(buf)), maxBytes));
      } else {
         bytesRead = device->readLine(buf, sizeof(buf));
      }

   } else

#endif
   {
      if (maxBytes != -1) {
         bytesRead = device->read(buf, qMin(static_cast<qint64>(sizeof(buf)), maxBytes));
      } else {
         bytesRead = device->read(buf, sizeof(buf));
      }
   }

   // reset the Text flag
   if (textModeEnabled) {
      device->setTextModeEnabled(true);
   }

   if (bytesRead <= 0)  {
      return false;
   }

#ifndef QT_NO_TEXTCODEC
   // codec auto detection, explicitly defaults to locale encoding if the codec has been set to 0.

   if (! codec || autoDetectUnicode) {
      autoDetectUnicode = false;

      codec = QTextCodec::codecForUtfText(QByteArray::fromRawData(buf, bytesRead), codec);

      if (! codec) {
         codec = QTextCodec::codecForLocale();
         writeConverterState.m_flags |= QTextCodec::IgnoreHeader;
      }
   }

#endif

   QString tmpBuffer;

#ifndef QT_NO_TEXTCODEC
   // convert to unicode
   tmpBuffer = codec->toUnicode(buf, bytesRead, &readConverterState);
#else
   tmpBuffer = QString::fromUtf8(buf, bytesRead);
#endif

   if (! tmpBuffer.isEmpty() && textModeEnabled) {
      tmpBuffer.replace("\r", "");
   }

   readBuffer += tmpBuffer;

   return true;
}

void QTextStreamPrivate::resetReadBuffer()
{
   readBuffer.clear();
   readBufferOffset = 0;
   readBufferStartDevicePos = (device ? device->pos() : 0);
}

void QTextStreamPrivate::flushWriteBuffer()
{
   // no buffer next to the QString itself, should only be called internally for devices.
   if (m_string || ! device) {
      return;
   }

   // Stream is gone, appending further data may succeed but would create a damaged stream
   if (status != QTextStream::Ok) {
      return;
   }

   if (writeBuffer.isEmpty()) {
      return;
   }

#if defined (Q_OS_WIN)
   // handle text translation and bypass the Text flag in the device.
   bool textModeEnabled = device->isTextModeEnabled();

   if (textModeEnabled) {
      device->setTextModeEnabled(false);
      writeBuffer.replace('\n', "\r\n");
   }

#endif

#ifndef QT_NO_TEXTCODEC

   if (! codec) {
      codec = QTextCodec::codecForLocale();
   }

   // convert from unicode to raw data
   QByteArray data = codec->fromUnicode(writeBuffer, &writeConverterState);

#else
   QByteArray data = writeBuffer.toUtf8();

#endif

   writeBuffer.clear();

   // write raw data to the device
   qint64 bytesWritten = device->write(data);

#if defined (Q_OS_WIN)

   // replace the text flag
   if (textModeEnabled) {
      device->setTextModeEnabled(true);
   }

#endif

   if (bytesWritten <= 0) {
      status = QTextStream::WriteFailed;
      return;
   }

   // flush the file
   QFileDevice *file = qobject_cast<QFileDevice *>(device);
   bool flushed = !file || file->flush();

   if (! flushed || bytesWritten != qint64(data.size())) {
      status = QTextStream::WriteFailed;
   }
}

QString QTextStreamPrivate::read(int maxlen)
{
   QString ret;

   if (m_string) {
      lastTokenSize = qMin(maxlen, m_string->size() - stringOffset);
      ret = m_string->mid(stringOffset, lastTokenSize);

   } else {
      while (readBuffer.size() - readBufferOffset < maxlen && fillReadBuffer()) ;

      lastTokenSize = qMin(maxlen, readBuffer.size() - readBufferOffset);
      ret = readBuffer.mid(readBufferOffset, lastTokenSize);
   }

   consumeLastToken();

   return ret;
}

bool QTextStreamPrivate::scan(QString *newToken, int maxlen, TokenDelimiter delimiter)
{
   bool consumeDelimiter       = false;
   bool foundToken             = false;
   bool canStillReadFromDevice = true;

   int totalSize   = 0;
   int delimSize   = 0;
   int startOffset = device ? readBufferOffset : stringOffset;

   QChar lastChar;

   do {
      QString::const_iterator iter_begin;
      QString::const_iterator iter_end;

      if (device) {
         iter_begin = readBuffer.begin();
         iter_end   = readBuffer.end();

      } else {
         iter_begin = m_string->begin();
         iter_end   = m_string->end();
      }

      iter_begin += startOffset;

      for (; ! foundToken && iter_begin != iter_end && (! maxlen || totalSize < maxlen); ++startOffset) {
         const QChar ch = *iter_begin;

         ++iter_begin;
         ++totalSize;

         switch (delimiter) {
            case Space:
               if (ch.isSpace()) {
                  foundToken = true;
                  delimSize = 1;
               }

               break;

            case NotSpace:
               if (!ch.isSpace()) {
                  foundToken = true;
                  delimSize = 1;
               }

               break;

            case EndOfLine:
               if (ch == '\n') {
                  foundToken = true;
                  delimSize = (lastChar == '\r') ? 2 : 1;
                  consumeDelimiter = true;
               }

               lastChar = ch;
               break;
         }
      }

   } while (! foundToken && (! maxlen || totalSize < maxlen) && (device && (canStillReadFromDevice = fillReadBuffer())));

   // if the token was not found but we reached the end of input,
   // then we accept what we got. if we are not at the end of input, we return false.

   if (! foundToken && (! maxlen || totalSize < maxlen) && (totalSize == 0
         || (m_string && stringOffset + totalSize < m_string->size())
         || (device && ! device->atEnd() && canStillReadFromDevice))) {

      return false;
   }

   // if we find a '\r' at the end of the data when reading lines,
   // don't make it part of the line.

   if (delimiter == EndOfLine && totalSize > 0 && ! foundToken) {
      if (((m_string && stringOffset + totalSize == m_string->size()) || (device && device->atEnd())) && lastChar == '\r') {
         consumeDelimiter = true;
         ++delimSize;
      }
   }

   // set the read offset and length of the token
   if (newToken != nullptr) {
      QString::const_iterator tmpIter = readPtr();
      *newToken = QString(tmpIter, tmpIter + totalSize - delimSize);
   }

   // update last token size. the callee will call consumeLastToken() when done
   lastTokenSize = totalSize;

   if (! consumeDelimiter) {
      lastTokenSize -= delimSize;
   }

   return true;
}

inline QString::const_iterator QTextStreamPrivate::readPtr() const
{
   Q_ASSERT(readBufferOffset <= readBuffer.size());

   if (m_string) {
      return m_string->begin() + stringOffset;
   }

   return readBuffer.begin() + readBufferOffset;
}

inline void QTextStreamPrivate::consumeLastToken()
{
   if (lastTokenSize) {
      consume(lastTokenSize);
   }

   lastTokenSize = 0;
}

inline void QTextStreamPrivate::consume(int size)
{
   if (m_string) {
      stringOffset += size;

      if (stringOffset > m_string->size()) {
         stringOffset = m_string->size();
      }

   } else {
      readBufferOffset += size;

      if (readBufferOffset >= readBuffer.size()) {
         readBufferOffset = 0;
         readBuffer.clear();
         saveConverterState(device->pos());

      } else if (readBufferOffset > QTEXTSTREAM_BUFFERSIZE) {
         readBuffer = readBuffer.remove(0, readBufferOffset);
         readConverterSavedStateOffset += readBufferOffset;
         readBufferOffset = 0;
      }
   }
}

inline void QTextStreamPrivate::saveConverterState(qint64 newPos)
{
#ifndef QT_NO_TEXTCODEC

   if (readConverterState.m_data) {
      // converter cannot be copied, so don't save anything
      // don't update readBufferStartDevicePos either
      return;
   }

   if (!readConverterSavedState) {
      readConverterSavedState = new QTextCodec::ConverterState;
   }

   copyConverterStateHelper(readConverterSavedState, &readConverterState);
#endif

   readBufferStartDevicePos = newPos;
   readConverterSavedStateOffset = 0;
}

inline void QTextStreamPrivate::restoreToSavedConverterState()
{
#ifndef QT_NO_TEXTCODEC

   if (readConverterSavedState) {
      // we have a saved state
      // that means the converter can be copied
      copyConverterStateHelper(&readConverterState, readConverterSavedState);

   } else {
      // the only state we could save was the initial
      // so reset to that
      resetCodecConverterStateHelper(&readConverterState);
   }

#endif
}

inline void QTextStreamPrivate::write(const QString &data)
{
   if (m_string) {
      // ### What about seek()??
      m_string->append(data);

   } else {
      writeBuffer += data;

      if (writeBuffer.size_storage() > QTEXTSTREAM_BUFFERSIZE) {
         flushWriteBuffer();
      }
   }
}

inline bool QTextStreamPrivate::getChar(QChar *ch)
{
   if ((m_string && stringOffset == m_string->size()) || (device && readBuffer.isEmpty() && ! fillReadBuffer())) {

      if (ch) {
         *ch = 0;
      }

      return false;
   }

   if (ch) {
      *ch = *readPtr();
   }

   consume(1);
   return true;
}

inline void QTextStreamPrivate::ungetChar(const QChar &ch)
{
   if (m_string != nullptr) {
      if (stringOffset == 0) {
         m_string->prepend(ch);

      } else {
         --stringOffset;
         m_string->replace(stringOffset, 1, ch);

      }

      return;
   }

   if (readBufferOffset == 0) {
      readBuffer.prepend(ch);
      return;
   }

   --readBufferOffset;
   readBuffer.replace(readBufferOffset, 1, ch);
}

inline void QTextStreamPrivate::putString(const QString &s, bool number)
{
   QString tmp = s;

   // handle padding
   int padSize = fieldWidth - s.size();

   if (padSize > 0) {
      QString pad(padSize, padChar);

      if (fieldAlignment == QTextStream::AlignLeft) {
         tmp.append(QString(padSize, padChar));

      } else if (fieldAlignment == QTextStream::AlignRight || fieldAlignment == QTextStream::AlignAccountingStyle) {
         tmp.prepend(QString(padSize, padChar));

         if (fieldAlignment == QTextStream::AlignAccountingStyle && number) {

            if (s.startsWith(locale.negativeSign()))  {
               const QChar sign = s.at(0);

               tmp.replace(padSize, 1, tmp.at(0));
               tmp.replace(0, 1, sign);
            }

            if (s.startsWith(locale.positiveSign()))  {
               const QChar sign = s.at(0);

               tmp.replace(padSize, 1, tmp.at(0));
               tmp.replace(0, 1, sign);
            }
         }

      } else if (fieldAlignment == QTextStream::AlignCenter) {
         tmp.prepend(QString(padSize / 2, padChar));
         tmp.append(QString(padSize - padSize / 2, padChar));
      }
   }

   write(tmp);
}

QTextStream::QTextStream()
   : d_ptr(new QTextStreamPrivate(this))
{
   Q_D(QTextStream);
   d->status = Ok;
}

QTextStream::QTextStream(QIODevice *device)
   : d_ptr(new QTextStreamPrivate(this))
{
#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QTextStream::QTextStream(QIODevice *device == *%p)", device);
#endif

   Q_D(QTextStream);
   d->device = device;

   d->deviceClosedNotifier.setupDevice(this, d->device);
   d->status = Ok;
}

QTextStream::QTextStream(QString *str, QIODevice::OpenMode openMode)
   : d_ptr(new QTextStreamPrivate(this))
{
   Q_D(QTextStream);

   d->m_string = str;
   d->stringOpenMode = openMode;
   d->status = Ok;
}

QTextStream::QTextStream(QByteArray *array, QIODevice::OpenMode openMode)
   : d_ptr(new QTextStreamPrivate(this))
{
   Q_D(QTextStream);
   d->device = new QBuffer(array);
   d->device->open(openMode);
   d->deleteDevice = true;
   d->deviceClosedNotifier.setupDevice(this, d->device);
   d->status = Ok;
}

QTextStream::QTextStream(const QByteArray &array, QIODevice::OpenMode openMode)
   : d_ptr(new QTextStreamPrivate(this))
{
   QBuffer *buffer = new QBuffer;
   buffer->setData(array);
   buffer->open(openMode);

   Q_D(QTextStream);
   d->device = buffer;
   d->deleteDevice = true;
   d->deviceClosedNotifier.setupDevice(this, d->device);
   d->status = Ok;
}

QTextStream::QTextStream(FILE *fileHandle, QIODevice::OpenMode openMode)
   : d_ptr(new QTextStreamPrivate(this))
{
#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QTextStream::QTextStream(FILE *fileHandle = %p, openMode = %d)", fileHandle, int(openMode));
#endif

   QFile *file = new QFile;
   file->open(fileHandle, openMode);

   Q_D(QTextStream);
   d->device       = file;
   d->deleteDevice = true;

   d->deviceClosedNotifier.setupDevice(this, d->device);

   d->status       = Ok;
}

QTextStream::~QTextStream()
{
   Q_D(QTextStream);

   if (! d->writeBuffer.isEmpty()) {
      d->flushWriteBuffer();
   }
}

void QTextStream::reset()
{
   Q_D(QTextStream);

   d->realNumberPrecision = 6;
   d->integerBase = 0;
   d->fieldWidth = 0;
   d->padChar = ' ';
   d->fieldAlignment = QTextStream::AlignRight;
   d->realNumberNotation = QTextStream::SmartNotation;
   d->numberFlags = Qt::EmptyFlag;
}

void QTextStream::flush()
{
   Q_D(QTextStream);
   d->flushWriteBuffer();
}

bool QTextStream::seek(qint64 pos)
{
   Q_D(QTextStream);

   d->lastTokenSize = 0;

   if (d->device) {
      // Empty the write buffer
      d->flushWriteBuffer();

      if (!d->device->seek(pos)) {
         return false;
      }

      d->resetReadBuffer();

#ifndef QT_NO_TEXTCODEC
      // Reset the codec converter states.
      resetCodecConverterStateHelper(&d->readConverterState);
      resetCodecConverterStateHelper(&d->writeConverterState);
      delete d->readConverterSavedState;
      d->readConverterSavedState = nullptr;
      d->writeConverterState.m_flags |= QTextCodec::IgnoreHeader;
#endif
      return true;
   }

   // string
   if (d->m_string && pos <= d->m_string->size()) {
      d->stringOffset = int(pos);
      return true;
   }

   return false;
}

qint64 QTextStream::pos() const
{
   Q_D(const QTextStream);

   if (d->device) {
      // Cutoff
      if (d->readBuffer.isEmpty()) {
         return d->device->pos();
      }

      if (d->device->isSequential()) {
         return 0;
      }

      // Seek the device
      if (!d->device->seek(d->readBufferStartDevicePos)) {
         return qint64(-1);
      }

      // Reset the read buffer
      QTextStreamPrivate *thatd = const_cast<QTextStreamPrivate *>(d);
      thatd->readBuffer.clear();

#ifndef QT_NO_TEXTCODEC
      thatd->restoreToSavedConverterState();

      if (d->readBufferStartDevicePos == 0) {
         thatd->autoDetectUnicode = true;
      }

#endif

      // Rewind the device to get to the current position Ensure that
      // readBufferOffset is unaffected by fillReadBuffer()
      int oldReadBufferOffset = d->readBufferOffset + d->readConverterSavedStateOffset;

      while (d->readBuffer.size() < oldReadBufferOffset) {
         if (! thatd->fillReadBuffer(1)) {
            return qint64(-1);
         }
      }

      thatd->readBufferOffset = oldReadBufferOffset;
      thatd->readConverterSavedStateOffset = 0;

      // Return the device position.
      return d->device->pos();
   }

   if (d->m_string) {
      return d->stringOffset;
   }

   qWarning("QTextStream::pos() No device was provided");

   return qint64(-1);
}

void QTextStream::skipWhiteSpace()
{
   Q_D(QTextStream);

   d->scan(nullptr, 0, QTextStreamPrivate::NotSpace);
   d->consumeLastToken();
}

void QTextStream::setDevice(QIODevice *device)
{
   Q_D(QTextStream);

   flush();

   if (d->deleteDevice) {
      d->deviceClosedNotifier.disconnect();
      delete d->device;
      d->deleteDevice = false;
   }

   d->reset();
   d->status = Ok;
   d->device = device;
   d->resetReadBuffer();
   d->deviceClosedNotifier.setupDevice(this, d->device);
}

QIODevice *QTextStream::device() const
{
   Q_D(const QTextStream);
   return d->device;
}

void QTextStream::setString(QString *str, QIODevice::OpenMode openMode)
{
   Q_D(QTextStream);
   flush();

   if (d->deleteDevice) {

      d->deviceClosedNotifier.disconnect();
      d->device->blockSignals(true);
      delete d->device;
      d->deleteDevice = false;
   }

   d->reset();
   d->status = Ok;
   d->m_string = str;
   d->stringOpenMode = openMode;
}

QString *QTextStream::string() const
{
   Q_D(const QTextStream);
   return d->m_string;
}

void QTextStream::setFieldAlignment(FieldAlignment mode)
{
   Q_D(QTextStream);
   d->fieldAlignment = mode;
}

QTextStream::FieldAlignment QTextStream::fieldAlignment() const
{
   Q_D(const QTextStream);
   return d->fieldAlignment;
}

void QTextStream::setPadChar(QChar ch)
{
   Q_D(QTextStream);
   d->padChar = ch;
}

QChar QTextStream::padChar() const
{
   Q_D(const QTextStream);
   return d->padChar;
}

void QTextStream::setFieldWidth(int width)
{
   Q_D(QTextStream);
   d->fieldWidth = width;
}

int QTextStream::fieldWidth() const
{
   Q_D(const QTextStream);
   return d->fieldWidth;
}

void QTextStream::setNumberFlags(NumberFlags flags)
{
   Q_D(QTextStream);
   d->numberFlags = flags;
}

QTextStream::NumberFlags QTextStream::numberFlags() const
{
   Q_D(const QTextStream);
   return d->numberFlags;
}

void QTextStream::setIntegerBase(int base)
{
   Q_D(QTextStream);
   d->integerBase = base;
}

int QTextStream::integerBase() const
{
   Q_D(const QTextStream);
   return d->integerBase;
}

void QTextStream::setRealNumberNotation(RealNumberNotation notation)
{
   Q_D(QTextStream);
   d->realNumberNotation = notation;
}

QTextStream::RealNumberNotation QTextStream::realNumberNotation() const
{
   Q_D(const QTextStream);
   return d->realNumberNotation;
}

void QTextStream::setRealNumberPrecision(int precision)
{
   Q_D(QTextStream);

   if (precision < 0) {
      qWarning("QTextStream::setRealNumberPrecision() Invalid precision value, %d", precision);
      d->realNumberPrecision = 6;
      return;
   }

   d->realNumberPrecision = precision;
}

int QTextStream::realNumberPrecision() const
{
   Q_D(const QTextStream);
   return d->realNumberPrecision;
}

QTextStream::Status QTextStream::status() const
{
   Q_D(const QTextStream);
   return d->status;
}

void QTextStream::resetStatus()
{
   Q_D(QTextStream);
   d->status = Ok;
}

void QTextStream::setStatus(Status status)
{
   Q_D(QTextStream);

   if (d->status == Ok) {
      d->status = status;
   }
}

bool QTextStream::atEnd() const
{
   Q_D(const QTextStream);
   CHECK_VALID_STREAM(true);

   if (d->m_string) {
      return d->m_string->size() == d->stringOffset;
   }

   return d->readBuffer.isEmpty() && d->device->atEnd();
}

QString QTextStream::readAll()
{
   Q_D(QTextStream);
   CHECK_VALID_STREAM(QString());

   return d->read(INT_MAX);
}

QString QTextStream::readLine(qint64 maxlen)
{
   QString retval;
   readLineInto(&retval, maxlen);

   return retval;
}

bool QTextStream::readLineInto(QString *line, qint64 maxlen)
{
   Q_D(QTextStream);

   if (d->m_string == nullptr && d->device == nullptr) {
      qWarning("QTextStream::readLineInto() No device was provided");

      if (line != nullptr && ! line->isEmpty()) {
         line->clear();
      }

      return false;
   }

   if (! d->scan(line, int(maxlen), QTextStreamPrivate::EndOfLine)) {
      if (line != nullptr && ! line->isEmpty()) {
         line->clear();
      }

      return false;
   }

   d->consumeLastToken();

   return true;
}

QString QTextStream::read(qint64 maxlen)
{
   Q_D(QTextStream);
   CHECK_VALID_STREAM(QString());

   if (maxlen <= 0) {
      return QString();
   }

   return d->read(int(maxlen));
}

QTextStreamPrivate::NumberParsingStatus QTextStreamPrivate::getNumber(quint64 *ret)
{
   scan(nullptr, 0, NotSpace);
   consumeLastToken();

   // detect int encoding
   int base = integerBase;

   if (base == 0) {
      QChar ch;

      if (!getChar(&ch)) {
         return npsInvalidPrefix;
      }

      if (ch == '0') {
         QChar ch2;

         if (! getChar(&ch2)) {
            // Result is the number 0
            *ret = 0;
            return npsOk;
         }

         ch2 = ch2.toLower()[0];

         if (ch2 == 'x') {
            base = 16;

         } else if (ch2 == 'b') {
            base = 2;

         } else if (ch2.isDigit() && ch2.digitValue() >= 0 && ch2.digitValue() <= 7) {
            base = 8;

         } else {
            base = 10;
         }

         ungetChar(ch2);

      } else if (ch == locale.negativeSign() || ch == locale.positiveSign() || ch.isDigit()) {
         base = 10;

      } else {
         ungetChar(ch);
         return npsInvalidPrefix;
      }

      ungetChar(ch);
      // State of the stream is now the same as on entry
      // (cursor is at prefix),
      // and local variable 'base' has been set appropriately.
   }

   quint64 val = 0;

   switch (base) {
      case 2: {
         QChar pf1;
         QChar pf2;
         QChar dig;

         // Parse prefix '0b'
         if (! getChar(&pf1) || pf1 != '0') {
            return npsInvalidPrefix;
         }

         if (! getChar(&pf2) || pf2.toLower()[0] != 'b') {
            return npsInvalidPrefix;
         }

         // Parse digits
         int ndigits = 0;

         while (getChar(&dig)) {

            if (dig == '0') {
               val = val << 1;

            } else if (dig == '1') {
               val = val << 1;
               val += 1;

            } else {
               ungetChar(dig);
               break;
            }

            ndigits++;
         }

         if (ndigits == 0) {
            // Unwind the prefix and abort
            ungetChar(pf2);
            ungetChar(pf1);

            return npsMissingDigit;
         }

         break;
      }

      case 8: {
         QChar pf;
         QChar dig;

         // Parse prefix '0'
         if (! getChar(&pf) || pf != '0') {
            return npsInvalidPrefix;
         }

         // Parse digits
         int ndigits = 0;

         while (getChar(&dig)) {
            int n = dig.unicode();

            if (n >= '0' && n <= '7') {
               val *= 8;
               val += n - '0';

            } else {
               ungetChar(dig);
               break;
            }

            ndigits++;
         }

         if (ndigits == 0) {
            // Unwind the prefix and abort
            ungetChar(pf);
            return npsMissingDigit;
         }

         break;
      }

      case 10: {
         // Parse sign (or first digit)
         QChar sign;
         int ndigits = 0;

         if (! getChar(&sign)) {
            return npsMissingDigit;
         }

         if (sign != locale.negativeSign() && sign != locale.positiveSign()) {
            if (! sign.isDigit()) {
               ungetChar(sign);
               return npsMissingDigit;
            }

            val += sign.digitValue();
            ndigits++;
         }

         // Parse digits
         QChar ch;

         while (getChar(&ch)) {

            if (ch.isDigit()) {
               val *= 10;
               val += ch.digitValue();

            } else if (locale != QLocale::c() && ch == locale.groupSeparator()) {
               continue;

            } else {
               ungetChar(ch);
               break;
            }

            ndigits++;
         }

         if (ndigits == 0) {
            return npsMissingDigit;
         }

         if (sign == locale.negativeSign()) {
            qint64 ival = qint64(val);

            if (ival > 0) {
               ival = -ival;
            }

            val = quint64(ival);
         }

         break;
      }

      case 16: {
         QChar pf1;
         QChar pf2;
         QChar dig;

         // Parse prefix ' 0x'
         if (! getChar(&pf1) || pf1 != '0') {
            return npsInvalidPrefix;
         }

         if (! getChar(&pf2) || pf2.toLower()[0] != 'x') {
            return npsInvalidPrefix;
         }

         // Parse digits
         int ndigits = 0;

         while (getChar(&dig)) {
            int n = dig.toLower()[0].unicode();

            if (n >= '0' && n <= '9') {
               val <<= 4;
               val += n - '0';

            } else if (n >= 'a' && n <= 'f') {
               val <<= 4;
               val += 10 + (n - 'a');

            } else {
               ungetChar(dig);
               break;
            }

            ndigits++;
         }

         if (ndigits == 0) {
            return npsMissingDigit;
         }

         break;
      }

      default:
         // Unsupported integerBase
         return npsInvalidPrefix;
   }

   if (ret) {
      *ret = val;
   }

   return npsOk;
}

bool QTextStreamPrivate::getReal(double *f)
{
   // use a table-driven FSM to parse floating point numbers, strtod() can not be
   // used directly since we may be reading from a QIODevice.

   enum ParserState {
      Init = 0,
      Sign = 1,
      Mantissa = 2,
      Dot = 3,
      Abscissa = 4,
      ExpMark = 5,
      ExpSign = 6,
      Exponent = 7,
      Nan1 = 8,
      Nan2 = 9,
      Inf1 = 10,
      Inf2 = 11,
      NanInf = 12,
      Done = 13
   };

   enum InputToken {
      None = 0,
      InputSign = 1,
      InputDigit = 2,
      InputDot = 3,
      InputExp = 4,
      InputI = 5,
      InputN = 6,
      InputF = 7,
      InputA = 8,
      InputT = 9
   };

   static const uchar table[13][10] = {
      // None InputSign InputDigit InputDot InputExp InputI    InputN    InputF    InputA    InputT
      { 0,    Sign,     Mantissa,  Dot,     0,       Inf1,     Nan1,     0,        0,        0      }, // 0  Init
      { 0,    0,        Mantissa,  Dot,     0,       Inf1,     Nan1,     0,        0,        0      }, // 1  Sign
      { Done, Done,     Mantissa,  Dot,     ExpMark, 0,        0,        0,        0,        0      }, // 2  Mantissa
      { 0,    0,        Abscissa,  0,       0,       0,        0,        0,        0,        0      }, // 3  Dot
      { Done, Done,     Abscissa,  Done,    ExpMark, 0,        0,        0,        0,        0      }, // 4  Abscissa
      { 0,    ExpSign,  Exponent,  0,       0,       0,        0,        0,        0,        0      }, // 5  ExpMark
      { 0,    0,        Exponent,  0,       0,       0,        0,        0,        0,        0      }, // 6  ExpSign
      { Done, Done,     Exponent,  Done,    Done,    0,        0,        0,        0,        0      }, // 7  Exponent
      { 0,    0,        0,         0,       0,       0,        0,        0,        Nan2,     0      }, // 8  Nan1
      { 0,    0,        0,         0,       0,       0,        NanInf,   0,        0,        0      }, // 9  Nan2
      { 0,    0,        0,         0,       0,       0,        Inf2,     0,        0,        0      }, // 10 Inf1
      { 0,    0,        0,         0,       0,       0,        0,        NanInf,   0,        0      }, // 11 Inf2
      { Done, 0,        0,         0,       0,       0,        0,        0,        0,        0      }, // 11 NanInf
   };

   ParserState state = Init;
   InputToken input = None;

   scan(nullptr, 0, NotSpace);
   consumeLastToken();

   const int BufferSize = 128;
   char buf[BufferSize];
   int i = 0;

   QChar c;

   while (getChar(&c)) {
      switch (c.unicode()) {
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
            input = InputDigit;
            break;

         case 'i':
         case 'I':
            input = InputI;
            break;

         case 'n':
         case 'N':
            input = InputN;
            break;

         case 'f':
         case 'F':
            input = InputF;
            break;

         case 'a':
         case 'A':
            input = InputA;
            break;

         case 't':
         case 'T':
            input = InputT;
            break;

         default: {
            QString lc = c.toLower();

            if (lc == locale.decimalPoint().toLower()) {
               input = InputDot;

            } else if (lc == locale.exponential().toLower()) {
               input = InputExp;

            } else if (lc == locale.negativeSign().toLower() || lc == locale.positiveSign().toLower()) {
               input = InputSign;

            } else if (locale != QLocale::c() && lc == locale.groupSeparator().toLower()) {
               // backward compatibility, not a digit
               input = InputDigit;

            } else {
               input = None;
            }
         }
         break;
      }

      state = ParserState(table[state][input]);

      if  (state == Init || state == Done || i > (BufferSize - 5)) {
         ungetChar(c);

         if (i > (BufferSize - 5)) {
            // ignore rest of digits

            while (getChar(&c)) {
               if (!c.isDigit()) {
                  ungetChar(c);
                  break;
               }
            }
         }

         break;
      }

      buf[i++] = c.toLatin1();
   }

   if (i == 0) {
      return false;
   }

   if (! f) {
      return true;
   }

   buf[i] = '\0';

   // backward-compatibility. Old implementation supported +nan/-nan
   // for some reason. QLocale only checks for lower-case
   // nan/+inf/-inf, so here we also check for uppercase and mixed case versions.

   if (qstricmp(buf, "nan") == 0 || qstricmp(buf, "+nan") == 0  || qstricmp(buf, "-nan") == 0) {
      *f = qSNaN();
      return true;

   } else if (qstricmp(buf, "+inf") == 0 || qstricmp(buf, "inf") == 0) {
      *f = qInf();
      return true;

   } else if (qstricmp(buf, "-inf") == 0) {
      *f = -qInf();
      return true;
   }

   bool ok;
   *f = locale.toDouble(QString::fromLatin1(buf), &ok);

   return ok;
}

QTextStream &QTextStream::operator>>(QChar &c)
{
   Q_D(QTextStream);
   CHECK_VALID_STREAM(*this);

   d->scan(nullptr, 0, QTextStreamPrivate::NotSpace);

   if (!d->getChar(&c)) {
      setStatus(ReadPastEnd);
   }

   return *this;
}

QTextStream &QTextStream::operator>>(char &c)
{
   QChar ch;
   *this >> ch;
   c = ch.toLatin1();
   return *this;
}

QTextStream &QTextStream::operator>>(signed short &i)
{
   IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(signed short);
}

QTextStream &QTextStream::operator>>(unsigned short &i)
{
   IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(unsigned short);
}

QTextStream &QTextStream::operator>>(signed int &i)
{
   IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(signed int);
}

QTextStream &QTextStream::operator>>(unsigned int &i)
{
   IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(unsigned int);
}

QTextStream &QTextStream::operator>>(signed long &i)
{
   IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(signed long);
}

QTextStream &QTextStream::operator>>(unsigned long &i)
{
   IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(unsigned long);
}

QTextStream &QTextStream::operator>>(qint64 &i)
{
   IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(qint64);
}

QTextStream &QTextStream::operator>>(quint64 &i)
{
   IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(quint64);
}

QTextStream &QTextStream::operator>>(float &f)
{
   IMPLEMENT_STREAM_RIGHT_REAL_OPERATOR(float);
}

QTextStream &QTextStream::operator>>(double &f)
{
   IMPLEMENT_STREAM_RIGHT_REAL_OPERATOR(double);
}

QTextStream &QTextStream::operator>>(QString &str)
{
   Q_D(QTextStream);
   CHECK_VALID_STREAM(*this);

   str.clear();

   d->scan(nullptr, 0, QTextStreamPrivate::NotSpace);
   d->consumeLastToken();

   if (! d->scan(&str, 0, QTextStreamPrivate::Space)) {
      setStatus(ReadPastEnd);
      return *this;
   }

   d->consumeLastToken();

   return *this;
}

QTextStream &QTextStream::operator>>(QByteArray &array)
{
   Q_D(QTextStream);
   CHECK_VALID_STREAM(*this);

   array.clear();
   d->scan(nullptr, 0, QTextStreamPrivate::NotSpace);
   d->consumeLastToken();

   QString tmp;

   if (! d->scan(&tmp, 0, QTextStreamPrivate::Space)) {
      setStatus(ReadPastEnd);
      return *this;
   }

   array = tmp.toLatin1();
   d->consumeLastToken();

   return *this;
}

void QTextStreamPrivate::putNumber(quint64 number, bool negative)
{
   QString result;

   unsigned flags = 0;

   if (numberFlags & QTextStream::ShowBase) {
      flags |= QLocaleData::ShowBase;
   }

   if (numberFlags & QTextStream::ForceSign) {
      flags |= QLocaleData::AlwaysShowSign;
   }

   if (numberFlags & QTextStream::UppercaseBase) {
      flags |= QLocaleData::UppercaseBase;
   }

   if (numberFlags & QTextStream::UppercaseDigits) {
      flags |= QLocaleData::CapitalEorX;
   }

   // add thousands group separators. For backward compatibility we do not
   // add a group separator for C locale.

   if (locale != QLocale::c() && ! locale.numberOptions().testFlag(QLocale::OmitGroupSeparator)) {
      flags |= QLocaleData::ThousandsGroup;
   }

   const QLocaleData *dd = locale.d->m_data;
   int base = integerBase ? integerBase : 10;

   if (negative && base == 10) {
      result = dd->longLongToString(-static_cast<qint64>(number), -1, base, -1, flags);

   } else if (negative) {
      // Workaround for backward compatibility for writing negative numbers in octal and hex:
      // QTextStream(result) << showbase << hex << -1 << oct << -1
      // should output: -0x1 -0b1

      result = dd->unsLongLongToString(number, -1, base, -1, flags);
      result.prepend(locale.negativeSign());

   } else {
      result = dd->unsLongLongToString(number, -1, base, -1, flags);

      // workaround for backward compatibility - in octal form with
      // ShowBase flag set zero should be written as '00'

      if (number == 0 && base == 8 && numberFlags & QTextStream::ShowBase && result == "0") {
         result.prepend('0');
      }
   }

   putString(result, true);
}

QTextStream &QTextStream::operator<<(bool b)
{
   return *this << int(b);
}

QTextStream &QTextStream::operator<<(QChar c)
{
   Q_D(QTextStream);

   CHECK_VALID_STREAM(*this);
   d->putString(QString(c));

   return *this;
}

QTextStream &QTextStream::operator<<(char c)
{
   Q_D(QTextStream);

   CHECK_VALID_STREAM(*this);
   d->putString(QString(QChar::fromLatin1(c)));

   return *this;
}

QTextStream &QTextStream::operator<<(signed short i)
{
   Q_D(QTextStream);

   CHECK_VALID_STREAM(*this);
   d->putNumber((quint64)qAbs(qint64(i)), i < 0);

   return *this;
}

QTextStream &QTextStream::operator<<(unsigned short i)
{
   Q_D(QTextStream);

   CHECK_VALID_STREAM(*this);
   d->putNumber((quint64)i, false);

   return *this;
}

QTextStream &QTextStream::operator<<(signed int i)
{
   Q_D(QTextStream);

   CHECK_VALID_STREAM(*this);
   d->putNumber((quint64)qAbs(qint64(i)), i < 0);

   return *this;
}

QTextStream &QTextStream::operator<<(unsigned int i)
{
   Q_D(QTextStream);
   CHECK_VALID_STREAM(*this);
   d->putNumber((quint64)i, false);
   return *this;
}

QTextStream &QTextStream::operator<<(signed long i)
{
   Q_D(QTextStream);

   CHECK_VALID_STREAM(*this);
   d->putNumber((quint64)qAbs(qint64(i)), i < 0);

   return *this;
}

QTextStream &QTextStream::operator<<(unsigned long i)
{
   Q_D(QTextStream);

   CHECK_VALID_STREAM(*this);
   d->putNumber((quint64)i, false);

   return *this;
}

QTextStream &QTextStream::operator<<(qint64 i)
{
   Q_D(QTextStream);

   CHECK_VALID_STREAM(*this);
   d->putNumber((quint64)qAbs(i), i < 0);
   return *this;
}

QTextStream &QTextStream::operator<<(quint64 i)
{
   Q_D(QTextStream);

   CHECK_VALID_STREAM(*this);
   d->putNumber(i, false);

   return *this;
}

QTextStream &QTextStream::operator<<(float f)
{
   return *this << double(f);
}

QTextStream &QTextStream::operator<<(double f)
{
   Q_D(QTextStream);
   CHECK_VALID_STREAM(*this);

   QLocaleData::DoubleForm form = QLocaleData::DFDecimal;

   switch (realNumberNotation()) {
      case FixedNotation:
         form = QLocaleData::DFDecimal;
         break;

      case ScientificNotation:
         form = QLocaleData::DFExponent;
         break;

      case SmartNotation:
         form = QLocaleData::DFSignificantDigits;
         break;
   }

   uint flags = 0;

   if (numberFlags() & ShowBase) {
      flags |= QLocaleData::ShowBase;
   }

   if (numberFlags() & ForceSign) {
      flags |= QLocaleData::AlwaysShowSign;
   }

   if (numberFlags() & UppercaseBase) {
      flags |= QLocaleData::UppercaseBase;
   }

   if (numberFlags() & UppercaseDigits) {
      flags |= QLocaleData::CapitalEorX;
   }

   if (numberFlags() & ForcePoint) {
      flags |= QLocaleData::Alternate;
   }

   if (locale() != QLocale::c() && !(locale().numberOptions() & QLocale::OmitGroupSeparator)) {
      flags |= QLocaleData::ThousandsGroup;
   }

   const QLocaleData *dd = d->locale.d->m_data;
   QString num = dd->doubleToString(f, d->realNumberPrecision, form, -1, flags);
   d->putString(num, true);

   return *this;
}

QTextStream &QTextStream::operator<<(const QString &str)
{
   Q_D(QTextStream);

   CHECK_VALID_STREAM(*this);
   d->putString(str);

   return *this;
}

QTextStream &QTextStream::operator<<(const QStringView &str)
{
   Q_D(QTextStream);

   CHECK_VALID_STREAM(*this);
   d->putString(str);

   return *this;
}

QTextStream &QTextStream::operator<<(const QByteArray &array)
{
   Q_D(QTextStream);

   CHECK_VALID_STREAM(*this);
   d->putString(QString::fromLatin1(array.constData(), array.length()));

   return *this;
}

QTextStream &QTextStream::operator<<(const void *ptr)
{
   Q_D(QTextStream);
   CHECK_VALID_STREAM(*this);

   int oldBase = d->integerBase;
   NumberFlags oldFlags = d->numberFlags;
   d->integerBase = 16;
   d->numberFlags |= ShowBase;
   d->putNumber(reinterpret_cast<quintptr>(ptr), false);
   d->integerBase = oldBase;
   d->numberFlags = oldFlags;

   return *this;
}

QTextStream &bin(QTextStream &stream)
{
   stream.setIntegerBase(2);
   return stream;
}

QTextStream &oct(QTextStream &stream)
{
   stream.setIntegerBase(8);
   return stream;
}

QTextStream &dec(QTextStream &stream)
{
   stream.setIntegerBase(10);
   return stream;
}

QTextStream &hex(QTextStream &stream)
{
   stream.setIntegerBase(16);
   return stream;
}

QTextStream &showbase(QTextStream &stream)
{
   stream.setNumberFlags(stream.numberFlags() | QTextStream::ShowBase);
   return stream;
}

QTextStream &forcesign(QTextStream &stream)
{
   stream.setNumberFlags(stream.numberFlags() | QTextStream::ForceSign);
   return stream;
}

QTextStream &forcepoint(QTextStream &stream)
{
   stream.setNumberFlags(stream.numberFlags() | QTextStream::ForcePoint);
   return stream;
}

QTextStream &noshowbase(QTextStream &stream)
{
   stream.setNumberFlags(stream.numberFlags() &= ~QTextStream::ShowBase);
   return stream;
}

QTextStream &noforcesign(QTextStream &stream)
{
   stream.setNumberFlags(stream.numberFlags() &= ~QTextStream::ForceSign);
   return stream;
}

QTextStream &noforcepoint(QTextStream &stream)
{
   stream.setNumberFlags(stream.numberFlags() &= ~QTextStream::ForcePoint);
   return stream;
}

QTextStream &uppercasebase(QTextStream &stream)
{
   stream.setNumberFlags(stream.numberFlags() | QTextStream::UppercaseBase);
   return stream;
}

QTextStream &uppercasedigits(QTextStream &stream)
{
   stream.setNumberFlags(stream.numberFlags() | QTextStream::UppercaseDigits);
   return stream;
}

QTextStream &lowercasebase(QTextStream &stream)
{
   stream.setNumberFlags(stream.numberFlags() & ~QTextStream::UppercaseBase);
   return stream;
}

QTextStream &lowercasedigits(QTextStream &stream)
{
   stream.setNumberFlags(stream.numberFlags() & ~QTextStream::UppercaseDigits);
   return stream;
}

QTextStream &fixed(QTextStream &stream)
{
   stream.setRealNumberNotation(QTextStream::FixedNotation);
   return stream;
}

QTextStream &scientific(QTextStream &stream)
{
   stream.setRealNumberNotation(QTextStream::ScientificNotation);
   return stream;
}

QTextStream &left(QTextStream &stream)
{
   stream.setFieldAlignment(QTextStream::AlignLeft);
   return stream;
}

QTextStream &right(QTextStream &stream)
{
   stream.setFieldAlignment(QTextStream::AlignRight);
   return stream;
}

QTextStream &center(QTextStream &stream)
{
   stream.setFieldAlignment(QTextStream::AlignCenter);
   return stream;
}

QTextStream &endl(QTextStream &stream)
{
   return stream << '\n' << flush;
}

QTextStream &flush(QTextStream &stream)
{
   stream.flush();
   return stream;
}

QTextStream &reset(QTextStream &stream)
{
   stream.reset();
   return stream;
}

QTextStream &ws(QTextStream &stream)
{
   stream.skipWhiteSpace();
   return stream;
}

#ifndef QT_NO_TEXTCODEC

QTextStream &bom(QTextStream &stream)
{
   stream.setGenerateByteOrderMark(true);
   return stream;
}

void QTextStream::setCodec(QTextCodec *codec)
{
   Q_D(QTextStream);
   qint64 seekPos = -1;

   if (! d->readBuffer.isEmpty()) {
      if (! d->device->isSequential()) {
         seekPos = pos();
      }
   }

   d->codec = codec;

   if (seekPos >= 0 && !d->readBuffer.isEmpty()) {
      seek(seekPos);
   }
}

void QTextStream::setCodec(const char *codecName)
{
   QTextCodec *codec = QTextCodec::codecForName(codecName);

   if (codec) {
      setCodec(codec);
   }
}

QTextCodec *QTextStream::codec() const
{
   Q_D(const QTextStream);
   return d->codec;
}

void QTextStream::setAutoDetectUnicode(bool enabled)
{
   Q_D(QTextStream);
   d->autoDetectUnicode = enabled;
}

bool QTextStream::autoDetectUnicode() const
{
   Q_D(const QTextStream);
   return d->autoDetectUnicode;
}

void QTextStream::setGenerateByteOrderMark(bool generate)
{
   Q_D(QTextStream);

   if (d->writeBuffer.isEmpty()) {
      if (generate) {
         d->writeConverterState.m_flags &= ~QTextCodec::IgnoreHeader;
      } else {
         d->writeConverterState.m_flags |= QTextCodec::IgnoreHeader;
      }
   }
}

bool QTextStream::generateByteOrderMark() const
{
   Q_D(const QTextStream);
   return (d->writeConverterState.m_flags & QTextCodec::IgnoreHeader) == 0;
}
#endif

void QTextStream::setLocale(const QLocale &locale)
{
   Q_D(QTextStream);
   d->locale = locale;
}

QLocale QTextStream::locale() const
{
   Q_D(const QTextStream);
   return d->locale;
}
