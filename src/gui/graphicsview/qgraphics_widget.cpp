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

#include <qglobal.h>

#ifndef QT_NO_GRAPHICSVIEW

#include <qgraphicswidget.h>
#include <qgraphicslayout.h>
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>
#include <qmutex.h>
#include <qapplication.h>
#include <qgraphicsview.h>
#include <qgraphicsproxywidget.h>
#include <qpalette.h>
#include <qstyleoption.h>
#include <qdebug.h>

#include <qapplication_p.h>
#include <qgraphics_layout_p.h>
#include <qgraphics_widget_p.h>
#include <qgraphics_scene_p.h>

#ifndef QT_NO_ACTION
#include <qaction_p.h>
#endif

#ifndef QT_NO_SHORTCUT
#include <qshortcutmap_p.h>
#endif

QGraphicsWidget::QGraphicsWidget(QGraphicsItem *parent, Qt::WindowFlags flags)
   : QGraphicsObject(*new QGraphicsWidgetPrivate, nullptr), QGraphicsLayoutItem(nullptr, false)
{
   Q_D(QGraphicsWidget);
   d->init(parent, flags);
}

// internal
QGraphicsWidget::QGraphicsWidget(QGraphicsWidgetPrivate &dd, QGraphicsItem *parent, Qt::WindowFlags wFlags)
   : QGraphicsObject(dd, nullptr), QGraphicsLayoutItem(nullptr, false)
{
   Q_D(QGraphicsWidget);
   d->init(parent, wFlags);
}

// internal
class QGraphicsWidgetStyles
{
 public:
   QStyle *styleForWidget(const QGraphicsWidget *widget) const {
      QMutexLocker locker(&mutex);
      return styles.value(widget, nullptr);
   }

   void setStyleForWidget(QGraphicsWidget *widget, QStyle *style) {
      QMutexLocker locker(&mutex);
      if (style) {
         styles[widget] = style;
      } else {
         styles.remove(widget);
      }
   }

 private:
   QHash<const QGraphicsWidget *, QStyle *> styles;
   mutable QMutex mutex;
};

static QGraphicsWidgetStyles *widgetStyles()
{
   static QGraphicsWidgetStyles retval;
   return &retval;
}

QGraphicsWidget::~QGraphicsWidget()
{
   Q_D(QGraphicsWidget);

#ifndef QT_NO_ACTION
   // Remove all actions from this widget
   for (int i = 0; i < d->actions.size(); ++i) {
      QActionPrivate *apriv = d->actions.at(i)->d_func();
      apriv->graphicsWidgets.removeAll(this);
   }
   d->actions.clear();
#endif

   if (QGraphicsScene *scn = scene()) {
      QGraphicsScenePrivate *sceneD = scn->d_func();

      if (sceneD->tabFocusFirst == this) {
         sceneD->tabFocusFirst = (d->focusNext == this ? nullptr : d->focusNext);
      }
   }
   d->focusPrev->d_func()->focusNext = d->focusNext;
   d->focusNext->d_func()->focusPrev = d->focusPrev;

   // Play it really safe
   d->focusNext = this;
   d->focusPrev = this;

   clearFocus();

   //we check if we have a layout previously
   if (d->layout) {
      QGraphicsLayout *temp = d->layout;

      for (QGraphicsItem *item : childItems()) {
         // In case of a custom layout which doesn't remove and delete items, we ensure that
         // the parent layout item does not point to the deleted layout.
         if (item->isWidget()) {
            QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(item);
            if (widget->parentLayoutItem() == d->layout) {
               widget->setParentLayoutItem(nullptr);
            }
         }
      }

      d->layout = nullptr;
      delete temp;
   }

   // Remove this graphics widget from widgetStyles
   widgetStyles()->setStyleForWidget(this, nullptr);
   setParentItem(nullptr);
}

QSizeF QGraphicsWidget::size() const
{
   return QGraphicsLayoutItem::geometry().size();
}

void QGraphicsWidget::resize(const QSizeF &size)
{
   setGeometry(QRectF(pos(), size));
}

void QGraphicsWidget::setGeometry(const QRectF &rect)
{
   QGraphicsWidgetPrivate *wd = QGraphicsWidget::d_func();
   QGraphicsLayoutItemPrivate *d = QGraphicsLayoutItem::d_ptr.data();
   QRectF newGeom;
   QPointF oldPos = d->geom.topLeft();

   if (!wd->inSetPos) {
      setAttribute(Qt::WA_Resized);
      newGeom = rect;
      newGeom.setSize(rect.size().expandedTo(effectiveSizeHint(Qt::MinimumSize))
         .boundedTo(effectiveSizeHint(Qt::MaximumSize)));

      if (newGeom == d->geom) {
         goto relayoutChildrenAndReturn;
      }

      // setPos triggers ItemPositionChange, which can adjust position
      wd->inSetGeometry = 1;
      setPos(newGeom.topLeft());
      wd->inSetGeometry = 0;
      newGeom.moveTopLeft(pos());

      if (newGeom == d->geom) {
         goto relayoutChildrenAndReturn;
      }

      // Update and prepare to change the geometry (remove from index) if the size has changed.
      if (wd->scene) {
         if (rect.topLeft() == d->geom.topLeft()) {
            prepareGeometryChange();
         }
      }
   }

   // Update the layout item geometry
   {
      bool moved = oldPos != pos();
      if (moved) {
         // Send move event.
         QGraphicsSceneMoveEvent event;
         event.setOldPos(oldPos);
         event.setNewPos(pos());
         QApplication::sendEvent(this, &event);
         if (wd->inSetPos) {
            //set the new pos
            d->geom.moveTopLeft(pos());
            emit geometryChanged();
            goto relayoutChildrenAndReturn;
         }
      }
      QSizeF oldSize = size();
      QGraphicsLayoutItem::setGeometry(newGeom);

      // Send resize event
      bool resized = newGeom.size() != oldSize;

      if (resized) {
         QGraphicsSceneResizeEvent re;
         re.setOldSize(oldSize);
         re.setNewSize(newGeom.size());

         if (oldSize.width() != newGeom.size().width()) {
            emit widthChanged();
         }

         if (oldSize.height() != newGeom.size().height()) {
            emit heightChanged();
         }

         QGraphicsLayout *lay = wd->layout;
         if (QGraphicsLayout::instantInvalidatePropagation()) {
            if (!lay || lay->isActivated()) {
               QApplication::sendEvent(this, &re);
            }
         } else {
            QApplication::sendEvent(this, &re);
         }

      }
   }

   emit geometryChanged();

relayoutChildrenAndReturn:
   if (QGraphicsLayout::instantInvalidatePropagation()) {
      if (QGraphicsLayout *lay = wd->layout) {
         if (!lay->isActivated()) {
            QEvent layoutRequest(QEvent::LayoutRequest);
            QApplication::sendEvent(this, &layoutRequest);
         }
      }
   }
}

