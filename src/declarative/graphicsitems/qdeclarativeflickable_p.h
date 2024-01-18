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

#ifndef QDECLARATIVEFLICKABLE_P_H
#define QDECLARATIVEFLICKABLE_P_H

#include <qdeclarativeitem.h>

QT_BEGIN_NAMESPACE

class QDeclarativeFlickablePrivate;
class QDeclarativeFlickableVisibleArea;

class QDeclarativeFlickable : public QDeclarativeItem
{
   DECL_CS_OBJECT(QDeclarativeFlickable)

   DECL_CS_PROPERTY_READ(contentWidth, contentWidth)
   DECL_CS_PROPERTY_WRITE(contentWidth, setContentWidth)
   DECL_CS_PROPERTY_NOTIFY(contentWidth, contentWidthChanged)
   DECL_CS_PROPERTY_READ(contentHeight, contentHeight)
   DECL_CS_PROPERTY_WRITE(contentHeight, setContentHeight)
   DECL_CS_PROPERTY_NOTIFY(contentHeight, contentHeightChanged)
   DECL_CS_PROPERTY_READ(contentX, contentX)
   DECL_CS_PROPERTY_WRITE(contentX, setContentX)
   DECL_CS_PROPERTY_NOTIFY(contentX, contentXChanged)
   DECL_CS_PROPERTY_READ(contentY, contentY)
   DECL_CS_PROPERTY_WRITE(contentY, setContentY)
   DECL_CS_PROPERTY_NOTIFY(contentY, contentYChanged)
   DECL_CS_PROPERTY_READ(*contentItem, contentItem)
   DECL_CS_PROPERTY_CONSTANT(*contentItem)

   DECL_CS_PROPERTY_READ(horizontalVelocity, horizontalVelocity)
   DECL_CS_PROPERTY_NOTIFY(horizontalVelocity, horizontalVelocityChanged)
   DECL_CS_PROPERTY_READ(verticalVelocity, verticalVelocity)
   DECL_CS_PROPERTY_NOTIFY(verticalVelocity, verticalVelocityChanged)

   DECL_CS_PROPERTY_READ(boundsBehavior, boundsBehavior)
   DECL_CS_PROPERTY_WRITE(boundsBehavior, setBoundsBehavior)
   DECL_CS_PROPERTY_NOTIFY(boundsBehavior, boundsBehaviorChanged)
   DECL_CS_PROPERTY_READ(maximumFlickVelocity, maximumFlickVelocity)
   DECL_CS_PROPERTY_WRITE(maximumFlickVelocity, setMaximumFlickVelocity)
   DECL_CS_PROPERTY_NOTIFY(maximumFlickVelocity, maximumFlickVelocityChanged)
   DECL_CS_PROPERTY_READ(flickDeceleration, flickDeceleration)
   DECL_CS_PROPERTY_WRITE(flickDeceleration, setFlickDeceleration)
   DECL_CS_PROPERTY_NOTIFY(flickDeceleration, flickDecelerationChanged)
   DECL_CS_PROPERTY_READ(moving, isMoving)
   DECL_CS_PROPERTY_NOTIFY(moving, movingChanged)
   DECL_CS_PROPERTY_READ(movingHorizontally, isMovingHorizontally)
   DECL_CS_PROPERTY_NOTIFY(movingHorizontally, movingHorizontallyChanged)
   DECL_CS_PROPERTY_READ(movingVertically, isMovingVertically)
   DECL_CS_PROPERTY_NOTIFY(movingVertically, movingVerticallyChanged)
   DECL_CS_PROPERTY_READ(flicking, isFlicking)
   DECL_CS_PROPERTY_NOTIFY(flicking, flickingChanged)
   DECL_CS_PROPERTY_READ(flickingHorizontally, isFlickingHorizontally)
   DECL_CS_PROPERTY_NOTIFY(flickingHorizontally, flickingHorizontallyChanged)
   DECL_CS_PROPERTY_READ(flickingVertically, isFlickingVertically)
   DECL_CS_PROPERTY_NOTIFY(flickingVertically, flickingVerticallyChanged)
   DECL_CS_PROPERTY_READ(flickableDirection, flickableDirection)
   DECL_CS_PROPERTY_WRITE(flickableDirection, setFlickableDirection)
   DECL_CS_PROPERTY_NOTIFY(flickableDirection, flickableDirectionChanged)

