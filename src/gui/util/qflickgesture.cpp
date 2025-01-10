/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qflickgesture_p.h>

#include <qapplication.h>
#include <qdebug.h>
#include <qevent.h>
#include <qgesture.h>
#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsview.h>
#include <qscroller.h>
#include <qwidget.h>

#include <qapplication_p.h>
#include <qevent_p.h>

#ifndef QT_NO_GESTURES

extern bool qt_sendSpontaneousEvent(QObject *receiver, QEvent *event);

static QMouseEvent *copyMouseEvent(QEvent *e)
{
   switch (e->type()) {
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseMove: {
         QMouseEvent *me = static_cast<QMouseEvent *>(e);
         QMouseEvent *cme = new QMouseEvent(me->type(), QPoint(0, 0), me->windowPos(), me->screenPos(),
               me->button(), me->buttons(), me->modifiers(), me->source());
         return cme;
      }

#ifndef QT_NO_GRAPHICSVIEW

      case QEvent::GraphicsSceneMousePress:
      case QEvent::GraphicsSceneMouseRelease:
      case QEvent::GraphicsSceneMouseMove: {
         QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(e);

         QEvent::Type met = me->type() == QEvent::GraphicsSceneMousePress ? QEvent::MouseButtonPress :
               (me->type() == QEvent::GraphicsSceneMouseRelease ? QEvent::MouseButtonRelease : QEvent::MouseMove);

         QMouseEvent *cme = new QMouseEvent(met, QPoint(0, 0), QPoint(0, 0), me->screenPos(),
               me->button(), me->buttons(), me->modifiers(), me->source());

         return cme;
      }

#endif

      default:
         return nullptr;
   }
}

class PressDelayHandler : public QObject
{
 private:
   PressDelayHandler(QObject *parent = nullptr)
      : QObject(parent), pressDelayTimer(0), sendingEvent(false), mouseButton(Qt::NoButton),
        mouseTarget(nullptr), mouseEventSource(Qt::MouseEventNotSynthesized)
   {
   }

   static PressDelayHandler *inst;

 public:
   enum {
      UngrabMouseBefore     = 1,
      RegrabMouseAfterwards = 2
   };

   static PressDelayHandler *instance() {
      static PressDelayHandler *inst = nullptr;

      if (! inst) {
         inst = new PressDelayHandler(QCoreApplication::instance());
      }

      return inst;
   }

   bool shouldEventBeIgnored(QEvent *) const {
      return sendingEvent;
   }

   bool isDelaying() const {
      return ! pressDelayEvent.isNull();
   }

   void pressed(QEvent *e, int delay) {
      if (!pressDelayEvent) {
         pressDelayEvent.reset(copyMouseEvent(e));
         pressDelayTimer = startTimer(delay);
         mouseTarget = QApplication::widgetAt(pressDelayEvent->globalPos());
         mouseButton = pressDelayEvent->button();
         mouseEventSource = pressDelayEvent->source();

#if defined(CS_SHOW_DEBUG_GUI)
         qDebug("QFG: consuming/delaying mouse press");
#endif

      } else {
#if defined(CS_SHOW_DEBUG_GUI)
         qDebug("QFG: NOT consuming/delaying mouse press");
#endif
      }

      e->setAccepted(true);
   }

   bool released(QEvent *e, bool scrollerWasActive, bool scrollerIsActive) {
      // consume this event if the scroller was or is active
      bool result = scrollerWasActive || scrollerIsActive;

      // stop the timer
      if (pressDelayTimer) {
         killTimer(pressDelayTimer);
         pressDelayTimer = 0;
      }

      // we still haven't even sent the press, so do it now
      if (pressDelayEvent && mouseTarget && !scrollerIsActive) {
         QScopedPointer<QMouseEvent> releaseEvent(copyMouseEvent(e));

#if defined(CS_SHOW_DEBUG_GUI)
         qDebug() << "QFG: re-sending mouse press (due to release) for " << mouseTarget;
#endif

         sendMouseEvent(pressDelayEvent.data(), UngrabMouseBefore);

#if defined(CS_SHOW_DEBUG_GUI)
         qDebug() << "QFG: faking mouse release (due to release) for " << mouseTarget;
#endif

         sendMouseEvent(releaseEvent.data());

         result = true; // consume this event

      } else if (mouseTarget && scrollerIsActive) {
         // we grabbed the mouse expicitly when the scroller became active, so undo that now
         sendMouseEvent(nullptr, UngrabMouseBefore);
      }

      pressDelayEvent.reset(nullptr);
      mouseTarget = nullptr;
      return result;
   }

