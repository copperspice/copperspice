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

#include <qmath.h>
#include <qdebug.h>

#include <qgraphics_scene_p.h>
#include <qgraphics_scenebsptreeindex_p.h>
#include <qgraphics_sceneindex_p.h>

static inline int intmaxlog(int n)
{
   return  (n > 0 ? qMax(qCeil(qLn(qreal(n)) / qLn(qreal(2))), 5) : 0);
}

QGraphicsSceneBspTreeIndexPrivate::QGraphicsSceneBspTreeIndexPrivate(QGraphicsScene *scene)
   : QGraphicsSceneIndexPrivate(scene),
     bspTreeDepth(0),
     indexTimerId(0),
     restartIndexTimer(false),
     regenerateIndex(true),
     lastItemCount(0),
     purgePending(false),
     sortCacheEnabled(false),
     updatingSortCache(false)
{
}

void QGraphicsSceneBspTreeIndexPrivate::_q_updateIndex()
{
   Q_Q(QGraphicsSceneBspTreeIndex);
   if (!indexTimerId) {
      return;
   }

   q->killTimer(indexTimerId);
   indexTimerId = 0;

   purgeRemovedItems();

   // Add unindexedItems to indexedItems
   for (int i = 0; i < unindexedItems.size(); ++i) {
      if (QGraphicsItem *item = unindexedItems.at(i)) {
         Q_ASSERT(!item->d_ptr->itemDiscovered);
         if (!freeItemIndexes.isEmpty()) {
            int freeIndex = freeItemIndexes.takeFirst();
            item->d_func()->index = freeIndex;
            indexedItems[freeIndex] = item;
         } else {
            item->d_func()->index = indexedItems.size();
            indexedItems << item;
         }
      }
   }

   static constexpr const int slack = 100;

   // Determine whether we should regenerate the BSP tree.
   if (bspTreeDepth == 0) {
      int oldDepth = intmaxlog(lastItemCount);
      bspTreeDepth = intmaxlog(indexedItems.size());

      if (bsp.leafCount() == 0 || (oldDepth != bspTreeDepth && qAbs(lastItemCount - indexedItems.size()) > slack)) {
         // ### Crude algorithm.
         regenerateIndex = true;
      }
   }

   // Regenerate the tree.
   if (regenerateIndex) {
      regenerateIndex = false;
      bsp.initialize(sceneRect, bspTreeDepth);
      unindexedItems = indexedItems;
      lastItemCount = indexedItems.size();
   }

   // Insert all unindexed items into the tree.
   for (int i = 0; i < unindexedItems.size(); ++i) {
      if (QGraphicsItem *item = unindexedItems.at(i)) {
         if (item->d_ptr->itemIsUntransformable()) {
            untransformableItems << item;
            continue;
         }

         if (item->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren
            || item->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorContainsChildren) {
            continue;
         }

         bsp.insertItem(item, item->d_ptr->sceneEffectiveBoundingRect());
      }
   }
   unindexedItems.clear();
}

void QGraphicsSceneBspTreeIndexPrivate::purgeRemovedItems()
{
   if (!purgePending && removedItems.isEmpty()) {
      return;
   }

   // Remove stale items from the BSP tree.
   bsp.removeItems(removedItems);
   // Purge this list.
   removedItems.clear();
   freeItemIndexes.clear();
   for (int i = 0; i < indexedItems.size(); ++i) {
      if (!indexedItems.at(i)) {
         freeItemIndexes << i;
      }
   }
   purgePending = false;
}

void QGraphicsSceneBspTreeIndexPrivate::startIndexTimer(int interval)
{
   Q_Q(QGraphicsSceneBspTreeIndex);
   if (indexTimerId) {
      restartIndexTimer = true;
   } else {
      indexTimerId = q->startTimer(interval);
   }
}

void QGraphicsSceneBspTreeIndexPrivate::resetIndex()
{
   purgeRemovedItems();
   for (int i = 0; i < indexedItems.size(); ++i) {
      if (QGraphicsItem *item = indexedItems.at(i)) {
         item->d_ptr->index = -1;
         Q_ASSERT(!item->d_ptr->itemDiscovered);
         unindexedItems << item;
      }
   }
   indexedItems.clear();
   freeItemIndexes.clear();
   untransformableItems.clear();
   regenerateIndex = true;
   startIndexTimer();
}

