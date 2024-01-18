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

#include <qdeclarativemousearea_p.h>
#include <qdeclarativemousearea_p_p.h>
#include <qdeclarativeevents_p_p.h>
#include <QGraphicsSceneMouseEvent>
#include <float.h>

QT_BEGIN_NAMESPACE
static const int PressAndHoldDelay = 800;

QDeclarativeDrag::QDeclarativeDrag(QObject *parent)
   : QObject(parent), _target(0), _axis(XandYAxis), _xmin(-FLT_MAX), _xmax(FLT_MAX), _ymin(-FLT_MAX), _ymax(FLT_MAX),
     _active(false), _filterChildren(false)
{
}

QDeclarativeDrag::~QDeclarativeDrag()
{
}

QGraphicsObject *QDeclarativeDrag::target() const
{
   return _target;
}

void QDeclarativeDrag::setTarget(QGraphicsObject *t)
{
   if (_target == t) {
      return;
   }
   _target = t;
   emit targetChanged();
}

void QDeclarativeDrag::resetTarget()
{
   if (!_target) {
      return;
   }
   _target = 0;
   emit targetChanged();
}

QDeclarativeDrag::Axis QDeclarativeDrag::axis() const
{
   return _axis;
}

void QDeclarativeDrag::setAxis(QDeclarativeDrag::Axis a)
{
   if (_axis == a) {
      return;
   }
   _axis = a;
   emit axisChanged();
}

qreal QDeclarativeDrag::xmin() const
{
   return _xmin;
}

void QDeclarativeDrag::setXmin(qreal m)
{
   if (_xmin == m) {
      return;
   }
   _xmin = m;
   emit minimumXChanged();
}

qreal QDeclarativeDrag::xmax() const
{
   return _xmax;
}

void QDeclarativeDrag::setXmax(qreal m)
{
   if (_xmax == m) {
      return;
   }
   _xmax = m;
   emit maximumXChanged();
}

qreal QDeclarativeDrag::ymin() const
{
   return _ymin;
}

void QDeclarativeDrag::setYmin(qreal m)
{
   if (_ymin == m) {
      return;
   }
   _ymin = m;
   emit minimumYChanged();
}

qreal QDeclarativeDrag::ymax() const
{
   return _ymax;
}

void QDeclarativeDrag::setYmax(qreal m)
{
   if (_ymax == m) {
      return;
   }
   _ymax = m;
   emit maximumYChanged();
}

bool QDeclarativeDrag::active() const
{
   return _active;
}

void QDeclarativeDrag::setActive(bool drag)
{
   if (_active == drag) {
      return;
   }
   _active = drag;
   emit activeChanged();
}

bool QDeclarativeDrag::filterChildren() const
{
   return _filterChildren;
}

void QDeclarativeDrag::setFilterChildren(bool filter)
{
   if (_filterChildren == filter) {
      return;
   }
   _filterChildren = filter;
   emit filterChildrenChanged();
}

QDeclarativeMouseAreaPrivate::~QDeclarativeMouseAreaPrivate()
{
   delete drag;
}

/*!
    \qmlclass MouseArea QDeclarativeMouseArea
    \ingroup qml-basic-interaction-elements
    \since 4.7
    \brief The MouseArea item enables simple mouse handling.
    \inherits Item

    A MouseArea is an invisible item that is typically used in conjunction with
    a visible item in order to provide mouse handling for that item.
    By effectively acting as a proxy, the logic for mouse handling can be
    contained within a MouseArea item.

    For basic key handling, see the \l{Keys}{Keys attached property}.

    The \l enabled property is used to enable and disable mouse handling for
    the proxied item. When disabled, the mouse area becomes transparent to
    mouse events.

    The \l pressed read-only property indicates whether or not the user is
    holding down a mouse button over the mouse area. This property is often
    used in bindings between properties in a user interface. The containsMouse
    read-only property indicates the presence of the mouse cursor over the
    mouse area but, by default, only when a mouse button is held down; see below
    for further details.

    Information about the mouse position and button clicks are provided via
    signals for which event handler properties are defined. The most commonly
    used involved handling mouse presses and clicks: onClicked, onDoubleClicked,
    onPressed, onReleased and onPressAndHold.

    By default, MouseArea items only report mouse clicks and not changes to the
    position of the mouse cursor. Setting the hoverEnabled property ensures that
    handlers defined for onPositionChanged, onEntered and onExited are used and
    that the containsMouse property is updated even when no mouse buttons are
    pressed.

    \section1 Example Usage

    \div {class="float-right"}
    \inlineimage qml-mousearea-snippet.png
    \enddiv

    The following example uses a MouseArea in a \l Rectangle that changes
    the \l Rectangle color to red when clicked:

    \snippet doc/src/snippets/declarative/mousearea/mousearea.qml import
    \codeline
    \snippet doc/src/snippets/declarative/mousearea/mousearea.qml intro

    \clearfloat
    Many MouseArea signals pass a \l{MouseEvent}{mouse} parameter that contains
    additional information about the mouse event, such as the position, button,
    and any key modifiers.

    Here is an extension of the previous example that produces a different
    color when the area is right clicked:

    \snippet doc/src/snippets/declarative/mousearea/mousearea.qml intro-extended

    \sa MouseEvent, {declarative/touchinteraction/mousearea}{MouseArea example}
*/

