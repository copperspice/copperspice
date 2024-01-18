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

#ifndef QABSTRACTANIMATION_H
#define QABSTRACTANIMATION_H

#include <qobject.h>
#include <qscopedpointer.h>

#ifndef QT_NO_ANIMATION

class QAnimationGroup;
class QSequentialAnimationGroup;
class QAnimationDriver;
class QAbstractAnimationPrivate;
class QAnimationDriverPrivate;

class Q_CORE_EXPORT QAbstractAnimation : public QObject
{
   CORE_CS_OBJECT(QAbstractAnimation)

   CORE_CS_PROPERTY_READ(state, state)
   CORE_CS_PROPERTY_NOTIFY(state, stateChanged)

   CORE_CS_PROPERTY_READ(loopCount, loopCount)
   CORE_CS_PROPERTY_WRITE(loopCount, setLoopCount)

   CORE_CS_PROPERTY_READ(currentTime, currentTime)
   CORE_CS_PROPERTY_WRITE(currentTime, setCurrentTime)

   CORE_CS_PROPERTY_READ(currentLoop, currentLoop)
   CORE_CS_PROPERTY_NOTIFY(currentLoop, currentLoopChanged)

   CORE_CS_PROPERTY_READ(direction, direction)
   CORE_CS_PROPERTY_WRITE(direction, setDirection)
   CORE_CS_PROPERTY_NOTIFY(direction, directionChanged)

   CORE_CS_PROPERTY_READ(duration, duration)

 public:
   enum Direction {
      Forward,
      Backward
   };

   enum State {
      Stopped,
      Paused,
      Running
   };

   enum DeletionPolicy {
      KeepWhenStopped = 0,
      DeleteWhenStopped
   };

   CORE_CS_ENUM(State)
   CORE_CS_ENUM(Direction)

   QAbstractAnimation(QObject *parent = nullptr);

   QAbstractAnimation(const QAbstractAnimation &) = delete;
   QAbstractAnimation &operator=(const QAbstractAnimation &) = delete;

   virtual ~QAbstractAnimation();

   State state() const;

   QAnimationGroup *group() const;

   Direction direction() const;
   void setDirection(Direction direction);

   int currentTime() const;
   int currentLoopTime() const;

   int loopCount() const;
   void setLoopCount(int loopCount);
   int currentLoop() const;

   virtual int duration() const = 0;
   int totalDuration() const;

   CORE_CS_SIGNAL_1(Public, void finished())
   CORE_CS_SIGNAL_2(finished)
   CORE_CS_SIGNAL_1(Public, void stateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State oldState))
   CORE_CS_SIGNAL_2(stateChanged, newState, oldState)
   CORE_CS_SIGNAL_1(Public, void currentLoopChanged(int currentLoop))
   CORE_CS_SIGNAL_2(currentLoopChanged, currentLoop)
   CORE_CS_SIGNAL_1(Public, void directionChanged(QAbstractAnimation::Direction newDirection))
   CORE_CS_SIGNAL_2(directionChanged, newDirection)

   CORE_CS_SLOT_1(Public, void start(QAbstractAnimation::DeletionPolicy policy = KeepWhenStopped))
   CORE_CS_SLOT_2(start)
   CORE_CS_SLOT_1(Public, void pause())
   CORE_CS_SLOT_2(pause)
   CORE_CS_SLOT_1(Public, void resume())
   CORE_CS_SLOT_2(resume)
   CORE_CS_SLOT_1(Public, void setPaused(bool paused))
   CORE_CS_SLOT_2(setPaused)
   CORE_CS_SLOT_1(Public, void stop())
   CORE_CS_SLOT_2(stop)
   CORE_CS_SLOT_1(Public, void setCurrentTime(int msecs))
   CORE_CS_SLOT_2(setCurrentTime)

 protected:
   QAbstractAnimation(QAbstractAnimationPrivate &dd, QObject *parent = nullptr);
   bool event(QEvent *event) override;

   virtual void updateCurrentTime(int currentTime) = 0;
   virtual void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
   virtual void updateDirection(QAbstractAnimation::Direction direction);

   QScopedPointer<QAbstractAnimationPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QAbstractAnimation)
};

class Q_CORE_EXPORT QAnimationDriver : public QObject
{
   CORE_CS_OBJECT(QAnimationDriver)

 public:
   QAnimationDriver(QObject *parent = nullptr);
   ~QAnimationDriver();

   void advance();
   void install();

   bool isRunning() const;

 protected:
   virtual void started() {}
   virtual void stopped() {}

   QAnimationDriver(QAnimationDriverPrivate &dd, QObject *parent = nullptr);
   QScopedPointer<QAnimationDriverPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QAnimationDriver)
   friend class QUnifiedTimer;

   void start();
   void stop();
};

#endif // QT_NO_ANIMATION

#endif
