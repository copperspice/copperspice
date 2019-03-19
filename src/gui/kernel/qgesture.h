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

#ifndef QGESTURE_H
#define QGESTURE_H

#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qmetatype.h>
#include <QScopedPointer>

#ifndef QT_NO_GESTURES

Q_DECLARE_METATYPE(Qt::GestureState)
Q_DECLARE_METATYPE(Qt::GestureType)

QT_BEGIN_NAMESPACE

class QGesturePrivate;
class QPanGesturePrivate;
class QPinchGesturePrivate;
class QTapAndHoldGesturePrivate;
class QSwipeGesturePrivate;
class QTapGesturePrivate;

class Q_GUI_EXPORT QGesture : public QObject
{
   GUI_CS_OBJECT(QGesture)
   Q_DECLARE_PRIVATE(QGesture)

   GUI_CS_PROPERTY_READ(state, state)
   GUI_CS_PROPERTY_READ(gestureType, gestureType)
   GUI_CS_PROPERTY_READ(gestureCancelPolicy, gestureCancelPolicy)
   GUI_CS_PROPERTY_WRITE(gestureCancelPolicy, setGestureCancelPolicy)
   GUI_CS_PROPERTY_READ(hotSpot, hotSpot)
   GUI_CS_PROPERTY_WRITE(hotSpot, setHotSpot)
   GUI_CS_PROPERTY_RESET(hotSpot, unsetHotSpot)
   GUI_CS_PROPERTY_READ(hasHotSpot, hasHotSpot)

 public:
   explicit QGesture(QObject *parent = nullptr);
   ~QGesture();

   Qt::GestureType gestureType() const;

   Qt::GestureState state() const;

   QPointF hotSpot() const;
   void setHotSpot(const QPointF &value);
   bool hasHotSpot() const;
   void unsetHotSpot();

   enum GestureCancelPolicy {
      CancelNone = 0,
      CancelAllInContext
   };

   void setGestureCancelPolicy(GestureCancelPolicy policy);
   GestureCancelPolicy gestureCancelPolicy() const;

 protected:
   QGesture(QGesturePrivate &dd, QObject *parent);
   QScopedPointer<QGesturePrivate> d_ptr;

 private:
   friend class QGestureEvent;
   friend class QGestureRecognizer;
   friend class QGestureManager;
   friend class QGraphicsScenePrivate;

};

class Q_GUI_EXPORT QPanGesture : public QGesture
{
   GUI_CS_OBJECT(QPanGesture)
   Q_DECLARE_PRIVATE(QPanGesture)

   GUI_CS_PROPERTY_READ(lastOffset, lastOffset)
   GUI_CS_PROPERTY_WRITE(lastOffset, setLastOffset)
   GUI_CS_PROPERTY_READ(offset, offset)
   GUI_CS_PROPERTY_WRITE(offset, setOffset)
   GUI_CS_PROPERTY_READ(delta, delta)
   GUI_CS_PROPERTY_STORED(delta, false)
   GUI_CS_PROPERTY_READ(acceleration, acceleration)
   GUI_CS_PROPERTY_WRITE(acceleration, setAcceleration)

   // private properties
   GUI_CS_PROPERTY_READ(horizontalVelocity, cs_horizontalVelocity)
   GUI_CS_PROPERTY_WRITE(horizontalVelocity, cs_setHorizontalVelocity)

   // private properties
   GUI_CS_PROPERTY_READ(verticalVelocity, cs_verticalVelocity)
   GUI_CS_PROPERTY_WRITE(verticalVelocity, cs_setVerticalVelocity)

 public:
   QPanGesture(QObject *parent = nullptr);

   QPointF lastOffset() const;
   QPointF offset() const;
   QPointF delta() const;
   qreal acceleration() const;

   void setLastOffset(const QPointF &value);
   void setOffset(const QPointF &value);
   void setAcceleration(qreal value);

   friend class QPanGestureRecognizer;
   friend class QWinNativePanGestureRecognizer;

 private:

   // wrapper for overloaded method
   qreal cs_horizontalVelocity() const;

   // wrapper for overloaded method
   void cs_setHorizontalVelocity(qreal velocity);

   // wrapper for overloaded method
   qreal cs_verticalVelocity() const;

   // wrapper for overloaded method
   void cs_setVerticalVelocity(qreal velocity);

};

class Q_GUI_EXPORT QPinchGesture : public QGesture
{
   GUI_CS_OBJECT(QPinchGesture)
   Q_DECLARE_PRIVATE(QPinchGesture)

   GUI_CS_FLAG(ChangeFlags, ChangeFlag)

 public:
   enum ChangeFlag {
      ScaleFactorChanged = 0x1,
      RotationAngleChanged = 0x2,
      CenterPointChanged = 0x4
   };
   using ChangeFlags = QFlags<ChangeFlag>;

   GUI_CS_PROPERTY_READ(totalChangeFlags, totalChangeFlags)
   GUI_CS_PROPERTY_WRITE(totalChangeFlags, setTotalChangeFlags)
   GUI_CS_PROPERTY_READ(changeFlags, changeFlags)
   GUI_CS_PROPERTY_WRITE(changeFlags, setChangeFlags)