/*!
    \qmlsignal MouseArea::onEntered()

    This handler is called when the mouse enters the mouse area.

    By default the onEntered handler is only called while a button is
    pressed. Setting hoverEnabled to true enables handling of
    onEntered when no mouse button is pressed.

    \sa hoverEnabled
*/

/*!
    \qmlsignal MouseArea::onExited()

    This handler is called when the mouse exists the mouse area.

    By default the onExited handler is only called while a button is
    pressed. Setting hoverEnabled to true enables handling of
    onExited when no mouse button is pressed.

    \sa hoverEnabled
*/

/*!
    \qmlsignal MouseArea::onPositionChanged(MouseEvent mouse)

    This handler is called when the mouse position changes.

    The \l {MouseEvent}{mouse} parameter provides information about the mouse, including the x and y
    position, and any buttons currently pressed.

    The \e accepted property of the MouseEvent parameter is ignored in this handler.

    By default the onPositionChanged handler is only called while a button is
    pressed.  Setting hoverEnabled to true enables handling of
    onPositionChanged when no mouse button is pressed.
*/

/*!
    \qmlsignal MouseArea::onClicked(MouseEvent mouse)

    This handler is called when there is a click. A click is defined as a press followed by a release,
    both inside the MouseArea (pressing, moving outside the MouseArea, and then moving back inside and
    releasing is also considered a click).

    The \l {MouseEvent}{mouse} parameter provides information about the click, including the x and y
    position of the release of the click, and whether the click was held.

    The \e accepted property of the MouseEvent parameter is ignored in this handler.
*/

/*!
    \qmlsignal MouseArea::onPressed(MouseEvent mouse)

    This handler is called when there is a press.
    The \l {MouseEvent}{mouse} parameter provides information about the press, including the x and y
    position and which button was pressed.

    The \e accepted property of the MouseEvent parameter determines whether this MouseArea
    will handle the press and all future mouse events until release.  The default is to accept
    the event and not allow other MouseArea beneath this one to handle the event.  If \e accepted
    is set to false, no further events will be sent to this MouseArea until the button is next
    pressed.
*/

/*!
    \qmlsignal MouseArea::onReleased(MouseEvent mouse)

    This handler is called when there is a release.
    The \l {MouseEvent}{mouse} parameter provides information about the click, including the x and y
    position of the release of the click, and whether the click was held.

    The \e accepted property of the MouseEvent parameter is ignored in this handler.

    \sa onCanceled
*/

/*!
    \qmlsignal MouseArea::onPressAndHold(MouseEvent mouse)

    This handler is called when there is a long press (currently 800ms).
    The \l {MouseEvent}{mouse} parameter provides information about the press, including the x and y
    position of the press, and which button is pressed.

    The \e accepted property of the MouseEvent parameter is ignored in this handler.
*/

/*!
    \qmlsignal MouseArea::onDoubleClicked(MouseEvent mouse)

    This handler is called when there is a double-click (a press followed by a release followed by a press).
    The \l {MouseEvent}{mouse} parameter provides information about the click, including the x and y
    position of the release of the click, and whether the click was held.

    If the \e accepted property of the \l {MouseEvent}{mouse} parameter is set to false
    in the handler, the onPressed/onReleased/onClicked handlers will be called for the second
    click; otherwise they are suppressed.  The accepted property defaults to true.
*/

