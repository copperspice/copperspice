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

#ifndef QGRAPHICSSCENEBSPTREEINDEX_P_H
#define QGRAPHICSSCENEBSPTREEINDEX_P_H

#include <QtCore/qglobal.h>

#if !defined(QT_NO_GRAPHICSVIEW)

#include <qgraphicssceneindex_p.h>
#include <qgraphicsitem_p.h>
#include <qgraphicsscene_bsp_p.h>
#include <QtCore/qrect.h>
#include <QtCore/qlist.h>



static const int QGRAPHICSSCENE_INDEXTIMER_TIMEOUT = 2000;

class QGraphicsScene;
class QGraphicsSceneBspTreeIndexPrivate;

class QGraphicsSceneBspTreeIndex : public QGraphicsSceneIndex
{
   GUI_CS_OBJECT(QGraphicsSceneBspTreeIndex)

   GUI_CS_PROPERTY_READ(bspTreeDepth, bspTreeDepth)
   GUI_CS_PROPERTY_WRITE(bspTreeDepth, setBspTreeDepth)

 public:
   QGraphicsSceneBspTreeIndex(QGraphicsScene *scene = 0);
   ~QGraphicsSceneBspTreeIndex();

   QList<QGraphicsItem *> estimateItems(const QRectF &rect, Qt::SortOrder order) const override;
   QList<QGraphicsItem *> estimateTopLevelItems(const QRectF &rect, Qt::SortOrder order) const override;
   QList<QGraphicsItem *> items(Qt::SortOrder order = Qt::DescendingOrder) const override;

   int bspTreeDepth() const;
   void setBspTreeDepth(int depth);

 protected :
   GUI_CS_SLOT_1(Protected, void updateSceneRect(const QRectF &rect) override)
   GUI_CS_SLOT_2(updateSceneRect)

   bool event(QEvent *event) override;
   void clear() override;

   void addItem(QGraphicsItem *item) override;
   void removeItem(QGraphicsItem *item) override;
   void prepareBoundingRectChange(const QGraphicsItem *item) override;

   void itemChange(const QGraphicsItem *item, QGraphicsItem::GraphicsItemChange change, const void *const value) override;

 private :
   Q_DECLARE_PRIVATE(QGraphicsSceneBspTreeIndex)
   Q_DISABLE_COPY(QGraphicsSceneBspTreeIndex)

   GUI_CS_SLOT_1(Private, void _q_updateSortCache())
   GUI_CS_SLOT_2(_q_updateSortCache)

   GUI_CS_SLOT_1(Private, void _q_updateIndex())
   GUI_CS_SLOT_2(_q_updateIndex)

   friend class QGraphicsScene;
   friend class QGraphicsScenePrivate;
};

class QGraphicsSceneBspTreeIndexPrivate : public QGraphicsSceneIndexPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsSceneBspTreeIndex)

 public:
   QGraphicsSceneBspTreeIndexPrivate(QGraphicsScene *scene);

   QGraphicsSceneBspTree bsp;
   QRectF sceneRect;
   int bspTreeDepth;
   int indexTimerId;
   bool restartIndexTimer;
   bool regenerateIndex;
   int lastItemCount;

   QList<QGraphicsItem *> indexedItems;
   QList<QGraphicsItem *> unindexedItems;
   QList<QGraphicsItem *> untransformableItems;
   QList<int> freeItemIndexes;

   bool purgePending;
   QSet<QGraphicsItem *> removedItems;
   void purgeRemovedItems();

   void _q_updateIndex();
   void startIndexTimer(int interval = QGRAPHICSSCENE_INDEXTIMER_TIMEOUT);
   void resetIndex();

   void _q_updateSortCache();
   bool sortCacheEnabled;
   bool updatingSortCache;
   void invalidateSortCache();
   void addItem(QGraphicsItem *item, bool recursive = false);
   void removeItem(QGraphicsItem *item, bool recursive = false, bool moveToUnindexedItems = false);
   QList<QGraphicsItem *> estimateItems(const QRectF &, Qt::SortOrder, bool b = false);

   static void climbTree(QGraphicsItem *item, int *stackingOrder);

   static inline bool closestItemFirst_withCache(const QGraphicsItem *item1, const QGraphicsItem *item2) {
      return item1->d_ptr->globalStackingOrder < item2->d_ptr->globalStackingOrder;
   }

   static inline bool closestItemLast_withCache(const QGraphicsItem *item1, const QGraphicsItem *item2) {
      return item1->d_ptr->globalStackingOrder >= item2->d_ptr->globalStackingOrder;
   }

   static void sortItems(QList<QGraphicsItem *> *itemList, Qt::SortOrder order, bool cached, bool onlyTopLevelItems = false);
};

static inline bool QRectF_intersects(const QRectF &s, const QRectF &r)
{
   qreal xp = s.left();
   qreal yp = s.top();
   qreal w = s.width();
   qreal h = s.height();
   qreal l1 = xp;
   qreal r1 = xp;

   if (w < 0) {
      l1 += w;
   } else {
      r1 += w;
   }

   qreal l2 = r.left();
   qreal r2 = r.left();
   if (w < 0) {
      l2 += r.width();
   } else {
      r2 += r.width();
   }

   if (l1 >= r2 || l2 >= r1) {
      return false;
   }

   qreal t1 = yp;
   qreal b1 = yp;
   if (h < 0) {
      t1 += h;
   } else {
      b1 += h;
   }

   qreal t2 = r.top();
   qreal b2 = r.top();
   if (r.height() < 0) {
      t2 += r.height();
   } else {
      b2 += r.height();
   }

   return !(t1 >= b2 || t2 >= b1);
}



#endif // QT_NO_GRAPHICSVIEW

#endif // QGRAPHICSBSPTREEINDEX_H