void QGraphicsWidget::setContentsMargins(qreal left, qreal top, qreal right, qreal bottom)
{
   Q_D(QGraphicsWidget);

   if (!d->margins && left == 0 && top == 0 && right == 0 && bottom == 0) {
      return;
   }

   d->ensureMargins();

   if (left == d->margins[d->Left]
      && top == d->margins[d->Top]
      && right == d->margins[d->Right]
      && bottom == d->margins[d->Bottom]) {
      return;
   }

   d->margins[d->Left] = left;
   d->margins[d->Top] = top;
   d->margins[d->Right] = right;
   d->margins[d->Bottom] = bottom;

   if (QGraphicsLayout *l = d->layout) {
      l->invalidate();
   } else {
      updateGeometry();
   }

   QEvent e(QEvent::ContentsRectChange);
   QApplication::sendEvent(this, &e);
}

void QGraphicsWidget::getContentsMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const
{
   Q_D(const QGraphicsWidget);
   if (left || top || right || bottom) {
      d->ensureMargins();
   }
   if (left) {
      *left = d->margins[d->Left];
   }
   if (top) {
      *top = d->margins[d->Top];
   }
   if (right) {
      *right = d->margins[d->Right];
   }
   if (bottom) {
      *bottom = d->margins[d->Bottom];
   }
}

void QGraphicsWidget::setWindowFrameMargins(qreal left, qreal top, qreal right, qreal bottom)
{
   Q_D(QGraphicsWidget);

   if (!d->windowFrameMargins && left == 0 && top == 0 && right == 0 && bottom == 0) {
      return;
   }
   d->ensureWindowFrameMargins();
   bool unchanged =
      d->windowFrameMargins[d->Left] == left
      && d->windowFrameMargins[d->Top] == top
      && d->windowFrameMargins[d->Right] == right
      && d->windowFrameMargins[d->Bottom] == bottom;
   if (d->setWindowFrameMargins && unchanged) {
      return;
   }
   if (!unchanged) {
      prepareGeometryChange();
   }
   d->windowFrameMargins[d->Left] = left;
   d->windowFrameMargins[d->Top] = top;
   d->windowFrameMargins[d->Right] = right;
   d->windowFrameMargins[d->Bottom] = bottom;
   d->setWindowFrameMargins = true;
}

void QGraphicsWidget::getWindowFrameMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const
{
   Q_D(const QGraphicsWidget);

   if (left || top || right || bottom) {
      d->ensureWindowFrameMargins();
   }

   if (left) {
      *left = d->windowFrameMargins[d->Left];
   }

   if (top) {
      *top = d->windowFrameMargins[d->Top];
   }

   if (right) {
      *right = d->windowFrameMargins[d->Right];
   }

   if (bottom) {
      *bottom = d->windowFrameMargins[d->Bottom];
   }
}

void QGraphicsWidget::unsetWindowFrameMargins()
{
   Q_D(QGraphicsWidget);

   if ((d->m_flags & Qt::Window) && (d->m_flags & Qt::WindowType_Mask) != Qt::Popup &&
         (d->m_flags & Qt::WindowType_Mask) != Qt::ToolTip && ! (d->m_flags & Qt::FramelessWindowHint)) {
      QStyleOptionTitleBar bar;
      d->initStyleOptionTitleBar(&bar);

      QStyle *style = this->style();
      qreal margin = style->pixelMetric(QStyle::PM_MdiSubWindowFrameWidth);
      qreal titleBarHeight  = d->titleBarHeight(bar);
      setWindowFrameMargins(margin, titleBarHeight, margin, margin);

   } else {
      setWindowFrameMargins(0, 0, 0, 0);
   }

   d->setWindowFrameMargins = false;
}

QRectF QGraphicsWidget::windowFrameGeometry() const
{
   Q_D(const QGraphicsWidget);

   return d->windowFrameMargins
      ? geometry().adjusted(-d->windowFrameMargins[d->Left], -d->windowFrameMargins[d->Top],
         d->windowFrameMargins[d->Right], d->windowFrameMargins[d->Bottom])
      : geometry();
}

QRectF QGraphicsWidget::windowFrameRect() const
{
   Q_D(const QGraphicsWidget);
   return d->windowFrameMargins
      ? rect().adjusted(-d->windowFrameMargins[d->Left], -d->windowFrameMargins[d->Top],
         d->windowFrameMargins[d->Right], d->windowFrameMargins[d->Bottom])
      : rect();
}