   DECL_CS_PROPERTY_READ(interactive, isInteractive)
   DECL_CS_PROPERTY_WRITE(interactive, setInteractive)
   DECL_CS_PROPERTY_NOTIFY(interactive, interactiveChanged)
   DECL_CS_PROPERTY_READ(pressDelay, pressDelay)
   DECL_CS_PROPERTY_WRITE(pressDelay, setPressDelay)
   DECL_CS_PROPERTY_NOTIFY(pressDelay, pressDelayChanged)

   DECL_CS_PROPERTY_READ(atXEnd, isAtXEnd)
   DECL_CS_PROPERTY_NOTIFY(atXEnd, isAtBoundaryChanged)
   DECL_CS_PROPERTY_READ(atYEnd, isAtYEnd)
   DECL_CS_PROPERTY_NOTIFY(atYEnd, isAtBoundaryChanged)
   DECL_CS_PROPERTY_READ(atXBeginning, isAtXBeginning)
   DECL_CS_PROPERTY_NOTIFY(atXBeginning, isAtBoundaryChanged)
   DECL_CS_PROPERTY_READ(atYBeginning, isAtYBeginning)
   DECL_CS_PROPERTY_NOTIFY(atYBeginning, isAtBoundaryChanged)

   DECL_CS_PROPERTY_READ(*visibleArea, visibleArea)
   DECL_CS_PROPERTY_CONSTANT(*visibleArea)

   DECL_CS_PROPERTY_READ(flickableData, flickableData)
   DECL_CS_PROPERTY_READ(flickableChildren, flickableChildren)

   DECL_CS_CLASSINFO("DefaultProperty", "flickableData")

   DECL_CS_ENUM(FlickableDirection)
   DECL_CS_ENUM(BoundsBehavior)

 public:
   QDeclarativeFlickable(QDeclarativeItem *parent = 0);
   ~QDeclarativeFlickable();

   QDeclarativeListProperty<QObject> flickableData();
   QDeclarativeListProperty<QGraphicsObject> flickableChildren();

   enum BoundsBehavior { StopAtBounds, DragOverBounds, DragAndOvershootBounds };
   BoundsBehavior boundsBehavior() const;
   void setBoundsBehavior(BoundsBehavior);

   qreal contentWidth() const;
   void setContentWidth(qreal);

   qreal contentHeight() const;
   void setContentHeight(qreal);

   qreal contentX() const;
   virtual void setContentX(qreal pos);

   qreal contentY() const;
   virtual void setContentY(qreal pos);

   bool isMoving() const;
   bool isMovingHorizontally() const;
   bool isMovingVertically() const;
   bool isFlicking() const;
   bool isFlickingHorizontally() const;
   bool isFlickingVertically() const;

   int pressDelay() const;
   void setPressDelay(int delay);

   qreal maximumFlickVelocity() const;
   void setMaximumFlickVelocity(qreal);

   qreal flickDeceleration() const;
   void setFlickDeceleration(qreal);

   bool isInteractive() const;
   void setInteractive(bool);

   qreal horizontalVelocity() const;
   qreal verticalVelocity() const;

   bool isAtXEnd() const;
   bool isAtXBeginning() const;
   bool isAtYEnd() const;
   bool isAtYBeginning() const;

   QDeclarativeItem *contentItem();

   enum FlickableDirection { AutoFlickDirection = 0x00, HorizontalFlick = 0x01, VerticalFlick = 0x02, HorizontalAndVerticalFlick = 0x03 };
   FlickableDirection flickableDirection() const;
   void setFlickableDirection(FlickableDirection);

   DECL_CS_INVOKABLE_METHOD_1(Public, void resizeContent(qreal w, qreal h, QPointF center))
   DECL_CS_INVOKABLE_METHOD_2(resizeContent)
   DECL_CS_REVISION(resizeContent, 1)

   DECL_CS_INVOKABLE_METHOD_1(Public, void returnToBounds())
   DECL_CS_INVOKABLE_METHOD_2(returnToBounds)
   DECL_CS_REVISION(returnToBounds, 1)

