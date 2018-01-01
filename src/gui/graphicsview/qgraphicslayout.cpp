/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qapplication.h>

#ifndef QT_NO_GRAPHICSVIEW
#include <qgraphicslayout.h>
#include <qgraphicslayout_p.h>
#include <qgraphicslayoutitem.h>
#include <qgraphicslayoutitem_p.h>
#include <qgraphicswidget.h>
#include <qgraphicswidget_p.h>
#include <qgraphicsscene.h>

QT_BEGIN_NAMESPACE

QGraphicsLayout::QGraphicsLayout(QGraphicsLayoutItem *parent)
   : QGraphicsLayoutItem(*new QGraphicsLayoutPrivate)
{
   setParentLayoutItem(parent);
   if (parent && !parent->isLayout()) {
      // If a layout has a parent that is not a layout it must be a QGraphicsWidget.
      QGraphicsItem *itemParent = parent->graphicsItem();
      if (itemParent && itemParent->isWidget()) {
         static_cast<QGraphicsWidget *>(itemParent)->d_func()->setLayout_helper(this);
      } else {
         qWarning("QGraphicsLayout::QGraphicsLayout: Attempt to create a layout with a parent that is"
                  " neither a QGraphicsWidget nor QGraphicsLayout");
      }
   }
   d_func()->sizePolicy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, QSizePolicy::DefaultType);
   setOwnedByLayout(true);
}

/*!
    \internal
*/
QGraphicsLayout::QGraphicsLayout(QGraphicsLayoutPrivate &dd, QGraphicsLayoutItem *parent)
   : QGraphicsLayoutItem(dd)
{
   setParentLayoutItem(parent);
   if (parent && !parent->isLayout()) {
      // If a layout has a parent that is not a layout it must be a QGraphicsWidget.
      QGraphicsItem *itemParent = parent->graphicsItem();
      if (itemParent && itemParent->isWidget()) {
         static_cast<QGraphicsWidget *>(itemParent)->d_func()->setLayout_helper(this);
      } else {
         qWarning("QGraphicsLayout::QGraphicsLayout: Attempt to create a layout with a parent that is"
                  " neither a QGraphicsWidget nor QGraphicsLayout");
      }
   }
   d_func()->sizePolicy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, QSizePolicy::DefaultType);
   setOwnedByLayout(true);
}

/*!
    Destroys the QGraphicsLayout object.
*/
QGraphicsLayout::~QGraphicsLayout()
{
}

/*!
    Sets the contents margins to \a left, \a top, \a right and \a bottom. The
    default contents margins for toplevel layouts are style dependent
    (by querying the pixelMetric for QStyle::PM_LayoutLeftMargin,
    QStyle::PM_LayoutTopMargin, QStyle::PM_LayoutRightMargin and
    QStyle::PM_LayoutBottomMargin).

    For sublayouts the default margins are 0.

    Changing the contents margins automatically invalidates the layout.

    \sa invalidate()
*/
void QGraphicsLayout::setContentsMargins(qreal left, qreal top, qreal right, qreal bottom)
{
   Q_D(QGraphicsLayout);
   if (d->left == left && d->top == top && d->right == right && d->bottom == bottom) {
      return;
   }
   d->left = left;
   d->right = right;
   d->top = top;
   d->bottom = bottom;
   invalidate();
}

/*!
    \reimp
*/
void QGraphicsLayout::getContentsMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const
{
   Q_D(const QGraphicsLayout);
   d->getMargin(left, d->left, QStyle::PM_LayoutLeftMargin);
   d->getMargin(top, d->top, QStyle::PM_LayoutTopMargin);
   d->getMargin(right, d->right, QStyle::PM_LayoutRightMargin);
   d->getMargin(bottom, d->bottom, QStyle::PM_LayoutBottomMargin);
}

