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

#include "qdeclarativepincharea_p.h"
#include "qdeclarativepincharea_p_p.h"

#include <QApplication>
#include <QGraphicsScene>

#include <float.h>
#include <math.h>

QT_BEGIN_NAMESPACE


/*!
    \qmlclass PinchEvent QDeclarativePinchEvent
    \ingroup qml-event-elements
    \brief The PinchEvent object provides information about a pinch event.

    \bold {The PinchEvent element was added in QtQuick 1.1}

    The \c center, \c startCenter, \c previousCenter properties provide the center position between the two touch points.

    The \c scale and \c previousScale properties provide the scale factor.

    The \c angle, \c previousAngle and \c rotation properties provide the angle between the two points and the amount of rotation.

    The \c point1, \c point2, \c startPoint1, \c startPoint2 properties provide the positions of the touch points.

    The \c accepted property may be set to false in the \c onPinchStarted handler if the gesture should not
    be handled.

    \sa PinchArea
*/

/*!
    \qmlproperty QPointF PinchEvent::center
    \qmlproperty QPointF PinchEvent::startCenter
    \qmlproperty QPointF PinchEvent::previousCenter

    These properties hold the position of the center point between the two touch points.

    \list
    \o \c center is the current center point
    \o \c previousCenter is the center point of the previous event.
    \o \c startCenter is the center point when the gesture began
    \endlist
*/

/*!
    \qmlproperty real PinchEvent::scale
    \qmlproperty real PinchEvent::previousScale

    These properties hold the scale factor determined by the change in distance between the two touch points.

    \list
    \o \c scale is the current scale factor.
    \o \c previousScale is the scale factor of the previous event.
    \endlist

    When a pinch gesture is started, the scale is 1.0.
*/

/*!
    \qmlproperty real PinchEvent::angle
    \qmlproperty real PinchEvent::previousAngle
    \qmlproperty real PinchEvent::rotation

    These properties hold the angle between the two touch points.

    \list
    \o \c angle is the current angle between the two points in the range -180 to 180.
    \o \c previousAngle is the angle of the previous event.
    \o \c rotation is the total rotation since the pinch gesture started.
    \endlist

    When a pinch gesture is started, the rotation is 0.0.
*/

/*!
    \qmlproperty QPointF PinchEvent::point1
    \qmlproperty QPointF PinchEvent::startPoint1
    \qmlproperty QPointF PinchEvent::point2
    \qmlproperty QPointF PinchEvent::startPoint2

    These properties provide the actual touch points generating the pinch.

    \list
    \o \c point1 and \c point2 hold the current positions of the points.
    \o \c startPoint1 and \c startPoint2 hold the positions of the points when the second point was touched.
    \endlist
*/

/*!
    \qmlproperty bool PinchEvent::accepted

    Setting this property to false in the \c PinchArea::onPinchStarted handler
    will result in no further pinch events being generated, and the gesture
    ignored.
*/

/*!
    \qmlproperty int PinchEvent::pointCount

    Holds the number of points currently touched.  The PinchArea will not react
    until two touch points have initited a gesture, but will remain active until
    all touch points have been released.
*/

QDeclarativePinch::QDeclarativePinch()
   : m_target(0), m_minScale(1.0), m_maxScale(1.0)
   , m_minRotation(0.0), m_maxRotation(0.0)
   , m_axis(NoDrag), m_xmin(-FLT_MAX), m_xmax(FLT_MAX)
   , m_ymin(-FLT_MAX), m_ymax(FLT_MAX), m_active(false)
{
}

QDeclarativePinchAreaPrivate::~QDeclarativePinchAreaPrivate()
{
   delete pinch;
}