/*!
    \qmlsignal MouseArea::onCanceled()

    This handler is called when mouse events have been canceled, either because an event was not accepted, or
    because another element stole the mouse event handling.

    This signal is for advanced use: it is useful when there is more than one MouseArea
    that is handling input, or when there is a MouseArea inside a \l Flickable. In the latter
    case, if you execute some logic on the pressed signal and then start dragging, the
    \l Flickable will steal the mouse handling from the MouseArea. In these cases, to reset
    the logic when the MouseArea has lost the mouse handling to the \l Flickable,
    \c onCanceled should be used in addition to onReleased.
*/

QDeclarativeMouseArea::QDeclarativeMouseArea(QDeclarativeItem *parent)
   : QDeclarativeItem(*(new QDeclarativeMouseAreaPrivate), parent)
{
   Q_D(QDeclarativeMouseArea);
   d->init();
}

QDeclarativeMouseArea::~QDeclarativeMouseArea()
{
}

/*!
    \qmlproperty real MouseArea::mouseX
    \qmlproperty real MouseArea::mouseY
    These properties hold the coordinates of the mouse cursor.

    If the hoverEnabled property is false then these properties will only be valid
    while a button is pressed, and will remain valid as long as the button is held
    down even if the mouse is moved outside the area.

    By default, this property is false.

    If hoverEnabled is true then these properties will be valid when:
    \list
        \i no button is pressed, but the mouse is within the MouseArea (containsMouse is true).
        \i a button is pressed and held, even if it has since moved out of the area.
    \endlist

    The coordinates are relative to the MouseArea.
*/
qreal QDeclarativeMouseArea::mouseX() const
{
   Q_D(const QDeclarativeMouseArea);
   return d->lastPos.x();
}

qreal QDeclarativeMouseArea::mouseY() const
{
   Q_D(const QDeclarativeMouseArea);
   return d->lastPos.y();
}

/*!
    \qmlproperty bool MouseArea::enabled
    This property holds whether the item accepts mouse events.

    By default, this property is true.
*/
bool QDeclarativeMouseArea::isEnabled() const
{
   Q_D(const QDeclarativeMouseArea);
   return d->absorb;
}

void QDeclarativeMouseArea::setEnabled(bool a)
{
   Q_D(QDeclarativeMouseArea);
   if (a != d->absorb) {
      d->absorb = a;
      emit enabledChanged();
   }
}

/*!
    \qmlproperty bool MouseArea::preventStealing
    \since QtQuick 1.1
    This property holds whether the mouse events may be stolen from this
    MouseArea.

    If a MouseArea is placed within an item that filters child mouse
    events, such as Flickable, the mouse
    events may be stolen from the MouseArea if a gesture is recognized
    by the parent element, e.g. a flick gesture.  If preventStealing is
    set to true, no element will steal the mouse events.

    Note that setting preventStealing to true once an element has started
    stealing events will have no effect until the next press event.

    By default this property is false.
*/
bool QDeclarativeMouseArea::preventStealing() const
{
   Q_D(const QDeclarativeMouseArea);
   return d->preventStealing;
}

void QDeclarativeMouseArea::setPreventStealing(bool prevent)
{
   Q_D(QDeclarativeMouseArea);
   if (prevent != d->preventStealing) {
      d->preventStealing = prevent;
      setKeepMouseGrab(d->preventStealing && d->absorb);
      emit preventStealingChanged();
   }
}

/*!
    \qmlproperty MouseButtons MouseArea::pressedButtons
    This property holds the mouse buttons currently pressed.

    It contains a bitwise combination of:
    \list
    \o Qt.LeftButton
    \o Qt.RightButton
    \o Qt.MiddleButton
    \endlist

    The code below displays "right" when the right mouse buttons is pressed:

    \snippet doc/src/snippets/declarative/mousearea/mousearea.qml mousebuttons

    \sa acceptedButtons
*/
Qt::MouseButtons QDeclarativeMouseArea::pressedButtons() const
{
   Q_D(const QDeclarativeMouseArea);
   return d->lastButtons;
}