/*!
    Activates the layout, causing all items in the layout to be immediately
    rearranged. This function is based on calling count() and itemAt(), and
    then calling setGeometry() on all items sequentially. When activated,
    the layout will adjust its geometry to its parent's contentsRect().
    The parent will then invalidate any layout of its own.

    If called in sequence or recursively, e.g., by one of the arranged items
    in response to being resized, this function will do nothing.

    Note that the layout is free to use geometry caching to optimize this
    process.  To forcefully invalidate any such cache, you can call
    invalidate() before calling activate().

    \sa invalidate()
*/
void QGraphicsLayout::activate()
{
   Q_D(QGraphicsLayout);
   if (d->activated) {
      return;
   }

   d->activateRecursive(this);

   // we don't call activate on a sublayout, but somebody might.
   // Therefore, we walk to the parentitem of the toplevel layout.
   QGraphicsLayoutItem *parentItem = this;
   while (parentItem && parentItem->isLayout()) {
      parentItem = parentItem->parentLayoutItem();
   }
   if (!parentItem) {
      return;
   }
   Q_ASSERT(!parentItem->isLayout());

   setGeometry(parentItem->contentsRect());    // relayout children
   if (!QGraphicsLayout::instantInvalidatePropagation()) {
      parentLayoutItem()->updateGeometry();
   }
}

/*!
    Returns true if the layout is currently being activated; otherwise,
    returns false. If the layout is being activated, this means that it is
    currently in the process of rearranging its items (i.e., the activate()
    function has been called, and has not yet returned).

    \sa activate(), invalidate()
*/
bool QGraphicsLayout::isActivated() const
{
   Q_D(const QGraphicsLayout);
   return d->activated;
}

/*!
    Clears any cached geometry and size hint information in the layout, and
    posts a \l{QEvent::LayoutRequest}{LayoutRequest} event to the managed
    parent QGraphicsLayoutItem.

    \sa activate(), setGeometry()
*/
void QGraphicsLayout::invalidate()
{
   if (QGraphicsLayout::instantInvalidatePropagation()) {
      updateGeometry();
   } else {
      // only mark layouts as invalid (activated = false) if we can post a LayoutRequest event.
      QGraphicsLayoutItem *layoutItem = this;
      while (layoutItem && layoutItem->isLayout()) {
         // we could call updateGeometry(), but what if that method
         // does not call the base implementation? In addition, updateGeometry()
         // does more than we need.
         layoutItem->d_func()->sizeHintCacheDirty = true;
         layoutItem->d_func()->sizeHintWithConstraintCacheDirty = true;
         layoutItem = layoutItem->parentLayoutItem();
      }
      if (layoutItem) {
         layoutItem->d_func()->sizeHintCacheDirty = true;
         layoutItem->d_func()->sizeHintWithConstraintCacheDirty = true;
      }

      bool postIt = layoutItem ? !layoutItem->isLayout() : false;
      if (postIt) {
         layoutItem = this;
         while (layoutItem && layoutItem->isLayout()
                && static_cast<QGraphicsLayout *>(layoutItem)->d_func()->activated) {
            static_cast<QGraphicsLayout *>(layoutItem)->d_func()->activated = false;
            layoutItem = layoutItem->parentLayoutItem();
         }
         if (layoutItem && !layoutItem->isLayout()) {
            // If a layout has a parent that is not a layout it must be a QGraphicsWidget.
            QApplication::postEvent(static_cast<QGraphicsWidget *>(layoutItem), new QEvent(QEvent::LayoutRequest));
         }
      }
   }
}

/*!
    \reimp
*/
void QGraphicsLayout::updateGeometry()
{
   Q_D(QGraphicsLayout);
   if (QGraphicsLayout::instantInvalidatePropagation()) {
      d->activated = false;
      QGraphicsLayoutItem::updateGeometry();

      QGraphicsLayoutItem *parentItem = parentLayoutItem();
      if (!parentItem) {
         return;
      }

      if (parentItem->isLayout()) {
         static_cast<QGraphicsLayout *>(parentItem)->invalidate();
      } else {
         parentItem->updateGeometry();
      }
   } else {
      QGraphicsLayoutItem::updateGeometry();
      if (QGraphicsLayoutItem *parentItem = parentLayoutItem()) {
         if (parentItem->isLayout()) {
            parentItem->updateGeometry();
         } else {
            invalidate();
         }
      }
   }
}