void QGraphicsWidget::initStyleOption(QStyleOption *option) const
{
   Q_ASSERT(option);

   option->state = QStyle::State_None;
   if (isEnabled()) {
      option->state |= QStyle::State_Enabled;
   }
   if (hasFocus()) {
      option->state |= QStyle::State_HasFocus;
   }
   // if (window->testAttribute(Qt::WA_KeyboardFocusChange)) // ### Window
   //     option->state |= QStyle::State_KeyboardFocusChange;
   if (isUnderMouse()) {
      option->state |= QStyle::State_MouseOver;
   }
   if (QGraphicsWidget *w = window()) {
      if (w->isActiveWindow()) {
         option->state |= QStyle::State_Active;
      }
   }
   if (isWindow()) {
      option->state |= QStyle::State_Window;
   }

   /*
   #ifdef Q_OS_DARWIN
   extern bool qt_mac_can_clickThrough(const QGraphicsWidget *w); //qwidget_mac.cpp
   if (!(option->state & QStyle::State_Active) && !qt_mac_can_clickThrough(widget))
       option->state &= ~QStyle::State_Enabled;

   switch (QMacStyle::widgetSizePolicy(widget)) {
   case QMacStyle::SizeSmall:
       option->state |= QStyle::State_Small;
       break;
   case QMacStyle::SizeMini:
       option->state |= QStyle::State_Mini;
       break;
   default:
       ;
   }
   #endif

   #ifdef QT_KEYPAD_NAVIGATION
   if (widget->hasEditFocus())
       state |= QStyle::State_HasEditFocus;
   #endif
   */

   option->direction = layoutDirection();
   option->rect = rect().toRect(); // ### truncation!
   option->palette = palette();
   if (!isEnabled()) {
      option->palette.setCurrentColorGroup(QPalette::Disabled);
   } else if (isActiveWindow()) {
      option->palette.setCurrentColorGroup(QPalette::Active);
   } else {
      option->palette.setCurrentColorGroup(QPalette::Inactive);
   }

   option->fontMetrics = QFontMetrics(font());
   option->styleObject = const_cast<QGraphicsWidget *>(this);
}

QSizeF QGraphicsWidget::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
   Q_D(const QGraphicsWidget);
   QSizeF sh;

   if (d->layout) {
      QSizeF marginSize(0, 0);

      if (d->margins) {
         marginSize = QSizeF(d->margins[d->Left] + d->margins[d->Right],
               d->margins[d->Top] + d->margins[d->Bottom]);
      }

      sh = d->layout->effectiveSizeHint(which, constraint - marginSize);
      sh += marginSize;

   } else {
      switch (which) {
         case Qt::MinimumSize:
            sh = QSizeF(0, 0);
            break;

         case Qt::PreferredSize:
            sh = QSizeF(50, 50);    //rather arbitrary
            break;

         case Qt::MaximumSize:
            sh = QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            break;

         default:
            qWarning("QGraphicsWidget::sizeHint() Value for size hint is not valid");
            break;
      }
   }
   return sh;
}

QGraphicsLayout *QGraphicsWidget::layout() const
{
   Q_D(const QGraphicsWidget);
   return d->layout;
}

void QGraphicsWidget::setLayout(QGraphicsLayout *l)
{
   Q_D(QGraphicsWidget);
   if (d->layout == l) {
      return;
   }
   d->setLayout_helper(l);
   if (!l) {
      return;
   }

   // Prevent assigning a layout that is already assigned to another widget.
   QGraphicsLayoutItem *oldParent = l->parentLayoutItem();

   if (oldParent && oldParent != this) {
      qWarning("QGraphicsWidget::setLayout() Unable to set a layout on %s, current layout already has a parent",
         csPrintable(metaObject()->className()));
      return;
   }

   // Install and activate the layout.
   l->setParentLayoutItem(this);
   l->d_func()->reparentChildItems(this);
   l->invalidate();
   emit layoutChanged();
}

void QGraphicsWidget::adjustSize()
{
   QSizeF sz = effectiveSizeHint(Qt::PreferredSize);
   // What if sz is not valid?!
   if (sz.isValid()) {
      resize(sz);
   }
}

Qt::LayoutDirection QGraphicsWidget::layoutDirection() const
{
   return testAttribute(Qt::WA_RightToLeft) ? Qt::RightToLeft : Qt::LeftToRight;
}
void QGraphicsWidget::setLayoutDirection(Qt::LayoutDirection direction)
{
   Q_D(QGraphicsWidget);
   setAttribute(Qt::WA_SetLayoutDirection, true);
   d->setLayoutDirection_helper(direction);
}
void QGraphicsWidget::unsetLayoutDirection()
{
   Q_D(QGraphicsWidget);
   setAttribute(Qt::WA_SetLayoutDirection, false);
   d->resolveLayoutDirection();
}

QStyle *QGraphicsWidget::style() const
{
   if (QStyle *style = widgetStyles()->styleForWidget(this)) {
      return style;
   }
   // ### This is not thread-safe. QApplication::style() is not thread-safe.
   return scene() ? scene()->style() : QApplication::style();
}

void QGraphicsWidget::setStyle(QStyle *style)
{
   setAttribute(Qt::WA_SetStyle, style != nullptr);
   widgetStyles()->setStyleForWidget(this, style);

   // Deliver StyleChange to the widget itself (doesn't propagate).
   QEvent event(QEvent::StyleChange);
   QApplication::sendEvent(this, &event);
}