void QDeclarativeMouseArea::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativeMouseArea);
   d->moved = false;
   d->stealMouse = d->preventStealing;
   if (!d->absorb) {
      QDeclarativeItem::mousePressEvent(event);
   } else {
      d->longPress = false;
      d->saveEvent(event);
      if (d->drag) {
         d->drag->setActive(false);
      }
      setHovered(true);
      d->startScene = event->scenePos();
      // we should only start timer if pressAndHold is connected to.
      if (d->isPressAndHoldConnected()) {
         d->pressAndHoldTimer.start(PressAndHoldDelay, this);
      }
      setKeepMouseGrab(d->stealMouse);
      event->setAccepted(setPressed(true));
   }
}

void QDeclarativeMouseArea::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativeMouseArea);
   if (!d->absorb) {
      QDeclarativeItem::mouseMoveEvent(event);
      return;
   }

   d->saveEvent(event);

   // ### we should skip this if these signals aren't used
   // ### can GV handle this for us?
   bool contains = boundingRect().contains(d->lastPos);
   if (d->hovered && !contains) {
      setHovered(false);
   } else if (!d->hovered && contains) {
      setHovered(true);
   }

   if (d->drag && d->drag->target()) {
      if (!d->moved) {
         d->startX = drag()->target()->x();
         d->startY = drag()->target()->y();
      }

      QPointF startLocalPos;
      QPointF curLocalPos;
      if (drag()->target()->parentItem()) {
         startLocalPos = drag()->target()->parentItem()->mapFromScene(d->startScene);
         curLocalPos = drag()->target()->parentItem()->mapFromScene(event->scenePos());
      } else {
         startLocalPos = d->startScene;
         curLocalPos = event->scenePos();
      }

      if (keepMouseGrab() && d->stealMouse) {
         d->drag->setActive(true);
      }

      bool dragX = drag()->axis() & QDeclarativeDrag::XAxis;
      bool dragY = drag()->axis() & QDeclarativeDrag::YAxis;

      const qreal x = dragX
                      ? qBound(d->drag->xmin(), d->startX + curLocalPos.x() - startLocalPos.x(), d->drag->xmax())
                      : d->startX;
      const qreal y = dragY
                      ? qBound(d->drag->ymin(), d->startY + curLocalPos.y() - startLocalPos.y(), d->drag->ymax())
                      : d->startY;

      if (d->drag->active()) {
         if (dragX && dragY) {
            d->drag->target()->setPos(x, y);
         } else if (dragX) {
            d->drag->target()->setX(x);
         } else if (dragY) {
            d->drag->target()->setY(y);
         }
      }

      if (!keepMouseGrab()) {
         const int dragThreshold = QApplication::startDragDistance();

         if (qAbs(x - d->startX) > dragThreshold || qAbs(y - d->startY) > dragThreshold) {
            setKeepMouseGrab(true);
            d->stealMouse = true;
         }
      }

      d->moved = true;
   }
   QDeclarativeMouseEvent me(d->lastPos.x(), d->lastPos.y(), d->lastButton, d->lastButtons, d->lastModifiers, false,
                             d->longPress);
   emit mousePositionChanged(&me);
   me.setX(d->lastPos.x());
   me.setY(d->lastPos.y());
   emit positionChanged(&me);
}


void QDeclarativeMouseArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativeMouseArea);
   d->stealMouse = false;
   if (!d->absorb) {
      QDeclarativeItem::mouseReleaseEvent(event);
   } else {
      d->saveEvent(event);
      setPressed(false);
      if (d->drag) {
         d->drag->setActive(false);
      }
      // If we don't accept hover, we need to reset containsMouse.
      if (!acceptHoverEvents()) {
         setHovered(false);
      }
      QGraphicsScene *s = scene();
      if (s && s->mouseGrabberItem() == this) {
         ungrabMouse();
      }
      setKeepMouseGrab(false);
   }
   d->doubleClick = false;
}

void QDeclarativeMouseArea::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativeMouseArea);
   if (!d->absorb) {
      QDeclarativeItem::mouseDoubleClickEvent(event);
   } else {
      if (d->isDoubleClickConnected()) {
         d->doubleClick = true;
      }
      d->saveEvent(event);
      QDeclarativeMouseEvent me(d->lastPos.x(), d->lastPos.y(), d->lastButton, d->lastButtons, d->lastModifiers, true, false);
      me.setAccepted(d->isDoubleClickConnected());
      emit this->doubleClicked(&me);
      QDeclarativeItem::mouseDoubleClickEvent(event);
   }
}

