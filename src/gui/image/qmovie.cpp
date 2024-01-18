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

#include <qglobal.h>

#ifndef QT_NO_MOVIE

#include <qmovie.h>
#include <qimage.h>
#include <qimagereader.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qpair.h>
#include <qmap.h>
#include <qlist.h>
#include <qbuffer.h>
#include <qdir.h>

#define QMOVIE_INVALID_DELAY -1

class QFrameInfo
{
 public:
   QFrameInfo(bool endMark)
      : pixmap(QPixmap()), delay(QMOVIE_INVALID_DELAY), endMark(endMark) {
   }

   QFrameInfo()
      : pixmap(QPixmap()), delay(QMOVIE_INVALID_DELAY), endMark(false) {
   }

   QFrameInfo(const QPixmap &pixmap, int delay)
      : pixmap(pixmap), delay(delay), endMark(false) {
   }

   bool isValid() {
      return endMark || !(pixmap.isNull() && (delay == QMOVIE_INVALID_DELAY));
   }

   bool isEndMarker() {
      return endMark;
   }

   static inline QFrameInfo endMarker() {
      return QFrameInfo(true);
   }

   QPixmap pixmap;
   int delay;
   bool endMark;
};

class QMoviePrivate
{
   Q_DECLARE_PUBLIC(QMovie)

 public:
   QMoviePrivate(QMovie *qq);
   virtual ~QMoviePrivate() {}

   bool isDone();
   bool next();
   int speedAdjustedDelay(int delay) const;
   bool isValid() const;
   bool jumpToFrame(int frameNumber);
   int frameCount() const;
   bool jumpToNextFrame();
   QFrameInfo infoForFrame(int frameNumber);
   void reset();

   void enterState(QMovie::MovieState newState) {
      movieState = newState;
      emit q_func()->stateChanged(newState);
   }

   // private slots
   void _q_loadNextFrame();
   void _q_loadNextFrame(bool starting);

   QImageReader *reader;
   int speed;
   QMovie::MovieState movieState;
   QRect frameRect;
   QPixmap currentPixmap;
   int currentFrameNumber;
   int nextFrameNumber;
   int greatestFrameNumber;
   int nextDelay;
   int playCounter;
   qint64 initialDevicePos;
   QMovie::CacheMode cacheMode;
   bool haveReadAll;
   bool isFirstIteration;
   QMap<int, QFrameInfo> frameMap;
   QString absoluteFilePath;

   QTimer nextImageTimer;

 protected:
   QMovie *q_ptr;

};

/*! \internal
 */
QMoviePrivate::QMoviePrivate(QMovie *qq)
   : reader(nullptr), speed(100), movieState(QMovie::NotRunning), currentFrameNumber(-1), nextFrameNumber(0),
     greatestFrameNumber(-1), nextDelay(0), playCounter(-1),
     cacheMode(QMovie::CacheNone), haveReadAll(false), isFirstIteration(true)
{
   q_ptr = qq;
   nextImageTimer.setSingleShot(true);
}

/*! \internal
 */
void QMoviePrivate::reset()
{
   nextImageTimer.stop();
   if (reader->device()) {
      initialDevicePos = reader->device()->pos();
   }
   currentFrameNumber = -1;
   nextFrameNumber = 0;
   greatestFrameNumber = -1;
   nextDelay = 0;
   playCounter = -1;
   haveReadAll = false;
   isFirstIteration = true;
   frameMap.clear();
}

/*! \internal
 */
bool QMoviePrivate::isDone()
{
   return (playCounter == 0);
}

/*!
    \internal

    Given the original \a delay, this function returns the
    actual number of milliseconds to delay according to
    the current speed. E.g. if the speed is 200%, the
    result will be half of the original delay.
*/
int QMoviePrivate::speedAdjustedDelay(int delay) const
{
   return int( (qint64(delay) * qint64(100) ) / qint64(speed) );
}

