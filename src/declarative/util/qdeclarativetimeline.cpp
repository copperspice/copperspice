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

#include <algorithm>
#include <qdeclarativetimeline_p_p.h>

#include <QDebug>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QEvent>
#include <QCoreApplication>
#include <QEasingCurve>
#include <QTime>

QT_BEGIN_NAMESPACE

struct Update {
   Update(QDeclarativeTimeLineValue *_g, qreal _v)
      : g(_g), v(_v) {}
   Update(const QDeclarativeTimeLineCallback &_e)
      : g(0), v(0), e(_e) {}

   QDeclarativeTimeLineValue *g;
   qreal v;
   QDeclarativeTimeLineCallback e;
};

struct QDeclarativeTimeLinePrivate {
   QDeclarativeTimeLinePrivate(QDeclarativeTimeLine *);

   struct Op {
      enum Type {
         Pause,
         Set,
         Move,
         MoveBy,
         Accel,
         AccelDistance,
         Execute
      };
      Op() {}
      Op(Type t, int l, qreal v, qreal v2, int o,
         const QDeclarativeTimeLineCallback &ev = QDeclarativeTimeLineCallback(), const QEasingCurve &es = QEasingCurve())
         : type(t), length(l), value(v), value2(v2), order(o), event(ev),
           easing(es) {}
      Op(const Op &o)
         : type(o.type), length(o.length), value(o.value), value2(o.value2),
           order(o.order), event(o.event), easing(o.easing) {}
      Op &operator=(const Op &o) {
         type = o.type;
         length = o.length;
         value = o.value;
         value2 = o.value2;
         order = o.order;
         event = o.event;
         easing = o.easing;
         return *this;
      }

      Type type;
      int length;
      qreal value;
      qreal value2;

      int order;
      QDeclarativeTimeLineCallback event;
      QEasingCurve easing;
   };
   struct TimeLine {
      TimeLine() : length(0), consumedOpLength(0), base(0.) {}
      QList<Op> ops;
      int length;
      int consumedOpLength;
      qreal base;
   };

   int length;
   int syncPoint;
   typedef QHash<QDeclarativeTimeLineObject *, TimeLine> Ops;
   Ops ops;
   QDeclarativeTimeLine *q;

   void add(QDeclarativeTimeLineObject &, const Op &);
   qreal value(const Op &op, int time, qreal base, bool *) const;

   int advance(int);

   bool clockRunning;
   int prevTime;

   int order;

   QDeclarativeTimeLine::SyncMode syncMode;
   int syncAdj;
   QList<QPair<int, Update> > *updateQueue;
};

QDeclarativeTimeLinePrivate::QDeclarativeTimeLinePrivate(QDeclarativeTimeLine *parent)
   : length(0), syncPoint(0), q(parent), clockRunning(false), prevTime(0), order(0),
     syncMode(QDeclarativeTimeLine::LocalSync), syncAdj(0), updateQueue(0)
{
}

void QDeclarativeTimeLinePrivate::add(QDeclarativeTimeLineObject &g, const Op &o)
{
   if (g._t && g._t != q) {
      qWarning() << "QDeclarativeTimeLine: Cannot modify a QDeclarativeTimeLineValue owned by"
                 << "another timeline.";
      return;
   }
   g._t = q;

   Ops::Iterator iter = ops.find(&g);
   if (iter == ops.end()) {
      iter = ops.insert(&g, TimeLine());
      if (syncPoint > 0) {
         q->pause(g, syncPoint);
      }
   }
   if (!iter->ops.isEmpty() &&
         o.type == Op::Pause &&
         iter->ops.last().type == Op::Pause) {
      iter->ops.last().length += o.length;
      iter->length += o.length;
   } else {
      iter->ops.append(o);
      iter->length += o.length;
   }

   if (iter->length > length) {
      length = iter->length;
   }

   if (!clockRunning) {
      q->stop();
      prevTime = 0;
      clockRunning = true;

      if (syncMode == QDeclarativeTimeLine::LocalSync)  {
         syncAdj = -1;
      } else {
         syncAdj = 0;
      }
      q->start();
      /*        q->tick(0);
              if (syncMode == QDeclarativeTimeLine::LocalSync)  {
                  syncAdj = -1;
              } else {
                  syncAdj = 0;
              }
              */
   }
}

