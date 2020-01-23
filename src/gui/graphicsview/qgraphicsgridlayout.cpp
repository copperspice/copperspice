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

#include <qapplication.h>
#include <qwidget.h>
#include <qgraphicslayout_p.h>
#include <qgraphicslayoutitem.h>
#include <qgraphicsgridlayout.h>
#include <qgraphicswidget.h>
#include <qscopedpointer.h>

#include <qgraphicsgridlayoutengine_p.h>
#include <qgraphicslayoutstyleinfo_p.h>

#include <qdebug.h>

class QGraphicsGridLayoutPrivate : public QGraphicsLayoutPrivate
{
 public:
   QGraphicsGridLayoutPrivate() { }
   QGraphicsLayoutStyleInfo *styleInfo() const;

   mutable QScopedPointer<QGraphicsLayoutStyleInfo> m_styleInfo;
   QGraphicsGridLayoutEngine engine;
#ifdef QGRIDLAYOUTENGINE_DEBUG
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
         item->setParentLayoutItem(0);
         if (item->ownedByLayout()) {
            delete item;
         }
      }
   }
}

/*!
    Adds \a item to the grid on \a row and \a column. You can specify a
    \a rowSpan and \a columnSpan and an optional \a alignment.
*/
void QGraphicsGridLayout::addItem(QGraphicsLayoutItem *item, int row, int column,
   int rowSpan, int columnSpan, Qt::Alignment alignment)
{
   Q_D(QGraphicsGridLayout);
   if (row < 0 || column < 0) {
      qWarning("QGraphicsGridLayout::addItem: invalid row/column: %d",
         row < 0 ? row : column);
      return;
   }
   if (columnSpan < 1 || rowSpan < 1) {
      qWarning("QGraphicsGridLayout::addItem: invalid row span/column span: %d",
         rowSpan < 1 ? rowSpan : columnSpan);
      return;
   }
   if (!item) {
      qWarning("QGraphicsGridLayout::addItem: cannot add null item");
      return;
   }
   if (item == this) {
      qWarning("QGraphicsGridLayout::addItem: cannot insert itself");
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

/*!
    Sets the default vertical spacing for the grid layout to \a spacing.
*/
void QGraphicsGridLayout::setVerticalSpacing(qreal spacing)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setSpacing(spacing, Qt::Vertical);
   invalidate();
}

/*!
    Returns the default vertical spacing for the grid layout.
*/
qreal QGraphicsGridLayout::verticalSpacing() const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.spacing(Qt::Vertical, d->styleInfo());
}

/*!
    Sets the grid layout's default spacing, both vertical and
    horizontal, to \a spacing.

    \sa rowSpacing(), columnSpacing()
*/
void QGraphicsGridLayout::setSpacing(qreal spacing)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setSpacing(spacing, Qt::Horizontal | Qt::Vertical);
   invalidate();
}

/*!
    Sets the spacing for \a row to \a spacing.
*/
void QGraphicsGridLayout::setRowSpacing(int row, qreal spacing)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSpacing(row, spacing, Qt::Vertical);
   invalidate();
}

/*!
    Returns the row spacing for \a row.
*/
qreal QGraphicsGridLayout::rowSpacing(int row) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSpacing(row, Qt::Vertical);
}

/*!
    Sets the spacing for \a column to \a spacing.
*/
void QGraphicsGridLayout::setColumnSpacing(int column, qreal spacing)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSpacing(column, spacing, Qt::Horizontal);
   invalidate();
}

/*!
    Returns the column spacing for \a column.
*/
qreal QGraphicsGridLayout::columnSpacing(int column) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSpacing(column, Qt::Horizontal);
}

/*!
    Sets the stretch factor for \a row to \a stretch.
*/
void QGraphicsGridLayout::setRowStretchFactor(int row, int stretch)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowStretchFactor(row, stretch, Qt::Vertical);
   invalidate();
}

/*!
    Returns the stretch factor for \a row.
*/
int QGraphicsGridLayout::rowStretchFactor(int row) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowStretchFactor(row, Qt::Vertical);
}

/*!
    Sets the stretch factor for \a column to \a stretch.
*/
void QGraphicsGridLayout::setColumnStretchFactor(int column, int stretch)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowStretchFactor(column, stretch, Qt::Horizontal);
   invalidate();
}

/*!
    Returns the stretch factor for \a column.
*/
int QGraphicsGridLayout::columnStretchFactor(int column) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowStretchFactor(column, Qt::Horizontal);
}

