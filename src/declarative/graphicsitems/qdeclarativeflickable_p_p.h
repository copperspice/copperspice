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

#ifndef QDECLARATIVEFLICKABLE_P_P_H
#define QDECLARATIVEFLICKABLE_P_P_H

#include <qdeclarativeflickable_p.h>
#include <qdeclarativeitem_p.h>
#include <qdeclarativeitemchangelistener_p.h>
#include <qdeclarative.h>
#include <qdeclarativetimeline_p_p.h>
#include <qdeclarativeanimation_p_p.h>
#include <qdatetime.h>
#include <qplatformdefs.h>

QT_BEGIN_NAMESPACE

// Really slow flicks can be annoying
#ifndef QML_FLICK_MINVELOCITY
#define QML_FLICK_MINVELOCITY 175
#endif

const qreal MinimumFlickVelocity = QML_FLICK_MINVELOCITY;

class QDeclarativeFlickableVisibleArea;
class QDeclarativeFlickablePrivate : public QDeclarativeItemPrivate, public QDeclarativeItemChangeListener
{
   Q_DECLARE_PUBLIC(QDeclarativeFlickable)

 public:
   QDeclarativeFlickablePrivate();
   void init();

   struct Velocity : public QDeclarativeTimeLineValue {
      Velocity(QDeclarativeFlickablePrivate *p)
         : parent(p) {}
      virtual void setValue(qreal v) {
         if (v != value()) {
            QDeclarativeTimeLineValue::setValue(v);
            parent->updateVelocity();
         }
      }
      QDeclarativeFlickablePrivate *parent;
   };

   struct AxisData {
      AxisData(QDeclarativeFlickablePrivate *fp, void (QDeclarativeFlickablePrivate::*func)(qreal))
         : move(fp, func), viewSize(-1), smoothVelocity(fp), atEnd(false), atBeginning(true)
         , fixingUp(false), inOvershoot(false), moving(false), flicking(false) {
      }

      void reset() {
         velocityBuffer.clear();
         dragStartOffset = 0;
         fixingUp = false;
         inOvershoot = false;
      }

      void addVelocitySample(qreal v, qreal maxVelocity);
      void updateVelocity();

      QDeclarativeTimeLineValueProxy<QDeclarativeFlickablePrivate> move;
      qreal viewSize;
      qreal pressPos;
      qreal dragStartOffset;
      qreal dragMinBound;
      qreal dragMaxBound;
      qreal velocity;
      qreal flickTarget;
      QDeclarativeFlickablePrivate::Velocity smoothVelocity;
      QPODVector<qreal, 10> velocityBuffer;
      bool atEnd : 1;
      bool atBeginning : 1;
      bool fixingUp : 1;
      bool inOvershoot : 1;
      bool moving : 1;
      bool flicking : 1;
   };

   void flickX(qreal velocity);
   void flickY(qreal velocity);
   virtual void flick(AxisData &data, qreal minExtent, qreal maxExtent, qreal vSize,
                      QDeclarativeTimeLineCallback::Callback fixupCallback, qreal velocity);

   void fixupX();
   void fixupY();
   virtual void fixup(AxisData &data, qreal minExtent, qreal maxExtent);

   void updateBeginningEnd();

   bool isOutermostPressDelay() const;
   void captureDelayedPress(QGraphicsSceneMouseEvent *event);
   void clearDelayedPress();

   void setRoundedViewportX(qreal x);
   void setRoundedViewportY(qreal y);

   qreal overShootDistance(qreal size);

   void itemGeometryChanged(QDeclarativeItem *, const QRectF &, const QRectF &);

 public:
   QDeclarativeItem *contentItem;

   AxisData hData;
   AxisData vData;

   QDeclarativeTimeLine timeline;
   bool hMoved : 1;
   bool vMoved : 1;
   bool stealMouse : 1;
   bool pressed : 1;
   bool interactive : 1;
   bool calcVelocity : 1;
   QElapsedTimer lastPosTime;
   QPointF lastPos;
   QPointF pressPos;
   QElapsedTimer pressTime;
   qreal deceleration;
   qreal maxVelocity;
   QElapsedTimer velocityTime;
   QPointF lastFlickablePosition;
   qreal reportedVelocitySmoothing;
   QGraphicsSceneMouseEvent *delayedPressEvent;
   QGraphicsItem *delayedPressTarget;
   QBasicTimer delayedPressTimer;
   int pressDelay;
   int fixupDuration;

   enum FixupMode { Normal, Immediate, ExtentChanged };
   FixupMode fixupMode;

   static void fixupY_callback(void *);
   static void fixupX_callback(void *);

   void updateVelocity();
   int vTime;
   QDeclarativeTimeLine velocityTimeline;
   QDeclarativeFlickableVisibleArea *visibleArea;
   QDeclarativeFlickable::FlickableDirection flickableDirection;
   QDeclarativeFlickable::BoundsBehavior boundsBehavior;

   void handleMousePressEvent(QGraphicsSceneMouseEvent *);
   void handleMouseMoveEvent(QGraphicsSceneMouseEvent *);
   void handleMouseReleaseEvent(QGraphicsSceneMouseEvent *);

   // flickableData property
   static void data_append(QDeclarativeListProperty<QObject> *, QObject *);
   static int data_count(QDeclarativeListProperty<QObject> *);
   static QObject *data_at(QDeclarativeListProperty<QObject> *, int);
   static void data_clear(QDeclarativeListProperty<QObject> *);
};

class QDeclarativeFlickableVisibleArea : public QObject
{
   DECL_CS_OBJECT(QDeclarativeFlickableVisibleArea)

   DECL_CS_PROPERTY_READ(xPosition, xPosition)
   DECL_CS_PROPERTY_NOTIFY(xPosition, xPositionChanged)
   DECL_CS_PROPERTY_READ(yPosition, yPosition)
   DECL_CS_PROPERTY_NOTIFY(yPosition, yPositionChanged)
   DECL_CS_PROPERTY_READ(widthRatio, widthRatio)
   DECL_CS_PROPERTY_NOTIFY(widthRatio, widthRatioChanged)
   DECL_CS_PROPERTY_READ(heightRatio, heightRatio)
   DECL_CS_PROPERTY_NOTIFY(heightRatio, heightRatioChanged)

 public:
   QDeclarativeFlickableVisibleArea(QDeclarativeFlickable *parent = 0);

   qreal xPosition() const;
   qreal widthRatio() const;
   qreal yPosition() const;
   qreal heightRatio() const;

   void updateVisible();

 public:
   DECL_CS_SIGNAL_1(Public, void xPositionChanged(qreal xPosition))
   DECL_CS_SIGNAL_2(xPositionChanged, xPosition)
   DECL_CS_SIGNAL_1(Public, void yPositionChanged(qreal yPosition))
   DECL_CS_SIGNAL_2(yPositionChanged, yPosition)
   DECL_CS_SIGNAL_1(Public, void widthRatioChanged(qreal widthRatio))
   DECL_CS_SIGNAL_2(widthRatioChanged, widthRatio)
   DECL_CS_SIGNAL_1(Public, void heightRatioChanged(qreal heightRatio))
   DECL_CS_SIGNAL_2(heightRatioChanged, heightRatio)

 private:
   QDeclarativeFlickable *flickable;
   qreal m_xPosition;
   qreal m_widthRatio;
   qreal m_yPosition;
   qreal m_heightRatio;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeFlickableVisibleArea)

#endif
