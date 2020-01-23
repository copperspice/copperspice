/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qglobal.h>

#ifndef QT_NO_GRAPHICSVIEW

#include <qgraphicslayout.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicsproxywidget_p.h>
#include <qwidget_p.h>
#include <qapplication_p.h>

#include <QtCore/qdebug.h>
#include <QtGui/qevent.h>
#include <QtGui/qgraphicsscene.h>
#include <QtGui/qgraphicssceneevent.h>
#include <QtGui/qlayout.h>
#include <QtGui/qpainter.h>
#include <QtGui/qstyleoption.h>
#include <QtGui/qgraphicsview.h>
#include <QtGui/qlistview.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qtextedit.h>


//#define GRAPHICSPROXYWIDGET_DEBUG

extern bool qt_sendSpontaneousEvent(QObject *, QEvent *);
Q_GUI_EXPORT extern bool qt_tab_all_widgets();

/*!
    \internal
*/
void QGraphicsProxyWidgetPrivate::init()
{
   Q_Q(QGraphicsProxyWidget);
   q->setFocusPolicy(Qt::WheelFocus);
   q->setAcceptDrops(true);
}

/*!
    \internal
*/
void QGraphicsProxyWidgetPrivate::sendWidgetMouseEvent(QGraphicsSceneHoverEvent *event)
{
   QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
   mouseEvent.setPos(event->pos());
   mouseEvent.setScreenPos(event->screenPos());
   mouseEvent.setButton(Qt::NoButton);
   mouseEvent.setButtons(0);
   mouseEvent.setModifiers(event->modifiers());
   sendWidgetMouseEvent(&mouseEvent);
   event->setAccepted(mouseEvent.isAccepted());
}

/*!
    \internal
*/
void QGraphicsProxyWidgetPrivate::sendWidgetMouseEvent(QGraphicsSceneMouseEvent *event)
{
   if (!event || !widget || !widget->isVisible()) {
      return;
   }
   Q_Q(QGraphicsProxyWidget);

   // Find widget position and receiver.
   QPointF pos = event->pos();
   QPointer<QWidget> alienWidget = widget->childAt(pos.toPoint());
   QPointer<QWidget> receiver =  alienWidget ? alienWidget : widget;

   if (QWidgetPrivate::nearestGraphicsProxyWidget(receiver) != q) {
      return;   //another proxywidget will handle the events
   }

   // Translate QGraphicsSceneMouse events to QMouseEvents.
   QEvent::Type type = QEvent::None;
   switch (event->type()) {
      case QEvent::GraphicsSceneMousePress:
         type = QEvent::MouseButtonPress;
         if (!embeddedMouseGrabber) {
            embeddedMouseGrabber = receiver;
         } else {
            receiver = embeddedMouseGrabber;
         }
         break;
      case QEvent::GraphicsSceneMouseRelease:
         type = QEvent::MouseButtonRelease;
         if (embeddedMouseGrabber) {
            receiver = embeddedMouseGrabber;
         }
         break;
      case QEvent::GraphicsSceneMouseDoubleClick:
         type = QEvent::MouseButtonDblClick;
         if (!embeddedMouseGrabber) {
            embeddedMouseGrabber = receiver;
         } else {
            receiver = embeddedMouseGrabber;
         }
         break;
      case QEvent::GraphicsSceneMouseMove:
         type = QEvent::MouseMove;
         if (embeddedMouseGrabber) {
            receiver = embeddedMouseGrabber;
         }
         break;
      default:
         Q_ASSERT_X(false, "QGraphicsProxyWidget", "internal error");
         break;
   }

   if (!lastWidgetUnderMouse) {
      QApplicationPrivate::dispatchEnterLeave(embeddedMouseGrabber ? embeddedMouseGrabber : receiver, 0, event->screenPos());
      lastWidgetUnderMouse = receiver;
   }

   // Map event position from us to the receiver
   pos = mapToReceiver(pos, receiver);

   // Send mouse event.
   QMouseEvent mouseEvent(type, pos, receiver->mapTo(receiver->topLevelWidget(), pos.toPoint()),
      receiver->mapToGlobal(pos.toPoint()),
      event->button(), event->buttons(), event->modifiers(), event->source());

   QWidget *embeddedMouseGrabberPtr = (QWidget *)embeddedMouseGrabber;
   QApplicationPrivate::sendMouseEvent(receiver, &mouseEvent, alienWidget, widget,
      &embeddedMouseGrabberPtr, lastWidgetUnderMouse, event->spontaneous());
   embeddedMouseGrabber = embeddedMouseGrabberPtr;

   // Handle enter/leave events when last button is released from mouse
   // grabber child widget.
   if (embeddedMouseGrabber && type == QEvent::MouseButtonRelease && !event->buttons()) {
      Q_Q(QGraphicsProxyWidget);
      if (q->rect().contains(event->pos()) && q->acceptHoverEvents()) {
         lastWidgetUnderMouse = alienWidget ? alienWidget : widget;
      } else { // released on the frame our outside the item, or doesn't accept hover events.
         lastWidgetUnderMouse = 0;
      }

      QApplicationPrivate::dispatchEnterLeave(lastWidgetUnderMouse, embeddedMouseGrabber, event->screenPos());
      embeddedMouseGrabber = 0;

#ifndef QT_NO_CURSOR
      // ### Restore the cursor, don't override it.
      if (!lastWidgetUnderMouse) {
         q->unsetCursor();
      }
#endif
   }

   event->setAccepted(mouseEvent.isAccepted());
}