void QGraphicsSceneBspTreeIndexPrivate::climbTree(QGraphicsItem *item, int *stackingOrder)
{
   if (!item->d_ptr->children.isEmpty()) {
      QList<QGraphicsItem *> childList = item->d_ptr->children;
      std::sort(childList.begin(), childList.end(), qt_closestLeaf);

      for (int i = 0; i < childList.size(); ++i) {
         QGraphicsItem *item = childList.at(i);
         if (!(item->flags() & QGraphicsItem::ItemStacksBehindParent)) {
            climbTree(childList.at(i), stackingOrder);
         }
      }
      item->d_ptr->globalStackingOrder = (*stackingOrder)++;
      for (int i = 0; i < childList.size(); ++i) {
         QGraphicsItem *item = childList.at(i);
         if (item->flags() & QGraphicsItem::ItemStacksBehindParent) {
            climbTree(childList.at(i), stackingOrder);
         }
      }
   } else {
      item->d_ptr->globalStackingOrder = (*stackingOrder)++;
   }
}

void QGraphicsSceneBspTreeIndexPrivate::_q_updateSortCache()
{
   Q_Q(QGraphicsSceneBspTreeIndex);
   _q_updateIndex();

   if (!sortCacheEnabled || !updatingSortCache) {
      return;
   }

   updatingSortCache = false;
   int stackingOrder = 0;

   QList<QGraphicsItem *> topLevels;
   const QList<QGraphicsItem *> items = q->items();
   for (int i = 0; i < items.size(); ++i) {
      QGraphicsItem *item = items.at(i);
      if (item && !item->d_ptr->parent) {
         topLevels << item;
      }
   }

   std::sort(topLevels.begin(), topLevels.end(), qt_closestLeaf);
   for (int i = 0; i < topLevels.size(); ++i) {
      climbTree(topLevels.at(i), &stackingOrder);
   }
}

void QGraphicsSceneBspTreeIndexPrivate::invalidateSortCache()
{
   Q_Q(QGraphicsSceneBspTreeIndex);
   if (!sortCacheEnabled || updatingSortCache) {
      return;
   }

   updatingSortCache = true;
   QMetaObject::invokeMethod(q, "_q_updateSortCache", Qt::QueuedConnection);
}

void QGraphicsSceneBspTreeIndexPrivate::addItem(QGraphicsItem *item, bool recursive)
{
   if (!item) {
      return;
   }

   // Prevent reusing a recently deleted pointer: purge all removed item from our lists.
   purgeRemovedItems();

   // Invalidate any sort caching; arrival of a new item means we need to resort.
   // Update the scene's sort cache settings.
   item->d_ptr->globalStackingOrder = -1;
   invalidateSortCache();

   // Indexing requires sceneBoundingRect(), but because \a item might
   // not be completely constructed at this point, we need to store it in
   // a temporary list and schedule an indexing for later.
   if (item->d_ptr->index == -1) {
      Q_ASSERT(!unindexedItems.contains(item));
      unindexedItems << item;
      startIndexTimer(0);
   } else {
      Q_ASSERT(indexedItems.contains(item));
      qWarning("QGraphicsSceneBspTreeIndex::addItem() Item has already been added to this BSP index");
   }

   if (recursive) {
      for (int i = 0; i < item->d_ptr->children.size(); ++i) {
         addItem(item->d_ptr->children.at(i), recursive);
      }
   }
}

