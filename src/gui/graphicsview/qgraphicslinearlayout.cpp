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

#include <qwidget.h>
#include <qgraphicslayout_p.h>
#include <qgraphicslayoutitem.h>
#include <qgraphicslinearlayout.h>
#include <qgraphicswidget.h>
#include <qgridlayoutengine_p.h>

#ifdef QT_DEBUG
#include <QtCore/qdebug.h>
#endif

QT_BEGIN_NAMESPACE

class QGraphicsLinearLayoutPrivate : public QGraphicsLayoutPrivate
{
 public:
   QGraphicsLinearLayoutPrivate(Qt::Orientation orientation) : orientation(orientation) { }

   void removeGridItem(QGridLayoutItem *gridItem);
   QLayoutStyleInfo styleInfo() const;
   void fixIndex(int *index) const;
   int gridRow(int index) const;
   int gridColumn(int index) const;

   Qt::Orientation orientation;
   QGridLayoutEngine engine;
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

Q_GLOBAL_STATIC(QWidget, globalStyleInfoWidget)

QLayoutStyleInfo QGraphicsLinearLayoutPrivate::styleInfo() const
{
   QGraphicsItem *item = parentItem();
   QStyle *style = (item && item->isWidget()) ? static_cast<QGraphicsWidget *>(item)->style() : QApplication::style();
   return QLayoutStyleInfo(style, globalStyleInfoWidget());
}

/*!
    Constructs a QGraphicsLinearLayout instance. You can pass the
    \a orientation for the layout, either horizontal or vertical, and
    \a parent is passed to QGraphicsLayout's constructor.
*/
QGraphicsLinearLayout::QGraphicsLinearLayout(Qt::Orientation orientation, QGraphicsLayoutItem *parent)
   : QGraphicsLayout(*new QGraphicsLinearLayoutPrivate(orientation), parent)
{
}

/*!
    Constructs a QGraphicsLinearLayout instance using Qt::Horizontal
    orientation. \a parent is passed to QGraphicsLayout's constructor.
*/
QGraphicsLinearLayout::QGraphicsLinearLayout(QGraphicsLayoutItem *parent)
   : QGraphicsLayout(*new QGraphicsLinearLayoutPrivate(Qt::Horizontal), parent)
{
}

/*!
    Destroys the QGraphicsLinearLayout object.
*/
QGraphicsLinearLayout::~QGraphicsLinearLayout()
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
  Change the layout orientation to \a orientation. Changing the layout
  orientation will automatically invalidate the layout.