qreal QDeclarativeTimeLinePrivate::value(const Op &op, int time, qreal base, bool *changed) const
{
   Q_ASSERT(time >= 0);
   Q_ASSERT(time <= op.length);
   *changed = true;

   switch (op.type) {
      case Op::Pause:
         *changed = false;
         return base;
      case Op::Set:
         return op.value;
      case Op::Move:
         if (time == 0) {
            return base;
         } else if (time == (op.length)) {
            return op.value;
         } else {
            qreal delta = op.value - base;
            qreal pTime = (qreal)(time) / (qreal)op.length;
            if (op.easing.type() == QEasingCurve::Linear) {
               return base + delta * pTime;
            } else {
               return base + delta * op.easing.valueForProgress(pTime);
            }
         }
      case Op::MoveBy:
         if (time == 0) {
            return base;
         } else if (time == (op.length)) {
            return base + op.value;
         } else {
            qreal delta = op.value;
            qreal pTime = (qreal)(time) / (qreal)op.length;
            if (op.easing.type() == QEasingCurve::Linear) {
               return base + delta * pTime;
            } else {
               return base + delta * op.easing.valueForProgress(pTime);
            }
         }
      case Op::Accel:
         if (time == 0) {
            return base;
         } else {
            qreal t = (qreal)(time) / 1000.0f;
            qreal delta = op.value * t + 0.5f * op.value2 * t * t;
            return base + delta;
         }
      case Op::AccelDistance:
         if (time == 0) {
            return base;
         } else if (time == (op.length)) {
            return base + op.value2;
         } else {
            qreal t = (qreal)(time) / 1000.0f;
            qreal accel = -1.0f * 1000.0f * op.value / (qreal)op.length;
            qreal delta = op.value * t + 0.5f * accel * t * t;
            return base + delta;

         }
      case Op::Execute:
         op.event.d0(op.event.d1);
         *changed = false;
         return -1;
   }

   return base;
}

/*!
    \internal
    \class QDeclarativeTimeLine
    \brief The QDeclarativeTimeLine class provides a timeline for controlling animations.

    QDeclarativeTimeLine is similar to QTimeLine except:
    \list
    \i It updates QDeclarativeTimeLineValue instances directly, rather than maintaining a single
    current value.

    For example, the following animates a simple value over 200 milliseconds:
    \code
    QDeclarativeTimeLineValue v(<starting value>);
    QDeclarativeTimeLine tl;
    tl.move(v, 100., 200);
    tl.start()
    \endcode

    If your program needs to know when values are changed, it can either
    connect to the QDeclarativeTimeLine's updated() signal, or inherit from QDeclarativeTimeLineValue
    and reimplement the QDeclarativeTimeLineValue::setValue() method.

    \i Supports multiple QDeclarativeTimeLineValue, arbitrary start and end values and allows
    animations to be strung together for more complex effects.

    For example, the following animation moves the x and y coordinates of
    an object from wherever they are to the position (100, 100) in 50
    milliseconds and then further animates them to (100, 200) in 50
    milliseconds:

    \code
    QDeclarativeTimeLineValue x(<starting value>);
    QDeclarativeTimeLineValue y(<starting value>);

    QDeclarativeTimeLine tl;
    tl.start();

    tl.move(x, 100., 50);
    tl.move(y, 100., 50);
    tl.move(y, 200., 50);
    \endcode

    \i All QDeclarativeTimeLine instances share a single, synchronized clock.

    Actions scheduled within the same event loop tick are scheduled
    synchronously against each other, regardless of the wall time between the
    scheduling.  Synchronized scheduling applies both to within the same
    QDeclarativeTimeLine and across separate QDeclarativeTimeLine's within the same process.

    \endlist

    Currently easing functions are not supported.
*/


/*!
    Construct a new QDeclarativeTimeLine with the specified \a parent.
*/
QDeclarativeTimeLine::QDeclarativeTimeLine(QObject *parent)
   : QAbstractAnimation(parent)
{
   d = new QDeclarativeTimeLinePrivate(this);
}