void QGraphicsSceneBspTreeIndexPrivate::removeItem(QGraphicsItem *item, bool recursive,
   bool moveToUnindexedItems)
{
   if (!item) {
      return;
   }

   if (item->d_ptr->index != -1) {
      Q_ASSERT(item->d_ptr->index < indexedItems.size());
      Q_ASSERT(indexedItems.at(item->d_ptr->index) == item);
      Q_ASSERT(!item->d_ptr->itemDiscovered);
      freeItemIndexes << item->d_ptr->index;
      indexedItems[item->d_ptr->index] = nullptr;
      item->d_ptr->index = -1;

      if (item->d_ptr->itemIsUntransformable()) {
         untransformableItems.removeOne(item);
      } else if (item->d_ptr->inDestructor) {
         // Avoid virtual function calls from the destructor.
         purgePending = true;
         removedItems << item;

      } else if (!(item->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren
            || item->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorContainsChildren)) {
         bsp.removeItem(item, item->d_ptr->sceneEffectiveBoundingRect());
      }

   } else {
      unindexedItems.removeOne(item);
   }
   invalidateSortCache(); // ### Only do this when removing from BSP?

   Q_ASSERT(item->d_ptr->index == -1);
   Q_ASSERT(!indexedItems.contains(item));
   Q_ASSERT(!unindexedItems.contains(item));
   Q_ASSERT(!untransformableItems.contains(item));

   if (moveToUnindexedItems) {
      addItem(item);
   }

   if (recursive) {
      for (int i = 0; i < item->d_ptr->children.size(); ++i) {
         removeItem(item->d_ptr->children.at(i), recursive, moveToUnindexedItems);
      }
   }
}

QList<QGraphicsItem *> QGraphicsSceneBspTreeIndexPrivate::estimateItems(const QRectF &rect, Qt::SortOrder order,
   bool onlyTopLevelItems)
{
   Q_Q(QGraphicsSceneBspTreeIndex);
   if (onlyTopLevelItems && rect.isNull()) {
      return q->QGraphicsSceneIndex::estimateTopLevelItems(rect, order);
   }

   purgeRemovedItems();
   _q_updateSortCache();
   Q_ASSERT(unindexedItems.isEmpty());

   QList<QGraphicsItem *> rectItems = bsp.items(rect, onlyTopLevelItems);
   if (onlyTopLevelItems) {
      for (int i = 0; i < untransformableItems.size(); ++i) {
         QGraphicsItem *item = untransformableItems.at(i);
         if (!item->d_ptr->parent) {
            rectItems << item;
         } else {
            item = item->topLevelItem();
            if (!rectItems.contains(item)) {
               rectItems << item;
            }
         }
      }
   } else {
      rectItems += untransformableItems;
   }

   sortItems(&rectItems, order, sortCacheEnabled, onlyTopLevelItems);
   return rectItems;
}

void QGraphicsSceneBspTreeIndexPrivate::sortItems(QList<QGraphicsItem *> *itemList, Qt::SortOrder order,
   bool sortCacheEnabled, bool onlyTopLevelItems)
{
   if (order == Qt::SortOrder(-1)) {
      return;
   }

   if (onlyTopLevelItems) {
      if (order == Qt::DescendingOrder) {
         std::sort(itemList->begin(), itemList->end(), qt_closestLeaf);
      } else if (order == Qt::AscendingOrder) {
         std::sort(itemList->begin(), itemList->end(), qt_notclosestLeaf);
      }
      return;
   }

   if (sortCacheEnabled) {
      if (order == Qt::DescendingOrder) {
         std::sort(itemList->begin(), itemList->end(), closestItemFirst_withCache);
      } else if (order == Qt::AscendingOrder) {
         std::sort(itemList->begin(), itemList->end(), closestItemLast_withCache);
      }
   } else {
      if (order == Qt::DescendingOrder) {
         std::sort(itemList->begin(), itemList->end(), qt_closestItemFirst);
      } else if (order == Qt::AscendingOrder) {
         std::sort(itemList->begin(), itemList->end(), qt_closestItemLast);
      }
   }
}

QGraphicsSceneBspTreeIndex::QGraphicsSceneBspTreeIndex(QGraphicsScene *scene)
   : QGraphicsSceneIndex(*new QGraphicsSceneBspTreeIndexPrivate(scene), scene)
{

}

QGraphicsSceneBspTreeIndex::~QGraphicsSceneBspTreeIndex()
{
   Q_D(QGraphicsSceneBspTreeIndex);
   for (int i = 0; i < d->indexedItems.size(); ++i) {
      // Ensure item bits are reset properly.
      if (QGraphicsItem *item = d->indexedItems.at(i)) {
         Q_ASSERT(!item->d_ptr->itemDiscovered);
         item->d_ptr->index = -1;
      }
   }
}