/*!
    \qmlclass PinchArea QDeclarativePinchArea
    \brief The PinchArea item enables simple pinch gesture handling.
    \inherits Item

    \bold {The PinchArea element was added in QtQuick 1.1}

    A PinchArea is an invisible item that is typically used in conjunction with
    a visible item in order to provide pinch gesture handling for that item.

    The \l enabled property is used to enable and disable pinch handling for
    the proxied item. When disabled, the pinch area becomes transparent to
    mouse/touch events.

    PinchArea can be used in two ways:

    \list
    \o setting a \c pinch.target to provide automatic interaction with an element
    \o using the onPinchStarted, onPinchUpdated and onPinchFinished handlers
    \endlist

    \sa PinchEvent
*/

/*!
    \qmlsignal PinchArea::onPinchStarted()

    This handler is called when the pinch area detects that a pinch gesture has started.

    The \l {PinchEvent}{pinch} parameter provides information about the pinch gesture,
    including the scale, center and angle of the pinch.

    To ignore this gesture set the \c pinch.accepted property to false.  The gesture
    will be cancelled and no further events will be sent.
*/

/*!
    \qmlsignal PinchArea::onPinchUpdated()

    This handler is called when the pinch area detects that a pinch gesture has changed.

    The \l {PinchEvent}{pinch} parameter provides information about the pinch gesture,
    including the scale, center and angle of the pinch.
*/

/*!
    \qmlsignal PinchArea::onPinchFinished()

    This handler is called when the pinch area detects that a pinch gesture has finished.

    The \l {PinchEvent}{pinch} parameter provides information about the pinch gesture,
    including the scale, center and angle of the pinch.
*/


/*!
    \qmlproperty Item PinchArea::pinch.target
    \qmlproperty bool PinchArea::pinch.active
    \qmlproperty real PinchArea::pinch.minimumScale
    \qmlproperty real PinchArea::pinch.maximumScale
    \qmlproperty real PinchArea::pinch.minimumRotation
    \qmlproperty real PinchArea::pinch.maximumRotation
    \qmlproperty enumeration PinchArea::pinch.dragAxis
    \qmlproperty real PinchArea::pinch.minimumX
    \qmlproperty real PinchArea::pinch.maximumX
    \qmlproperty real PinchArea::pinch.minimumY
    \qmlproperty real PinchArea::pinch.maximumY

    \c pinch provides a convenient way to make an item react to pinch gestures.

    \list
    \i \c pinch.target specifies the id of the item to drag.
    \i \c pinch.active specifies if the target item is currently being dragged.
    \i \c pinch.minimumScale and \c pinch.maximumScale limit the range of the Item::scale property.
    \i \c pinch.minimumRotation and \c pinch.maximumRotation limit the range of the Item::rotation property.
    \i \c pinch.dragAxis specifies whether dragging in not allowed (\c Pinch.NoDrag), can be done horizontally (\c Pinch.XAxis), vertically (\c Pinch.YAxis), or both (\c Pinch.XandYAxis)
    \i \c pinch.minimum and \c pinch.maximum limit how far the target can be dragged along the corresponding axes.
    \endlist
*/

QDeclarativePinchArea::QDeclarativePinchArea(QDeclarativeItem *parent)
   : QDeclarativeItem(*(new QDeclarativePinchAreaPrivate), parent)
{
   Q_D(QDeclarativePinchArea);
   d->init();
}

QDeclarativePinchArea::~QDeclarativePinchArea()
{
}

/*!
    \qmlproperty bool PinchArea::enabled
    This property holds whether the item accepts pinch gestures.

    This property defaults to true.
*/
bool QDeclarativePinchArea::isEnabled() const
{
   Q_D(const QDeclarativePinchArea);
   return d->absorb;
}

void QDeclarativePinchArea::setEnabled(bool a)
{
   Q_D(QDeclarativePinchArea);
   if (a != d->absorb) {
      d->absorb = a;
      emit enabledChanged();
   }
}