/*!
    \internal

    Returns the QFrameInfo for the given \a frameNumber.

    If the frame number is invalid, an invalid QFrameInfo is
    returned.

    If the end of the animation has been reached, a
    special end marker QFrameInfo is returned.

*/
QFrameInfo QMoviePrivate::infoForFrame(int frameNumber)
{
   if (frameNumber < 0) {
      return QFrameInfo();   // Invalid
   }

   if (haveReadAll && (frameNumber > greatestFrameNumber)) {
      if (frameNumber == greatestFrameNumber + 1) {
         return QFrameInfo::endMarker();
      }
      return QFrameInfo(); // Invalid
   }

   if (cacheMode == QMovie::CacheNone) {
      if (frameNumber != currentFrameNumber + 1) {
         // Non-sequential frame access
         if (!reader->jumpToImage(frameNumber)) {
            if (frameNumber == 0) {
               // Special case: Attempt to "rewind" so we can loop
               // ### This could be implemented as QImageReader::rewind()
               if (reader->device()->isSequential()) {
                  return QFrameInfo();   // Invalid
               }

               QString fileName  = reader->fileName();
               QString format    = reader->format();
               QIODevice *device = reader->device();
               QColor bgColor    = reader->backgroundColor();
               QSize scaledSize  = reader->scaledSize();

               delete reader;
               if (fileName.isEmpty()) {
                  reader = new QImageReader(device, format);
               } else {
                  reader = new QImageReader(absoluteFilePath, format);
               }
               (void)reader->canRead(); // Provoke a device->open() call
               reader->device()->seek(initialDevicePos);
               reader->setBackgroundColor(bgColor);
               reader->setScaledSize(scaledSize);
            } else {
               return QFrameInfo(); // Invalid
            }
         }
      }
      if (reader->canRead()) {
         // reader says we can read. Attempt to actually read image
         QImage anImage = reader->read();
         if (anImage.isNull()) {
            // Reading image failed.
            return QFrameInfo(); // Invalid
         }
         if (frameNumber > greatestFrameNumber) {
            greatestFrameNumber = frameNumber;
         }
         QPixmap aPixmap = QPixmap::fromImage(anImage);
         int aDelay = reader->nextImageDelay();
         return QFrameInfo(aPixmap, aDelay);
      } else if (frameNumber != 0) {
         // We've read all frames now. Return an end marker
         haveReadAll = true;
         return QFrameInfo::endMarker();
      } else {
         // No readable frames
         haveReadAll = true;
         return QFrameInfo();
      }
   }

   // CacheMode == CacheAll
   if (frameNumber > greatestFrameNumber) {
      // Frame hasn't been read from file yet. Try to do it
      for (int i = greatestFrameNumber + 1; i <= frameNumber; ++i) {
         if (reader->canRead()) {
            // reader says we can read. Attempt to actually read image
            QImage anImage = reader->read();
            if (anImage.isNull()) {
               // Reading image failed.
               return QFrameInfo(); // Invalid
            }
            greatestFrameNumber = i;
            QPixmap aPixmap = QPixmap::fromImage(anImage);
            int aDelay = reader->nextImageDelay();
            QFrameInfo info(aPixmap, aDelay);
            // Cache it!
            frameMap.insert(i, info);
            if (i == frameNumber) {
               return info;
            }
         } else {
            // We've read all frames now. Return an end marker
            haveReadAll = true;
            return QFrameInfo::endMarker();
         }
      }
   }
   // Return info for requested (cached) frame
   return frameMap.value(frameNumber);
}

/*!
    \internal

    Attempts to advance the animation to the next frame.
    If successful, currentFrameNumber, currentPixmap and
    nextDelay are updated accordingly, and true is returned.
    Otherwise, false is returned.
    When false is returned, isDone() can be called to
    determine whether the animation ended gracefully or
    an error occurred when reading the frame.
*/
bool QMoviePrivate::next()
{
   QTime time;
   time.start();
   QFrameInfo info = infoForFrame(nextFrameNumber);
   if (!info.isValid()) {
      return false;
   }
   if (info.isEndMarker()) {
      // We reached the end of the animation.
      if (isFirstIteration) {
         if (nextFrameNumber == 0) {
            // No frames could be read at all (error).
            return false;
         }
         // End of first iteration. Initialize play counter
         playCounter = reader->loopCount();
         isFirstIteration = false;
      }
      // Loop as appropriate
      if (playCounter != 0) {
         if (playCounter != -1) { // Infinite?
            playCounter--;   // Nope
         }
         nextFrameNumber = 0;
         return next();
      }
      // Loop no more. Done
      return false;
   }
   // Image and delay OK, update internal state
   currentFrameNumber = nextFrameNumber++;
   QSize scaledSize = reader->scaledSize();
   if (scaledSize.isValid() && (scaledSize != info.pixmap.size())) {
      currentPixmap = QPixmap::fromImage( info.pixmap.toImage().scaled(scaledSize) );
   } else {
      currentPixmap = info.pixmap;
   }
   nextDelay = speedAdjustedDelay(info.delay);
   // Adjust delay according to the time it took to read the frame
   int processingTime = time.elapsed();
   if (processingTime > nextDelay) {
      nextDelay = 0;
   } else {
      nextDelay = nextDelay - processingTime;
   }
   return true;
}

