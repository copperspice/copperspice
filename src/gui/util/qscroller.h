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

#ifndef QSCROLLER_H
#define QSCROLLER_H

#include <qobject.h>
#include <qpointf.h>
#include <qscrollerproperties.h>

class QScrollerPrivate;
class QScrollerProperties;
class QWidget;

#ifndef QT_NO_GESTURES
class QFlickGestureRecognizer;
class QMouseFlickGestureRecognizer;
#endif

class Q_GUI_EXPORT QScroller : public QObject
{
   GUI_CS_OBJECT(QScroller)

   GUI_CS_ENUM(State)

   GUI_CS_PROPERTY_READ(state, state)
   GUI_CS_PROPERTY_NOTIFY(state, stateChanged)

   GUI_CS_PROPERTY_READ(scrollerProperties, scrollerProperties)
   GUI_CS_PROPERTY_WRITE(scrollerProperties, setScrollerProperties)
   GUI_CS_PROPERTY_NOTIFY(scrollerProperties, scrollerPropertiesChanged)

 public:
   enum State {
      Inactive,
      Pressed,
      Dragging,
      Scrolling
   };

   enum ScrollerGestureType {
      TouchGesture,
      LeftMouseButtonGesture,
      RightMouseButtonGesture,
      MiddleMouseButtonGesture
   };

   enum Input {
      InputPress = 1,
      InputMove,
      InputRelease
   };

   QScroller(const QScroller &) = delete;
   QScroller &operator=(const QScroller &) = delete;

   static bool hasScroller(QObject *target);

   static QScroller *scroller(QObject *target);
   static const QScroller *scroller(const QObject *target);

#ifndef QT_NO_GESTURES
   static Qt::GestureType grabGesture(QObject *target, ScrollerGestureType gestureType = TouchGesture);
   static Qt::GestureType grabbedGesture(QObject *target);
   static void ungrabGesture(QObject *target);
#endif

   static QList<QScroller *> activeScrollers();

   QObject *target() const;

   State state() const;

   bool handleInput(Input input, const QPointF &position, qint64 timestamp = 0);

   void stop();
   QPointF velocity() const;
   QPointF finalPosition() const;
   QPointF pixelPerMeter() const;

   QScrollerProperties scrollerProperties() const;

   void setSnapPositionsX( const QList<qreal> &positions );
   void setSnapPositionsX( qreal first, qreal interval );
   void setSnapPositionsY( const QList<qreal> &positions );
   void setSnapPositionsY( qreal first, qreal interval );

   GUI_CS_SLOT_1(Public, void setScrollerProperties(const QScrollerProperties &prop))
   GUI_CS_SLOT_2(setScrollerProperties)

   GUI_CS_SLOT_1(Public, void scrollTo(const QPointF &pos))
   GUI_CS_SLOT_OVERLOAD(scrollTo, (const QPointF &))

   GUI_CS_SLOT_1(Public, void scrollTo(const QPointF &pos, int scrollTime))
   GUI_CS_SLOT_OVERLOAD(scrollTo, (const QPointF &, int))

   GUI_CS_SLOT_1(Public, void ensureVisible(const QRectF &rect, qreal xmargin, qreal ymargin))
   GUI_CS_SLOT_OVERLOAD(ensureVisible, (const QRectF &, qreal, qreal))

   GUI_CS_SLOT_1(Public, void ensureVisible(const QRectF &rect, qreal xmargin, qreal ymargin, int scrollTime))
   GUI_CS_SLOT_OVERLOAD(ensureVisible, (const QRectF &, qreal, qreal, int))

   GUI_CS_SLOT_1(Public, void resendPrepareEvent())
   GUI_CS_SLOT_2(resendPrepareEvent)

   GUI_CS_SIGNAL_1(Public, void stateChanged(QScroller::State newState))
   GUI_CS_SIGNAL_2(stateChanged, newState)

   GUI_CS_SIGNAL_1(Public, void scrollerPropertiesChanged(const QScrollerProperties &newProperties))
   GUI_CS_SIGNAL_2(scrollerPropertiesChanged, newProperties)

 private:
   Q_DECLARE_PRIVATE(QScroller)

   QScroller(QObject *target);
   virtual ~QScroller();

   QScrollerPrivate *d_ptr;

#ifndef QT_NO_GESTURES
   friend class QFlickGestureRecognizer;
#endif
};

#endif