QFont QGraphicsWidget::font() const
{
   Q_D(const QGraphicsWidget);
   QFont fnt = d->font;
   fnt.resolve(fnt.resolve() | d->inheritedFontResolveMask);
   return fnt;
}
void QGraphicsWidget::setFont(const QFont &font)
{
   Q_D(QGraphicsWidget);
   setAttribute(Qt::WA_SetFont, font.resolve() != 0);

   QFont naturalFont = d->naturalWidgetFont();
   QFont resolvedFont = font.resolve(naturalFont);
   d->setFont_helper(resolvedFont);
}

QPalette QGraphicsWidget::palette() const
{
   Q_D(const QGraphicsWidget);
   return d->palette;
}
void QGraphicsWidget::setPalette(const QPalette &palette)
{
   Q_D(QGraphicsWidget);
   setAttribute(Qt::WA_SetPalette, palette.resolve() != 0);

   QPalette naturalPalette = d->naturalWidgetPalette();
   QPalette resolvedPalette = palette.resolve(naturalPalette);
   d->setPalette_helper(resolvedPalette);
}

bool QGraphicsWidget::autoFillBackground() const
{
   Q_D(const QGraphicsWidget);
   return d->autoFillBackground;
}

void QGraphicsWidget::setAutoFillBackground(bool enabled)
{
   Q_D(QGraphicsWidget);
   if (d->autoFillBackground != enabled) {
      d->autoFillBackground = enabled;
      update();
   }
}

void QGraphicsWidget::updateGeometry()
{
   QGraphicsLayoutItem::updateGeometry();
   QGraphicsLayoutItem *parentItem = parentLayoutItem();

   if (parentItem && parentItem->isLayout()) {
      if (QGraphicsLayout::instantInvalidatePropagation()) {
         static_cast<QGraphicsLayout *>(parentItem)->invalidate();
      } else {
         parentItem->updateGeometry();
      }

   } else {

      if (parentItem) {
         // This is for custom layouting
         QGraphicsWidget *parentWid = parentWidget();

         if (parentWid->isVisible()) {
            QApplication::postEvent(parentWid, new QEvent(QEvent::LayoutRequest));
         }

      } else {
         /**
          * If this is the topmost widget, post a LayoutRequest event to the widget.
          * When the event is received, it will start flowing all the way down to the leaf
          * widgets in one go. This will make a relayout flicker-free.
          */

         if (QGraphicsLayout::instantInvalidatePropagation()) {

            QApplication::postEvent(static_cast<QGraphicsWidget *>(this), new QEvent(QEvent::LayoutRequest));
         }
      }

      if (!QGraphicsLayout::instantInvalidatePropagation()) {
         bool wasResized = testAttribute(Qt::WA_Resized);
         resize(size()); // this will restrict the size
         setAttribute(Qt::WA_Resized, wasResized);
      }
   }
}


QVariant QGraphicsWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
   Q_D(QGraphicsWidget);

   switch (change) {
      case ItemEnabledHasChanged: {
         // Send EnabledChange after the enabled state has changed.
         QEvent event(QEvent::EnabledChange);
         QApplication::sendEvent(this, &event);
         break;
      }
      case ItemVisibleChange:
         if (value.toBool()) {
            // Send Show event before the item has been shown.
            QShowEvent event;
            QApplication::sendEvent(this, &event);
            bool resized = testAttribute(Qt::WA_Resized);
            if (!resized) {
               adjustSize();
               setAttribute(Qt::WA_Resized, false);
            }
         }
         // layout size hint only changes if an item changes from/to explicitly hidden state
         if (value.toBool() || d->explicitlyHidden) {
            updateGeometry();
         }
         break;
      case ItemVisibleHasChanged:
         if (!value.toBool()) {
            // Send Hide event after the item has been hidden.
            QHideEvent event;
            QApplication::sendEvent(this, &event);
         }
         break;
      case ItemPositionHasChanged:
         d->setGeometryFromSetPos();
         break;
      case ItemParentChange: {
         // Deliver ParentAboutToChange.
         QEvent event(QEvent::ParentAboutToChange);
         QApplication::sendEvent(this, &event);
         break;
      }

      case ItemParentHasChanged: {
         // Deliver ParentChange.
         QEvent event(QEvent::ParentChange);
         QApplication::sendEvent(this, &event);
         break;
      }

      case ItemCursorHasChanged: {
         // Deliver CursorChange.
         QEvent event(QEvent::CursorChange);
         QApplication::sendEvent(this, &event);
         break;
      }

      case ItemToolTipHasChanged: {
         // Deliver ToolTipChange.
         QEvent event(QEvent::ToolTipChange);
         QApplication::sendEvent(this, &event);
         break;
      }

      default:
         break;
   }
   return QGraphicsItem::itemChange(change, value);
}

// internal
QVariant QGraphicsWidget::propertyChange(const QString &propertyName, const QVariant &value)
{
   (void) propertyName;
   return value;
}

bool QGraphicsWidget::sceneEvent(QEvent *event)
{
   return QGraphicsItem::sceneEvent(event);
}

bool QGraphicsWidget::windowFrameEvent(QEvent *event)
{
   Q_D(QGraphicsWidget);
   switch (event->type()) {
      case QEvent::GraphicsSceneMousePress:
         d->windowFrameMousePressEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
         break;
      case QEvent::GraphicsSceneMouseMove:
         d->ensureWindowData();
         if (d->windowData->grabbedSection != Qt::NoSection) {
            d->windowFrameMouseMoveEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
            event->accept();
         }
         break;
      case QEvent::GraphicsSceneMouseRelease:
         d->windowFrameMouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
         break;
      case QEvent::GraphicsSceneHoverMove:
         d->windowFrameHoverMoveEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
         break;
      case QEvent::GraphicsSceneHoverLeave:
         d->windowFrameHoverLeaveEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
         break;
      default:
         break;
   }
   return event->isAccepted();
}