   void scrollerWasIntercepted() {
#if defined(CS_SHOW_DEBUG_GUI)
      qDebug("QFG: deleting delayed mouse press, since scroller was only intercepted");
#endif

      if (pressDelayEvent) {
         // we still have not even sent the press, just throw it away now
         if (pressDelayTimer) {
            killTimer(pressDelayTimer);
            pressDelayTimer = 0;
         }

         pressDelayEvent.reset(nullptr);
      }

      mouseTarget = nullptr;
   }

   void scrollerBecameActive() {
      if (pressDelayEvent) {
         // we still have not even sent the press, just throw it away now

#if defined(CS_SHOW_DEBUG_GUI)
         qDebug("QFG: deleting delayed mouse press, since scroller is active now");
#endif

         if (pressDelayTimer) {
            killTimer(pressDelayTimer);
            pressDelayTimer = 0;
         }

         pressDelayEvent.reset(nullptr);
         mouseTarget = nullptr;

      } else if (mouseTarget) {
         // we did send a press, so we need to fake a release now

         // release all pressed mouse buttons
         /* Qt::MouseButtons mouseButtons = QApplication::mouseButtons();
            for (int i = 0; i < 32; ++i) {
                if (mouseButtons & (1 << i)) {
                    Qt::MouseButton b = static_cast<Qt::MouseButton>(1 << i);
                    mouseButtons &= ~b;
                    QPoint farFarAway(-QWIDGETSIZE_MAX, -QWIDGETSIZE_MAX);

#if defined(CS_SHOW_DEBUG_GUI)
                    qDebug() << "QFG: sending a fake mouse release at far-far-away to " << mouseTarget;
#endif

                    QMouseEvent re(QEvent::MouseButtonRelease, QPoint(), farFarAway,
                                   b, mouseButtons, QApplication::keyboardModifiers());
                    sendMouseEvent(&re);
                }
            }*/

         QPoint farFarAway(-QWIDGETSIZE_MAX, -QWIDGETSIZE_MAX);

#if defined(CS_SHOW_DEBUG_GUI)
         qDebug() << "QFG: sending a fake mouse release to " << mouseTarget;
#endif

         QMouseEvent re(QEvent::MouseButtonRelease, QPoint(), farFarAway, farFarAway,
               mouseButton, QApplication::mouseButtons() & ~mouseButton,
               QApplication::keyboardModifiers(), mouseEventSource);

         sendMouseEvent(&re, RegrabMouseAfterwards);

         // do not clear the mouseTarget just yet, since we need to explicitly ungrab the mouse on release!
      }
   }

 protected:
   void timerEvent(QTimerEvent *e) override {
      if (e->timerId() == pressDelayTimer) {
         if (pressDelayEvent && mouseTarget) {

#if defined(CS_SHOW_DEBUG_GUI)
            qDebug() << "QFG: timer event: re-sending mouse press to " << mouseTarget;
#endif

            sendMouseEvent(pressDelayEvent.data(), UngrabMouseBefore);
         }

         pressDelayEvent.reset(nullptr);

         if (pressDelayTimer) {
            killTimer(pressDelayTimer);
            pressDelayTimer = 0;
         }
      }
   }