void QGraphicsProxyWidgetPrivate::sendWidgetKeyEvent(QKeyEvent *event)
{
   Q_Q(QGraphicsProxyWidget);
   if (!event || !widget || !widget->isVisible()) {
      return;
   }

   QPointer<QWidget> receiver = widget->focusWidget();
   if (!receiver) {
      receiver = widget;
   }
   Q_ASSERT(receiver);

   do {
      bool res = QApplication::sendEvent(receiver, event);
      if ((res && event->isAccepted()) || (q->isWindow() && receiver == widget)) {
         break;
      }
      receiver = receiver->parentWidget();
   } while (receiver);
}

/*!
    \internal
*/
void QGraphicsProxyWidgetPrivate::removeSubFocusHelper(QWidget *widget, Qt::FocusReason reason)
{
   QFocusEvent event(QEvent::FocusOut, reason);
   QPointer<QWidget> widgetGuard = widget;
   QApplication::sendEvent(widget, &event);

   if (widgetGuard && event.isAccepted()) {
      QApplication::sendEvent(widget->style(), &event);
   }
}

/*!
    \internal
    Some of the logic is shared with QApplicationPrivate::focusNextPrevChild_helper
*/
QWidget *QGraphicsProxyWidgetPrivate::findFocusChild(QWidget *child, bool next) const
{
   if (!widget) {
      return 0;
   }

   // Run around the focus chain until we find a widget that can take tab focus.
   if (!child) {
      child = next ? (QWidget *)widget : widget->d_func()->focus_prev;
   } else {
      child = next ? child->d_func()->focus_next : child->d_func()->focus_prev;
      if ((next && child == widget) || (!next && child == widget->d_func()->focus_prev)) {
         return 0;
      }
   }

   if (!child) {
      return 0;
   }

   QWidget *oldChild = child;
   uint focus_flag = qt_tab_all_widgets() ? Qt::TabFocus : Qt::StrongFocus;

   do {
      if (child->isEnabled()
         && child->isVisibleTo(widget)
         && ((child->focusPolicy() & focus_flag) == focus_flag)
         && !(child->d_func()->extra && child->d_func()->extra->focus_proxy)) {
         return child;
      }
      child = next ? child->d_func()->focus_next : child->d_func()->focus_prev;
   } while (child != oldChild && !(next && child == widget) && !(!next && child == widget->d_func()->focus_prev));
   return 0;
}

/*!
    \internal
*/
void QGraphicsProxyWidgetPrivate::_q_removeWidgetSlot()
{
   Q_Q(QGraphicsProxyWidget);
   if (!widget.isNull()) {
      if (QWExtra *extra = widget->d_func()->extra) {
         extra->proxyWidget = 0;
      }
   }
   widget = 0;
   delete q;
}

/*!
    \internal
*/
void QGraphicsProxyWidgetPrivate::updateWidgetGeometryFromProxy()
{
}

/*!
    \internal
*/
void QGraphicsProxyWidgetPrivate::updateProxyGeometryFromWidget()
{
   Q_Q(QGraphicsProxyWidget);
   if (!widget) {
      return;
   }

   QRectF widgetGeometry = widget->geometry();
   QWidget *parentWidget = widget->parentWidget();
   if (widget->isWindow()) {
      QGraphicsProxyWidget *proxyParent = 0;
      if (parentWidget && (proxyParent = qobject_cast<QGraphicsProxyWidget *>(q->parentWidget()))) {
         // Nested window proxy (e.g., combobox popup), map widget to the
         // parent widget's global coordinates, and map that to the parent
         // proxy's child coordinates.
         widgetGeometry.moveTo(proxyParent->subWidgetRect(parentWidget).topLeft()
            + parentWidget->mapFromGlobal(widget->pos()));
      }
   }

   // Adjust to size hint if the widget has never been resized.
   if (!widget->size().isValid()) {
      widgetGeometry.setSize(widget->sizeHint());
   }

   // Assign new geometry.
   posChangeMode = QGraphicsProxyWidgetPrivate::WidgetToProxyMode;
   sizeChangeMode = QGraphicsProxyWidgetPrivate::WidgetToProxyMode;
   q->setGeometry(widgetGeometry);
   posChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
   sizeChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
}

/*!
    \internal
*/
void QGraphicsProxyWidgetPrivate::updateProxyInputMethodAcceptanceFromWidget()
{
   Q_Q(QGraphicsProxyWidget);
   if (!widget) {
      return;
   }

   QWidget *focusWidget = widget->focusWidget();
   if (!focusWidget) {
      focusWidget = widget;
   }
   q->setFlag(QGraphicsItem::ItemAcceptsInputMethod,
      focusWidget->testAttribute(Qt::WA_InputMethodEnabled));
}

/*!
    \internal

    Embeds \a subWin as a subwindow of this proxy widget. \a subWin must be a top-level
    widget and a descendant of the widget managed by this proxy. A separate subproxy
    will be created as a child of this proxy widget to manage \a subWin.
*/
void QGraphicsProxyWidgetPrivate::embedSubWindow(QWidget *subWin)
{
   QWExtra *extra;
   if (!((extra = subWin->d_func()->extra) && extra->proxyWidget)) {
      QGraphicsProxyWidget *subProxy = new QGraphicsProxyWidget(q_func(), subWin->windowFlags());
      subProxy->d_func()->setWidget_helper(subWin, false);
   }
}

/*!
    \internal

    Removes ("unembeds") \a subWin and deletes the proxy holder item. This can
    happen when QWidget::setParent() reparents the embedded window out of
    "embedded space".
*/
void QGraphicsProxyWidgetPrivate::unembedSubWindow(QWidget *subWin)
{
   for (QGraphicsItem *child : children) {
      if (child->isWidget()) {
         if (QGraphicsProxyWidget *proxy = qobject_cast<QGraphicsProxyWidget *>(static_cast<QGraphicsWidget *>(child))) {
            if (proxy->widget() == subWin) {
               proxy->setWidget(0);
               scene->removeItem(proxy);
               delete proxy;
               return;
            }
         }
      }
   }
}