Qt::WindowFrameSection QGraphicsWidget::windowFrameSectionAt(const QPointF &pos) const
{
   Q_D(const QGraphicsWidget);

   const QRectF r = windowFrameRect();
   if (!r.contains(pos)) {
      return Qt::NoSection;
   }

   const qreal left = r.left();
   const qreal top = r.top();
   const qreal right = r.right();
   const qreal bottom = r.bottom();
   const qreal x = pos.x();
   const qreal y = pos.y();

   const qreal cornerMargin = 20;
   //### Not sure of this one, it should be the same value for all edges.
   const qreal windowFrameWidth = d->windowFrameMargins
      ? d->windowFrameMargins[d->Left] : 0;

   Qt::WindowFrameSection s = Qt::NoSection;
   if (x <= left + cornerMargin) {
      if (y <= top + windowFrameWidth || (x <= left + windowFrameWidth && y <= top + cornerMargin)) {
         s = Qt::TopLeftSection;
      } else if (y >= bottom - windowFrameWidth || (x <= left + windowFrameWidth && y >= bottom - cornerMargin)) {
         s = Qt::BottomLeftSection;
      } else if (x <= left + windowFrameWidth) {
         s = Qt::LeftSection;
      }
   } else if (x >= right - cornerMargin) {
      if (y <= top + windowFrameWidth || (x >= right - windowFrameWidth && y <= top + cornerMargin)) {
         s = Qt::TopRightSection;
      } else if (y >= bottom - windowFrameWidth || (x >= right - windowFrameWidth && y >= bottom - cornerMargin)) {
         s = Qt::BottomRightSection;
      } else if (x >= right - windowFrameWidth) {
         s = Qt::RightSection;
      }
   } else if (y <= top + windowFrameWidth) {
      s = Qt::TopSection;
   } else if (y >= bottom - windowFrameWidth) {
      s = Qt::BottomSection;
   }
   if (s == Qt::NoSection) {
      QRectF r1 = r;
      r1.setHeight(d->windowFrameMargins
         ? d->windowFrameMargins[d->Top] : 0);
      if (r1.contains(pos)) {
         s = Qt::TitleBarArea;
      }
   }
   return s;
}

bool QGraphicsWidget::event(QEvent *event)
{
   Q_D(QGraphicsWidget);
   // Forward the event to the layout first.
   if (d->layout) {
      d->layout->widgetEvent(event);
   }

   // Handle the event itself.
   switch (event->type()) {
      case QEvent::GraphicsSceneMove:
         moveEvent(static_cast<QGraphicsSceneMoveEvent *>(event));
         break;
      case QEvent::GraphicsSceneResize:
         resizeEvent(static_cast<QGraphicsSceneResizeEvent *>(event));
         break;
      case QEvent::Show:
         showEvent(static_cast<QShowEvent *>(event));
         break;
      case QEvent::Hide:
         hideEvent(static_cast<QHideEvent *>(event));
         break;
      case QEvent::Polish:
         polishEvent();
         d->polished = true;
         if (!d->font.isCopyOf(QApplication::font())) {
            d->updateFont(d->font);
         }
         break;
      case QEvent::WindowActivate:
      case QEvent::WindowDeactivate:
         update();
         break;
      case QEvent::StyleAnimationUpdate:
         if (isVisible()) {
            event->accept();
            update();
         }
         break;
      // Taken from QWidget::event
      case QEvent::ActivationChange:
      case QEvent::EnabledChange:
      case QEvent::FontChange:
      case QEvent::StyleChange:
      case QEvent::PaletteChange:
      case QEvent::ParentChange:
      case QEvent::ContentsRectChange:
      case QEvent::LayoutDirectionChange:
         changeEvent(event);
         break;

      case QEvent::Close:
         closeEvent((QCloseEvent *)event);
         break;

      case QEvent::GrabMouse:
         grabMouseEvent(event);
         break;

      case QEvent::UngrabMouse:
         ungrabMouseEvent(event);
         break;

      case QEvent::GrabKeyboard:
         grabKeyboardEvent(event);
         break;

      case QEvent::UngrabKeyboard:
         ungrabKeyboardEvent(event);
         break;

      case QEvent::GraphicsSceneMousePress:
         if (d->hasDecoration() && windowFrameEvent(event)) {
            return true;
         }
         break;

      case QEvent::GraphicsSceneMouseMove:
      case QEvent::GraphicsSceneMouseRelease:
      case QEvent::GraphicsSceneMouseDoubleClick:
         d->ensureWindowData();
         if (d->hasDecoration() && d->windowData->grabbedSection != Qt::NoSection) {
            return windowFrameEvent(event);
         }
         break;

      case QEvent::GraphicsSceneHoverEnter:
      case QEvent::GraphicsSceneHoverMove:
      case QEvent::GraphicsSceneHoverLeave:
         if (d->hasDecoration()) {
            windowFrameEvent(event);
            // Filter out hover events if they were sent to us only because of the
            // decoration (special case in QGraphicsScenePrivate::dispatchHoverEvent).
            if (!acceptHoverEvents()) {
               return true;
            }
         }
         break;

      default:
         break;
   }
   return QObject::event(event);
}

