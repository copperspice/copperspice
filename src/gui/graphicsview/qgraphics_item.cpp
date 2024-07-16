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

#include <algorithm>

#include <qgraphicsitem.h>

#ifndef QT_NO_GRAPHICSVIEW

#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsview.h>
#include <qgraphicswidget.h>
#include <qgraphicsproxywidget.h>

#include <qbitarray.h>
#include <qdebug.h>
#include <qpoint.h>
#include <qstack.h>
#include <qtimer.h>
#include <qvariant.h>
#include <qvarlengtharray.h>
#include <qnumeric.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpixmapcache.h>
#include <qstyleoption.h>
#include <qevent.h>
#include <qinputmethod.h>
#include <qgraphicseffect.h>

#include <qgraphics_scene_p.h>
#include <qgraphics_item_p.h>
#include <qgraphics_scenebsptreeindex_p.h>
#include <qgraphics_widget_p.h>
#include <qtextcontrol_p.h>
#include <qtextdocumentlayout_p.h>
#include <qtextengine_p.h>
#include <qwidget_p.h>
#include <qapplication_p.h>
#include <qgesturemanager_p.h>
#include <qdebug_p.h>

static inline void _q_adjustRect(QRect *rect)
{
   Q_ASSERT(rect);

   if ( !rect->width()) {
      rect->adjust(0, 0, 1, 0);
   }

   if (!rect->height()) {
      rect->adjust(0, 0, 0, 1);
   }
}

// ### Move this into QGraphicsItemPrivate
class QGraphicsItemCustomDataStore
{
 public:
   QHash<const QGraphicsItem *, QMap<int, QVariant>> data;
};

static QGraphicsItemCustomDataStore *qt_dataStore()
{
   static QGraphicsItemCustomDataStore retval;
   return &retval;
}

// internal
static QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen)
{
   // We unfortunately need this adjustment since QPainterPathStroker will set a width of 1.0
   // if we pass a value of 0.0 to QPainterPathStroker::setWidth()
   const qreal penWidthZero = qreal(0.00000001);

   if (path == QPainterPath() || pen == Qt::NoPen) {
      return path;
   }

   QPainterPathStroker ps;
   ps.setCapStyle(pen.capStyle());

   if (pen.widthF() <= 0.0) {
      ps.setWidth(penWidthZero);
   } else {
      ps.setWidth(pen.widthF());
   }

   ps.setJoinStyle(pen.joinStyle());
   ps.setMiterLimit(pen.miterLimit());
   QPainterPath p = ps.createStroke(path);
   p.addPath(path);

   return p;
}

void QGraphicsItemPrivate::updateAncestorFlag(QGraphicsItem::GraphicsItemFlag childFlag,
   AncestorFlag flag, bool enabled, bool root)
{
   Q_Q(QGraphicsItem);

   if (root) {
      // For root items only. This is the item that has either enabled or
      // disabled \a childFlag, or has been reparented.
      switch (int(childFlag)) {
         case -2:
            flag = AncestorFiltersChildEvents;
            enabled = q->filtersChildEvents();
            break;

         case -1:
            flag = AncestorHandlesChildEvents;
            enabled = q->handlesChildEvents();
            break;

         case QGraphicsItem::ItemClipsChildrenToShape:
            flag = AncestorClipsChildren;
            enabled = itemFlags & QGraphicsItem::ItemClipsChildrenToShape;
            break;

         case QGraphicsItem::ItemIgnoresTransformations:
            flag = AncestorIgnoresTransformations;
            enabled = itemFlags & QGraphicsItem::ItemIgnoresTransformations;
            break;

         case QGraphicsItem::ItemContainsChildrenInShape:
            flag = AncestorContainsChildren;
            enabled = itemFlags & QGraphicsItem::ItemContainsChildrenInShape;
            break;

         default:
            return;
      }

      if (parent) {
         // Inherit the enabled-state from our parents.
         if ((parent->d_ptr->ancestorFlags & flag)
            || (int(parent->d_ptr->itemFlags & childFlag) == childFlag)
            || (int(childFlag) == -1 && parent->d_ptr->handlesChildEvents)
            || (int(childFlag) == -2 && parent->d_ptr->filtersDescendantEvents)) {
            enabled = true;
            ancestorFlags |= flag;

         } else {
            ancestorFlags &= ~flag;
         }

      } else {
         // Top-level root items don't have any ancestors, so there are no
         // ancestor flags either.
         ancestorFlags = 0;
      }
   } else {
      // Don't set or propagate the ancestor flag if it's already correct.
      if (((ancestorFlags & flag) && enabled) || (!(ancestorFlags & flag) && !enabled)) {
         return;
      }

      // Set the flag.
      if (enabled) {
         ancestorFlags |= flag;
      } else {
         ancestorFlags &= ~flag;
      }

      // Don't process children if the item has the main flag set on itself.
      if ((int(childFlag) != -1 &&  int(itemFlags & childFlag) == childFlag)
         || (int(childFlag) == -1 && handlesChildEvents)
         || (int(childFlag) == -2 && filtersDescendantEvents)) {
         return;
      }
   }

   for (int i = 0; i < children.size(); ++i) {
      children.at(i)->d_ptr->updateAncestorFlag(childFlag, flag, enabled, false);
   }
}

void QGraphicsItemPrivate::updateAncestorFlags()
{
   int flags = 0;

   if (parent) {
      // Inherit the parent's ancestor flags.
      QGraphicsItemPrivate *pd = parent->d_ptr.data();
      flags = pd->ancestorFlags;

      // Add in flags from the parent.
      if (pd->filtersDescendantEvents) {
         flags |= AncestorFiltersChildEvents;
      }

      if (pd->handlesChildEvents) {
         flags |= AncestorHandlesChildEvents;
      }

      if (pd->itemFlags & QGraphicsItem::ItemClipsChildrenToShape) {
         flags |= AncestorClipsChildren;
      }

      if (pd->itemFlags & QGraphicsItem::ItemIgnoresTransformations) {
         flags |= AncestorIgnoresTransformations;
      }

      if (pd->itemFlags & QGraphicsItem::ItemContainsChildrenInShape) {
         flags |= AncestorContainsChildren;
      }
   }

   if (ancestorFlags == flags) {
      return;   // No change; stop propagation.
   }

   ancestorFlags = flags;

   // Propagate to children recursively.
   for (int i = 0; i < children.size(); ++i) {
      children.at(i)->d_ptr->updateAncestorFlags();
   }
}

void QGraphicsItemPrivate::setIsMemberOfGroup(bool enabled)
{
   Q_Q(QGraphicsItem);
   isMemberOfGroup = enabled;

   if (! qgraphicsitem_cast<QGraphicsItemGroup *>(q)) {
      for (QGraphicsItem *child : children) {
         child->d_func()->setIsMemberOfGroup(enabled);
      }
   }
}

void QGraphicsItemPrivate::remapItemPos(QEvent *event, QGraphicsItem *item)
{
   Q_Q(QGraphicsItem);
   switch (event->type()) {
      case QEvent::GraphicsSceneMouseMove:
      case QEvent::GraphicsSceneMousePress:
      case QEvent::GraphicsSceneMouseRelease:
      case QEvent::GraphicsSceneMouseDoubleClick: {
         QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
         mouseEvent->setPos(item->mapFromItem(q, mouseEvent->pos()));
         mouseEvent->setLastPos(item->mapFromItem(q, mouseEvent->pos()));
         for (int i = 0x1; i <= 0x10; i <<= 1) {
            if (mouseEvent->buttons() & i) {
               Qt::MouseButton button = Qt::MouseButton(i);
               mouseEvent->setButtonDownPos(button, item->mapFromItem(q, mouseEvent->buttonDownPos(button)));
            }
         }
         break;
      }
      case QEvent::GraphicsSceneWheel: {
         QGraphicsSceneWheelEvent *wheelEvent = static_cast<QGraphicsSceneWheelEvent *>(event);
         wheelEvent->setPos(item->mapFromItem(q, wheelEvent->pos()));
         break;
      }
      case QEvent::GraphicsSceneContextMenu: {
         QGraphicsSceneContextMenuEvent *contextEvent = static_cast<QGraphicsSceneContextMenuEvent *>(event);
         contextEvent->setPos(item->mapFromItem(q, contextEvent->pos()));
         break;
      }
      case QEvent::GraphicsSceneHoverMove: {
         QGraphicsSceneHoverEvent *hoverEvent = static_cast<QGraphicsSceneHoverEvent *>(event);
         hoverEvent->setPos(item->mapFromItem(q, hoverEvent->pos()));
         break;
      }
      default:
         break;
   }
}

QPointF QGraphicsItemPrivate::genericMapFromScene(const QPointF &pos,
   const QWidget *viewport) const
{
   Q_Q(const QGraphicsItem);
   if (!itemIsUntransformable()) {
      return q->mapFromScene(pos);
   }
   QGraphicsView *view = nullptr;
   if (viewport) {
      view = qobject_cast<QGraphicsView *>(viewport->parentWidget());
   }
   if (!view) {
      return q->mapFromScene(pos);
   }

   return q->deviceTransform(view->viewportTransform()).inverted().map(view->mapFromScene(pos));
}

void QGraphicsItemPrivate::combineTransformToParent(QTransform *x, const QTransform *viewTransform) const
{
   // COMBINE
   if (viewTransform && itemIsUntransformable()) {
      *x = q_ptr->deviceTransform(*viewTransform);
   } else {
      if (transformData) {
         *x *= transformData->computedFullTransform();
      }
      if (!pos.isNull()) {
         *x *= QTransform::fromTranslate(pos.x(), pos.y());
      }
   }
}

void QGraphicsItemPrivate::combineTransformFromParent(QTransform *x, const QTransform *viewTransform) const
{
   // COMBINE
   if (viewTransform && itemIsUntransformable()) {
      *x = q_ptr->deviceTransform(*viewTransform);
   } else {
      x->translate(pos.x(), pos.y());
      if (transformData) {
         *x = transformData->computedFullTransform(x);
      }
   }
}

void QGraphicsItemPrivate::updateSceneTransformFromParent()
{
   if (parent) {
      Q_ASSERT(!parent->d_ptr->dirtySceneTransform);
      if (parent->d_ptr->sceneTransformTranslateOnly) {
         sceneTransform = QTransform::fromTranslate(parent->d_ptr->sceneTransform.dx() + pos.x(),
               parent->d_ptr->sceneTransform.dy() + pos.y());
      } else {
         sceneTransform = parent->d_ptr->sceneTransform;
         sceneTransform.translate(pos.x(), pos.y());
      }
      if (transformData) {
         sceneTransform = transformData->computedFullTransform(&sceneTransform);
         sceneTransformTranslateOnly = (sceneTransform.type() <= QTransform::TxTranslate);
      } else {
         sceneTransformTranslateOnly = parent->d_ptr->sceneTransformTranslateOnly;
      }
   } else if (!transformData) {
      sceneTransform = QTransform::fromTranslate(pos.x(), pos.y());
      sceneTransformTranslateOnly = 1;
   } else if (transformData->onlyTransform) {
      sceneTransform = transformData->transform;
      if (!pos.isNull()) {
         sceneTransform *= QTransform::fromTranslate(pos.x(), pos.y());
      }
      sceneTransformTranslateOnly = (sceneTransform.type() <= QTransform::TxTranslate);
   } else if (pos.isNull()) {
      sceneTransform = transformData->computedFullTransform();
      sceneTransformTranslateOnly = (sceneTransform.type() <= QTransform::TxTranslate);
   } else {
      sceneTransform = QTransform::fromTranslate(pos.x(), pos.y());
      sceneTransform = transformData->computedFullTransform(&sceneTransform);
      sceneTransformTranslateOnly = (sceneTransform.type() <= QTransform::TxTranslate);
   }
   dirtySceneTransform = 0;
}

void QGraphicsItemPrivate::setParentItemHelper(QGraphicsItem *newParent, const QVariant *newParentVariant,
   const QVariant *thisPointerVariant)
{
   Q_Q(QGraphicsItem);

   if (newParent == parent) {
      return;
   }

   if (isWidget)
      static_cast<QGraphicsWidgetPrivate *>(this)->fixFocusChainBeforeReparenting((newParent &&
            newParent->isWidget()) ? static_cast<QGraphicsWidget *>(newParent) : nullptr,
         scene);
   if (scene) {
      // Deliver the change to the index
      if (scene->d_func()->indexMethod != QGraphicsScene::NoIndex) {
         scene->d_func()->index->itemChange(q, QGraphicsItem::ItemParentChange, newParent);
      }

      // Disable scene pos notifications for old ancestors
      if (scenePosDescendants || (itemFlags & QGraphicsItem::ItemSendsScenePositionChanges)) {
         scene->d_func()->setScenePosItemEnabled(q, false);
      }
   }

   if (subFocusItem && parent) {
      // Make sure none of the old parents point to this guy.
      subFocusItem->d_ptr->clearSubFocus(parent);
   }

   // We anticipate geometry changes. If the item is deleted, it will be
   // removed from the index at a later stage, and the whole scene will be
   // updated.
   if (!inDestructor) {
      q_ptr->prepareGeometryChange();
   }

   if (parent) {
      // Remove from current parent
      parent->d_ptr->removeChild(q);
      if (thisPointerVariant) {
         parent->itemChange(QGraphicsItem::ItemChildRemovedChange, *thisPointerVariant);
      }
   }

   // Update toplevelitem list. If this item is being deleted, its parent
   // will be 0 but we don't want to register/unregister it in the TLI list.
   if (scene && !inDestructor) {
      if (parent && !newParent) {
         scene->d_func()->registerTopLevelItem(q);
      } else if (!parent && newParent) {
         scene->d_func()->unregisterTopLevelItem(q);
      }
   }

   // Ensure any last parent focus scope does not point to this item or any of
   // its descendents.
   QGraphicsItem *p = parent;
   QGraphicsItem *parentFocusScopeItem = nullptr;

   while (p) {
      if (p->d_ptr->itemFlags & QGraphicsItem::ItemIsFocusScope) {
         // If this item's focus scope's focus scope item points
         // to this item or a descendent, then clear it.
         QGraphicsItem *fsi = p->d_ptr->focusScopeItem;

         if (q_ptr == fsi || q_ptr->isAncestorOf(fsi)) {
            parentFocusScopeItem = fsi;
            p->d_ptr->focusScopeItem = nullptr;
            fsi->d_ptr->focusScopeItemChange(false);
         }

         break;
      }
      p = p->d_ptr->parent;
   }

   // Update graphics effect optimization flag
   if (newParent && (graphicsEffect || mayHaveChildWithGraphicsEffect)) {
      newParent->d_ptr->updateChildWithGraphicsEffectFlagRecursively();
   }

   // Update focus scope item ptr in new scope.
   QGraphicsItem *newFocusScopeItem = subFocusItem ? subFocusItem : parentFocusScopeItem;
   if (newFocusScopeItem && newParent) {
      QGraphicsItem *p = newParent;

      while (p) {
         if (p->d_ptr->itemFlags & QGraphicsItem::ItemIsFocusScope) {
            if (subFocusItem && subFocusItem != q_ptr) {
               // Find the subFocusItem's topmost focus scope within the new parent's focusscope
               QGraphicsItem *ancestorScope = nullptr;
               QGraphicsItem *p2 = subFocusItem->d_ptr->parent;

               while (p2 && p2 != p) {
                  if (p2->d_ptr->itemFlags & QGraphicsItem::ItemIsFocusScope) {
                     ancestorScope = p2;
                  }
                  if (p2->d_ptr->itemFlags & QGraphicsItem::ItemIsPanel) {
                     break;
                  }
                  if (p2 == q_ptr) {
                     break;
                  }
                  p2 = p2->d_ptr->parent;
               }

               if (ancestorScope) {
                  newFocusScopeItem = ancestorScope;
               }
            }

            p->d_ptr->focusScopeItem = newFocusScopeItem;
            newFocusScopeItem->d_ptr->focusScopeItemChange(true);
            // Ensure the new item is no longer the subFocusItem. The
            // only way to set focus on a child of a focus scope is
            // by setting focus on the scope itself.
            if (subFocusItem && !p->focusItem()) {
               subFocusItem->d_ptr->clearSubFocus();
            }
            break;
         }
         p = p->d_ptr->parent;
      }
   }

   // Resolve depth.
   invalidateDepthRecursively();

   if ((parent = newParent)) {
      if (parent->d_func()->scene && parent->d_func()->scene != scene) {
         // Move this item to its new parent's scene
         parent->d_func()->scene->addItem(q);

      } else if (! parent->d_func()->scene && scene) {
         // Remove this item from its former scene
         scene->removeItem(q);
      }

      parent->d_ptr->addChild(q);
      if (thisPointerVariant) {
         parent->itemChange(QGraphicsItem::ItemChildAddedChange, *thisPointerVariant);
      }

      if (scene) {
         // Re-enable scene pos notifications for new ancestors
         if (scenePosDescendants || (itemFlags & QGraphicsItem::ItemSendsScenePositionChanges)) {
            scene->d_func()->setScenePosItemEnabled(q, true);
         }
      }

      // Propagate dirty flags to the new parent
      markParentDirty(/*updateBoundingRect=*/true);

      // Inherit ancestor flags from the new parent.
      updateAncestorFlags();

      // Update item visible / enabled.
      if (parent->d_ptr->visible != visible) {
         if (!parent->d_ptr->visible || !explicitlyHidden) {
            setVisibleHelper(parent->d_ptr->visible, /* explicit = */ false, /* update = */ false);
         }
      }
      if (parent->isEnabled() != enabled) {
         if (!parent->d_ptr->enabled || !explicitlyDisabled) {
            setEnabledHelper(parent->d_ptr->enabled, /* explicit = */ false, /* update = */ false);
         }
      }

      // Auto-activate if visible and the parent is active.
      if (visible && parent->isActive()) {
         q->setActive(true);
      }
   } else {
      // Inherit ancestor flags from the new parent.
      updateAncestorFlags();

      if (!inDestructor) {
         // Update item visible / enabled.
         if (!visible && !explicitlyHidden) {
            setVisibleHelper(true, /* explicit = */ false);
         }
         if (!enabled && !explicitlyDisabled) {
            setEnabledHelper(true, /* explicit = */ false);
         }
      }
   }

   dirtySceneTransform = 1;
   if (!inDestructor && (transformData || (newParent && newParent->d_ptr->transformData))) {
      transformChanged();
   }

   // Restore the sub focus chain.
   if (subFocusItem) {
      subFocusItem->d_ptr->setSubFocus(newParent);
      if (parent && parent->isActive()) {
         subFocusItem->setFocus();
      }
   }

   // Deliver post-change notification
   if (newParentVariant) {
      q->itemChange(QGraphicsItem::ItemParentHasChanged, *newParentVariant);
   }

   if (isObject) {
      emit static_cast<QGraphicsObject *>(q)->parentChanged();
   }
}

void QGraphicsItemPrivate::childrenBoundingRectHelper(QTransform *x, QRectF *rect, QGraphicsItem *topMostEffectItem)
{
   Q_Q(QGraphicsItem);

   QRectF childrenRect;
   QRectF *result = rect;
   rect = &childrenRect;
   const bool setTopMostEffectItem = !topMostEffectItem;

   for (int i = 0; i < children.size(); ++i) {
      QGraphicsItem *child = children.at(i);
      QGraphicsItemPrivate *childd = child->d_ptr.data();

      if (setTopMostEffectItem) {
         topMostEffectItem = child;
      }

      bool hasPos = !childd->pos.isNull();

      if (hasPos || childd->transformData) {
         // COMBINE
         QTransform matrix = childd->transformToParent();
         if (x) {
            matrix *= *x;
         }

         *rect |= matrix.mapRect(child->d_ptr->effectiveBoundingRect(topMostEffectItem));
         if (!childd->children.isEmpty()) {
            childd->childrenBoundingRectHelper(&matrix, rect, topMostEffectItem);
         }

      } else {
         if (x) {
            *rect |= x->mapRect(child->d_ptr->effectiveBoundingRect(topMostEffectItem));
         } else {
            *rect |= child->d_ptr->effectiveBoundingRect(topMostEffectItem);
         }
         if (!childd->children.isEmpty()) {
            childd->childrenBoundingRectHelper(x, rect, topMostEffectItem);
         }
      }
   }

   if (itemFlags & QGraphicsItem::ItemClipsChildrenToShape) {
      if (x) {
         *rect &= x->mapRect(q->boundingRect());
      } else {
         *rect &= q->boundingRect();
      }
   }

   *result |= *rect;
}

void QGraphicsItemPrivate::initStyleOption(QStyleOptionGraphicsItem *option, const QTransform &worldTransform,
   const QRegion &exposedRegion, bool allItems) const
{
   Q_ASSERT(option);
   Q_Q(const QGraphicsItem);

   // Initialize standard QStyleOption values.
   const QRectF brect = q->boundingRect();
   option->state = QStyle::State_None;
   option->rect = brect.toRect();
   option->levelOfDetail = 1;
   option->exposedRect = brect;
   // Style animations require a QObject-based animation target.
   // If a plain QGraphicsItem is used to draw animated controls,
   // QStyle is let to send animation updates to the whole scene.
   option->styleObject = q_ptr->toGraphicsObject();

   if (!option->styleObject) {
      option->styleObject = scene;
   }
   if (selected) {
      option->state |= QStyle::State_Selected;
   }
   if (enabled) {
      option->state |= QStyle::State_Enabled;
   }
   if (q->hasFocus()) {
      option->state |= QStyle::State_HasFocus;
   }
   if (scene) {
      if (scene->d_func()->hoverItems.contains(q_ptr)) {
         option->state |= QStyle::State_MouseOver;
      }
      if (q == scene->mouseGrabberItem()) {
         option->state |= QStyle::State_Sunken;
      }
   }

   if (! (itemFlags & QGraphicsItem::ItemUsesExtendedStyleOption)) {
      return;
   }

   // Initialize QStyleOptionGraphicsItem specific values (matrix, exposedRect).
   option->matrix = worldTransform.toAffine(); //### discards perspective

   if (!allItems) {
      // Determine the item's exposed area
      option->exposedRect = QRectF();
      const QTransform reverseMap = worldTransform.inverted();
      const QVector<QRect> exposedRects(exposedRegion.rects());
      for (int i = 0; i < exposedRects.size(); ++i) {
         option->exposedRect |= reverseMap.mapRect(QRectF(exposedRects.at(i)));
         if (option->exposedRect.contains(brect)) {
            break;
         }
      }
      option->exposedRect &= brect;
   }
}

// internal
void QGraphicsItemCache::purge()
{
   QPixmapCache::remove(key);
   key = QPixmapCache::Key();

   QMutableHashIterator<QPaintDevice *, DeviceData> it(deviceData);

   while (it.hasNext()) {
      DeviceData &data = it.next().value();
      QPixmapCache::remove(data.key);
      data.cacheIndent = QPoint();
   }

   deviceData.clear();
   allExposed = true;
   exposed.clear();
}


QGraphicsItem::QGraphicsItem(QGraphicsItem *parent)
   : d_ptr(new QGraphicsItemPrivate)
{
   d_ptr->q_ptr = this;
   setParentItem(parent);
}

// internal
QGraphicsItem::QGraphicsItem(QGraphicsItemPrivate &dd, QGraphicsItem *parent)
   : d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   setParentItem(parent);
}

QGraphicsItem::~QGraphicsItem()
{
   if (d_ptr->isObject) {
      QGraphicsObject *obj = static_cast<QGraphicsObject *>(this);
      CSInternalRefCount::set_m_wasDeleted(obj, true);

      // used by declarative library
      CSAbstractDeclarativeData *tmp_Data = CSInternalDeclarativeData::get_m_declarativeData(obj);

      if (tmp_Data) {
         CSAbstractDeclarativeData::destroyed(tmp_Data, obj);
         CSInternalDeclarativeData::set_m_declarativeData(obj, nullptr);
      }
   }

   d_ptr->inDestructor = 1;
   d_ptr->removeExtraItemCache();

#ifndef QT_NO_GESTURES
   if (d_ptr->isObject && !d_ptr->gestureContext.isEmpty()) {
      QGraphicsObject *o = static_cast<QGraphicsObject *>(this);

      if (QGestureManager *manager = QGestureManager::instance()) {
         for (Qt::GestureType type : d_ptr->gestureContext.keys()) {
            manager->cleanupCachedGestures(o, type);
         }
      }
   }
#endif

   clearFocus();
   setFocusProxy(nullptr);

   // Update focus scope item ptr.
   QGraphicsItem *p = d_ptr->parent;

   while (p) {
      if (p->flags() & ItemIsFocusScope) {
         if (p->d_ptr->focusScopeItem == this) {
            p->d_ptr->focusScopeItem = nullptr;
         }
         break;
      }
      p = p->d_ptr->parent;
   }

   if (!d_ptr->children.isEmpty()) {
      while (!d_ptr->children.isEmpty()) {
         delete d_ptr->children.first();
      }
      Q_ASSERT(d_ptr->children.isEmpty());
   }

   if (d_ptr->scene) {
      d_ptr->scene->d_func()->removeItemHelper(this);
   } else {
      d_ptr->resetFocusProxy();
      setParentItem(nullptr);
   }

#ifndef QT_NO_GRAPHICSEFFECT
   delete d_ptr->graphicsEffect;
#endif

   if (d_ptr->transformData) {
      for (int i = 0; i < d_ptr->transformData->graphicsTransforms.size(); ++i) {
         QGraphicsTransform *t = d_ptr->transformData->graphicsTransforms.at(i);
         static_cast<QGraphicsTransformPrivate *>(t->d_ptr.data())->item = nullptr;
         delete t;
      }
   }

   delete d_ptr->transformData;

   if (QGraphicsItemCustomDataStore *dataStore = qt_dataStore()) {
      dataStore->data.remove(this);
   }
}

QGraphicsScene *QGraphicsItem::scene() const
{
   return d_ptr->scene;
}

QGraphicsItemGroup *QGraphicsItem::group() const
{
   if (! d_ptr->isMemberOfGroup) {
      return nullptr;
   }

   QGraphicsItem *parent = const_cast<QGraphicsItem *>(this);

   while ((parent = parent->d_ptr->parent)) {
      if (QGraphicsItemGroup *group = qgraphicsitem_cast<QGraphicsItemGroup *>(parent)) {
         return group;
      }
   }

   // Unreachable; if d_ptr->isMemberOfGroup is != 0, then one parent of this
   // item is a group item.

   return nullptr;
}

void QGraphicsItem::setGroup(QGraphicsItemGroup *group)
{
   if (!group) {
      if (QGraphicsItemGroup *group = this->group()) {
         group->removeFromGroup(this);
      }
   } else {
      group->addToGroup(this);
   }
}

QGraphicsItem *QGraphicsItem::parentItem() const
{
   return d_ptr->parent;
}