bool QDeclarativePinchArea::event(QEvent *event)
{
   Q_D(QDeclarativePinchArea);
   if (!d->absorb || !isVisible()) {
      return QDeclarativeItem::event(event);
   }
   switch (event->type()) {
      case QEvent::TouchBegin:
         d->touchEventsActive = true;
      // No break, continue to next case.
      case QEvent::TouchUpdate:
         if (d->touchEventsActive) {
            QTouchEvent *touch = static_cast<QTouchEvent *>(event);
            d->touchPoints.clear();
            for (int i = 0; i < touch->touchPoints().count(); ++i) {
               if (!(touch->touchPoints().at(i).state() & Qt::TouchPointReleased)) {
                  d->touchPoints << touch->touchPoints().at(i);
               }
            }
            updatePinch();
            return true;
         }
         break;
      case QEvent::WindowDeactivate:
      // No break, continue to next case.
      case QEvent::TouchEnd:
         d->touchEventsActive = false;
         d->touchPoints.clear();
         updatePinch();
         break;
      default:
         return QDeclarativeItem::event(event);
   }

   return QDeclarativeItem::event(event);
}

void QDeclarativePinchArea::updatePinch()
{
   Q_D(QDeclarativePinchArea);
   if (d->touchPoints.count() == 0) {
      if (d->inPinch) {
         d->stealMouse = false;
         setKeepMouseGrab(false);
         d->inPinch = false;
         QPointF pinchCenter = mapFromScene(d->sceneLastCenter);
         QDeclarativePinchEvent pe(pinchCenter, d->pinchLastScale, d->pinchLastAngle, d->pinchRotation);
         pe.setStartCenter(d->pinchStartCenter);
         pe.setPreviousCenter(pinchCenter);
         pe.setPreviousAngle(d->pinchLastAngle);
         pe.setPreviousScale(d->pinchLastScale);
         pe.setStartPoint1(mapFromScene(d->sceneStartPoint1));
         pe.setStartPoint2(mapFromScene(d->sceneStartPoint2));
         pe.setPoint1(mapFromScene(d->lastPoint1));
         pe.setPoint2(mapFromScene(d->lastPoint2));
         emit pinchFinished(&pe);
         d->pinchStartDist = 0;
         d->pinchActivated = false;
         if (d->pinch && d->pinch->target()) {
            d->pinch->setActive(false);
         }
      }
      return;
   }
   QTouchEvent::TouchPoint touchPoint1 = d->touchPoints.at(0);
   QTouchEvent::TouchPoint touchPoint2 = d->touchPoints.at(d->touchPoints. count() >= 2 ? 1 : 0);
   if (d->touchPoints.count() == 2
         && (touchPoint1.state() & Qt::TouchPointPressed || touchPoint2.state() & Qt::TouchPointPressed)) {
      d->id1 = touchPoint1.id();
      d->sceneStartPoint1 = touchPoint1.scenePos();
      d->sceneStartPoint2 = touchPoint2.scenePos();
      d->inPinch = false;
      d->pinchRejected = false;
      d->pinchActivated = true;
   } else if (d->pinchActivated && !d->pinchRejected) {
      const int dragThreshold = QApplication::startDragDistance();
      QPointF p1 = touchPoint1.scenePos();
      QPointF p2 = touchPoint2.scenePos();
      qreal dx = p1.x() - p2.x();
      qreal dy = p1.y() - p2.y();
      qreal dist = sqrt(dx * dx + dy * dy);
      QPointF sceneCenter = (p1 + p2) / 2;
      qreal angle = QLineF(p1, p2).angle();
      if (d->touchPoints.count() == 1) {
         // If we only have one point then just move the center
         if (d->id1 == touchPoint1.id()) {
            sceneCenter = d->sceneLastCenter + touchPoint1.scenePos() - d->lastPoint1;
         } else {
            sceneCenter = d->sceneLastCenter + touchPoint2.scenePos() - d->lastPoint2;
         }
         angle = d->pinchLastAngle;
      }
      d->id1 = touchPoint1.id();
      if (angle > 180) {
         angle -= 360;
      }
      if (!d->inPinch) {
         if (d->touchPoints.count() >= 2
               && (qAbs(p1.x() - d->sceneStartPoint1.x()) > dragThreshold
                   || qAbs(p1.y() - d->sceneStartPoint1.y()) > dragThreshold
                   || qAbs(p2.x() - d->sceneStartPoint2.x()) > dragThreshold
                   || qAbs(p2.y() - d->sceneStartPoint2.y()) > dragThreshold)) {
            d->sceneStartCenter = sceneCenter;
            d->sceneLastCenter = sceneCenter;
            d->pinchStartCenter = mapFromScene(sceneCenter);
            d->pinchStartDist = dist;
            d->pinchStartAngle = angle;
            d->pinchLastScale = 1.0;
            d->pinchLastAngle = angle;
            d->pinchRotation = 0.0;
            d->lastPoint1 = p1;
            d->lastPoint2 = p2;
            QDeclarativePinchEvent pe(d->pinchStartCenter, 1.0, angle, 0.0);
            pe.setStartCenter(d->pinchStartCenter);
            pe.setPreviousCenter(d->pinchStartCenter);
            pe.setPreviousAngle(d->pinchLastAngle);
            pe.setPreviousScale(d->pinchLastScale);
            pe.setStartPoint1(mapFromScene(d->sceneStartPoint1));
            pe.setStartPoint2(mapFromScene(d->sceneStartPoint2));
            pe.setPoint1(mapFromScene(d->lastPoint1));
            pe.setPoint2(mapFromScene(d->lastPoint2));
            pe.setPointCount(d->touchPoints.count());
            emit pinchStarted(&pe);
            if (pe.accepted()) {
               d->inPinch = true;
               d->stealMouse = true;
               QGraphicsScene *s = scene();
               if (s && s->mouseGrabberItem() != this) {
                  grabMouse();
               }
               setKeepMouseGrab(true);
               if (d->pinch && d->pinch->target()) {
                  d->pinchStartPos = pinch()->target()->pos();
                  d->pinchStartScale = d->pinch->target()->scale();
                  d->pinchStartRotation = d->pinch->target()->rotation();
                  d->pinch->setActive(true);
               }
            } else {
               d->pinchRejected = true;
            }
         }
      } else if (d->pinchStartDist > 0) {
         qreal scale = dist ? dist / d->pinchStartDist : d->pinchLastScale;
         qreal da = d->pinchLastAngle - angle;
         if (da > 180) {
            da -= 360;
         } else if (da < -180) {
            da += 360;
         }
         d->pinchRotation += da;
         QPointF pinchCenter = mapFromScene(sceneCenter);
         QDeclarativePinchEvent pe(pinchCenter, scale, angle, d->pinchRotation);
         pe.setStartCenter(d->pinchStartCenter);
         pe.setPreviousCenter(mapFromScene(d->sceneLastCenter));
         pe.setPreviousAngle(d->pinchLastAngle);
         pe.setPreviousScale(d->pinchLastScale);
         pe.setStartPoint1(mapFromScene(d->sceneStartPoint1));
         pe.setStartPoint2(mapFromScene(d->sceneStartPoint2));
         pe.setPoint1(touchPoint1.pos());
         pe.setPoint2(touchPoint2.pos());
         pe.setPointCount(d->touchPoints.count());
         d->pinchLastScale = scale;
         d->sceneLastCenter = sceneCenter;
         d->pinchLastAngle = angle;
         d->lastPoint1 = touchPoint1.scenePos();
         d->lastPoint2 = touchPoint2.scenePos();
         emit pinchUpdated(&pe);
         if (d->pinch && d->pinch->target()) {
            qreal s = d->pinchStartScale * scale;
            s = qMin(qMax(pinch()->minimumScale(), s), pinch()->maximumScale());
            pinch()->target()->setScale(s);
            QPointF pos = sceneCenter - d->sceneStartCenter + d->pinchStartPos;
            if (pinch()->axis() & QDeclarativePinch::XAxis) {
               qreal x = pos.x();
               if (x < pinch()->xmin()) {
                  x = pinch()->xmin();
               } else if (x > pinch()->xmax()) {
                  x = pinch()->xmax();
               }
               pinch()->target()->setX(x);
            }
            if (pinch()->axis() & QDeclarativePinch::YAxis) {
               qreal y = pos.y();
               if (y < pinch()->ymin()) {
                  y = pinch()->ymin();
               } else if (y > pinch()->ymax()) {
                  y = pinch()->ymax();
               }
               pinch()->target()->setY(y);
            }
            if (d->pinchStartRotation >= pinch()->minimumRotation()
                  && d->pinchStartRotation <= pinch()->maximumRotation()) {
               qreal r = d->pinchRotation + d->pinchStartRotation;
               r = qMin(qMax(pinch()->minimumRotation(), r), pinch()->maximumRotation());
               pinch()->target()->setRotation(r);
            }
         }
      }
   }
}

