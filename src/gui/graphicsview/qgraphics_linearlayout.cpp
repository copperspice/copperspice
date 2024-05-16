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

#include <qwidget.h>
#include <qgraphicslayoutitem.h>
#include <qgraphicslinearlayout.h>
#include <qgraphicswidget.h>

#include <qgraphics_layout_p.h>
#include <qgraphics_layoutstyleinfo_p.h>
#include <qgraphics_gridlayoutengine_p.h>

class QGraphicsLinearLayoutPrivate : public QGraphicsLayoutPrivate
{
 public:
   QGraphicsLinearLayoutPrivate(Qt::Orientation orientation) : orientation(orientation)
   { }

   void removeGridItem(QGridLayoutItem *gridItem);
   QGraphicsLayoutStyleInfo *styleInfo() const;
   void fixIndex(int *index) const;
   int gridRow(int index) const;
   int gridColumn(int index) const;

   Qt::Orientation orientation;
   mutable QScopedPointer<QGraphicsLayoutStyleInfo> m_styleInfo;
   QGraphicsGridLayoutEngine engine;
};

void QGraphicsLinearLayoutPrivate::removeGridItem(QGridLayoutItem *gridItem)
{
   int index = gridItem->firstRow(orientation);
   engine.removeItem(gridItem);
   engine.removeRows(index, 1, orientation);
}

void QGraphicsLinearLayoutPrivate::fixIndex(int *index) const
{
   int count = engine.rowCount(orientation);
   if (uint(*index) > uint(count)) {
      *index = count;
   }
}

int QGraphicsLinearLayoutPrivate::gridRow(int index) const
{
   if (orientation == Qt::Horizontal) {
      return 0;
   }
   return int(qMin(uint(index), uint(engine.rowCount())));
}

int QGraphicsLinearLayoutPrivate::gridColumn(int index) const
{
   if (orientation == Qt::Vertical) {
      return 0;
   }
   return int(qMin(uint(index), uint(engine.columnCount())));
}


QGraphicsLayoutStyleInfo *QGraphicsLinearLayoutPrivate::styleInfo() const
{
   if (!m_styleInfo) {
      m_styleInfo.reset(new QGraphicsLayoutStyleInfo(this));
   }
   return m_styleInfo.data();
}

QGraphicsLinearLayout::QGraphicsLinearLayout(Qt::Orientation orientation, QGraphicsLayoutItem *parent)
   : QGraphicsLayout(*new QGraphicsLinearLayoutPrivate(orientation), parent)
{
}

QGraphicsLinearLayout::QGraphicsLinearLayout(QGraphicsLayoutItem *parent)
   : QGraphicsLayout(*new QGraphicsLinearLayoutPrivate(Qt::Horizontal), parent)
{
}

QGraphicsLinearLayout::~QGraphicsLinearLayout()
{
   for (int i = count() - 1; i >= 0; --i) {
      QGraphicsLayoutItem *item = itemAt(i);
      // The following lines can be removed, but this removes the item
      // from the layout more efficiently than the implementation of
      // ~QGraphicsLayoutItem.
      removeAt(i);

      if (item) {
         item->setParentLayoutItem(nullptr);
         if (item->ownedByLayout()) {
            delete item;
         }
      }
   }
}

void QGraphicsLinearLayout::setOrientation(Qt::Orientation orientation)
{
   Q_D(QGraphicsLinearLayout);
   if (orientation != d->orientation) {
      d->engine.transpose();
      d->orientation = orientation;
      invalidate();
   }
}

Qt::Orientation QGraphicsLinearLayout::orientation() const
{
   Q_D(const QGraphicsLinearLayout);
   return d->orientation;
}

void QGraphicsLinearLayout::insertItem(int index, QGraphicsLayoutItem *item)
{
   Q_D(QGraphicsLinearLayout);
   if (!item) {
      qWarning("QGraphicsLinearLayout::insertItem() Unable to insert an invalid item (nullptr)");
      return;
   }
   if (item == this) {
      qWarning("QGraphicsLinearLayout::insertItem() Item already exists, can not insert again");
      return;
   }
   d->addChildLayoutItem(item);

   Q_ASSERT(item);
   d->fixIndex(&index);
   d->engine.insertRow(index, d->orientation);
   QGraphicsGridLayoutEngineItem *gridEngineItem = new QGraphicsGridLayoutEngineItem(item, d->gridRow(index),
               d->gridColumn(index), 1, 1, Qt::EmptyFlag);
   d->engine.insertItem(gridEngineItem, index);
   invalidate();
}

void QGraphicsLinearLayout::insertStretch(int index, int stretch)
{
   Q_D(QGraphicsLinearLayout);
   d->fixIndex(&index);
   d->engine.insertRow(index, d->orientation);
   d->engine.setRowStretchFactor(index, stretch, d->orientation);
   invalidate();
}

void QGraphicsLinearLayout::removeItem(QGraphicsLayoutItem *item)
{
   Q_D(QGraphicsLinearLayout);
   if (QGraphicsGridLayoutEngineItem *gridItem = d->engine.findLayoutItem(item)) {
      item->setParentLayoutItem(nullptr);
      d->removeGridItem(gridItem);
      delete gridItem;
      invalidate();
   }
}