QGraphicsItem *QGraphicsItem::topLevelItem() const
{
   QGraphicsItem *parent = const_cast<QGraphicsItem *>(this);
   while (QGraphicsItem *grandPa = parent->parentItem()) {
      parent = grandPa;
   }
   return parent;
}

QGraphicsObject *QGraphicsItem::parentObject() const
{
   QGraphicsItem *p = d_ptr->parent;
   return (p && p->d_ptr->isObject) ? static_cast<QGraphicsObject *>(p) : nullptr;
}

QGraphicsWidget *QGraphicsItem::parentWidget() const
{
   QGraphicsItem *p = parentItem();
   while (p && !p->isWidget()) {
      p = p->parentItem();
   }
   return (p && p->isWidget()) ? static_cast<QGraphicsWidget *>(p) : nullptr;
}

QGraphicsWidget *QGraphicsItem::topLevelWidget() const
{
   if (const QGraphicsWidget *p = parentWidget()) {
      return p->topLevelWidget();
   }
   return isWidget() ? static_cast<QGraphicsWidget *>(const_cast<QGraphicsItem *>(this)) : nullptr;
}

QGraphicsWidget *QGraphicsItem::window() const
{
   QGraphicsItem *p = panel();
   if (p && p->isWindow()) {
      return static_cast<QGraphicsWidget *>(p);
   }
   return nullptr;
}

QGraphicsItem *QGraphicsItem::panel() const
{
   if (d_ptr->itemFlags & ItemIsPanel) {
      return const_cast<QGraphicsItem *>(this);
   }
   return d_ptr->parent ? d_ptr->parent->panel() : nullptr;
}

QGraphicsObject *QGraphicsItem::toGraphicsObject()
{
   return d_ptr->isObject ? static_cast<QGraphicsObject *>(this) : nullptr;
}

const QGraphicsObject *QGraphicsItem::toGraphicsObject() const
{
   return d_ptr->isObject ? static_cast<const QGraphicsObject *>(this) : nullptr;
}

void QGraphicsItem::setParentItem(QGraphicsItem *newParent)
{
   if (newParent == this) {
      qWarning("QGraphicsItem::setParentItem() Unable to assign an item as a parent of itself");
      return;
   }
   if (newParent == d_ptr->parent) {
      return;
   }

   const QVariant newParentVariant(itemChange(QGraphicsItem::ItemParentChange,
         QVariant::fromValue<QGraphicsItem *>(newParent)));

   newParent = newParentVariant.value<QGraphicsItem *>();

   if (newParent == d_ptr->parent) {
      return;
   }

   const QVariant thisPointerVariant(QVariant::fromValue<QGraphicsItem *>(this));
   d_ptr->setParentItemHelper(newParent, &newParentVariant, &thisPointerVariant);
}

QList<QGraphicsItem *> QGraphicsItem::childItems() const
{
   const_cast<QGraphicsItem *>(this)->d_ptr->ensureSortedChildren();
   return d_ptr->children;
}

bool QGraphicsItem::isWidget() const
{
   return d_ptr->isWidget;
}

bool QGraphicsItem::isWindow() const
{
   return d_ptr->isWidget && (static_cast<const QGraphicsWidget *>(this)->windowType() & Qt::Window);
}

bool QGraphicsItem::isPanel() const
{
   return d_ptr->itemFlags & ItemIsPanel;
}

QGraphicsItem::GraphicsItemFlags QGraphicsItem::flags() const
{
   return GraphicsItemFlags(d_ptr->itemFlags);
}

void QGraphicsItem::setFlag(GraphicsItemFlag flag, bool enabled)
{
   if (enabled) {
      setFlags(GraphicsItemFlags(d_ptr->itemFlags) | flag);
   } else {
      setFlags(GraphicsItemFlags(d_ptr->itemFlags) & ~flag);
   }
}

void QGraphicsItem::setFlags(GraphicsItemFlags flags)
{
   // Notify change and check for adjustment.
   if (quint32(d_ptr->itemFlags) == quint32(flags)) {
      return;
   }

   flags = GraphicsItemFlags(itemChange(ItemFlagsChange, quint32(flags)).toUInt());
   if (quint32(d_ptr->itemFlags) == quint32(flags)) {
      return;
   }

   if (d_ptr->scene && d_ptr->scene->d_func()->indexMethod != QGraphicsScene::NoIndex) {
      d_ptr->scene->d_func()->index->itemChange(this, ItemFlagsChange, &flags);
   }

   // Flags that alter the geometry of the item (or its children).
   const quint32 geomChangeFlagsMask = (ItemClipsChildrenToShape | ItemClipsToShape | ItemIgnoresTransformations |
         ItemIsSelectable);

   bool fullUpdate = (quint32(flags) & geomChangeFlagsMask) != (d_ptr->itemFlags & geomChangeFlagsMask);
   if (fullUpdate) {
      d_ptr->updatePaintedViewBoundingRects(true);
   }

   // Keep the old flags to compare the diff.
   GraphicsItemFlags oldFlags = GraphicsItemFlags(d_ptr->itemFlags);

   // Update flags.
   d_ptr->itemFlags = flags;

   if (!(d_ptr->itemFlags & ItemIsFocusable) && hasFocus()) {
      // Clear focus on the item if it has focus when the focusable flag is unset
      clearFocus();
   }

   if (! (d_ptr->itemFlags & ItemIsSelectable) && isSelected()) {
      // Unselect the item if it is selected when the selectable flag is
      // unset.
      setSelected(false);
   }

   if ((flags & ItemClipsChildrenToShape) != (oldFlags & ItemClipsChildrenToShape)) {
      // Item children clipping changes. Propagate the ancestor flag to all children.
      d_ptr->updateAncestorFlag(ItemClipsChildrenToShape);

      // The childrenBoundingRect is clipped to the boundingRect in case of ItemClipsChildrenToShape,
      // which means we have to invalidate the cached childrenBoundingRect whenever this flag changes.
      d_ptr->dirtyChildrenBoundingRect = 1;
      d_ptr->markParentDirty(true);
   }

   if ((flags & ItemContainsChildrenInShape) != (oldFlags & ItemContainsChildrenInShape)) {
      d_ptr->updateAncestorFlag(ItemContainsChildrenInShape);
   }
   if ((flags & ItemIgnoresTransformations) != (oldFlags & ItemIgnoresTransformations)) {
      // Item children clipping changes. Propagate the ancestor flag to
      // all children.
      d_ptr->updateAncestorFlag(ItemIgnoresTransformations);
   }

   if ((flags & ItemNegativeZStacksBehindParent) != (oldFlags & ItemNegativeZStacksBehindParent)) {
      // NB! We change the flags directly here, so we must also update d_ptr->flags.
      // Note that this has do be done before the ItemStacksBehindParent check
      // below; otherwise we will loose the change.

      // Update stack-behind.
      if (d_ptr->z < qreal(0.0)) {
         flags |= ItemStacksBehindParent;
      } else {
         flags &= ~ItemStacksBehindParent;
      }
      d_ptr->itemFlags = flags;
   }

   if ((flags & ItemStacksBehindParent) != (oldFlags & ItemStacksBehindParent)) {
      // Note: this check has to come after the ItemNegativeZStacksBehindParent
      // check above. Be careful.

      // Ensure child item sorting is up to date when toggling this flag.
      if (d_ptr->parent) {
         d_ptr->parent->d_ptr->needSortChildren = 1;
      } else if (d_ptr->scene) {
         d_ptr->scene->d_func()->needSortTopLevelItems = 1;
      }
   }

   if ((flags & ItemAcceptsInputMethod) != (oldFlags & ItemAcceptsInputMethod)) {
      // Update input method sensitivity in any views.
      if (d_ptr->scene) {
         d_ptr->scene->d_func()->updateInputMethodSensitivityInViews();
      }
   }

   if ((flags & ItemIsPanel) != (oldFlags & ItemIsPanel)) {
      bool becomesPanel = (flags & ItemIsPanel);
      if ((d_ptr->panelModality != NonModal) && d_ptr->scene) {
         // update the panel's modal state
         if (becomesPanel) {
            d_ptr->scene->d_func()->enterModal(this);
         } else {
            d_ptr->scene->d_func()->leaveModal(this);
         }
      }
      if (d_ptr->isWidget && (becomesPanel || parentWidget())) {
         QGraphicsWidget *w = static_cast<QGraphicsWidget *>(this);
         QGraphicsWidget *focusFirst = w;
         QGraphicsWidget *focusLast = w;
         for (;;) {
            QGraphicsWidget *test = focusLast->d_func()->focusNext;
            if (!w->isAncestorOf(test) || test == w) {
               break;
            }
            focusLast = test;
         }

         if (becomesPanel) {
            QGraphicsWidget *beforeMe = w->d_func()->focusPrev;
            QGraphicsWidget *afterMe = focusLast->d_func()->focusNext;
            beforeMe->d_func()->focusNext = afterMe;
            afterMe->d_func()->focusPrev = beforeMe;
            focusFirst->d_func()->focusPrev = focusLast;
            focusLast->d_func()->focusNext = focusFirst;
            if (!isAncestorOf(focusFirst->d_func()->focusNext)) {
               focusFirst->d_func()->focusNext = w;
            }
         } else if (QGraphicsWidget *pw = parentWidget()) {
            QGraphicsWidget *beforeMe = pw;
            QGraphicsWidget *afterMe = pw->d_func()->focusNext;
            beforeMe->d_func()->focusNext = w;
            afterMe->d_func()->focusPrev = focusLast;
            w->d_func()->focusPrev = beforeMe;
            focusLast->d_func()->focusNext = afterMe;
         }
      }
   }
   if (d_ptr->scene) {
      if ((flags & ItemSendsScenePositionChanges) != (oldFlags & ItemSendsScenePositionChanges)) {
         if (flags & ItemSendsScenePositionChanges) {
            d_ptr->scene->d_func()->registerScenePosItem(this);
         } else {
            d_ptr->scene->d_func()->unregisterScenePosItem(this);
         }
      }
      d_ptr->scene->d_func()->markDirty(this, QRectF(), /*invalidateChildren=*/true);
   }

   // Notify change.
   itemChange(ItemFlagsHaveChanged, quint32(flags));
}

QGraphicsItem::CacheMode QGraphicsItem::cacheMode() const
{
   return QGraphicsItem::CacheMode(d_ptr->cacheMode);
}

void QGraphicsItem::setCacheMode(CacheMode mode, const QSize &logicalCacheSize)
{
   CacheMode lastMode = CacheMode(d_ptr->cacheMode);
   d_ptr->cacheMode = mode;

   bool noVisualChange = (mode == NoCache && lastMode == NoCache)
      || (mode == NoCache && lastMode == DeviceCoordinateCache)
      || (mode == DeviceCoordinateCache && lastMode == NoCache)
      || (mode == DeviceCoordinateCache && lastMode == DeviceCoordinateCache);

   if (mode == NoCache) {
      d_ptr->removeExtraItemCache();

   } else {
      QGraphicsItemCache *cache = d_ptr->extraItemCache();

      // Reset old cache
      cache->purge();

      if (mode == ItemCoordinateCache) {
         if (lastMode == mode && cache->fixedSize == logicalCacheSize) {
            noVisualChange = true;
         }
         cache->fixedSize = logicalCacheSize;
      }
   }
   if (!noVisualChange) {
      update();
   }
}

QGraphicsItem::PanelModality QGraphicsItem::panelModality() const
{
   return d_ptr->panelModality;
}

void QGraphicsItem::setPanelModality(PanelModality panelModality)
{
   if (d_ptr->panelModality == panelModality) {
      return;
   }

   PanelModality previousModality = d_ptr->panelModality;
   bool enterLeaveModal = (isPanel() && d_ptr->scene && isVisible());
   if (enterLeaveModal && panelModality == NonModal) {
      d_ptr->scene->d_func()->leaveModal(this);
   }
   d_ptr->panelModality = panelModality;
   if (enterLeaveModal && d_ptr->panelModality != NonModal) {
      d_ptr->scene->d_func()->enterModal(this, previousModality);
   }
}

bool QGraphicsItem::isBlockedByModalPanel(QGraphicsItem **blockingPanel) const
{
   if (!d_ptr->scene) {
      return false;
   }

   QGraphicsItem *dummy = nullptr;
   if (!blockingPanel) {
      blockingPanel = &dummy;
   }

   QGraphicsScenePrivate *scene_d = d_ptr->scene->d_func();
   if (scene_d->modalPanels.isEmpty()) {
      return false;
   }

   // ###
   if (!scene_d->popupWidgets.isEmpty() && scene_d->popupWidgets.first() == this) {
      return false;
   }

   for (int i = 0; i < scene_d->modalPanels.count(); ++i) {
      QGraphicsItem *modalPanel = scene_d->modalPanels.at(i);
      if (modalPanel->panelModality() == QGraphicsItem::SceneModal) {
         // Scene modal panels block all non-descendents.
         if (modalPanel != this && !modalPanel->isAncestorOf(this)) {
            *blockingPanel = modalPanel;
            return true;
         }
      } else {
         // Window modal panels block ancestors and siblings/cousins.
         if (modalPanel != this
            && !modalPanel->isAncestorOf(this)
            && commonAncestorItem(modalPanel)) {
            *blockingPanel = modalPanel;
            return true;
         }
      }
   }
   return false;
}

#ifndef QT_NO_TOOLTIP

QString QGraphicsItem::toolTip() const
{
   return d_ptr->extra(QGraphicsItemPrivate::ExtraToolTip).toString();
}

void QGraphicsItem::setToolTip(const QString &toolTip)
{
   const QVariant toolTipVariant(itemChange(ItemToolTipChange, toolTip));
   d_ptr->setExtra(QGraphicsItemPrivate::ExtraToolTip, toolTipVariant.toString());
   itemChange(ItemToolTipHasChanged, toolTipVariant);
}
#endif

#ifndef QT_NO_CURSOR
QCursor QGraphicsItem::cursor() const
{
   QVariant variant = d_ptr->extra(QGraphicsItemPrivate::ExtraCursor);

   return variant.value<QCursor>();
}

void QGraphicsItem::setCursor(const QCursor &cursor)
{
   const QVariant cursorVariant(itemChange(ItemCursorChange, QVariant::fromValue<QCursor>(cursor)));
   d_ptr->setExtra(QGraphicsItemPrivate::ExtraCursor, cursorVariant.value<QCursor>());
   d_ptr->hasCursor = 1;

   if (d_ptr->scene) {
      d_ptr->scene->d_func()->allItemsUseDefaultCursor = false;

      for (QGraphicsView *view : d_ptr->scene->views()) {
         view->viewport()->setMouseTracking(true);

         // Note: Some of this logic is duplicated in QGraphicsView's mouse events.
         if (view->underMouse()) {

            for (QGraphicsItem *itemUnderCursor : view->items(view->mapFromGlobal(QCursor::pos()))) {
               if (itemUnderCursor->hasCursor()) {
                  QMetaObject::invokeMethod(view, "_q_setViewportCursor", Q_ARG(const QCursor &, itemUnderCursor->cursor()));
                  break;
               }
            }
            break;
         }
      }
   }
   itemChange(ItemCursorHasChanged, cursorVariant);
}

bool QGraphicsItem::hasCursor() const
{
   return d_ptr->hasCursor;
}

void QGraphicsItem::unsetCursor()
{
   if (! d_ptr->hasCursor) {
      return;
   }

   d_ptr->unsetExtra(QGraphicsItemPrivate::ExtraCursor);
   d_ptr->hasCursor = 0;
   if (d_ptr->scene) {
      for (QGraphicsView *view : d_ptr->scene->views()) {
         if (view->underMouse() && view->itemAt(view->mapFromGlobal(QCursor::pos())) == this) {
            QMetaObject::invokeMethod(view, "_q_unsetViewportCursor");
            break;
         }
      }
   }
}
#endif // QT_NO_CURSOR

bool QGraphicsItem::isVisible() const
{
   return d_ptr->visible;
}

bool QGraphicsItem::isVisibleTo(const QGraphicsItem *parent) const
{
   const QGraphicsItem *p = this;
   if (d_ptr->explicitlyHidden) {
      return false;
   }
   do {
      if (p == parent) {
         return true;
      }
      if (p->d_ptr->explicitlyHidden) {
         return false;
      }
   } while ((p = p->d_ptr->parent));
   return parent == nullptr;
}

void QGraphicsItemPrivate::setVisibleHelper(bool newVisible, bool explicitly,
   bool update, bool hiddenByPanel)
{
   Q_Q(QGraphicsItem);

   // Update explicit bit.
   if (explicitly) {
      explicitlyHidden = newVisible ? 0 : 1;
   }

   // Check if there's nothing to do.
   if (visible == quint32(newVisible)) {
      return;
   }

   // Don't show child if parent is not visible
   if (parent && newVisible && !parent->d_ptr->visible) {
      return;
   }

   // Modify the property.
   const QVariant newVisibleVariant(q_ptr->itemChange(QGraphicsItem::ItemVisibleChange,
         quint32(newVisible)));

   newVisible = newVisibleVariant.toBool();

   if (visible == quint32(newVisible)) {
      return;
   }
   visible = newVisible;

   // Schedule redrawing
   if (update) {

      QGraphicsItemCache *c = (QGraphicsItemCache *)(extra(ExtraCacheData)).value<void *>();

      if (c) {
         c->purge();
      }

      if (scene) {
#ifndef QT_NO_GRAPHICSEFFECT
         invalidateParentGraphicsEffectsRecursively();
#endif

         scene->d_func()->markDirty(q_ptr, QRectF(), /*invalidateChildren=*/false, /*force=*/true);
      }
   }

   // Certain properties are dropped as an item becomes invisible.
   bool hasFocus = q_ptr->hasFocus();
   if (!newVisible) {
      if (scene) {
         if (scene->d_func()->mouseGrabberItems.contains(q)) {
            q->ungrabMouse();
         }
         if (scene->d_func()->keyboardGrabberItems.contains(q)) {
            q->ungrabKeyboard();
         }
         if (q->isPanel() && panelModality != QGraphicsItem::NonModal) {
            scene->d_func()->leaveModal(q_ptr);
         }
      }

      if (hasFocus && scene) {
         // Hiding the closest non-panel ancestor of the focus item
         QGraphicsItem *focusItem = scene->focusItem();
         bool clear = true;

         if (isWidget && !focusItem->isPanel()) {
            do {
               if (focusItem == q_ptr) {
                  clear = !static_cast<QGraphicsWidget *>(q_ptr)->focusNextPrevChild(true);
                  break;
               }
            } while ((focusItem = focusItem->parentWidget()) && !focusItem->isPanel());
         }

         if (clear) {
            clearFocusHelper(/* giveFocusToParent = */ false, hiddenByPanel);
         }
      }

      if (q_ptr->isSelected()) {
         q_ptr->setSelected(false);
      }

   } else {
      geometryChanged = 1;
      paintedViewBoundingRectsNeedRepaint = 1;

      if (scene) {
         if (isWidget) {
            QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(q_ptr);
            if (widget->windowType() == Qt::Popup) {
               scene->d_func()->addPopup(widget);
            }
         }
         if (q->isPanel() && panelModality != QGraphicsItem::NonModal) {
            scene->d_func()->enterModal(q_ptr);
         }
      }
   }

   // Update children with explicitly = false.
   const bool updateChildren = update && ! ((itemFlags & QGraphicsItem::ItemClipsChildrenToShape
            || itemFlags & QGraphicsItem::ItemContainsChildrenInShape) && !(itemFlags & QGraphicsItem::ItemHasNoContents));

   for (QGraphicsItem *child : children) {
      if (! newVisible || !child->d_ptr->explicitlyHidden) {
         child->d_ptr->setVisibleHelper(newVisible, false, updateChildren, hiddenByPanel);
      }
   }

   // Update activation
   if (scene && q->isPanel()) {
      if (newVisible) {
         if (parent && parent->isActive()) {
            q->setActive(true);
         }
      } else {
         if (q->isActive()) {
            scene->setActivePanel(parent);
         }
      }
   }

   // Enable subfocus
   if (scene) {
      if (newVisible) {
         // Item is shown
         QGraphicsItem *p = parent;
         bool done = false;

         while (p) {
            if (p->flags() & QGraphicsItem::ItemIsFocusScope) {
               QGraphicsItem *fsi = p->d_ptr->focusScopeItem;

               if (q_ptr == fsi || q_ptr->isAncestorOf(fsi)) {
                  done = true;

                  while (fsi->d_ptr->focusScopeItem && fsi->d_ptr->focusScopeItem->isVisible()) {
                     fsi = fsi->d_ptr->focusScopeItem;
                  }

                  fsi->d_ptr->setFocusHelper(Qt::OtherFocusReason, true, false);
               }
               break;
            }
            p = p->d_ptr->parent;
         }

         if (!done) {
            QGraphicsItem *fi = subFocusItem;
            if (fi && fi != scene->focusItem()) {
               scene->setFocusItem(fi);

            } else if (itemFlags & QGraphicsItem::ItemIsFocusScope && ! scene->focusItem() &&
               q->isAncestorOf(scene->d_func()->lastFocusItem)) {
               q_ptr->setFocus();
            }
         }

      } else {
         // Item is hidden
         if (hasFocus) {
            QGraphicsItem *p = parent;

            while (p) {
               if (p->flags() & QGraphicsItem::ItemIsFocusScope) {
                  if (p->d_ptr->visible) {
                     p->d_ptr->setFocusHelper(Qt::OtherFocusReason, true, true);
                  }
                  break;
               }
               p = p->d_ptr->parent;
            }
         }
      }
   }

   // Deliver post-change notification.
   q_ptr->itemChange(QGraphicsItem::ItemVisibleHasChanged, newVisibleVariant);

   if (isObject) {
      emit static_cast<QGraphicsObject *>(q_ptr)->visibleChanged();
   }
}

void QGraphicsItem::setVisible(bool visible)
{
   d_ptr->setVisibleHelper(visible, true, true, isPanel());

}

bool QGraphicsItem::isEnabled() const
{
   return d_ptr->enabled;
}

void QGraphicsItemPrivate::setEnabledHelper(bool newEnabled, bool explicitly, bool update)
{
   // Update explicit bit.
   if (explicitly) {
      explicitlyDisabled = newEnabled ? 0 : 1;
   }

   // Check if there's nothing to do.
   if (enabled == quint32(newEnabled)) {
      return;
   }

   // Certain properties are dropped when an item is disabled.
   if (!newEnabled) {
      if (scene && scene->mouseGrabberItem() == q_ptr) {
         q_ptr->ungrabMouse();
      }
      if (q_ptr->hasFocus()) {
         // Disabling the closest non-panel ancestor of the focus item
         // causes focus to pop to the next item, otherwise it's cleared.
         QGraphicsItem *focusItem = scene->focusItem();
         bool clear = true;
         if (isWidget && !focusItem->isPanel() && q_ptr->isAncestorOf(focusItem)) {
            do {
               if (focusItem == q_ptr) {
                  clear = ! static_cast<QGraphicsWidget *>(q_ptr)->focusNextPrevChild(true);
                  break;
               }
            } while ((focusItem = focusItem->parentWidget()) && !focusItem->isPanel());
         }

         if (clear) {
            q_ptr->clearFocus();
         }

      }
      if (q_ptr->isSelected()) {
         q_ptr->setSelected(false);
      }
   }

   // Modify the property.
   const QVariant newEnabledVariant(q_ptr->itemChange(QGraphicsItem::ItemEnabledChange,
         quint32(newEnabled)));
   enabled = newEnabledVariant.toBool();

   // Schedule redraw.
   if (update) {
      q_ptr->update();
   }

   for (QGraphicsItem *child : children) {
      if (!newEnabled || !child->d_ptr->explicitlyDisabled) {
         child->d_ptr->setEnabledHelper(newEnabled, /* explicitly = */ false);
      }
   }

   // Deliver post-change notification.
   q_ptr->itemChange(QGraphicsItem::ItemEnabledHasChanged, newEnabledVariant);

   if (isObject) {
      emit static_cast<QGraphicsObject *>(q_ptr)->enabledChanged();
   }
}

void QGraphicsItem::setEnabled(bool enabled)
{
   d_ptr->setEnabledHelper(enabled, /* explicitly = */ true);
}

bool QGraphicsItem::isSelected() const
{
   if (QGraphicsItemGroup *group = this->group()) {
      return group->isSelected();
   }
   return d_ptr->selected;
}

void QGraphicsItem::setSelected(bool selected)
{
   if (QGraphicsItemGroup *group = this->group()) {
      group->setSelected(selected);
      return;
   }

   if (! (d_ptr->itemFlags & ItemIsSelectable) || !d_ptr->enabled || !d_ptr->visible) {
      selected = false;
   }

   if (d_ptr->selected == selected) {
      return;
   }

   const QVariant newSelectedVariant(itemChange(ItemSelectedChange, quint32(selected)));
   bool newSelected = newSelectedVariant.toBool();

   if (d_ptr->selected == newSelected) {
      return;
   }

   d_ptr->selected = newSelected;

   update();
   if (d_ptr->scene) {
      QGraphicsScenePrivate *sceneD = d_ptr->scene->d_func();

      if (selected) {
         sceneD->selectedItems << this;
      } else {
         // QGraphicsScene::selectedItems() lazily pulls out all items that are
         // no longer selected.
      }
      if (! sceneD->selectionChanging) {
         emit d_ptr->scene->selectionChanged();
      }
   }

   // Deliver post-change notification.
   itemChange(QGraphicsItem::ItemSelectedHasChanged, newSelectedVariant);
}

qreal QGraphicsItem::opacity() const
{
   return d_ptr->opacity;
}

qreal QGraphicsItem::effectiveOpacity() const
{
   return d_ptr->effectiveOpacity();
}

void QGraphicsItem::setOpacity(qreal opacity)
{
   // Notify change.
   const QVariant newOpacityVariant(itemChange(ItemOpacityChange, opacity));

   // Normalized opacity
   qreal newOpacity = qBound(qreal(0), newOpacityVariant.toReal(), qreal(1));

   // No change? Done.
   if (newOpacity == d_ptr->opacity) {
      return;
   }

   bool wasFullyTransparent = d_ptr->isOpacityNull();
   d_ptr->opacity = newOpacity;

   // Notify change.
   itemChange(ItemOpacityHasChanged, newOpacityVariant);

   // Update
   if (d_ptr->scene) {

#ifndef QT_NO_GRAPHICSEFFECT
      d_ptr->invalidateParentGraphicsEffectsRecursively();

      if (! (d_ptr->itemFlags & ItemDoesntPropagateOpacityToChildren)) {
         d_ptr->invalidateChildGraphicsEffectsRecursively(QGraphicsItemPrivate::OpacityChanged);
      }
#endif

      d_ptr->scene->d_func()->markDirty(this, QRectF(),
         /*invalidateChildren=*/true,
         /*force=*/false,
         /*ignoreOpacity=*/d_ptr->isOpacityNull());
      if (wasFullyTransparent) {
         d_ptr->paintedViewBoundingRectsNeedRepaint = 1;
      }
   }

   if (d_ptr->isObject) {
      emit static_cast<QGraphicsObject *>(this)->opacityChanged();
   }
}