void QGraphicsWidget::changeEvent(QEvent *event)
{
   Q_D(QGraphicsWidget);

   switch (event->type()) {
      case QEvent::StyleChange:
         // ### Don't unset if the margins are explicitly set.
         unsetWindowFrameMargins();

         if (d->layout) {
            d->layout->invalidate();
         }
         [[fallthrough]];

      case QEvent::FontChange:
         update();
         updateGeometry();
         break;

      case QEvent::PaletteChange:
         update();
         break;

      case QEvent::ParentChange:
         d->resolveFont(d->inheritedFontResolveMask);
         d->resolvePalette(d->inheritedPaletteResolveMask);
         break;

      default:
         break;
   }
}

void QGraphicsWidget::closeEvent(QCloseEvent *event)
{
   event->accept();
}

void QGraphicsWidget::focusInEvent(QFocusEvent *event)
{
   (void) event;
   if (focusPolicy() != Qt::NoFocus) {
      update();
   }
}

bool QGraphicsWidget::focusNextPrevChild(bool next)
{
   Q_D(QGraphicsWidget);
   // Let the parent's focusNextPrevChild implementation decide what to do.
   QGraphicsWidget *parent = nullptr;
   if (!isWindow() && (parent = parentWidget())) {
      return parent->focusNextPrevChild(next);
   }
   if (!d->scene) {
      return false;
   }
   if (d->scene->focusNextPrevChild(next)) {
      return true;
   }
   if (isWindow()) {
      setFocus(next ? Qt::TabFocusReason : Qt::BacktabFocusReason);
      if (hasFocus()) {
         return true;
      }
   }
   return false;
}

void QGraphicsWidget::focusOutEvent(QFocusEvent *event)
{
   (void) event;
   if (focusPolicy() != Qt::NoFocus) {
      update();
   }
}

void QGraphicsWidget::hideEvent(QHideEvent *event)
{
   ///### focusNextPrevChild(true), don't lose focus when the focus widget is hidden.
   (void) event;
}

void QGraphicsWidget::moveEvent(QGraphicsSceneMoveEvent *event)
{
   // ### Last position is always == current position
   (void) event;
}

void QGraphicsWidget::polishEvent()
{
}

void QGraphicsWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
   (void) event;
}

void QGraphicsWidget::showEvent(QShowEvent *event)
{
   (void) event;
}

void QGraphicsWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
   (void) event;
}

void QGraphicsWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
   QGraphicsObject::hoverLeaveEvent(event);
}

void QGraphicsWidget::grabMouseEvent(QEvent *event)
{
   (void) event;
}

void QGraphicsWidget::ungrabMouseEvent(QEvent *event)
{
   (void) event;
}

void QGraphicsWidget::grabKeyboardEvent(QEvent *event)
{
   (void) event;
}

void QGraphicsWidget::ungrabKeyboardEvent(QEvent *event)
{
   (void) event;
}

Qt::WindowType QGraphicsWidget::windowType() const
{
   return Qt::WindowType(int(windowFlags()) & Qt::WindowType_Mask);
}

Qt::WindowFlags QGraphicsWidget::windowFlags() const
{
   Q_D(const QGraphicsWidget);
   return d->m_flags;
}

void QGraphicsWidget::setWindowFlags(Qt::WindowFlags flags)
{
   Q_D(QGraphicsWidget);

   if (d->m_flags == flags) {
      return;
   }
   bool wasPopup = (d->m_flags & Qt::WindowType_Mask) == Qt::Popup;

   d->adjustWindowFlags(&flags);
   d->m_flags = flags;

   if (! d->setWindowFrameMargins) {
      unsetWindowFrameMargins();
   }

   setFlag(ItemIsPanel, d->m_flags & Qt::Window);

   bool isPopup = (d->m_flags & Qt::WindowType_Mask) == Qt::Popup;

   if (d->scene && isVisible() && wasPopup != isPopup) {
      // Popup state changed; update implicit mouse grab.
      if (! isPopup) {
         d->scene->d_func()->removePopup(this);
      } else {
         d->scene->d_func()->addPopup(this);
      }
   }

   if (d->scene && d->scene->d_func()->allItemsIgnoreHoverEvents && d->hasDecoration()) {
      d->scene->d_func()->allItemsIgnoreHoverEvents = false;
      d->scene->d_func()->enableMouseTrackingOnViews();
   }
}

bool QGraphicsWidget::isActiveWindow() const
{
   return isActive();
}

void QGraphicsWidget::setWindowTitle(const QString &title)
{
   Q_D(QGraphicsWidget);
   d->ensureWindowData();
   d->windowData->windowTitle = title;
}

QString QGraphicsWidget::windowTitle() const
{
   Q_D(const QGraphicsWidget);
   return d->windowData ? d->windowData->windowTitle : QString();
}

Qt::FocusPolicy QGraphicsWidget::focusPolicy() const
{
   Q_D(const QGraphicsWidget);
   return d->focusPolicy;
}

void QGraphicsWidget::setFocusPolicy(Qt::FocusPolicy policy)
{
   Q_D(QGraphicsWidget);
   if (d->focusPolicy == policy) {
      return;
   }

   d->focusPolicy = policy;
   if (hasFocus() && policy == Qt::NoFocus) {
      clearFocus();
   }

   setFlag(ItemIsFocusable, policy != Qt::NoFocus);
}

QGraphicsWidget *QGraphicsWidget::focusWidget() const
{
   Q_D(const QGraphicsWidget);
   if (d->subFocusItem && d->subFocusItem->d_ptr->isWidget) {
      return static_cast<QGraphicsWidget *>(d->subFocusItem);
   }

   return nullptr;
}

#ifndef QT_NO_SHORTCUT

