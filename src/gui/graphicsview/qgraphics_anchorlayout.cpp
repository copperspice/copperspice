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

#include <qgraphics_anchorlayout_p.h>

#ifndef QT_NO_GRAPHICSVIEW

QGraphicsAnchor::QGraphicsAnchor(QGraphicsAnchorLayout *parentLayout)
   : QObject(nullptr), d_ptr(new QGraphicsAnchorPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QGraphicsAnchor);

   Q_ASSERT(parentLayout);
   d->layoutPrivate = parentLayout->d_func();
}

QGraphicsAnchor::~QGraphicsAnchor()
{
}

void QGraphicsAnchor::setSizePolicy(QSizePolicy::Policy policy)
{
   Q_D(QGraphicsAnchor);
   d->setSizePolicy(policy);
}

QSizePolicy::Policy QGraphicsAnchor::sizePolicy() const
{
   Q_D(const QGraphicsAnchor);
   return d->sizePolicy;
}

void QGraphicsAnchor::setSpacing(qreal spacing)
{
   Q_D(QGraphicsAnchor);
   d->setSpacing(spacing);
}

qreal QGraphicsAnchor::spacing() const
{
   Q_D(const QGraphicsAnchor);
   return d->spacing();
}

void QGraphicsAnchor::unsetSpacing()
{
   Q_D(QGraphicsAnchor);
   d->unsetSpacing();
}

QGraphicsAnchorLayout::QGraphicsAnchorLayout(QGraphicsLayoutItem *parent)
   : QGraphicsLayout(*new QGraphicsAnchorLayoutPrivate(), parent)
{
   Q_D(QGraphicsAnchorLayout);
   d->createLayoutEdges();
}

QGraphicsAnchorLayout::~QGraphicsAnchorLayout()
{
   Q_D(QGraphicsAnchorLayout);

   for (int i = count() - 1; i >= 0; --i) {
      QGraphicsLayoutItem *item = d->items.at(i);
      removeAt(i);
      if (item) {
         if (item->ownedByLayout()) {
            delete item;
         }
      }
   }

   d->removeCenterConstraints(this, QGraphicsAnchorLayoutPrivate::Horizontal);
   d->removeCenterConstraints(this, QGraphicsAnchorLayoutPrivate::Vertical);
   d->deleteLayoutEdges();

   Q_ASSERT(d->itemCenterConstraints[0].isEmpty());
   Q_ASSERT(d->itemCenterConstraints[1].isEmpty());
   Q_ASSERT(d->items.isEmpty());
   Q_ASSERT(d->m_vertexList.isEmpty());
}

QGraphicsAnchor *QGraphicsAnchorLayout::addAnchor(QGraphicsLayoutItem *firstItem, Qt::AnchorPoint firstEdge,
   QGraphicsLayoutItem *secondItem, Qt::AnchorPoint secondEdge)
{
   Q_D(QGraphicsAnchorLayout);
   QGraphicsAnchor *a = d->addAnchor(firstItem, firstEdge, secondItem, secondEdge);
   invalidate();
   return a;
}

QGraphicsAnchor *QGraphicsAnchorLayout::anchor(QGraphicsLayoutItem *firstItem, Qt::AnchorPoint firstEdge,
   QGraphicsLayoutItem *secondItem, Qt::AnchorPoint secondEdge)
{
   Q_D(QGraphicsAnchorLayout);
   return d->getAnchor(firstItem, firstEdge, secondItem, secondEdge);
}

void QGraphicsAnchorLayout::addCornerAnchors(QGraphicsLayoutItem *firstItem,
   Qt::Corner firstCorner,
   QGraphicsLayoutItem *secondItem,
   Qt::Corner secondCorner)
{
   Q_D(QGraphicsAnchorLayout);

   // Horizontal anchor
   Qt::AnchorPoint firstEdge = (firstCorner & 1 ? Qt::AnchorRight : Qt::AnchorLeft);
   Qt::AnchorPoint secondEdge = (secondCorner & 1 ? Qt::AnchorRight : Qt::AnchorLeft);
   if (d->addAnchor(firstItem, firstEdge, secondItem, secondEdge)) {
      // Vertical anchor
      firstEdge = (firstCorner & 2 ? Qt::AnchorBottom : Qt::AnchorTop);
      secondEdge = (secondCorner & 2 ? Qt::AnchorBottom : Qt::AnchorTop);
      d->addAnchor(firstItem, firstEdge, secondItem, secondEdge);

      invalidate();
   }
}

