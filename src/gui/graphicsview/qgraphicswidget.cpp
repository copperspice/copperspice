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

#include <qgraphicswidget.h>
#include <qgraphicswidget_p.h>
#include <qgraphicslayout.h>
#include <qgraphicslayout_p.h>
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>

#ifndef QT_NO_ACTION
#include <qaction_p.h>
#endif

#include <qapplication_p.h>
#include <qgraphicsscene_p.h>

#ifndef QT_NO_SHORTCUT
#include <qshortcutmap_p.h>
#endif

#include <QtCore/qmutex.h>
#include <QtGui/qapplication.h>
#include <QtGui/qgraphicsview.h>
#include <QtGui/qgraphicsproxywidget.h>
#include <QtGui/qpalette.h>
#include <QtGui/qstyleoption.h>
#include <qdebug.h>


QGraphicsWidget::QGraphicsWidget(QGraphicsItem *parent, Qt::WindowFlags wFlags)
   : QGraphicsObject(*new QGraphicsWidgetPrivate, nullptr), QGraphicsLayoutItem(0, false)
{
   Q_D(QGraphicsWidget);
   d->init(parent, wFlags);
}

/*!
    \internal

    Constructs a new QGraphicsWidget, using \a dd as parent.
*/
QGraphicsWidget::QGraphicsWidget(QGraphicsWidgetPrivate &dd, QGraphicsItem *parent, Qt::WindowFlags wFlags)
   : QGraphicsObject(dd, nullptr), QGraphicsLayoutItem(0, false)
{
   Q_D(QGraphicsWidget);
   d->init(parent, wFlags);
}

/*
    \internal
    \class QGraphicsWidgetStyles

    We use this thread-safe class to maintain a hash of styles for widgets
    styles. Note that QApplication::style() itself isn't thread-safe, QStyle
    isn't thread-safe, and we don't have a thread-safe factory for creating
    the default style, nor cloning a style.
*/
class QGraphicsWidgetStyles
{
 public:
   QStyle *styleForWidget(const QGraphicsWidget *widget) const {
      QMutexLocker locker(&mutex);
      return styles.value(widget, 0);
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
Q_GLOBAL_STATIC(QGraphicsWidgetStyles, widgetStyles)

/*!
    Destroys the QGraphicsWidget instance.
*/
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
         sceneD->tabFocusFirst = (d->focusNext == this ? 0 : d->focusNext);
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
         // the parent layout item does not point to the deleted layout. This code is here to
         // avoid regression from 4.4 to 4.5, because according to 4.5 docs it is not really needed.
         if (item->isWidget()) {
            QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(item);
            if (widget->parentLayoutItem() == d->layout) {
               widget->setParentLayoutItem(0);
            }
         }
      }
      d->layout = 0;
      delete temp;
   }

   // Remove this graphics widget from widgetStyles
   widgetStyles()->setStyleForWidget(this, 0);
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

/*!
    Gets the widget's contents margins. The margins are stored in \a left, \a
    top, \a right and \a bottom, as pointers to qreals. Each argument can
    be \e {omitted} by passing 0.

    \sa setContentsMargins()
*/
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

/*!
    Sets the widget's window frame margins to \a left, \a top, \a right and
    \a bottom. The default frame margins are provided by the style, and they
    depend on the current window flags.

    If you would like to draw your own window decoration, you can set your
    own frame margins to override the default margins.

    \sa unsetWindowFrameMargins(), getWindowFrameMargins(), windowFrameRect()
*/
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

/*!
    Gets the widget's window frame margins. The margins are stored in \a left,
    \a top, \a right and \a bottom as pointers to qreals. Each argument can
    be \e {omitted} by passing 0.

    \sa setWindowFrameMargins(), windowFrameRect()
*/
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