int QGraphicsWidget::grabShortcut(const QKeySequence &sequence, Qt::ShortcutContext context)
{
   Q_ASSERT(qApp);
   if (sequence.isEmpty()) {
      return 0;
   }

   // ### setAttribute(Qt::WA_GrabbedShortcut);
   return qApp->d_func()->shortcutMap.addShortcut(this, sequence, context, qWidgetShortcutContextMatcher);
}

void QGraphicsWidget::releaseShortcut(int id)
{
   Q_ASSERT(qApp);
   if (id) {
      qApp->d_func()->shortcutMap.removeShortcut(id, this, 0);
   }
}

void QGraphicsWidget::setShortcutEnabled(int id, bool enabled)
{
   Q_ASSERT(qApp);
   if (id) {
      qApp->d_func()->shortcutMap.setShortcutEnabled(enabled, id, this, 0);
   }
}

void QGraphicsWidget::setShortcutAutoRepeat(int id, bool enabled)
{
   Q_ASSERT(qApp);
   if (id) {
      qApp->d_func()->shortcutMap.setShortcutAutoRepeat(enabled, id, this, 0);
   }
}
#endif

#ifndef QT_NO_ACTION

void QGraphicsWidget::addAction(QAction *action)
{
   insertAction(nullptr, action);
}

void QGraphicsWidget::addActions(const QList<QAction *> &actions)
{
   for (int i = 0; i < actions.count(); ++i) {
      insertAction(nullptr, actions.at(i));
   }
}

void QGraphicsWidget::insertAction(QAction *before, QAction *action)
{
   if (!action) {
      qWarning("QWidget::insertAction() Unable to to insert an invalid action (nullptr)");
      return;
   }

   Q_D(QGraphicsWidget);
   int index = d->actions.indexOf(action);
   if (index != -1) {
      d->actions.removeAt(index);
   }

   int pos = d->actions.indexOf(before);
   if (pos < 0) {
      before = nullptr;
      pos = d->actions.size();
   }
   d->actions.insert(pos, action);

   if (index == -1) {
      QActionPrivate *apriv = action->d_func();
      apriv->graphicsWidgets.append(this);
   }

   QActionEvent e(QEvent::ActionAdded, action, before);
   QApplication::sendEvent(this, &e);
}

void QGraphicsWidget::insertActions(QAction *before, QList<QAction *> actions)
{
   for (int i = 0; i < actions.count(); ++i) {
      insertAction(before, actions.at(i));
   }
}

void QGraphicsWidget::removeAction(QAction *action)
{
   if (!action) {
      return;
   }

   Q_D(QGraphicsWidget);

   QActionPrivate *apriv = action->d_func();
   apriv->graphicsWidgets.removeAll(this);

   if (d->actions.removeAll(action)) {
      QActionEvent e(QEvent::ActionRemoved, action);
      QApplication::sendEvent(this, &e);
   }
}

QList<QAction *> QGraphicsWidget::actions() const
{
   Q_D(const QGraphicsWidget);
   return d->actions;
}
#endif

void QGraphicsWidget::setTabOrder(QGraphicsWidget *first, QGraphicsWidget *second)
{
   if (!first && !second) {
      qWarning("QGraphicsWidget::setTabOrder() Both widgets are invalid (nullptr)");
      return;
   }

   if ((first && second) && first->scene() != second->scene()) {
      qWarning("QGraphicsWidget::setTabOrder() Items belong to different QGraphicsScene");
      return;
   }

   QGraphicsScene *scene = first ? first->scene() : second->scene();
   if (!scene && (!first || !second)) {
      qWarning("QGraphicsWidget::setTabOrder() Assigning a tab order requires both items to be in a QGraphicsScene");
      return;
   }

   // If either first or second are 0, the scene's tabFocusFirst is updated
   // to point to the first item in the scene's focus chain. Then first or
   // second are set to point to tabFocusFirst.
   QGraphicsScenePrivate *sceneD = scene->d_func();
   if (!first) {
      sceneD->tabFocusFirst = second;
      return;
   }
   if (!second) {
      sceneD->tabFocusFirst = first->d_func()->focusNext;
      return;
   }

   // Both first and second are != 0.
   QGraphicsWidget *firstFocusNext = first->d_func()->focusNext;
   if (firstFocusNext == second) {
      // Nothing to do.
      return;
   }

   // Update the focus chain.
   QGraphicsWidget *secondFocusPrev = second->d_func()->focusPrev;
   QGraphicsWidget *secondFocusNext = second->d_func()->focusNext;
   firstFocusNext->d_func()->focusPrev = second;
   first->d_func()->focusNext = second;
   second->d_func()->focusNext = firstFocusNext;
   second->d_func()->focusPrev = first;
   secondFocusPrev->d_func()->focusNext = secondFocusNext;
   secondFocusNext->d_func()->focusPrev = secondFocusPrev;

   Q_ASSERT(first->d_func()->focusNext->d_func()->focusPrev == first);
   Q_ASSERT(first->d_func()->focusPrev->d_func()->focusNext == first);

   Q_ASSERT(second->d_func()->focusNext->d_func()->focusPrev == second);
   Q_ASSERT(second->d_func()->focusPrev->d_func()->focusNext == second);

}

void QGraphicsWidget::setAttribute(Qt::WidgetAttribute attribute, bool on)
{
   Q_D(QGraphicsWidget);
   // ### most flags require some immediate action
   // ### we might want to qWarn use of unsupported attributes
   // ### we might want to not use Qt::WidgetAttribute, but roll our own instead
   d->setAttribute(attribute, on);
}

bool QGraphicsWidget::testAttribute(Qt::WidgetAttribute attribute) const
{
   Q_D(const QGraphicsWidget);
   return d->testAttribute(attribute);
}

