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

#ifndef QDECLARATIVEMOUSEAREA_P_H
#define QDECLARATIVEMOUSEAREA_P_H

#include <qdeclarativeitem.h>

QT_BEGIN_NAMESPACE

class QDeclarativeDrag : public QObject
{
   DECL_CS_OBJECT(QDeclarativeDrag)

   DECL_CS_ENUM(Axis)
   DECL_CS_PROPERTY_READ(*target, target)
   DECL_CS_PROPERTY_WRITE(*target, setTarget)
   DECL_CS_PROPERTY_NOTIFY(*target, targetChanged)
   DECL_CS_PROPERTY_RESET(*target, resetTarget)
   DECL_CS_PROPERTY_READ(axis, axis)
   DECL_CS_PROPERTY_WRITE(axis, setAxis)
   DECL_CS_PROPERTY_NOTIFY(axis, axisChanged)
   DECL_CS_PROPERTY_READ(minimumX, xmin)
   DECL_CS_PROPERTY_WRITE(minimumX, setXmin)
   DECL_CS_PROPERTY_NOTIFY(minimumX, minimumXChanged)
   DECL_CS_PROPERTY_READ(maximumX, xmax)
   DECL_CS_PROPERTY_WRITE(maximumX, setXmax)
   DECL_CS_PROPERTY_NOTIFY(maximumX, maximumXChanged)
   DECL_CS_PROPERTY_READ(minimumY, ymin)
   DECL_CS_PROPERTY_WRITE(minimumY, setYmin)
   DECL_CS_PROPERTY_NOTIFY(minimumY, minimumYChanged)
   DECL_CS_PROPERTY_READ(maximumY, ymax)
   DECL_CS_PROPERTY_WRITE(maximumY, setYmax)
   DECL_CS_PROPERTY_NOTIFY(maximumY, maximumYChanged)
   DECL_CS_PROPERTY_READ(active, active)
   DECL_CS_PROPERTY_NOTIFY(active, activeChanged)
   DECL_CS_PROPERTY_READ(filterChildren, filterChildren)
   DECL_CS_PROPERTY_WRITE(filterChildren, setFilterChildren)
   DECL_CS_PROPERTY_NOTIFY(filterChildren, filterChildrenChanged)
   //### consider drag and drop

 public:
   QDeclarativeDrag(QObject *parent = nullptr);
   ~QDeclarativeDrag();

   QGraphicsObject *target() const;
   void setTarget(QGraphicsObject *);
   void resetTarget();

   enum Axis { XAxis = 0x01, YAxis = 0x02, XandYAxis = 0x03 };
   Axis axis() const;
   void setAxis(Axis);

   qreal xmin() const;
   void setXmin(qreal);
   qreal xmax() const;
   void setXmax(qreal);
   qreal ymin() const;
   void setYmin(qreal);
   qreal ymax() const;
   void setYmax(qreal);

   bool active() const;
   void setActive(bool);

   bool filterChildren() const;
   void setFilterChildren(bool);

   DECL_CS_SIGNAL_1(Public, void targetChanged())
   DECL_CS_SIGNAL_2(targetChanged)
   DECL_CS_SIGNAL_1(Public, void axisChanged())
   DECL_CS_SIGNAL_2(axisChanged)
   DECL_CS_SIGNAL_1(Public, void minimumXChanged())
   DECL_CS_SIGNAL_2(minimumXChanged)
   DECL_CS_SIGNAL_1(Public, void maximumXChanged())
   DECL_CS_SIGNAL_2(maximumXChanged)
   DECL_CS_SIGNAL_1(Public, void minimumYChanged())
   DECL_CS_SIGNAL_2(minimumYChanged)
   DECL_CS_SIGNAL_1(Public, void maximumYChanged())
   DECL_CS_SIGNAL_2(maximumYChanged)
   DECL_CS_SIGNAL_1(Public, void activeChanged())
   DECL_CS_SIGNAL_2(activeChanged)
   DECL_CS_SIGNAL_1(Public, void filterChildrenChanged())
   DECL_CS_SIGNAL_2(filterChildrenChanged)

 private:
   QGraphicsObject *_target;
   Axis _axis;
   qreal _xmin;
   qreal _xmax;
   qreal _ymin;
   qreal _ymax;
   bool _active : 1;
   bool _filterChildren: 1;
   Q_DISABLE_COPY(QDeclarativeDrag)
};

class QDeclarativeMouseEvent;
class QDeclarativeMouseAreaPrivate;

class QDeclarativeMouseArea : public QDeclarativeItem
{
   DECL_CS_OBJECT(QDeclarativeMouseArea)

   DECL_CS_PROPERTY_READ(mouseX, mouseX)
   DECL_CS_PROPERTY_NOTIFY(mouseX, mousePositionChanged)
   DECL_CS_PROPERTY_READ(mouseY, mouseY)
   DECL_CS_PROPERTY_NOTIFY(mouseY, mousePositionChanged)

   DECL_CS_PROPERTY_READ(containsMouse, hovered)
   DECL_CS_PROPERTY_NOTIFY(containsMouse, hoveredChanged)

   DECL_CS_PROPERTY_READ(pressed, cs_pressed)
   DECL_CS_PROPERTY_NOTIFY(pressed, pressedChanged)

   // wrapper for overloaded method
   bool cs_pressed() const {
      return pressed();
   }

   DECL_CS_PROPERTY_READ(enabled, isEnabled)
   DECL_CS_PROPERTY_WRITE(enabled, setEnabled)
   DECL_CS_PROPERTY_NOTIFY(enabled, enabledChanged)

