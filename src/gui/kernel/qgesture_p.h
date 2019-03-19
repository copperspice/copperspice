/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QGESTURE_P_H
#define QGESTURE_P_H

#include <qrect.h>
#include <qpoint.h>
#include <qgesture.h>
#include <qelapsedtimer.h>

#ifndef QT_NO_GESTURES

QT_BEGIN_NAMESPACE

class QGesturePrivate
{
   Q_DECLARE_PUBLIC(QGesture)

 public:
   QGesturePrivate()
      : gestureType(Qt::CustomGesture), state(Qt::NoGesture),
        isHotSpotSet(false), gestureCancelPolicy(0) {
   }

   virtual ~QGesturePrivate() {}

   Qt::GestureType gestureType;
   Qt::GestureState state;
   QPointF hotSpot;
   QPointF sceneHotSpot;
   uint isHotSpotSet : 1;
   uint gestureCancelPolicy : 2;

 protected:
   QGesture *q_ptr;
};

class QPanGesturePrivate : public QGesturePrivate
{
   Q_DECLARE_PUBLIC(QPanGesture)

 public:
   QPanGesturePrivate()
      : acceleration(0), xVelocity(0), yVelocity(0) {
   }

   qreal horizontalVelocity() const {
      return xVelocity;
   }
   void setHorizontalVelocity(qreal value) {
      xVelocity = value;
   }
   qreal verticalVelocity() const {
      return yVelocity;
   }
   void setVerticalVelocity(qreal value) {
      yVelocity = value;
   }

   QPointF lastOffset;
   QPointF offset;
   QPoint startPosition;
   qreal acceleration;
   qreal xVelocity;
   qreal yVelocity;
};

class QPinchGesturePrivate : public QGesturePrivate
{
   Q_DECLARE_PUBLIC(QPinchGesture)

 public:
   QPinchGesturePrivate()
      : totalChangeFlags(0), changeFlags(0),
        totalScaleFactor(1), lastScaleFactor(1), scaleFactor(1),
        totalRotationAngle(0), lastRotationAngle(0), rotationAngle(0),
        isNewSequence(true) {
   }

   QPinchGesture::ChangeFlags totalChangeFlags;
   QPinchGesture::ChangeFlags changeFlags;

   QPointF startCenterPoint;
   QPointF lastCenterPoint;
   QPointF centerPoint;

   qreal totalScaleFactor;
   qreal lastScaleFactor;
   qreal scaleFactor;

   qreal totalRotationAngle;
   qreal lastRotationAngle;
   qreal rotationAngle;

   bool isNewSequence;
   QPointF startPosition[2];
};

class QSwipeGesturePrivate : public QGesturePrivate
{
   Q_DECLARE_PUBLIC(QSwipeGesture)

 public:
   QSwipeGesturePrivate()
      : horizontalDirection(QSwipeGesture::NoDirection),
        verticalDirection(QSwipeGesture::NoDirection),
        swipeAngle(0),
        started(false), velocityValue(0) {
   }

   qreal velocity() const {
      return velocityValue;
   }
   void setVelocity(qreal value) {
      velocityValue = value;
   }

   QSwipeGesture::SwipeDirection horizontalDirection;
   QSwipeGesture::SwipeDirection verticalDirection;
   qreal swipeAngle;

   QPoint lastPositions[3];
   bool started;
   qreal velocityValue;
   QElapsedTimer time;
};

class QTapGesturePrivate : public QGesturePrivate
{
   Q_DECLARE_PUBLIC(QTapGesture)

 public:
   QTapGesturePrivate() {
   }

   QPointF position;
};

class QTapAndHoldGesturePrivate : public QGesturePrivate
{
   Q_DECLARE_PUBLIC(QTapAndHoldGesture)

 public:
   QTapAndHoldGesturePrivate()
      : timerId(0) {
   }

   QPointF position;
   int timerId;
   static int Timeout;
};

QT_END_NAMESPACE

#endif // QT_NO_GESTURES

#endif // QGESTURE_P_H