  \sa orientation()
*/
void QGraphicsLinearLayout::setOrientation(Qt::Orientation orientation)
{
   Q_D(QGraphicsLinearLayout);
   if (orientation != d->orientation) {
      d->engine.transpose();
      d->orientation = orientation;
      invalidate();
   }
}

/*!
  Returns the layout orientation.
  \sa setOrientation()
 */
Qt::Orientation QGraphicsLinearLayout::orientation() const
{
   Q_D(const QGraphicsLinearLayout);
   return d->orientation;
}

/*!
    \fn void QGraphicsLinearLayout::addItem(QGraphicsLayoutItem *item)

    This convenience function is equivalent to calling
    insertItem(-1, \a item).
*/

/*!
    \fn void QGraphicsLinearLayout::addStretch(int stretch)

    This convenience function is equivalent to calling
    insertStretch(-1, \a stretch).
*/

/*!
    Inserts \a item into the layout at \a index, or before any item that is
    currently at \a index.

    \sa addItem(), itemAt(), insertStretch(), setItemSpacing()
*/
void QGraphicsLinearLayout::insertItem(int index, QGraphicsLayoutItem *item)
{
   Q_D(QGraphicsLinearLayout);
   if (!item) {
      qWarning("QGraphicsLinearLayout::insertItem: cannot insert null item");
      return;
   }
   if (item == this) {
      qWarning("QGraphicsLinearLayout::insertItem: cannot insert itself");
      return;
   }
   d->addChildLayoutItem(item);

   Q_ASSERT(item);
   d->fixIndex(&index);
   d->engine.insertRow(index, d->orientation);
   new QGridLayoutItem(&d->engine, item, d->gridRow(index), d->gridColumn(index), 1, 1, 0, index);
   invalidate();
}

/*!
    Inserts a stretch of \a stretch at \a index, or before any item that is
    currently at \a index.

    \sa addStretch(), setStretchFactor(), setItemSpacing(), insertItem()
*/
void QGraphicsLinearLayout::insertStretch(int index, int stretch)
{
   Q_D(QGraphicsLinearLayout);
   d->fixIndex(&index);
   d->engine.insertRow(index, d->orientation);
   d->engine.setRowStretchFactor(index, stretch, d->orientation);
   invalidate();
}

/*!
    Removes \a item from the layout without destroying it. Ownership of
    \a item is transferred to the caller.

    \sa removeAt(), insertItem()
*/
void QGraphicsLinearLayout::removeItem(QGraphicsLayoutItem *item)
{
   Q_D(QGraphicsLinearLayout);
   if (QGridLayoutItem *gridItem = d->engine.findLayoutItem(item)) {
      item->setParentLayoutItem(0);
      d->removeGridItem(gridItem);
      delete gridItem;
      invalidate();
   }
}

/*!
    Removes the item at \a index without destroying it. Ownership of the item
    is transferred to the caller.

    \sa removeItem(), insertItem()
*/
void QGraphicsLinearLayout::removeAt(int index)
{
   Q_D(QGraphicsLinearLayout);
   if (index < 0 || index >= d->engine.itemCount()) {
      qWarning("QGraphicsLinearLayout::removeAt: invalid index %d", index);
      return;
   }
   if (QGridLayoutItem *gridItem = d->engine.itemAt(index)) {
      if (QGraphicsLayoutItem *layoutItem = gridItem->layoutItem()) {
         layoutItem->setParentLayoutItem(0);
      }
      d->removeGridItem(gridItem);
      delete gridItem;
      invalidate();
   }
}

/*!
  Sets the layout's spacing to \a spacing. Spacing refers to the
  vertical and horizontal distances between items.

   \sa setItemSpacing(), setStretchFactor(), QGraphicsGridLayout::setSpacing()
*/
void QGraphicsLinearLayout::setSpacing(qreal spacing)
{
   Q_D(QGraphicsLinearLayout);
   if (spacing < 0) {
      qWarning("QGraphicsLinearLayout::setSpacing: invalid spacing %g", spacing);
      return;
   }
   d->engine.setSpacing(spacing, Qt::Horizontal | Qt::Vertical);
   invalidate();
}

/*!
  Returns the layout's spacing. Spacing refers to the
  vertical and horizontal distances between items.

  \sa setSpacing()
 */
qreal QGraphicsLinearLayout::spacing() const
{
   Q_D(const QGraphicsLinearLayout);
   return d->engine.spacing(d->styleInfo(), d->orientation);
}

/*!
    Sets the spacing after item at \a index to \a spacing.
*/
void QGraphicsLinearLayout::setItemSpacing(int index, qreal spacing)
{
   Q_D(QGraphicsLinearLayout);
   d->engine.setRowSpacing(index, spacing, d->orientation);
   invalidate();
}
/*!
    Returns the spacing after item at \a index.
*/
qreal QGraphicsLinearLayout::itemSpacing(int index) const
{
   Q_D(const QGraphicsLinearLayout);
   return d->engine.rowSpacing(index, d->orientation);
}

/*!
    Sets the stretch factor for \a item to \a stretch. If an item's stretch
    factor changes, this function will invalidate the layout.

    Setting \a stretch to 0 removes the stretch factor from the item, and is
    effectively equivalent to setting \a stretch to 1.

    \sa stretchFactor()
*/
void QGraphicsLinearLayout::setStretchFactor(QGraphicsLayoutItem *item, int stretch)
{
   Q_D(QGraphicsLinearLayout);
   if (!item) {
      qWarning("QGraphicsLinearLayout::setStretchFactor: cannot assign"
               " a stretch factor to a null item");
      return;
   }
   if (stretchFactor(item) == stretch) {
      return;
   }
   d->engine.setStretchFactor(item, stretch, d->orientation);
   invalidate();
}

/*!
    Returns the stretch factor for \a item. The default stretch factor is 0,
    meaning that the item has no assigned stretch factor.

    \sa setStretchFactor()
*/
int QGraphicsLinearLayout::stretchFactor(QGraphicsLayoutItem *item) const
{
   Q_D(const QGraphicsLinearLayout);
   if (!item) {
      qWarning("QGraphicsLinearLayout::setStretchFactor: cannot return"
               " a stretch factor for a null item");
      return 0;
   }
   return d->engine.stretchFactor(item, d->orientation);
}

/*!
    Sets the alignment of \a item to \a alignment. If \a item's alignment
    changes, the layout is automatically invalidated.

    \sa alignment(), invalidate()
*/
void QGraphicsLinearLayout::setAlignment(QGraphicsLayoutItem *item, Qt::Alignment alignment)
{
   Q_D(QGraphicsLinearLayout);
   if (this->alignment(item) == alignment) {
      return;
   }
   d->engine.setAlignment(item, alignment);
   invalidate();
}

/*!
    Returns the alignment for \a item. The default alignment is
    Qt::AlignTop | Qt::AlignLeft.

    The alignment decides how the item is positioned within its assigned space
    in the case where there's more space available in the layout than the
    widgets can occupy.

    \sa setAlignment()
*/
Qt::Alignment QGraphicsLinearLayout::alignment(QGraphicsLayoutItem *item) const
{
   Q_D(const QGraphicsLinearLayout);
   return d->engine.alignment(item);
}


/*!
    \reimp
*/
int QGraphicsLinearLayout::count() const
{
   Q_D(const QGraphicsLinearLayout);
   return d->engine.itemCount();
}

/*!
    \reimp
    When iterating from 0 and up, it will return the items in the visual arranged order.
*/
QGraphicsLayoutItem *QGraphicsLinearLayout::itemAt(int index) const
{
   Q_D(const QGraphicsLinearLayout);
   if (index < 0 || index >= d->engine.itemCount()) {
      qWarning("QGraphicsLinearLayout::itemAt: invalid index %d", index);
      return 0;
   }
   QGraphicsLayoutItem *item = 0;
   if (QGridLayoutItem *gridItem = d->engine.itemAt(index)) {
      item = gridItem->layoutItem();
   }
   return item;
}

/*!
    \reimp
*/
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
#ifdef QT_DEBUG
   if (qt_graphicsLayoutDebug()) {
      static int counter = 0;
      qDebug() << counter++ << "QGraphicsLinearLayout::setGeometry - " << rect;
      dump(1);
   }
#endif
   d->engine.setGeometries(d->styleInfo(), effectiveRect);
#ifdef QT_DEBUG
   if (qt_graphicsLayoutDebug()) {
      qDebug() << "post dump";
      dump(1);
   }
#endif
}

/*!
    \reimp
*/
QSizeF QGraphicsLinearLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
   Q_D(const QGraphicsLinearLayout);
   qreal left, top, right, bottom;
   getContentsMargins(&left, &top, &right, &bottom);
   const QSizeF extraMargins(left + right, top + bottom);
   return d->engine.sizeHint(d->styleInfo(), which , constraint - extraMargins) + extraMargins;
}

/*!
    \reimp
*/
void QGraphicsLinearLayout::invalidate()
{
   Q_D(QGraphicsLinearLayout);
   d->engine.invalidate();
   QGraphicsLayout::invalidate();
}

/*!
    \internal
*/
void QGraphicsLinearLayout::dump(int indent) const
{
#ifdef QT_DEBUG
   if (qt_graphicsLayoutDebug()) {
      Q_D(const QGraphicsLinearLayout);
      qDebug("%*s%s layout", indent, "",
             d->orientation == Qt::Horizontal ? "Horizontal" : "Vertical");
      d->engine.dump(indent + 1);
   }
#else
   Q_UNUSED(indent);
#endif
}

QT_END_NAMESPACE

#endif //QT_NO_GRAPHICSVIEW
