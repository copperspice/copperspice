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

#include <qaudiobuffer.h>
#include <qaudiobuffer_p.h>

#include <qobject.h>
#include <qdebug.h>

class QAudioBufferPrivate : public QSharedData
{
 public:
   QAudioBufferPrivate(QAbstractAudioBuffer *provider)
      : mProvider(provider), mCount(1)
   {
   }

   ~QAudioBufferPrivate() {
      if (mProvider) {
         mProvider->release();
      }
   }

   void ref() {
      mCount.ref();
   }

   void deref() {
      if (!mCount.deref()) {
         delete this;
      }
   }

   QAudioBufferPrivate *clone();

   static QAudioBufferPrivate *acquire(QAudioBufferPrivate *other) {
      if (! other) {
         return nullptr;
      }

      // Ref the other (if there are extant data() pointers, they will
      // also point here - it's a feature, not a bug, like QByteArray)
      other->ref();

      return other;
   }

   QAbstractAudioBuffer *mProvider;
   QAtomicInt mCount;
};

// Private class to go in .cpp file
class QMemoryAudioBufferProvider : public QAbstractAudioBuffer
{
 public:
   QMemoryAudioBufferProvider(const void *data, int frameCount, const QAudioFormat &format, qint64 startTime)
      : mStartTime(startTime), mFrameCount(frameCount), mFormat(format) {
      int numBytes = format.bytesForFrames(frameCount);
      if (numBytes > 0) {
         mBuffer = malloc(numBytes);

         if (!mBuffer) {
            // OOM, if that's likely
            mStartTime = -1;
            mFrameCount = 0;
            mFormat = QAudioFormat();

         } else {
            // Allocated, see if we have data to copy
            if (data) {
               memcpy(mBuffer, data, numBytes);
            } else {
               // We have to fill with the zero value..
               switch (format.sampleType()) {
                  case QAudioFormat::SignedInt:
                     // Signed int means 0x80, 0x8000 is zero
                     // XXX this is not right for > 8 bits(0x8080 vs 0x8000)
                     memset(mBuffer, 0x80, numBytes);
                     break;
                  default:
                     memset(mBuffer, 0x0, numBytes);
               }
            }
         }
      } else {
         mBuffer = nullptr;
      }
   }

   ~QMemoryAudioBufferProvider() {
      if (mBuffer) {
         free(mBuffer);
      }
   }

   void release() override {
      delete this;
   }

   QAudioFormat format() const override {
      return mFormat;
   }

   qint64 startTime() const override {
      return mStartTime;
   }

   int frameCount() const override {
      return mFrameCount;
   }

   void *constData() const override {
      return mBuffer;
   }

   void *writableData() override {
      return mBuffer;
   }

   QAbstractAudioBuffer *clone() const override {
      return new QMemoryAudioBufferProvider(mBuffer, mFrameCount, mFormat, mStartTime);
   }

   void *mBuffer;
   qint64 mStartTime;
   int mFrameCount;
   QAudioFormat mFormat;
};

QAudioBufferPrivate *QAudioBufferPrivate::clone()
{
   // We want to create a single bufferprivate with a
   // single qaab
   // This should only be called when the count is > 1
   Q_ASSERT(mCount.load() > 1);

   if (mProvider) {
      QAbstractAudioBuffer *abuf = mProvider->clone();

      if (!abuf) {
         abuf = new QMemoryAudioBufferProvider(mProvider->constData(), mProvider->frameCount(), mProvider->format(), mProvider->startTime());
      }

      if (abuf) {
         return new QAudioBufferPrivate(abuf);
      }
   }

   return nullptr;
}

QAudioBuffer::QAudioBuffer()
   : d(nullptr)
{
}

QAudioBuffer::QAudioBuffer(QAbstractAudioBuffer *provider)
   : d(new QAudioBufferPrivate(provider))
{
}
/*!
    Generally this will have copy-on-write semantics - a copy will only be made when it has to be.
 */
QAudioBuffer::QAudioBuffer(const QAudioBuffer &other)
{
   d = QAudioBufferPrivate::acquire(other.d);
}

QAudioBuffer::QAudioBuffer(const QByteArray &data, const QAudioFormat &format, qint64 startTime)
{
   if (format.isValid()) {
      int frameCount = format.framesForBytes(data.size());
      d = new QAudioBufferPrivate(new QMemoryAudioBufferProvider(data.constData(), frameCount, format, startTime));
   } else {
      d = nullptr;
   }
}

QAudioBuffer::QAudioBuffer(int numFrames, const QAudioFormat &format, qint64 startTime)
{
   if (format.isValid()) {
      d = new QAudioBufferPrivate(new QMemoryAudioBufferProvider(nullptr, numFrames, format, startTime));
   } else {
      d = nullptr;
   }
}

QAudioBuffer &QAudioBuffer::operator =(const QAudioBuffer &other)
{
   if (this->d != other.d) {
      if (d) {
         d->deref();
      }
      d = QAudioBufferPrivate::acquire(other.d);
   }
   return *this;
}

QAudioBuffer::~QAudioBuffer()
{
   if (d) {
      d->deref();
   }
}

bool QAudioBuffer::isValid() const
{
   if (!d || !d->mProvider) {
      return false;
   }
   return d->mProvider->format().isValid() && (d->mProvider->frameCount() > 0);
}

QAudioFormat QAudioBuffer::format() const
{
   if (!isValid()) {
      return QAudioFormat();
   }
   return d->mProvider->format();
}

int QAudioBuffer::frameCount() const
{
   if (!isValid()) {
      return 0;
   }
   return d->mProvider->frameCount();
}

int QAudioBuffer::sampleCount() const
{
   if (!isValid()) {
      return 0;
   }

   return frameCount() * format().channelCount();
}

int QAudioBuffer::byteCount() const
{
   const QAudioFormat f(format());
   return format().bytesForFrames(frameCount());
}

qint64 QAudioBuffer::duration() const
{
   return format().durationForFrames(frameCount());
}

qint64 QAudioBuffer::startTime() const
{
   if (!isValid()) {
      return -1;
   }
   return d->mProvider->startTime();
}

const void *QAudioBuffer::constData() const
{
   if (!isValid()) {
      return nullptr;
   }

   return d->mProvider->constData();
}

const void *QAudioBuffer::data() const
{
   if (!isValid()) {
      return nullptr;
   }

   return d->mProvider->constData();
}

void *QAudioBuffer::data()
{
   if (!isValid()) {
      return nullptr;
   }

   if (d->mCount.load() != 1) {
      // Can't share a writable buffer
      // so we need to detach
      QAudioBufferPrivate *newd = d->clone();

      // This shouldn't happen
      if (! newd) {
         return nullptr;
      }

      d->deref();
      d = newd;
   }

   // We're (now) the only user of this qaab, so
   // see if it's writable directly
   void *buffer = d->mProvider->writableData();
   if (buffer) {
      return buffer;
   }

   // Wasn't writable, so turn it into a memory provider
   QAbstractAudioBuffer *memBuffer = new QMemoryAudioBufferProvider(constData(), frameCount(), format(), startTime());

   if (memBuffer) {
      d->mProvider->release();
      d->mCount.store(1);
      d->mProvider = memBuffer;

      return memBuffer->writableData();
   }

   return nullptr;
}