   DECL_CS_PROPERTY_READ(pressedButtons, pressedButtons)
   DECL_CS_PROPERTY_NOTIFY(pressedButtons, pressedChanged)

   DECL_CS_PROPERTY_READ(acceptedButtons, acceptedButtons)
   DECL_CS_PROPERTY_WRITE(acceptedButtons, setAcceptedButtons)
   DECL_CS_PROPERTY_NOTIFY(acceptedButtons, acceptedButtonsChanged)

   DECL_CS_PROPERTY_READ(hoverEnabled, hoverEnabled)
   DECL_CS_PROPERTY_WRITE(hoverEnabled, setHoverEnabled)
   DECL_CS_PROPERTY_NOTIFY(hoverEnabled, hoverEnabledChanged)

   DECL_CS_PROPERTY_READ(drag, drag)
   DECL_CS_PROPERTY_CONSTANT(drag)      //### add flicking to QDeclarativeDrag or add a QDeclarativeFlick ???

   DECL_CS_PROPERTY_READ(preventStealing, preventStealing)
   DECL_CS_PROPERTY_WRITE(preventStealing, setPreventStealing)
   DECL_CS_PROPERTY_NOTIFY(preventStealing, preventStealingChanged)
   DECL_CS_PROPERTY_REVISION(preventStealing, 1)

 public:
   QDeclarativeMouseArea(QDeclarativeItem *parent = 0);
   ~QDeclarativeMouseArea();

   qreal mouseX() const;
   qreal mouseY() const;

   bool isEnabled() const;
   void setEnabled(bool);

   bool hovered() const;
   bool pressed() const;

   Qt::MouseButtons pressedButtons() const;

   Qt::MouseButtons acceptedButtons() const;
   void setAcceptedButtons(Qt::MouseButtons buttons);

   bool hoverEnabled() const;
   void setHoverEnabled(bool h);

   QDeclarativeDrag *drag();

   bool preventStealing() const;
   void setPreventStealing(bool prevent);

   DECL_CS_SIGNAL_1(Public, void hoveredChanged())
   DECL_CS_SIGNAL_2(hoveredChanged)

   DECL_CS_SIGNAL_1(Public, void pressedChanged())
   DECL_CS_SIGNAL_2(pressedChanged)

   DECL_CS_SIGNAL_1(Public, void enabledChanged())
   DECL_CS_SIGNAL_2(enabledChanged)

   DECL_CS_SIGNAL_1(Public, void acceptedButtonsChanged())
   DECL_CS_SIGNAL_2(acceptedButtonsChanged)

   DECL_CS_SIGNAL_1(Public, void hoverEnabledChanged())
   DECL_CS_SIGNAL_2(hoverEnabledChanged)

   DECL_CS_SIGNAL_1(Public, void positionChanged(QDeclarativeMouseEvent *mouse))
   DECL_CS_SIGNAL_2(positionChanged, mouse)

   DECL_CS_SIGNAL_1(Public, void mousePositionChanged(QDeclarativeMouseEvent *mouse))
   DECL_CS_SIGNAL_2(mousePositionChanged, mouse)

   DECL_CS_SIGNAL_1(Public, void preventStealingChanged())
   DECL_CS_SIGNAL_2(preventStealingChanged)
   DECL_CS_REVISION(preventStealingChanged, 1)

   DECL_CS_SIGNAL_1(Public, void pressed(QDeclarativeMouseEvent *mouse))
   DECL_CS_SIGNAL_OVERLOAD(pressed, (QDeclarativeMouseEvent *), mouse)

   DECL_CS_SIGNAL_1(Public, void pressAndHold(QDeclarativeMouseEvent *mouse))
   DECL_CS_SIGNAL_2(pressAndHold, mouse)

   DECL_CS_SIGNAL_1(Public, void released(QDeclarativeMouseEvent *mouse))
   DECL_CS_SIGNAL_2(released, mouse)

   DECL_CS_SIGNAL_1(Public, void clicked(QDeclarativeMouseEvent *mouse))
   DECL_CS_SIGNAL_2(clicked, mouse)

   DECL_CS_SIGNAL_1(Public, void doubleClicked(QDeclarativeMouseEvent *mouse))
   DECL_CS_SIGNAL_2(doubleClicked, mouse)

   DECL_CS_SIGNAL_1(Public, void entered())
   DECL_CS_SIGNAL_2(entered)

   DECL_CS_SIGNAL_1(Public, void exited())
   DECL_CS_SIGNAL_2(exited)

   DECL_CS_SIGNAL_1(Public, void canceled())
   DECL_CS_SIGNAL_2(canceled)

 protected:
   void setHovered(bool);
   bool setPressed(bool);

   void mousePressEvent(QGraphicsSceneMouseEvent *event);
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
   void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
   void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
   void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
   void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
   void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
#endif

   bool sceneEvent(QEvent *);
   bool sendMouseEvent(QGraphicsSceneMouseEvent *event);
   bool sceneEventFilter(QGraphicsItem *i, QEvent *e);
   void timerEvent(QTimerEvent *event);

   virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
   virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

 private:
   void handlePress();
   void handleRelease();

 private:
   Q_DISABLE_COPY(QDeclarativeMouseArea)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeMouseArea)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeDrag)
QML_DECLARE_TYPE(QDeclarativeMouseArea)

#endif // QDECLARATIVEMOUSEAREA_H