#ifndef QT_NO_GRAPHICSEFFECT
QGraphicsEffect *QGraphicsItem::graphicsEffect() const
{
   return d_ptr->graphicsEffect;
}
void QGraphicsItem::setGraphicsEffect(QGraphicsEffect *effect)
{
   if (d_ptr->graphicsEffect == effect) {
      return;
   }

   if (d_ptr->graphicsEffect) {
      delete d_ptr->graphicsEffect;
      d_ptr->graphicsEffect = nullptr;
   } else if (d_ptr->parent) {
      d_ptr->parent->d_ptr->updateChildWithGraphicsEffectFlagRecursively();
   }

   if (effect) {
      // Set new effect.
      QGraphicsEffectSourcePrivate *sourced = new QGraphicsItemEffectSourcePrivate(this);
      QGraphicsEffectSource *source = new QGraphicsEffectSource(*sourced);
      d_ptr->graphicsEffect = effect;
      effect->d_func()->setGraphicsEffectSource(source);
      prepareGeometryChange();
   }
}
#endif

void QGraphicsItemPrivate::updateChildWithGraphicsEffectFlagRecursively()
{
#ifndef QT_NO_GRAPHICSEFFECT
   QGraphicsItemPrivate *itemPrivate = this;
   do {
      // parent chain already notified?
      if (itemPrivate->mayHaveChildWithGraphicsEffect) {
         return;
      }
      itemPrivate->mayHaveChildWithGraphicsEffect = 1;
   } while ((itemPrivate = itemPrivate->parent ? itemPrivate->parent->d_ptr.data() : nullptr));
#endif
}

QRectF QGraphicsItemPrivate::effectiveBoundingRect(const QRectF &rect) const
{
#ifndef QT_NO_GRAPHICSEFFECT
   Q_Q(const QGraphicsItem);
   QGraphicsEffect *effect = graphicsEffect;
   if (scene && effect && effect->isEnabled()) {
      if (scene->d_func()->views.isEmpty()) {
         return effect->boundingRectFor(rect);
      }

      QRectF sceneRect = q->mapRectToScene(rect);
      QRectF sceneEffectRect;

      for (QGraphicsView *view : scene->views()) {
         QRectF deviceRect = view->d_func()->mapRectFromScene(sceneRect);
         QRect deviceEffectRect = effect->boundingRectFor(deviceRect).toAlignedRect();
         sceneEffectRect |= view->d_func()->mapRectToScene(deviceEffectRect);
      }
      return q->mapRectFromScene(sceneEffectRect);
   }
#endif

   return rect;
}

QRectF QGraphicsItemPrivate::effectiveBoundingRect(QGraphicsItem *topMostEffectItem) const
{
#ifndef QT_NO_GRAPHICSEFFECT
   Q_Q(const QGraphicsItem);

   QRectF brect = effectiveBoundingRect(q_ptr->boundingRect());

   if (ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren
      || ancestorFlags & QGraphicsItemPrivate::AncestorContainsChildren
      || topMostEffectItem == q) {
      return brect;
   }

   const QGraphicsItem *effectParent = parent;
   while (effectParent) {
      QGraphicsEffect *effect = effectParent->d_ptr->graphicsEffect;
      if (scene && effect && effect->isEnabled()) {
         const QRectF brectInParentSpace = q->mapRectToItem(effectParent, brect);
         const QRectF effectRectInParentSpace = effectParent->d_ptr->effectiveBoundingRect(brectInParentSpace);
         brect = effectParent->mapRectToItem(q, effectRectInParentSpace);
      }
      if (effectParent->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren
         || effectParent->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorContainsChildren
         || topMostEffectItem == effectParent) {
         return brect;
      }
      effectParent = effectParent->d_ptr->parent;
   }

   return brect;

#else
   return q_ptr->boundingRect();

#endif

}

QRectF QGraphicsItemPrivate::sceneEffectiveBoundingRect() const
{
   // Find translate-only offset
   // COMBINE
   QPointF offset;
   const QGraphicsItem *parentItem = q_ptr;
   const QGraphicsItemPrivate *itemd;
   do {
      itemd = parentItem->d_ptr.data();
      if (itemd->transformData) {
         break;
      }
      offset += itemd->pos;
   } while ((parentItem = itemd->parent));

   QRectF br = effectiveBoundingRect();
   br.translate(offset);
   return !parentItem ? br : parentItem->sceneTransform().mapRect(br);
}

bool QGraphicsItem::acceptDrops() const
{
   return d_ptr->acceptDrops;
}

void QGraphicsItem::setAcceptDrops(bool on)
{
   d_ptr->acceptDrops = on;
}

Qt::MouseButtons QGraphicsItem::acceptedMouseButtons() const
{
   return Qt::MouseButtons(d_ptr->acceptedMouseButtons);
}

void QGraphicsItem::setAcceptedMouseButtons(Qt::MouseButtons buttons)
{
   if (Qt::MouseButtons(d_ptr->acceptedMouseButtons) != buttons) {
      if (buttons == 0 && d_ptr->scene && d_ptr->scene->mouseGrabberItem() == this
         && d_ptr->scene->d_func()->lastMouseGrabberItemHasImplicitMouseGrab) {
         ungrabMouse();
      }
      d_ptr->acceptedMouseButtons = quint32(buttons);
   }
}

bool QGraphicsItem::acceptHoverEvents() const
{
   return d_ptr->acceptsHover;
}

void QGraphicsItem::setAcceptHoverEvents(bool enabled)
{
   if (d_ptr->acceptsHover == quint32(enabled)) {
      return;
   }

   d_ptr->acceptsHover = quint32(enabled);

   if (d_ptr->acceptsHover && d_ptr->scene && d_ptr->scene->d_func()->allItemsIgnoreHoverEvents) {
      d_ptr->scene->d_func()->allItemsIgnoreHoverEvents = false;
      d_ptr->scene->d_func()->enableMouseTrackingOnViews();
   }
}

bool QGraphicsItem::acceptTouchEvents() const
{
   return d_ptr->acceptTouchEvents;
}

void QGraphicsItem::setAcceptTouchEvents(bool enabled)
{
   if (d_ptr->acceptTouchEvents == quint32(enabled)) {
      return;
   }
   d_ptr->acceptTouchEvents = quint32(enabled);
   if (d_ptr->acceptTouchEvents && d_ptr->scene && d_ptr->scene->d_func()->allItemsIgnoreTouchEvents) {
      d_ptr->scene->d_func()->allItemsIgnoreTouchEvents = false;
      d_ptr->scene->d_func()->enableTouchEventsOnViews();
   }
}

bool QGraphicsItem::filtersChildEvents() const
{
   return d_ptr->filtersDescendantEvents;
}

void QGraphicsItem::setFiltersChildEvents(bool enabled)
{
   if (d_ptr->filtersDescendantEvents == enabled) {
      return;
   }

   d_ptr->filtersDescendantEvents = enabled;
   d_ptr->updateAncestorFlag(QGraphicsItem::GraphicsItemFlag(-2));
}

// obsolete
bool QGraphicsItem::handlesChildEvents() const
{
   return d_ptr->handlesChildEvents;
}

// obsolete
void QGraphicsItem::setHandlesChildEvents(bool enabled)
{
   if (d_ptr->handlesChildEvents == enabled) {
      return;
   }

   d_ptr->handlesChildEvents = enabled;
   d_ptr->updateAncestorFlag(QGraphicsItem::GraphicsItemFlag(-1));
}

bool QGraphicsItem::isActive() const
{
   if (!d_ptr->scene || !d_ptr->scene->isActive()) {
      return false;
   }
   return panel() == d_ptr->scene->activePanel();
}

void QGraphicsItem::setActive(bool active)
{
   d_ptr->explicitActivate = 1;
   d_ptr->wantsActive = active;

   if (d_ptr->scene) {
      if (active) {
         // Activate this item.
         d_ptr->scene->setActivePanel(this);

      } else {
         QGraphicsItem *activePanel = d_ptr->scene->activePanel();
         QGraphicsItem *thisPanel = panel();
         if (!activePanel || activePanel == thisPanel) {
            // Deactivate this item, and reactivate the last active item
            // (if any).
            QGraphicsItem *nextToActivate = nullptr;
            if (d_ptr->parent) {
               nextToActivate = d_ptr->parent->panel();
            }
            if (!nextToActivate) {
               nextToActivate = d_ptr->scene->d_func()->lastActivePanel;
            }
            if (nextToActivate == this || isAncestorOf(nextToActivate)) {
               nextToActivate = nullptr;
            }
            d_ptr->scene->setActivePanel(nextToActivate);
         }
      }
   }
}

bool QGraphicsItem::hasFocus() const
{
   if (! d_ptr->scene || ! d_ptr->scene->isActive()) {
      return false;
   }

   if (d_ptr->focusProxy) {
      return d_ptr->focusProxy->hasFocus();
   }

   if (d_ptr->scene->d_func()->focusItem != this) {
      return false;
   }

   return panel() == d_ptr->scene->d_func()->activePanel;
}

void QGraphicsItem::setFocus(Qt::FocusReason focusReason)
{
   d_ptr->setFocusHelper(focusReason, /* climb = */ true, /* focusFromHide = */ false);
}

void QGraphicsItemPrivate::setFocusHelper(Qt::FocusReason focusReason, bool climb, bool focusFromHide)
{
   // Disabled / unfocusable items cannot accept focus.
   if (! q_ptr->isEnabled() || ! (itemFlags & QGraphicsItem::ItemIsFocusable)) {
      return;
   }

   // Find focus proxy.
   QGraphicsItem *f = q_ptr;
   while (f->d_ptr->focusProxy) {
      f = f->d_ptr->focusProxy;
   }

   // Return if it already has focus.
   if (scene && scene->focusItem() == f) {
      return;
   }

   // Update focus scope item ptr.
   QGraphicsItem *p = parent;
   while (p) {
      if (p->flags() & QGraphicsItem::ItemIsFocusScope) {
         QGraphicsItem *oldFocusScopeItem = p->d_ptr->focusScopeItem;
         p->d_ptr->focusScopeItem = q_ptr;

         if (oldFocusScopeItem) {
            oldFocusScopeItem->d_ptr->focusScopeItemChange(false);
         }

         focusScopeItemChange(true);

         if (!p->focusItem() && !focusFromHide) {
            // Calling setFocus() on a child of a focus scope that does
            // not have focus changes only the focus scope pointer,
            // so that focus is restored the next time the scope gains
            // focus.
            return;
         }
         break;
      }
      p = p->d_ptr->parent;
   }

   if (climb) {
      while (f->d_ptr->focusScopeItem && f->d_ptr->focusScopeItem->isVisible() ) {
         f = f->d_ptr->focusScopeItem;
      }
   }

   // Update the child focus chain.
   QGraphicsItem *commonAncestor = nullptr;

   if (scene && scene->focusItem() && scene->focusItem()->panel() == q_ptr->panel()) {
      commonAncestor = scene->focusItem()->commonAncestorItem(f);
      scene->focusItem()->d_ptr->clearSubFocus(scene->focusItem(), commonAncestor);
   }

   f->d_ptr->setSubFocus(f, commonAncestor);

   // Update the scene's focus item.
   if (scene) {
      QGraphicsItem *p = q_ptr->panel();
      if ((!p && scene->isActive()) || (p && p->isActive())) {
         // Visible items immediately gain focus from scene.
         scene->d_func()->setFocusItemHelper(f, focusReason);
      }
   }
}

void QGraphicsItem::clearFocus()
{
   d_ptr->clearFocusHelper(true, false);
}

void QGraphicsItemPrivate::clearFocusHelper(bool giveFocusToParent, bool hiddenByParentPanel)
{
   QGraphicsItem *subFocusItem = q_ptr;
   if (itemFlags & QGraphicsItem::ItemIsFocusScope) {
      while (subFocusItem->d_ptr->focusScopeItem) {
         subFocusItem = subFocusItem->d_ptr->focusScopeItem;
      }
   }

   if (giveFocusToParent) {
      // Pass focus to the closest parent focus scope
      if (! inDestructor) {
         QGraphicsItem *p = parent;

         while (p) {
            if (p->flags() & QGraphicsItem::ItemIsFocusScope) {
               if (p->d_ptr->focusScopeItem == q_ptr) {
                  p->d_ptr->focusScopeItem = nullptr;

                  if (! subFocusItem->hasFocus()) {
                     // if it has focus, focusScopeItemChange is called elsewhere
                     focusScopeItemChange(false);
                  }
               }

               if (subFocusItem->hasFocus()) {
                  p->d_ptr->setFocusHelper(Qt::OtherFocusReason, false, false);
               }

               return;
            }

            p = p->d_ptr->parent;
         }
      }
   }

   if (subFocusItem->hasFocus()) {
      // Invisible items with focus must explicitly clear subfocus.
      if (! hiddenByParentPanel) {
         clearSubFocus(q_ptr);
      }

      // If this item has the scene's input focus, clear it.
      scene->setFocusItem(nullptr);
   }
}

QGraphicsItem *QGraphicsItem::focusProxy() const
{
   return d_ptr->focusProxy;
}

void QGraphicsItem::setFocusProxy(QGraphicsItem *item)
{
   if (item == d_ptr->focusProxy) {
      return;
   }
   if (item == this) {
      qWarning("QGraphicsItem::setFocusProxy() Unable to assign an item as a proxy for itself");
      return;
   }
   if (item) {
      if (item->d_ptr->scene != d_ptr->scene) {
         qWarning("QGraphicsItem::setFocusProxy() Focus proxy must be in the same QGraphicsScene");
         return;
      }
      for (QGraphicsItem *f = item->focusProxy(); f != nullptr; f = f->focusProxy()) {
         if (f == this) {
            qWarning("QGraphicsItem::setFocusProxy() Item is already a focus proxy");
            return;
         }
      }
   }

   QGraphicsItem *lastFocusProxy = d_ptr->focusProxy;
   if (lastFocusProxy) {
      lastFocusProxy->d_ptr->focusProxyRefs.removeOne(&d_ptr->focusProxy);
   }
   d_ptr->focusProxy = item;
   if (item) {
      item->d_ptr->focusProxyRefs << &d_ptr->focusProxy;
   }
}

QGraphicsItem *QGraphicsItem::focusItem() const
{
   return d_ptr->subFocusItem;
}

// internal
QGraphicsItem *QGraphicsItem::focusScopeItem() const
{
   return d_ptr->focusScopeItem;
}

void QGraphicsItem::grabMouse()
{
   if (!d_ptr->scene) {
      qWarning("QGraphicsItem::grabMouse() Unable to grab the mouse without a QGraphicsScene");
      return;
   }
   if (!d_ptr->visible) {
      qWarning("QGraphicsItem::grabMouse() Unable to grab the mouse while this item is invisible");
      return;
   }
   d_ptr->scene->d_func()->grabMouse(this);
}

void QGraphicsItem::ungrabMouse()
{
   if (!d_ptr->scene) {
      qWarning("QGraphicsItem::ungrabMouse() Unable to ungrab the mouse without a QGraphicsScene");
      return;
   }
   d_ptr->scene->d_func()->ungrabMouse(this);
}

void QGraphicsItem::grabKeyboard()
{
   if (!d_ptr->scene) {
      qWarning("QGraphicsItem::grabKeyboard() Unable to grab the keyboard without a QGraphicsScene");
      return;
   }
   if (!d_ptr->visible) {
      qWarning("QGraphicsItem::grabKeyboard() Unable to grab the keyboard while this item is invisible");
      return;
   }
   d_ptr->scene->d_func()->grabKeyboard(this);
}

void QGraphicsItem::ungrabKeyboard()
{
   if (!d_ptr->scene) {
      qWarning("QGraphicsItem::ungrabKeyboard() Unable to ungrab the keyboard without a QGraphicsScene");
      return;
   }
   d_ptr->scene->d_func()->ungrabKeyboard(this);
}

QPointF QGraphicsItem::pos() const
{
   return d_ptr->pos;
}

void QGraphicsItem::setX(qreal x)
{
   if (d_ptr->inDestructor) {
      return;
   }

   if (qIsNaN(x)) {
      return;
   }

   setPos(QPointF(x, d_ptr->pos.y()));
}

void QGraphicsItem::setY(qreal y)
{
   if (d_ptr->inDestructor) {
      return;
   }

   if (qIsNaN(y)) {
      return;
   }

   setPos(QPointF(d_ptr->pos.x(), y));
}

QPointF QGraphicsItem::scenePos() const
{
   return mapToScene(0, 0);
}

void QGraphicsItemPrivate::setPosHelper(const QPointF &pos)
{
   Q_Q(QGraphicsItem);

   inSetPosHelper = 1;
   if (scene) {
      q->prepareGeometryChange();
   }

   QPointF oldPos = this->pos;
   this->pos = pos;
   dirtySceneTransform = 1;
   inSetPosHelper = 0;

   if (isObject) {
      if (pos.x() != oldPos.x()) {
         emit static_cast<QGraphicsObject *>(q_ptr)->xChanged();
      }
      if (pos.y() != oldPos.y()) {
         emit static_cast<QGraphicsObject *>(q_ptr)->yChanged();
      }
   }
}

void QGraphicsItemPrivate::setTransformHelper(const QTransform &transform)
{
   q_ptr->prepareGeometryChange();
   transformData->transform = transform;
   dirtySceneTransform = 1;
   transformChanged();
}

void QGraphicsItem::setPos(const QPointF &pos)
{
   if (d_ptr->pos == pos) {
      return;
   }

   if (d_ptr->inDestructor) {
      return;
   }

   // Update and repositition.
   if (! (d_ptr->itemFlags & (ItemSendsGeometryChanges | ItemSendsScenePositionChanges))) {
      d_ptr->setPosHelper(pos);
      if (d_ptr->isWidget) {
         static_cast<QGraphicsWidget *>(this)->d_func()->setGeometryFromSetPos();
      }

      if (d_ptr->scenePosDescendants) {
         d_ptr->sendScenePosChange();
      }

      return;
   }

   // Notify the item that the position is changing.
   const QVariant newPosVariant(itemChange(ItemPositionChange, QVariant::fromValue<QPointF>(pos)));
   QPointF newPos = newPosVariant.toPointF();

   if (newPos == d_ptr->pos) {
      return;
   }

   // Update and repositition.
   d_ptr->setPosHelper(newPos);

   // Send post-notification.
   itemChange(QGraphicsItem::ItemPositionHasChanged, newPosVariant);
   d_ptr->sendScenePosChange();
}

void QGraphicsItem::ensureVisible(const QRectF &rect, int xmargin, int ymargin)
{
   if (d_ptr->scene) {
      QRectF sceneRect;

      if (!rect.isNull()) {
         sceneRect = sceneTransform().mapRect(rect);
      } else {
         sceneRect = sceneBoundingRect();
      }

      for (QGraphicsView *view : d_ptr->scene->d_func()->views) {
         view->ensureVisible(sceneRect, xmargin, ymargin);
      }
   }
}

// obsolete
QMatrix QGraphicsItem::matrix() const
{
   return transform().toAffine();
}

QTransform QGraphicsItem::transform() const
{
   if (!d_ptr->transformData) {
      return QTransform();
   }
   return d_ptr->transformData->transform;
}

qreal QGraphicsItem::rotation() const
{
   if (! d_ptr->transformData) {
      return 0;
   }
   return d_ptr->transformData->rotation;
}

void QGraphicsItem::setRotation(qreal angle)
{
   prepareGeometryChange();
   qreal newRotation = angle;

   if (d_ptr->itemFlags & ItemSendsGeometryChanges) {
      // Notify the item that the rotation is changing.
      const QVariant newRotationVariant(itemChange(ItemRotationChange, angle));
      newRotation = newRotationVariant.toReal();
   }

   if (!d_ptr->transformData) {
      d_ptr->transformData = new QGraphicsItemPrivate::TransformData;
   }

   if (d_ptr->transformData->rotation == newRotation) {
      return;
   }

   d_ptr->transformData->rotation = newRotation;
   d_ptr->transformData->onlyTransform = false;
   d_ptr->dirtySceneTransform = 1;

   // Send post-notification.
   if (d_ptr->itemFlags & ItemSendsGeometryChanges) {
      itemChange(ItemRotationHasChanged, newRotation);
   }

   if (d_ptr->isObject) {
      emit static_cast<QGraphicsObject *>(this)->rotationChanged();
   }

   d_ptr->transformChanged();
}

qreal QGraphicsItem::scale() const
{
   if (! d_ptr->transformData) {
      return 1.;
   }
   return d_ptr->transformData->scale;
}

void QGraphicsItem::setScale(qreal factor)
{
   prepareGeometryChange();
   qreal newScale = factor;

   if (d_ptr->itemFlags & ItemSendsGeometryChanges) {
      // Notify the item that the scale is changing.
      const QVariant newScaleVariant(itemChange(ItemScaleChange, factor));
      newScale = newScaleVariant.toReal();
   }

   if (!d_ptr->transformData) {
      d_ptr->transformData = new QGraphicsItemPrivate::TransformData;
   }

   if (d_ptr->transformData->scale == newScale) {
      return;
   }

   d_ptr->transformData->scale = newScale;
   d_ptr->transformData->onlyTransform = false;
   d_ptr->dirtySceneTransform = 1;

   // Send post-notification.
   if (d_ptr->itemFlags & ItemSendsGeometryChanges) {
      itemChange(ItemScaleHasChanged, newScale);
   }

   if (d_ptr->isObject) {
      emit static_cast<QGraphicsObject *>(this)->scaleChanged();
   }

   d_ptr->transformChanged();
}

QList<QGraphicsTransform *> QGraphicsItem::transformations() const
{
   if (!d_ptr->transformData) {
      return QList<QGraphicsTransform *>();
   }
   return d_ptr->transformData->graphicsTransforms;
}

void QGraphicsItem::setTransformations(const QList<QGraphicsTransform *> &transformations)
{
   prepareGeometryChange();
   if (! d_ptr->transformData) {
      d_ptr->transformData = new QGraphicsItemPrivate::TransformData;
   }
   d_ptr->transformData->graphicsTransforms = transformations;
   for (int i = 0; i < transformations.size(); ++i) {
      transformations.at(i)->d_func()->setItem(this);
   }
   d_ptr->transformData->onlyTransform = false;
   d_ptr->dirtySceneTransform = 1;
   d_ptr->transformChanged();
}

void QGraphicsItemPrivate::prependGraphicsTransform(QGraphicsTransform *t)
{
   if (!transformData) {
      transformData = new QGraphicsItemPrivate::TransformData;
   }
   if (!transformData->graphicsTransforms.contains(t)) {
      transformData->graphicsTransforms.prepend(t);
   }

   Q_Q(QGraphicsItem);
   t->d_func()->setItem(q);
   transformData->onlyTransform = false;
   dirtySceneTransform = 1;
   transformChanged();
}


void QGraphicsItemPrivate::appendGraphicsTransform(QGraphicsTransform *t)
{
   if (!transformData) {
      transformData = new QGraphicsItemPrivate::TransformData;
   }
   if (!transformData->graphicsTransforms.contains(t)) {
      transformData->graphicsTransforms.append(t);
   }

   Q_Q(QGraphicsItem);
   t->d_func()->setItem(q);
   transformData->onlyTransform = false;
   dirtySceneTransform = 1;
   transformChanged();
}

QPointF QGraphicsItem::transformOriginPoint() const
{
   if (!d_ptr->transformData) {
      return QPointF(0, 0);
   }
   return QPointF(d_ptr->transformData->xOrigin, d_ptr->transformData->yOrigin);
}

void QGraphicsItem::setTransformOriginPoint(const QPointF &origin)
{
   prepareGeometryChange();
   QPointF newOrigin = origin;

   if (d_ptr->itemFlags & ItemSendsGeometryChanges) {
      // Notify the item that the origin point is changing.
      const QVariant newOriginVariant(itemChange(ItemTransformOriginPointChange,
            QVariant::fromValue<QPointF>(origin)));
      newOrigin = newOriginVariant.toPointF();
   }

   if (!d_ptr->transformData) {
      d_ptr->transformData = new QGraphicsItemPrivate::TransformData;
   }

   if (d_ptr->transformData->xOrigin == newOrigin.x()
      && d_ptr->transformData->yOrigin == newOrigin.y()) {
      return;
   }

   d_ptr->transformData->xOrigin = newOrigin.x();
   d_ptr->transformData->yOrigin = newOrigin.y();
   d_ptr->transformData->onlyTransform = false;
   d_ptr->dirtySceneTransform = 1;

   // Send post-notification.
   if (d_ptr->itemFlags & ItemSendsGeometryChanges) {
      itemChange(ItemTransformOriginPointHasChanged, QVariant::fromValue<QPointF>(newOrigin));
   }
}

// obsolete
QMatrix QGraphicsItem::sceneMatrix() const
{
   d_ptr->ensureSceneTransform();
   return d_ptr->sceneTransform.toAffine();
}

QTransform QGraphicsItem::sceneTransform() const
{
   d_ptr->ensureSceneTransform();
   return d_ptr->sceneTransform;
}

QTransform QGraphicsItem::deviceTransform(const QTransform &viewportTransform) const
{
   // Ensure we return the standard transform if we're not untransformable.
   if (!d_ptr->itemIsUntransformable()) {
      d_ptr->ensureSceneTransform();
      return d_ptr->sceneTransform * viewportTransform;
   }

   // Find the topmost item that ignores view transformations.
   const QGraphicsItem *untransformedAncestor = this;
   QList<const QGraphicsItem *> parents;
   while (untransformedAncestor && ((untransformedAncestor->d_ptr->ancestorFlags
            & QGraphicsItemPrivate::AncestorIgnoresTransformations))) {
      parents.prepend(untransformedAncestor);
      untransformedAncestor = untransformedAncestor->parentItem();
   }

   if (!untransformedAncestor) {
      // Assert in debug mode, continue in release.
      Q_ASSERT_X(untransformedAncestor, "QGraphicsItem::deviceTransform",
         "Invalid object structure!");
      return QTransform();
   }

   // Determine the inherited origin. Find the parent of the topmost untransformable.
   // Use its scene transform to map the position of the untransformable. Then use
   // that viewport position as the anchoring point for the untransformable subtree.
   QGraphicsItem *parentOfUntransformedAncestor = untransformedAncestor->parentItem();
   QTransform inheritedMatrix;
   if (parentOfUntransformedAncestor) {
      inheritedMatrix = parentOfUntransformedAncestor->sceneTransform();
   }
   QPointF mappedPoint = (inheritedMatrix * viewportTransform).map(untransformedAncestor->pos());

   // COMBINE
   QTransform matrix = QTransform::fromTranslate(mappedPoint.x(), mappedPoint.y());
   if (untransformedAncestor->d_ptr->transformData) {
      matrix = untransformedAncestor->d_ptr->transformData->computedFullTransform(&matrix);
   }

   // Then transform and translate all children.
   for (int i = 0; i < parents.size(); ++i) {
      const QGraphicsItem *parent = parents.at(i);
      parent->d_ptr->combineTransformFromParent(&matrix);
   }

   return matrix;
}

