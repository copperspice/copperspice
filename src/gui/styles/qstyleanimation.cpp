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

#include <qstyleanimation_p.h>

#ifndef QT_NO_ANIMATION

#include <qcoreapplication.h>
#include <qwidget.h>
#include <qevent.h>


static const qreal ScrollBarFadeOutDuration = 200.0;
static const qreal ScrollBarFadeOutDelay = 450.0;

QStyleAnimation::QStyleAnimation(QObject *target) : QAbstractAnimation(target),
   _delay(0), _duration(-1), _startTime(QTime::currentTime()), _fps(ThirtyFps), _skip(0)
{
}

QStyleAnimation::~QStyleAnimation()
{
}

QObject *QStyleAnimation::target() const
{
   return parent();
}

int QStyleAnimation::duration() const
{
   return _duration;
}

void QStyleAnimation::setDuration(int duration)
{
   _duration = duration;
}

int QStyleAnimation::delay() const
{
   return _delay;
}

void QStyleAnimation::setDelay(int delay)
{
   _delay = delay;
}

QTime QStyleAnimation::startTime() const
{
   return _startTime;
}

void QStyleAnimation::setStartTime(const QTime &time)
{
   _startTime = time;
}

QStyleAnimation::FrameRate QStyleAnimation::frameRate() const
{
   return _fps;
}

void QStyleAnimation::setFrameRate(FrameRate fps)
{
   _fps = fps;
}

void QStyleAnimation::updateTarget()
{
   QEvent event(QEvent::StyleAnimationUpdate);
   event.setAccepted(false);
   QCoreApplication::sendEvent(target(), &event);
   if (!event.isAccepted()) {
      stop();
   }
}

void QStyleAnimation::start()
{
   _skip = 0;
   QAbstractAnimation::start(DeleteWhenStopped);
}

bool QStyleAnimation::isUpdateNeeded() const
{
   return currentTime() > _delay;
}

void QStyleAnimation::updateCurrentTime(int)
{
   if (++_skip >= _fps) {
      _skip = 0;
      if (target() && isUpdateNeeded()) {
         updateTarget();
      }
   }
}

QProgressStyleAnimation::QProgressStyleAnimation(int speed, QObject *target) :
   QStyleAnimation(target), _speed(speed), _step(-1)
{
}

int QProgressStyleAnimation::animationStep() const
{
   return currentTime() / (1000.0 / _speed);
}

int QProgressStyleAnimation::progressStep(int width) const
{
   int step = animationStep();
   int progress = (step * width / _speed) % width;
   if (((step * width / _speed) % (2 * width)) >= width) {
      progress = width - progress;
   }
   return progress;
}

int QProgressStyleAnimation::speed() const
{
   return _speed;
}

void QProgressStyleAnimation::setSpeed(int speed)
{
   _speed = speed;
}

bool QProgressStyleAnimation::isUpdateNeeded() const
{
   if (QStyleAnimation::isUpdateNeeded()) {
      int current = animationStep();
      if (_step == -1 || _step != current) {
         _step = current;
         return true;
      }
   }
   return false;
}

QNumberStyleAnimation::QNumberStyleAnimation(QObject *target) :
   QStyleAnimation(target), _start(0.0), _end(1.0), _prev(0.0)
{
   setDuration(250);
}

qreal QNumberStyleAnimation::startValue() const
{
   return _start;
}

void QNumberStyleAnimation::setStartValue(qreal value)
{
   _start = value;
}

qreal QNumberStyleAnimation::endValue() const
{
   return _end;
}

void QNumberStyleAnimation::setEndValue(qreal value)
{
   _end = value;
}

qreal QNumberStyleAnimation::currentValue() const
{
   qreal step = qreal(currentTime() - delay()) / (duration() - delay());
   return _start + qMax(qreal(0), step) * (_end - _start);
}

bool QNumberStyleAnimation::isUpdateNeeded() const
{
   if (QStyleAnimation::isUpdateNeeded()) {
      qreal current = currentValue();
      if (!qFuzzyCompare(_prev, current)) {
         _prev = current;
         return true;
      }
   }
   return false;
}

