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

   if (! parentItem) {
      return;
   }

   Q_ASSERT(!parentItem->isLayout());
   if (QGraphicsLayout::instantInvalidatePropagation()) {
      QGraphicsWidget *parentWidget = static_cast<QGraphicsWidget *>(parentItem);
      if (!parentWidget->parentLayoutItem()) {
         bool wasResized = parentWidget->testAttribute(Qt::WA_Resized);
         parentWidget->resize(parentWidget->size());
         parentWidget->setAttribute(Qt::WA_Resized, wasResized);
      }

      setGeometry(parentItem->contentsRect());    // relayout children

   } else {

      setGeometry(parentItem->contentsRect());    // relayout children

      parentLayoutItem()->updateGeometry();
   }
}


bool QGraphicsLayout::isActivated() const
{
   Q_D(const QGraphicsLayout);
   return d->activated;
}


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


void QGraphicsLayout::addChildLayoutItem(QGraphicsLayoutItem *layoutItem)
{
   Q_D(QGraphicsLayout);
   d->addChildLayoutItem(layoutItem);
}

static bool g_instantInvalidatePropagation = false;


void QGraphicsLayout::setInstantInvalidatePropagation(bool enable)
{
   g_instantInvalidatePropagation = enable;
}


bool QGraphicsLayout::instantInvalidatePropagation()
{
   return g_instantInvalidatePropagation;
}



#endif