void QGraphicsSceneBspTreeIndex::clear()
{
   Q_D(QGraphicsSceneBspTreeIndex);
   d->bsp.clear();
   d->lastItemCount = 0;
   d->freeItemIndexes.clear();
   for (int i = 0; i < d->indexedItems.size(); ++i) {
      // Ensure item bits are reset properly.
      if (QGraphicsItem *item = d->indexedItems.at(i)) {
         Q_ASSERT(!item->d_ptr->itemDiscovered);
         item->d_ptr->index = -1;
      }
   }
   d->indexedItems.clear();
   d->unindexedItems.clear();
   d->untransformableItems.clear();
   d->regenerateIndex = true;
}

void QGraphicsSceneBspTreeIndex::addItem(QGraphicsItem *item)
{
   Q_D(QGraphicsSceneBspTreeIndex);
   d->addItem(item);
}


void QGraphicsSceneBspTreeIndex::removeItem(QGraphicsItem *item)
{
   Q_D(QGraphicsSceneBspTreeIndex);
   d->removeItem(item);
}

void QGraphicsSceneBspTreeIndex::prepareBoundingRectChange(const QGraphicsItem *item)
{
   if (!item) {
      return;
   }

   if (item->d_ptr->index == -1 || item->d_ptr->itemIsUntransformable()
      || (item->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren
         || item->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorContainsChildren)) {
      return; // Item is not in BSP tree; nothing to do.
   }

   Q_D(QGraphicsSceneBspTreeIndex);
   QGraphicsItem *thatItem = const_cast<QGraphicsItem *>(item);
   d->removeItem(thatItem, /*recursive=*/false, /*moveToUnindexedItems=*/true);
   for (int i = 0; i < item->d_ptr->children.size(); ++i) { // ### Do we really need this?
      prepareBoundingRectChange(item->d_ptr->children.at(i));
   }
}

QList<QGraphicsItem *> QGraphicsSceneBspTreeIndex::estimateItems(const QRectF &rect, Qt::SortOrder order) const
{
   Q_D(const QGraphicsSceneBspTreeIndex);
   return const_cast<QGraphicsSceneBspTreeIndexPrivate *>(d)->estimateItems(rect, order);
}

QList<QGraphicsItem *> QGraphicsSceneBspTreeIndex::estimateTopLevelItems(const QRectF &rect, Qt::SortOrder order) const
{
   Q_D(const QGraphicsSceneBspTreeIndex);
   return const_cast<QGraphicsSceneBspTreeIndexPrivate *>(d)->estimateItems(rect, order, /*onlyTopLevels=*/true);
}

QList<QGraphicsItem *> QGraphicsSceneBspTreeIndex::items(Qt::SortOrder order) const
{
   Q_D(const QGraphicsSceneBspTreeIndex);
   const_cast<QGraphicsSceneBspTreeIndexPrivate *>(d)->purgeRemovedItems();
   QList<QGraphicsItem *> itemList;

   // If freeItemIndexes is empty, we know there are no holes in indexedItems and
   // unindexedItems.
   if (d->freeItemIndexes.isEmpty()) {
      if (d->unindexedItems.isEmpty()) {
         itemList = d->indexedItems;
      } else {
         itemList = d->indexedItems + d->unindexedItems;
      }
   } else {
      // Rebuild the list of items to avoid holes. ### We could also just
      // compress the item lists at this point.
      for (QGraphicsItem *item : d->indexedItems + d->unindexedItems) {
         if (item) {
            itemList << item;
         }
      }
   }

   d->sortItems(&itemList, order, d->sortCacheEnabled);

   return itemList;
}

int QGraphicsSceneBspTreeIndex::bspTreeDepth() const
{
   Q_D(const QGraphicsSceneBspTreeIndex);
   return d->bspTreeDepth;
}

void QGraphicsSceneBspTreeIndex::setBspTreeDepth(int depth)
{
   Q_D(QGraphicsSceneBspTreeIndex);

   if (d->bspTreeDepth == depth) {
      return;
   }

   d->bspTreeDepth = depth;
   d->resetIndex();
}