void QGraphicsLinearLayout::removeAt(int index)
{
   Q_D(QGraphicsLinearLayout);

   if (index < 0 || index >= d->engine.itemCount()) {
      qWarning("QGraphicsLinearLayout::removeAt() Invalid index %d", index);
      return;
   }

   if (QGraphicsGridLayoutEngineItem *gridItem = static_cast<QGraphicsGridLayoutEngineItem *>(d->engine.itemAt(index))) {
      if (QGraphicsLayoutItem *layoutItem = gridItem->layoutItem()) {
         layoutItem->setParentLayoutItem(nullptr);
      }

      d->removeGridItem(gridItem);
      delete gridItem;
      invalidate();
   }
}

void QGraphicsLinearLayout::setSpacing(qreal spacing)
{
   Q_D(QGraphicsLinearLayout);
   if (spacing < 0) {
      qWarning("QGraphicsLinearLayout::setSpacing() Invalid spacing %g", spacing);
      return;
   }
   d->engine.setSpacing(spacing, Qt::Horizontal | Qt::Vertical);
   invalidate();
}

qreal QGraphicsLinearLayout::spacing() const
{
   Q_D(const QGraphicsLinearLayout);
   return d->engine.spacing(d->orientation, d->styleInfo());
}

void QGraphicsLinearLayout::setItemSpacing(int index, qreal spacing)
{
   Q_D(QGraphicsLinearLayout);
   d->engine.setRowSpacing(index, spacing, d->orientation);
   invalidate();
}

qreal QGraphicsLinearLayout::itemSpacing(int index) const
{
   Q_D(const QGraphicsLinearLayout);
   return d->engine.rowSpacing(index, d->orientation);
}

void QGraphicsLinearLayout::setStretchFactor(QGraphicsLayoutItem *item, int stretch)
{
   Q_D(QGraphicsLinearLayout);
   if (!item) {
      qWarning("QGraphicsLinearLayout::setStretchFactor() Unable to assign a stretch factor to an invalid item (nullptr)");
      return;
   }
   if (stretchFactor(item) == stretch) {
      return;
   }
   d->engine.setStretchFactor(item, stretch, d->orientation);
   invalidate();
}

int QGraphicsLinearLayout::stretchFactor(QGraphicsLayoutItem *item) const
{
   Q_D(const QGraphicsLinearLayout);
   if (!item) {
      qWarning("QGraphicsLinearLayout::setStretchFactor() Unable to return a stretch factor for an invalid item (nullptr)");
      return 0;
   }
   return d->engine.stretchFactor(item, d->orientation);
}

void QGraphicsLinearLayout::setAlignment(QGraphicsLayoutItem *item, Qt::Alignment alignment)
{
   Q_D(QGraphicsLinearLayout);
   if (this->alignment(item) == alignment) {
      return;
   }
   d->engine.setAlignment(item, alignment);
   invalidate();
}

Qt::Alignment QGraphicsLinearLayout::alignment(QGraphicsLayoutItem *item) const
{
   Q_D(const QGraphicsLinearLayout);
   return d->engine.alignment(item);
}

int QGraphicsLinearLayout::count() const
{
   Q_D(const QGraphicsLinearLayout);
   return d->engine.itemCount();
}

QGraphicsLayoutItem *QGraphicsLinearLayout::itemAt(int index) const
{
   Q_D(const QGraphicsLinearLayout);
   if (index < 0 || index >= d->engine.itemCount()) {
      qWarning("QGraphicsLinearLayout::itemAt() Invalid index %d", index);
      return nullptr;
   }

   QGraphicsLayoutItem *item = nullptr;
   if (QGraphicsGridLayoutEngineItem *gridItem = static_cast<QGraphicsGridLayoutEngineItem *>(d->engine.itemAt(index))) {
      item = gridItem->layoutItem();
   }

   return item;
}

void QGraphicsLinearLayout::setGeometry(const QRectF &rect)
{
   Q_D(QGraphicsLinearLayout);

   QGraphicsLayout::setGeometry(rect);
   QRectF effectiveRect = geometry();
   qreal left, top, right, bottom;
   getContentsMargins(&left, &top, &right, &bottom);

   Qt::LayoutDirection visualDir = d->visualDirection();
   d->engine.setVisualDirection(visualDir);

   if (visualDir == Qt::RightToLeft) {
      qSwap(left, right);
   }

   effectiveRect.adjust(+left, +top, -right, -bottom);

   d->engine.setGeometries(effectiveRect, d->styleInfo());
}

QSizeF QGraphicsLinearLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
   Q_D(const QGraphicsLinearLayout);

   qreal left, top, right, bottom;
   getContentsMargins(&left, &top, &right, &bottom);
   const QSizeF extraMargins(left + right, top + bottom);

   return d->engine.sizeHint(which, constraint - extraMargins, d->styleInfo()) + extraMargins;
}

void QGraphicsLinearLayout::invalidate()
{
   Q_D(QGraphicsLinearLayout);

   d->engine.invalidate();
   if (d->m_styleInfo) {
      d->m_styleInfo->invalidate();
   }

   QGraphicsLayout::invalidate();
}

// internal
void QGraphicsLinearLayout::dump(int indent) const
{
   (void) indent;
}

#endif