/*!
    Destroys the time line.  Any inprogress animations are canceled, but not
    completed.
*/
QDeclarativeTimeLine::~QDeclarativeTimeLine()
{
   for (QDeclarativeTimeLinePrivate::Ops::Iterator iter = d->ops.begin();
         iter != d->ops.end();
         ++iter) {
      iter.key()->_t = 0;
   }

   delete d;
   d = 0;
}

/*!
    \enum QDeclarativeTimeLine::SyncMode
 */

/*!
    Return the timeline's synchronization mode.
 */
QDeclarativeTimeLine::SyncMode QDeclarativeTimeLine::syncMode() const
{
   return d->syncMode;
}

/*!
    Set the timeline's synchronization mode to \a syncMode.
 */
void QDeclarativeTimeLine::setSyncMode(SyncMode syncMode)
{
   d->syncMode = syncMode;
}

/*!
    Pause \a obj for \a time milliseconds.
*/
void QDeclarativeTimeLine::pause(QDeclarativeTimeLineObject &obj, int time)
{
   if (time <= 0) {
      return;
   }
   QDeclarativeTimeLinePrivate::Op op(QDeclarativeTimeLinePrivate::Op::Pause, time, 0., 0., d->order++);
   d->add(obj, op);
}

/*!
    Execute the \a event.
 */
void QDeclarativeTimeLine::callback(const QDeclarativeTimeLineCallback &callback)
{
   QDeclarativeTimeLinePrivate::Op op(QDeclarativeTimeLinePrivate::Op::Execute, 0, 0, 0., d->order++, callback);
   d->add(*callback.callbackObject(), op);
}

/*!
    Set the \a value of \a timeLineValue.
*/
void QDeclarativeTimeLine::set(QDeclarativeTimeLineValue &timeLineValue, qreal value)
{
   QDeclarativeTimeLinePrivate::Op op(QDeclarativeTimeLinePrivate::Op::Set, 0, value, 0., d->order++);
   d->add(timeLineValue, op);
}

/*!
    Decelerate \a timeLineValue from the starting \a velocity to zero at the
    given \a acceleration rate.  Although the \a acceleration is technically
    a deceleration, it should always be positive.  The QDeclarativeTimeLine will ensure
    that the deceleration is in the opposite direction to the initial velocity.
*/
int QDeclarativeTimeLine::accel(QDeclarativeTimeLineValue &timeLineValue, qreal velocity, qreal acceleration)
{
   if (acceleration == 0.0f) {
      return -1;
   }

   if ((velocity > 0.0f) == (acceleration > 0.0f)) {
      acceleration = acceleration * -1.0f;
   }

   int time = static_cast<int>(-1000 * velocity / acceleration);

   QDeclarativeTimeLinePrivate::Op op(QDeclarativeTimeLinePrivate::Op::Accel, time, velocity, acceleration, d->order++);
   d->add(timeLineValue, op);

   return time;
}

/*!
    \overload

    Decelerate \a timeLineValue from the starting \a velocity to zero at the
    given \a acceleration rate over a maximum distance of maxDistance.

    If necessary, QDeclarativeTimeLine will reduce the acceleration to ensure that the
    entire operation does not require a move of more than \a maxDistance.
    \a maxDistance should always be positive.
*/
int QDeclarativeTimeLine::accel(QDeclarativeTimeLineValue &timeLineValue, qreal velocity, qreal acceleration,
                                qreal maxDistance)
{
   if (maxDistance == 0.0f || acceleration == 0.0f) {
      return -1;
   }

   Q_ASSERT(acceleration > 0.0f && maxDistance > 0.0f);

   qreal maxAccel = (velocity * velocity) / (2.0f * maxDistance);
   if (maxAccel > acceleration) {
      acceleration = maxAccel;
   }

   if ((velocity > 0.0f) == (acceleration > 0.0f)) {
      acceleration = acceleration * -1.0f;
   }

   int time = static_cast<int>(-1000 * velocity / acceleration);

   QDeclarativeTimeLinePrivate::Op op(QDeclarativeTimeLinePrivate::Op::Accel, time, velocity, acceleration, d->order++);
   d->add(timeLineValue, op);

   return time;
}