   void sendMouseEvent(QMouseEvent *me, int flags = 0) {
      if (mouseTarget) {
         sendingEvent = true;

#ifndef QT_NO_GRAPHICSVIEW
         QGraphicsItem *grabber = nullptr;

         if (mouseTarget->parentWidget()) {
            if (QGraphicsView *gv = qobject_cast<QGraphicsView *>(mouseTarget->parentWidget())) {
               if (gv->scene()) {
                  grabber = gv->scene()->mouseGrabberItem();
               }
            }
         }

         if (grabber && (flags & UngrabMouseBefore)) {
            // GraphicsView Mouse Handling Workaround #1:
            // we need to ungrab the mouse before re-sending the press,
            // since the scene had already set the mouse grabber to the
            // original (and consumed) event's receiver

#if defined(CS_SHOW_DEBUG_GUI)
            qDebug() << "QFG: ungrabbing" << grabber;
#endif
            grabber->ungrabMouse();
         }

#endif

         if (me) {
            QMouseEvent copy(me->type(), mouseTarget->mapFromGlobal(me->globalPos()),
                  mouseTarget->topLevelWidget()->mapFromGlobal(me->globalPos()), me->screenPos(),
                  me->button(), me->buttons(), me->modifiers(), me->source());
            qt_sendSpontaneousEvent(mouseTarget, &copy);
         }

#ifndef QT_NO_GRAPHICSVIEW

         if (grabber && (flags & RegrabMouseAfterwards)) {
            // GraphicsView Mouse Handling Workaround #2:
            // we need to re-grab the mouse after sending a faked mouse
            // release, since we still need the mouse moves for the gesture
            // (the scene will clear the item's mouse grabber status on release)

#if defined(CS_SHOW_DEBUG_GUI)
            qDebug() << "QFG: re-grabbing" << grabber;
#endif
            grabber->grabMouse();
         }

#endif
         sendingEvent = false;
      }
   }

 private:
   int pressDelayTimer;
   QScopedPointer<QMouseEvent> pressDelayEvent;
   bool sendingEvent;
   Qt::MouseButton mouseButton;
   QPointer<QWidget> mouseTarget;
   Qt::MouseEventSource mouseEventSource;
};

QFlickGesture::QFlickGesture(QObject *receiver, Qt::MouseButton button, QObject *parent)
   : QGesture(*new QFlickGesturePrivate, parent)
{
   d_func()->q_ptr = this;
   d_func()->receiver = receiver;
   d_func()->receiverScroller = (receiver && QScroller::hasScroller(receiver)) ? QScroller::scroller(receiver) : nullptr;
   d_func()->button = button;
}

QFlickGesture::~QFlickGesture()
{
}

QFlickGesturePrivate::QFlickGesturePrivate()
   : receiverScroller(nullptr), button(Qt::NoButton), macIgnoreWheel(false)
{
}

QFlickGestureRecognizer::QFlickGestureRecognizer(Qt::MouseButton button)
{
   this->button = button;
}

QGesture *QFlickGestureRecognizer::create(QObject *target)
{
#ifndef QT_NO_GRAPHICSVIEW
   QGraphicsObject *go = qobject_cast<QGraphicsObject *>(target);

   if (go && button == Qt::NoButton) {
      go->setAcceptTouchEvents(true);
   }

#endif

   return new QFlickGesture(target, button);
}