void QDeclarativeMouseArea::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
   Q_D(QDeclarativeMouseArea);
   if (!d->absorb) {
      QDeclarativeItem::hoverEnterEvent(event);
   } else {
      d->lastPos = event->pos();
      setHovered(true);
      QDeclarativeMouseEvent me(d->lastPos.x(), d->lastPos.y(), Qt::NoButton, Qt::NoButton, event->modifiers(), false, false);
      emit mousePositionChanged(&me);
   }
}

void QDeclarativeMouseArea::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
   Q_D(QDeclarativeMouseArea);
   if (!d->absorb) {
      QDeclarativeItem::hoverMoveEvent(event);
   } else {
      d->lastPos = event->pos();
      QDeclarativeMouseEvent me(d->lastPos.x(), d->lastPos.y(), Qt::NoButton, Qt::NoButton, event->modifiers(), false, false);
      emit mousePositionChanged(&me);
      me.setX(d->lastPos.x());
      me.setY(d->lastPos.y());
      emit positionChanged(&me);
   }
}

void QDeclarativeMouseArea::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
   Q_D(QDeclarativeMouseArea);
   if (!d->absorb) {
      QDeclarativeItem::hoverLeaveEvent(event);
   } else {
      setHovered(false);
   }
}

#ifndef QT_NO_CONTEXTMENU
void QDeclarativeMouseArea::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
   bool acceptsContextMenuButton;

   acceptsContextMenuButton = acceptedButtons() & Qt::RightButton;

   if (isEnabled() && event->reason() == QGraphicsSceneContextMenuEvent::Mouse
         && acceptsContextMenuButton) {
      // Do not let the context menu event propagate to items behind.
      return;
   }

   QDeclarativeItem::contextMenuEvent(event);
}
#endif // QT_NO_CONTEXTMENU

bool QDeclarativeMouseArea::sceneEvent(QEvent *event)
{
   bool rv = QDeclarativeItem::sceneEvent(event);
   if (event->type() == QEvent::UngrabMouse) {
      Q_D(QDeclarativeMouseArea);
      if (d->pressed) {
         // if our mouse grab has been removed (probably by Flickable), fix our
         // state
         d->pressed = false;
         d->stealMouse = false;
         setKeepMouseGrab(false);
         emit canceled();
         emit pressedChanged();
         if (d->hovered) {
            d->hovered = false;
            emit hoveredChanged();
         }
      }
   }
   return rv;
}

bool QDeclarativeMouseArea::sendMouseEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativeMouseArea);
   QGraphicsSceneMouseEvent mouseEvent(event->type());
   QRectF myRect = mapToScene(QRectF(0, 0, width(), height())).boundingRect();

   QGraphicsScene *s = scene();
   QDeclarativeItem *grabber = s ? qobject_cast<QDeclarativeItem *>(s->mouseGrabberItem()) : 0;
   bool stealThisEvent = d->stealMouse;
   if ((stealThisEvent || myRect.contains(event->scenePos().toPoint())) && (!grabber || !grabber->keepMouseGrab())) {
      mouseEvent.setAccepted(false);
      for (int i = 0x1; i <= 0x10; i <<= 1) {
         if (event->buttons() & i) {
            Qt::MouseButton button = Qt::MouseButton(i);
            mouseEvent.setButtonDownPos(button, mapFromScene(event->buttonDownPos(button)));
         }
      }
      mouseEvent.setScenePos(event->scenePos());
      mouseEvent.setLastScenePos(event->lastScenePos());
      mouseEvent.setPos(mapFromScene(event->scenePos()));
      mouseEvent.setLastPos(mapFromScene(event->lastScenePos()));

      switch (mouseEvent.type()) {
         case QEvent::GraphicsSceneMouseMove:
            mouseMoveEvent(&mouseEvent);
            break;
         case QEvent::GraphicsSceneMousePress:
            mousePressEvent(&mouseEvent);
            break;
         case QEvent::GraphicsSceneMouseRelease:
            mouseReleaseEvent(&mouseEvent);
            break;
         default:
            break;
      }
      grabber = qobject_cast<QDeclarativeItem *>(s->mouseGrabberItem());
      if (grabber && stealThisEvent && !grabber->keepMouseGrab() && grabber != this) {
         grabMouse();
      }

      return stealThisEvent;
   }
   if (mouseEvent.type() == QEvent::GraphicsSceneMouseRelease) {
      if (d->pressed) {
         d->pressed = false;
         d->stealMouse = false;
         if (s && s->mouseGrabberItem() == this) {
            ungrabMouse();
         }
         emit canceled();
         emit pressedChanged();
         if (d->hovered) {
            d->hovered = false;
            emit hoveredChanged();
         }
      }
   }
   return false;
}