/*!
    Decelerate \a timeLineValue from the starting \a velocity to zero over the given
    \a distance.  This is like accel(), but the QDeclarativeTimeLine calculates the exact
    deceleration to use.

    \a distance should be positive.
*/
int QDeclarativeTimeLine::accelDistance(QDeclarativeTimeLineValue &timeLineValue, qreal velocity, qreal distance)
{
   if (distance == 0.0f || velocity == 0.0f) {
      return -1;
   }

   Q_ASSERT((distance >= 0.0f) == (velocity >= 0.0f));

   int time = static_cast<int>(1000 * (2.0f * distance) / velocity);

   QDeclarativeTimeLinePrivate::Op op(QDeclarativeTimeLinePrivate::Op::AccelDistance, time, velocity, distance,
                                      d->order++);
   d->add(timeLineValue, op);

   return time;
}

/*!
    Linearly change the \a timeLineValue from its current value to the given
    \a destination value over \a time milliseconds.
*/
void QDeclarativeTimeLine::move(QDeclarativeTimeLineValue &timeLineValue, qreal destination, int time)
{
   if (time <= 0) {
      return;
   }
   QDeclarativeTimeLinePrivate::Op op(QDeclarativeTimeLinePrivate::Op::Move, time, destination, 0.0f, d->order++);
   d->add(timeLineValue, op);
}

/*!
    Change the \a timeLineValue from its current value to the given \a destination
    value over \a time milliseconds using the \a easing curve.
 */
void QDeclarativeTimeLine::move(QDeclarativeTimeLineValue &timeLineValue, qreal destination, const QEasingCurve &easing,
                                int time)
{
   if (time <= 0) {
      return;
   }
   QDeclarativeTimeLinePrivate::Op op(QDeclarativeTimeLinePrivate::Op::Move, time, destination, 0.0f, d->order++,
                                      QDeclarativeTimeLineCallback(), easing);
   d->add(timeLineValue, op);
}

/*!
    Linearly change the \a timeLineValue from its current value by the \a change amount
    over \a time milliseconds.
*/
void QDeclarativeTimeLine::moveBy(QDeclarativeTimeLineValue &timeLineValue, qreal change, int time)
{
   if (time <= 0) {
      return;
   }
   QDeclarativeTimeLinePrivate::Op op(QDeclarativeTimeLinePrivate::Op::MoveBy, time, change, 0.0f, d->order++);
   d->add(timeLineValue, op);
}

/*!
    Change the \a timeLineValue from its current value by the \a change amount over
    \a time milliseconds using the \a easing curve.
 */
void QDeclarativeTimeLine::moveBy(QDeclarativeTimeLineValue &timeLineValue, qreal change, const QEasingCurve &easing,
                                  int time)
{
   if (time <= 0) {
      return;
   }
   QDeclarativeTimeLinePrivate::Op op(QDeclarativeTimeLinePrivate::Op::MoveBy, time, change, 0.0f, d->order++,
                                      QDeclarativeTimeLineCallback(), easing);
   d->add(timeLineValue, op);
}

/*!
    Cancel (but don't complete) all scheduled actions for \a timeLineValue.
*/
void QDeclarativeTimeLine::reset(QDeclarativeTimeLineValue &timeLineValue)
{
   if (!timeLineValue._t) {
      return;
   }
   if (timeLineValue._t != this) {
      qWarning() << "QDeclarativeTimeLine: Cannot reset a QDeclarativeTimeLineValue owned by another timeline.";
      return;
   }
   remove(&timeLineValue);
   timeLineValue._t = 0;
}

int QDeclarativeTimeLine::duration() const
{
   return -1;
}

