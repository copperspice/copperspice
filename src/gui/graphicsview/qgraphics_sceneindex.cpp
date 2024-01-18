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

#include <qgraphicsscene.h>

#include <qdebug.h>
#include <qgraphicswidget.h>

#include <qgraphics_item_p.h>
#include <qgraphics_scene_p.h>
#include <qgraphics_sceneindex_p.h>
#include <qgraphics_scenebsptreeindex_p.h>

#ifndef QT_NO_GRAPHICSVIEW

namespace QtPrivate {

static bool intersect_rect(const QGraphicsItem *item, const QRectF &exposeRect, Qt::ItemSelectionMode mode,
   const QTransform &deviceTransform, const void *intersectData)
{
   (void) exposeRect;

   const QRectF sceneRect = *static_cast<const QRectF *>(intersectData);

   QRectF brect = item->boundingRect();
   _q_adjustRect(&brect);

   bool keep = true;
   const QGraphicsItemPrivate *itemd = QGraphicsItemPrivate::get(item);

   if (itemd->itemIsUntransformable()) {
      // Untransformable items; map the scene rect to item coordinates.
      const QTransform transform = item->deviceTransform(deviceTransform);
      QRectF itemRect = (deviceTransform * transform.inverted()).mapRect(sceneRect);
      if (mode == Qt::ContainsItemShape || mode == Qt::ContainsItemBoundingRect) {
         keep = itemRect.contains(brect) && itemRect != brect;
      } else {
         keep = itemRect.intersects(brect);
      }
      if (keep && (mode == Qt::ContainsItemShape || mode == Qt::IntersectsItemShape)) {
         QPainterPath itemPath;
         itemPath.addRect(itemRect);
         keep = QGraphicsSceneIndexPrivate::itemCollidesWithPath(item, itemPath, mode);
      }

   } else {
      Q_ASSERT(!itemd->dirtySceneTransform);
      const QRectF itemSceneBoundingRect = itemd->sceneTransformTranslateOnly
         ? brect.translated(itemd->sceneTransform.dx(),
            itemd->sceneTransform.dy())
         : itemd->sceneTransform.mapRect(brect);
      if (mode == Qt::ContainsItemShape || mode == Qt::ContainsItemBoundingRect) {
         keep = sceneRect != brect && sceneRect.contains(itemSceneBoundingRect);
      } else {
         keep = sceneRect.intersects(itemSceneBoundingRect);
      }
      if (keep && (mode == Qt::ContainsItemShape || mode == Qt::IntersectsItemShape)) {
         QPainterPath rectPath;
         rectPath.addRect(sceneRect);
         if (itemd->sceneTransformTranslateOnly) {
            rectPath.translate(-itemd->sceneTransform.dx(), -itemd->sceneTransform.dy());
         } else {
            rectPath = itemd->sceneTransform.inverted().map(rectPath);
         }
         keep = QGraphicsSceneIndexPrivate::itemCollidesWithPath(item, rectPath, mode);
      }
   }
   return keep;
}

static bool intersect_point(const QGraphicsItem *item, const QRectF &exposeRect, Qt::ItemSelectionMode mode,
   const QTransform &deviceTransform, const void *intersectData)
{
   (void) exposeRect;

   const QPointF scenePoint = *static_cast<const QPointF *>(intersectData);

   QRectF brect = item->boundingRect();
   _q_adjustRect(&brect);

   bool keep = false;
   const QGraphicsItemPrivate *itemd = QGraphicsItemPrivate::get(item);

   if (itemd->itemIsUntransformable()) {
      // Untransformable items; map the scene point to item coordinates.
      const QTransform transform = item->deviceTransform(deviceTransform);
      QPointF itemPoint = (deviceTransform * transform.inverted()).map(scenePoint);
      keep = brect.contains(itemPoint);
      if (keep && (mode == Qt::ContainsItemShape || mode == Qt::IntersectsItemShape)) {
         QPainterPath pointPath;
         pointPath.addRect(QRectF(itemPoint, QSizeF(1, 1)));
         keep = QGraphicsSceneIndexPrivate::itemCollidesWithPath(item, pointPath, mode);
      }

   } else {
      Q_ASSERT(!itemd->dirtySceneTransform);
      QRectF sceneBoundingRect = itemd->sceneTransformTranslateOnly
         ? brect.translated(itemd->sceneTransform.dx(),
            itemd->sceneTransform.dy())
         : itemd->sceneTransform.mapRect(brect);
      keep = sceneBoundingRect.intersects(QRectF(scenePoint, QSizeF(1, 1)));
      if (keep && (mode == Qt::ContainsItemShape || mode == Qt::IntersectsItemShape)) {
         QPointF p = itemd->sceneTransformTranslateOnly
            ? QPointF(scenePoint.x() - itemd->sceneTransform.dx(),
               scenePoint.y() - itemd->sceneTransform.dy())
            : itemd->sceneTransform.inverted().map(scenePoint);
         keep = item->contains(p);
      }
   }

   return keep;
}

static bool intersect_path(const QGraphicsItem *item, const QRectF &exposeRect, Qt::ItemSelectionMode mode,
   const QTransform &deviceTransform, const void *intersectData)
{
   (void) exposeRect;

   const QPainterPath scenePath = *static_cast<const QPainterPath *>(intersectData);
   QRectF brect = item->boundingRect();
   _q_adjustRect(&brect);

   bool keep = true;
   const QGraphicsItemPrivate *itemd = QGraphicsItemPrivate::get(item);

   if (itemd->itemIsUntransformable()) {
      // Untransformable items; map the scene rect to item coordinates.
      const QTransform transform = item->deviceTransform(deviceTransform);
      QPainterPath itemPath = (deviceTransform * transform.inverted()).map(scenePath);
      if (mode == Qt::ContainsItemShape || mode == Qt::ContainsItemBoundingRect) {
         keep = itemPath.contains(brect);
      } else {
         keep = itemPath.intersects(brect);
      }
      if (keep && (mode == Qt::ContainsItemShape || mode == Qt::IntersectsItemShape)) {
         keep = QGraphicsSceneIndexPrivate::itemCollidesWithPath(item, itemPath, mode);
      }
   } else {
      Q_ASSERT(!itemd->dirtySceneTransform);
      const QRectF itemSceneBoundingRect = itemd->sceneTransformTranslateOnly
         ? brect.translated(itemd->sceneTransform.dx(),
            itemd->sceneTransform.dy())
         : itemd->sceneTransform.mapRect(brect);
      if (mode == Qt::ContainsItemShape || mode == Qt::ContainsItemBoundingRect) {
         keep = scenePath.contains(itemSceneBoundingRect);
      } else {
         keep = scenePath.intersects(itemSceneBoundingRect);
      }
      if (keep && (mode == Qt::ContainsItemShape || mode == Qt::IntersectsItemShape)) {
         QPainterPath itemPath = itemd->sceneTransformTranslateOnly
            ? scenePath.translated(-itemd->sceneTransform.dx(),
               -itemd->sceneTransform.dy())
            : itemd->sceneTransform.inverted().map(scenePath);
         keep = QGraphicsSceneIndexPrivate::itemCollidesWithPath(item, itemPath, mode);
      }
   }
   return keep;
}


} // namespace QtPrivate

