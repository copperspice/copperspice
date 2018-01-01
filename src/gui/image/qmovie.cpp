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

QT_BEGIN_NAMESPACE

class QFrameInfo
{
 public:
   QPixmap pixmap;
   int delay;
   bool endMark;
   inline QFrameInfo(bool endMark)
      : pixmap(QPixmap()), delay(QMOVIE_INVALID_DELAY), endMark(endMark) {
   }

   inline QFrameInfo()
      : pixmap(QPixmap()), delay(QMOVIE_INVALID_DELAY), endMark(false) {
   }

   inline QFrameInfo(const QPixmap &pixmap, int delay)
      : pixmap(pixmap), delay(delay), endMark(false) {
   }

   inline bool isValid() {
      return endMark || !(pixmap.isNull() && (delay == QMOVIE_INVALID_DELAY));
   }

   inline bool isEndMarker() {
      return endMark;
   }

   static inline QFrameInfo endMarker() {
      return QFrameInfo(true);
   }
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

   inline void enterState(QMovie::MovieState newState) {
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
   : reader(0), speed(100), movieState(QMovie::NotRunning),
     currentFrameNumber(-1), nextFrameNumber(0), greatestFrameNumber(-1),
     nextDelay(0), playCounter(-1),
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
               QString fileName = reader->fileName();
               QByteArray format = reader->format();
               QIODevice *device = reader->device();
               QColor bgColor = reader->backgroundColor();
               QSize scaledSize = reader->scaledSize();
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

/*!
    \internal
*/
int QMoviePrivate::frameCount() const
{
   int result;
   if ((result = reader->imageCount()) != 0) {
      return result;
   }
   if (haveReadAll) {
      return greatestFrameNumber + 1;
   }
   return 0; // Don't know
}

/*!
    \internal
*/
bool QMoviePrivate::jumpToNextFrame()
{
   return jumpToFrame(currentFrameNumber + 1);
}

/*!
    Constructs a QMovie object, passing the \a parent object to QObject's
    constructor.

    \sa setFileName(), setDevice(), setFormat()
 */
QMovie::QMovie(QObject *parent)
   : QObject(parent), d_ptr(new QMoviePrivate(this))
{
   d_ptr->q_ptr = this;
   Q_D(QMovie);

   d->reader = new QImageReader;
   connect(&d->nextImageTimer, SIGNAL(timeout()), this, SLOT(_q_loadNextFrame()));
}

/*!
    Constructs a QMovie object. QMovie will use read image data from \a
    device, which it assumes is open and readable. If \a format is not empty,
    QMovie will use the image format \a format for decoding the image
    data. Otherwise, QMovie will attempt to guess the format.

    The \a parent object is passed to QObject's constructor.
 */
QMovie::QMovie(QIODevice *device, const QByteArray &format, QObject *parent)
   : QObject(parent), d_ptr(new QMoviePrivate(this))
{
   d_ptr->q_ptr = this;
   Q_D(QMovie);

   d->reader = new QImageReader(device, format);
   d->initialDevicePos = device->pos();
   connect(&d->nextImageTimer, SIGNAL(timeout()), this, SLOT(_q_loadNextFrame()));
}

/*!
    Constructs a QMovie object. QMovie will use read image data from \a
    fileName. If \a format is not empty, QMovie will use the image format \a
    format for decoding the image data. Otherwise, QMovie will attempt to
    guess the format.

    The \a parent object is passed to QObject's constructor.
 */
QMovie::QMovie(const QString &fileName, const QByteArray &format, QObject *parent)
   : QObject(parent), d_ptr(new QMoviePrivate(this))
{
   d_ptr->q_ptr = this;
   Q_D(QMovie);

   d->absoluteFilePath = QDir(fileName).absolutePath();
   d->reader = new QImageReader(fileName, format);
   if (d->reader->device()) {
      d->initialDevicePos = d->reader->device()->pos();
   }
   connect(&d->nextImageTimer, SIGNAL(timeout()), this, SLOT(_q_loadNextFrame()));
}

/*!
    Destructs the QMovie object.
*/
QMovie::~QMovie()
{
   Q_D(QMovie);
   delete d->reader;
}

/*!
    Sets the current device to \a device. QMovie will read image data from
    this device when the movie is running.

    \sa device(), setFormat()
*/
void QMovie::setDevice(QIODevice *device)
{
   Q_D(QMovie);
   d->reader->setDevice(device);
   d->reset();
}

/*!
    Returns the device QMovie reads image data from. If no device has
    currently been assigned, 0 is returned.

    \sa setDevice(), fileName()
*/
QIODevice *QMovie::device() const
{
   Q_D(const QMovie);
   return d->reader->device();
}

/*!
    Sets the name of the file that QMovie reads image data from, to \a
    fileName.

    \sa fileName(), setDevice(), setFormat()
*/
void QMovie::setFileName(const QString &fileName)
{
   Q_D(QMovie);
   d->absoluteFilePath = QDir(fileName).absolutePath();
   d->reader->setFileName(fileName);
   d->reset();
}

/*!
    Returns the name of the file that QMovie reads image data from. If no file
    name has been assigned, or if the assigned device is not a file, an empty
    QString is returned.

    \sa setFileName(), device()
*/
QString QMovie::fileName() const
{
   Q_D(const QMovie);
   return d->reader->fileName();
}

/*!
    Sets the format that QMovie will use when decoding image data, to \a
    format. By default, QMovie will attempt to guess the format of the image
    data.

    You can call supportedFormats() for the full list of formats
    QMovie supports.

    \sa QImageReader::supportedImageFormats()
*/
void QMovie::setFormat(const QByteArray &format)
{
   Q_D(QMovie);
   d->reader->setFormat(format);
}

/*!
    Returns the format that QMovie uses when decoding image data. If no format
    has been assigned, an empty QByteArray() is returned.

    \sa setFormat()
*/
QByteArray QMovie::format() const
{
   Q_D(const QMovie);
   return d->reader->format();
}

/*!
    For image formats that support it, this function sets the background color
    to \a color.

    \sa backgroundColor()
*/
void QMovie::setBackgroundColor(const QColor &color)
{
   Q_D(QMovie);
   d->reader->setBackgroundColor(color);
}

/*!
    Returns the background color of the movie. If no background color has been
    assigned, an invalid QColor is returned.

    \sa setBackgroundColor()
*/
QColor QMovie::backgroundColor() const
{
   Q_D(const QMovie);
   return d->reader->backgroundColor();
}

/*!
    Returns the current state of QMovie.

    \sa MovieState, stateChanged()
*/
QMovie::MovieState QMovie::state() const
{
   Q_D(const QMovie);
   return d->movieState;
}

/*!
    Returns the rect of the last frame. If no frame has yet been updated, an
    invalid QRect is returned.

    \sa currentImage(), currentPixmap()
*/
QRect QMovie::frameRect() const
{
   Q_D(const QMovie);
   return d->frameRect;
}

/*! \fn QImage QMovie::framePixmap() const

    Use currentPixmap() instead.
*/

/*! \fn void QMovie::pause()

    Use setPaused(true) instead.
*/

/*! \fn void QMovie::unpause()

    Use setPaused(false) instead.
*/

/*!
    Returns the current frame as a QPixmap.

    \sa currentImage(), updated()
*/
QPixmap QMovie::currentPixmap() const
{
   Q_D(const QMovie);
   return d->currentPixmap;
}

/*! \fn QImage QMovie::frameImage() const

    Use currentImage() instead.
*/

/*!
    Returns the current frame as a QImage.

    \sa currentPixmap(), updated()
*/
QImage QMovie::currentImage() const
{
   Q_D(const QMovie);
   return d->currentPixmap.toImage();
}

/*!
    Returns true if the movie is valid (e.g., the image data is readable and
    the image format is supported); otherwise returns false.
*/
bool QMovie::isValid() const
{
   Q_D(const QMovie);
   return d->isValid();
}

/*! \fn bool QMovie::running() const

    Use state() instead.
*/

/*! \fn bool QMovie::isNull() const

    Use isValid() instead.
*/

/*! \fn int QMovie::frameNumber() const

    Use currentFrameNumber() instead.
*/

/*! \fn bool QMovie::paused() const

    Use state() instead.
*/

/*! \fn bool QMovie::finished() const

    Use state() instead.
*/

/*! \fn void QMovie::restart()

    Use stop() and start() instead.
*/

/*!
    \fn void QMovie::step()

    Use jumpToNextFrame() instead.
*/

/*!
    Returns the number of frames in the movie.

    Certain animation formats do not support this feature, in which
    case 0 is returned.
*/
int QMovie::frameCount() const
{
   Q_D(const QMovie);
   return d->frameCount();
}

/*!
    Returns the number of milliseconds QMovie will wait before updating the
    next frame in the animation.
*/
int QMovie::nextFrameDelay() const
{
   Q_D(const QMovie);
   return d->nextDelay;
}

/*!
    Returns the sequence number of the current frame. The number of the first
    frame in the movie is 0.
*/
int QMovie::currentFrameNumber() const
{
   Q_D(const QMovie);
   return d->currentFrameNumber;
}

/*!
    Jumps to the next frame. Returns true on success; otherwise returns false.
*/
bool QMovie::jumpToNextFrame()
{
   Q_D(QMovie);
   return d->jumpToNextFrame();
}

/*!
    Jumps to frame number \a frameNumber. Returns true on success; otherwise
    returns false.
*/
bool QMovie::jumpToFrame(int frameNumber)
{
   Q_D(QMovie);
   return d->jumpToFrame(frameNumber);
}

/*!
    Returns the number of times the movie will loop before it finishes.
    If the movie will only play once (no looping), loopCount returns 0.
    If the movie loops forever, loopCount returns -1.

    Note that, if the image data comes from a sequential device (e.g. a
    socket), QMovie can only loop the movie if the cacheMode is set to
    QMovie::CacheAll.
*/
int QMovie::loopCount() const
{
   Q_D(const QMovie);
   return d->reader->loopCount();
}

/*!
    If \a paused is true, QMovie will enter \l Paused state and emit
    stateChanged(Paused); otherwise it will enter \l Running state and emit
    stateChanged(Running).

    \sa state()
*/
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

/*!
    \property QMovie::speed
    \brief the movie's speed

    The speed is measured in percentage of the original movie speed.
    The default speed is 100%.
    Example:

    \snippet doc/src/snippets/code/src_gui_image_qmovie.cpp 1
*/
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

/*!
    Starts the movie. QMovie will enter \l Running state, and start emitting
    updated() and resized() as the movie progresses.

    If QMovie is in the \l Paused state, this function is equivalent
    to calling setPaused(false). If QMovie is already in the \l
    Running state, this function does nothing.

    \sa stop(), setPaused()
*/
void QMovie::start()
{
   Q_D(QMovie);
   if (d->movieState == NotRunning) {
      d->_q_loadNextFrame(true);
   } else if (d->movieState == Paused) {
      setPaused(false);
   }
}

/*!
    Stops the movie. QMovie enters \l NotRunning state, and stops emitting
    updated() and resized(). If start() is called again, the movie will
    restart from the beginning.

    If QMovie is already in the \l NotRunning state, this function
    does nothing.

    \sa start(), setPaused()
*/
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

/*!
    \since 4.1

    Returns the scaled size of frames.

    \sa QImageReader::scaledSize()
*/
QSize QMovie::scaledSize()
{
   Q_D(QMovie);
   return d->reader->scaledSize();
}

/*!
    \since 4.1

    Sets the scaled frame size to \a size.

    \sa QImageReader::setScaledSize()
*/
void QMovie::setScaledSize(const QSize &size)
{
   Q_D(QMovie);
   d->reader->setScaledSize(size);
}

/*!
    \since 4.1

    Returns the list of image formats supported by QMovie.

    \sa QImageReader::supportedImageFormats()
*/
QList<QByteArray> QMovie::supportedFormats()
{
   QList<QByteArray> list = QImageReader::supportedImageFormats();
   QMutableListIterator<QByteArray> it(list);
   QBuffer buffer;
   buffer.open(QIODevice::ReadOnly);
   while (it.hasNext()) {
      QImageReader reader(&buffer, it.next());
      if (!reader.supportsAnimation()) {
         it.remove();
      }
   }
   return list;
}

/*!
    \property QMovie::cacheMode
    \brief the movie's cache mode

    Caching frames can be useful when the underlying animation format handler
    that QMovie relies on to decode the animation data does not support
    jumping to particular frames in the animation, or even "rewinding" the
    animation to the beginning (for looping). Furthermore, if the image data
    comes from a sequential device, it is not possible for the underlying
    animation handler to seek back to frames whose data has already been read
    (making looping altogether impossible).

    To aid in such situations, a QMovie object can be instructed to cache the
    frames, at the added memory cost of keeping the frames in memory for the
    lifetime of the object.

    By default, this property is set to \l CacheNone.

    \sa QMovie::CacheMode
*/

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


QT_END_NAMESPACE


#endif // QT_NO_MOVIE