/*!
    Synchronize the end point of \a timeLineValue to the endpoint of \a syncTo
    within this timeline.

    Following operations on \a timeLineValue in this timeline will be scheduled after
    all the currently scheduled actions on \a syncTo are complete.  In
    pseudo-code this is equivalent to:
    \code
    QDeclarativeTimeLine::pause(timeLineValue, min(0, length_of(syncTo) - length_of(timeLineValue)))
    \endcode
*/
void QDeclarativeTimeLine::sync(QDeclarativeTimeLineValue &timeLineValue, QDeclarativeTimeLineValue &syncTo)
{
   QDeclarativeTimeLinePrivate::Ops::Iterator iter = d->ops.find(&syncTo);
   if (iter == d->ops.end()) {
      return;
   }
   int length = iter->length;

   iter = d->ops.find(&timeLineValue);
   if (iter == d->ops.end()) {
      pause(timeLineValue, length);
   } else {
      int glength = iter->length;
      pause(timeLineValue, length - glength);
   }
}

/*!
    Synchronize the end point of \a timeLineValue to the endpoint of the longest
    action cursrently scheduled in the timeline.

    In pseudo-code, this is equivalent to:
    \code
    QDeclarativeTimeLine::pause(timeLineValue, length_of(timeline) - length_of(timeLineValue))
    \endcode
*/
void QDeclarativeTimeLine::sync(QDeclarativeTimeLineValue &timeLineValue)
{
   QDeclarativeTimeLinePrivate::Ops::Iterator iter = d->ops.find(&timeLineValue);
   if (iter == d->ops.end()) {
      pause(timeLineValue, d->length);
   } else {
      pause(timeLineValue, d->length - iter->length);
   }
}

/*
    Synchronize all currently and future scheduled values in this timeline to
    the longest action currently scheduled.

    For example:
    \code
    value1->setValue(0.);
    value2->setValue(0.);
    value3->setValue(0.);
    QDeclarativeTimeLine tl;
    ...
    tl.move(value1, 10, 200);
    tl.move(value2, 10, 100);
    tl.sync();
    tl.move(value2, 20, 100);
    tl.move(value3, 20, 100);
    \endcode

    will result in:

    \table
    \header \o \o 0ms \o 50ms \o 100ms \o 150ms \o 200ms \o 250ms \o 300ms
    \row \o value1 \o 0 \o 2.5 \o 5.0 \o 7.5 \o 10 \o 10 \o 10
    \row \o value2 \o 0 \o 5.0 \o 10.0 \o 10.0 \o 10.0 \o 15.0 \o 20.0
    \row \o value2 \o 0 \o 0 \o 0 \o 0 \o 0 \o 10.0 \o 20.0
    \endtable
*/

/*void QDeclarativeTimeLine::sync()
{
    for (QDeclarativeTimeLinePrivate::Ops::Iterator iter = d->ops.begin();
            iter != d->ops.end();
            ++iter)
        pause(*iter.key(), d->length - iter->length);
    d->syncPoint = d->length;
}*/

/*!
    \internal

    Temporary hack.
 */
void QDeclarativeTimeLine::setSyncPoint(int sp)
{
   d->syncPoint = sp;
}

/*!
    \internal

    Temporary hack.
 */
int QDeclarativeTimeLine::syncPoint() const
{
   return d->syncPoint;
}

/*!
    Returns true if the timeline is active.  An active timeline is one where
    QDeclarativeTimeLineValue actions are still pending.
*/
bool QDeclarativeTimeLine::isActive() const
{
   return !d->ops.isEmpty();
}

/*!
    Completes the timeline.  All queued actions are played to completion, and then discarded.  For example,
    \code
    QDeclarativeTimeLineValue v(0.);
    QDeclarativeTimeLine tl;
    tl.move(v, 100., 1000.);
    // 500 ms passes
    // v.value() == 50.
    tl.complete();
    // v.value() == 100.
    \endcode
*/
void QDeclarativeTimeLine::complete()
{
   d->advance(d->length);
}

/*!
    Resets the timeline.  All queued actions are discarded and QDeclarativeTimeLineValue's retain their current value. For example,
    \code
    QDeclarativeTimeLineValue v(0.);
    QDeclarativeTimeLine tl;
    tl.move(v, 100., 1000.);
    // 500 ms passes
    // v.value() == 50.
    tl.clear();
    // v.value() == 50.
    \endcode
*/
void QDeclarativeTimeLine::clear()
{
   for (QDeclarativeTimeLinePrivate::Ops::ConstIterator iter = d->ops.begin(); iter != d->ops.end(); ++iter) {
      iter.key()->_t = 0;
   }
   d->ops.clear();
   d->length = 0;
   d->syncPoint = 0;
   //XXX need stop here?
}