/*!
    Resets the window frame margins to the default value, provided by the style.

    \sa setWindowFrameMargins(), getWindowFrameMargins(), windowFrameRect()
*/
void QGraphicsWidget::unsetWindowFrameMargins()
{
   Q_D(QGraphicsWidget);
   if ((d->windowFlags & Qt::Window) && (d->windowFlags & Qt::WindowType_Mask) != Qt::Popup &&
      (d->windowFlags & Qt::WindowType_Mask) != Qt::ToolTip && !(d->windowFlags & Qt::FramelessWindowHint)) {
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

/*!
    Returns the widget's geometry in parent coordinates including any window
    frame.

    \sa windowFrameRect(), getWindowFrameMargins(), setWindowFrameMargins()
*/
QRectF QGraphicsWidget::windowFrameGeometry() const
{
   Q_D(const QGraphicsWidget);
   return d->windowFrameMargins
      ? geometry().adjusted(-d->windowFrameMargins[d->Left], -d->windowFrameMargins[d->Top],
         d->windowFrameMargins[d->Right], d->windowFrameMargins[d->Bottom])
      : geometry();
}

/*!
    Returns the widget's local rect including any window frame.

    \sa windowFrameGeometry(), getWindowFrameMargins(), setWindowFrameMargins()
*/
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

/*!
    \reimp
*/
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
            qWarning("QGraphicsWidget::sizeHint(): Don't know how to handle the value of 'which'");
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
      qWarning("QGraphicsWidget::setLayout: Attempting to set a layout on %s"
         " \"%s\", when the layout already has a parent", csPrintable(metaObject()->className()), csPrintable(objectName()));
      return;
   }

   // Install and activate the layout.
   l->setParentLayoutItem(this);
   l->d_func()->reparentChildItems(this);
   l->invalidate();
   emit layoutChanged();
}

/*!
    Adjusts the size of the widget to its effective preferred size hint.

    This function is called implicitly when the item is shown for the first
    time.

    \sa effectiveSizeHint(), Qt::MinimumSize
*/
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
   setAttribute(Qt::WA_SetStyle, style != 0);
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

/*!
    \property QGraphicsWidget::autoFillBackground
    \brief whether the widget background is filled automatically
    \since 4.7

    If enabled, this property will cause Qt to fill the background of the
    widget before invoking the paint() method. The color used is defined by the
    QPalette::Window color role from the widget's \l{QPalette}{palette}.

    In addition, Windows are always filled with QPalette::Window, unless the
    WA_OpaquePaintEvent or WA_NoSystemBackground attributes are set.

    By default, this property is false.

    \sa Qt::WA_OpaquePaintEvent, Qt::WA_NoSystemBackground,
*/
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

/*!
    If this widget is currently managed by a layout, this function notifies
    the layout that the widget's size hints have changed and the layout
    may need to resize and reposition the widget accordingly.

    Call this function if the widget's sizeHint() has changed.

    \sa QGraphicsLayout::invalidate()
*/
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

/*!
    \internal

    This virtual function is used to notify changes to any property (both
    dynamic properties, and registered with Q_PROPERTY) in the
    widget. Depending on the property itself, the notification can be
    delivered before or after the value has changed.

    \a propertyName is the name of the property (e.g., "size" or "font"), and
    \a value is the (proposed) new value of the property. The function returns
    the new value, which may be different from \a value if the notification
    supports adjusting the property value. The base implementation simply
    returns \a value for any \a propertyName.

    QGraphicsWidget delivers notifications for the following properties:

    \table     \o propertyName        \o Property
    \row       \o layoutDirection     \o QGraphicsWidget::layoutDirection
    \row       \o size                \o QGraphicsWidget::size
    \row       \o font                \o QGraphicsWidget::font
    \row       \o palette             \o QGraphicsWidget::palette
    \endtable

    \sa itemChange()
*/
QVariant QGraphicsWidget::propertyChange(const QString &propertyName, const QVariant &value)
{
   Q_UNUSED(propertyName);
   return value;
}

/*!
    QGraphicsWidget's implementation of sceneEvent() simply passes \a event to
    QGraphicsWidget::event(). You can handle all events for your widget in
    event() or in any of the convenience functions; you should not have to
    reimplement this function in a subclass of QGraphicsWidget.

    \sa QGraphicsItem::sceneEvent()
*/
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

/*!
   This event handler can be reimplemented to handle state changes.

   The state being changed in this event can be retrieved through \a event.

   Change events include: QEvent::ActivationChange, QEvent::EnabledChange,
   QEvent::FontChange, QEvent::StyleChange, QEvent::PaletteChange,
   QEvent::ParentChange, QEvent::LayoutDirectionChange, and
   QEvent::ContentsRectChange.
*/
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

/*!
    This event handler, for \a event, can be reimplemented in a subclass to
    receive widget close events.  The default implementation accepts the
    event.

    \sa close(), QCloseEvent
*/
void QGraphicsWidget::closeEvent(QCloseEvent *event)
{
   event->accept();
}