QGraphicsSceneIndexPrivate::QGraphicsSceneIndexPrivate(QGraphicsScene *scene) : scene(scene)
{
}

QGraphicsSceneIndexPrivate::~QGraphicsSceneIndexPrivate()
{
}

bool QGraphicsSceneIndexPrivate::itemCollidesWithPath(const QGraphicsItem *item,
   const QPainterPath &path, Qt::ItemSelectionMode mode)
{
   if (item->collidesWithPath(path, mode)) {
      return true;
   }

   if (item->isWidget()) {
      // Check if this is a window, and if its frame rect collides.
      const QGraphicsWidget *widget = static_cast<const QGraphicsWidget *>(item);
      if (widget->isWindow()) {
         QRectF frameRect = widget->windowFrameRect();
         QPainterPath framePath;
         framePath.addRect(frameRect);
         bool intersects = path.intersects(frameRect);
         if (mode == Qt::IntersectsItemShape || mode == Qt::IntersectsItemBoundingRect)
            return intersects || path.contains(frameRect.topLeft())
               || framePath.contains(path.elementAt(0));
         return !intersects && path.contains(frameRect.topLeft());
      }
   }

   return false;
}

void QGraphicsSceneIndexPrivate::recursive_items_helper(QGraphicsItem *item, QRectF exposeRect,
   QGraphicsSceneIndexIntersector intersect, QList<QGraphicsItem *> *items,
   const QTransform &viewTransform, Qt::ItemSelectionMode mode,
   qreal parentOpacity, const void *intersectData) const
{
   Q_ASSERT(item);

   if (!item->d_ptr->visible) {
      return;
   }

   const qreal opacity = item->d_ptr->combineOpacityFromParent(parentOpacity);
   const bool itemIsFullyTransparent = QGraphicsItemPrivate::isOpacityNull(opacity);
   const bool itemHasChildren = !item->d_ptr->children.isEmpty();

   if (itemIsFullyTransparent && (!itemHasChildren || item->d_ptr->childrenCombineOpacity())) {
      return;
   }

   // Update the item's scene transform if dirty.
   const bool itemIsUntransformable = item->d_ptr->itemIsUntransformable();
   const bool wasDirtyParentSceneTransform = item->d_ptr->dirtySceneTransform && !itemIsUntransformable;

   if (wasDirtyParentSceneTransform) {
      item->d_ptr->updateSceneTransformFromParent();
      Q_ASSERT(!item->d_ptr->dirtySceneTransform);
   }

   const bool itemClipsChildrenToShape = (item->d_ptr->itemFlags & QGraphicsItem::ItemClipsChildrenToShape
         || item->d_ptr->itemFlags & QGraphicsItem::ItemContainsChildrenInShape);

   bool processItem = !itemIsFullyTransparent;

   if (processItem) {
      processItem = intersect(item, exposeRect, mode, viewTransform, intersectData);
      if (!processItem && (!itemHasChildren || itemClipsChildrenToShape)) {
         if (wasDirtyParentSceneTransform) {
            item->d_ptr->invalidateChildrenSceneTransform();
         }
         return;
      }
   } // else we know for sure this item has children we must process.

   int i = 0;
   if (itemHasChildren) {
      // Sort children.
      item->d_ptr->ensureSortedChildren();

      // Clip to shape.
      if (itemClipsChildrenToShape && !itemIsUntransformable) {
         QPainterPath mappedShape = item->d_ptr->sceneTransformTranslateOnly
            ? item->shape().translated(item->d_ptr->sceneTransform.dx(),
               item->d_ptr->sceneTransform.dy())
            : item->d_ptr->sceneTransform.map(item->shape());
         exposeRect &= mappedShape.controlPointRect();
      }

      // Process children behind
      for (i = 0; i < item->d_ptr->children.size(); ++i) {
         QGraphicsItem *child = item->d_ptr->children.at(i);
         if (wasDirtyParentSceneTransform) {
            child->d_ptr->dirtySceneTransform = 1;
         }

         if (!(child->d_ptr->itemFlags & QGraphicsItem::ItemStacksBehindParent)) {
            break;
         }

         if (itemIsFullyTransparent && ! (child->d_ptr->itemFlags & QGraphicsItem::ItemIgnoresParentOpacity)) {
            continue;
         }

         recursive_items_helper(child, exposeRect, intersect, items, viewTransform,
            mode, opacity, intersectData);
      }
   }

   // Process item
   if (processItem) {
      items->append(item);
   }

   // Process children in front
   if (itemHasChildren) {
      for (; i < item->d_ptr->children.size(); ++i) {
         QGraphicsItem *child = item->d_ptr->children.at(i);

         if (wasDirtyParentSceneTransform) {
            child->d_ptr->dirtySceneTransform = 1;
         }

         if (itemIsFullyTransparent && !(child->d_ptr->itemFlags & QGraphicsItem::ItemIgnoresParentOpacity)) {
            continue;
         }

         recursive_items_helper(child, exposeRect, intersect, items, viewTransform,
            mode, opacity, intersectData);
      }
   }
}

