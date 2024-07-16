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

#include <qgesture.h>
#include <qgesture_p.h>
#include <qstandardgestures_p.h>
#include <qgraphicsview.h>

#include <qdebug_p.h>

#ifndef QT_NO_GESTURES

QGesture::QGesture(QObject *parent)
   : QObject(parent), d_ptr(new QGesturePrivate)
{
   d_ptr->q_ptr = this;
   d_func()->gestureType = Qt::CustomGesture;
}

QGesture::QGesture(QGesturePrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QGesture::~QGesture()
{
}

Qt::GestureType QGesture::gestureType() const
{
   return d_func()->gestureType;
}

Qt::GestureState QGesture::state() const
{
   return d_func()->state;
}

QPointF QGesture::hotSpot() const
{
   return d_func()->hotSpot;
}

void QGesture::setHotSpot(const QPointF &value)
{
   Q_D(QGesture);
   d->hotSpot = value;
   d->isHotSpotSet = true;
}

bool QGesture::hasHotSpot() const
{
   return d_func()->isHotSpotSet;
}

void QGesture::unsetHotSpot()
{
   d_func()->isHotSpotSet = false;
}

void QGesture::setGestureCancelPolicy(GestureCancelPolicy policy)
{
   Q_D(QGesture);
   d->gestureCancelPolicy = static_cast<uint>(policy);
}

QGesture::GestureCancelPolicy QGesture::gestureCancelPolicy() const
{
   Q_D(const QGesture);
   return static_cast<GestureCancelPolicy>(d->gestureCancelPolicy);
}

QPanGesture::QPanGesture(QObject *parent)
   : QGesture(*new QPanGesturePrivate, parent)
{
   d_func()->gestureType = Qt::PanGesture;
}

QPanGesture::~QPanGesture()
{
}

QPointF QPanGesture::lastOffset() const
{
   return d_func()->lastOffset;
}

QPointF QPanGesture::offset() const
{
   return d_func()->offset;
}

QPointF QPanGesture::delta() const
{
   Q_D(const QPanGesture);
   return d->offset - d->lastOffset;
}

qreal QPanGesture::acceleration() const
{
   return d_func()->acceleration;
}

void QPanGesture::setLastOffset(const QPointF &value)
{
   d_func()->lastOffset = value;
}

void QPanGesture::setOffset(const QPointF &value)
{
   d_func()->offset = value;
}

void QPanGesture::setAcceleration(qreal value)
{
   d_func()->acceleration = value;
}


QPinchGesture::QPinchGesture(QObject *parent)
   : QGesture(*new QPinchGesturePrivate, parent)
{
   d_func()->gestureType = Qt::PinchGesture;
}
QPinchGesture::~QPinchGesture()
{
}

QPinchGesture::ChangeFlags QPinchGesture::totalChangeFlags() const
{
   return d_func()->totalChangeFlags;
}

void QPinchGesture::setTotalChangeFlags(QPinchGesture::ChangeFlags value)
{
   d_func()->totalChangeFlags = value;
}

QPinchGesture::ChangeFlags QPinchGesture::changeFlags() const
{
   return d_func()->changeFlags;
}

void QPinchGesture::setChangeFlags(QPinchGesture::ChangeFlags value)
{
   d_func()->changeFlags = value;
}

QPointF QPinchGesture::startCenterPoint() const
{
   return d_func()->startCenterPoint;
}

QPointF QPinchGesture::lastCenterPoint() const
{
   return d_func()->lastCenterPoint;
}

QPointF QPinchGesture::centerPoint() const
{
   return d_func()->centerPoint;
}

void QPinchGesture::setStartCenterPoint(const QPointF &value)
{
   d_func()->startCenterPoint = value;
}

void QPinchGesture::setLastCenterPoint(const QPointF &value)
{
   d_func()->lastCenterPoint = value;
}

void QPinchGesture::setCenterPoint(const QPointF &value)
{
   d_func()->centerPoint = value;
}


qreal QPinchGesture::totalScaleFactor() const
{
   return d_func()->totalScaleFactor;
}

qreal QPinchGesture::lastScaleFactor() const
{
   return d_func()->lastScaleFactor;
}

qreal QPinchGesture::scaleFactor() const
{
   return d_func()->scaleFactor;
}

void QPinchGesture::setTotalScaleFactor(qreal value)
{
   d_func()->totalScaleFactor = value;
}

void QPinchGesture::setLastScaleFactor(qreal value)
{
   d_func()->lastScaleFactor = value;
}

void QPinchGesture::setScaleFactor(qreal value)
{
   d_func()->scaleFactor = value;
}


qreal QPinchGesture::totalRotationAngle() const
{
   return d_func()->totalRotationAngle;
}

qreal QPinchGesture::lastRotationAngle() const
{
   return d_func()->lastRotationAngle;
}

qreal QPinchGesture::rotationAngle() const
{
   return d_func()->rotationAngle;
}

void QPinchGesture::setTotalRotationAngle(qreal value)
{
   d_func()->totalRotationAngle = value;
}

void QPinchGesture::setLastRotationAngle(qreal value)
{
   d_func()->lastRotationAngle = value;
}

void QPinchGesture::setRotationAngle(qreal value)
{
   d_func()->rotationAngle = value;
}

QSwipeGesture::QSwipeGesture(QObject *parent)
   : QGesture(*new QSwipeGesturePrivate, parent)
{
   d_func()->gestureType = Qt::SwipeGesture;
}

QSwipeGesture::~QSwipeGesture()
{
}
QSwipeGesture::SwipeDirection QSwipeGesture::horizontalDirection() const
{
   Q_D(const QSwipeGesture);
   if (d->swipeAngle < 0 || d->swipeAngle == 90 || d->swipeAngle == 270) {
      return QSwipeGesture::NoDirection;
   } else if (d->swipeAngle < 90 || d->swipeAngle > 270) {
      return QSwipeGesture::Right;
   } else {
      return QSwipeGesture::Left;
   }
}

QSwipeGesture::SwipeDirection QSwipeGesture::verticalDirection() const
{
   Q_D(const QSwipeGesture);
   if (d->swipeAngle <= 0 || d->swipeAngle == 180) {
      return QSwipeGesture::NoDirection;
   } else if (d->swipeAngle < 180) {
      return QSwipeGesture::Up;
   } else {
      return QSwipeGesture::Down;
   }
}

qreal QSwipeGesture::swipeAngle() const
{
   return d_func()->swipeAngle;
}

void QSwipeGesture::setSwipeAngle(qreal value)
{
   d_func()->swipeAngle = value;
}

QTapGesture::QTapGesture(QObject *parent)
   : QGesture(*new QTapGesturePrivate, parent)
{
   d_func()->gestureType = Qt::TapGesture;
}

QTapGesture::~QTapGesture()
{
}

QPointF QTapGesture::position() const
{
   return d_func()->position;
}

void QTapGesture::setPosition(const QPointF &value)
{
   d_func()->position = value;
}

QTapAndHoldGesture::QTapAndHoldGesture(QObject *parent)
   : QGesture(*new QTapAndHoldGesturePrivate, parent)
{
   d_func()->gestureType = Qt::TapAndHoldGesture;
}
QTapAndHoldGesture::~QTapAndHoldGesture()
{
}

QPointF QTapAndHoldGesture::position() const
{
   return d_func()->position;
}

void QTapAndHoldGesture::setPosition(const QPointF &value)
{
   d_func()->position = value;
}

// static
void QTapAndHoldGesture::setTimeout(int msecs)
{
   QTapAndHoldGesturePrivate::Timeout = msecs;
}

// static
int QTapAndHoldGesture::timeout()
{
   return QTapAndHoldGesturePrivate::Timeout;
}

int QTapAndHoldGesturePrivate::Timeout = 700; // in ms

QGestureEvent::QGestureEvent(const QList<QGesture *> &gestures)
    : QEvent(QEvent::Gesture), m_gestures(gestures), m_widget(nullptr)
{
}

QGestureEvent::~QGestureEvent()
{
}

QList<QGesture *> QGestureEvent::gestures() const
{
    return m_gestures;
}

QGesture *QGestureEvent::gesture(Qt::GestureType type) const
{
    for (int i = 0; i < m_gestures.size(); ++i) {
        if (m_gestures.at(i)->gestureType() == type) {
            return m_gestures.at(i);
        }
    }

    return nullptr;
}

QList<QGesture *> QGestureEvent::activeGestures() const
{
    QList<QGesture *> gestures;

    for (QGesture *gesture : m_gestures) {
        if (gesture->state() != Qt::GestureCanceled)
            gestures.append(gesture);
    }
    return gestures;
}

QList<QGesture *> QGestureEvent::canceledGestures() const
{
    QList<QGesture *> gestures;

    for (QGesture *gesture : m_gestures) {
        if (gesture->state() == Qt::GestureCanceled) {
           gestures.append(gesture);
        }
    }

    return gestures;
}

void QGestureEvent::setAccepted(QGesture *gesture, bool isAccepted)
{
   if (gesture) {
      setAccepted(gesture->gestureType(), isAccepted);
   }
}

void QGestureEvent::accept(QGesture *gesture)
{
   if (gesture) {
      setAccepted(gesture->gestureType(), true);
   }
}

void QGestureEvent::ignore(QGesture *gesture)
{
   if (gesture) {
      setAccepted(gesture->gestureType(), false);
   }
}

bool QGestureEvent::isAccepted(QGesture *gesture) const
{
    return gesture ? isAccepted(gesture->gestureType()) : false;
}

void QGestureEvent::setAccepted(Qt::GestureType gestureType, bool isAccepted)
{
    setAccepted(false);
    m_accepted[gestureType] = isAccepted;
}

void QGestureEvent::accept(Qt::GestureType gestureType)
{
    setAccepted(gestureType, true);
}

void QGestureEvent::ignore(Qt::GestureType gestureType)
{
    setAccepted(gestureType, false);
}

bool QGestureEvent::isAccepted(Qt::GestureType gestureType) const
{
    return m_accepted.value(gestureType, true);
}

void QGestureEvent::setWidget(QWidget *widget)
{
    m_widget = widget;
}

QWidget *QGestureEvent::widget() const
{
    return m_widget;
}

#ifndef QT_NO_GRAPHICSVIEW
QPointF QGestureEvent::mapToGraphicsScene(const QPointF &gesturePoint) const
{
    QWidget *w = widget();

    if (w != nullptr) {
       // we get the viewport as widget, not the graphics view
       w = w->parentWidget();
    }

    QGraphicsView *view = qobject_cast<QGraphicsView*>(w);

    if (view != nullptr) {
        return view->mapToScene(view->mapFromGlobal(gesturePoint.toPoint()));
    }

    return QPointF();
}
#endif

static void formatGestureHeader(QDebug debug, const char *className, const QGesture *gesture)
{
     debug << className << "(state = ";
     QtDebugUtils::formatQEnum(debug, gesture->state());

     if (gesture->hasHotSpot()) {
         debug << ", hotSpot = ";
         QtDebugUtils::formatQPoint(debug, gesture->hotSpot());
     }
}

Q_GUI_EXPORT QDebug operator<<(QDebug debug, const QGesture *gesture)
{
    QDebugStateSaver saver(debug);
    debug.nospace();

    switch (gesture->gestureType()) {
       case Qt::TapGesture:
           formatGestureHeader(debug, "QTapGesture", gesture);

           debug << ", position = ";
           QtDebugUtils::formatQPoint(debug, static_cast<const QTapGesture*>(gesture)->position());

           debug << ')';
           break;

       case Qt::TapAndHoldGesture: {
           const QTapAndHoldGesture *tap = static_cast<const QTapAndHoldGesture*>(gesture);
           formatGestureHeader(debug, "QTapAndHoldGesture", tap);

           debug << ", position = ";
           QtDebugUtils::formatQPoint(debug, tap->position());

           debug << ", timeout = " << tap->timeout() << ')';
       }
       break;

       case Qt::PanGesture: {
           const QPanGesture *pan = static_cast<const QPanGesture*>(gesture);
           formatGestureHeader(debug, "QPanGesture", pan);

           debug << ", lastOffset = ";
           QtDebugUtils::formatQPoint(debug, pan->lastOffset());

           debug << ", offset = ";
           QtDebugUtils::formatQPoint(debug, pan->offset());

           debug  << ", acceleration = " << pan->acceleration() << ", delta = ";
           QtDebugUtils::formatQPoint(debug, pan->delta());
           debug << ')';
       }
       break;

       case Qt::PinchGesture: {
           const QPinchGesture *pinch = static_cast<const QPinchGesture*>(gesture);
           formatGestureHeader(debug, "QPinchGesture", pinch);

           debug << ", totalChangeFlags = " << pinch->totalChangeFlags()
             << ", changeFlags = " << pinch->changeFlags() << ",startCenterPoint=";
           QtDebugUtils::formatQPoint(debug, pinch->startCenterPoint());

           debug << ", lastCenterPoint = ";
           QtDebugUtils::formatQPoint(debug, pinch->lastCenterPoint());

           debug << ", centerPoint = ";
           QtDebugUtils::formatQPoint(debug, pinch->centerPoint());

           debug << ",totalScaleFactor = " << pinch->totalScaleFactor()
               << ", lastScaleFactor = " << pinch->lastScaleFactor()
               << ", scaleFactor = " << pinch->scaleFactor()
               << ", totalRotationAngle = " << pinch->totalRotationAngle()
               << ", lastRotationAngle = " << pinch->lastRotationAngle()
               << ", rotationAngle =" << pinch->rotationAngle() << ')';
       }
       break;

       case Qt::SwipeGesture: {
           const QSwipeGesture *swipe = static_cast<const QSwipeGesture*>(gesture);
           formatGestureHeader(debug, "QSwipeGesture", swipe);

           debug << ", horizontalDirection = ";
           QtDebugUtils::formatQEnum(debug, swipe->horizontalDirection());

           debug << ", verticalDirection = ";
           QtDebugUtils::formatQEnum(debug, swipe->verticalDirection());

           debug << ", swipeAngle = " << swipe->swipeAngle() << ')';
       }
       break;

    default:
        formatGestureHeader(debug, "Custom gesture", gesture);
        debug << ", type = " << gesture->gestureType() << ')';
        break;
    }

    return debug;
}

Q_GUI_EXPORT QDebug operator<<(QDebug debug, const QGestureEvent *gestureEvent)
{
    QDebugStateSaver saver(debug);
    debug.nospace();

    debug << "QGestureEvent(" << gestureEvent->gestures() << ')';

    return debug;
}

// wrapper for overloaded method
qreal QPanGesture::cs_horizontalVelocity() const
{
   Q_D(const QPanGesture);
   return d->horizontalVelocity();
}

// wrapper for overloaded method
void QPanGesture::cs_setHorizontalVelocity(qreal velocity)
{
   Q_D(QPanGesture);
   d->setHorizontalVelocity(velocity);
}

// wrapper for overloaded method
qreal QPanGesture::cs_verticalVelocity() const
{
   Q_D(const QPanGesture);
   return d->verticalVelocity();
}

// wrapper for overloaded method
void QPanGesture::cs_setVerticalVelocity(qreal velocity)
{
   Q_D(QPanGesture);
   d->setVerticalVelocity(velocity);
}

// wrapper for overloaded method
qreal QSwipeGesture::cs_velocity() const
{
   Q_D(const QSwipeGesture);
   return d->velocity();
}

// wrapper for overloaded method
void QSwipeGesture::cs_setVelocity(qreal velocity)
{
   Q_D(QSwipeGesture);
   d->setVelocity(velocity);
}

#endif // QT_NO_GESTURES