/*!
    \reimp
*/
void QGraphicsWidget::focusInEvent(QFocusEvent *event)
{
   Q_UNUSED(event);
   if (focusPolicy() != Qt::NoFocus) {
      update();
   }
}

/*!
    Finds a new widget to give the keyboard focus to, as appropriate for Tab
    and Shift+Tab, and returns true if it can find a new widget; returns false
    otherwise. If \a next is true, this function searches forward; if \a next
    is false, it searches backward.

    Sometimes, you will want to reimplement this function to provide special
    focus handling for your widget and its subwidgets. For example, a web
    browser might reimplement it to move its current active link forward or
    backward, and call the base implementation only when it reaches the last
    or first link on the page.

    Child widgets call focusNextPrevChild() on their parent widgets, but only
    the window that contains the child widgets decides where to redirect
    focus. By reimplementing this function for an object, you gain control of
    focus traversal for all child widgets.

    \sa focusPolicy()
*/
bool QGraphicsWidget::focusNextPrevChild(bool next)
{
   Q_D(QGraphicsWidget);
   // Let the parent's focusNextPrevChild implementation decide what to do.
   QGraphicsWidget *parent = 0;
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

/*!
    \reimp
*/
void QGraphicsWidget::focusOutEvent(QFocusEvent *event)
{
   Q_UNUSED(event);
   if (focusPolicy() != Qt::NoFocus) {
      update();
   }
}

/*!
    This event handler, for \l{QEvent::Hide}{Hide} events, is delivered after
    the widget has been hidden, for example, setVisible(false) has been called
    for the widget or one of its ancestors when the widget was previously
    shown.

    You can reimplement this event handler to detect when your widget is
    hidden. Calling QEvent::accept() or QEvent::ignore() on \a event has no
    effect.

    \sa showEvent(), QWidget::hideEvent(), ItemVisibleChange
*/
void QGraphicsWidget::hideEvent(QHideEvent *event)
{
   ///### focusNextPrevChild(true), don't lose focus when the focus widget
   // is hidden.
   Q_UNUSED(event);
}

/*!
    This event handler, for \l{QEvent::GraphicsSceneMove}{GraphicsSceneMove}
    events, is delivered after the widget has moved (e.g., its local position
    has changed).

    This event is only delivered when the item is moved locally. Calling
    setTransform() or moving any of the item's ancestors does not affect the
    item's local position.

    You can reimplement this event handler to detect when your widget has
    moved. Calling QEvent::accept() or QEvent::ignore() on \a event has no
    effect.

    \sa ItemPositionChange, ItemPositionHasChanged
*/
void QGraphicsWidget::moveEvent(QGraphicsSceneMoveEvent *event)
{
   // ### Last position is always == current position
   Q_UNUSED(event);
}

/*!
    This event is delivered to the item by the scene at some point after it
    has been constructed, but before it is shown or otherwise accessed through
    the scene. You can use this event handler to do last-minute initializations
    of the widget which require the item to be fully constructed.

    The base implementation does nothing.
*/
void QGraphicsWidget::polishEvent()
{
}

/*!
    This event handler, for
    \l{QEvent::GraphicsSceneResize}{GraphicsSceneResize} events, is
    delivered after the widget has been resized (i.e., its local size has
    changed). \a event contains both the old and the new size.

    This event is only delivered when the widget is resized locally; calling
    setTransform() on the widget or any of its ancestors or view, does not
    affect the widget's local size.

    You can reimplement this event handler to detect when your widget has been
    resized. Calling QEvent::accept() or QEvent::ignore() on \a event has no
    effect.

    \sa geometry(), setGeometry()
*/
void QGraphicsWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
   Q_UNUSED(event);
}

/*!
    This event handler, for \l{QEvent::Show}{Show} events, is delivered before
    the widget has been shown, for example, setVisible(true) has been called
    for the widget or one of its ancestors when the widget was previously
    hidden.

    You can reimplement this event handler to detect when your widget is
    shown. Calling QEvent::accept() or QEvent::ignore() on \a event has no
    effect.

    \sa hideEvent(), QWidget::showEvent(), ItemVisibleChange
*/
void QGraphicsWidget::showEvent(QShowEvent *event)
{
   Q_UNUSED(event);
}

