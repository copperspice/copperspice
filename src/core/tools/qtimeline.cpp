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

#include <qtimeline.h>

#include <qcoreevent.h>
#include <qdebug.h>
#include <qelapsedtimer.h>
#include <qmath.h>

class QTimeLinePrivate
{
   Q_DECLARE_PUBLIC(QTimeLine)

 public:
   QTimeLinePrivate()
      : startTime(0), duration(1000), startFrame(0), endFrame(0),
        updateInterval(1000 / 25),
        totalLoopCount(1), currentLoopCount(0), currentTime(0), timerId(0),
        direction(QTimeLine::Forward), easingCurve(QEasingCurve::InOutSine),
        state(QTimeLine::NotRunning)
   { }

   virtual ~QTimeLinePrivate()
   { }

   int startTime;
   int duration;
   int startFrame;
   int endFrame;
   int updateInterval;
   int totalLoopCount;
   int currentLoopCount;

   int currentTime;
   int timerId;
   QElapsedTimer timer;

   QTimeLine::Direction direction;
   QEasingCurve easingCurve;
   QTimeLine::State state;

   void setState(QTimeLine::State newState) {
      Q_Q(QTimeLine);

      if (newState != state) {
         emit q->stateChanged(state = newState);
      }
   }

   void setCurrentTime(int msecs);

 protected:
   QTimeLine *q_ptr;
};

void QTimeLinePrivate::setCurrentTime(int msecs)
{
   Q_Q(QTimeLine);

   qreal lastValue = q->currentValue();
   int lastFrame = q->currentFrame();

   // Determine if we are looping.
   int elapsed = (direction == QTimeLine::Backward) ? (-msecs +  duration) : msecs;
   int loopCount = elapsed / duration;

   bool looping = (loopCount != currentLoopCount);

#if defined(CS_SHOW_DEBUG_CORE)
   qDebug() << "QTimeLinePrivate::setCurrentTime:" << msecs << duration << "with loopCount" << loopCount
         << "currentLoopCount" << currentLoopCount << "looping" << looping;
#endif

   if (looping) {
      currentLoopCount = loopCount;
   }

   // Normalize msecs to be between 0 and duration, inclusive.
   currentTime = elapsed % duration;

   if (direction == QTimeLine::Backward) {
      currentTime = duration - currentTime;
   }

   // Check if we have reached the end of loopcount.
   bool finished = false;

   if (totalLoopCount && currentLoopCount >= totalLoopCount) {
      finished = true;
      currentTime = (direction == QTimeLine::Backward) ? 0 : duration;
      currentLoopCount = totalLoopCount - 1;
   }

   int currentFrame = q->frameForTime(currentTime);

#if defined(CS_SHOW_DEBUG_CORE)
   qDebug() << "QTimeLinePrivate::setCurrentTime: frameForTime" << currentTime << currentFrame;
#endif

   if (!qFuzzyCompare(lastValue, q->currentValue())) {
      emit q->valueChanged(q->currentValue());
   }

   if (lastFrame != currentFrame) {
      const int transitionframe = (direction == QTimeLine::Forward ? endFrame : startFrame);

      if (looping && !finished && transitionframe != currentFrame) {

#if defined(CS_SHOW_DEBUG_CORE)
         qDebug() << "QTimeLinePrivate::setCurrentTime: transitionframe";
#endif

         emit q->frameChanged(transitionframe);
      }

#if defined(CS_SHOW_DEBUG_CORE)
      else {
         QByteArray reason;

         if (!looping) {
            reason += " not looping";
         }

         if (finished) {
            if (!reason.isEmpty()) {
               reason += " and";
            }

            reason += " finished";
         }

         if (transitionframe == currentFrame) {
            if (!reason.isEmpty()) {
               reason += " and";
            }

            reason += " transitionframe is equal to currentFrame: " + QByteArray::number(currentFrame);
         }

         qDebug("QTimeLinePrivate::setCurrentTime: not transitionframe because %s",  reason.constData());
      }
#endif

      emit q->frameChanged(currentFrame);
   }

   if (finished && state == QTimeLine::Running) {
      q->stop();
      emit q->finished();
   }
}

QTimeLine::QTimeLine(int duration, QObject *parent)
   : QObject(parent), d_ptr(new QTimeLinePrivate)
{
   d_ptr->q_ptr = this;
   setDuration(duration);
}

QTimeLine::~QTimeLine()
{
   Q_D(QTimeLine);

   if (d->state == Running) {
      stop();
   }
}

QTimeLine::State QTimeLine::state() const
{
   Q_D(const QTimeLine);
   return d->state;
}

int QTimeLine::loopCount() const
{
   Q_D(const QTimeLine);
   return d->totalLoopCount;
}

void QTimeLine::setLoopCount(int count)
{
   Q_D(QTimeLine);
   d->totalLoopCount = count;
}

QTimeLine::Direction QTimeLine::direction() const
{
   Q_D(const QTimeLine);
   return d->direction;
}

void QTimeLine::setDirection(Direction direction)
{
   Q_D(QTimeLine);
   d->direction = direction;
   d->startTime = d->currentTime;
   d->timer.start();
}

int QTimeLine::duration() const
{
   Q_D(const QTimeLine);
   return d->duration;
}

void QTimeLine::setDuration(int duration)
{
   Q_D(QTimeLine);

   if (duration <= 0) {
      qWarning("QTimeLine::setDuration() Unable to set duration less than or equal to 0");
      return;
   }

   d->duration = duration;
}

int QTimeLine::startFrame() const
{
   Q_D(const QTimeLine);
   return d->startFrame;
}

void QTimeLine::setStartFrame(int frame)
{
   Q_D(QTimeLine);
   d->startFrame = frame;
}