void QGraphicsSceneBspTreeIndex::updateSceneRect(const QRectF &rect)
{
   Q_D(QGraphicsSceneBspTreeIndex);
   d->sceneRect = rect;
   d->resetIndex();
}

void QGraphicsSceneBspTreeIndex::itemChange(const QGraphicsItem *item, QGraphicsItem::GraphicsItemChange change,
   const void *const value)
{
   Q_D(QGraphicsSceneBspTreeIndex);

   switch (change) {
      case QGraphicsItem::ItemFlagsChange: {
         // Handle ItemIgnoresTransformations
         QGraphicsItem::GraphicsItemFlags newFlags = *static_cast<const QGraphicsItem::GraphicsItemFlags *>(value);

         bool ignoredTransform = item->d_ptr->itemFlags & QGraphicsItem::ItemIgnoresTransformations;
         bool willIgnoreTransform = newFlags & QGraphicsItem::ItemIgnoresTransformations;

         bool clipsChildren = item->d_ptr->itemFlags & QGraphicsItem::ItemClipsChildrenToShape
            || item->d_ptr->itemFlags & QGraphicsItem::ItemContainsChildrenInShape;

         bool willClipChildren = newFlags & QGraphicsItem::ItemClipsChildrenToShape
            || newFlags & QGraphicsItem::ItemContainsChildrenInShape;

         if ((ignoredTransform != willIgnoreTransform) || (clipsChildren != willClipChildren)) {
            QGraphicsItem *thatItem = const_cast<QGraphicsItem *>(item);
            // Remove item and its descendants from the index and append
            // them to the list of unindexed items. Then, when the index
            // is updated, they will be put into the bsp-tree or the list
            // of untransformable items.
            d->removeItem(thatItem, /*recursive=*/true, /*moveToUnidexedItems=*/true);
         }
         break;
      }

      case QGraphicsItem::ItemZValueChange:
         d->invalidateSortCache();
         break;

      case QGraphicsItem::ItemParentChange: {
         d->invalidateSortCache();

         // Handle ItemIgnoresTransformations
         const QGraphicsItem *newParent = static_cast<const QGraphicsItem *>(value);

         bool ignoredTransform = item->d_ptr->itemIsUntransformable();

         bool willIgnoreTransform = (item->d_ptr->itemFlags & QGraphicsItem::ItemIgnoresTransformations)
            || (newParent && newParent->d_ptr->itemIsUntransformable());

         bool ancestorClippedChildren = item->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren
            || item->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorContainsChildren;

         bool ancestorWillClipChildren = newParent
            && ((newParent->d_ptr->itemFlags & QGraphicsItem::ItemClipsChildrenToShape
                  || newParent->d_ptr->itemFlags & QGraphicsItem::ItemContainsChildrenInShape)
               || (newParent->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren
                  || newParent->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorContainsChildren));

         if ((ignoredTransform != willIgnoreTransform) || (ancestorClippedChildren != ancestorWillClipChildren)) {
            QGraphicsItem *thatItem = const_cast<QGraphicsItem *>(item);
            // Remove item and its descendants from the index and append
            // them to the list of unindexed items. Then, when the index
            // is updated, they will be put into the bsp-tree or the list
            // of untransformable items.
            d->removeItem(thatItem, /*recursive=*/true, /*moveToUnidexedItems=*/true);
         }
         break;
      }

      default:
         break;
   }
}

bool QGraphicsSceneBspTreeIndex::event(QEvent *event)
{
   Q_D(QGraphicsSceneBspTreeIndex);

   if (event->type() == QEvent::Timer) {
      if (d->indexTimerId && static_cast<QTimerEvent *>(event)->timerId() == d->indexTimerId) {
         if (d->restartIndexTimer) {
            d->restartIndexTimer = false;
         } else {
            // this call will kill the timer
            d->_q_updateIndex();
         }
      }
   }
   return QObject::event(event);
}

void QGraphicsSceneBspTreeIndex::_q_updateSortCache()
{
   Q_D(QGraphicsSceneBspTreeIndex);
   d->_q_updateSortCache();
}

void QGraphicsSceneBspTreeIndex::_q_updateIndex()
{
   Q_D(QGraphicsSceneBspTreeIndex);
   d->_q_updateIndex();
}

#endif