bool QDeclarativeMouseArea::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
   Q_D(QDeclarativeMouseArea);
   if (!d->absorb || !isVisible() || !d->drag || !d->drag->filterChildren()) {
      return QDeclarativeItem::sceneEventFilter(i, e);
   }
   switch (e->type()) {
      case QEvent::GraphicsSceneMousePress:
      case QEvent::GraphicsSceneMouseMove:
      case QEvent::GraphicsSceneMouseRelease:
         return sendMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
      default:
         break;
   }

   return QDeclarativeItem::sceneEventFilter(i, e);
}

void QDeclarativeMouseArea::timerEvent(QTimerEvent *event)
{
   Q_D(QDeclarativeMouseArea);
   if (event->timerId() == d->pressAndHoldTimer.timerId()) {
      d->pressAndHoldTimer.stop();
      bool dragged = d->drag && d->drag->active();
      if (d->pressed && dragged == false && d->hovered == true) {
         d->longPress = true;
         QDeclarativeMouseEvent me(d->lastPos.x(), d->lastPos.y(), d->lastButton, d->lastButtons, d->lastModifiers, false,
                                   d->longPress);
         emit pressAndHold(&me);
      }
   }
}

void QDeclarativeMouseArea::geometryChanged(const QRectF &newGeometry,
      const QRectF &oldGeometry)
{
   Q_D(QDeclarativeMouseArea);
   QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);

   if (d->lastScenePos.isNull) {
      d->lastScenePos = mapToScene(d->lastPos);
   } else if (newGeometry.x() != oldGeometry.x() || newGeometry.y() != oldGeometry.y()) {
      d->lastPos = mapFromScene(d->lastScenePos);
   }
}

QVariant QDeclarativeMouseArea::itemChange(GraphicsItemChange change,
      const QVariant &value)
{
   Q_D(QDeclarativeMouseArea);
   switch (change) {
      case ItemVisibleHasChanged:
         if (acceptHoverEvents() && d->hovered != (isVisible() && isUnderMouse())) {
            setHovered(!d->hovered);
         }
         break;
      default:
         break;
   }

   return QDeclarativeItem::itemChange(change, value);
}

/*!
    \qmlproperty bool MouseArea::hoverEnabled
    This property holds whether hover events are handled.

    By default, mouse events are only handled in response to a button event, or when a button is
    pressed.  Hover enables handling of all mouse events even when no mouse button is
    pressed.

    This property affects the containsMouse property and the onEntered, onExited and
    onPositionChanged signals.
*/
bool QDeclarativeMouseArea::hoverEnabled() const
{
   return acceptHoverEvents();
}

void QDeclarativeMouseArea::setHoverEnabled(bool h)
{
   Q_D(QDeclarativeMouseArea);
   if (h == acceptHoverEvents()) {
      return;
   }

   setAcceptHoverEvents(h);
   emit hoverEnabledChanged();
   if (d->hovered != isUnderMouse()) {
      setHovered(!d->hovered);
   }
}

/*!
    \qmlproperty bool MouseArea::containsMouse
    This property holds whether the mouse is currently inside the mouse area.

    \warning This property is not updated if the area moves under the mouse: \e containsMouse will not change.
    In addition, if hoverEnabled is false, containsMouse will only be valid when the mouse is pressed.
*/
bool QDeclarativeMouseArea::hovered() const
{
   Q_D(const QDeclarativeMouseArea);
   return d->hovered;
}

/*!
    \qmlproperty bool MouseArea::pressed
    This property holds whether the mouse area is currently pressed.
*/
bool QDeclarativeMouseArea::pressed() const
{
   Q_D(const QDeclarativeMouseArea);
   return d->pressed;
}