bool QGraphicsProxyWidgetPrivate::isProxyWidget() const
{
   return true;
}

/*!
     \internal
*/
QPointF QGraphicsProxyWidgetPrivate::mapToReceiver(const QPointF &pos, const QWidget *receiver) const
{
   QPointF p = pos;
   // Map event position from us to the receiver, preserving its
   // precision (don't use QWidget::mapFrom here).
   while (receiver && receiver != widget) {
      p -= QPointF(receiver->pos());
      receiver = receiver->parentWidget();
   }
   return p;
}

/*!
    Constructs a new QGraphicsProxy widget. \a parent and \a wFlags are passed
    to QGraphicsItem's constructor.
*/
QGraphicsProxyWidget::QGraphicsProxyWidget(QGraphicsItem *parent, Qt::WindowFlags wFlags)
   : QGraphicsWidget(*new QGraphicsProxyWidgetPrivate, parent, wFlags)
{
   Q_D(QGraphicsProxyWidget);
   d->init();
}

/*!
    Destroys the proxy widget and any embedded widget.
*/
QGraphicsProxyWidget::~QGraphicsProxyWidget()
{
   Q_D(QGraphicsProxyWidget);

   if (d->widget) {
      d->widget->removeEventFilter(this);
      QObject::disconnect(d->widget, SIGNAL(destroyed()), this, SLOT(_q_removeWidgetSlot()));
      delete d->widget;
   }
}


void QGraphicsProxyWidget::setWidget(QWidget *widget)
{
   Q_D(QGraphicsProxyWidget);
   d->setWidget_helper(widget, true);
}

void QGraphicsProxyWidgetPrivate::setWidget_helper(QWidget *newWidget, bool autoShow)
{
   Q_Q(QGraphicsProxyWidget);
   if (newWidget == widget) {
      return;
   }
   if (widget) {
      QObject::disconnect(widget, SIGNAL(destroyed()), q, SLOT(_q_removeWidgetSlot()));
      widget->removeEventFilter(q);
      widget->setAttribute(Qt::WA_DontShowOnScreen, false);
      widget->d_func()->extra->proxyWidget = 0;
      resolveFont(inheritedFontResolveMask);
      resolvePalette(inheritedPaletteResolveMask);
      widget->update();

      for (QGraphicsItem *child : q->childItems()) {
         if (child->d_ptr->isProxyWidget()) {
            QGraphicsProxyWidget *childProxy = static_cast<QGraphicsProxyWidget *>(child);
            QWidget *parent = childProxy->widget();
            while (parent->parentWidget() != 0) {
               if (parent == widget) {
                  break;
               }
               parent = parent->parentWidget();
            }
            if (!childProxy->widget() || parent != widget) {
               continue;
            }
            childProxy->setWidget(0);
            delete childProxy;
         }
      }

      widget = 0;
#ifndef QT_NO_CURSOR
      q->unsetCursor();
#endif
      q->setAcceptHoverEvents(false);
      if (!newWidget) {
         q->update();
      }
   }
   if (! newWidget) {
      return;
   }
   if (!newWidget->isWindow()) {
      QWExtra *extra = newWidget->parentWidget()->d_func()->extra;
      if (!extra || !extra->proxyWidget)  {
         qWarning("QGraphicsProxyWidget::setWidget: cannot embed widget %p "
            "which is not a toplevel widget, and is not a child of an embedded widget", newWidget);
         return;
      }
   }

   // Register this proxy within the widget's private.
   // ### This is a bit backdoorish
   QWExtra *extra = newWidget->d_func()->extra;
   if (!extra) {
      newWidget->d_func()->createExtra();
      extra = newWidget->d_func()->extra;
   }
   QGraphicsProxyWidget **proxyWidget = &extra->proxyWidget;
   if (*proxyWidget) {
      if (*proxyWidget != q) {
         qWarning("QGraphicsProxyWidget::setWidget: cannot embed widget %p"
            "; already embedded", newWidget);
      }
      return;
   }
   *proxyWidget = q;

   newWidget->setAttribute(Qt::WA_DontShowOnScreen);
   newWidget->ensurePolished();
   // Do not wait for this widget to close before the app closes ###
   // shouldn't this widget inherit the attribute?
   newWidget->setAttribute(Qt::WA_QuitOnClose, false);
   q->setAcceptHoverEvents(true);

   if (newWidget->testAttribute(Qt::WA_NoSystemBackground)) {
      q->setAttribute(Qt::WA_NoSystemBackground);
   }
   if (newWidget->testAttribute(Qt::WA_OpaquePaintEvent)) {
      q->setAttribute(Qt::WA_OpaquePaintEvent);
   }

   widget = newWidget;

   // Changes only go from the widget to the proxy.
   enabledChangeMode = QGraphicsProxyWidgetPrivate::WidgetToProxyMode;
   visibleChangeMode = QGraphicsProxyWidgetPrivate::WidgetToProxyMode;
   posChangeMode = QGraphicsProxyWidgetPrivate::WidgetToProxyMode;
   sizeChangeMode = QGraphicsProxyWidgetPrivate::WidgetToProxyMode;

   if ((autoShow && !newWidget->testAttribute(Qt::WA_WState_ExplicitShowHide)) ||
      ! newWidget->testAttribute(Qt::WA_WState_Hidden)) {
      newWidget->show();
   }

   // Copy the state from the widget onto the proxy.
#ifndef QT_NO_CURSOR
   if (newWidget->testAttribute(Qt::WA_SetCursor)) {
      q->setCursor(widget->cursor());
   }
#endif
   q->setEnabled(newWidget->isEnabled());
   q->setVisible(newWidget->isVisible());
   q->setLayoutDirection(newWidget->layoutDirection());
   if (newWidget->testAttribute(Qt::WA_SetStyle)) {
      q->setStyle(widget->style());
   }

   resolveFont(inheritedFontResolveMask);
   resolvePalette(inheritedPaletteResolveMask);

   if (!newWidget->testAttribute(Qt::WA_Resized)) {
      newWidget->adjustSize();
   }

   int left, top, right, bottom;
   newWidget->getContentsMargins(&left, &top, &right, &bottom);
   q->setContentsMargins(left, top, right, bottom);
   q->setWindowTitle(newWidget->windowTitle());

   // size policies and constraints..
   q->setSizePolicy(newWidget->sizePolicy());
   QSize sz = newWidget->minimumSize();
   q->setMinimumSize(sz.isNull() ? QSizeF() : QSizeF(sz));
   sz = newWidget->maximumSize();
   q->setMaximumSize(sz.isNull() ? QSizeF() : QSizeF(sz));

   updateProxyGeometryFromWidget();

   updateProxyInputMethodAcceptanceFromWidget();

   // Hook up the event filter to keep the state up to date.
   newWidget->installEventFilter(q);
   QObject::connect(newWidget, SIGNAL(destroyed()), q, SLOT(_q_removeWidgetSlot()));

   // Changes no longer go only from the widget to the proxy.
   enabledChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
   visibleChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
   posChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
   sizeChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
}