void QGraphicsSceneIndexPrivate::init()
{
   if (! scene) {
      return;
   }

   QObject::connect(scene, &QGraphicsScene::sceneRectChanged, q_func(), &QGraphicsSceneIndex::updateSceneRect);
}

QGraphicsSceneIndex::QGraphicsSceneIndex(QGraphicsScene *scene)
   : QObject(scene), d_ptr(new QGraphicsSceneIndexPrivate(scene) )
{
   d_ptr->q_ptr = this;
   d_func()->init();
}

// internal
QGraphicsSceneIndex::QGraphicsSceneIndex(QGraphicsSceneIndexPrivate &dd, QGraphicsScene *scene)
   : QObject(scene), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   d_func()->init();
}

QGraphicsSceneIndex::~QGraphicsSceneIndex()
{

}

QGraphicsScene *QGraphicsSceneIndex::scene() const
{
   Q_D(const QGraphicsSceneIndex);
   return d->scene;
}


QList<QGraphicsItem *> QGraphicsSceneIndex::items(const QPointF &pos, Qt::ItemSelectionMode mode,
   Qt::SortOrder order, const QTransform &deviceTransform) const
{

   Q_D(const QGraphicsSceneIndex);
   QList<QGraphicsItem *> itemList;
   d->items_helper(QRectF(pos, QSizeF(1, 1)), &QtPrivate::intersect_point, &itemList, deviceTransform, mode, order, &pos);

   return itemList;
}