QTransform QGraphicsItem::itemTransform(const QGraphicsItem *other, bool *ok) const
{
   // Catch simple cases first.
   if (other == nullptr) {
      qWarning("QGraphicsItem::itemTransform() Invalid item (nullptr)");
      return QTransform();
   }
   if (other == this) {
      if (ok) {
         *ok = true;
      }
      return QTransform();
   }

   QGraphicsItem *parent = d_ptr->parent;
   const QGraphicsItem *otherParent = other->d_ptr->parent;

   // This is other's child
   if (parent == other) {
      if (ok) {
         *ok = true;
      }
      QTransform x;
      d_ptr->combineTransformFromParent(&x);
      return x;
   }

   // This is other's parent
   if (otherParent == this) {
      const QPointF &otherPos = other->d_ptr->pos;
      if (other->d_ptr->transformData) {
         QTransform otherToParent;
         other->d_ptr->combineTransformFromParent(&otherToParent);
         return otherToParent.inverted(ok);
      }
      if (ok) {
         *ok = true;
      }
      return QTransform::fromTranslate(-otherPos.x(), -otherPos.y());
   }

   // Siblings
   if (parent == otherParent) {
      // COMBINE
      const QPointF &itemPos = d_ptr->pos;
      const QPointF &otherPos = other->d_ptr->pos;
      if (!d_ptr->transformData && !other->d_ptr->transformData) {
         QPointF delta = itemPos - otherPos;
         if (ok) {
            *ok = true;
         }
         return QTransform::fromTranslate(delta.x(), delta.y());
      }

      QTransform itemToParent;
      d_ptr->combineTransformFromParent(&itemToParent);
      QTransform otherToParent;
      other->d_ptr->combineTransformFromParent(&otherToParent);
      return itemToParent * otherToParent.inverted(ok);
   }

   // Find the closest common ancestor. If the two items don't share an
   // ancestor, then the only way is to combine their scene transforms.
   const QGraphicsItem *commonAncestor = commonAncestorItem(other);
   if (!commonAncestor) {
      d_ptr->ensureSceneTransform();
      other->d_ptr->ensureSceneTransform();
      return d_ptr->sceneTransform * other->d_ptr->sceneTransform.inverted(ok);
   }

   // If the two items are cousins (in sibling branches), map both to the
   // common ancestor, and combine the two transforms.
   bool cousins = other != commonAncestor && this != commonAncestor;
   if (cousins) {
      bool good = false;
      QTransform thisToScene = itemTransform(commonAncestor, &good);
      QTransform otherToScene(Qt::NoData);

      if (good) {
         otherToScene = other->itemTransform(commonAncestor, &good);
      }
      if (!good) {
         if (ok) {
            *ok = false;
         }
         return QTransform();
      }
      return thisToScene * otherToScene.inverted(ok);
   }

   // One is an ancestor of the other; walk the chain.
   bool parentOfOther = isAncestorOf(other);
   const QGraphicsItem *child = parentOfOther ? other : this;
   const QGraphicsItem *root = parentOfOther ? this : other;

   QTransform x;
   const QGraphicsItem *p = child;
   do {
      p->d_ptr.data()->combineTransformToParent(&x);
   } while ((p = p->d_ptr->parent) && p != root);
   if (parentOfOther) {
      return x.inverted(ok);
   }
   if (ok) {
      *ok = true;
   }
   return x;
}

// obsolete
void QGraphicsItem::setMatrix(const QMatrix &matrix, bool combine)
{
   if (!d_ptr->transformData) {
      d_ptr->transformData = new QGraphicsItemPrivate::TransformData;
   }

   QTransform newTransform(combine ? QTransform(matrix) * d_ptr->transformData->transform : QTransform(matrix));
   if (d_ptr->transformData->transform == newTransform) {
      return;
   }

   // Update and set the new transformation.
   if (!(d_ptr->itemFlags & ItemSendsGeometryChanges)) {
      d_ptr->setTransformHelper(newTransform);
      return;
   }

   // Notify the item that the transformation matrix is changing.
   const QVariant newMatrixVariant = QVariant::fromValue<QMatrix>(newTransform.toAffine());

   newTransform = QTransform( (itemChange(ItemMatrixChange, newMatrixVariant)).value<QMatrix>());

   if (d_ptr->transformData->transform == newTransform) {
      return;
   }

   // Update and set the new transformation.
   d_ptr->setTransformHelper(newTransform);

   // Send post-notification.
   itemChange(ItemTransformHasChanged, QVariant::fromValue<QTransform>(newTransform));
}

void QGraphicsItem::setTransform(const QTransform &matrix, bool combine)
{
   if (!d_ptr->transformData) {
      d_ptr->transformData = new QGraphicsItemPrivate::TransformData;
   }

   QTransform newTransform(combine ? matrix * d_ptr->transformData->transform : matrix);
   if (d_ptr->transformData->transform == newTransform) {
      return;
   }

   // Update and set the new transformation.
   if (! (d_ptr->itemFlags & (ItemSendsGeometryChanges | ItemSendsScenePositionChanges))) {
      d_ptr->setTransformHelper(newTransform);
      if (d_ptr->scenePosDescendants) {
         d_ptr->sendScenePosChange();
      }
      return;
   }

   // Notify the item that the transformation matrix is changing.
   const QVariant newTransformVariant(itemChange(ItemTransformChange,
         QVariant::fromValue<QTransform>(newTransform)));

   newTransform = newTransformVariant.value<QTransform>();

   if (d_ptr->transformData->transform == newTransform) {
      return;
   }

   // Update and set the new transformation.
   d_ptr->setTransformHelper(newTransform);

   // Send post-notification.
   itemChange(ItemTransformHasChanged, newTransformVariant);
   d_ptr->sendScenePosChange();
}

// obsolete
void QGraphicsItem::resetMatrix()
{
   resetTransform();
}

void QGraphicsItem::resetTransform()
{
   setTransform(QTransform(), false);
}

void QGraphicsItem::advance(int phase)
{
   (void) phase;
}

qreal QGraphicsItem::zValue() const
{
   return d_ptr->z;
}

void QGraphicsItem::setZValue(qreal z)
{
   const QVariant newZVariant(itemChange(ItemZValueChange, z));
   qreal newZ = newZVariant.toReal();

   if (newZ == d_ptr->z) {
      return;
   }

   if (d_ptr->scene && d_ptr->scene->d_func()->indexMethod != QGraphicsScene::NoIndex) {
      // Z Value has changed, we have to notify the index.
      d_ptr->scene->d_func()->index->itemChange(this, ItemZValueChange, &newZ);
   }

   d_ptr->z = newZ;
   if (d_ptr->parent) {
      d_ptr->parent->d_ptr->needSortChildren = 1;
   } else if (d_ptr->scene) {
      d_ptr->scene->d_func()->needSortTopLevelItems = 1;
   }

   if (d_ptr->scene) {
      d_ptr->scene->d_func()->markDirty(this, QRectF(), /*invalidateChildren=*/true);
   }

   itemChange(ItemZValueHasChanged, newZVariant);

   if (d_ptr->itemFlags & ItemNegativeZStacksBehindParent) {
      setFlag(QGraphicsItem::ItemStacksBehindParent, z < qreal(0.0));
   }

   if (d_ptr->isObject) {
      emit static_cast<QGraphicsObject *>(this)->zChanged();
   }
}

void QGraphicsItemPrivate::ensureSequentialSiblingIndex()
{
   if (!sequentialOrdering) {
      std::sort(children.begin(), children.end(), insertionOrder);
      sequentialOrdering = 1;
      needSortChildren = 1;
   }

   if (holesInSiblingIndex) {
      holesInSiblingIndex = 0;
      for (int i = 0; i < children.size(); ++i) {
         children[i]->d_ptr->siblingIndex = i;
      }
   }
}

inline void QGraphicsItemPrivate::sendScenePosChange()
{
   Q_Q(QGraphicsItem);
   if (scene) {
      if (itemFlags & QGraphicsItem::ItemSendsScenePositionChanges) {
         q->itemChange(QGraphicsItem::ItemScenePositionHasChanged, q->scenePos());
      }

      if (scenePosDescendants) {
         for (QGraphicsItem *item : scene->d_func()->scenePosItems) {
            if (q->isAncestorOf(item)) {
               item->itemChange(QGraphicsItem::ItemScenePositionHasChanged, item->scenePos());
            }
         }
      }
   }
}

void QGraphicsItem::stackBefore(const QGraphicsItem *sibling)
{
   if (sibling == this) {
      return;
   }

   if (!sibling || d_ptr->parent != sibling->parentItem()) {
      qWarning("QGraphicsItem::stackBefore() Unable to change the stack order, items must have the same parent");
      return;
   }

   QList<QGraphicsItem *> *siblings = d_ptr->parent
      ? &d_ptr->parent->d_ptr->children
      : (d_ptr->scene ? &d_ptr->scene->d_func()->topLevelItems : nullptr);

   if (!siblings) {
       qWarning("QGraphicsItem::stackBefore() Unable to change the stack order, items must have the same parent");
      return;
   }

   // First, make sure that the sibling indexes have no holes. This also
   // marks the children list for sorting.
   if (d_ptr->parent) {
      d_ptr->parent->d_ptr->ensureSequentialSiblingIndex();
   } else {
      d_ptr->scene->d_func()->ensureSequentialTopLevelSiblingIndexes();
   }

   // Only move items with the same Z value, and that need moving.
   int siblingIndex = sibling->d_ptr->siblingIndex;
   int myIndex = d_ptr->siblingIndex;
   if (myIndex >= siblingIndex) {
      siblings->move(myIndex, siblingIndex);
      // Fixup the insertion ordering.
      for (int i = 0; i < siblings->size(); ++i) {
         int &index = siblings->at(i)->d_ptr->siblingIndex;
         if (i != siblingIndex && index >= siblingIndex && index <= myIndex) {
            ++index;
         }
      }
      d_ptr->siblingIndex = siblingIndex;
      for (int i = 0; i < siblings->size(); ++i) {
         int &index = siblings->at(i)->d_ptr->siblingIndex;
         if (i != siblingIndex && index >= siblingIndex && index <= myIndex) {
            siblings->at(i)->d_ptr->siblingOrderChange();
         }
      }
      d_ptr->siblingOrderChange();
   }
}

QRectF QGraphicsItem::childrenBoundingRect() const
{
   if (!d_ptr->dirtyChildrenBoundingRect) {
      return d_ptr->childrenBoundingRect;
   }

   d_ptr->childrenBoundingRect = QRectF();
   d_ptr->childrenBoundingRectHelper(nullptr, &d_ptr->childrenBoundingRect, nullptr);
   d_ptr->dirtyChildrenBoundingRect = 0;
   return d_ptr->childrenBoundingRect;
}

QRectF QGraphicsItem::sceneBoundingRect() const
{
   // Find translate-only offset
   // COMBINE
   QPointF offset;
   const QGraphicsItem *parentItem = this;
   const QGraphicsItemPrivate *itemd;
   do {
      itemd = parentItem->d_ptr.data();
      if (itemd->transformData) {
         break;
      }
      offset += itemd->pos;
   } while ((parentItem = itemd->parent));

   QRectF br = boundingRect();
   br.translate(offset);
   if (!parentItem) {
      return br;
   }
   if (parentItem->d_ptr->hasTranslateOnlySceneTransform()) {
      br.translate(parentItem->d_ptr->sceneTransform.dx(), parentItem->d_ptr->sceneTransform.dy());
      return br;
   }
   return parentItem->d_ptr->sceneTransform.mapRect(br);
}

QPainterPath QGraphicsItem::shape() const
{
   QPainterPath path;
   path.addRect(boundingRect());
   return path;
}

bool QGraphicsItem::isClipped() const
{
   Q_D(const QGraphicsItem);
   return (d->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren)
      || (d->itemFlags & QGraphicsItem::ItemClipsToShape);
}

QPainterPath QGraphicsItem::clipPath() const
{
   Q_D(const QGraphicsItem);
   if (!isClipped()) {
      return QPainterPath();
   }

   const QRectF thisBoundingRect(boundingRect());
   if (thisBoundingRect.isEmpty()) {
      return QPainterPath();
   }

   QPainterPath clip;
   // Start with the item's bounding rect.
   clip.addRect(thisBoundingRect);

   if (d->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren) {
      const QGraphicsItem *parent = this;
      const QGraphicsItem *lastParent = this;

      // Intersect any in-between clips starting at the top and moving downwards.
      while ((parent = parent->d_ptr->parent)) {
         if (parent->d_ptr->itemFlags & ItemClipsChildrenToShape) {
            // Map clip to the current parent and intersect with its shape/clipPath
            clip = lastParent->itemTransform(parent).map(clip);
            clip = clip.intersected(parent->shape());

            if (clip.isEmpty()) {
               return clip;
            }
            lastParent = parent;
         }

         if (!(parent->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren)) {
            break;
         }
      }

      if (lastParent != this) {
         // Map clip back to the item's transform.
         // ### what if itemtransform fails
         clip = lastParent->itemTransform(this).map(clip);
      }
   }

   if (d->itemFlags & ItemClipsToShape) {
      clip = clip.intersected(shape());
   }

   return clip;
}

bool QGraphicsItem::contains(const QPointF &point) const
{
   return isClipped() ? clipPath().contains(point) : shape().contains(point);
}

bool QGraphicsItem::collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode) const
{
   if (other == this) {
      return true;
   }
   if (!other) {
      return false;
   }
   // The items share the same clip if their closest clipper is the same, or
   // if one clips the other.
   bool clips = (d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren);
   bool otherClips = (other->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren);
   if (clips || otherClips) {
      const QGraphicsItem *closestClipper = isAncestorOf(other) ? this : parentItem();
      while (closestClipper && !(closestClipper->flags() & ItemClipsChildrenToShape)) {
         closestClipper = closestClipper->parentItem();
      }
      const QGraphicsItem *otherClosestClipper = other->isAncestorOf(this) ? other : other->parentItem();
      while (otherClosestClipper && !(otherClosestClipper->flags() & ItemClipsChildrenToShape)) {
         otherClosestClipper = otherClosestClipper->parentItem();
      }
      if (closestClipper == otherClosestClipper) {
         d_ptr->localCollisionHack = 1;
         bool res = collidesWithPath(mapFromItem(other, other->shape()), mode);
         d_ptr->localCollisionHack = 0;
         return res;
      }
   }

   QPainterPath otherShape = other->isClipped() ? other->clipPath() : other->shape();
   return collidesWithPath(mapFromItem(other, otherShape), mode);
}

bool QGraphicsItem::collidesWithPath(const QPainterPath &path, Qt::ItemSelectionMode mode) const
{
   if (path.isEmpty()) {
      // No collision with empty paths.
      return false;
   }

   QRectF rectA(boundingRect());
   _q_adjustRect(&rectA);
   QRectF rectB(path.controlPointRect());
   _q_adjustRect(&rectB);
   if (!rectA.intersects(rectB)) {
      // This we can determine efficiently. If the two rects neither
      // intersect nor contain eachother, then the two items do not collide.
      return false;
   }

   // For further testing, we need this item's shape or bounding rect.
   QPainterPath thisShape;
   if (mode == Qt::IntersectsItemShape || mode == Qt::ContainsItemShape) {
      thisShape = (isClipped() && !d_ptr->localCollisionHack) ? clipPath() : shape();
   } else {
      thisShape.addRect(rectA);
   }

   if (thisShape == QPainterPath()) {
      // Empty shape? No collision.
      return false;
   }

   // Use QPainterPath boolean operations to determine the collision, O(N*logN).
   if (mode == Qt::IntersectsItemShape || mode == Qt::IntersectsItemBoundingRect) {
      return path.intersects(thisShape);
   }
   return path.contains(thisShape);
}

QList<QGraphicsItem *> QGraphicsItem::collidingItems(Qt::ItemSelectionMode mode) const
{
   if (d_ptr->scene) {
      return d_ptr->scene->collidingItems(this, mode);
   }
   return QList<QGraphicsItem *>();
}

// internal
static bool qt_QGraphicsItem_isObscured(const QGraphicsItem *item,
   const QGraphicsItem *other,
   const QRectF &rect)
{
   return other->mapToItem(item, other->opaqueArea()).contains(rect);
}

bool QGraphicsItem::isObscured(const QRectF &rect) const
{
   Q_D(const QGraphicsItem);
   if (!d->scene) {
      return false;
   }

   QRectF br = boundingRect();
   QRectF testRect = rect.isNull() ? br : rect;

   for (QGraphicsItem *item : d->scene->items(mapToScene(br), Qt::IntersectsItemBoundingRect)) {
      if (item == this) {
         break;
      }
      if (qt_QGraphicsItem_isObscured(this, item, testRect)) {
         return true;
      }
   }
   return false;
}

bool QGraphicsItem::isObscuredBy(const QGraphicsItem *item) const
{
   if (!item) {
      return false;
   }
   return qt_closestItemFirst(item, this)
      && qt_QGraphicsItem_isObscured(this, item, boundingRect());
}

QPainterPath QGraphicsItem::opaqueArea() const
{
   return QPainterPath();
}

QRegion QGraphicsItem::boundingRegion(const QTransform &itemToDeviceTransform) const
{
   // ### Ideally we would have a better way to generate this region,
   // preferably something in the lines of QPainterPath::toRegion(QTransform)
   // coupled with a way to generate a painter path from a set of painter
   // operations (e.g., QPicture::toPainterPath() or so). The current
   // approach generates a bitmap with the size of the item's bounding rect
   // in device coordinates, scaled by b.r.granularity, then paints the item
   // into the bitmap, converts the result to a QRegion and scales the region
   // back to device space with inverse granularity.
   qreal granularity = boundingRegionGranularity();
   QRect deviceRect = itemToDeviceTransform.mapRect(boundingRect()).toRect();
   _q_adjustRect(&deviceRect);
   if (granularity == 0.0) {
      return QRegion(deviceRect);
   }

   int pad = 1;
   QSize bitmapSize(qMax(1, int(deviceRect.width() * granularity) + pad * 2),
      qMax(1, int(deviceRect.height() * granularity) + pad * 2));
   QImage mask(bitmapSize, QImage::Format_ARGB32_Premultiplied);
   mask.fill(0);
   QPainter p(&mask);
   p.setRenderHints(QPainter::Antialiasing);

   // Transform painter (### this code is from QGraphicsScene::drawItemHelper
   // and doesn't work properly with perspective transformations).
   QPointF viewOrigo = itemToDeviceTransform.map(QPointF(0,  0));
   QPointF offset = viewOrigo - deviceRect.topLeft();
   p.scale(granularity, granularity);
   p.translate(offset);
   p.translate(pad, pad);
   p.setWorldTransform(itemToDeviceTransform, true);
   p.translate(itemToDeviceTransform.inverted().map(QPointF(0, 0)));

   // Render
   QStyleOptionGraphicsItem option;
   const_cast<QGraphicsItem *>(this)->paint(&p, &option, nullptr);
   p.end();

   // Transform QRegion back to device space
   QTransform unscale = QTransform::fromScale(1 / granularity, 1 / granularity);
   QRegion r;
   QBitmap colorMask = QBitmap::fromImage(mask.createMaskFromColor(0));

   for (const QRect &rect : QRegion( colorMask ).rects()) {
      QRect xrect = unscale.mapRect(rect).translated(deviceRect.topLeft() - QPoint(pad, pad));
      r += xrect.adjusted(-1, -1, 1, 1) & deviceRect;
   }
   return r;
}

qreal QGraphicsItem::boundingRegionGranularity() const
{
   if (d_ptr->hasBoundingRegionGranularity) {
      return (d_ptr->extra(QGraphicsItemPrivate::ExtraBoundingRegionGranularity)).value<qreal>();

   } else {
      return 0;
   }
}

void QGraphicsItem::setBoundingRegionGranularity(qreal granularity)
{
   if (granularity < 0.0 || granularity > 1.0) {
      qWarning("QGraphicsItem::setBoundingRegionGranularity() Invalid granularity %g", granularity);
      return;
   }
   if (granularity == 0.0) {
      d_ptr->unsetExtra(QGraphicsItemPrivate::ExtraBoundingRegionGranularity);
      d_ptr->hasBoundingRegionGranularity = 0;
      return;
   }
   d_ptr->hasBoundingRegionGranularity = 1;
   d_ptr->setExtra(QGraphicsItemPrivate::ExtraBoundingRegionGranularity,
      QVariant::fromValue<qreal>(granularity));
}

bool QGraphicsItemPrivate::discardUpdateRequest(bool ignoreVisibleBit, bool ignoreDirtyBit,
   bool ignoreOpacity) const
{
   // No scene, or if the scene is updating everything, means we have nothing
   // to do. The only exception is if the scene tracks the growing scene rect.
   return !scene
      || (!visible && !ignoreVisibleBit && !this->ignoreVisible)
      || (!ignoreDirtyBit && fullUpdatePending)
      || (!ignoreOpacity && !this->ignoreOpacity && childrenCombineOpacity() && isFullyTransparent());
}

int QGraphicsItemPrivate::depth() const
{
   if (itemDepth == -1) {
      const_cast<QGraphicsItemPrivate *>(this)->resolveDepth();
   }

   return itemDepth;
}

// internal
#ifndef QT_NO_GRAPHICSEFFECT
void QGraphicsItemPrivate::invalidateParentGraphicsEffectsRecursively()
{
   QGraphicsItemPrivate *itemPrivate = this;
   do {
      if (itemPrivate->graphicsEffect) {
         itemPrivate->notifyInvalidated = 1;

         if (! itemPrivate->updateDueToGraphicsEffect) {
            static_cast<QGraphicsItemEffectSourcePrivate *>
                  (itemPrivate->graphicsEffect->d_func()->source->d_func())->invalidateCache();
         }
      }

   } while ((itemPrivate = itemPrivate->parent ? itemPrivate->parent->d_ptr.data() : nullptr));
}

void QGraphicsItemPrivate::invalidateChildGraphicsEffectsRecursively(QGraphicsItemPrivate::InvalidateReason reason)
{
   if (!mayHaveChildWithGraphicsEffect) {
      return;
   }

   for (int i = 0; i < children.size(); ++i) {
      QGraphicsItemPrivate *childPrivate = children.at(i)->d_ptr.data();
      if (reason == OpacityChanged && (childPrivate->itemFlags & QGraphicsItem::ItemIgnoresParentOpacity)) {
         continue;
      }

      if (childPrivate->graphicsEffect) {
         childPrivate->notifyInvalidated = 1;
         static_cast<QGraphicsItemEffectSourcePrivate *>(childPrivate->graphicsEffect->d_func()->source->d_func())->invalidateCache();
      }

      childPrivate->invalidateChildGraphicsEffectsRecursively(reason);
   }
}
#endif //QT_NO_GRAPHICSEFFECT

void QGraphicsItemPrivate::invalidateDepthRecursively()
{
   if (itemDepth == -1) {
      return;
   }

   itemDepth = -1;
   for (int i = 0; i < children.size(); ++i) {
      children.at(i)->d_ptr->invalidateDepthRecursively();
   }
}

void QGraphicsItemPrivate::resolveDepth()
{
   if (!parent) {
      itemDepth = 0;
   } else {
      if (parent->d_ptr->itemDepth == -1) {
         parent->d_ptr->resolveDepth();
      }
      itemDepth = parent->d_ptr->itemDepth + 1;
   }
}

void QGraphicsItemPrivate::addChild(QGraphicsItem *child)
{
   // Remove all holes from the sibling index list. Now the max index
   // number is equal to the size of the children list.
   ensureSequentialSiblingIndex();
   needSortChildren = 1; // ### maybe 0
   child->d_ptr->siblingIndex = children.size();
   children.append(child);
   if (isObject) {
      emit static_cast<QGraphicsObject *>(q_ptr)->childrenChanged();
   }
}

void QGraphicsItemPrivate::removeChild(QGraphicsItem *child)
{
   // When removing elements in the middle of the children list,
   // there will be a "gap" in the list of sibling indexes (0,1,3,4).
   if (!holesInSiblingIndex) {
      holesInSiblingIndex = child->d_ptr->siblingIndex != children.size() - 1;
   }
   if (sequentialOrdering && !holesInSiblingIndex) {
      children.removeAt(child->d_ptr->siblingIndex);
   } else {
      children.removeOne(child);
   }
   // NB! Do not use children.removeAt(child->d_ptr->siblingIndex) because
   // the child is not guaranteed to be at the index after the list is sorted.
   // (see ensureSortedChildren()).
   child->d_ptr->siblingIndex = -1;
   if (isObject) {
      emit static_cast<QGraphicsObject *>(q_ptr)->childrenChanged();
   }
}

// internal
QGraphicsItemCache *QGraphicsItemPrivate::maybeExtraItemCache() const
{
   return (QGraphicsItemCache *)(extra(ExtraCacheData)).value<void *>();
}

// internal
QGraphicsItemCache *QGraphicsItemPrivate::extraItemCache() const
{
   QGraphicsItemCache *c = (QGraphicsItemCache *)(extra(ExtraCacheData)).value<void *>();

   if (!c) {
      QGraphicsItemPrivate *that = const_cast<QGraphicsItemPrivate *>(this);
      c = new QGraphicsItemCache;
      that->setExtra(ExtraCacheData, QVariant::fromValue<void *>(c));
   }

   return c;
}

// internal
void QGraphicsItemPrivate::removeExtraItemCache()
{
   QGraphicsItemCache *c = (QGraphicsItemCache *)(extra(ExtraCacheData)).value<void *>();
   if (c) {
      c->purge();
      delete c;
   }
   unsetExtra(ExtraCacheData);
}

void QGraphicsItemPrivate::updatePaintedViewBoundingRects(bool updateChildren)
{
   if (!scene) {
      return;
   }

   for (int i = 0; i < scene->d_func()->views.size(); ++i) {
      QGraphicsViewPrivate *viewPrivate = scene->d_func()->views.at(i)->d_func();
      QRect rect = paintedViewBoundingRects.value(viewPrivate->viewport);
      rect.translate(viewPrivate->dirtyScrollOffset);
      viewPrivate->updateRect(rect);
   }

   if (updateChildren) {
      for (int i = 0; i < children.size(); ++i) {
         children.at(i)->d_ptr->updatePaintedViewBoundingRects(true);
      }
   }
}