/*!
    Returns a pointer to the embedded widget.

    \sa setWidget()
*/
QWidget *QGraphicsProxyWidget::widget() const
{
   Q_D(const QGraphicsProxyWidget);
   return d->widget;
}

/*!
    Returns the rectangle for \a widget, which must be a descendant of
    widget(), or widget() itself, in this proxy item's local coordinates.

    If no widget is embedded, \a widget is 0, or \a widget is not a
    descendant of the embedded widget, this function returns an empty QRectF.

    \sa widget()
*/
QRectF QGraphicsProxyWidget::subWidgetRect(const QWidget *widget) const
{
   Q_D(const QGraphicsProxyWidget);
   if (!widget || !d->widget) {
      return QRectF();
   }
   if (d->widget == widget || d->widget->isAncestorOf(widget)) {
      return QRectF(widget->mapTo(d->widget, QPoint(0, 0)), widget->size());
   }
   return QRectF();
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::setGeometry(const QRectF &rect)
{
   Q_D(QGraphicsProxyWidget);
   bool proxyResizesWidget = !d->posChangeMode && !d->sizeChangeMode;
   if (proxyResizesWidget) {
      d->posChangeMode = QGraphicsProxyWidgetPrivate::ProxyToWidgetMode;
      d->sizeChangeMode = QGraphicsProxyWidgetPrivate::ProxyToWidgetMode;
   }
   QGraphicsWidget::setGeometry(rect);
   if (proxyResizesWidget) {
      d->posChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
      d->sizeChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
   }
}

/*!
    \reimp
*/
QVariant QGraphicsProxyWidget::itemChange(GraphicsItemChange change,
   const QVariant &value)
{
   Q_D(QGraphicsProxyWidget);

   switch (change) {
      case ItemPositionChange:
         // The item's position is either changed directly on the proxy, in
         // which case the position change should propagate to the widget,
         // otherwise it happens as a side effect when filtering QEvent::Move.
         if (!d->posChangeMode) {
            d->posChangeMode = QGraphicsProxyWidgetPrivate::ProxyToWidgetMode;
         }
         break;
      case ItemPositionHasChanged:
         // Move the internal widget if we're in widget-to-proxy
         // mode. Otherwise the widget has already moved.
         if (d->widget && d->posChangeMode != QGraphicsProxyWidgetPrivate::WidgetToProxyMode) {
            d->widget->move(value.toPoint());
         }
         if (d->posChangeMode == QGraphicsProxyWidgetPrivate::ProxyToWidgetMode) {
            d->posChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
         }
         break;
      case ItemVisibleChange:
         if (!d->visibleChangeMode) {
            d->visibleChangeMode = QGraphicsProxyWidgetPrivate::ProxyToWidgetMode;
         }
         break;
      case ItemVisibleHasChanged:
         if (d->widget && d->visibleChangeMode != QGraphicsProxyWidgetPrivate::WidgetToProxyMode) {
            d->widget->setVisible(isVisible());
         }
         if (d->visibleChangeMode == QGraphicsProxyWidgetPrivate::ProxyToWidgetMode) {
            d->visibleChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
         }
         break;
      case ItemEnabledChange:
         if (!d->enabledChangeMode) {
            d->enabledChangeMode = QGraphicsProxyWidgetPrivate::ProxyToWidgetMode;
         }
         break;
      case ItemEnabledHasChanged:
         if (d->widget && d->enabledChangeMode != QGraphicsProxyWidgetPrivate::WidgetToProxyMode) {
            d->widget->setEnabled(isEnabled());
         }
         if (d->enabledChangeMode == QGraphicsProxyWidgetPrivate::ProxyToWidgetMode) {
            d->enabledChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
         }
         break;
      default:
         break;
   }
   return QGraphicsWidget::itemChange(change, value);
}

/*!
    \reimp
*/
bool QGraphicsProxyWidget::event(QEvent *event)
{
   Q_D(QGraphicsProxyWidget);
   if (!d->widget) {
      return QGraphicsWidget::event(event);
   }

   switch (event->type()) {
      case QEvent::StyleChange:
         // Propagate style changes to the embedded widget.
         if (!d->styleChangeMode) {
            d->styleChangeMode = QGraphicsProxyWidgetPrivate::ProxyToWidgetMode;
            d->widget->setStyle(style());
            d->styleChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
         }
         break;
      case QEvent::FontChange: {
         // Propagate to widget.
         QWidgetPrivate *wd = d->widget->d_func();
         int mask = d->font.resolve() | d->inheritedFontResolveMask;
         wd->inheritedFontResolveMask = mask;
         wd->resolveFont();
         break;
      }
      case QEvent::PaletteChange: {
         // Propagate to widget.
         QWidgetPrivate *wd = d->widget->d_func();
         int mask = d->palette.resolve() | d->inheritedPaletteResolveMask;
         wd->inheritedPaletteResolveMask = mask;
         wd->resolvePalette();
         break;
      }

      case QEvent::InputMethod: {
         inputMethodEvent(static_cast<QInputMethodEvent *>(event));

         if (event->isAccepted()) {
            return true;
         }

         return false;
      }

      case QEvent::ShortcutOverride: {
         QWidget *focusWidget = d->widget->focusWidget();

         while (focusWidget) {
            QApplication::sendEvent(focusWidget, event);

            if (event->isAccepted()) {
               return true;
            }

            focusWidget = focusWidget->parentWidget();
         }

         return false;
      }

      case QEvent::KeyPress: {
         QKeyEvent *k = static_cast<QKeyEvent *>(event);
         if (k->key() == Qt::Key_Tab || k->key() == Qt::Key_Backtab) {
            if (!(k->modifiers() & (Qt::ControlModifier | Qt::AltModifier))) {  //### Add MetaModifier?
               QWidget *focusWidget = d->widget->focusWidget();
               while (focusWidget) {
                  bool res = QApplication::sendEvent(focusWidget, event);
                  if ((res && event->isAccepted()) || (isWindow() && focusWidget == d->widget)) {
                     event->accept();
                     break;
                  }
                  focusWidget = focusWidget->parentWidget();
               }
               return true;
            }
         }
         break;
      }
#ifndef QT_NO_TOOLTIP
      case QEvent::GraphicsSceneHelp: {
         // Propagate the help event (for tooltip) to the widget under mouse
         if (d->lastWidgetUnderMouse) {
            QGraphicsSceneHelpEvent *he = static_cast<QGraphicsSceneHelpEvent *>(event);
            QPoint pos = d->mapToReceiver(mapFromScene(he->scenePos()), d->lastWidgetUnderMouse).toPoint();
            QHelpEvent e(QEvent::ToolTip, pos, he->screenPos());
            QApplication::sendEvent(d->lastWidgetUnderMouse, &e);
            event->setAccepted(e.isAccepted());
            return e.isAccepted();
         }
         break;
      }
      case QEvent::ToolTipChange: {
         // Propagate tooltip change to the widget
         if (!d->tooltipChangeMode) {
            d->tooltipChangeMode = QGraphicsProxyWidgetPrivate::ProxyToWidgetMode;
            d->widget->setToolTip(toolTip());
            d->tooltipChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
         }
         break;
      }
#endif

      case QEvent::TouchBegin:
      case QEvent::TouchUpdate:
      case QEvent::TouchEnd: {
         if (event->spontaneous()) {
            qt_sendSpontaneousEvent(d->widget, event);
         } else {
            QApplication::sendEvent(d->widget, event);
         }

         if (event->isAccepted()) {
            return true;
         }

         break;
      }
      default:
         break;
   }
   return QGraphicsWidget::event(event);
}

/*!
    \reimp
*/
bool QGraphicsProxyWidget::eventFilter(QObject *object, QEvent *event)
{
   Q_D(QGraphicsProxyWidget);

   if (object == d->widget) {
      switch (event->type()) {
         case QEvent::LayoutRequest:
            updateGeometry();
            break;
         case QEvent::Resize:
            // If the widget resizes itself, we resize the proxy too.
            // Prevent feed-back by checking the geometry change mode.
            if (!d->sizeChangeMode) {
               d->updateProxyGeometryFromWidget();
            }
            break;
         case QEvent::Move:
            // If the widget moves itself, we move the proxy too.  Prevent
            // feed-back by checking the geometry change mode.
            if (!d->posChangeMode) {
               d->updateProxyGeometryFromWidget();
            }
            break;
         case QEvent::Hide:
         case QEvent::Show:
            // If the widget toggles its visible state, the proxy will follow.
            if (!d->visibleChangeMode) {
               d->visibleChangeMode = QGraphicsProxyWidgetPrivate::WidgetToProxyMode;
               setVisible(event->type() == QEvent::Show);
               d->visibleChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
            }
            break;
         case QEvent::EnabledChange:
            // If the widget toggles its enabled state, the proxy will follow.
            if (!d->enabledChangeMode) {
               d->enabledChangeMode = QGraphicsProxyWidgetPrivate::WidgetToProxyMode;
               setEnabled(d->widget->isEnabled());
               d->enabledChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
            }
            break;
         case QEvent::StyleChange:
            // Propagate style changes to the proxy.
            if (!d->styleChangeMode) {
               d->styleChangeMode = QGraphicsProxyWidgetPrivate::WidgetToProxyMode;
               setStyle(d->widget->style());
               d->styleChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
            }
            break;
#ifndef QT_NO_TOOLTIP
         case QEvent::ToolTipChange:
            // Propagate tooltip change to the proxy.
            if (!d->tooltipChangeMode) {
               d->tooltipChangeMode = QGraphicsProxyWidgetPrivate::WidgetToProxyMode;
               setToolTip(d->widget->toolTip());
               d->tooltipChangeMode = QGraphicsProxyWidgetPrivate::NoMode;
            }
            break;
#endif
         default:
            break;
      }
   }
   return QGraphicsWidget::eventFilter(object, event);
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::showEvent(QShowEvent *event)
{
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::hideEvent(QHideEvent *event)
{

}

#ifndef QT_NO_CONTEXTMENU
/*!
    \reimp
*/
void QGraphicsProxyWidget::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
   Q_D(QGraphicsProxyWidget);
   if (!event || !d->widget || !d->widget->isVisible() || !hasFocus()) {
      return;
   }

   // Find widget position and receiver.
   QPointF pos = event->pos();
   QPointer<QWidget> alienWidget = d->widget->childAt(pos.toPoint());
   QPointer<QWidget> receiver =  alienWidget ? alienWidget : d->widget;

   // Map event position from us to the receiver
   pos = d->mapToReceiver(pos, receiver);

   QPoint globalPos = receiver->mapToGlobal(pos.toPoint());
   //If the receiver by-pass the proxy its popups
   //will be top level QWidgets therefore they need
   //the screen position. mapToGlobal expect the widget to
   //have proper coordinates in regards of the windowing system
   //but it's not true because the widget is embedded.
   if (bypassGraphicsProxyWidget(receiver)) {
      globalPos = event->screenPos();
   }

   // Send mouse event. ### Doesn't propagate the event.
   QContextMenuEvent contextMenuEvent(QContextMenuEvent::Reason(event->reason()),
      pos.toPoint(), globalPos, event->modifiers());
   QApplication::sendEvent(receiver, &contextMenuEvent);

   event->setAccepted(contextMenuEvent.isAccepted());
}
#endif // QT_NO_CONTEXTMENU

#ifndef QT_NO_DRAGANDDROP
/*!
    \reimp
*/
void QGraphicsProxyWidget::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
#ifdef QT_NO_DRAGANDDROP
   Q_UNUSED(event);
#else
   Q_D(QGraphicsProxyWidget);
   if (!d->widget) {
      return;
   }

   QDragEnterEvent proxyDragEnter(event->pos().toPoint(), event->dropAction(), event->mimeData(), event->buttons(),
      event->modifiers());
   proxyDragEnter.setAccepted(event->isAccepted());
   QApplication::sendEvent(d->widget, &proxyDragEnter);
   event->setAccepted(proxyDragEnter.isAccepted());
   if (proxyDragEnter.isAccepted()) {  // we discard answerRect
      event->setDropAction(proxyDragEnter.dropAction());
   }
#endif
}
/*!
    \reimp
*/
void QGraphicsProxyWidget::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
   Q_UNUSED(event);
#ifndef QT_NO_DRAGANDDROP
   Q_D(QGraphicsProxyWidget);
   if (!d->widget || !d->dragDropWidget) {
      return;
   }
   QDragLeaveEvent proxyDragLeave;
   QApplication::sendEvent(d->dragDropWidget, &proxyDragLeave);
   d->dragDropWidget = 0;
#endif
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
#ifdef QT_NO_DRAGANDDROP
   Q_UNUSED(event);
#else
   Q_D(QGraphicsProxyWidget);
   if (!d->widget) {
      return;
   }
   QPointF p = event->pos();
   event->ignore();
   QPointer<QWidget> subWidget = d->widget->childAt(p.toPoint());
   QPointer<QWidget> receiver =  subWidget ? subWidget : d->widget;
   bool eventDelivered = false;
   for (; receiver; receiver = receiver->parentWidget()) {
      if (!receiver->isEnabled() || !receiver->acceptDrops()) {
         continue;
      }
      // Map event position from us to the receiver
      QPoint receiverPos = d->mapToReceiver(p, receiver).toPoint();
      if (receiver != d->dragDropWidget) {
         // Try to enter before we leave
         QDragEnterEvent dragEnter(receiverPos, event->possibleActions(), event->mimeData(), event->buttons(),
            event->modifiers());
         dragEnter.setDropAction(event->proposedAction());
         QApplication::sendEvent(receiver, &dragEnter);
         event->setAccepted(dragEnter.isAccepted());
         event->setDropAction(dragEnter.dropAction());
         if (!event->isAccepted()) {
            // propagate to the parent widget
            continue;
         }

         d->lastDropAction = event->dropAction();

         if (d->dragDropWidget) {
            QDragLeaveEvent dragLeave;
            QApplication::sendEvent(d->dragDropWidget, &dragLeave);
         }
         d->dragDropWidget = receiver;
      }

      QDragMoveEvent dragMove(receiverPos, event->possibleActions(), event->mimeData(), event->buttons(), event->modifiers());
      event->setDropAction(d->lastDropAction);
      QApplication::sendEvent(receiver, &dragMove);
      event->setAccepted(dragMove.isAccepted());
      event->setDropAction(dragMove.dropAction());
      if (event->isAccepted()) {
         d->lastDropAction = event->dropAction();
      }
      eventDelivered = true;
      break;
   }

   if (!eventDelivered) {
      if (d->dragDropWidget) {
         // Leave the last drag drop item
         QDragLeaveEvent dragLeave;
         QApplication::sendEvent(d->dragDropWidget, &dragLeave);
         d->dragDropWidget = 0;
      }
      // Propagate
      event->setDropAction(Qt::IgnoreAction);
   }
#endif
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::dropEvent(QGraphicsSceneDragDropEvent *event)
{
#ifdef QT_NO_DRAGANDDROP
   Q_UNUSED(event);
#else
   Q_D(QGraphicsProxyWidget);
   if (d->widget && d->dragDropWidget) {
      QPoint widgetPos = d->mapToReceiver(event->pos(), d->dragDropWidget).toPoint();
      QDropEvent dropEvent(widgetPos, event->possibleActions(), event->mimeData(), event->buttons(), event->modifiers());
      QApplication::sendEvent(d->dragDropWidget, &dropEvent);
      event->setAccepted(dropEvent.isAccepted());
      d->dragDropWidget = 0;
   }
#endif
}
#endif

/*!
    \reimp
*/
void QGraphicsProxyWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
   Q_UNUSED(event);
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
   Q_D(QGraphicsProxyWidget);

   // If hoverMove was compressed away, make sure we update properly here.
   if (d->lastWidgetUnderMouse) {
      QApplicationPrivate::dispatchEnterLeave(0, d->lastWidgetUnderMouse, event->screenPos());
      d->lastWidgetUnderMouse = 0;
   }
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
   Q_D(QGraphicsProxyWidget);
#ifdef GRAPHICSPROXYWIDGET_DEBUG
   qDebug("QGraphicsProxyWidget::hoverMoveEvent");
#endif
   // Ignore events on the window frame.
   if (!d->widget || !rect().contains(event->pos())) {
      if (d->lastWidgetUnderMouse) {
         QApplicationPrivate::dispatchEnterLeave(0, d->lastWidgetUnderMouse, event->screenPos());
         d->lastWidgetUnderMouse = 0;
      }
      return;
   }

   d->embeddedMouseGrabber = 0;
   d->sendWidgetMouseEvent(event);
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::grabMouseEvent(QEvent *event)
{
   Q_UNUSED(event);
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::ungrabMouseEvent(QEvent *event)
{
   Q_D(QGraphicsProxyWidget);

   d->embeddedMouseGrabber = 0;
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QGraphicsProxyWidget);
#ifdef GRAPHICSPROXYWIDGET_DEBUG
   qDebug("QGraphicsProxyWidget::mouseMoveEvent");
#endif
   d->sendWidgetMouseEvent(event);
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QGraphicsProxyWidget);

#ifdef GRAPHICSPROXYWIDGET_DEBUG
   qDebug("QGraphicsProxyWidget::mousePressEvent");
#endif

   d->sendWidgetMouseEvent(event);
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QGraphicsProxyWidget);
#ifdef GRAPHICSPROXYWIDGET_DEBUG
   qDebug("QGraphicsProxyWidget::mouseDoubleClickEvent");