/*! \internal
 */
void QMoviePrivate::_q_loadNextFrame()
{
   _q_loadNextFrame(false);
}

void QMoviePrivate::_q_loadNextFrame(bool starting)
{
   Q_Q(QMovie);
   if (next()) {
      if (starting && movieState == QMovie::NotRunning) {
         enterState(QMovie::Running);
         emit q->started();
      }

      if (frameRect.size() != currentPixmap.rect().size()) {
         frameRect = currentPixmap.rect();
         emit q->resized(frameRect.size());
      }

      emit q->updated(frameRect);
      emit q->frameChanged(currentFrameNumber);

      if (movieState == QMovie::Running) {
         nextImageTimer.start(nextDelay);
      }
   } else {
      // Could not read another frame
      if (!isDone()) {
         emit q->error(reader->error());
      }

      // Graceful finish
      if (movieState != QMovie::Paused) {
         nextFrameNumber = 0;
         isFirstIteration = true;
         playCounter = -1;
         enterState(QMovie::NotRunning);
         emit q->finished();
      }
   }
}

/*!
    \internal
*/
bool QMoviePrivate::isValid() const
{
   return (greatestFrameNumber >= 0) // have we seen valid data
      || reader->canRead(); // or does the reader see valid data
}

/*!
    \internal
*/
bool QMoviePrivate::jumpToFrame(int frameNumber)
{
   if (frameNumber < 0) {
      return false;
   }
   if (currentFrameNumber == frameNumber) {
      return true;
   }
   nextFrameNumber = frameNumber;
   if (movieState == QMovie::Running) {
      nextImageTimer.stop();
   }
   _q_loadNextFrame();
   return (nextFrameNumber == currentFrameNumber + 1);
}

int QMoviePrivate::frameCount() const
{
   int result;

   if ((result = reader->imageCount()) != 0) {
      return result;
   }

   if (haveReadAll) {
      return greatestFrameNumber + 1;
   }

   return 0;
}

bool QMoviePrivate::jumpToNextFrame()
{
   return jumpToFrame(currentFrameNumber + 1);
}

QMovie::QMovie(QObject *parent)
   : QObject(parent), d_ptr(new QMoviePrivate(this))
{
   d_ptr->q_ptr = this;
   Q_D(QMovie);

   d->reader = new QImageReader;
   connect(&d->nextImageTimer, &QTimer::timeout, this, &QMovie::_q_loadNextFrame);
}

QMovie::QMovie(QIODevice *device, const QString &format, QObject *parent)
   : QObject(parent), d_ptr(new QMoviePrivate(this))
{
   d_ptr->q_ptr = this;
   Q_D(QMovie);

   d->reader = new QImageReader(device, format);
   d->initialDevicePos = device->pos();

   connect(&d->nextImageTimer, &QTimer::timeout, this, &QMovie::_q_loadNextFrame);
}

QMovie::QMovie(const QString &fileName, const QString &format, QObject *parent)
   : QObject(parent), d_ptr(new QMoviePrivate(this))
{
   d_ptr->q_ptr = this;
   Q_D(QMovie);

   d->absoluteFilePath = QDir(fileName).absolutePath();
   d->reader = new QImageReader(fileName, format);

   if (d->reader->device()) {
      d->initialDevicePos = d->reader->device()->pos();
   }

   connect(&d->nextImageTimer, &QTimer::timeout, this, &QMovie::_q_loadNextFrame);
}

QMovie::~QMovie()
{
   Q_D(QMovie);
   delete d->reader;
}

void QMovie::setDevice(QIODevice *device)
{
   Q_D(QMovie);
   d->reader->setDevice(device);
   d->reset();
}

QIODevice *QMovie::device() const
{
   Q_D(const QMovie);
   return d->reader->device();
}