   GUI_CS_PROPERTY_READ(totalScaleFactor, totalScaleFactor)
   GUI_CS_PROPERTY_WRITE(totalScaleFactor, setTotalScaleFactor)
   GUI_CS_PROPERTY_READ(lastScaleFactor, lastScaleFactor)
   GUI_CS_PROPERTY_WRITE(lastScaleFactor, setLastScaleFactor)
   GUI_CS_PROPERTY_READ(scaleFactor, scaleFactor)
   GUI_CS_PROPERTY_WRITE(scaleFactor, setScaleFactor)

   GUI_CS_PROPERTY_READ(totalRotationAngle, totalRotationAngle)
   GUI_CS_PROPERTY_WRITE(totalRotationAngle, setTotalRotationAngle)
   GUI_CS_PROPERTY_READ(lastRotationAngle, lastRotationAngle)
   GUI_CS_PROPERTY_WRITE(lastRotationAngle, setLastRotationAngle)
   GUI_CS_PROPERTY_READ(rotationAngle, rotationAngle)
   GUI_CS_PROPERTY_WRITE(rotationAngle, setRotationAngle)

   GUI_CS_PROPERTY_READ(startCenterPoint, startCenterPoint)
   GUI_CS_PROPERTY_WRITE(startCenterPoint, setStartCenterPoint)
   GUI_CS_PROPERTY_READ(lastCenterPoint, lastCenterPoint)
   GUI_CS_PROPERTY_WRITE(lastCenterPoint, setLastCenterPoint)
   GUI_CS_PROPERTY_READ(centerPoint, centerPoint)
   GUI_CS_PROPERTY_WRITE(centerPoint, setCenterPoint)

   QPinchGesture(QObject *parent = nullptr);

   ChangeFlags totalChangeFlags() const;
   void setTotalChangeFlags(ChangeFlags value);

   ChangeFlags changeFlags() const;
   void setChangeFlags(ChangeFlags value);

   QPointF startCenterPoint() const;
   QPointF lastCenterPoint() const;
   QPointF centerPoint() const;
   void setStartCenterPoint(const QPointF &value);
   void setLastCenterPoint(const QPointF &value);
   void setCenterPoint(const QPointF &value);

   qreal totalScaleFactor() const;
   qreal lastScaleFactor() const;
   qreal scaleFactor() const;
   void setTotalScaleFactor(qreal value);
   void setLastScaleFactor(qreal value);
   void setScaleFactor(qreal value);

   qreal totalRotationAngle() const;
   qreal lastRotationAngle() const;
   qreal rotationAngle() const;
   void setTotalRotationAngle(qreal value);
   void setLastRotationAngle(qreal value);
   void setRotationAngle(qreal value);

   friend class QPinchGestureRecognizer;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QPinchGesture::ChangeFlags)

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPinchGesture::ChangeFlags)

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QSwipeGesture : public QGesture
{
   GUI_CS_OBJECT(QSwipeGesture)
   Q_DECLARE_PRIVATE(QSwipeGesture)

   GUI_CS_ENUM(SwipeDirection)

   GUI_CS_PROPERTY_READ(horizontalDirection, horizontalDirection)
   GUI_CS_PROPERTY_STORED(horizontalDirection, false)
   GUI_CS_PROPERTY_READ(verticalDirection, verticalDirection)
   GUI_CS_PROPERTY_STORED(verticalDirection, false)
   GUI_CS_PROPERTY_READ(swipeAngle, swipeAngle)
   GUI_CS_PROPERTY_WRITE(swipeAngle, setSwipeAngle)

   // private properties
   GUI_CS_PROPERTY_READ(velocity, cs_velocity)
   GUI_CS_PROPERTY_WRITE(velocity, cs_setVelocity)

 public:
   enum SwipeDirection { NoDirection, Left, Right, Up, Down };
   QSwipeGesture(QObject *parent = nullptr);

   SwipeDirection horizontalDirection() const;
   SwipeDirection verticalDirection() const;

   qreal swipeAngle() const;
   void setSwipeAngle(qreal value);

   friend class QSwipeGestureRecognizer;

 private:
   // wrapper for overloaded method
   qreal cs_velocity() const;

   // wrapper for overloaded method
   void cs_setVelocity(qreal velocity);

};

class Q_GUI_EXPORT QTapGesture : public QGesture
{
   GUI_CS_OBJECT(QTapGesture)
   Q_DECLARE_PRIVATE(QTapGesture)

   GUI_CS_PROPERTY_READ(position, position)
   GUI_CS_PROPERTY_WRITE(position, setPosition)

 public:
   QTapGesture(QObject *parent = nullptr);

   QPointF position() const;
   void setPosition(const QPointF &pos);

   friend class QTapGestureRecognizer;
};

class Q_GUI_EXPORT QTapAndHoldGesture : public QGesture
{
   GUI_CS_OBJECT(QTapAndHoldGesture)
   Q_DECLARE_PRIVATE(QTapAndHoldGesture)

   GUI_CS_PROPERTY_READ(position, position)
   GUI_CS_PROPERTY_WRITE(position, setPosition)

 public:
   QTapAndHoldGesture(QObject *parent = nullptr);

   QPointF position() const;
   void setPosition(const QPointF &pos);

   static void setTimeout(int msecs);
   static int timeout();

   friend class QTapAndHoldGestureRecognizer;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QGesture::GestureCancelPolicy)

#endif // QT_NO_GESTURES

#endif // QGESTURE_H
