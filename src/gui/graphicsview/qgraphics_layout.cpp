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

#include <qapplication.h>

#ifndef QT_NO_GRAPHICSVIEW

#include <qgraphicslayout.h>
#include <qgraphicslayoutitem.h>
#include <qgraphicswidget.h>
#include <qgraphicsscene.h>

#include <qgraphics_layout_p.h>
#include <qgraphics_layoutitem_p.h>
#include <qgraphics_widget_p.h>

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
         qWarning("QGraphicsLayout::QGraphicsLayout() Unable to create a layout with a parent which is not "
            " a QGraphicsWidget or a QGraphicsLayout");
      }
   }
   d_func()->sizePolicy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, QSizePolicy::DefaultType);
   setOwnedByLayout(true);
}

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
         qWarning("QGraphicsLayout::QGraphicsLayout() Unable to create a layout with a parent which is not "
            " a QGraphicsWidget or a QGraphicsLayout");
      }
   }

   d_func()->sizePolicy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, QSizePolicy::DefaultType);
   setOwnedByLayout(true);
}

QGraphicsLayout::~QGraphicsLayout()
{
}

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

void QGraphicsLayout::getContentsMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const
{
   Q_D(const QGraphicsLayout);
   d->getMargin(left, d->left, QStyle::PM_LayoutLeftMargin);
   d->getMargin(top, d->top, QStyle::PM_LayoutTopMargin);
   d->getMargin(right, d->right, QStyle::PM_LayoutRightMargin);
   d->getMargin(bottom, d->bottom, QStyle::PM_LayoutBottomMargin);
}

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