QList<QGraphicsItem *> QGraphicsSceneIndex::items(const QRectF &rect, Qt::ItemSelectionMode mode,
   Qt::SortOrder order, const QTransform &deviceTransform) const
{
   Q_D(const QGraphicsSceneIndex);
   QRectF exposeRect = rect;
   _q_adjustRect(&exposeRect);

   QList<QGraphicsItem *> itemList;
   d->items_helper(exposeRect, &QtPrivate::intersect_rect, &itemList, deviceTransform, mode, order, &rect);

   return itemList;
}

QList<QGraphicsItem *> QGraphicsSceneIndex::items(const QPolygonF &polygon, Qt::ItemSelectionMode mode,
   Qt::SortOrder order, const QTransform &deviceTransform) const
{
   Q_D(const QGraphicsSceneIndex);
   QList<QGraphicsItem *> itemList;
   QRectF exposeRect = polygon.boundingRect();
   _q_adjustRect(&exposeRect);
   QPainterPath path;
   path.addPolygon(polygon);
   d->items_helper(exposeRect, &QtPrivate::intersect_path, &itemList, deviceTransform, mode, order, &path);

   return itemList;
}

QList<QGraphicsItem *> QGraphicsSceneIndex::items(const QPainterPath &path, Qt::ItemSelectionMode mode,
   Qt::SortOrder order, const QTransform &deviceTransform) const
{
   Q_D(const QGraphicsSceneIndex);
   QList<QGraphicsItem *> itemList;
   QRectF exposeRect = path.controlPointRect();
   _q_adjustRect(&exposeRect);
   d->items_helper(exposeRect, &QtPrivate::intersect_path, &itemList, deviceTransform, mode, order, &path);

   return itemList;
}

QList<QGraphicsItem *> QGraphicsSceneIndex::estimateItems(const QPointF &point, Qt::SortOrder order) const
{
   return estimateItems(QRectF(point, QSize(1, 1)), order);
}

QList<QGraphicsItem *> QGraphicsSceneIndex::estimateTopLevelItems(const QRectF &rect, Qt::SortOrder order) const
{
   (void) rect;

   Q_D(const QGraphicsSceneIndex);

   QGraphicsScenePrivate *scened = d->scene->d_func();
   scened->ensureSortedTopLevelItems();

   if (order == Qt::DescendingOrder) {
      QList<QGraphicsItem *> sorted;
      const int numTopLevelItems = scened->topLevelItems.size();

      for (int i = numTopLevelItems - 1; i >= 0; --i) {
         sorted << scened->topLevelItems.at(i);
      }

      return sorted;
   }

   return scened->topLevelItems;
}

void QGraphicsSceneIndex::updateSceneRect(const QRectF &rect)
{
   (void) rect;
}

void QGraphicsSceneIndex::clear()
{
   const QList<QGraphicsItem *> allItems = items();
   for (int i = 0 ; i < allItems.size(); ++i) {
      removeItem(allItems.at(i));
   }
}

void QGraphicsSceneIndex::deleteItem(QGraphicsItem *item)
{
   removeItem(item);
}

void QGraphicsSceneIndex::itemChange(const QGraphicsItem *item, QGraphicsItem::GraphicsItemChange change,
   const void *const value)
{
   (void) item;
   (void) change;
   (void) value;
}

void QGraphicsSceneIndex::prepareBoundingRectChange(const QGraphicsItem *item)
{
   (void) item;
}



#endif // QT_NO_GRAPHICSVIEW