void QMovie::setFileName(const QString &fileName)
{
   Q_D(QMovie);
   d->absoluteFilePath = QDir(fileName).absolutePath();
   d->reader->setFileName(fileName);
   d->reset();
}

QString QMovie::fileName() const
{
   Q_D(const QMovie);
   return d->reader->fileName();
}

void QMovie::setFormat(const QString &format)
{
   Q_D(QMovie);
   d->reader->setFormat(format);
}

QString QMovie::format() const
{
   Q_D(const QMovie);
   return d->reader->format();
}

void QMovie::setBackgroundColor(const QColor &color)
{
   Q_D(QMovie);
   d->reader->setBackgroundColor(color);
}

QColor QMovie::backgroundColor() const
{
   Q_D(const QMovie);
   return d->reader->backgroundColor();
}

QMovie::MovieState QMovie::state() const
{
   Q_D(const QMovie);
   return d->movieState;
}

QRect QMovie::frameRect() const
{
   Q_D(const QMovie);
   return d->frameRect;
}

QPixmap QMovie::currentPixmap() const
{
   Q_D(const QMovie);
   return d->currentPixmap;
}

QImage QMovie::currentImage() const
{
   Q_D(const QMovie);
   return d->currentPixmap.toImage();
}

bool QMovie::isValid() const
{
   Q_D(const QMovie);
   return d->isValid();
}

int QMovie::frameCount() const
{
   Q_D(const QMovie);
   return d->frameCount();
}

int QMovie::nextFrameDelay() const
{
   Q_D(const QMovie);
   return d->nextDelay;
}

int QMovie::currentFrameNumber() const
{
   Q_D(const QMovie);
   return d->currentFrameNumber;
}

bool QMovie::jumpToNextFrame()
{
   Q_D(QMovie);
   return d->jumpToNextFrame();
}

bool QMovie::jumpToFrame(int frameNumber)
{
   Q_D(QMovie);
   return d->jumpToFrame(frameNumber);
}

int QMovie::loopCount() const
{
   Q_D(const QMovie);
   return d->reader->loopCount();
}

void QMovie::setPaused(bool paused)
{
   Q_D(QMovie);
   if (paused) {
      if (d->movieState == NotRunning) {
         return;
      }
      d->enterState(Paused);
      d->nextImageTimer.stop();
   } else {
      if (d->movieState == Running) {
         return;
      }
      d->enterState(Running);
      d->nextImageTimer.start(nextFrameDelay());
   }
}

void QMovie::setSpeed(int percentSpeed)
{
   Q_D(QMovie);
   d->speed = percentSpeed;
}

int QMovie::speed() const
{
   Q_D(const QMovie);
   return d->speed;
}

void QMovie::start()
{
   Q_D(QMovie);
   if (d->movieState == NotRunning) {
      d->_q_loadNextFrame(true);
   } else if (d->movieState == Paused) {
      setPaused(false);
   }
}

void QMovie::stop()
{
   Q_D(QMovie);
   if (d->movieState == NotRunning) {
      return;
   }
   d->enterState(NotRunning);
   d->nextImageTimer.stop();
   d->nextFrameNumber = 0;
}

QSize QMovie::scaledSize()
{
   Q_D(QMovie);
   return d->reader->scaledSize();
}

void QMovie::setScaledSize(const QSize &size)
{
   Q_D(QMovie);
   d->reader->setScaledSize(size);
}

QList<QString> QMovie::supportedFormats()
{
   QList<QString> list = QImageReader::supportedImageFormats();

   QBuffer buffer;
   buffer.open(QIODevice::ReadOnly);

   auto iter = list.begin();

   while (iter != list.end()) {
      QImageReader reader(&buffer, *iter);

      if (! reader.supportsAnimation()) {
         iter = list.erase(iter);
      } else {
         ++iter;
      }
   }

   return list;
}

QMovie::CacheMode QMovie::cacheMode() const
{
   Q_D(const QMovie);
   return d->cacheMode;
}

void QMovie::setCacheMode(CacheMode cacheMode)
{
   Q_D(QMovie);
   d->cacheMode = cacheMode;
}

void QMovie::_q_loadNextFrame()
{
   Q_D(QMovie);
   d->_q_loadNextFrame();
}

#endif // QT_NO_MOVIE