QBlendStyleAnimation::QBlendStyleAnimation(Type type, QObject *target) :
   QStyleAnimation(target), _type(type)
{
   setDuration(250);
}

QImage QBlendStyleAnimation::startImage() const
{
   return _start;
}

void QBlendStyleAnimation::setStartImage(const QImage &image)
{
   _start = image;
}

QImage QBlendStyleAnimation::endImage() const
{
   return _end;
}

void QBlendStyleAnimation::setEndImage(const QImage &image)
{
   _end = image;
}

QImage QBlendStyleAnimation::currentImage() const
{
   return _current;
}

/*! \internal

    A helper function to blend two images.

    The result consists of ((alpha)*startImage) + ((1-alpha)*endImage)

*/
static QImage blendedImage(const QImage &start, const QImage &end, float alpha)
{
   if (start.isNull() || end.isNull()) {
      return QImage();
   }

   QImage blended;
   const int a = qRound(alpha * 256);
   const int ia = 256 - a;
   const int sw = start.width();
   const int sh = start.height();
   const int bpl = start.bytesPerLine();
   switch (start.depth()) {
      case 32: {
         blended = QImage(sw, sh, start.format());
         blended.setDevicePixelRatio(start.devicePixelRatio());
         uchar *mixed_data = blended.bits();
         const uchar *back_data = start.bits();
         const uchar *front_data = end.bits();
         for (int sy = 0; sy < sh; sy++) {
            quint32 *mixed = (quint32 *)mixed_data;
            const quint32 *back = (const quint32 *)back_data;
            const quint32 *front = (const quint32 *)front_data;
            for (int sx = 0; sx < sw; sx++) {
               quint32 bp = back[sx];
               quint32 fp = front[sx];
               mixed[sx] =  qRgba ((qRed(bp) * ia + qRed(fp) * a) >> 8,
                     (qGreen(bp) * ia + qGreen(fp) * a) >> 8,
                     (qBlue(bp) * ia + qBlue(fp) * a) >> 8,
                     (qAlpha(bp) * ia + qAlpha(fp) * a) >> 8);
            }
            mixed_data += bpl;
            back_data += bpl;
            front_data += bpl;
         }
      }
      default:
         break;
   }
   return blended;
}

void QBlendStyleAnimation::updateCurrentTime(int time)
{
   QStyleAnimation::updateCurrentTime(time);

   float alpha = 1.0;
   if (duration() > 0) {
      if (_type == Pulse) {
         time = time % duration() * 2;
         if (time > duration()) {
            time = duration() * 2 - time;
         }
      }

      alpha = time / static_cast<float>(duration());

      if (_type == Transition && time > duration()) {
         alpha = 1.0;
         stop();
      }
   } else if (time > 0) {
      stop();
   }

   _current = blendedImage(_start, _end, alpha);
}

QScrollbarStyleAnimation::QScrollbarStyleAnimation(Mode mode, QObject *target) : QNumberStyleAnimation(target), _mode(mode),
   _active(false)
{
   switch (mode) {
      case Activating:
         setDuration(ScrollBarFadeOutDuration);
         setStartValue(0.0);
         setEndValue(1.0);
         break;
      case Deactivating:
         setDuration(ScrollBarFadeOutDelay + ScrollBarFadeOutDuration);
         setDelay(ScrollBarFadeOutDelay);
         setStartValue(1.0);
         setEndValue(0.0);
         break;
   }
}

QScrollbarStyleAnimation::Mode QScrollbarStyleAnimation::mode() const
{
   return _mode;
}

bool QScrollbarStyleAnimation::wasActive() const
{
   return _active;
}

void QScrollbarStyleAnimation::setActive(bool active)
{
   _active = active;
}

void QScrollbarStyleAnimation::updateCurrentTime(int time)
{
   QNumberStyleAnimation::updateCurrentTime(time);
   if (_mode == Deactivating && qFuzzyIsNull(currentValue())) {
      target()->setProperty("visible", false);
   }
}


#endif //QT_NO_ANIMATION