int QDeclarativeTimeLine::time() const
{
   return d->prevTime;
}

/*!
    \fn void QDeclarativeTimeLine::updated()

    Emitted each time the timeline modifies QDeclarativeTimeLineValues.  Even if multiple
    QDeclarativeTimeLineValues are changed, this signal is only emitted once for each clock tick.
*/

void QDeclarativeTimeLine::updateCurrentTime(int v)
{
   if (d->syncAdj == -1) {
      d->syncAdj = v;
   }
   v -= d->syncAdj;

   int timeChanged = v - d->prevTime;
#if 0
   if (!timeChanged) {
      return;
   }
#endif
   d->prevTime = v;
   d->advance(timeChanged);
   emit updated();

   // Do we need to stop the clock?
   if (d->ops.isEmpty()) {
      stop();
      d->prevTime = 0;
      d->clockRunning = false;
      emit completed();
   } /*else if (pauseTime > 0) {
        GfxClock::cancelClock();
        d->prevTime = 0;
        GfxClock::pauseFor(pauseTime);
        d->syncAdj = 0;
        d->clockRunning = false;
    }*/ else if (/*!GfxClock::isActive()*/ state() != Running) {
      stop();
      d->prevTime = 0;
      d->clockRunning = true;
      d->syncAdj = 0;
      start();
   }
}

bool operator<(const QPair<int, Update> &lhs,
               const QPair<int, Update> &rhs)
{
   return lhs.first < rhs.first;
}

int QDeclarativeTimeLinePrivate::advance(int t)
{
   int pauseTime = -1;

   // XXX - surely there is a more efficient way?
   do {
      pauseTime = -1;
      // Minimal advance time
      int advanceTime = t;
      for (Ops::Iterator iter = ops.begin(); iter != ops.end(); ++iter) {
         TimeLine &tl = *iter;
         Op &op = tl.ops.first();
         int length = op.length - tl.consumedOpLength;

         if (length < advanceTime) {
            advanceTime = length;
            if (advanceTime == 0) {
               break;
            }
         }
      }
      t -= advanceTime;

      // Process until then.  A zero length advance time will only process
      // sets.
      QList<QPair<int, Update> > updates;

      for (Ops::Iterator iter = ops.begin(); iter != ops.end(); ) {
         QDeclarativeTimeLineValue *v = static_cast<QDeclarativeTimeLineValue *>(iter.key());
         TimeLine &tl = *iter;
         Q_ASSERT(!tl.ops.isEmpty());

         do {
            Op &op = tl.ops.first();
            if (advanceTime == 0 && op.length != 0) {
               continue;
            }

            if (tl.consumedOpLength == 0 &&
                  op.type != Op::Pause &&
                  op.type != Op::Execute) {
               tl.base = v->value();
            }

            if ((tl.consumedOpLength + advanceTime) == op.length) {
               if (op.type == Op::Execute) {
                  updates << qMakePair(op.order, Update(op.event));
               } else {
                  bool changed = false;
                  qreal val = value(op, op.length, tl.base, &changed);
                  if (changed) {
                     updates << qMakePair(op.order, Update(v, val));
                  }
               }
               tl.length -= qMin(advanceTime, tl.length);
               tl.consumedOpLength = 0;
               tl.ops.removeFirst();
            } else {
               tl.consumedOpLength += advanceTime;
               bool changed = false;
               qreal val = value(op, tl.consumedOpLength, tl.base, &changed);
               if (changed) {
                  updates << qMakePair(op.order, Update(v, val));
               }
               tl.length -= qMin(advanceTime, tl.length);
               break;
            }

         } while (!tl.ops.isEmpty() && advanceTime == 0 && tl.ops.first().length == 0);


         if (tl.ops.isEmpty()) {
            iter = ops.erase(iter);
            v->_t = 0;
         } else {
            if (tl.ops.first().type == Op::Pause && pauseTime != 0) {
               int opPauseTime = tl.ops.first().length - tl.consumedOpLength;
               if (pauseTime == -1 || opPauseTime < pauseTime) {
                  pauseTime = opPauseTime;
               }
            } else {
               pauseTime = 0;
            }
            ++iter;
         }
      }

      length -= qMin(length, advanceTime);
      syncPoint -= advanceTime;

      std::sort(updates.begin(), updates.end());
      updateQueue = &updates;

      for (int ii = 0; ii < updates.count(); ++ii) {
         const Update &v = updates.at(ii).second;
         if (v.g) {
            v.g->setValue(v.v);
         } else {
            v.e.d0(v.e.d1);
         }
      }
      updateQueue = 0;
   } while (t);

   return pauseTime;
}