/*!
    This virtual event handler receives all events for the managed
    widget. QGraphicsLayout uses this event handler to listen for layout
    related events such as geometry changes, layout changes or layout
    direction changes.

    \a e is a pointer to the event.

    You can reimplement this event handler to track similar events for your
    own custom layout.

    \sa QGraphicsWidget::event(), QGraphicsItem::sceneEvent()
*/
void QGraphicsLayout::widgetEvent(QEvent *e)
{
   switch (e->type()) {
      case QEvent::GraphicsSceneResize:
         if (isActivated()) {
            setGeometry(parentLayoutItem()->contentsRect());
         } else {
            activate(); // relies on that activate() will call updateGeometry()
         }
         break;
      case QEvent::LayoutRequest:
         activate();
         break;
      case QEvent::LayoutDirectionChange:
         invalidate();
         break;
      default:
         break;
   }
}

/*!
    \fn virtual int QGraphicsLayout::count() const = 0

    This pure virtual function must be reimplemented in a subclass of
    QGraphicsLayout to return the number of items in the layout.

    The subclass is free to decide how to store the items.

    \sa itemAt(), removeAt()
*/

/*!
    \fn virtual QGraphicsLayoutItem *QGraphicsLayout::itemAt(int i) const = 0

    This pure virtual function must be reimplemented in a subclass of
    QGraphicsLayout to return a pointer to the item at index \a i. The
    reimplementation can assume that \a i is valid (i.e., it respects the
    value of count()).
    Together with count(), it is provided as a means of iterating over all items in a layout.

    The subclass is free to decide how to store the items, and the visual arrangement
    does not have to be reflected through this function.

    \sa count(), removeAt()
*/

/*!
    \fn virtual void QGraphicsLayout::removeAt(int index) = 0

    This pure virtual function must be reimplemented in a subclass of
    QGraphicsLayout to remove the item at \a index. The
    reimplementation can assume that \a index is valid (i.e., it
    respects the value of count()).

    The implementation must ensure that the parentLayoutItem() of
    the removed item does not point to this layout, since the item is
    considered to be removed from the layout hierarchy.

    If the layout is to be reused between applications, we recommend
    that the layout deletes the item, but the graphics view framework
    does not depend on this.

    The subclass is free to decide how to store the items.

    \sa itemAt(), count()
*/

/*!
    \since 4.6

    This function is a convenience function provided for custom layouts, and will go through
    all items in the layout and reparent their graphics items to the closest QGraphicsWidget
    ancestor of the layout.

    If \a layoutItem is already in a different layout, it will be removed  from that layout.

    If custom layouts want special behaviour they can ignore to use this function, and implement
    their own behaviour.

    \sa graphicsItem()
 */
void QGraphicsLayout::addChildLayoutItem(QGraphicsLayoutItem *layoutItem)
{
   Q_D(QGraphicsLayout);
   d->addChildLayoutItem(layoutItem);
}

static bool g_instantInvalidatePropagation = false;

/*!
    \since 4.8
    \sa instantInvalidatePropagation()

    Calling this function with \a enable set to true will enable a feature that
    makes propagation of invalidation up to ancestor layout items to be done in
    one go. It will propagate up the parentLayoutItem() hierarchy until it has
    reached the root. If the root item is a QGraphicsWidget, it will *post* a
    layout request to it. When the layout request is consumed it will traverse
    down the hierarchy of layouts and widgets and activate all layouts that is
    invalid (not activated). This is the recommended behaviour.

    If not set it will also propagate up the parentLayoutItem() hierarchy, but
    it will stop at the \e{first widget} it encounters, and post a layout
    request to the widget. When the layout request is consumed, this might
    cause it to continue propagation up to the parentLayoutItem() of the
    widget. It will continue in this fashion until it has reached a widget with
    no parentLayoutItem(). This strategy might cause drawing artifacts, since
    it is not done in one go, and the consumption of layout requests might be
    interleaved by consumption of paint events, which might cause significant
    flicker.
    Note, this is not the recommended behavior, but for compatibility reasons
    this is the default behaviour.
*/
void QGraphicsLayout::setInstantInvalidatePropagation(bool enable)
{
   g_instantInvalidatePropagation = enable;
}

/*!
    \since 4.8
    \sa setInstantInvalidatePropagation()

    returns true if the complete widget/layout hierarchy is rearranged in one go.
*/
bool QGraphicsLayout::instantInvalidatePropagation()
{
   return g_instantInvalidatePropagation;
}

QT_END_NAMESPACE

#endif //QT_NO_GRAPHICSVIEW