   DECL_CS_SIGNAL_1(Public, void contentWidthChanged())
   DECL_CS_SIGNAL_2(contentWidthChanged)
   DECL_CS_SIGNAL_1(Public, void contentHeightChanged())
   DECL_CS_SIGNAL_2(contentHeightChanged)
   DECL_CS_SIGNAL_1(Public, void contentXChanged())
   DECL_CS_SIGNAL_2(contentXChanged)
   DECL_CS_SIGNAL_1(Public, void contentYChanged())
   DECL_CS_SIGNAL_2(contentYChanged)
   DECL_CS_SIGNAL_1(Public, void movingChanged())
   DECL_CS_SIGNAL_2(movingChanged)
   DECL_CS_SIGNAL_1(Public, void movingHorizontallyChanged())
   DECL_CS_SIGNAL_2(movingHorizontallyChanged)
   DECL_CS_SIGNAL_1(Public, void movingVerticallyChanged())
   DECL_CS_SIGNAL_2(movingVerticallyChanged)
   DECL_CS_SIGNAL_1(Public, void flickingChanged())
   DECL_CS_SIGNAL_2(flickingChanged)
   DECL_CS_SIGNAL_1(Public, void flickingHorizontallyChanged())
   DECL_CS_SIGNAL_2(flickingHorizontallyChanged)
   DECL_CS_SIGNAL_1(Public, void flickingVerticallyChanged())
   DECL_CS_SIGNAL_2(flickingVerticallyChanged)
   DECL_CS_SIGNAL_1(Public, void horizontalVelocityChanged())
   DECL_CS_SIGNAL_2(horizontalVelocityChanged)
   DECL_CS_SIGNAL_1(Public, void verticalVelocityChanged())
   DECL_CS_SIGNAL_2(verticalVelocityChanged)
   DECL_CS_SIGNAL_1(Public, void isAtBoundaryChanged())
   DECL_CS_SIGNAL_2(isAtBoundaryChanged)
   DECL_CS_SIGNAL_1(Public, void flickableDirectionChanged())
   DECL_CS_SIGNAL_2(flickableDirectionChanged)
   DECL_CS_SIGNAL_1(Public, void interactiveChanged())
   DECL_CS_SIGNAL_2(interactiveChanged)
   DECL_CS_SIGNAL_1(Public, void boundsBehaviorChanged())
   DECL_CS_SIGNAL_2(boundsBehaviorChanged)
   DECL_CS_SIGNAL_1(Public, void maximumFlickVelocityChanged())
   DECL_CS_SIGNAL_2(maximumFlickVelocityChanged)
   DECL_CS_SIGNAL_1(Public, void flickDecelerationChanged())
   DECL_CS_SIGNAL_2(flickDecelerationChanged)
   DECL_CS_SIGNAL_1(Public, void pressDelayChanged())
   DECL_CS_SIGNAL_2(pressDelayChanged)
   DECL_CS_SIGNAL_1(Public, void movementStarted())
   DECL_CS_SIGNAL_2(movementStarted)
   DECL_CS_SIGNAL_1(Public, void movementEnded())
   DECL_CS_SIGNAL_2(movementEnded)
   DECL_CS_SIGNAL_1(Public, void flickStarted())
   DECL_CS_SIGNAL_2(flickStarted)
   DECL_CS_SIGNAL_1(Public, void flickEnded())
   DECL_CS_SIGNAL_2(flickEnded)

 protected:
   virtual bool sceneEventFilter(QGraphicsItem *, QEvent *);
   void mousePressEvent(QGraphicsSceneMouseEvent *event);
   void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
   void wheelEvent(QGraphicsSceneWheelEvent *event);
   void timerEvent(QTimerEvent *event);

   QDeclarativeFlickableVisibleArea *visibleArea();

   DECL_CS_SLOT_1(Protected, virtual void ticked())
   DECL_CS_SLOT_2(ticked)
   DECL_CS_SLOT_1(Protected, void movementStarting())
   DECL_CS_SLOT_2(movementStarting)
   DECL_CS_SLOT_1(Protected, void movementEnding())
   DECL_CS_SLOT_2(movementEnding)

   void movementXEnding();
   void movementYEnding();
   virtual qreal minXExtent() const;
   virtual qreal minYExtent() const;
   virtual qreal maxXExtent() const;
   virtual qreal maxYExtent() const;
   qreal vWidth() const;
   qreal vHeight() const;
   virtual void viewportMoved();
   virtual void geometryChanged(const QRectF &newGeometry,
                                const QRectF &oldGeometry);
   bool sceneEvent(QEvent *event);
   bool sendMouseEvent(QGraphicsSceneMouseEvent *event);

   bool xflick() const;
   bool yflick() const;
   void cancelFlick();

   QDeclarativeFlickable(QDeclarativeFlickablePrivate &dd, QDeclarativeItem *parent);

 private:
   Q_DISABLE_COPY(QDeclarativeFlickable)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeFlickable)
   friend class QDeclarativeFlickableVisibleArea;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeFlickable)

#endif