/*!
    Sets the minimum height for row, \a row, to \a height.
*/
void QGraphicsGridLayout::setRowMinimumHeight(int row, qreal height)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::MinimumSize, row, height, Qt::Vertical);
   invalidate();
}

/*!
    Returns the minimum height for row, \a row.
*/
qreal QGraphicsGridLayout::rowMinimumHeight(int row) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSizeHint(Qt::MinimumSize, row, Qt::Vertical);
}

/*!
    Sets the preferred height for row, \a row, to \a height.
*/
void QGraphicsGridLayout::setRowPreferredHeight(int row, qreal height)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::PreferredSize, row, height, Qt::Vertical);
   invalidate();
}

/*!
    Returns the preferred height for row, \a row.
*/
qreal QGraphicsGridLayout::rowPreferredHeight(int row) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSizeHint(Qt::PreferredSize, row, Qt::Vertical);
}

/*!
    Sets the maximum height for row, \a row, to \a height.
*/
void QGraphicsGridLayout::setRowMaximumHeight(int row, qreal height)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::MaximumSize, row, height, Qt::Vertical);
   invalidate();
}

/*!
    Returns the maximum height for row, \a row.
*/
qreal QGraphicsGridLayout::rowMaximumHeight(int row) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSizeHint(Qt::MaximumSize, row, Qt::Vertical);
}

/*!
    Sets the fixed height for row, \a row, to \a height.
*/
void QGraphicsGridLayout::setRowFixedHeight(int row, qreal height)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::MinimumSize, row, height, Qt::Vertical);
   d->engine.setRowSizeHint(Qt::MaximumSize, row, height, Qt::Vertical);
   invalidate();
}

/*!
    Sets the minimum width for \a column to \a width.
*/
void QGraphicsGridLayout::setColumnMinimumWidth(int column, qreal width)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::MinimumSize, column, width, Qt::Horizontal);
   invalidate();
}

/*!
    Returns the minimum width for \a column.
*/
qreal QGraphicsGridLayout::columnMinimumWidth(int column) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSizeHint(Qt::MinimumSize, column, Qt::Horizontal);
}

/*!
    Sets the preferred width for \a column to \a width.
*/
void QGraphicsGridLayout::setColumnPreferredWidth(int column, qreal width)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::PreferredSize, column, width, Qt::Horizontal);
   invalidate();
}

/*!
    Returns the preferred width for \a column.
*/
qreal QGraphicsGridLayout::columnPreferredWidth(int column) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSizeHint(Qt::PreferredSize, column, Qt::Horizontal);
}

/*!
    Sets the maximum width of \a column to \a width.
*/
void QGraphicsGridLayout::setColumnMaximumWidth(int column, qreal width)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::MaximumSize, column, width, Qt::Horizontal);
   invalidate();
}

/*!
    Returns the maximum width for \a column.
*/
qreal QGraphicsGridLayout::columnMaximumWidth(int column) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowSizeHint(Qt::MaximumSize, column, Qt::Horizontal);
}

/*!
    Sets the fixed width of \a column to \a width.
*/
void QGraphicsGridLayout::setColumnFixedWidth(int column, qreal width)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowSizeHint(Qt::MinimumSize, column, width, Qt::Horizontal);
   d->engine.setRowSizeHint(Qt::MaximumSize, column, width, Qt::Horizontal);
   invalidate();
}

/*!
    Sets the alignment of \a row to \a alignment.
*/
void QGraphicsGridLayout::setRowAlignment(int row, Qt::Alignment alignment)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowAlignment(row, alignment, Qt::Vertical);
   invalidate();
}

/*!
    Returns the alignment of \a row.
*/
Qt::Alignment QGraphicsGridLayout::rowAlignment(int row) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowAlignment(row, Qt::Vertical);
}

/*!
    Sets the alignment for \a column to \a alignment.
*/
void QGraphicsGridLayout::setColumnAlignment(int column, Qt::Alignment alignment)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setRowAlignment(column, alignment, Qt::Horizontal);
   invalidate();
}

/*!
    Returns the alignment for \a column.
*/
Qt::Alignment QGraphicsGridLayout::columnAlignment(int column) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.rowAlignment(column, Qt::Horizontal);
}

/*!
    Sets the alignment for \a item to \a alignment.
*/
void QGraphicsGridLayout::setAlignment(QGraphicsLayoutItem *item, Qt::Alignment alignment)
{
   Q_D(QGraphicsGridLayout);
   d->engine.setAlignment(item, alignment);
   invalidate();
}