/*!
    \reimp
*/
void QGraphicsWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
   Q_UNUSED(event);
}

/*!
    \reimp
*/
void QGraphicsWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
   QGraphicsObject::hoverLeaveEvent(event);
}

/*!
    This event handler, for \a event, can be reimplemented in a subclass to
    receive notifications for QEvent::GrabMouse events.

    \sa grabMouse(), grabKeyboard()
*/
void QGraphicsWidget::grabMouseEvent(QEvent *event)
{
   Q_UNUSED(event);
}

/*!
    This event handler, for \a event, can be reimplemented in a subclass to
    receive notifications for QEvent::UngrabMouse events.

    \sa ungrabMouse(), ungrabKeyboard()
*/
void QGraphicsWidget::ungrabMouseEvent(QEvent *event)
{
   Q_UNUSED(event);
}

/*!
    This event handler, for \a event, can be reimplemented in a subclass to
    receive notifications for QEvent::GrabKeyboard events.

    \sa grabKeyboard(), grabMouse()
*/
void QGraphicsWidget::grabKeyboardEvent(QEvent *event)
{
   Q_UNUSED(event);
}

/*!
    This event handler, for \a event, can be reimplemented in a subclass to
    receive notifications for QEvent::UngrabKeyboard events.

    \sa ungrabKeyboard(), ungrabMouse()
*/
void QGraphicsWidget::ungrabKeyboardEvent(QEvent *event)
{
   Q_UNUSED(event);
}

/*!
    Returns the widgets window type.

    \sa windowFlags(), isWindow(), isPanel()
*/
Qt::WindowType QGraphicsWidget::windowType() const
{
   return Qt::WindowType(int(windowFlags()) & Qt::WindowType_Mask);
}