// Traverses all the ancestors up to the top-level and updates the pointer to
// always point to the top-most item that has a dirty scene transform.
// It then backtracks to the top-most dirty item and start calculating the
// scene transform by combining the item's transform (+pos) with the parent's
// cached scene transform (which we at this point know for sure is valid).
void QGraphicsItemPrivate::ensureSceneTransformRecursive(QGraphicsItem **topMostDirtyItem)
{
   Q_ASSERT(topMostDirtyItem);

   if (dirtySceneTransform) {
      *topMostDirtyItem = q_ptr;
   }

   if (parent) {
      parent->d_ptr->ensureSceneTransformRecursive(topMostDirtyItem);
   }

   if (*topMostDirtyItem == q_ptr) {
      if (!dirtySceneTransform) {
         return;   // OK, neither my ancestors nor I have dirty scene transforms.
      }
      *topMostDirtyItem = nullptr;
   } else if (*topMostDirtyItem) {
      return; // Continue backtrack.
   }

   // This item and all its descendants have dirty scene transforms.
   // We're about to validate this item's scene transform, so we have to
   // invalidate all the children; otherwise there's no way for the descendants
   // to detect that the ancestor has changed.
   invalidateChildrenSceneTransform();

   // COMBINE my transform with the parent's scene transform.
   updateSceneTransformFromParent();
   Q_ASSERT(!dirtySceneTransform);
}


void QGraphicsItemPrivate::setSubFocus(QGraphicsItem *rootItem, QGraphicsItem *stopItem)
{
   // Update focus child chain. Stop at panels, or if this item
   // is hidden, stop at the first item with a visible parent.
   QGraphicsItem *parent = rootItem ? rootItem : q_ptr;
   if (parent->panel() != q_ptr->panel()) {
      return;
   }

   do {
      // Clear any existing ancestor's subFocusItem.
      if (parent != q_ptr && parent->d_ptr->subFocusItem) {
         if (parent->d_ptr->subFocusItem == q_ptr) {
            break;
         }
         parent->d_ptr->subFocusItem->d_ptr->clearSubFocus(nullptr, stopItem);
      }
      parent->d_ptr->subFocusItem = q_ptr;
      parent->d_ptr->subFocusItemChange();
   } while (!parent->isPanel() && (parent = parent->d_ptr->parent) && (visible || !parent->d_ptr->visible));

   if (scene && !scene->isActive()) {
      scene->d_func()->passiveFocusItem = subFocusItem;
      scene->d_func()->lastFocusItem = subFocusItem;
   }
}


void QGraphicsItemPrivate::clearSubFocus(QGraphicsItem *rootItem, QGraphicsItem *stopItem)
{
   // Reset sub focus chain.
   QGraphicsItem *parent = rootItem ? rootItem : q_ptr;
   do {
      if (parent->d_ptr->subFocusItem != q_ptr) {
         break;
      }
      parent->d_ptr->subFocusItem = nullptr;
      if (parent != stopItem && !parent->isAncestorOf(stopItem)) {
         parent->d_ptr->subFocusItemChange();
      }
   } while (!parent->isPanel() && (parent = parent->d_ptr->parent));
}

void QGraphicsItemPrivate::resetFocusProxy()
{
   for (int i = 0; i < focusProxyRefs.size(); ++i) {
      *focusProxyRefs.at(i) = nullptr;
   }
   focusProxyRefs.clear();
}

void QGraphicsItemPrivate::subFocusItemChange()
{
}

void QGraphicsItemPrivate::focusScopeItemChange(bool isSubFocusItem)
{
   (void) isSubFocusItem;
}

void QGraphicsItemPrivate::siblingOrderChange()
{
}

bool QGraphicsItemPrivate::isProxyWidget() const
{
   return false;
}

void QGraphicsItem::update(const QRectF &rect)
{
   if (rect.isEmpty() && ! rect.isNull()) {
      return;
   }

   // Make sure we notify effects about invalidated source.

#ifndef QT_NO_GRAPHICSEFFECT
   d_ptr->invalidateParentGraphicsEffectsRecursively();
#endif

   if (CacheMode(d_ptr->cacheMode) != NoCache) {
      // Invalidate cache.
      QGraphicsItemCache *cache = d_ptr->extraItemCache();

      if (!cache->allExposed) {
         if (rect.isNull()) {
            cache->allExposed = true;
            cache->exposed.clear();
         } else {
            cache->exposed.append(rect);
         }
      }

      // Only invalidate cache; item is already dirty.
      if (d_ptr->fullUpdatePending) {
         return;
      }
   }

   if (d_ptr->scene) {
      d_ptr->scene->d_func()->markDirty(this, rect);
   }
}

void QGraphicsItem::scroll(qreal dx, qreal dy, const QRectF &rect)
{
   Q_D(QGraphicsItem);
   if (dx == 0.0 && dy == 0.0) {
      return;
   }
   if (!d->scene) {
      return;
   }

   // Accelerated scrolling means moving pixels from one location to another
   // and only redraw the newly exposed area. The following requirements must
   // be fulfilled in order to do that:
   //
   // 1) Item is opaque.
   // 2) Item is not overlapped by other items.
   //
   // There's (yet) no way to detect whether an item is opaque or not, which means
   // we cannot do accelerated scrolling unless the cache is enabled. In case of using
   // DeviceCoordinate cache we also have to take the device transform into account in
   // order to determine whether we can do accelerated scrolling or not. That's left out
   // for simplicity here, but it is definitely something we can consider in the future
   // as a performance improvement.
   if (d->cacheMode != QGraphicsItem::ItemCoordinateCache
      || !qFuzzyIsNull(dx - int(dx)) || !qFuzzyIsNull(dy - int(dy))) {
      update(rect);
      return;
   }

   QGraphicsItemCache *cache = d->extraItemCache();
   if (cache->allExposed || cache->fixedSize.isValid()) {
      // Cache is either invalidated or item is scaled (see QGraphicsItem::setCacheMode).
      update(rect);
      return;
   }

   // Find pixmap in cache.
   QPixmap cachedPixmap;
   if (!QPixmapCache::find(cache->key, &cachedPixmap)) {
      update(rect);
      return;
   }

   QRect scrollRect = (rect.isNull() ? boundingRect() : rect).toAlignedRect();
   if (!scrollRect.intersects(cache->boundingRect)) {
      return;   // Nothing to scroll.
   }

   // Remove from cache to avoid deep copy when modifying.
   QPixmapCache::remove(cache->key);

   QRegion exposed;
   cachedPixmap.scroll(dx, dy, scrollRect.translated(-cache->boundingRect.topLeft()), &exposed);

   // Reinsert into cache.
   cache->key = QPixmapCache::insert(cachedPixmap);

   // Translate the existing expose.
   for (int i = 0; i < cache->exposed.size(); ++i) {
      QRectF &e = cache->exposed[i];
      if (!rect.isNull() && !e.intersects(rect)) {
         continue;
      }
      e.translate(dx, dy);
   }

   // Append newly exposed areas. Note that the exposed region is currently
   // in pixmap coordinates, so we have to translate it to item coordinates.
   exposed.translate(cache->boundingRect.topLeft());
   const QVector<QRect> exposedRects = exposed.rects();
   for (int i = 0; i < exposedRects.size(); ++i) {
      cache->exposed += exposedRects.at(i);
   }

   // Trigger update. This will redraw the newly exposed area and make sure
   // the pixmap is re-blitted in case there are overlapping items.
   d->scene->d_func()->markDirty(this, rect);
}

QPointF QGraphicsItem::mapToItem(const QGraphicsItem *item, const QPointF &point) const
{
   if (item) {
      return itemTransform(item).map(point);
   }

   return mapToScene(point);
}

QPointF QGraphicsItem::mapToParent(const QPointF &point) const
{
   // COMBINE
   if (!d_ptr->transformData) {
      return point + d_ptr->pos;
   }
   return d_ptr->transformToParent().map(point);
}

QPointF QGraphicsItem::mapToScene(const QPointF &point) const
{
   if (d_ptr->hasTranslateOnlySceneTransform()) {
      return QPointF(point.x() + d_ptr->sceneTransform.dx(), point.y() + d_ptr->sceneTransform.dy());
   }
   return d_ptr->sceneTransform.map(point);
}

QPolygonF QGraphicsItem::mapToItem(const QGraphicsItem *item, const QRectF &rect) const
{
   if (item) {
      return itemTransform(item).map(rect);
   }
   return mapToScene(rect);
}

QPolygonF QGraphicsItem::mapToParent(const QRectF &rect) const
{
   // COMBINE
   if (!d_ptr->transformData) {
      return rect.translated(d_ptr->pos);
   }
   return d_ptr->transformToParent().map(rect);
}

QPolygonF QGraphicsItem::mapToScene(const QRectF &rect) const
{
   if (d_ptr->hasTranslateOnlySceneTransform()) {
      return rect.translated(d_ptr->sceneTransform.dx(), d_ptr->sceneTransform.dy());
   }
   return d_ptr->sceneTransform.map(rect);
}

QRectF QGraphicsItem::mapRectToItem(const QGraphicsItem *item, const QRectF &rect) const
{
   if (item) {
      return itemTransform(item).mapRect(rect);
   }
   return mapRectToScene(rect);
}

QRectF QGraphicsItem::mapRectToParent(const QRectF &rect) const
{
   // COMBINE
   if (!d_ptr->transformData) {
      return rect.translated(d_ptr->pos);
   }
   return d_ptr->transformToParent().mapRect(rect);
}

QRectF QGraphicsItem::mapRectToScene(const QRectF &rect) const
{
   if (d_ptr->hasTranslateOnlySceneTransform()) {
      return rect.translated(d_ptr->sceneTransform.dx(), d_ptr->sceneTransform.dy());
   }
   return d_ptr->sceneTransform.mapRect(rect);
}

QRectF QGraphicsItem::mapRectFromItem(const QGraphicsItem *item, const QRectF &rect) const
{
   if (item) {
      return item->itemTransform(this).mapRect(rect);
   }
   return mapRectFromScene(rect);
}

QRectF QGraphicsItem::mapRectFromParent(const QRectF &rect) const
{
   // COMBINE
   if (!d_ptr->transformData) {
      return rect.translated(-d_ptr->pos);
   }
   return d_ptr->transformToParent().inverted().mapRect(rect);
}

QRectF QGraphicsItem::mapRectFromScene(const QRectF &rect) const
{
   if (d_ptr->hasTranslateOnlySceneTransform()) {
      return rect.translated(-d_ptr->sceneTransform.dx(), -d_ptr->sceneTransform.dy());
   }
   return d_ptr->sceneTransform.inverted().mapRect(rect);
}

QPolygonF QGraphicsItem::mapToItem(const QGraphicsItem *item, const QPolygonF &polygon) const
{
   if (item) {
      return itemTransform(item).map(polygon);
   }
   return mapToScene(polygon);
}

QPolygonF QGraphicsItem::mapToParent(const QPolygonF &polygon) const
{
   // COMBINE
   if (!d_ptr->transformData) {
      return polygon.translated(d_ptr->pos);
   }

   return d_ptr->transformToParent().map(polygon);
}

QPolygonF QGraphicsItem::mapToScene(const QPolygonF &polygon) const
{
   if (d_ptr->hasTranslateOnlySceneTransform()) {
      return polygon.translated(d_ptr->sceneTransform.dx(), d_ptr->sceneTransform.dy());
   }
   return d_ptr->sceneTransform.map(polygon);
}

QPainterPath QGraphicsItem::mapToItem(const QGraphicsItem *item, const QPainterPath &path) const
{
   if (item) {
      return itemTransform(item).map(path);
   }
   return mapToScene(path);
}

QPainterPath QGraphicsItem::mapToParent(const QPainterPath &path) const
{
   // COMBINE
   if (!d_ptr->transformData) {
      return path.translated(d_ptr->pos);
   }
   return d_ptr->transformToParent().map(path);
}

QPainterPath QGraphicsItem::mapToScene(const QPainterPath &path) const
{
   if (d_ptr->hasTranslateOnlySceneTransform()) {
      return path.translated(d_ptr->sceneTransform.dx(), d_ptr->sceneTransform.dy());
   }
   return d_ptr->sceneTransform.map(path);
}

QPointF QGraphicsItem::mapFromItem(const QGraphicsItem *item, const QPointF &point) const
{
   if (item) {
      return item->itemTransform(this).map(point);
   }
   return mapFromScene(point);
}

QPointF QGraphicsItem::mapFromParent(const QPointF &point) const
{
   // COMBINE
   if (d_ptr->transformData) {
      return d_ptr->transformToParent().inverted().map(point);
   }
   return point - d_ptr->pos;
}

QPointF QGraphicsItem::mapFromScene(const QPointF &point) const
{
   if (d_ptr->hasTranslateOnlySceneTransform()) {
      return QPointF(point.x() - d_ptr->sceneTransform.dx(), point.y() - d_ptr->sceneTransform.dy());
   }
   return d_ptr->sceneTransform.inverted().map(point);
}

QPolygonF QGraphicsItem::mapFromItem(const QGraphicsItem *item, const QRectF &rect) const
{
   if (item) {
      return item->itemTransform(this).map(rect);
   }
   return mapFromScene(rect);
}

QPolygonF QGraphicsItem::mapFromParent(const QRectF &rect) const
{
   // COMBINE
   if (!d_ptr->transformData) {
      return rect.translated(-d_ptr->pos);
   }
   return d_ptr->transformToParent().inverted().map(rect);
}

QPolygonF QGraphicsItem::mapFromScene(const QRectF &rect) const
{
   if (d_ptr->hasTranslateOnlySceneTransform()) {
      return rect.translated(-d_ptr->sceneTransform.dx(), -d_ptr->sceneTransform.dy());
   }
   return d_ptr->sceneTransform.inverted().map(rect);
}

QPolygonF QGraphicsItem::mapFromItem(const QGraphicsItem *item, const QPolygonF &polygon) const
{
   if (item) {
      return item->itemTransform(this).map(polygon);
   }
   return mapFromScene(polygon);
}

QPolygonF QGraphicsItem::mapFromParent(const QPolygonF &polygon) const
{
   // COMBINE
   if (!d_ptr->transformData) {
      return polygon.translated(-d_ptr->pos);
   }
   return d_ptr->transformToParent().inverted().map(polygon);
}

QPolygonF QGraphicsItem::mapFromScene(const QPolygonF &polygon) const
{
   if (d_ptr->hasTranslateOnlySceneTransform()) {
      return polygon.translated(-d_ptr->sceneTransform.dx(), -d_ptr->sceneTransform.dy());
   }
   return d_ptr->sceneTransform.inverted().map(polygon);
}

QPainterPath QGraphicsItem::mapFromItem(const QGraphicsItem *item, const QPainterPath &path) const
{
   if (item) {
      return item->itemTransform(this).map(path);
   }
   return mapFromScene(path);
}

QPainterPath QGraphicsItem::mapFromParent(const QPainterPath &path) const
{
   // COMBINE
   if (!d_ptr->transformData) {
      return path.translated(-d_ptr->pos);
   }
   return d_ptr->transformToParent().inverted().map(path);
}

QPainterPath QGraphicsItem::mapFromScene(const QPainterPath &path) const
{
   if (d_ptr->hasTranslateOnlySceneTransform()) {
      return path.translated(-d_ptr->sceneTransform.dx(), -d_ptr->sceneTransform.dy());
   }
   return d_ptr->sceneTransform.inverted().map(path);
}

bool QGraphicsItem::isAncestorOf(const QGraphicsItem *child) const
{
   if (!child || child == this) {
      return false;
   }
   if (child->d_ptr->depth() < d_ptr->depth()) {
      return false;
   }
   const QGraphicsItem *ancestor = child;
   while ((ancestor = ancestor->d_ptr->parent)) {
      if (ancestor == this) {
         return true;
      }
   }
   return false;
}

QGraphicsItem *QGraphicsItem::commonAncestorItem(const QGraphicsItem *other) const
{
   if (!other) {
      return nullptr;
   }
   if (other == this) {
      return const_cast<QGraphicsItem *>(this);
   }
   const QGraphicsItem *thisw = this;
   const QGraphicsItem *otherw = other;
   int thisDepth = d_ptr->depth();
   int otherDepth = other->d_ptr->depth();
   while (thisDepth > otherDepth) {
      thisw = thisw->d_ptr->parent;
      --thisDepth;
   }
   while (otherDepth > thisDepth) {
      otherw = otherw->d_ptr->parent;
      --otherDepth;
   }
   while (thisw && thisw != otherw) {
      thisw = thisw->d_ptr->parent;
      otherw = otherw->d_ptr->parent;
   }
   return const_cast<QGraphicsItem *>(thisw);
}

bool QGraphicsItem::isUnderMouse() const
{
   Q_D(const QGraphicsItem);
   if (!d->scene) {
      return false;
   }

   QPoint cursorPos = QCursor::pos();
   for (QGraphicsView *view : d->scene->views()) {
      if (contains(mapFromScene(view->mapToScene(view->mapFromGlobal(cursorPos))))) {
         return true;
      }
   }
   return false;
}

QVariant QGraphicsItem::data(int key) const
{
   QGraphicsItemCustomDataStore *store = qt_dataStore();
   if (!store->data.contains(this)) {
      return QVariant();
   }
   return store->data.value(this).value(key);
}

void QGraphicsItem::setData(int key, const QVariant &value)
{
   qt_dataStore()->data[this][key] = value;
}

int QGraphicsItem::type() const
{
   return (int)UserType;
}

void QGraphicsItem::installSceneEventFilter(QGraphicsItem *filterItem)
{
   if (!d_ptr->scene) {
      qWarning("QGraphicsItem::installSceneEventFilter() Event filters can only be installed"
         " on items in a QGraphicsScene");
      return;
   }
   if (d_ptr->scene != filterItem->scene()) {
      qWarning("QGraphicsItem::installSceneEventFilter() Event filters can only be installed"
         " on items in the same QGraphicsScene");
      return;
   }
   d_ptr->scene->d_func()->installSceneEventFilter(this, filterItem);
}

void QGraphicsItem::removeSceneEventFilter(QGraphicsItem *filterItem)
{
   if (!d_ptr->scene || d_ptr->scene != filterItem->scene()) {
      return;
   }
   d_ptr->scene->d_func()->removeSceneEventFilter(this, filterItem);
}

bool QGraphicsItem::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
   (void) watched;
   (void) event;

   return false;
}

bool QGraphicsItem::sceneEvent(QEvent *event)
{
   if (d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorHandlesChildEvents) {
      if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverLeave
         || event->type() == QEvent::DragEnter || event->type() == QEvent::DragLeave) {
         // Hover enter and hover leave events for children are ignored;
         // hover move events are forwarded.
         return true;
      }

      QGraphicsItem *handler = this;
      do {
         handler = handler->d_ptr->parent;
         Q_ASSERT(handler);
      } while (handler->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorHandlesChildEvents);
      // Forward the event to the closest parent that handles child
      // events, mapping existing item-local coordinates to its
      // coordinate system.
      d_ptr->remapItemPos(event, handler);
      handler->sceneEvent(event);
      return true;
   }

   if (event->type() == QEvent::FocusOut) {
      focusOutEvent(static_cast<QFocusEvent *>(event));
      return true;
   }

   if (!d_ptr->visible) {
      // Eaten
      return true;
   }

   switch (event->type()) {
      case QEvent::FocusIn:
         focusInEvent(static_cast<QFocusEvent *>(event));
         break;
      case QEvent::GraphicsSceneContextMenu:
         contextMenuEvent(static_cast<QGraphicsSceneContextMenuEvent *>(event));
         break;
      case QEvent::GraphicsSceneDragEnter:
         dragEnterEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
         break;
      case QEvent::GraphicsSceneDragMove:
         dragMoveEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
         break;
      case QEvent::GraphicsSceneDragLeave:
         dragLeaveEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
         break;
      case QEvent::GraphicsSceneDrop:
         dropEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
         break;
      case QEvent::GraphicsSceneHoverEnter:
         hoverEnterEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
         break;
      case QEvent::GraphicsSceneHoverMove:
         hoverMoveEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
         break;
      case QEvent::GraphicsSceneHoverLeave:
         hoverLeaveEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
         break;
      case QEvent::GraphicsSceneMouseMove:
         mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
         break;
      case QEvent::GraphicsSceneMousePress:
         mousePressEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
         break;
      case QEvent::GraphicsSceneMouseRelease:
         mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
         break;
      case QEvent::GraphicsSceneMouseDoubleClick:
         mouseDoubleClickEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
         break;
      case QEvent::GraphicsSceneWheel:
         wheelEvent(static_cast<QGraphicsSceneWheelEvent *>(event));
         break;
      case QEvent::KeyPress: {
         QKeyEvent *k = static_cast<QKeyEvent *>(event);
         if (k->key() == Qt::Key_Tab || k->key() == Qt::Key_Backtab) {
            if (!(k->modifiers() & (Qt::ControlModifier | Qt::AltModifier))) {  //### Add MetaModifier?
               bool res = false;
               if (k->key() == Qt::Key_Backtab
                  || (k->key() == Qt::Key_Tab && (k->modifiers() & Qt::ShiftModifier))) {
                  if (d_ptr->isWidget) {
                     res = static_cast<QGraphicsWidget *>(this)->focusNextPrevChild(false);
                  } else if (d_ptr->scene) {
                     res = d_ptr->scene->focusNextPrevChild(false);
                  }
               } else if (k->key() == Qt::Key_Tab) {
                  if (d_ptr->isWidget) {
                     res = static_cast<QGraphicsWidget *>(this)->focusNextPrevChild(true);
                  } else if (d_ptr->scene) {
                     res = d_ptr->scene->focusNextPrevChild(true);
                  }
               }
               if (!res) {
                  event->ignore();
               }
               return true;
            }
         }
         keyPressEvent(static_cast<QKeyEvent *>(event));
         break;
      }
      case QEvent::KeyRelease:
         keyReleaseEvent(static_cast<QKeyEvent *>(event));
         break;
      case QEvent::InputMethod:
         inputMethodEvent(static_cast<QInputMethodEvent *>(event));
         break;
      case QEvent::WindowActivate:
      case QEvent::WindowDeactivate:
         // Propagate panel activation.
         if (d_ptr->scene) {
            for (int i = 0; i < d_ptr->children.size(); ++i) {
               QGraphicsItem *child = d_ptr->children.at(i);
               if (child->isVisible() && !child->isPanel()) {
                  if (!(child->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorHandlesChildEvents)) {
                     d_ptr->scene->sendEvent(child, event);
                  }
               }
            }
         }
         break;
      default:
         return false;
   }

   return true;
}

void QGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
   event->ignore();
}

void QGraphicsItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
   Q_D(QGraphicsItem);
   // binary compatibility workaround between 4.4 and 4.5
   if (d->isProxyWidget()) {
      static_cast<QGraphicsProxyWidget *>(this)->dragEnterEvent(event);
   }
}

void QGraphicsItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
   Q_D(QGraphicsItem);
   // binary compatibility workaround between 4.4 and 4.5
   if (d->isProxyWidget()) {
      static_cast<QGraphicsProxyWidget *>(this)->dragLeaveEvent(event);
   }
}

void QGraphicsItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
   Q_D(QGraphicsItem);
   // binary compatibility workaround between 4.4 and 4.5
   if (d->isProxyWidget()) {
      static_cast<QGraphicsProxyWidget *>(this)->dragMoveEvent(event);
   }
}

void QGraphicsItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
   Q_D(QGraphicsItem);
   // binary compatibility workaround between 4.4 and 4.5
   if (d->isProxyWidget()) {
      static_cast<QGraphicsProxyWidget *>(this)->dropEvent(event);
   }
}

void QGraphicsItem::focusInEvent(QFocusEvent *event)
{
   (void) event;
   update();
}

void QGraphicsItem::focusOutEvent(QFocusEvent *event)
{
   (void) event;
   update();
}

void QGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
   (void) event;
   update();
}

void QGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
   (void) event;
}

void QGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
   (void) event;
   update();
}

void QGraphicsItem::keyPressEvent(QKeyEvent *event)
{
   event->ignore();
}

void QGraphicsItem::keyReleaseEvent(QKeyEvent *event)
{
   event->ignore();
}

void QGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   if (event->button() == Qt::LeftButton && (flags() & ItemIsSelectable)) {
      bool multiSelect = (event->modifiers() & Qt::ControlModifier) != 0;
      if (!multiSelect) {
         if (!d_ptr->selected) {
            if (QGraphicsScene *scene = d_ptr->scene) {
               ++scene->d_func()->selectionChanging;
               scene->clearSelection();
               --scene->d_func()->selectionChanging;
            }
            setSelected(true);
         }
      }
   } else if (!(flags() & ItemIsMovable)) {
      event->ignore();
   }
   if (d_ptr->isWidget) {
      // Qt::Popup closes when you click outside.
      QGraphicsWidget *w = static_cast<QGraphicsWidget *>(this);
      if ((w->windowFlags() & Qt::Popup) == Qt::Popup) {
         event->accept();
         if (!w->rect().contains(event->pos())) {
            w->close();
         }
      }
   }
}

// obsolete
bool _qt_movableAncestorIsSelected(const QGraphicsItem *item)
{
   const QGraphicsItem *parent = item->parentItem();
   return parent && (((parent->flags() & QGraphicsItem::ItemIsMovable) && parent->isSelected()) ||
         _qt_movableAncestorIsSelected(parent));
}

bool QGraphicsItemPrivate::movableAncestorIsSelected(const QGraphicsItem *item)
{
   const QGraphicsItem *parent = item->d_ptr->parent;
   return parent && (((parent->flags() & QGraphicsItem::ItemIsMovable) && parent->isSelected()) ||
         _qt_movableAncestorIsSelected(parent));
}

void QGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
   if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
      // Determine the list of items that need to be moved.
      QList<QGraphicsItem *> selectedItems;
      QHash<QGraphicsItem *, QPointF> initialPositions;

      if (d_ptr->scene) {
         selectedItems    = d_ptr->scene->selectedItems();
         initialPositions = d_ptr->scene->d_func()->movingItemsInitialPositions;

         if (initialPositions.isEmpty()) {
            for (QGraphicsItem *item : selectedItems) {
               initialPositions[item] = item->pos();
            }

            initialPositions[this] = pos();
         }

         d_ptr->scene->d_func()->movingItemsInitialPositions = initialPositions;
      }

      // Find the active view.
      QGraphicsView *view = nullptr;
      if (event->widget()) {
         view = qobject_cast<QGraphicsView *>(event->widget()->parentWidget());
      }

      // Move all selected items
      int i = 0;
      bool movedMe = false;
      while (i <= selectedItems.size()) {
         QGraphicsItem *item = nullptr;
         if (i < selectedItems.size()) {
            item = selectedItems.at(i);
         } else {
            item = this;
         }
         if (item == this) {
            // Slightly clumsy-looking way to ensure that "this" is part
            // of the list of items to move, this is to avoid allocations
            // (appending this item to the list of selected items causes a
            // detach).
            if (movedMe) {
               break;
            }
            movedMe = true;
         }

         if ((item->flags() & ItemIsMovable) && !QGraphicsItemPrivate::movableAncestorIsSelected(item)) {
            QPointF currentParentPos;
            QPointF buttonDownParentPos;
            if (item->d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorIgnoresTransformations) {
               // Items whose ancestors ignore transformations need to
               // map screen coordinates to local coordinates, then map
               // those to the parent.
               QTransform viewToItemTransform = (item->deviceTransform(view->viewportTransform())).inverted();
               currentParentPos = mapToParent(viewToItemTransform.map(QPointF(view->mapFromGlobal(event->screenPos()))));
               buttonDownParentPos = mapToParent(viewToItemTransform.map(QPointF(view->mapFromGlobal(event->buttonDownScreenPos(
                                 Qt::LeftButton)))));
            } else if (item->flags() & ItemIgnoresTransformations) {
               // Root items that ignore transformations need to
               // calculate their diff by mapping viewport coordinates
               // directly to parent coordinates.
               // COMBINE
               QTransform itemTransform;
               if (item->d_ptr->transformData) {
                  itemTransform = item->d_ptr->transformData->computedFullTransform();
               }
               itemTransform.translate(item->d_ptr->pos.x(), item->d_ptr->pos.y());
               QTransform viewToParentTransform = itemTransform
                  * (item->sceneTransform() * view->viewportTransform()).inverted();
               currentParentPos = viewToParentTransform.map(QPointF(view->mapFromGlobal(event->screenPos())));
               buttonDownParentPos = viewToParentTransform.map(QPointF(view->mapFromGlobal(event->buttonDownScreenPos(
                              Qt::LeftButton))));
            } else {
               // All other items simply map from the scene.
               currentParentPos = item->mapToParent(item->mapFromScene(event->scenePos()));
               buttonDownParentPos = item->mapToParent(item->mapFromScene(event->buttonDownScenePos(Qt::LeftButton)));
            }

            item->setPos(initialPositions.value(item) + currentParentPos - buttonDownParentPos);

            if (item->flags() & ItemIsSelectable) {
               item->setSelected(true);
            }
         }
         ++i;
      }

   } else {
      event->ignore();
   }
}

void QGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   if (event->button() == Qt::LeftButton && (flags() & ItemIsSelectable)) {
      bool multiSelect = (event->modifiers() & Qt::ControlModifier) != 0;
      if (event->scenePos() == event->buttonDownScenePos(Qt::LeftButton)) {
         // The item didn't move
         if (multiSelect) {
            setSelected(!isSelected());
         } else {
            bool selectionChanged = false;
            if (QGraphicsScene *scene = d_ptr->scene) {
               ++scene->d_func()->selectionChanging;
               // Clear everything but this item. Bypass
               // QGraphicsScene::clearSelection()'s default behavior by
               // temporarily removing this item from the selection list.

               if (d_ptr->selected) {
                  scene->d_func()->selectedItems.remove(this);
                  for (QGraphicsItem *item : scene->d_func()->selectedItems) {
                     if (item->isSelected()) {
                        selectionChanged = true;
                        break;
                     }
                  }
               }
               scene->clearSelection();
               if (d_ptr->selected) {
                  scene->d_func()->selectedItems.insert(this);
               }
               --scene->d_func()->selectionChanging;
               if (selectionChanged) {
                  emit d_ptr->scene->selectionChanged();
               }
            }
            setSelected(true);
         }
      }
   }
   if (d_ptr->scene && !event->buttons()) {
      d_ptr->scene->d_func()->movingItemsInitialPositions.clear();
   }
}

void QGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
   mousePressEvent(event);
}

void QGraphicsItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
   event->ignore();
}

void QGraphicsItem::inputMethodEvent(QInputMethodEvent *event)
{
   event->ignore();
}

QVariant QGraphicsItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
   (void) query;
   return QVariant();
}

Qt::InputMethodHints QGraphicsItem::inputMethodHints() const
{
   Q_D(const QGraphicsItem);
   return d->imHints;
}

void QGraphicsItem::setInputMethodHints(Qt::InputMethodHints hints)
{
   Q_D(QGraphicsItem);

   d->imHints = hints;
   if (!hasFocus()) {
      return;
   }

   d->scene->d_func()->updateInputMethodSensitivityInViews();

   QWidget *fw = QApplication::focusWidget();

   if (!fw) {
      return;
   }

   QGuiApplication::inputMethod()->update(Qt::ImHints);
}

void QGraphicsItem::updateMicroFocus()
{

}

QVariant QGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
   (void) change;
   return value;
}

// internal
bool QGraphicsItem::supportsExtension(Extension extension) const
{
   (void) extension;
   return false;
}

// internal
void QGraphicsItem::setExtension(Extension extension, const QVariant &variant)
{
   (void) extension;
   (void) variant;
}

// internal
QVariant QGraphicsItem::extension(const QVariant &variant) const
{
   (void) variant;
   return QVariant();
}

// internal
void QGraphicsItem::addToIndex()
{
   if (d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren
      || d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorContainsChildren) {
      // ### add to child index only if applicable
      return;
   }

   if (d_ptr->scene) {
      d_ptr->scene->d_func()->index->addItem(this);
   }
}

// internal
void QGraphicsItem::removeFromIndex()
{
   if (d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorClipsChildren
      || d_ptr->ancestorFlags & QGraphicsItemPrivate::AncestorContainsChildren) {
      // ### remove from child index only if applicable
      return;
   }
   if (d_ptr->scene) {
      d_ptr->scene->d_func()->index->removeItem(this);
   }
}

void QGraphicsItem::prepareGeometryChange()
{
   if (d_ptr->inDestructor) {
      return;
   }

   if (d_ptr->scene) {
      d_ptr->scene->d_func()->dirtyGrowingItemsBoundingRect = true;
      d_ptr->geometryChanged = 1;
      d_ptr->paintedViewBoundingRectsNeedRepaint = 1;
      d_ptr->notifyBoundingRectChanged = !d_ptr->inSetPosHelper;

      QGraphicsScenePrivate *scenePrivate = d_ptr->scene->d_func();

      scenePrivate->index->prepareBoundingRectChange(this);
      scenePrivate->markDirty(this, QRectF(), true, false, false, false, true);

      // For compatibility reasons, we have to update the item's old geometry
      // if someone is connected to the changed signal or the scene has no views.
      // This has to be done *after* markDirty to ensure that _q_processDirtyItems is called before _q_emitUpdated.

      if (d_ptr->scene->isSignalConnected(scenePrivate->changedSignalMethod) || scenePrivate->views.isEmpty()) {

         if (d_ptr->hasTranslateOnlySceneTransform()) {
            d_ptr->scene->update(boundingRect().translated(d_ptr->sceneTransform.dx(),
                  d_ptr->sceneTransform.dy()));

         } else {
            d_ptr->scene->update(d_ptr->sceneTransform.mapRect(boundingRect()));
         }
      }
   }

   d_ptr->markParentDirty(true);
}

// internal
// This function is a duplicate of qt_graphicsItem_highlightSelected() in qgraphicssvgitem.cpp
static void qt_graphicsItem_highlightSelected(
   QGraphicsItem *item, QPainter *painter, const QStyleOptionGraphicsItem *option)
{
   const QRectF murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));
   if (qFuzzyIsNull(qMax(murect.width(), murect.height()))) {
      return;
   }

   const QRectF mbrect = painter->transform().mapRect(item->boundingRect());
   if (qMin(mbrect.width(), mbrect.height()) < qreal(1.0)) {
      return;
   }

   qreal itemPenWidth;
   switch (item->type()) {
      case QGraphicsEllipseItem::Type:
         itemPenWidth = static_cast<QGraphicsEllipseItem *>(item)->pen().widthF();
         break;
      case QGraphicsPathItem::Type:
         itemPenWidth = static_cast<QGraphicsPathItem *>(item)->pen().widthF();
         break;
      case QGraphicsPolygonItem::Type:
         itemPenWidth = static_cast<QGraphicsPolygonItem *>(item)->pen().widthF();
         break;
      case QGraphicsRectItem::Type:
         itemPenWidth = static_cast<QGraphicsRectItem *>(item)->pen().widthF();
         break;
      case QGraphicsSimpleTextItem::Type:
         itemPenWidth = static_cast<QGraphicsSimpleTextItem *>(item)->pen().widthF();
         break;
      case QGraphicsLineItem::Type:
         itemPenWidth = static_cast<QGraphicsLineItem *>(item)->pen().widthF();
         break;
      default:
         itemPenWidth = 1.0;
   }
   const qreal pad = itemPenWidth / 2;

   const qreal penWidth = 0; // cosmetic pen

   const QColor fgcolor = option->palette.windowText().color();
   const QColor bgcolor( // ensure good contrast against fgcolor
      fgcolor.red()   > 127 ? 0 : 255,
      fgcolor.green() > 127 ? 0 : 255,
      fgcolor.blue()  > 127 ? 0 : 255);

   painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
   painter->setBrush(Qt::NoBrush);
   painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));

   painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
   painter->setBrush(Qt::NoBrush);
   painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));
}


QGraphicsObject::QGraphicsObject(QGraphicsItem *parent)
   : QGraphicsItem(parent)
{
   QGraphicsItem::d_ptr->isObject = true;
}

// internal
QGraphicsObject::QGraphicsObject(QGraphicsItemPrivate &dd, QGraphicsItem *parent)
   : QGraphicsItem(dd, parent)
{
   QGraphicsItem::d_ptr->isObject = true;
}

QGraphicsObject::~QGraphicsObject()
{
}

bool QGraphicsObject::event(QEvent *ev)
{
   if (ev->type() == QEvent::StyleAnimationUpdate) {
      if (isVisible()) {
         ev->accept();
         update();
      }
      return true;
   }
   return QObject::event(ev);
}

#ifndef QT_NO_GESTURES

void QGraphicsObject::grabGesture(Qt::GestureType gesture, Qt::GestureFlags flags)
{
   bool contains = QGraphicsItem::d_ptr->gestureContext.contains(gesture);
   QGraphicsItem::d_ptr->gestureContext.insert(gesture, flags);
   if (!contains && QGraphicsItem::d_ptr->scene) {
      QGraphicsItem::d_ptr->scene->d_func()->grabGesture(this, gesture);
   }
}

void QGraphicsObject::ungrabGesture(Qt::GestureType gesture)
{
   if (QGraphicsItem::d_ptr->gestureContext.remove(gesture) && QGraphicsItem::d_ptr->scene) {
      QGraphicsItem::d_ptr->scene->d_func()->ungrabGesture(this, gesture);
   }
}
#endif

void QGraphicsObject::updateMicroFocus()
{
   QGraphicsItem::updateMicroFocus();
}

void QGraphicsItemPrivate::children_append(QDeclarativeListProperty<QGraphicsObject> *list, QGraphicsObject *item)
{
   if (item) {
      QGraphicsObject *graphicsObject = static_cast<QGraphicsObject *>(list->object);

      if (QGraphicsItemPrivate::get(graphicsObject)->sendParentChangeNotification) {
         item->setParentItem(graphicsObject);

      } else {
         QGraphicsItemPrivate::get(item)->setParentItemHelper(graphicsObject, nullptr, nullptr);
      }
   }
}

int QGraphicsItemPrivate::children_count(QDeclarativeListProperty<QGraphicsObject> *list)
{
   QGraphicsItemPrivate *d = QGraphicsItemPrivate::get(static_cast<QGraphicsObject *>(list->object));
   return d->children.count();
}

QGraphicsObject *QGraphicsItemPrivate::children_at(QDeclarativeListProperty<QGraphicsObject> *list, int index)
{
   QGraphicsItemPrivate *d = QGraphicsItemPrivate::get(static_cast<QGraphicsObject *>(list->object));

   if (index >= 0 && index < d->children.count())  {
      return d->children.at(index)->toGraphicsObject();
   } else  {
      return nullptr;
   }
}

void QGraphicsItemPrivate::children_clear(QDeclarativeListProperty<QGraphicsObject> *list)
{
   QGraphicsItemPrivate *d = QGraphicsItemPrivate::get(static_cast<QGraphicsObject *>(list->object));

   int childCount = d->children.count();
   if (d->sendParentChangeNotification) {
      for (int index = 0; index < childCount; index++) {
         d->children.at(0)->setParentItem(nullptr);
      }
   } else {
      for (int index = 0; index < childCount; index++) {
         QGraphicsItemPrivate::get(d->children.at(0))->setParentItemHelper(nullptr, nullptr, nullptr);
      }
   }
}

QDeclarativeListProperty<QGraphicsObject> QGraphicsItemPrivate::childrenList()
{
   Q_Q(QGraphicsItem);

   if (isObject) {
      QGraphicsObject *that = static_cast<QGraphicsObject *>(q);

      return QDeclarativeListProperty<QGraphicsObject>(that, &children, children_append,
            children_count, children_at, children_clear);

   } else {
      // QGraphicsItem is not supported for this property
      return QDeclarativeListProperty<QGraphicsObject>();
   }
}


qreal QGraphicsItemPrivate::width() const
{
   return 0;
}

void QGraphicsItemPrivate::setWidth(qreal w)
{
   (void) w;
}

void QGraphicsItemPrivate::resetWidth()
{
}

qreal QGraphicsItemPrivate::height() const
{
   return 0;
}

void QGraphicsItemPrivate::setHeight(qreal h)
{
   (void) h;
}

void QGraphicsItemPrivate::resetHeight()
{
}

class QAbstractGraphicsShapeItemPrivate : public QGraphicsItemPrivate
{
   Q_DECLARE_PUBLIC(QAbstractGraphicsShapeItem)
 public:

   QBrush brush;
   QPen pen;

   // Cached bounding rectangle
   mutable QRectF boundingRect;
};

QAbstractGraphicsShapeItem::QAbstractGraphicsShapeItem(QGraphicsItem *parent)
   : QGraphicsItem(*new QAbstractGraphicsShapeItemPrivate, parent)
{
}

// internal
QAbstractGraphicsShapeItem::QAbstractGraphicsShapeItem(QAbstractGraphicsShapeItemPrivate &dd, QGraphicsItem *parent)
   : QGraphicsItem(dd, parent)
{
}

QAbstractGraphicsShapeItem::~QAbstractGraphicsShapeItem()
{
}

QPen QAbstractGraphicsShapeItem::pen() const
{
   Q_D(const QAbstractGraphicsShapeItem);
   return d->pen;
}

void QAbstractGraphicsShapeItem::setPen(const QPen &pen)
{
   Q_D(QAbstractGraphicsShapeItem);
   if (d->pen == pen) {
      return;
   }
   prepareGeometryChange();
   d->pen = pen;
   d->boundingRect = QRectF();
   update();
}

QBrush QAbstractGraphicsShapeItem::brush() const
{
   Q_D(const QAbstractGraphicsShapeItem);
   return d->brush;
}

void QAbstractGraphicsShapeItem::setBrush(const QBrush &brush)
{
   Q_D(QAbstractGraphicsShapeItem);
   if (d->brush == brush) {
      return;
   }
   d->brush = brush;
   update();
}

bool QAbstractGraphicsShapeItem::isObscuredBy(const QGraphicsItem *item) const
{
   return QGraphicsItem::isObscuredBy(item);
}

QPainterPath QAbstractGraphicsShapeItem::opaqueArea() const
{
   Q_D(const QAbstractGraphicsShapeItem);
   if (d->brush.isOpaque()) {
      return isClipped() ? clipPath() : shape();
   }
   return QGraphicsItem::opaqueArea();
}

class QGraphicsPathItemPrivate : public QAbstractGraphicsShapeItemPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsPathItem)
 public:
   QPainterPath path;
};

QGraphicsPathItem::QGraphicsPathItem(const QPainterPath &path, QGraphicsItem *parent)
   : QAbstractGraphicsShapeItem(*new QGraphicsPathItemPrivate, parent)
{
   if (!path.isEmpty()) {
      setPath(path);
   }
}

QGraphicsPathItem::QGraphicsPathItem(QGraphicsItem *parent)
   : QAbstractGraphicsShapeItem(*new QGraphicsPathItemPrivate, parent)
{
}

QGraphicsPathItem::~QGraphicsPathItem()
{
}

QPainterPath QGraphicsPathItem::path() const
{
   Q_D(const QGraphicsPathItem);
   return d->path;
}

void QGraphicsPathItem::setPath(const QPainterPath &path)
{
   Q_D(QGraphicsPathItem);
   if (d->path == path) {
      return;
   }
   prepareGeometryChange();
   d->path = path;
   d->boundingRect = QRectF();
   update();
}

QRectF QGraphicsPathItem::boundingRect() const
{
   Q_D(const QGraphicsPathItem);
   if (d->boundingRect.isNull()) {
      qreal pw = pen().style() == Qt::NoPen ? qreal(0) : pen().widthF();
      if (pw == 0.0) {
         d->boundingRect = d->path.controlPointRect();
      } else {
         d->boundingRect = shape().controlPointRect();
      }
   }
   return d->boundingRect;
}

QPainterPath QGraphicsPathItem::shape() const
{
   Q_D(const QGraphicsPathItem);
   return qt_graphicsItem_shapeFromPath(d->path, d->pen);
}

bool QGraphicsPathItem::contains(const QPointF &point) const
{
   return QAbstractGraphicsShapeItem::contains(point);
}

void QGraphicsPathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   (void) widget;

   Q_D(QGraphicsPathItem);

   painter->setPen(d->pen);
   painter->setBrush(d->brush);
   painter->drawPath(d->path);

   if (option->state & QStyle::State_Selected) {
      qt_graphicsItem_highlightSelected(this, painter, option);
   }
}

bool QGraphicsPathItem::isObscuredBy(const QGraphicsItem *item) const
{
   return QAbstractGraphicsShapeItem::isObscuredBy(item);
}

QPainterPath QGraphicsPathItem::opaqueArea() const
{
   return QAbstractGraphicsShapeItem::opaqueArea();
}

int QGraphicsPathItem::type() const
{
   return Type;
}

// internal
bool QGraphicsPathItem::supportsExtension(Extension extension) const
{
   (void) extension;
   return false;
}

// internal
void QGraphicsPathItem::setExtension(Extension extension, const QVariant &variant)
{
   (void) extension;
   (void) variant;
}

// internal
QVariant QGraphicsPathItem::extension(const QVariant &variant) const
{
   (void) variant;
   return QVariant();
}

class QGraphicsRectItemPrivate : public QAbstractGraphicsShapeItemPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsRectItem)

 public:
   QRectF rect;
};

QGraphicsRectItem::QGraphicsRectItem(const QRectF &rect, QGraphicsItem *parent)
   : QAbstractGraphicsShapeItem(*new QGraphicsRectItemPrivate, parent)
{
   setRect(rect);
}

QGraphicsRectItem::QGraphicsRectItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent)
   : QAbstractGraphicsShapeItem(*new QGraphicsRectItemPrivate, parent)
{
   setRect(QRectF(x, y, w, h));
}

QGraphicsRectItem::QGraphicsRectItem(QGraphicsItem *parent)
   : QAbstractGraphicsShapeItem(*new QGraphicsRectItemPrivate, parent)
{
}

QGraphicsRectItem::~QGraphicsRectItem()
{
}

QRectF QGraphicsRectItem::rect() const
{
   Q_D(const QGraphicsRectItem);
   return d->rect;
}

void QGraphicsRectItem::setRect(const QRectF &rect)
{
   Q_D(QGraphicsRectItem);
   if (d->rect == rect) {
      return;
   }
   prepareGeometryChange();
   d->rect = rect;
   d->boundingRect = QRectF();
   update();
}

QRectF QGraphicsRectItem::boundingRect() const
{
   Q_D(const QGraphicsRectItem);
   if (d->boundingRect.isNull()) {
      qreal halfpw = pen().style() == Qt::NoPen ? qreal(0) : pen().widthF() / 2;
      d->boundingRect = d->rect;

      if (halfpw > 0.0) {
         d->boundingRect.adjust(-halfpw, -halfpw, halfpw, halfpw);
      }
   }
   return d->boundingRect;
}

QPainterPath QGraphicsRectItem::shape() const
{
   Q_D(const QGraphicsRectItem);
   QPainterPath path;
   path.addRect(d->rect);
   return qt_graphicsItem_shapeFromPath(path, d->pen);
}

bool QGraphicsRectItem::contains(const QPointF &point) const
{
   return QAbstractGraphicsShapeItem::contains(point);
}

void QGraphicsRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   (void) widget;

   Q_D(QGraphicsRectItem);

   painter->setPen(d->pen);
   painter->setBrush(d->brush);
   painter->drawRect(d->rect);

   if (option->state & QStyle::State_Selected) {
      qt_graphicsItem_highlightSelected(this, painter, option);
   }
}

bool QGraphicsRectItem::isObscuredBy(const QGraphicsItem *item) const
{
   return QAbstractGraphicsShapeItem::isObscuredBy(item);
}

QPainterPath QGraphicsRectItem::opaqueArea() const
{
   return QAbstractGraphicsShapeItem::opaqueArea();
}


int QGraphicsRectItem::type() const
{
   return Type;
}

bool QGraphicsRectItem::supportsExtension(Extension extension) const
{
   (void) extension;
   return false;
}

// internal
void QGraphicsRectItem::setExtension(Extension extension, const QVariant &variant)
{
   (void) extension;
   (void) variant;
}

QVariant QGraphicsRectItem::extension(const QVariant &variant) const
{
   (void) variant;
   return QVariant();
}

class QGraphicsEllipseItemPrivate : public QAbstractGraphicsShapeItemPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsEllipseItem)

 public:
   inline QGraphicsEllipseItemPrivate()
      : startAngle(0), spanAngle(360 * 16) {
   }

   QRectF rect;
   int startAngle;
   int spanAngle;
};

QGraphicsEllipseItem::QGraphicsEllipseItem(const QRectF &rect, QGraphicsItem *parent)
   : QAbstractGraphicsShapeItem(*new QGraphicsEllipseItemPrivate, parent)
{
   setRect(rect);
}

QGraphicsEllipseItem::QGraphicsEllipseItem(qreal x, qreal y, qreal w, qreal h,
   QGraphicsItem *parent)
   : QAbstractGraphicsShapeItem(*new QGraphicsEllipseItemPrivate, parent)
{
   setRect(x, y, w, h);
}

QGraphicsEllipseItem::QGraphicsEllipseItem(QGraphicsItem *parent)
   : QAbstractGraphicsShapeItem(*new QGraphicsEllipseItemPrivate, parent)
{
}

QGraphicsEllipseItem::~QGraphicsEllipseItem()
{
}

QRectF QGraphicsEllipseItem::rect() const
{
   Q_D(const QGraphicsEllipseItem);
   return d->rect;
}

void QGraphicsEllipseItem::setRect(const QRectF &rect)
{
   Q_D(QGraphicsEllipseItem);
   if (d->rect == rect) {
      return;
   }
   prepareGeometryChange();
   d->rect = rect;
   d->boundingRect = QRectF();
   update();
}

int QGraphicsEllipseItem::startAngle() const
{
   Q_D(const QGraphicsEllipseItem);
   return d->startAngle;
}

void QGraphicsEllipseItem::setStartAngle(int angle)
{
   Q_D(QGraphicsEllipseItem);
   if (angle != d->startAngle) {
      prepareGeometryChange();
      d->boundingRect = QRectF();
      d->startAngle = angle;
      update();
   }
}

int QGraphicsEllipseItem::spanAngle() const
{
   Q_D(const QGraphicsEllipseItem);
   return d->spanAngle;
}

void QGraphicsEllipseItem::setSpanAngle(int angle)
{
   Q_D(QGraphicsEllipseItem);
   if (angle != d->spanAngle) {
      prepareGeometryChange();
      d->boundingRect = QRectF();
      d->spanAngle = angle;
      update();
   }
}

QRectF QGraphicsEllipseItem::boundingRect() const
{
   Q_D(const QGraphicsEllipseItem);
   if (d->boundingRect.isNull()) {
      qreal pw = pen().style() == Qt::NoPen ? qreal(0) : pen().widthF();

      if (pw == 0.0 && d->spanAngle == 360 * 16) {
         d->boundingRect = d->rect;
      } else {
         d->boundingRect = shape().controlPointRect();
      }
   }
   return d->boundingRect;
}

QPainterPath QGraphicsEllipseItem::shape() const
{
   Q_D(const QGraphicsEllipseItem);
   QPainterPath path;
   if (d->rect.isNull()) {
      return path;
   }
   if (d->spanAngle != 360 * 16) {
      path.moveTo(d->rect.center());
      path.arcTo(d->rect, d->startAngle / 16.0, d->spanAngle / 16.0);

   } else {
      path.addEllipse(d->rect);
   }

   return qt_graphicsItem_shapeFromPath(path, d->pen);
}

bool QGraphicsEllipseItem::contains(const QPointF &point) const
{
   return QAbstractGraphicsShapeItem::contains(point);
}

void QGraphicsEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   (void) widget;

   Q_D(QGraphicsEllipseItem);

   painter->setPen(d->pen);
   painter->setBrush(d->brush);

   if ((d->spanAngle != 0) && (qAbs(d->spanAngle) % (360 * 16) == 0)) {
      painter->drawEllipse(d->rect);
   } else {
      painter->drawPie(d->rect, d->startAngle, d->spanAngle);
   }

   if (option->state & QStyle::State_Selected) {
      qt_graphicsItem_highlightSelected(this, painter, option);
   }
}

bool QGraphicsEllipseItem::isObscuredBy(const QGraphicsItem *item) const
{
   return QAbstractGraphicsShapeItem::isObscuredBy(item);
}

QPainterPath QGraphicsEllipseItem::opaqueArea() const
{
   return QAbstractGraphicsShapeItem::opaqueArea();
}

int QGraphicsEllipseItem::type() const
{
   return Type;
}

bool QGraphicsEllipseItem::supportsExtension(Extension extension) const
{
   (void) extension;
   return false;
}

void QGraphicsEllipseItem::setExtension(Extension extension, const QVariant &variant)
{
   (void) extension;
   (void) variant;
}

// internal
QVariant QGraphicsEllipseItem::extension(const QVariant &variant) const
{
   (void) variant;
   return QVariant();
}

class QGraphicsPolygonItemPrivate : public QAbstractGraphicsShapeItemPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsPolygonItem)

 public:
   inline QGraphicsPolygonItemPrivate()
      : fillRule(Qt::OddEvenFill) {
   }

   QPolygonF polygon;
   Qt::FillRule fillRule;
};

QGraphicsPolygonItem::QGraphicsPolygonItem(const QPolygonF &polygon, QGraphicsItem *parent)
   : QAbstractGraphicsShapeItem(*new QGraphicsPolygonItemPrivate, parent)
{
   setPolygon(polygon);
}

QGraphicsPolygonItem::QGraphicsPolygonItem(QGraphicsItem *parent)
   : QAbstractGraphicsShapeItem(*new QGraphicsPolygonItemPrivate, parent)
{
}

QGraphicsPolygonItem::~QGraphicsPolygonItem()
{
}

QPolygonF QGraphicsPolygonItem::polygon() const
{
   Q_D(const QGraphicsPolygonItem);
   return d->polygon;
}