#endif
   d->sendWidgetMouseEvent(event);
}

/*!
    \reimp
*/
#ifndef QT_NO_WHEELEVENT
void QGraphicsProxyWidget::wheelEvent(QGraphicsSceneWheelEvent *event)
{
   Q_D(QGraphicsProxyWidget);
#ifdef GRAPHICSPROXYWIDGET_DEBUG
   qDebug("QGraphicsProxyWidget::wheelEvent");
#endif
   if (!d->widget) {
      return;
   }

   QPointF pos = event->pos();
   QPointer<QWidget> receiver = d->widget->childAt(pos.toPoint());
   if (!receiver) {
      receiver = d->widget;
   }

   // Map event position from us to the receiver
   pos = d->mapToReceiver(pos, receiver);

   // Send mouse event.
   QWheelEvent wheelEvent(pos.toPoint(), event->screenPos(), event->delta(),
      event->buttons(), event->modifiers(), event->orientation());
   QPointer<QWidget> focusWidget = d->widget->focusWidget();
   extern bool qt_sendSpontaneousEvent(QObject *, QEvent *);
   qt_sendSpontaneousEvent(receiver, &wheelEvent);
   event->setAccepted(wheelEvent.isAccepted());

   // ### Remove, this should be done by proper focusIn/focusOut events.
   if (focusWidget && !focusWidget->hasFocus()) {
      focusWidget->update();
      focusWidget = d->widget->focusWidget();

      if (focusWidget && focusWidget->hasFocus()) {
         focusWidget->update();
      }
   }
}
#endif

