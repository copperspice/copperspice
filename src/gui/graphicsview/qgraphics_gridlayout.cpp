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

#include <qapplication.h>
#include <qdebug.h>
#include <qwidget.h>
#include <qgraphicslayoutitem.h>
#include <qgraphicsgridlayout.h>
#include <qgraphicswidget.h>
#include <qscopedpointer.h>

#include <qgraphics_layout_p.h>
#include <qgraphics_layoutstyleinfo_p.h>
#include <qgraphics_gridlayoutengine_p.h>

class QGraphicsGridLayoutPrivate : public QGraphicsLayoutPrivate
{
 public:
   QGraphicsGridLayoutPrivate() { }
   QGraphicsLayoutStyleInfo *styleInfo() const;

   mutable QScopedPointer<QGraphicsLayoutStyleInfo> m_styleInfo;
   QGraphicsGridLayoutEngine engine;

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
   void dump(int indent) const;
#endif
};

QGraphicsLayoutStyleInfo *QGraphicsGridLayoutPrivate::styleInfo() const
{
   if (!m_styleInfo) {
      m_styleInfo.reset(new QGraphicsLayoutStyleInfo(this));
   }

   return m_styleInfo.data();
}

QGraphicsGridLayout::QGraphicsGridLayout(QGraphicsLayoutItem *parent)
   : QGraphicsLayout(*new QGraphicsGridLayoutPrivate(), parent)
{
}

QGraphicsGridLayout::~QGraphicsGridLayout()
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

void QGraphicsGridLayout::addItem(QGraphicsLayoutItem *item, int row, int column,
   int rowSpan, int columnSpan, Qt::Alignment alignment)
{
   Q_D(QGraphicsGridLayout);
   if (row < 0 || column < 0) {
      qWarning("QGraphicsGridLayout::addItem() Invalid row/column: %d",
         row < 0 ? row : column);
      return;
   }
   if (columnSpan < 1 || rowSpan < 1) {
      qWarning("QGraphicsGridLayout::addItem() Invalid row span/column span: %d",
         rowSpan < 1 ? rowSpan : columnSpan);
      return;
   }
   if (!item) {
      qWarning("QGraphicsGridLayout::addItem() Unable to add an invalid item (nullptr)");
      return;
   }
   if (item == this) {
      qWarning("QGraphicsGridLayout::addItem() Unable to insert an item which has already been added");
      return;
   }

   d->addChildLayoutItem(item);

   QGraphicsGridLayoutEngineItem *gridEngineItem = new QGraphicsGridLayoutEngineItem(item, row, column, rowSpan, columnSpan, alignment);
   d->engine.insertItem(gridEngineItem, -1);
   invalidate();
}

void QGraphicsGridLayout::setHorizontalSpacing(qreal spacing)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setSpacing(spacing, Qt::Horizontal);
   invalidate();
}

qreal QGraphicsGridLayout::horizontalSpacing() const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.spacing(Qt::Horizontal, d->styleInfo());
}

void QGraphicsGridLayout::setVerticalSpacing(qreal spacing)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setSpacing(spacing, Qt::Vertical);
   invalidate();
}

qreal QGraphicsGridLayout::verticalSpacing() const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.spacing(Qt::Vertical, d->styleInfo());
}

void QGraphicsGridLayout::setSpacing(qreal spacing)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setSpacing(spacing, Qt::Horizontal | Qt::Vertical);
   invalidate();
}

void QGraphicsGridLayout::setRowSpacing(int row, qreal spacing)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSpacing(row, spacing, Qt::Vertical);
   invalidate();
}

qreal QGraphicsGridLayout::rowSpacing(int row) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSpacing(row, Qt::Vertical);
}

void QGraphicsGridLayout::setColumnSpacing(int column, qreal spacing)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSpacing(column, spacing, Qt::Horizontal);
   invalidate();
}

qreal QGraphicsGridLayout::columnSpacing(int column) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSpacing(column, Qt::Horizontal);
}