void QDeclarativeMouseArea::setHovered(bool h)
{
   Q_D(QDeclarativeMouseArea);
   if (d->hovered != h) {
      d->hovered = h;
      emit hoveredChanged();
      d->hovered ? emit entered() : emit exited();
   }
}

/*!
    \qmlproperty Qt::MouseButtons MouseArea::acceptedButtons
    This property holds the mouse buttons that the mouse area reacts to.

    The available buttons are:
    \list
    \o Qt.LeftButton
    \o Qt.RightButton
    \o Qt.MiddleButton
    \o Qt.XButton1
    \o Qt.XButton2
    \endlist

    To accept more than one button the flags can be combined with the
    "|" (or) operator:

    \code
    MouseArea { acceptedButtons: Qt.LeftButton | Qt.RightButton }
    \endcode

    The default value is \c Qt.LeftButton.
*/
Qt::MouseButtons QDeclarativeMouseArea::acceptedButtons() const
{
   return acceptedMouseButtons();
}

void QDeclarativeMouseArea::setAcceptedButtons(Qt::MouseButtons buttons)
{
   if (buttons != acceptedMouseButtons()) {
      setAcceptedMouseButtons(buttons);
      emit acceptedButtonsChanged();
   }
}

bool QDeclarativeMouseArea::setPressed(bool p)
{
   Q_D(QDeclarativeMouseArea);
   bool dragged = d->drag && d->drag->active();
   bool isclick = d->pressed == true && p == false && dragged == false && d->hovered == true;

   if (d->pressed != p) {
      d->pressed = p;
      QDeclarativeMouseEvent me(d->lastPos.x(), d->lastPos.y(), d->lastButton, d->lastButtons, d->lastModifiers, isclick,
                                d->longPress);
      if (d->pressed) {
         if (!d->doubleClick) {
            emit pressed(&me);
         }
         me.setX(d->lastPos.x());
         me.setY(d->lastPos.y());
         emit mousePositionChanged(&me);
         emit pressedChanged();
      } else {
         emit released(&me);
         me.setX(d->lastPos.x());
         me.setY(d->lastPos.y());
         emit pressedChanged();
         if (isclick && !d->longPress && !d->doubleClick) {
            emit clicked(&me);
         }
      }

      return me.isAccepted();
   }
   return false;
}

QDeclarativeDrag *QDeclarativeMouseArea::drag()
{
   Q_D(QDeclarativeMouseArea);
   if (!d->drag) {
      d->drag = new QDeclarativeDrag;
   }
   return d->drag;
}

/*!
    \qmlproperty Item MouseArea::drag.target
    \qmlproperty bool MouseArea::drag.active
    \qmlproperty enumeration MouseArea::drag.axis
    \qmlproperty real MouseArea::drag.minimumX
    \qmlproperty real MouseArea::drag.maximumX
    \qmlproperty real MouseArea::drag.minimumY
    \qmlproperty real MouseArea::drag.maximumY
    \qmlproperty bool MouseArea::drag.filterChildren

    \c drag provides a convenient way to make an item draggable.

    \list
    \i \c drag.target specifies the id of the item to drag.
    \i \c drag.active specifies if the target item is currently being dragged.
    \i \c drag.axis specifies whether dragging can be done horizontally (\c Drag.XAxis), vertically (\c Drag.YAxis), or both (\c Drag.XandYAxis)
    \i \c drag.minimum and \c drag.maximum limit how far the target can be dragged along the corresponding axes.
    \endlist

    The following example displays a \l Rectangle that can be dragged along the X-axis. The opacity
    of the rectangle is reduced when it is dragged to the right.

    \snippet doc/src/snippets/declarative/mousearea/mousearea.qml drag

    \note Items cannot be dragged if they are anchored for the requested
    \c drag.axis. For example, if \c anchors.left or \c anchors.right was set
    for \c rect in the above example, it cannot be dragged along the X-axis.
    This can be avoided by settng the anchor value to \c undefined in
    an \l onPressed handler.

    If \c drag.filterChildren is set to true, a drag can override descendant MouseAreas.  This
    enables a parent MouseArea to handle drags, for example, while descendants handle clicks:

    \snippet doc/src/snippets/declarative/mousearea/mouseareadragfilter.qml dragfilter

*/

QT_END_NAMESPACE