/*!
    Returns the alignment for \a item.
*/
Qt::Alignment QGraphicsGridLayout::alignment(QGraphicsLayoutItem *item) const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.alignment(item);
}

/*!
    Returns the number of rows in the grid layout. This is always one more
    than the index of the last row that is occupied by a layout item (empty
    rows are counted except for those at the end).
*/
int QGraphicsGridLayout::rowCount() const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.effectiveLastRow(Qt::Vertical) + 1;
}

/*!
    Returns the number of columns in the grid layout. This is always one more
    than the index of  the last column that is occupied by a layout item (empty
    columns are counted except for those at the end).
*/
int QGraphicsGridLayout::columnCount() const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.effectiveLastRow(Qt::Horizontal) + 1;
}

/*!
    Returns a pointer to the layout item at (\a row, \a column).
*/
QGraphicsLayoutItem *QGraphicsGridLayout::itemAt(int row, int column) const
{
   Q_D(const QGraphicsGridLayout);
   if (row < 0 || row >= rowCount() || column < 0 || column >= columnCount()) {
      qWarning("QGraphicsGridLayout::itemAt: invalid row, column %d, %d", row, column);
      return 0;
   }
   if (QGraphicsGridLayoutEngineItem *engineItem = static_cast<QGraphicsGridLayoutEngineItem *>(d->engine.itemAt(row, column))) {
      return engineItem->layoutItem();
   }
   return 0;
}

/*!
    Returns the number of layout items in this grid layout.
*/
int QGraphicsGridLayout::count() const
{
   Q_D(const QGraphicsGridLayout);
   return d->engine.itemCount();
}

/*!
    Returns the layout item at \a index, or 0 if there is no layout item at
    this index.
*/
QGraphicsLayoutItem *QGraphicsGridLayout::itemAt(int index) const
{
   Q_D(const QGraphicsGridLayout);
   if (index < 0 || index >= d->engine.itemCount()) {
      qWarning("QGraphicsGridLayout::itemAt: invalid index %d", index);
      return 0;
   }
   QGraphicsLayoutItem *item = 0;
   if (QGraphicsGridLayoutEngineItem *engineItem = static_cast<QGraphicsGridLayoutEngineItem *>(d->engine.itemAt(index))) {
      item = engineItem->layoutItem();
   }
   return item;
}

void QGraphicsGridLayout::removeAt(int index)
{
   Q_D(QGraphicsGridLayout);
   if (index < 0 || index >= d->engine.itemCount()) {
      qWarning("QGraphicsGridLayout::removeAt: invalid index %d", index);
      return;
   }

   if (QGraphicsGridLayoutEngineItem *gridItem = static_cast<QGraphicsGridLayoutEngineItem *>(d->engine.itemAt(index))) {
      if (QGraphicsLayoutItem *layoutItem = gridItem->layoutItem()) {
         layoutItem->setParentLayoutItem(0);
      }

      d->engine.removeItem(gridItem);

      // recalculate rowInfo.count if we remove an item that is on the right/bottommost row
      for (int j = 0; j < NOrientations; ++j) {
         // 0: Hor, 1: Ver
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
/*!
    \reimp
*/
void QGraphicsGridLayout::invalidate()
{
   Q_D(QGraphicsGridLayout);
   d->engine.invalidate();
   if (d->m_styleInfo) {
      d->m_styleInfo->invalidate();
   }
   QGraphicsLayout::invalidate();
}

#ifdef QGRIDLAYOUTENGINE_DEBUG
void QGraphicsGridLayoutPrivate::dump(int indent) const
{
   if (qt_graphicsLayoutDebug()) {
      engine.dump(indent + 1);
   }
}
#endif

/*!
    Sets the bounding geometry of the grid layout to \a rect.
*/
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

#ifdef QGRIDLAYOUTENGINE_DEBUG
   if (qt_graphicsLayoutDebug()) {
      static int counter = 0;
      qDebug("==== BEGIN DUMP OF QGraphicsGridLayout (%d)====", counter++);
      d->dump(1);
      qDebug("==== END DUMP OF QGraphicsGridLayout ====");
   }
#endif
}

/*!
    \reimp
*/
QSizeF QGraphicsGridLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
   Q_D(const QGraphicsGridLayout);
   qreal left, top, right, bottom;
   getContentsMargins(&left, &top, &right, &bottom);
   const QSizeF extraMargins(left + right, top + bottom);
   return d->engine.sizeHint(which, constraint - extraMargins, d->styleInfo()) + extraMargins;
}



#endif