void QDeclarativeTimeLine::remove(QDeclarativeTimeLineObject *v)
{
   QDeclarativeTimeLinePrivate::Ops::Iterator iter = d->ops.find(v);
   Q_ASSERT(iter != d->ops.end());

   int len = iter->length;
   d->ops.erase(iter);
   if (len == d->length) {
      // We need to recalculate the length
      d->length = 0;
      for (QDeclarativeTimeLinePrivate::Ops::Iterator iter = d->ops.begin();
            iter != d->ops.end();
            ++iter) {

         if (iter->length > d->length) {
            d->length = iter->length;
         }

      }
   }
   if (d->ops.isEmpty()) {
      stop();
      d->clockRunning = false;
   } else if (/*!GfxClock::isActive()*/ state() != Running) {
      stop();
      d->prevTime = 0;
      d->clockRunning = true;

      if (d->syncMode == QDeclarativeTimeLine::LocalSync) {
         d->syncAdj = -1;
      } else {
         d->syncAdj = 0;
      }
      start();
   }

   if (d->updateQueue) {
      for (int ii = 0; ii < d->updateQueue->count(); ++ii) {
         if (d->updateQueue->at(ii).second.g == v ||
               d->updateQueue->at(ii).second.e.callbackObject() == v) {
            d->updateQueue->removeAt(ii);
            --ii;
         }
      }
   }


}

/*!
    \internal
    \class QDeclarativeTimeLineValue
    \brief The QDeclarativeTimeLineValue class provides a value that can be modified by QDeclarativeTimeLine.
*/

/*!
    \fn QDeclarativeTimeLineValue::QDeclarativeTimeLineValue(qreal value = 0)

    Construct a new QDeclarativeTimeLineValue with an initial \a value.
*/

/*!
    \fn qreal QDeclarativeTimeLineValue::value() const

    Return the current value.
*/

/*!
    \fn void QDeclarativeTimeLineValue::setValue(qreal value)

    Set the current \a value.
*/

/*!
    \fn QDeclarativeTimeLine *QDeclarativeTimeLineValue::timeLine() const

    If a QDeclarativeTimeLine is operating on this value, return a pointer to it,
    otherwise return null.
*/


QDeclarativeTimeLineObject::QDeclarativeTimeLineObject()
   : _t(0)
{
}

QDeclarativeTimeLineObject::~QDeclarativeTimeLineObject()
{
   if (_t) {
      _t->remove(this);
      _t = 0;
   }
}

QDeclarativeTimeLineCallback::QDeclarativeTimeLineCallback()
   : d0(0), d1(0), d2(0)
{
}

QDeclarativeTimeLineCallback::QDeclarativeTimeLineCallback(QDeclarativeTimeLineObject *b, Callback f, void *d)
   : d0(f), d1(d), d2(b)
{
}

QDeclarativeTimeLineCallback::QDeclarativeTimeLineCallback(const QDeclarativeTimeLineCallback &o)
   : d0(o.d0), d1(o.d1), d2(o.d2)
{
}

QDeclarativeTimeLineCallback &QDeclarativeTimeLineCallback::operator=(const QDeclarativeTimeLineCallback &o)
{
   d0 = o.d0;
   d1 = o.d1;
   d2 = o.d2;
   return *this;
}

QDeclarativeTimeLineObject *QDeclarativeTimeLineCallback::callbackObject() const
{
   return d2;
}

QT_END_NAMESPACE
