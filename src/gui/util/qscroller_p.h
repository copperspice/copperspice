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

#ifndef QSCROLLER_P_H
#define QSCROLLER_P_H

#include <qscroller.h>

#include <qabstractanimation.h>
#include <qeasingcurve.h>
#include <qelapsedtimer.h>
#include <qobject.h>
#include <qpointer.h>
#include <qpointf.h>
#include <qqueue.h>
#include <qrectf.h>
#include <qscrollerproperties.h>
#include <qset.h>
#include <qsizef.h>

#include <qscrollerproperties_p.h>

#ifndef QT_NO_GESTURES
class QFlickGestureRecognizer;
#endif

#ifndef QT_NO_ANIMATION
class QScrollTimer;
#endif

class QScrollerPrivate : public QObject
{
   GUI_CS_OBJECT(QScrollerPrivate)
   Q_DECLARE_PUBLIC(QScroller)

 public:
   QScrollerPrivate(QScroller *q, QObject *target);
   void init();

   void sendEvent(QObject *object, QEvent *event);

   void setState(QScroller::State s);

   enum ScrollType {
      ScrollTypeFlick = 0,
      ScrollTypeScrollTo,
      ScrollTypeOvershoot
   };

   struct ScrollSegment {
      qint64 startTime;
      qint64 deltaTime;
      qreal startPos;
      qreal deltaPos;
      QEasingCurve curve;
      qreal stopProgress; // whatever is..
      qreal stopPos;      // ..reached first
      ScrollType type;
   };

   bool pressWhileInactive(const QPointF &position, qint64 timestamp);
   bool moveWhilePressed(const QPointF &position, qint64 timestamp);
   bool releaseWhilePressed(const QPointF &position, qint64 timestamp);
   bool moveWhileDragging(const QPointF &position, qint64 timestamp);
   bool releaseWhileDragging(const QPointF &position, qint64 timestamp);
   bool pressWhileScrolling(const QPointF &position, qint64 timestamp);

   void timerTick();
   void timerEventWhileDragging();
   void timerEventWhileScrolling();

   bool prepareScrolling(const QPointF &position);
   void handleDrag(const QPointF &position, qint64 timestamp);

   QPointF dpi() const;
   void setDpi(const QPointF &dpi);
   void setDpiFromWidget(QWidget *widget);

   void updateVelocity(const QPointF &deltaPixelRaw, qint64 deltaTime);
   void pushSegment(ScrollType type, qreal deltaTime, qreal stopProgress, qreal startPos, qreal deltaPos, qreal stopPos,
         QEasingCurve::Type curve, Qt::Orientation orientation);
   void recalcScrollingSegments(bool forceRecalc = false);
   qreal scrollingSegmentsEndPos(Qt::Orientation orientation) const;
   bool scrollingSegmentsValid(Qt::Orientation orientation);
   void createScrollToSegments(qreal v, qreal deltaTime, qreal endPos, Qt::Orientation orientation, ScrollType type);

   void createScrollingSegments(qreal v, qreal startPos,
         qreal deltaTime, qreal deltaPos, Qt::Orientation orientation);
   void createScrollingSegments(const QPointF &v, const QPointF &startPos, const QPointF &ppm);

   void setContentPositionHelperDragging(const QPointF &deltaPos);
   void setContentPositionHelperScrolling();

   qreal nextSnapPos(qreal p, int dir, Qt::Orientation orientation);
   static qreal nextSegmentPosition(QQueue<ScrollSegment> &segments, qint64 now, qreal oldPos);

   int frameRateSkip() const {
      return properties.d.data()->frameRate;
   }

   static const char *stateName(QScroller::State state);
   static const char *inputName(QScroller::Input input);

   GUI_CS_SLOT_1(Public, void targetDestroyed())
   GUI_CS_SLOT_2(targetDestroyed)

   // non static
   QObject *target;
   QScrollerProperties properties;

#ifndef QT_NO_GESTURES
   QFlickGestureRecognizer *recognizer;
   Qt::GestureType recognizerType;
#endif

   // scroller state:

   // QPointer<QObject> scrollTarget;
   QSizeF viewportSize;
   QRectF contentPosRange;
   QPointF contentPosition;
   QPointF overshootPosition; // the number of pixels we are overshooting (before overshootDragResistanceFactor)

   // state
   bool enabled;
   QScroller::State state;
   bool firstScroll;          // true if we haven't already send a scroll event

   QPointF oldVelocity;       // the release velocity of the last drag

   QPointF pressPosition;
   QPointF lastPosition;
   qint64  pressTimestamp;
   qint64  lastTimestamp;

   QPointF dragDistance;      // the distance we should move during the next drag timer event

   QQueue<ScrollSegment> xSegments;
   QQueue<ScrollSegment> ySegments;

   // snap positions
   QList<qreal> snapPositionsX;
   qreal snapFirstX;
   qreal snapIntervalX;
   QList<qreal> snapPositionsY;
   qreal snapFirstY;
   qreal snapIntervalY;

   QPointF pixelPerMeter;

   QElapsedTimer monotonicTimer;

   QPointF releaseVelocity;   // the starting velocity of the scrolling state

#ifndef QT_NO_ANIMATION
   QScrollTimer *scrollTimer;
#endif

   QScroller *q_ptr;
};

#endif