QGestureRecognizer::Result QFlickGestureRecognizer::recognize(QGesture *state, QObject *watched, QEvent *event)
{
   (void) watched;

   static QElapsedTimer monotonicTimer;

   if (! monotonicTimer.isValid()) {
      monotonicTimer.start();
   }

   QFlickGesture *q = static_cast<QFlickGesture *>(state);
   QFlickGesturePrivate *d = q->d_func();

   QScroller *scroller = d->receiverScroller;

   if (! scroller) {
      // nothing to do without a scroller?
      return Ignore;
   }

   QWidget *receiverWidget = qobject_cast<QWidget *>(d->receiver);

#ifndef QT_NO_GRAPHICSVIEW
   QGraphicsObject *receiverGraphicsObject = qobject_cast<QGraphicsObject *>(d->receiver);
#endif

   // only set for events that we inject into the event loop via sendEvent()
   if (PressDelayHandler::instance()->shouldEventBeIgnored(event)) {
      return Ignore;
   }

   const QMouseEvent *me = nullptr;

#ifndef QT_NO_GRAPHICSVIEW
   const QGraphicsSceneMouseEvent *gsme = nullptr;
#endif

   const QTouchEvent *te = nullptr;
   QPoint globalPos;

   switch (event->type()) {
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseMove:
         if (! receiverWidget) {
            return Ignore;
         }

         if (button != Qt::NoButton) {
            me = static_cast<const QMouseEvent *>(event);
            globalPos = me->globalPos();
         }

         break;

#ifndef QT_NO_GRAPHICSVIEW

      case QEvent::GraphicsSceneMousePress:
      case QEvent::GraphicsSceneMouseRelease:
      case QEvent::GraphicsSceneMouseMove:
         if (! receiverGraphicsObject) {
            return Ignore;
         }

         if (button != Qt::NoButton) {
            gsme = static_cast<const QGraphicsSceneMouseEvent *>(event);
            globalPos = gsme->screenPos();
         }

         break;
#endif

      case QEvent::TouchBegin:
      case QEvent::TouchEnd:
      case QEvent::TouchUpdate:
         if (button == Qt::NoButton) {
            te = static_cast<const QTouchEvent *>(event);

            if (! te->touchPoints().isEmpty()) {
               globalPos = te->touchPoints().at(0).screenPos().toPoint();
            }
         }

         break;

      // consume all wheel events if the scroller is active
      case QEvent::Wheel:
         if (d->macIgnoreWheel || (scroller->state() != QScroller::Inactive)) {
            return Ignore | ConsumeEventHint;
         }

         break;

      // consume all dbl click events if the scroller is active
      case QEvent::MouseButtonDblClick:
         if (scroller->state() != QScroller::Inactive) {
            return Ignore | ConsumeEventHint;
         }

         break;

      default:
         break;
   }

#ifndef QT_NO_GRAPHICSVIEW
   if (! me && ! gsme && ! te) {
#else
   if (! me && ! te) {
#endif

      // Neither mouse nor touch
      return Ignore;
   }

   // get the current pointer position in local coordinates.
   QPointF point;
   QScroller::Input inputType = (QScroller::Input) 0;

   switch (event->type()) {
      case QEvent::MouseButtonPress:
         if (me && me->button() == button && me->buttons() == button) {
            point = me->globalPos();
            inputType = QScroller::InputPress;
         } else if (me) {
            scroller->stop();
            return CancelGesture;
         }

         break;

      case QEvent::MouseButtonRelease:
         if (me && me->button() == button) {
            point = me->globalPos();
            inputType = QScroller::InputRelease;
         }

         break;

      case QEvent::MouseMove:
         if (me && me->buttons() == button) {
            point = me->globalPos();
            inputType = QScroller::InputMove;
         }

         break;

#ifndef QT_NO_GRAPHICSVIEW

      case QEvent::GraphicsSceneMousePress:
         if (gsme && gsme->button() == button && gsme->buttons() == button) {
            point = gsme->scenePos();
            inputType = QScroller::InputPress;
         } else if (gsme) {
            scroller->stop();
            return CancelGesture;
         }

         break;

      case QEvent::GraphicsSceneMouseRelease:
         if (gsme && gsme->button() == button) {
            point = gsme->scenePos();
            inputType = QScroller::InputRelease;
         }

         break;

      case QEvent::GraphicsSceneMouseMove:
         if (gsme && gsme->buttons() == button) {
            point = gsme->scenePos();
            inputType = QScroller::InputMove;
         }

         break;
#endif

      case QEvent::TouchBegin:
         inputType = QScroller::InputPress;
         [[fallthrough]];

      case QEvent::TouchEnd:
         if (!inputType) {
            inputType = QScroller::InputRelease;
         }

         [[fallthrough]];

      case QEvent::TouchUpdate:
         if (! inputType) {
            inputType = QScroller::InputMove;
         }

         if (te->device()->type() == QTouchDevice::TouchPad) {
            if (te->touchPoints().count() != 2) {
               // 2 fingers on pad
               return Ignore;
            }

            point = te->touchPoints().at(0).startScenePos() +
                  ((te->touchPoints().at(0).scenePos() - te->touchPoints().at(0).startScenePos()) +
                  (te->touchPoints().at(1).scenePos() - te->touchPoints().at(1).startScenePos())) / 2;

         } else {
            // TouchScreen
            if (te->touchPoints().count() != 1) {
               // 1 finger on screen
               return Ignore;
            }

            point = te->touchPoints().at(0).scenePos();
         }

         break;

      default:
         break;
   }

   // Check for an active scroller at globalPos
   if (inputType == QScroller::InputPress) {
      for (QScroller *as : QScroller::activeScrollers()) {
         if (as != scroller) {
            QRegion scrollerRegion;

            if (QWidget *w = dynamic_cast<QWidget *>(as->target())) {
               scrollerRegion = QRect(w->mapToGlobal(QPoint(0, 0)), w->size());

#ifndef QT_NO_GRAPHICSVIEW
            } else if (QGraphicsObject *go = dynamic_cast<QGraphicsObject *>(as->target())) {
               if (go->scene() && ! go->scene()->views().isEmpty()) {

                  for (QGraphicsView *gv : go->scene()->views()) {
                     scrollerRegion |= gv->mapFromScene(go->mapToScene(go->boundingRect()))
                           .translated(gv->mapToGlobal(QPoint(0, 0)));
                  }
               }

#endif
            }

            // active scrollers always have priority
            if (scrollerRegion.contains(globalPos)) {
               return Ignore;
            }
         }
      }
   }

   bool scrollerWasDragging  = (scroller->state() == QScroller::Dragging);
   bool scrollerWasScrolling = (scroller->state() == QScroller::Scrolling);

   if (inputType) {
      if (QWidget *w = qobject_cast<QWidget *>(d->receiver)) {
         point = w->mapFromGlobal(point.toPoint());
      }

#ifndef QT_NO_GRAPHICSVIEW
      else if (QGraphicsObject *go = qobject_cast<QGraphicsObject *>(d->receiver)) {
         point = go->mapFromScene(point);
      }
#endif

      // inform the scroller about the new event
      scroller->handleInput(inputType, point, monotonicTimer.elapsed());
   }

   // depending on the scroller state return the gesture state
   Result result(Qt::EmptyFlag);
   bool scrollerIsActive = (scroller->state() == QScroller::Dragging ||
         scroller->state() == QScroller::Scrolling);

   // Consume all mouse events while dragging or scrolling to avoid nasty
   // side effects with standard widgets

#ifndef QT_NO_GRAPHICSVIEW
   if ((me || gsme) && scrollerIsActive) {

#else
   if (me && scrollerIsActive) {

#endif

      result |= ConsumeEventHint;
   }

   // The only problem with this approach is that we consume the
   // MouseRelease when we start the scrolling with a flick gesture. We
   // have to fake a MouseRelease "somewhere" to not mess with the
   // state of the widgets (a QPushButton would stay in 'pressed' state
   // forever if it does not receive a MouseRelease).

#ifndef QT_NO_GRAPHICSVIEW
   if (me || gsme) {
#else
   if (me) {
#endif

      if (! scrollerWasDragging && ! scrollerWasScrolling && scrollerIsActive) {
         PressDelayHandler::instance()->scrollerBecameActive();
      } else if (scrollerWasScrolling && (scroller->state() == QScroller::Dragging || scroller->state() == QScroller::Inactive)) {
         PressDelayHandler::instance()->scrollerWasIntercepted();
      }
   }

   if (! inputType) {
      result |= Ignore;

   } else {
      switch (event->type()) {
         case QEvent::MouseButtonPress:

#ifndef QT_NO_GRAPHICSVIEW
         case QEvent::GraphicsSceneMousePress:
#endif

            if (scroller->state() == QScroller::Pressed) {
               int pressDelay = int(1000 * scroller->scrollerProperties().scrollMetric(QScrollerProperties::MousePressEventDelay).toReal());

               if (pressDelay > 0) {
                  result |= ConsumeEventHint;

                  PressDelayHandler::instance()->pressed(event, pressDelay);
                  event->accept();
               }
            }

            [[fallthrough]];

         case QEvent::TouchBegin:
            q->setHotSpot(globalPos);
            result |= scrollerIsActive ? TriggerGesture : MayBeGesture;
            break;

         case QEvent::MouseMove:
#ifndef QT_NO_GRAPHICSVIEW
         case QEvent::GraphicsSceneMouseMove:
#endif
            if (PressDelayHandler::instance()->isDelaying()) {
               result |= ConsumeEventHint;
            }

            [[fallthrough]];

         case QEvent::TouchUpdate:
            result |= scrollerIsActive ? TriggerGesture : Ignore;
            break;

#ifndef QT_NO_GRAPHICSVIEW

         case QEvent::GraphicsSceneMouseRelease:
#endif
         case QEvent::MouseButtonRelease:
            if (PressDelayHandler::instance()->released(event, scrollerWasDragging || scrollerWasScrolling, scrollerIsActive)) {
               result |= ConsumeEventHint;
            }

            [[fallthrough]];

         case QEvent::TouchEnd:
            result |= scrollerIsActive ? FinishGesture : CancelGesture;
            break;

         default:
            result |= Ignore;
            break;
      }
   }

   return result;
}

void QFlickGestureRecognizer::reset(QGesture *state)
{
   QGestureRecognizer::reset(state);
}

#endif // QT_NO_GESTURES