void QGraphicsGridLayout::setRowStretchFactor(int row, int stretch)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowStretchFactor(row, stretch, Qt::Vertical);
   invalidate();
}

int QGraphicsGridLayout::rowStretchFactor(int row) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowStretchFactor(row, Qt::Vertical);
}

void QGraphicsGridLayout::setColumnStretchFactor(int column, int stretch)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowStretchFactor(column, stretch, Qt::Horizontal);
   invalidate();
}

int QGraphicsGridLayout::columnStretchFactor(int column) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowStretchFactor(column, Qt::Horizontal);
}

void QGraphicsGridLayout::setRowMinimumHeight(int row, qreal height)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::MinimumSize, row, height, Qt::Vertical);
   invalidate();
}

qreal QGraphicsGridLayout::rowMinimumHeight(int row) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSizeHint(Qt::MinimumSize, row, Qt::Vertical);
}

void QGraphicsGridLayout::setRowPreferredHeight(int row, qreal height)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::PreferredSize, row, height, Qt::Vertical);
   invalidate();
}

qreal QGraphicsGridLayout::rowPreferredHeight(int row) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSizeHint(Qt::PreferredSize, row, Qt::Vertical);
}

void QGraphicsGridLayout::setRowMaximumHeight(int row, qreal height)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::MaximumSize, row, height, Qt::Vertical);
   invalidate();
}

qreal QGraphicsGridLayout::rowMaximumHeight(int row) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSizeHint(Qt::MaximumSize, row, Qt::Vertical);
}

void QGraphicsGridLayout::setRowFixedHeight(int row, qreal height)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::MinimumSize, row, height, Qt::Vertical);
   d->engine.setRowSizeHint(Qt::MaximumSize, row, height, Qt::Vertical);
   invalidate();
}

void QGraphicsGridLayout::setColumnMinimumWidth(int column, qreal width)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::MinimumSize, column, width, Qt::Horizontal);
   invalidate();
}

qreal QGraphicsGridLayout::columnMinimumWidth(int column) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSizeHint(Qt::MinimumSize, column, Qt::Horizontal);
}

void QGraphicsGridLayout::setColumnPreferredWidth(int column, qreal width)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::PreferredSize, column, width, Qt::Horizontal);
   invalidate();
}

qreal QGraphicsGridLayout::columnPreferredWidth(int column) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSizeHint(Qt::PreferredSize, column, Qt::Horizontal);
}

void QGraphicsGridLayout::setColumnMaximumWidth(int column, qreal width)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::MaximumSize, column, width, Qt::Horizontal);
   invalidate();
}

qreal QGraphicsGridLayout::columnMaximumWidth(int column) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSizeHint(Qt::MaximumSize, column, Qt::Horizontal);
}

void QGraphicsGridLayout::setColumnFixedWidth(int column, qreal width)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::MinimumSize, column, width, Qt::Horizontal);
   d->engine.setRowSizeHint(Qt::MaximumSize, column, width, Qt::Horizontal);
   invalidate();
}

void QGraphicsGridLayout::setRowAlignment(int row, Qt::Alignment alignment)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowAlignment(row, alignment, Qt::Vertical);
   invalidate();
}

Qt::Alignment QGraphicsGridLayout::rowAlignment(int row) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowAlignment(row, Qt::Vertical);
}

void QGraphicsGridLayout::setColumnAlignment(int column, Qt::Alignment alignment)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowAlignment(column, alignment, Qt::Horizontal);
   invalidate();
}

Qt::Alignment QGraphicsGridLayout::columnAlignment(int column) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowAlignment(column, Qt::Horizontal);
}

void QGraphicsGridLayout::setAlignment(QGraphicsLayoutItem *item, Qt::Alignment alignment)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setAlignment(item, alignment);
   invalidate();
}

Qt::Alignment QGraphicsGridLayout::alignment(QGraphicsLayoutItem *item) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.alignment(item);
}

int QGraphicsGridLayout::rowCount() const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.effectiveLastRow(Qt::Vertical) + 1;
}