/*!
    \property QGraphicsWidget::windowFlags
    \brief the widget's window flags

    Window flags are a combination of a window type (e.g., Qt::Dialog) and
    several flags giving hints on the behavior of the window. The behavior
    is platform-dependent.

    By default, this property contains no window flags.

    Windows are panels. If you set the Qt::Window flag, the ItemIsPanel flag
    will be set automatically. If you clear the Qt::Window flag, the
    ItemIsPanel flag is also cleared. Note that the ItemIsPanel flag can be
    set independently of Qt::Window.

    \sa isWindow(), isPanel()
*/
Qt::WindowFlags QGraphicsWidget::windowFlags() const
{
   Q_D(const QGraphicsWidget);
   return d->windowFlags;
}
void QGraphicsWidget::setWindowFlags(Qt::WindowFlags wFlags)
{
   Q_D(QGraphicsWidget);
   if (d->windowFlags == wFlags) {
      return;
   }
   bool wasPopup = (d->windowFlags & Qt::WindowType_Mask) == Qt::Popup;

   d->adjustWindowFlags(&wFlags);
   d->windowFlags = wFlags;
   if (!d->setWindowFrameMargins) {
      unsetWindowFrameMargins();
   }

   setFlag(ItemIsPanel, d->windowFlags & Qt::Window);

   bool isPopup = (d->windowFlags & Qt::WindowType_Mask) == Qt::Popup;
   if (d->scene && isVisible() && wasPopup != isPopup) {
      // Popup state changed; update implicit mouse grab.
      if (!isPopup) {
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
   return 0;
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
   insertAction(0, action);
}

void QGraphicsWidget::addActions(const QList<QAction *> &actions)
{
   for (int i = 0; i < actions.count(); ++i) {
      insertAction(0, actions.at(i));
   }
}

void QGraphicsWidget::insertAction(QAction *before, QAction *action)
{
   if (!action) {
      qWarning("QWidget::insertAction: Attempt to insert null action");
      return;
   }

   Q_D(QGraphicsWidget);
   int index = d->actions.indexOf(action);
   if (index != -1) {
      d->actions.removeAt(index);
   }

   int pos = d->actions.indexOf(before);
   if (pos < 0) {
      before = 0;
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

/*!
    \since 4.5

    Inserts the actions \a actions to this widget's list of actions,
    before the action \a before. It appends the action if \a before is 0 or
    \a before is not a valid action for this widget.

    A QGraphicsWidget can have at most one of each action.

    \sa removeAction(), QMenu, insertAction(), QWidget::insertActions()
*/
void QGraphicsWidget::insertActions(QAction *before, QList<QAction *> actions)
{
   for (int i = 0; i < actions.count(); ++i) {
      insertAction(before, actions.at(i));
   }
}

/*!
    \since 4.5

    Removes the action \a action from this widget's list of actions.

    \sa insertAction(), actions(), insertAction(), QWidget::removeAction()
*/
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

/*!
    \since 4.5

    Returns the (possibly empty) list of this widget's actions.

    \sa insertAction(), removeAction(), QWidget::actions(),
    QAction::associatedWidgets(), QAction::associatedGraphicsWidgets()
*/
QList<QAction *> QGraphicsWidget::actions() const
{
   Q_D(const QGraphicsWidget);
   return d->actions;
}
#endif

/*!
    Moves the \a second widget around the ring of focus widgets so that
    keyboard focus moves from the \a first widget to the \a second widget when
    the Tab key is pressed.

    Note that since the tab order of the \a second widget is changed, you
    should order a chain like this:

    \snippet doc/src/snippets/code/src_gui_graphicsview_qgraphicswidget.cpp 1

    \e not like this:

    \snippet doc/src/snippets/code/src_gui_graphicsview_qgraphicswidget.cpp 2

    If \a first is 0, this indicates that \a second should be the first widget
    to receive input focus should the scene gain Tab focus (i.e., the user
    hits Tab so that focus passes into the scene). If \a second is 0, this
    indicates that \a first should be the first widget to gain focus if the
    scene gained BackTab focus.

    By default, tab order is defined implicitly using widget creation order.

    \sa focusPolicy, {Keyboard Focus}
*/
void QGraphicsWidget::setTabOrder(QGraphicsWidget *first, QGraphicsWidget *second)
{
   if (!first && !second) {
      qWarning("QGraphicsWidget::setTabOrder(0, 0) is undefined");
      return;
   }
   if ((first && second) && first->scene() != second->scene()) {
      qWarning("QGraphicsWidget::setTabOrder: scenes %p and %p are different",
         first->scene(), second->scene());
      return;
   }
   QGraphicsScene *scene = first ? first->scene() : second->scene();
   if (!scene && (!first || !second)) {
      qWarning("QGraphicsWidget::setTabOrder: assigning tab order from/to the"
         " scene requires the item to be in a scene.");
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

/*!
    If \a on is true, this function enables \a attribute; otherwise
    \a attribute is disabled.

    See the class documentation for QGraphicsWidget for a complete list of
    which attributes are supported, and what they are for.

    \sa testAttribute(), QWidget::setAttribute()
*/
void QGraphicsWidget::setAttribute(Qt::WidgetAttribute attribute, bool on)
{
   Q_D(QGraphicsWidget);
   // ### most flags require some immediate action
   // ### we might want to qWarn use of unsupported attributes
   // ### we might want to not use Qt::WidgetAttribute, but roll our own instead
   d->setAttribute(attribute, on);
}

/*!
    Returns true if \a attribute is enabled for this widget; otherwise,
    returns false.

    \sa setAttribute()
*/
bool QGraphicsWidget::testAttribute(Qt::WidgetAttribute attribute) const
{
   Q_D(const QGraphicsWidget);
   return d->testAttribute(attribute);
}

/*!
    \reimp
*/
int QGraphicsWidget::type() const
{
   return Type;
}

/*!
    \reimp
*/
void QGraphicsWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   Q_UNUSED(painter);
   Q_UNUSED(option);
   Q_UNUSED(widget);
}

/*!
    This virtual function is called by QGraphicsScene to draw the window frame
    for windows using \a painter, \a option, and \a widget, in local
    coordinates. The base implementation uses the current style to render the
    frame and title bar.

    You can reimplement this function in a subclass of QGraphicsWidget to
    provide custom rendering of the widget's window frame.

    \sa QGraphicsItem::paint()
*/
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
   frameOptions.lineWidth = style()->pixelMetric(QStyle::PM_MdiSubWindowFrameWidth, 0, widget);
   frameOptions.midLineWidth = 1;
   style()->drawPrimitive(QStyle::PE_FrameWindow, &frameOptions, painter, widget);

#ifdef Q_OS_DARWIN
   realPainter->drawPixmap(QPoint(), pm);
   delete painter;
#endif
}

/*!
    \reimp
*/
QRectF QGraphicsWidget::boundingRect() const
{
   return windowFrameRect();
}

/*!
    \reimp
*/
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