int QGraphicsWidget::type() const
{
   return Type;
}

void QGraphicsWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   (void) painter;
   (void) option;
   (void) widget;
}

void QGraphicsWidget::paintWindowFrame(QPainter *painter, const QStyleOptionGraphicsItem *option,
   QWidget *widget)
{
   const bool fillBackground = !testAttribute(Qt::WA_OpaquePaintEvent)
      && !testAttribute(Qt::WA_NoSystemBackground);
   QGraphicsProxyWidget *proxy = qobject_cast<QGraphicsProxyWidget *>(this);
   const bool embeddedWidgetFillsOwnBackground = proxy && proxy->widget();

   if (rect().contains(option->exposedRect)) {
      if (fillBackground && !embeddedWidgetFillsOwnBackground) {
         painter->fillRect(option->exposedRect, palette().window());
      }
      return;
   }

   Q_D(QGraphicsWidget);

   QRect windowFrameRect = QRect(QPoint(), windowFrameGeometry().size().toSize());
   QStyleOptionTitleBar bar;
   bar.QStyleOption::operator=(*option);
   d->initStyleOptionTitleBar(&bar);   // this clear flags in bar.state
   d->ensureWindowData();
   if (d->windowData->buttonMouseOver) {
      bar.state |= QStyle::State_MouseOver;
   } else {
      bar.state &= ~QStyle::State_MouseOver;
   }
   if (d->windowData->buttonSunken) {
      bar.state |= QStyle::State_Sunken;
   } else {
      bar.state &= ~QStyle::State_Sunken;
   }

   bar.rect = windowFrameRect;

   // translate painter to make the style happy
   const QPointF styleOrigin = this->windowFrameRect().topLeft();
   painter->translate(styleOrigin);

#ifdef Q_OS_DARWIN
   const QSize pixmapSize = windowFrameRect.size();
   if (pixmapSize.width() <= 0 || pixmapSize.height() <= 0) {
      return;
   }
   QPainter *realPainter = painter;
   QPixmap pm(pixmapSize);
   painter = new QPainter(&pm);
#endif

   // Fill background
   QStyleHintReturnMask mask;
   bool setMask = style()->styleHint(QStyle::SH_WindowFrame_Mask, &bar, widget, &mask) && !mask.region.isEmpty();
   bool hasBorder = !style()->styleHint(QStyle::SH_TitleBar_NoBorder, &bar, widget);
   int frameWidth = style()->pixelMetric(QStyle::PM_MDIFrameWidth, &bar, widget);
   if (setMask) {
      painter->save();
      painter->setClipRegion(mask.region, Qt::IntersectClip);
   }
   if (fillBackground) {
      if (embeddedWidgetFillsOwnBackground) {
         // Don't fill the background twice.
         QPainterPath windowFrameBackground;
         windowFrameBackground.addRect(windowFrameRect);
         // Adjust with 0.5 to avoid border artifacts between
         // widget background and frame background.
         windowFrameBackground.addRect(rect().translated(-styleOrigin).adjusted(0.5, 0.5, -0.5, -0.5));
         painter->fillPath(windowFrameBackground, palette().window());
      } else {
         painter->fillRect(windowFrameRect, palette().window());
      }
   }


   // Draw title
   int height = (int)d->titleBarHeight(bar);
   bar.rect.setHeight(height);
   if (hasBorder) { // Frame is painted by PE_FrameWindow
      bar.rect.adjust(frameWidth, frameWidth, -frameWidth, 0);
   }

   painter->save();
   painter->setFont(QApplication::font("QMdiSubWindowTitleBar"));
   style()->drawComplexControl(QStyle::CC_TitleBar, &bar, painter, widget);
   painter->restore();
   if (setMask) {
      painter->restore();
   }

   // Draw window frame
   QStyleOptionFrame frameOptions;
   frameOptions.QStyleOption::operator=(*option);
   initStyleOption(&frameOptions);

   if (!hasBorder) {
      painter->setClipRect(windowFrameRect.adjusted(0, +height, 0, 0), Qt::IntersectClip);
   }
   if (hasFocus()) {
      frameOptions.state |= QStyle::State_HasFocus;
   } else {
      frameOptions.state &= ~QStyle::State_HasFocus;
   }
   bool isActive = isActiveWindow();
   if (isActive) {
      frameOptions.state |= QStyle::State_Active;
   } else {
      frameOptions.state &= ~QStyle::State_Active;
   }

   frameOptions.palette.setCurrentColorGroup(isActive ? QPalette::Active : QPalette::Normal);
   frameOptions.rect = windowFrameRect;
   frameOptions.lineWidth = style()->pixelMetric(QStyle::PM_MdiSubWindowFrameWidth, nullptr, widget);
   frameOptions.midLineWidth = 1;
   style()->drawPrimitive(QStyle::PE_FrameWindow, &frameOptions, painter, widget);

#ifdef Q_OS_DARWIN
   realPainter->drawPixmap(QPoint(), pm);
   delete painter;
#endif
}

QRectF QGraphicsWidget::boundingRect() const
{
   return windowFrameRect();
}

QPainterPath QGraphicsWidget::shape() const
{
   QPainterPath path;
   path.addRect(rect());
   return path;
}

bool QGraphicsWidget::close()
{
   QCloseEvent closeEvent;
   QApplication::sendEvent(this, &closeEvent);
   if (!closeEvent.isAccepted()) {
      return false;
   }
   // hide
   if (isVisible()) {
      hide();
   }
   if (testAttribute(Qt::WA_DeleteOnClose)) {
      deleteLater();
   }
   return true;
}

#endif //QT_NO_GRAPHICSVIEW