void QDeclarativePinchArea::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativePinchArea);
   d->stealMouse = false;
   if (!d->absorb) {
      QDeclarativeItem::mousePressEvent(event);
   } else {
      setKeepMouseGrab(false);
      event->setAccepted(true);
   }
}

void QDeclarativePinchArea::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativePinchArea);
   if (!d->absorb) {
      QDeclarativeItem::mouseMoveEvent(event);
      return;
   }
}

void QDeclarativePinchArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativePinchArea);
   d->stealMouse = false;
   if (!d->absorb) {
      QDeclarativeItem::mouseReleaseEvent(event);
   } else {
      QGraphicsScene *s = scene();
      if (s && s->mouseGrabberItem() == this) {
         ungrabMouse();
      }
      setKeepMouseGrab(false);
   }
}

bool QDeclarativePinchArea::sceneEvent(QEvent *event)
{
   bool rv = QDeclarativeItem::sceneEvent(event);
   if (event->type() == QEvent::UngrabMouse) {
      setKeepMouseGrab(false);
   }
   return rv;
}

bool QDeclarativePinchArea::sendMouseEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativePinchArea);
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
      d->stealMouse = false;
      if (s && s->mouseGrabberItem() == this) {
         ungrabMouse();
      }
      setKeepMouseGrab(false);
   }
   return false;
}

