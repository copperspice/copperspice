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

#ifndef QTIMELINE_H
#define QTIMELINE_H

#include <QtCore/qeasingcurve.h>
#include <QtCore/qobject.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QTimeLinePrivate;

class Q_CORE_EXPORT QTimeLine : public QObject
{
   CORE_CS_OBJECT(QTimeLine)

   CORE_CS_PROPERTY_READ(duration, duration)
   CORE_CS_PROPERTY_WRITE(duration, setDuration)
   CORE_CS_PROPERTY_READ(updateInterval, updateInterval)
   CORE_CS_PROPERTY_WRITE(updateInterval, setUpdateInterval)
   CORE_CS_PROPERTY_READ(currentTime, currentTime)
   CORE_CS_PROPERTY_WRITE(currentTime, setCurrentTime)
   CORE_CS_PROPERTY_READ(direction, direction)
   CORE_CS_PROPERTY_WRITE(direction, setDirection)
   CORE_CS_PROPERTY_READ(loopCount, loopCount)
   CORE_CS_PROPERTY_WRITE(loopCount, setLoopCount)
   CORE_CS_PROPERTY_READ(curveShape, curveShape)
   CORE_CS_PROPERTY_WRITE(curveShape, setCurveShape)
   CORE_CS_PROPERTY_READ(easingCurve, easingCurve)
   CORE_CS_PROPERTY_WRITE(easingCurve, setEasingCurve)

 public:
   enum State {
      NotRunning,
      Paused,
      Running
   };
   enum Direction {
      Forward,
      Backward
   };
   enum CurveShape {
      EaseInCurve,
      EaseOutCurve,
      EaseInOutCurve,
      LinearCurve,
      SineCurve,
      CosineCurve
   };

   explicit QTimeLine(int duration = 1000, QObject *parent = nullptr);
   virtual ~QTimeLine();

   State state() const;

   int loopCount() const;
   void setLoopCount(int count);

   Direction direction() const;
   void setDirection(Direction direction);

   int duration() const;
   void setDuration(int duration);

   int startFrame() const;
   void setStartFrame(int frame);
   int endFrame() const;
   void setEndFrame(int frame);
   void setFrameRange(int startFrame, int endFrame);

   int updateInterval() const;
   void setUpdateInterval(int interval);

   CurveShape curveShape() const;
   void setCurveShape(CurveShape shape);

   QEasingCurve easingCurve() const;
   void setEasingCurve(const QEasingCurve &curve);

   int currentTime() const;
   int currentFrame() const;
   qreal currentValue() const;

   int frameForTime(int msec) const;
   virtual qreal valueForTime(int msec) const;

   CORE_CS_SLOT_1(Public, void start())
   CORE_CS_SLOT_2(start)
   CORE_CS_SLOT_1(Public, void resume())
   CORE_CS_SLOT_2(resume)
   CORE_CS_SLOT_1(Public, void stop())
   CORE_CS_SLOT_2(stop)
   CORE_CS_SLOT_1(Public, void setPaused(bool paused))
   CORE_CS_SLOT_2(setPaused)
   CORE_CS_SLOT_1(Public, void setCurrentTime(int msec))
   CORE_CS_SLOT_2(setCurrentTime)
   CORE_CS_SLOT_1(Public, void toggleDirection())
   CORE_CS_SLOT_2(toggleDirection)

   CORE_CS_SIGNAL_1(Public, void valueChanged(qreal x))
   CORE_CS_SIGNAL_2(valueChanged, x)
   CORE_CS_SIGNAL_1(Public, void frameChanged(int un_named_arg1))
   CORE_CS_SIGNAL_2(frameChanged, un_named_arg1)
   CORE_CS_SIGNAL_1(Public, void stateChanged(QTimeLine::State newState))
   CORE_CS_SIGNAL_2(stateChanged, newState)
   CORE_CS_SIGNAL_1(Public, void finished())
   CORE_CS_SIGNAL_2(finished)

 protected:
   void timerEvent(QTimerEvent *event) override;

   QScopedPointer<QTimeLinePrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QTimeLine)
   Q_DECLARE_PRIVATE(QTimeLine)
  
};

QT_END_NAMESPACE

#endif