int QTimeLine::endFrame() const
{
   Q_D(const QTimeLine);
   return d->endFrame;
}

void QTimeLine::setEndFrame(int frame)
{
   Q_D(QTimeLine);
   d->endFrame = frame;
}

void QTimeLine::setFrameRange(int startFrame, int endFrame)
{
   Q_D(QTimeLine);
   d->startFrame = startFrame;
   d->endFrame = endFrame;
}

int QTimeLine::updateInterval() const
{
   Q_D(const QTimeLine);
   return d->updateInterval;
}

void QTimeLine::setUpdateInterval(int interval)
{
   Q_D(QTimeLine);
   d->updateInterval = interval;
}

QTimeLine::CurveShape QTimeLine::curveShape() const
{
   Q_D(const QTimeLine);

   switch (d->easingCurve.type()) {
      case QEasingCurve::InOutSine:
         return EaseInOutCurve;

      case QEasingCurve::InCurve:
         return EaseInCurve;

      case QEasingCurve::OutCurve:
         return EaseOutCurve;

      case QEasingCurve::Linear:
         return LinearCurve;

      case QEasingCurve::SineCurve:
         return SineCurve;

      case QEasingCurve::CosineCurve:
         return CosineCurve;

      default:
         return EaseInOutCurve;
   }
}

void QTimeLine::setCurveShape(CurveShape shape)
{
   switch (shape) {
      case EaseInOutCurve:
         setEasingCurve(QEasingCurve(QEasingCurve::InOutSine));
         break;

      case EaseInCurve:
         setEasingCurve(QEasingCurve(QEasingCurve::InCurve));
         break;

      case EaseOutCurve:
         setEasingCurve(QEasingCurve(QEasingCurve::OutCurve));
         break;

      case LinearCurve:
         setEasingCurve(QEasingCurve(QEasingCurve::Linear));
         break;

      case SineCurve:
         setEasingCurve(QEasingCurve(QEasingCurve::SineCurve));
         break;

      case CosineCurve:
         setEasingCurve(QEasingCurve(QEasingCurve::CosineCurve));
         break;

      default:
         setEasingCurve(QEasingCurve(QEasingCurve::InOutSine));
         break;
   }
}

QEasingCurve QTimeLine::easingCurve() const
{
   Q_D(const QTimeLine);
   return d->easingCurve;
}

void QTimeLine::setEasingCurve(const QEasingCurve &curve)
{
   Q_D(QTimeLine);
   d->easingCurve = curve;
}

int QTimeLine::currentTime() const
{
   Q_D(const QTimeLine);
   return d->currentTime;
}
void QTimeLine::setCurrentTime(int msec)
{
   Q_D(QTimeLine);
   d->startTime = 0;
   d->currentLoopCount = 0;
   d->timer.restart();
   d->setCurrentTime(msec);
}

int QTimeLine::currentFrame() const
{
   Q_D(const QTimeLine);
   return frameForTime(d->currentTime);
}

qreal QTimeLine::currentValue() const
{
   Q_D(const QTimeLine);
   return valueForTime(d->currentTime);
}

int QTimeLine::frameForTime(int msec) const
{
   Q_D(const QTimeLine);

   if (d->direction == Forward) {
      return d->startFrame + int((d->endFrame - d->startFrame) * valueForTime(msec));
   }

   return d->startFrame + qCeil((d->endFrame - d->startFrame) * valueForTime(msec));
}

qreal QTimeLine::valueForTime(int msec) const
{
   Q_D(const QTimeLine);
   msec = qMin(qMax(msec, 0), d->duration);

   qreal value = msec / qreal(d->duration);
   return d->easingCurve.valueForProgress(value);
}

void QTimeLine::start()
{
   Q_D(QTimeLine);

   if (d->timerId) {
      qWarning("QTimeLine::start() Already running");
      return;
   }

   int curTime = 0;

   if (d->direction == Backward) {
      curTime = d->duration;
   }

   d->timerId = startTimer(d->updateInterval);
   d->startTime = curTime;
   d->currentLoopCount = 0;
   d->timer.start();
   d->setState(Running);
   d->setCurrentTime(curTime);
}

void QTimeLine::resume()
{
   Q_D(QTimeLine);

   if (d->timerId) {
      qWarning("QTimeLine::resume() Already running");
      return;
   }

   d->timerId = startTimer(d->updateInterval);
   d->startTime = d->currentTime;
   d->timer.start();
   d->setState(Running);
}

void QTimeLine::stop()
{
   Q_D(QTimeLine);

   if (d->timerId) {
      killTimer(d->timerId);
   }

   d->setState(NotRunning);
   d->timerId = 0;
}

void QTimeLine::setPaused(bool paused)
{
   Q_D(QTimeLine);

   if (d->state == NotRunning) {
      qWarning("QTimeLine::setPaused() Not running");
      return;
   }

   if (paused && d->state != Paused) {
      d->startTime = d->currentTime;
      killTimer(d->timerId);
      d->timerId = 0;
      d->setState(Paused);
   } else if (!paused && d->state == Paused) {
      // Same as resume()
      d->timerId = startTimer(d->updateInterval);
      d->startTime = d->currentTime;
      d->timer.start();
      d->setState(Running);
   }
}

void QTimeLine::toggleDirection()
{
   Q_D(QTimeLine);
   setDirection(d->direction == Forward ? Backward : Forward);
}

void QTimeLine::timerEvent(QTimerEvent *event)
{
   Q_D(QTimeLine);

   if (event->timerId() != d->timerId) {
      event->ignore();
      return;
   }

   event->accept();

   if (d->direction == Forward) {
      d->setCurrentTime(d->startTime + d->timer.elapsed());
   } else {
      d->setCurrentTime(d->startTime - d->timer.elapsed());
   }
}