bool QDeclarativePinchArea::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
   Q_D(QDeclarativePinchArea);
   if (!d->absorb || !isVisible()) {
      return QDeclarativeItem::sceneEventFilter(i, e);
   }
   switch (e->type()) {
      case QEvent::GraphicsSceneMousePress:
      case QEvent::GraphicsSceneMouseMove:
      case QEvent::GraphicsSceneMouseRelease:
         return sendMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
         break;
      case QEvent::TouchBegin:
      case QEvent::TouchUpdate: {
         QTouchEvent *touch = static_cast<QTouchEvent *>(e);
         d->touchPoints.clear();
         for (int i = 0; i < touch->touchPoints().count(); ++i)
            if (!(touch->touchPoints().at(i).state() & Qt::TouchPointReleased)) {
               d->touchPoints << touch->touchPoints().at(i);
            }
         updatePinch();
      }
      return d->inPinch;
      case QEvent::TouchEnd:
         d->touchPoints.clear();
         updatePinch();
         break;
      default:
         break;
   }

   return QDeclarativeItem::sceneEventFilter(i, e);
}

void QDeclarativePinchArea::geometryChanged(const QRectF &newGeometry,
      const QRectF &oldGeometry)
{
   QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
}

QVariant QDeclarativePinchArea::itemChange(GraphicsItemChange change,
      const QVariant &value)
{
   return QDeclarativeItem::itemChange(change, value);
}

QDeclarativePinch *QDeclarativePinchArea::pinch()
{
   Q_D(QDeclarativePinchArea);
   if (!d->pinch) {
      d->pinch = new QDeclarativePinch;
   }
   return d->pinch;
}


QT_END_NAMESPACE