void QGraphicsPolygonItem::setPolygon(const QPolygonF &polygon)
{
   Q_D(QGraphicsPolygonItem);

   if (d->polygon == polygon) {
      return;
   }

   prepareGeometryChange();
   d->polygon = polygon;
   d->boundingRect = QRectF();
   update();
}

Qt::FillRule QGraphicsPolygonItem::fillRule() const
{
   Q_D(const QGraphicsPolygonItem);
   return d->fillRule;
}

void QGraphicsPolygonItem::setFillRule(Qt::FillRule rule)
{
   Q_D(QGraphicsPolygonItem);
   if (rule != d->fillRule) {
      d->fillRule = rule;
      update();
   }
}

QRectF QGraphicsPolygonItem::boundingRect() const
{
   Q_D(const QGraphicsPolygonItem);
   if (d->boundingRect.isNull()) {
      qreal pw = pen().style() == Qt::NoPen ? qreal(0) : pen().widthF();
      if (pw == 0.0) {
         d->boundingRect = d->polygon.boundingRect();
      } else {
         d->boundingRect = shape().controlPointRect();
      }
   }
   return d->boundingRect;
}

QPainterPath QGraphicsPolygonItem::shape() const
{
   Q_D(const QGraphicsPolygonItem);
   QPainterPath path;
   path.addPolygon(d->polygon);
   return qt_graphicsItem_shapeFromPath(path, d->pen);
}

bool QGraphicsPolygonItem::contains(const QPointF &point) const
{
   return QAbstractGraphicsShapeItem::contains(point);
}

void QGraphicsPolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   (void) widget;

   Q_D(QGraphicsPolygonItem);

   painter->setPen(d->pen);
   painter->setBrush(d->brush);
   painter->drawPolygon(d->polygon, d->fillRule);

   if (option->state & QStyle::State_Selected) {
      qt_graphicsItem_highlightSelected(this, painter, option);
   }
}

bool QGraphicsPolygonItem::isObscuredBy(const QGraphicsItem *item) const
{
   return QAbstractGraphicsShapeItem::isObscuredBy(item);
}

QPainterPath QGraphicsPolygonItem::opaqueArea() const
{
   return QAbstractGraphicsShapeItem::opaqueArea();
}

int QGraphicsPolygonItem::type() const
{
   return Type;
}

// internal
bool QGraphicsPolygonItem::supportsExtension(Extension extension) const
{
   (void) extension;
   return false;
}

// internal
void QGraphicsPolygonItem::setExtension(Extension extension, const QVariant &variant)
{
   (void) extension;
   (void) variant;
}

// internal
QVariant QGraphicsPolygonItem::extension(const QVariant &variant) const
{
   (void) variant;
   return QVariant();
}

class QGraphicsLineItemPrivate : public QGraphicsItemPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsLineItem)

 public:
   QLineF line;
   QPen pen;
};

QGraphicsLineItem::QGraphicsLineItem(const QLineF &line, QGraphicsItem *parent)
   : QGraphicsItem(*new QGraphicsLineItemPrivate, parent)
{
   setLine(line);
}

QGraphicsLineItem::QGraphicsLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent)
   : QGraphicsItem(*new QGraphicsLineItemPrivate, parent)
{
   setLine(x1, y1, x2, y2);
}

QGraphicsLineItem::QGraphicsLineItem(QGraphicsItem *parent)
   : QGraphicsItem(*new QGraphicsLineItemPrivate, parent)
{
}

QGraphicsLineItem::~QGraphicsLineItem()
{
}

QPen QGraphicsLineItem::pen() const
{
   Q_D(const QGraphicsLineItem);
   return d->pen;
}

void QGraphicsLineItem::setPen(const QPen &pen)
{
   Q_D(QGraphicsLineItem);

   if (d->pen == pen) {
      return;
   }

   prepareGeometryChange();
   d->pen = pen;
   update();
}

QLineF QGraphicsLineItem::line() const
{
   Q_D(const QGraphicsLineItem);
   return d->line;
}

void QGraphicsLineItem::setLine(const QLineF &line)
{
   Q_D(QGraphicsLineItem);

   if (d->line == line) {
      return;
   }

   prepareGeometryChange();
   d->line = line;
   update();
}

QRectF QGraphicsLineItem::boundingRect() const
{
   Q_D(const QGraphicsLineItem);

   if (d->pen.widthF() == 0.0) {
      const qreal x1 = d->line.p1().x();
      const qreal x2 = d->line.p2().x();
      const qreal y1 = d->line.p1().y();
      const qreal y2 = d->line.p2().y();
      qreal lx = qMin(x1, x2);
      qreal rx = qMax(x1, x2);
      qreal ty = qMin(y1, y2);
      qreal by = qMax(y1, y2);
      return QRectF(lx, ty, rx - lx, by - ty);
   }
   return shape().controlPointRect();
}

QPainterPath QGraphicsLineItem::shape() const
{
   Q_D(const QGraphicsLineItem);

   QPainterPath path;
   if (d->line == QLineF()) {
      return path;
   }

   path.moveTo(d->line.p1());
   path.lineTo(d->line.p2());
   return qt_graphicsItem_shapeFromPath(path, d->pen);
}

bool QGraphicsLineItem::contains(const QPointF &point) const
{
   return QGraphicsItem::contains(point);
}

void QGraphicsLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   (void) widget;

   Q_D(QGraphicsLineItem);

   painter->setPen(d->pen);
   painter->drawLine(d->line);

   if (option->state & QStyle::State_Selected) {
      qt_graphicsItem_highlightSelected(this, painter, option);
   }
}

bool QGraphicsLineItem::isObscuredBy(const QGraphicsItem *item) const
{
   return QGraphicsItem::isObscuredBy(item);
}

QPainterPath QGraphicsLineItem::opaqueArea() const
{
   return QGraphicsItem::opaqueArea();
}

int QGraphicsLineItem::type() const
{
   return Type;
}

// internal
bool QGraphicsLineItem::supportsExtension(Extension extension) const
{
   (void) extension;
   return false;
}

// internal
void QGraphicsLineItem::setExtension(Extension extension, const QVariant &variant)
{
   (void) extension;
   (void) variant;
}

// internal
QVariant QGraphicsLineItem::extension(const QVariant &variant) const
{
   (void) variant;
   return QVariant();
}

extern QPainterPath qt_regionToPath(const QRegion &region);

class QGraphicsPixmapItemPrivate : public QGraphicsItemPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsPixmapItem)

 public:
   QGraphicsPixmapItemPrivate()
      : transformationMode(Qt::FastTransformation),
        shapeMode(QGraphicsPixmapItem::MaskShape),
        hasShape(false) {
   }

   QPixmap pixmap;
   Qt::TransformationMode transformationMode;
   QPointF offset;
   QGraphicsPixmapItem::ShapeMode shapeMode;
   QPainterPath shape;
   bool hasShape;

   void updateShape() {
      shape = QPainterPath();

      switch (shapeMode) {
         case QGraphicsPixmapItem::MaskShape: {
            QBitmap mask = pixmap.mask();

            if (! mask.isNull()) {
               shape = qt_regionToPath(QRegion(mask).translated(offset.toPoint()));
               break;
            }
            [[fallthrough]];
         }

         case QGraphicsPixmapItem::BoundingRectShape:
            shape.addRect(QRectF(offset.x(), offset.y(), pixmap.width(), pixmap.height()));
            break;

         case QGraphicsPixmapItem::HeuristicMaskShape:
#ifndef QT_NO_IMAGE_HEURISTIC_MASK
            shape = qt_regionToPath(QRegion(pixmap.createHeuristicMask()).translated(offset.toPoint()));
#else
            shape.addRect(QRectF(offset.x(), offset.y(), pixmap.width(), pixmap.height()));
#endif
            break;
      }
   }
};

QGraphicsPixmapItem::QGraphicsPixmapItem(const QPixmap &pixmap, QGraphicsItem *parent)
   : QGraphicsItem(*new QGraphicsPixmapItemPrivate, parent)
{
   setPixmap(pixmap);
}

QGraphicsPixmapItem::QGraphicsPixmapItem(QGraphicsItem *parent)
   : QGraphicsItem(*new QGraphicsPixmapItemPrivate, parent)
{
}

QGraphicsPixmapItem::~QGraphicsPixmapItem()
{
}

void QGraphicsPixmapItem::setPixmap(const QPixmap &pixmap)
{
   Q_D(QGraphicsPixmapItem);
   prepareGeometryChange();
   d->pixmap = pixmap;
   d->hasShape = false;
   update();
}

QPixmap QGraphicsPixmapItem::pixmap() const
{
   Q_D(const QGraphicsPixmapItem);
   return d->pixmap;
}

Qt::TransformationMode QGraphicsPixmapItem::transformationMode() const
{
   Q_D(const QGraphicsPixmapItem);
   return d->transformationMode;
}

void QGraphicsPixmapItem::setTransformationMode(Qt::TransformationMode mode)
{
   Q_D(QGraphicsPixmapItem);

   if (mode != d->transformationMode) {
      d->transformationMode = mode;
      update();
   }
}

QPointF QGraphicsPixmapItem::offset() const
{
   Q_D(const QGraphicsPixmapItem);
   return d->offset;
}

void QGraphicsPixmapItem::setOffset(const QPointF &offset)
{
   Q_D(QGraphicsPixmapItem);

   if (d->offset == offset) {
      return;
   }

   prepareGeometryChange();
   d->offset = offset;
   d->hasShape = false;
   update();
}

// internal
QRectF QGraphicsPixmapItem::boundingRect() const
{
   Q_D(const QGraphicsPixmapItem);

   if (d->pixmap.isNull()) {
      return QRectF();
   }

   if (d->itemFlags & ItemIsSelectable) {
      qreal pw = 1.0;
      return QRectF(d->offset, d->pixmap.size() / d->pixmap.devicePixelRatio()).adjusted(-pw / 2, -pw / 2, pw / 2, pw / 2);

   } else {
      return QRectF(d->offset, d->pixmap.size() / d->pixmap.devicePixelRatio());
   }
}

// internal
QPainterPath QGraphicsPixmapItem::shape() const
{
   Q_D(const QGraphicsPixmapItem);
   if (!d->hasShape) {
      QGraphicsPixmapItemPrivate *thatD = const_cast<QGraphicsPixmapItemPrivate *>(d);
      thatD->updateShape();
      thatD->hasShape = true;
   }
   return d_func()->shape;
}

// internal
bool QGraphicsPixmapItem::contains(const QPointF &point) const
{
   return QGraphicsItem::contains(point);
}

// internal
void QGraphicsPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
   QWidget *widget)
{
   (void) widget;

   Q_D(QGraphicsPixmapItem);

   painter->setRenderHint(QPainter::SmoothPixmapTransform,
      (d->transformationMode == Qt::SmoothTransformation));

   painter->drawPixmap(d->offset, d->pixmap);

   if (option->state & QStyle::State_Selected) {
      qt_graphicsItem_highlightSelected(this, painter, option);
   }
}

bool QGraphicsPixmapItem::isObscuredBy(const QGraphicsItem *item) const
{
   return QGraphicsItem::isObscuredBy(item);
}

QPainterPath QGraphicsPixmapItem::opaqueArea() const
{
   return shape();
}

int QGraphicsPixmapItem::type() const
{
   return Type;
}

QGraphicsPixmapItem::ShapeMode QGraphicsPixmapItem::shapeMode() const
{
   return d_func()->shapeMode;
}

void QGraphicsPixmapItem::setShapeMode(ShapeMode mode)
{
   Q_D(QGraphicsPixmapItem);
   if (d->shapeMode == mode) {
      return;
   }
   d->shapeMode = mode;
   d->hasShape = false;
}

// internal
bool QGraphicsPixmapItem::supportsExtension(Extension extension) const
{
   (void) extension;
   return false;
}

// internal
void QGraphicsPixmapItem::setExtension(Extension extension, const QVariant &variant)
{
   (void) extension;
   (void) variant;
}

// internal
QVariant QGraphicsPixmapItem::extension(const QVariant &variant) const
{
   (void) variant;
   return QVariant();
}

class QGraphicsTextItemPrivate
{
 public:
   QGraphicsTextItemPrivate()
      : control(nullptr), pageNumber(0), useDefaultImpl(false), tabChangesFocus(false), clickCausedFocus(0) {
   }

   mutable QTextControl *control;
   QTextControl *textControl() const;

   inline QPointF controlOffset() const {
      return QPointF(0.0, pageNumber * control->document()->pageSize().height());
   }

   inline void sendControlEvent(QEvent *e) {
      if (control) {
         control->processEvent(e, controlOffset());
      }
   }

   void _q_updateBoundingRect(const QSizeF &);
   void _q_update(QRectF);
   void _q_ensureVisible(QRectF);
   bool _q_mouseOnEdge(QGraphicsSceneMouseEvent *);

   QRectF boundingRect;
   int pageNumber;
   bool useDefaultImpl;
   bool tabChangesFocus;

   uint clickCausedFocus : 1;

   QGraphicsTextItem *qq;
};

QGraphicsTextItem::QGraphicsTextItem(const QString &text, QGraphicsItem *parent)
   : QGraphicsObject(*new QGraphicsItemPrivate, parent), dd(new QGraphicsTextItemPrivate)
{
   dd->qq = this;

   if (! text.isEmpty()) {
      setPlainText(text);
   }

   setAcceptDrops(true);
   setAcceptHoverEvents(true);
   setFlags(ItemUsesExtendedStyleOption);
}

QGraphicsTextItem::QGraphicsTextItem(QGraphicsItem *parent)
   : QGraphicsObject(*new QGraphicsItemPrivate, parent), dd(new QGraphicsTextItemPrivate)
{
   dd->qq = this;

   setAcceptDrops(true);
   setAcceptHoverEvents(true);
   setFlag(ItemUsesExtendedStyleOption);
}

QGraphicsTextItem::~QGraphicsTextItem()
{
   delete dd;
}

QString QGraphicsTextItem::toHtml() const
{
#ifndef QT_NO_TEXTHTMLPARSER
   if (dd->control) {
      return dd->control->toHtml();
   }
#endif
   return QString();
}

void QGraphicsTextItem::setHtml(const QString &text)
{
   dd->textControl()->setHtml(text);
}

QString QGraphicsTextItem::toPlainText() const
{
   if (dd->control) {
      return dd->control->toPlainText();
   }
   return QString();
}

void QGraphicsTextItem::setPlainText(const QString &text)
{
   dd->textControl()->setPlainText(text);
}

QFont QGraphicsTextItem::font() const
{
   if (!dd->control) {
      return QFont();
   }
   return dd->control->document()->defaultFont();
}

void QGraphicsTextItem::setFont(const QFont &font)
{
   dd->textControl()->document()->setDefaultFont(font);
}

void QGraphicsTextItem::setDefaultTextColor(const QColor &col)
{
   QTextControl *c = dd->textControl();
   QPalette pal = c->palette();
   QColor old = pal.color(QPalette::Text);
   pal.setColor(QPalette::Text, col);
   c->setPalette(pal);
   if (old != col) {
      update();
   }
}

QColor QGraphicsTextItem::defaultTextColor() const
{
   return dd->textControl()->palette().color(QPalette::Text);
}

QRectF QGraphicsTextItem::boundingRect() const
{
   return dd->boundingRect;
}

QPainterPath QGraphicsTextItem::shape() const
{
   if (!dd->control) {
      return QPainterPath();
   }
   QPainterPath path;
   path.addRect(dd->boundingRect);
   return path;
}

bool QGraphicsTextItem::contains(const QPointF &point) const
{
   return dd->boundingRect.contains(point);
}

void QGraphicsTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
   QWidget *widget)
{
   (void) widget;

   if (dd->control) {
      painter->save();
      QRectF r = option->exposedRect;
      painter->translate(-dd->controlOffset());
      r.translate(dd->controlOffset());

      QTextDocument *doc = dd->control->document();
      QTextDocumentLayout *layout = qobject_cast<QTextDocumentLayout *>(doc->documentLayout());

      // the layout might need to expand the root frame to
      // the viewport if NoWrap is set
      if (layout) {
         layout->setViewport(dd->boundingRect);
      }

      dd->control->drawContents(painter, r);

      if (layout) {
         layout->setViewport(QRect());
      }

      painter->restore();
   }

   if (option->state & (QStyle::State_Selected | QStyle::State_HasFocus)) {
      qt_graphicsItem_highlightSelected(this, painter, option);
   }
}

bool QGraphicsTextItem::isObscuredBy(const QGraphicsItem *item) const
{
   return QGraphicsItem::isObscuredBy(item);
}

QPainterPath QGraphicsTextItem::opaqueArea() const
{
   return QGraphicsItem::opaqueArea();
}

int QGraphicsTextItem::type() const
{
   return Type;
}

void QGraphicsTextItem::setTextWidth(qreal width)
{
   dd->textControl()->setTextWidth(width);
}

qreal QGraphicsTextItem::textWidth() const
{
   if (!dd->control) {
      return -1;
   }
   return dd->control->textWidth();
}

void QGraphicsTextItem::adjustSize()
{
   if (dd->control) {
      dd->control->adjustSize();
   }
}

void QGraphicsTextItem::setDocument(QTextDocument *document)
{
   dd->textControl()->setDocument(document);
   dd->_q_updateBoundingRect(dd->control->size());
}

QTextDocument *QGraphicsTextItem::document() const
{
   return dd->textControl()->document();
}

bool QGraphicsTextItem::sceneEvent(QEvent *event)
{
   QEvent::Type t = event->type();
   if (! dd->tabChangesFocus && (t == QEvent::KeyPress || t == QEvent::KeyRelease)) {
      int k = ((QKeyEvent *)event)->key();

      if (k == Qt::Key_Tab || k == Qt::Key_Backtab) {
         dd->sendControlEvent(event);
         return true;
      }
   }
   bool result = QGraphicsItem::sceneEvent(event);

   // Ensure input context is updated.
   switch (event->type()) {
      case QEvent::ContextMenu:
      case QEvent::FocusIn:
      case QEvent::FocusOut:
      case QEvent::GraphicsSceneDragEnter:
      case QEvent::GraphicsSceneDragLeave:
      case QEvent::GraphicsSceneDragMove:
      case QEvent::GraphicsSceneDrop:
      case QEvent::GraphicsSceneHoverEnter:
      case QEvent::GraphicsSceneHoverLeave:
      case QEvent::GraphicsSceneHoverMove:
      case QEvent::GraphicsSceneMouseDoubleClick:
      case QEvent::GraphicsSceneMousePress:
      case QEvent::GraphicsSceneMouseMove:
      case QEvent::GraphicsSceneMouseRelease:
      case QEvent::KeyPress:
      case QEvent::KeyRelease:
         // Reset the focus widget's input context, regardless
         // of how this item gained or lost focus.

         if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) {
            QGuiApplication::inputMethod()->reset();
         } else {
            QGuiApplication::inputMethod()->update(Qt::ImQueryInput);
         }
         break;

      case QEvent::ShortcutOverride:
         dd->sendControlEvent(event);
         return true;

      default:
         break;
   }

   return result;
}

void QGraphicsTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   if ((QGraphicsItem::d_ptr->itemFlags & (ItemIsSelectable | ItemIsMovable))
                  && (event->buttons() & Qt::LeftButton) && dd->_q_mouseOnEdge(event)) {
      // User left-pressed on edge of selectable/movable item, use base impl.
      dd->useDefaultImpl = true;

   } else if (event->buttons() == event->button()
                  && dd->control->textInteractionFlags() == Qt::NoTextInteraction) {
      // user pressed first button on non-interactive item
      dd->useDefaultImpl = true;
   }

   if (dd->useDefaultImpl) {
      QGraphicsItem::mousePressEvent(event);

      if (! event->isAccepted()) {
         dd->useDefaultImpl = false;
      }

      return;
   }

   dd->sendControlEvent(event);
}

void QGraphicsTextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
   if (dd->useDefaultImpl) {
      QGraphicsItem::mouseMoveEvent(event);
      return;
   }

   dd->sendControlEvent(event);
}

void QGraphicsTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   if (dd->useDefaultImpl) {
      QGraphicsItem::mouseReleaseEvent(event);
      if (dd->control->textInteractionFlags() == Qt::NoTextInteraction
         && !event->buttons()) {
         // User released last button on non-interactive item.
         dd->useDefaultImpl = false;
      } else  if ((event->buttons() & Qt::LeftButton) == 0) {
         // User released the left button on an interactive item.
         dd->useDefaultImpl = false;
      }
      return;
   }

   QWidget *widget = event->widget();
   if (widget && (dd->control->textInteractionFlags() & Qt::TextEditable) && boundingRect().contains(event->pos())) {
      qt_widget_private(widget)->handleSoftwareInputPanel(event->button(), dd->clickCausedFocus);
   }
   dd->clickCausedFocus = 0;
   dd->sendControlEvent(event);
}

void QGraphicsTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
   if (dd->useDefaultImpl) {
      QGraphicsItem::mouseDoubleClickEvent(event);
      return;
   }

   if (!hasFocus()) {
      QGraphicsItem::mouseDoubleClickEvent(event);
      return;
   }

   dd->sendControlEvent(event);
}

void QGraphicsTextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
   dd->sendControlEvent(event);
}

void QGraphicsTextItem::keyPressEvent(QKeyEvent *event)
{
   dd->sendControlEvent(event);
}

void QGraphicsTextItem::keyReleaseEvent(QKeyEvent *event)
{
   dd->sendControlEvent(event);
}

void QGraphicsTextItem::focusInEvent(QFocusEvent *event)
{
   dd->sendControlEvent(event);
   if (event->reason() == Qt::MouseFocusReason) {
      dd->clickCausedFocus = 1;
   }
   update();
}

void QGraphicsTextItem::focusOutEvent(QFocusEvent *event)
{
   dd->sendControlEvent(event);
   update();
}

void QGraphicsTextItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
   dd->sendControlEvent(event);
}

void QGraphicsTextItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
   dd->sendControlEvent(event);
}

void QGraphicsTextItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
   dd->sendControlEvent(event);
}

void QGraphicsTextItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
   dd->sendControlEvent(event);
}

void QGraphicsTextItem::inputMethodEvent(QInputMethodEvent *event)
{
   dd->sendControlEvent(event);
}

void QGraphicsTextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
   dd->sendControlEvent(event);
}

void QGraphicsTextItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
   dd->sendControlEvent(event);
}

void QGraphicsTextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
   dd->sendControlEvent(event);
}

QVariant QGraphicsTextItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
   QVariant v;

   if (query == Qt::ImHints) {
      v = int(inputMethodHints());
   }  else if (dd->control) {
      v = dd->control->inputMethodQuery(query, QVariant());
   }

   if (v.type() == QVariant::RectF) {
      v = v.toRectF().translated(-dd->controlOffset());
   } else if (v.type() == QVariant::PointF) {
      v = v.toPointF() - dd->controlOffset();
   } else if (v.type() == QVariant::Rect) {
      v = v.toRect().translated(-dd->controlOffset().toPoint());
   } else if (v.type() == QVariant::Point) {
      v = v.toPoint() - dd->controlOffset().toPoint();
   }
   return v;
}

// internal
bool QGraphicsTextItem::supportsExtension(Extension extension) const
{
   (void) extension;
   return false;
}

// internal
void QGraphicsTextItem::setExtension(Extension extension, const QVariant &variant)
{
   (void) extension;
   (void) variant;
}

// internal
QVariant QGraphicsTextItem::extension(const QVariant &variant) const
{
   (void) variant;
   return QVariant();
}

// internal
void QGraphicsTextItemPrivate::_q_update(QRectF rect)
{
   if (rect.isValid()) {
      rect.translate(-controlOffset());
   } else {
      rect = boundingRect;
   }

   if (rect.intersects(boundingRect)) {
      qq->update(rect);
   }
}

// internal
void QGraphicsTextItemPrivate::_q_updateBoundingRect(const QSizeF &size)
{
   if (! control) {
      return;   // can not happen
   }

   const QSizeF pageSize = control->document()->pageSize();

   // paged items have a constant (page) size
   if (size == boundingRect.size() || pageSize.height() != -1) {
      return;
   }

   qq->prepareGeometryChange();
   boundingRect.setSize(size);
   qq->update();
}

// internal
void QGraphicsTextItemPrivate::_q_ensureVisible(QRectF rect)
{
   if (qq->hasFocus()) {
      rect.translate(-controlOffset());
      qq->ensureVisible(rect, /*xmargin=*/0, /*ymargin=*/0);
   }
}

QTextControl *QGraphicsTextItemPrivate::textControl() const
{
   if (! control) {
      QGraphicsTextItem *that = const_cast<QGraphicsTextItem *>(qq);
      control = new QTextControl(that);
      control->setTextInteractionFlags(Qt::NoTextInteraction);

      QObject::connect(control, &QTextControl::updateRequest,       qq, &QGraphicsTextItem::_q_update);
      QObject::connect(control, &QTextControl::documentSizeChanged, qq, &QGraphicsTextItem::_q_updateBoundingRect);
      QObject::connect(control, &QTextControl::visibilityRequest,   qq, &QGraphicsTextItem::_q_ensureVisible);
      QObject::connect(control, &QTextControl::linkActivated,       qq, &QGraphicsTextItem::linkActivated);
      QObject::connect(control, &QTextControl::linkHovered,         qq, &QGraphicsTextItem::linkHovered);

      const QSizeF pgSize = control->document()->pageSize();

      if (pgSize.height() != -1) {
         qq->prepareGeometryChange();
         that->dd->boundingRect.setSize(pgSize);
         qq->update();

      } else {
         that->dd->_q_updateBoundingRect(control->size());
      }
   }

   return control;
}

// internal
bool QGraphicsTextItemPrivate::_q_mouseOnEdge(QGraphicsSceneMouseEvent *event)
{
   QPainterPath path;
   path.addRect(qq->boundingRect());

   QPainterPath docPath;
   const QTextFrameFormat format = control->document()->rootFrame()->frameFormat();
   docPath.addRect(
      qq->boundingRect().adjusted(
         format.leftMargin(),
         format.topMargin(),
         -format.rightMargin(),
         -format.bottomMargin()));

   return path.subtracted(docPath).contains(event->pos());
}

void QGraphicsTextItem::setTextInteractionFlags(Qt::TextInteractionFlags flags)
{
   if (flags == Qt::NoTextInteraction) {
      setFlags(this->flags() & ~(QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemAcceptsInputMethod));
   } else {
      setFlags(this->flags() | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemAcceptsInputMethod);
   }

   dd->textControl()->setTextInteractionFlags(flags);
}

Qt::TextInteractionFlags QGraphicsTextItem::textInteractionFlags() const
{
   if (! dd->control) {
      return Qt::NoTextInteraction;
   }
   return dd->control->textInteractionFlags();
}

void QGraphicsTextItem::setTabChangesFocus(bool b)
{
   dd->tabChangesFocus = b;
}