/*!
    \reimp
*/
void QGraphicsProxyWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QGraphicsProxyWidget);
#ifdef GRAPHICSPROXYWIDGET_DEBUG
   qDebug("QGraphicsProxyWidget::mouseReleaseEvent");
#endif
   d->sendWidgetMouseEvent(event);
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::keyPressEvent(QKeyEvent *event)
{
   Q_D(QGraphicsProxyWidget);
#ifdef GRAPHICSPROXYWIDGET_DEBUG
   qDebug("QGraphicsProxyWidget::keyPressEvent");
#endif
   d->sendWidgetKeyEvent(event);
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::keyReleaseEvent(QKeyEvent *event)
{
   Q_D(QGraphicsProxyWidget);
#ifdef GRAPHICSPROXYWIDGET_DEBUG
   qDebug("QGraphicsProxyWidget::keyReleaseEvent");
#endif
   d->sendWidgetKeyEvent(event);
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::focusInEvent(QFocusEvent *event)
{
#ifdef GRAPHICSPROXYWIDGET_DEBUG
   qDebug("QGraphicsProxyWidget::focusInEvent");
#endif
   Q_D(QGraphicsProxyWidget);

   if (d->focusFromWidgetToProxy) {
      // Prevent recursion when the proxy autogains focus through the
      // embedded widget calling setFocus(). ### Could be done with event
      // filter on FocusIn instead?
      return;
   }

   d->proxyIsGivingFocus = true;

   switch (event->reason()) {
      case Qt::TabFocusReason: {
         if (QWidget *focusChild = d->findFocusChild(0, true)) {
            focusChild->setFocus(event->reason());
         }
         break;
      }
      case Qt::BacktabFocusReason:
         if (QWidget *focusChild = d->findFocusChild(0, false)) {
            focusChild->setFocus(event->reason());
         }
         break;
      default:
         if (d->widget && d->widget->focusWidget()) {
            d->widget->focusWidget()->setFocus(event->reason());
         }
         break;
   }

   d->proxyIsGivingFocus = false;
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::focusOutEvent(QFocusEvent *event)
{
#ifdef GRAPHICSPROXYWIDGET_DEBUG
   qDebug("QGraphicsProxyWidget::focusOutEvent");
#endif
   Q_D(QGraphicsProxyWidget);
   if (d->widget) {
      // We need to explicitly remove subfocus from the embedded widget's
      // focus widget.
      if (QWidget *focusWidget = d->widget->focusWidget()) {
         d->removeSubFocusHelper(focusWidget, event->reason());
      }
   }
}

/*!
    \reimp
*/
bool QGraphicsProxyWidget::focusNextPrevChild(bool next)
{
   Q_D(QGraphicsProxyWidget);

   if (!d->widget || !d->scene) {
      return QGraphicsWidget::focusNextPrevChild(next);
   }

   Qt::FocusReason reason = next ? Qt::TabFocusReason : Qt::BacktabFocusReason;
   QWidget *lastFocusChild = d->widget->focusWidget();
   if (QWidget *newFocusChild = d->findFocusChild(lastFocusChild, next)) {
      newFocusChild->setFocus(reason);
      return true;
   }

   return QGraphicsWidget::focusNextPrevChild(next);
}

/*!
    \reimp
*/
QVariant QGraphicsProxyWidget::inputMethodQuery(Qt::InputMethodQuery query) const
{
   Q_D(const QGraphicsProxyWidget);

   if (!d->widget || !hasFocus()) {
      return QVariant();
   }

   QWidget *focusWidget = widget()->focusWidget();
   if (!focusWidget) {
      focusWidget = d->widget;
   }
   QVariant v = focusWidget->inputMethodQuery(query);
   QPointF focusWidgetPos = subWidgetRect(focusWidget).topLeft();
   switch (v.type()) {
      case QVariant::RectF:
         v = v.toRectF().translated(focusWidgetPos);
         break;
      case QVariant::PointF:
         v = v.toPointF() + focusWidgetPos;
         break;
      case QVariant::Rect:
         v = v.toRect().translated(focusWidgetPos.toPoint());
         break;
      case QVariant::Point:
         v = v.toPoint() + focusWidgetPos.toPoint();
         break;
      default:
         break;
   }
   return v;
}
void QGraphicsProxyWidget::inputMethodEvent(QInputMethodEvent *event)
{
   // Forward input method events if the focus widget enables input methods.
   Q_D(const QGraphicsProxyWidget);
   QWidget *focusWidget = d->widget->focusWidget();
   if (focusWidget && focusWidget->testAttribute(Qt::WA_InputMethodEnabled)) {
      QApplication::sendEvent(focusWidget, event);
   }
}
QSizeF QGraphicsProxyWidget::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
   Q_D(const QGraphicsProxyWidget);
   if (!d->widget) {
      return QGraphicsWidget::sizeHint(which, constraint);
   }

   QSizeF sh;
   switch (which) {
      case Qt::PreferredSize:
         if (QLayout *l = d->widget->layout()) {
            sh = l->sizeHint();
         } else {
            sh = d->widget->sizeHint();
         }
         break;
      case Qt::MinimumSize:
         if (QLayout *l = d->widget->layout()) {
            sh = l->minimumSize();
         } else {
            sh = d->widget->minimumSizeHint();
         }
         break;
      case Qt::MaximumSize:
         if (QLayout *l = d->widget->layout()) {
            sh = l->maximumSize();
         } else {
            sh = QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
         }
         break;
      case Qt::MinimumDescent:
         sh = constraint;
         break;
      default:
         break;
   }
   return sh;
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
   Q_D(QGraphicsProxyWidget);
   if (d->widget) {
      if (d->sizeChangeMode != QGraphicsProxyWidgetPrivate::WidgetToProxyMode) {
         d->widget->resize(event->newSize().toSize());
      }
   }
   QGraphicsWidget::resizeEvent(event);
}

/*!
    \reimp
*/
void QGraphicsProxyWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   Q_D(QGraphicsProxyWidget);
   Q_UNUSED(widget);
   if (!d->widget || !d->widget->isVisible()) {
      return;
   }

   // Filter out repaints on the window frame.
   const QRect exposedWidgetRect = (option->exposedRect & rect()).toAlignedRect();
   if (exposedWidgetRect.isEmpty()) {
      return;
   }

   d->widget->render(painter, exposedWidgetRect.topLeft(), exposedWidgetRect);

}

/*!
    \reimp
*/
int QGraphicsProxyWidget::type() const
{
   return Type;
}

QGraphicsProxyWidget *QGraphicsProxyWidget::createProxyForChildWidget(QWidget *child)
{
   QGraphicsProxyWidget *proxy = child->graphicsProxyWidget();
   if (proxy) {
      return proxy;
   }

   if (!child->parentWidget()) {
      qWarning("QGraphicsProxyWidget::createProxyForChildWidget: top-level widget not in a QGraphicsScene");
      return 0;
   }

   QGraphicsProxyWidget *parentProxy = createProxyForChildWidget(child->parentWidget());
   if (! parentProxy) {
      return 0;
   }

   if (! QMetaObject::invokeMethod(parentProxy, "newProxyWidget",  Qt::DirectConnection,
         Q_RETURN_ARG(QGraphicsProxyWidget *, proxy), Q_ARG(const QWidget *, child)))  {
      return 0;
   }

   proxy->setParent(parentProxy);
   proxy->setWidget(child);
   return proxy;
}

QGraphicsProxyWidget *QGraphicsProxyWidget::newProxyWidget(const QWidget *)
{
   return new QGraphicsProxyWidget(this);
}

void QGraphicsProxyWidget::_q_removeWidgetSlot()
{
   Q_D(QGraphicsProxyWidget);
   d->_q_removeWidgetSlot();
}



#endif //QT_NO_GRAPHICSVIEW