void QGraphicsAnchorLayout::addAnchors(QGraphicsLayoutItem *firstItem,
   QGraphicsLayoutItem *secondItem,
   Qt::Orientations orientations)
{
   bool ok = true;
   if (orientations & Qt::Horizontal) {
      // Currently, if the first is ok, then the rest of the calls should be ok
      ok = addAnchor(secondItem, Qt::AnchorLeft, firstItem, Qt::AnchorLeft) != nullptr;
      if (ok) {
         addAnchor(firstItem, Qt::AnchorRight, secondItem, Qt::AnchorRight);
      }
   }
   if (orientations & Qt::Vertical && ok) {
      addAnchor(secondItem, Qt::AnchorTop, firstItem, Qt::AnchorTop);
      addAnchor(firstItem, Qt::AnchorBottom, secondItem, Qt::AnchorBottom);
   }
}

void QGraphicsAnchorLayout::setHorizontalSpacing(qreal spacing)
{
   Q_D(QGraphicsAnchorLayout);

   d->spacings[0] = spacing;
   invalidate();
}

void QGraphicsAnchorLayout::setVerticalSpacing(qreal spacing)
{
   Q_D(QGraphicsAnchorLayout);

   d->spacings[1] = spacing;
   invalidate();
}

void QGraphicsAnchorLayout::setSpacing(qreal spacing)
{
   Q_D(QGraphicsAnchorLayout);

   d->spacings[0] = d->spacings[1] = spacing;
   invalidate();
}

qreal QGraphicsAnchorLayout::horizontalSpacing() const
{
   Q_D(const QGraphicsAnchorLayout);
   return d->styleInfo().defaultSpacing(Qt::Horizontal);
}

qreal QGraphicsAnchorLayout::verticalSpacing() const
{
   Q_D(const QGraphicsAnchorLayout);
   return d->styleInfo().defaultSpacing(Qt::Vertical);
}

void QGraphicsAnchorLayout::setGeometry(const QRectF &geom)
{
   Q_D(QGraphicsAnchorLayout);

   QGraphicsLayout::setGeometry(geom);
   d->calculateVertexPositions(QGraphicsAnchorLayoutPrivate::Horizontal);
   d->calculateVertexPositions(QGraphicsAnchorLayoutPrivate::Vertical);
   d->setItemsGeometries(geom);
}

void QGraphicsAnchorLayout::removeAt(int index)
{
   Q_D(QGraphicsAnchorLayout);
   QGraphicsLayoutItem *item = d->items.value(index);

   if (!item) {
      return;
   }

   // Removing an item affects both horizontal and vertical graphs
   d->removeCenterConstraints(item, QGraphicsAnchorLayoutPrivate::Horizontal);
   d->removeCenterConstraints(item, QGraphicsAnchorLayoutPrivate::Vertical);
   d->removeAnchors(item);
   d->items.remove(index);

   item->setParentLayoutItem(nullptr);
   invalidate();
}

int QGraphicsAnchorLayout::count() const
{
   Q_D(const QGraphicsAnchorLayout);
   return d->items.size();
}

QGraphicsLayoutItem *QGraphicsAnchorLayout::itemAt(int index) const
{
   Q_D(const QGraphicsAnchorLayout);
   return d->items.value(index);
}

void QGraphicsAnchorLayout::invalidate()
{
   Q_D(QGraphicsAnchorLayout);
   QGraphicsLayout::invalidate();
   d->calculateGraphCacheDirty = true;
   d->styleInfoDirty = true;
}

QSizeF QGraphicsAnchorLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
   (void) constraint;
   Q_D(const QGraphicsAnchorLayout);

   // Some setup calculations are delayed until the information is
   // actually needed, avoiding unnecessary recalculations when
   // adding multiple anchors.

   // sizeHint() / effectiveSizeHint() already have a cache
   // mechanism, using invalidate() to force recalculation. However
   // sizeHint() is called three times after invalidation (for max,
   // min and pref), but we just need do our setup once.

   const_cast<QGraphicsAnchorLayoutPrivate *>(d)->calculateGraphs();

   // ### apply constraint!
   QSizeF engineSizeHint(
      d->sizeHints[QGraphicsAnchorLayoutPrivate::Horizontal][which],
      d->sizeHints[QGraphicsAnchorLayoutPrivate::Vertical][which]);

   qreal left, top, right, bottom;
   getContentsMargins(&left, &top, &right, &bottom);

   return engineSizeHint + QSizeF(left + right, top + bottom);
}

#endif //QT_NO_GRAPHICSVIEW