bool QGraphicsTextItem::tabChangesFocus() const
{
   return dd->tabChangesFocus;
}

void QGraphicsTextItem::setOpenExternalLinks(bool open)
{
   dd->textControl()->setOpenExternalLinks(open);
}

bool QGraphicsTextItem::openExternalLinks() const
{
   if (!dd->control) {
      return false;
   }
   return dd->control->openExternalLinks();
}

void QGraphicsTextItem::setTextCursor(const QTextCursor &cursor)
{
   dd->textControl()->setTextCursor(cursor);
}

QTextCursor QGraphicsTextItem::textCursor() const
{
   if (!dd->control) {
      return QTextCursor();
   }
   return dd->control->textCursor();
}

class QGraphicsSimpleTextItemPrivate : public QAbstractGraphicsShapeItemPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsSimpleTextItem)

 public:
   inline QGraphicsSimpleTextItemPrivate() {
      pen.setStyle(Qt::NoPen);
      brush.setStyle(Qt::SolidPattern);
   }
   QString text;
   QFont font;
   QRectF boundingRect;

   void updateBoundingRect();
};

static QRectF setupTextLayout(QTextLayout *layout)
{
   layout->setCacheEnabled(true);
   layout->beginLayout();
   while (layout->createLine().isValid())
      ;
   layout->endLayout();
   qreal maxWidth = 0;
   qreal y = 0;
   for (int i = 0; i < layout->lineCount(); ++i) {
      QTextLine line = layout->lineAt(i);
      maxWidth = qMax(maxWidth, line.naturalTextWidth());
      line.setPosition(QPointF(0, y));
      y += line.height();
   }
   return QRectF(0, 0, maxWidth, y);
}

void QGraphicsSimpleTextItemPrivate::updateBoundingRect()
{
   Q_Q(QGraphicsSimpleTextItem);
   QRectF br;

   if (text.isEmpty()) {
      br = QRectF();

   } else {
      QString tmp = text;
      tmp.replace('\n', QChar(QChar::LineSeparator));

      QStackTextEngine engine(tmp, font);
      QTextLayout layout(&engine);
      br = setupTextLayout(&layout);
   }

   if (br != boundingRect) {
      q->prepareGeometryChange();
      boundingRect = br;
      q->update();
   }
}

QGraphicsSimpleTextItem::QGraphicsSimpleTextItem(QGraphicsItem *parent)
   : QAbstractGraphicsShapeItem(*new QGraphicsSimpleTextItemPrivate, parent)
{
}

QGraphicsSimpleTextItem::QGraphicsSimpleTextItem(const QString &text, QGraphicsItem *parent)
   : QAbstractGraphicsShapeItem(*new QGraphicsSimpleTextItemPrivate, parent)
{
   setText(text);
}

QGraphicsSimpleTextItem::~QGraphicsSimpleTextItem()
{
}

void QGraphicsSimpleTextItem::setText(const QString &text)
{
   Q_D(QGraphicsSimpleTextItem);
   if (d->text == text) {
      return;
   }
   d->text = text;
   d->updateBoundingRect();
   update();
}

QString QGraphicsSimpleTextItem::text() const
{
   Q_D(const QGraphicsSimpleTextItem);
   return d->text;
}

void QGraphicsSimpleTextItem::setFont(const QFont &font)
{
   Q_D(QGraphicsSimpleTextItem);
   d->font = font;
   d->updateBoundingRect();
}

QFont QGraphicsSimpleTextItem::font() const
{
   Q_D(const QGraphicsSimpleTextItem);
   return d->font;
}

QRectF QGraphicsSimpleTextItem::boundingRect() const
{
   Q_D(const QGraphicsSimpleTextItem);
   return d->boundingRect;
}

QPainterPath QGraphicsSimpleTextItem::shape() const
{
   Q_D(const QGraphicsSimpleTextItem);
   QPainterPath path;
   path.addRect(d->boundingRect);
   return path;
}

bool QGraphicsSimpleTextItem::contains(const QPointF &point) const
{
   Q_D(const QGraphicsSimpleTextItem);
   return d->boundingRect.contains(point);
}

void QGraphicsSimpleTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   (void) widget;

   Q_D(QGraphicsSimpleTextItem);

   painter->setFont(d->font);

   QString tmp = d->text;
   tmp.replace('\n', QChar(QChar::LineSeparator));

   QStackTextEngine engine(tmp, d->font);
   QTextLayout layout(&engine);
   QPen p;
   p.setBrush(d->brush);
   painter->setPen(p);

   if (d->pen.style() == Qt::NoPen && d->brush.style() == Qt::SolidPattern) {
      painter->setBrush(Qt::NoBrush);
   } else {
      QTextLayout::FormatRange range;
      range.start = 0;
      range.length = layout.text().length();
      range.format.setTextOutline(d->pen);
      layout.setFormats(QVector<QTextLayout::FormatRange>(1, range));
   }

   setupTextLayout(&layout);
   layout.draw(painter, QPointF(0, 0));

   if (option->state & (QStyle::State_Selected | QStyle::State_HasFocus)) {
      qt_graphicsItem_highlightSelected(this, painter, option);
   }
}

bool QGraphicsSimpleTextItem::isObscuredBy(const QGraphicsItem *item) const
{
   return QAbstractGraphicsShapeItem::isObscuredBy(item);
}

QPainterPath QGraphicsSimpleTextItem::opaqueArea() const
{
   return QAbstractGraphicsShapeItem::opaqueArea();
}

int QGraphicsSimpleTextItem::type() const
{
   return Type;
}

// internal
bool QGraphicsSimpleTextItem::supportsExtension(Extension extension) const
{
   (void) extension;
   return false;
}

// internal
void QGraphicsSimpleTextItem::setExtension(Extension extension, const QVariant &variant)
{
   (void) extension;
   (void) variant;
}

// internal
QVariant QGraphicsSimpleTextItem::extension(const QVariant &variant) const
{
   (void) variant;
   return QVariant();
}

class QGraphicsItemGroupPrivate : public QGraphicsItemPrivate
{
 public:
   QRectF itemsBoundingRect;
};

QGraphicsItemGroup::QGraphicsItemGroup(QGraphicsItem *parent)
   : QGraphicsItem(*new QGraphicsItemGroupPrivate, parent)
{
   setHandlesChildEvents(true);
}

QGraphicsItemGroup::~QGraphicsItemGroup()
{
}

void QGraphicsItemGroup::addToGroup(QGraphicsItem *item)
{
   Q_D(QGraphicsItemGroup);
   if (!item) {
      qWarning("QGraphicsItemGroup::addToGroup() Unable to add an invalid item (nullptr)");
      return;
   }
   if (item == this) {
      qWarning("QGraphicsItemGroup::addToGroup() Unable to add a group to itself");
      return;
   }

   // COMBINE
   bool ok;
   QTransform itemTransform = item->itemTransform(this, &ok);

   if (!ok) {
      qWarning("QGraphicsItemGroup::addToGroup() Unable to find a valid transformation from item to group coordinates");
      return;
   }

   QTransform newItemTransform(itemTransform);
   item->setPos(mapFromItem(item, 0, 0));
   item->setParentItem(this);

   // removing position from translation component of the new transform
   if (!item->pos().isNull()) {
      newItemTransform *= QTransform::fromTranslate(-item->x(), -item->y());
   }

   // removing additional transformations properties applied with itemTransform()
   QPointF origin = item->transformOriginPoint();
   QMatrix4x4 m;
   QList<QGraphicsTransform *> transformList = item->transformations();
   for (int i = 0; i < transformList.size(); ++i) {
      transformList.at(i)->applyTo(&m);
   }
   newItemTransform *= m.toTransform().inverted();
   newItemTransform.translate(origin.x(), origin.y());
   newItemTransform.rotate(-item->rotation());
   newItemTransform.scale(1 / item->scale(), 1 / item->scale());
   newItemTransform.translate(-origin.x(), -origin.y());

   // ### Expensive, we could maybe use dirtySceneTransform bit for optimization

   item->setTransform(newItemTransform);
   item->d_func()->setIsMemberOfGroup(true);
   prepareGeometryChange();
   d->itemsBoundingRect |= itemTransform.mapRect(item->boundingRect() | item->childrenBoundingRect());
   update();
}

void QGraphicsItemGroup::removeFromGroup(QGraphicsItem *item)
{
   Q_D(QGraphicsItemGroup);
   if (!item) {
      qWarning("QGraphicsItemGroup::removeFromGroup() Unable to remove an invalid item (nullptr)");
      return;
   }

   QGraphicsItem *newParent = d_ptr->parent;

   // COMBINE
   bool ok;
   QTransform itemTransform;
   if (newParent) {
      itemTransform = item->itemTransform(newParent, &ok);
   } else {
      itemTransform = item->sceneTransform();
   }

   QPointF oldPos = item->mapToItem(newParent, 0, 0);
   item->setParentItem(newParent);
   item->setPos(oldPos);

   // removing position from translation component of the new transform
   if (!item->pos().isNull()) {
      itemTransform *= QTransform::fromTranslate(-item->x(), -item->y());
   }

   // removing additional transformations properties applied
   // with itemTransform() or sceneTransform()
   QPointF origin = item->transformOriginPoint();
   QMatrix4x4 m;
   QList<QGraphicsTransform *> transformList = item->transformations();
   for (int i = 0; i < transformList.size(); ++i) {
      transformList.at(i)->applyTo(&m);
   }
   itemTransform *= m.toTransform().inverted();
   itemTransform.translate(origin.x(), origin.y());
   itemTransform.rotate(-item->rotation());
   itemTransform.scale(1 / item->scale(), 1 / item->scale());
   itemTransform.translate(-origin.x(), -origin.y());

   // ### Expensive, we could maybe use dirtySceneTransform bit for optimization

   item->setTransform(itemTransform);
   item->d_func()->setIsMemberOfGroup(item->group() != nullptr);

   // ### Quite expensive. But removeFromGroup() isn't called very often.
   prepareGeometryChange();
   d->itemsBoundingRect = childrenBoundingRect();
}

QRectF QGraphicsItemGroup::boundingRect() const
{
   Q_D(const QGraphicsItemGroup);
   return d->itemsBoundingRect;
}

void QGraphicsItemGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
   QWidget *widget)
{
   (void) widget;

   if (option->state & QStyle::State_Selected) {
      Q_D(QGraphicsItemGroup);

      painter->setBrush(Qt::NoBrush);
      painter->drawRect(d->itemsBoundingRect);
   }
}

bool QGraphicsItemGroup::isObscuredBy(const QGraphicsItem *item) const
{
   return QGraphicsItem::isObscuredBy(item);
}

QPainterPath QGraphicsItemGroup::opaqueArea() const
{
   return QGraphicsItem::opaqueArea();
}

int QGraphicsItemGroup::type() const
{
   return Type;
}

#ifndef QT_NO_GRAPHICSEFFECT
QRectF QGraphicsItemEffectSourcePrivate::boundingRect(Qt::CoordinateSystem system) const
{
   const bool deviceCoordinates = (system == Qt::DeviceCoordinates);
   if (!info && deviceCoordinates) {
      // Device coordinates without info not yet supported.
      qWarning("QGraphicsEffectSource::boundingRect() Not implemented, no device context");
      return QRectF();
   }

   QRectF rect = item->boundingRect();
   if (!item->d_ptr->children.isEmpty()) {
      rect |= item->childrenBoundingRect();
   }

   if (deviceCoordinates) {
      Q_ASSERT(info->painter);
      rect = info->painter->worldTransform().mapRect(rect);
   }

   return rect;
}

void QGraphicsItemEffectSourcePrivate::draw(QPainter *painter)
{
   if (!info) {
      qWarning("QGraphicsEffectSource::draw() Unable to draw, call QGraphicsEffect draw method");
      return;
   }

   Q_ASSERT(item->d_ptr->scene);
   QGraphicsScenePrivate *scened = item->d_ptr->scene->d_func();
   if (painter == info->painter) {
      scened->draw(item, painter, info->viewTransform, info->transformPtr, info->exposedRegion,
         info->widget, info->opacity, info->effectTransform, info->wasDirtySceneTransform,
         info->drawItem);

   } else {
      QTransform effectTransform = info->painter->worldTransform().inverted();
      effectTransform *= painter->worldTransform();
      scened->draw(item, painter, info->viewTransform, info->transformPtr, info->exposedRegion,
         info->widget, info->opacity, &effectTransform, info->wasDirtySceneTransform,
         info->drawItem);
   }
}

// sourceRect must be in the given coordinate system
QRect QGraphicsItemEffectSourcePrivate::paddedEffectRect(Qt::CoordinateSystem system, QGraphicsEffect::PixmapPadMode mode,
   const QRectF &sourceRect, bool *unpadded) const
{
   QRectF effectRectF;

   if (unpadded) {
      *unpadded = false;
   }

   if (mode == QGraphicsEffect::PadToEffectiveBoundingRect) {
      if (info) {
         QRectF deviceRect = system == Qt::DeviceCoordinates ? sourceRect : info->painter->worldTransform().mapRect(sourceRect);
         effectRectF = item->graphicsEffect()->boundingRectFor(deviceRect);
         if (unpadded) {
            *unpadded = (effectRectF.size() == sourceRect.size());
         }
         if (info && system == Qt::LogicalCoordinates) {
            effectRectF = info->painter->worldTransform().inverted().mapRect(effectRectF);
         }
      } else {
         // no choice but to send a logical coordinate bounding rect to boundingRectFor
         effectRectF = item->graphicsEffect()->boundingRectFor(sourceRect);
      }

   } else if (mode == QGraphicsEffect::PadToTransparentBorder) {
      // adjust by 1.5 to account for cosmetic pens
      effectRectF = sourceRect.adjusted(-1.5, -1.5, 1.5, 1.5);

   } else {
      effectRectF = sourceRect;
      if (unpadded) {
         *unpadded = true;
      }
   }

   return effectRectF.toAlignedRect();
}

QPixmap QGraphicsItemEffectSourcePrivate::pixmap(Qt::CoordinateSystem system, QPoint *offset,
   QGraphicsEffect::PixmapPadMode mode) const
{
   const bool deviceCoordinates = (system == Qt::DeviceCoordinates);
   if (!info && deviceCoordinates) {
      // Device coordinates without info not yet supported.
      qWarning("QGraphicsEffectSource::pixmap() Not implemented, no device context");
      return QPixmap();
   }

   if (!item->d_ptr->scene) {
      return QPixmap();
   }

   QGraphicsScenePrivate *scened = item->d_ptr->scene->d_func();

   bool unpadded;
   const QRectF sourceRect = boundingRect(system);
   QRect effectRect = paddedEffectRect(system, mode, sourceRect, &unpadded);

   if (offset) {
      *offset = effectRect.topLeft();
   }

   bool untransformed = !deviceCoordinates
      || info->painter->worldTransform().type() <= QTransform::TxTranslate;
   if (untransformed && unpadded && isPixmap()) {
      if (offset) {
         *offset = boundingRect(system).topLeft().toPoint();
      }
      return static_cast<QGraphicsPixmapItem *>(item)->pixmap();
   }

   if (effectRect.isEmpty()) {
      return QPixmap();
   }

   QPixmap pixmap(effectRect.size());
   pixmap.fill(Qt::transparent);
   QPainter pixmapPainter(&pixmap);
   pixmapPainter.setRenderHints(info ? info->painter->renderHints() : QPainter::TextAntialiasing);

   QTransform effectTransform = QTransform::fromTranslate(-effectRect.x(), -effectRect.y());
   if (deviceCoordinates && info->effectTransform) {
      effectTransform *= *info->effectTransform;
   }

   if (!info) {
      // Logical coordinates without info.
      QTransform sceneTransform = item->sceneTransform();
      QTransform newEffectTransform = sceneTransform.inverted();
      newEffectTransform *= effectTransform;
      scened->draw(item, &pixmapPainter, nullptr, &sceneTransform, nullptr, nullptr, qreal(1.0),
         &newEffectTransform, false, true);

   } else if (deviceCoordinates) {
      // Device coordinates with info.
      scened->draw(item, &pixmapPainter, info->viewTransform, info->transformPtr, nullptr,
         info->widget, info->opacity, &effectTransform, info->wasDirtySceneTransform, info->drawItem);

   } else {
      // Item coordinates with info.
      QTransform newEffectTransform = info->transformPtr->inverted();
      newEffectTransform *= effectTransform;

      scened->draw(item, &pixmapPainter, info->viewTransform, info->transformPtr, nullptr,
         info->widget, info->opacity, &newEffectTransform, info->wasDirtySceneTransform, info->drawItem);
   }

   pixmapPainter.end();

   return pixmap;
}
#endif

static void formatGraphicsItemHelper(QDebug debug, const QGraphicsItem *item)
{
   if (const QGraphicsItem *parent = item->parentItem()) {
      debug << ", parent =" << static_cast<const void *>(parent);
   }

   debug << ", pos =";
   QtDebugUtils::formatQPoint(debug, item->pos());

   if (const qreal z = item->zValue()) {
      debug << ", z =" << z;
   }

   if (item->flags()) {
      debug <<  ", flags =" << item->flags();
   }
}

QDebug operator<<(QDebug debug, const QGraphicsItem *item)
{
   QDebugStateSaver saver(debug);
   debug.nospace();

   if (! item) {
      debug << "QGraphicsItem(0)";
      return debug;
   }

   if (const QGraphicsObject *o = item->toGraphicsObject()) {
      debug << o->metaObject()->className();
   } else {
      debug << "QGraphicsItem";
   }

   debug << '(' << static_cast<const void *>(item);
   if (const QGraphicsProxyWidget *pw = qgraphicsitem_cast<const QGraphicsProxyWidget *>(item)) {
      debug << ", widget =";

      if (const QWidget *w = pw->widget()) {
         debug << w->metaObject()->className() << '(' << static_cast<const void *>(w);
         if (! w->objectName().isEmpty()) {
            debug << ", name =" << w->objectName();
         }

         debug << ')';

      } else {
         debug << "QWidget(nullptr)";
      }
   }

   formatGraphicsItemHelper(debug, item);
   debug << ')';

   return debug;
}

QDebug operator<<(QDebug debug, const QGraphicsObject *item)
{
   QDebugStateSaver saver(debug);
   debug.nospace();

   if (! item) {
      debug << "QGraphicsObject(nullptr)";
      return debug;
   }

   debug << item->metaObject()->className() << '(' << static_cast<const void *>(item);

   if (! item->objectName().isEmpty()) {
      debug << ", Name = " << item->objectName();
   }

   formatGraphicsItemHelper(debug, item);
   debug << ')';

   return debug;
}

QDebug operator<<(QDebug debug, QGraphicsItem::GraphicsItemChange change)
{
   const char *str = "UnknownChange";

   switch (change) {
      case QGraphicsItem::ItemChildAddedChange:
         str = "ItemChildAddedChange";
         break;
      case QGraphicsItem::ItemChildRemovedChange:
         str = "ItemChildRemovedChange";
         break;
      case QGraphicsItem::ItemCursorChange:
         str = "ItemCursorChange";
         break;
      case QGraphicsItem::ItemCursorHasChanged:
         str = "ItemCursorHasChanged";
         break;
      case QGraphicsItem::ItemEnabledChange:
         str = "ItemEnabledChange";
         break;
      case QGraphicsItem::ItemEnabledHasChanged:
         str = "ItemEnabledHasChanged";
         break;
      case QGraphicsItem::ItemFlagsChange:
         str = "ItemFlagsChange";
         break;
      case QGraphicsItem::ItemFlagsHaveChanged:
         str = "ItemFlagsHaveChanged";
         break;
      case QGraphicsItem::ItemMatrixChange:
         str = "ItemMatrixChange";
         break;
      case QGraphicsItem::ItemParentChange:
         str = "ItemParentChange";
         break;
      case QGraphicsItem::ItemParentHasChanged:
         str = "ItemParentHasChanged";
         break;
      case QGraphicsItem::ItemPositionChange:
         str = "ItemPositionChange";
         break;
      case QGraphicsItem::ItemPositionHasChanged:
         str = "ItemPositionHasChanged";
         break;
      case QGraphicsItem::ItemSceneChange:
         str = "ItemSceneChange";
         break;
      case QGraphicsItem::ItemSceneHasChanged:
         str = "ItemSceneHasChanged";
         break;
      case QGraphicsItem::ItemSelectedChange:
         str = "ItemSelectedChange";
         break;
      case QGraphicsItem::ItemSelectedHasChanged:
         str = "ItemSelectedHasChanged";
         break;
      case QGraphicsItem::ItemToolTipChange:
         str = "ItemToolTipChange";
         break;
      case QGraphicsItem::ItemToolTipHasChanged:
         str = "ItemToolTipHasChanged";
         break;
      case QGraphicsItem::ItemTransformChange:
         str = "ItemTransformChange";
         break;
      case QGraphicsItem::ItemTransformHasChanged:
         str = "ItemTransformHasChanged";
         break;
      case QGraphicsItem::ItemVisibleChange:
         str = "ItemVisibleChange";
         break;
      case QGraphicsItem::ItemVisibleHasChanged:
         str = "ItemVisibleHasChanged";
         break;
      case QGraphicsItem::ItemZValueChange:
         str = "ItemZValueChange";
         break;
      case QGraphicsItem::ItemZValueHasChanged:
         str = "ItemZValueHasChanged";
         break;
      case QGraphicsItem::ItemOpacityChange:
         str = "ItemOpacityChange";
         break;
      case QGraphicsItem::ItemOpacityHasChanged:
         str = "ItemOpacityHasChanged";
         break;
      case QGraphicsItem::ItemScenePositionHasChanged:
         str = "ItemScenePositionHasChanged";
         break;
      case QGraphicsItem::ItemRotationChange:
         str = "ItemRotationChange";
         break;
      case QGraphicsItem::ItemRotationHasChanged:
         str = "ItemRotationHasChanged";
         break;
      case QGraphicsItem::ItemScaleChange:
         str = "ItemScaleChange";
         break;
      case QGraphicsItem::ItemScaleHasChanged:
         str = "ItemScaleHasChanged";
         break;
      case QGraphicsItem::ItemTransformOriginPointChange:
         str = "ItemTransformOriginPointChange";
         break;
      case QGraphicsItem::ItemTransformOriginPointHasChanged:
         str = "ItemTransformOriginPointHasChanged";
         break;
   }

   debug << str;

   return debug;
}

QDebug operator<<(QDebug debug, QGraphicsItem::GraphicsItemFlag flag)
{
   const char *str = "UnknownFlag";

   switch (flag) {
      case QGraphicsItem::ItemIsMovable:
         str = "ItemIsMovable";
         break;

      case QGraphicsItem::ItemIsSelectable:
         str = "ItemIsSelectable";
         break;

      case QGraphicsItem::ItemIsFocusable:
         str = "ItemIsFocusable";
         break;

      case QGraphicsItem::ItemClipsToShape:
         str = "ItemClipsToShape";
         break;

      case QGraphicsItem::ItemClipsChildrenToShape:
         str = "ItemClipsChildrenToShape";
         break;

      case QGraphicsItem::ItemIgnoresTransformations:
         str = "ItemIgnoresTransformations";
         break;

      case QGraphicsItem::ItemIgnoresParentOpacity:
         str = "ItemIgnoresParentOpacity";
         break;

      case QGraphicsItem::ItemDoesntPropagateOpacityToChildren:
         str = "ItemDoesntPropagateOpacityToChildren";
         break;

      case QGraphicsItem::ItemStacksBehindParent:
         str = "ItemStacksBehindParent";
         break;

      case QGraphicsItem::ItemUsesExtendedStyleOption:
         str = "ItemUsesExtendedStyleOption";
         break;

      case QGraphicsItem::ItemHasNoContents:
         str = "ItemHasNoContents";
         break;

      case QGraphicsItem::ItemSendsGeometryChanges:
         str = "ItemSendsGeometryChanges";
         break;

      case QGraphicsItem::ItemAcceptsInputMethod:
         str = "ItemAcceptsInputMethod";
         break;

      case QGraphicsItem::ItemNegativeZStacksBehindParent:
         str = "ItemNegativeZStacksBehindParent";
         break;

      case QGraphicsItem::ItemIsPanel:
         str = "ItemIsPanel";
         break;

      case QGraphicsItem::ItemIsFocusScope:
         str = "ItemIsFocusScope";
         break;

      case QGraphicsItem::ItemSendsScenePositionChanges:
         str = "ItemSendsScenePositionChanges";
         break;

      case QGraphicsItem::ItemStopsClickFocusPropagation:
         str = "ItemStopsClickFocusPropagation";
         break;

      case QGraphicsItem::ItemStopsFocusHandling:
         str = "ItemStopsFocusHandling";
         break;

      case QGraphicsItem::ItemContainsChildrenInShape:
         str = "ItemContainsChildrenInShape";
         break;
   }

   debug << str;

   return debug;
}

QDebug operator<<(QDebug debug, QGraphicsItem::GraphicsItemFlags flags)
{
   debug << '(';
   bool f = false;

   for (int i = 0; i < 17; ++i) {
      if (flags & (1 << i)) {
         if (f) {
            debug << '|';
         }
         f = true;
         debug << QGraphicsItem::GraphicsItemFlag(int(flags & (1 << i)));
      }
   }

   debug << ')';

   return debug;
}

void QGraphicsTextItem::_q_updateBoundingRect(const QSizeF &sizeF)
{
   dd->_q_updateBoundingRect(sizeF);
}

void QGraphicsTextItem::_q_update(const QRectF &rectF)
{
   dd->_q_update(rectF);
}

void QGraphicsTextItem::_q_ensureVisible(const QRectF &rectF)
{
   dd->_q_ensureVisible(rectF);
}

// wrapper for overloaded method
QDeclarativeListProperty<QGraphicsObject> QGraphicsObject::cs_childrenList() const
{
   Q_D( const QGraphicsItem);
   return const_cast<QGraphicsItemPrivate *>(d)->childrenList();
}

// wrapper for overloaded method
qreal QGraphicsObject::cs_width() const
{
   Q_D(const QGraphicsItem);
   return d->width();
}

// wrapper for overloaded method
void QGraphicsObject::cs_setWidth(qreal width)
{
   Q_D(QGraphicsItem);
   d->setWidth(width);
}

// wrapper for overloaded method
void QGraphicsObject::cs_resetWidth()
{
   Q_D(QGraphicsItem);
   d->resetWidth();
}

// wrapper for overloaded method
qreal QGraphicsObject::cs_height() const
{
   Q_D(const QGraphicsItem);
   return d->height();
}

// wrapper for overloaded method
void QGraphicsObject::cs_setHeight(qreal height)
{
   Q_D(QGraphicsItem);
   d->setHeight(height);
}

// wrapper for overloaded method
void QGraphicsObject::cs_resetHeight()
{
   Q_D(QGraphicsItem);
   d->resetHeight();
}

#endif // QT_NO_GRAPHICSVIEW