int QGraphicsGridLayout::columnCount() const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.effectiveLastRow(Qt::Horizontal) + 1;
}

QGraphicsLayoutItem *QGraphicsGridLayout::itemAt(int row, int column) const
{
   Q_D(const QGraphicsGridLayout);
   if (row < 0 || row >= rowCount() || column < 0 || column >= columnCount()) {
      qWarning("QGraphicsGridLayout::itemAt() Invalid row or column %d, %d", row, column);
      return nullptr;
   }

   if (QGraphicsGridLayoutEngineItem *engineItem = static_cast<QGraphicsGridLayoutEngineItem *>(d->engine.itemAt(row, column))) {
      return engineItem->layoutItem();
   }

   return nullptr;
}

int QGraphicsGridLayout::count() const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.itemCount();
}

QGraphicsLayoutItem *QGraphicsGridLayout::itemAt(int index) const
{
   Q_D(const QGraphicsGridLayout);

   if (index < 0 || index >= d->engine.itemCount()) {
      qWarning("QGraphicsGridLayout::itemAt() Invalid index %d", index);
      return nullptr;
   }

   QGraphicsLayoutItem *item = nullptr;
   if (QGraphicsGridLayoutEngineItem *engineItem = static_cast<QGraphicsGridLayoutEngineItem *>(d->engine.itemAt(index))) {
      item = engineItem->layoutItem();
   }

   return item;
}

void QGraphicsGridLayout::removeAt(int index)
{
   Q_D(QGraphicsGridLayout);

   if (index < 0 || index >= d->engine.itemCount()) {
      qWarning("QGraphicsGridLayout::removeAt() Invalid index %d", index);
      return;
   }

   if (QGraphicsGridLayoutEngineItem *gridItem = static_cast<QGraphicsGridLayoutEngineItem *>(d->engine.itemAt(index))) {
      if (QGraphicsLayoutItem *layoutItem = gridItem->layoutItem()) {
         layoutItem->setParentLayoutItem(nullptr);
      }

      d->engine.removeItem(gridItem);

      // recalculate rowInfo.count if we remove an item that is on the right/bottommost row
      for (int j = 0; j < GridOrientation_Count; ++j) {
         const Qt::Orientation orient = (j == 0 ? Qt::Horizontal : Qt::Vertical);
         const int oldCount = d->engine.rowCount(orient);

         if (gridItem->lastRow(orient) == oldCount - 1) {
            const int newCount = d->engine.effectiveLastRow(orient) + 1;
            d->engine.removeRows(newCount, oldCount - newCount, orient);
         }
      }

      delete gridItem;
      invalidate();
   }
}

void QGraphicsGridLayout::removeItem(QGraphicsLayoutItem *item)
{
   Q_D(QGraphicsGridLayout);
   int index = d->engine.indexOf(item);
   removeAt(index);
}

void QGraphicsGridLayout::invalidate()
{
   Q_D(QGraphicsGridLayout);
   d->engine.invalidate();
   if (d->m_styleInfo) {
      d->m_styleInfo->invalidate();
   }
   QGraphicsLayout::invalidate();
}

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
void QGraphicsGridLayoutPrivate::dump(int indent) const
{
   engine.dump(indent + 1);

}
#endif

void QGraphicsGridLayout::setGeometry(const QRectF &rect)
{
   Q_D(QGraphicsGridLayout);

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

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
   static int counter = 0;

   qDebug("==== BEGIN DUMP OF QGraphicsGridLayout (%d)====", counter++);
   d->dump(1);
   qDebug("==== END DUMP OF QGraphicsGridLayout ====");

#endif
}

QSizeF QGraphicsGridLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
   Q_D(const QGraphicsGridLayout);

   qreal left, top, right, bottom;
   getContentsMargins(&left, &top, &right, &bottom);
   const QSizeF extraMargins(left + right, top + bottom);

   return d->engine.sizeHint(which, constraint - extraMargins, d->styleInfo()) + extraMargins;
}

#endif
