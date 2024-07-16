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

#ifndef QGRAPHICS_ITEM_P_H
#define QGRAPHICS_ITEM_P_H

#include <qgraphicsitem.h>
#include <qgraphicseffect.h>
#include <qset.h>
#include <qpixmapcache.h>
#include <qpoint.h>
#include <qgraphicstransform.h>

#include <qgraphics_view_p.h>
#include <qgraphics_transform_p.h>
#include <qgraphicseffect_p.h>

#if !defined(QT_NO_GRAPHICSVIEW)

class QGraphicsItemPrivate;

class QGraphicsItemCache
{
 public:
   QGraphicsItemCache() : allExposed(false) { }

   // ItemCoordinateCache only
   QRect boundingRect;
   QSize fixedSize;
   QPixmapCache::Key key;

   // DeviceCoordinateCache only
   struct DeviceData {
      DeviceData() {}
      QTransform lastTransform;
      QPoint cacheIndent;
      QPixmapCache::Key key;
   };
   QHash<QPaintDevice *, DeviceData> deviceData;

   // List of logical exposed rects
   QVector<QRectF> exposed;
   bool allExposed;

   // Empty cache
   void purge();
};

class Q_GUI_EXPORT QGraphicsItemPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsItem)

 public:
   enum Extra {
      ExtraToolTip,
      ExtraCursor,
      ExtraCacheData,
      ExtraMaxDeviceCoordCacheSize,
      ExtraBoundingRegionGranularity
   };

   enum AncestorFlag {
      NoFlag = 0,
      AncestorHandlesChildEvents = 0x1,
      AncestorClipsChildren = 0x2,
      AncestorIgnoresTransformations = 0x4,
      AncestorFiltersChildEvents = 0x8,
      AncestorContainsChildren = 0x10
   };

   QGraphicsItemPrivate()
      : z(0), opacity(1.), scene(nullptr), parent(nullptr), transformData(nullptr), graphicsEffect(nullptr),
        index(-1), siblingIndex(-1), itemDepth(-1), focusProxy(nullptr), subFocusItem(nullptr),
        focusScopeItem(nullptr), imHints(Qt::ImhNone), panelModality(QGraphicsItem::NonModal),
        acceptedMouseButtons(0x1f), visible(1), explicitlyHidden(0), enabled(1), explicitlyDisabled(0), selected(0),
        acceptsHover(0), acceptDrops(0), isMemberOfGroup(0), handlesChildEvents(0), itemDiscovered(0),
        hasCursor(0), ancestorFlags(0), cacheMode(0), hasBoundingRegionGranularity(0), isWidget(0), dirty(0),
        dirtyChildren(0), localCollisionHack(0), inSetPosHelper(0), needSortChildren(0), allChildrenDirty(0),
        fullUpdatePending(0), itemFlags(0), paintedViewBoundingRectsNeedRepaint(0), dirtySceneTransform(1),
        geometryChanged(1), inDestructor(0), isObject(0), ignoreVisible(0), ignoreOpacity(0),
        acceptTouchEvents(0), acceptedTouchBeginEvent(0), filtersDescendantEvents(0), sceneTransformTranslateOnly(0),
        notifyBoundingRectChanged(0), notifyInvalidated(0), mouseSetsFocus(1), explicitActivate(0),
        wantsActive(0), holesInSiblingIndex(0), sequentialOrdering(1), updateDueToGraphicsEffect(0), scenePosDescendants(0),
        pendingPolish(0), mayHaveChildWithGraphicsEffect(0), isDeclarativeItem(0), sendParentChangeNotification(0),
        dirtyChildrenBoundingRect(1), globalStackingOrder(-1), q_ptr(nullptr)
   {
   }

   virtual ~QGraphicsItemPrivate() {
   }

   static const QGraphicsItemPrivate *get(const QGraphicsItem *item) {
      return item->d_ptr.data();
   }

   static QGraphicsItemPrivate *get(QGraphicsItem *item) {
      return item->d_ptr.data();
   }

   void updateChildWithGraphicsEffectFlagRecursively();
   void updateAncestorFlag(QGraphicsItem::GraphicsItemFlag childFlag,
      AncestorFlag flag = NoFlag, bool enabled = false, bool root = true);

   void updateAncestorFlags();
   void setIsMemberOfGroup(bool enabled);
   void remapItemPos(QEvent *event, QGraphicsItem *item);
   QPointF genericMapFromScene(const QPointF &pos, const QWidget *viewport) const;

   inline bool itemIsUntransformable() const {
      return (itemFlags & QGraphicsItem::ItemIgnoresTransformations) || (ancestorFlags & AncestorIgnoresTransformations);
   }

   void combineTransformToParent(QTransform *x, const QTransform *viewTransform = nullptr) const;
   void combineTransformFromParent(QTransform *x, const QTransform *viewTransform = nullptr) const;
   virtual void updateSceneTransformFromParent();

   static bool movableAncestorIsSelected(const QGraphicsItem *item);

   virtual void setPosHelper(const QPointF &pos);
   void setTransformHelper(const QTransform &transform);
   void prependGraphicsTransform(QGraphicsTransform *t);
   void appendGraphicsTransform(QGraphicsTransform *t);
   void setVisibleHelper(bool newVisible, bool explicitly, bool update = true, bool hiddenByPanel = false);
   void setEnabledHelper(bool newEnabled, bool explicitly, bool update = true);

   bool discardUpdateRequest(bool ignoreVisibleBit = false, bool ignoreDirtyBit = false, bool ignoreOpacity = false) const;
   virtual void transformChanged() {}
   int depth() const;

#ifndef QT_NO_GRAPHICSEFFECT
   enum InvalidateReason {
      OpacityChanged
   };
   void invalidateParentGraphicsEffectsRecursively();
   void invalidateChildGraphicsEffectsRecursively(InvalidateReason reason);
#endif

   void invalidateDepthRecursively();
   void resolveDepth();
   void addChild(QGraphicsItem *child);
   void removeChild(QGraphicsItem *child);

   QDeclarativeListProperty<QGraphicsObject> childrenList();
   void setParentItemHelper(QGraphicsItem *parent, const QVariant *newParentVariant, const QVariant *thisPointerVariant);

   void childrenBoundingRectHelper(QTransform *x, QRectF *rect, QGraphicsItem *topMostEffectItem);

   void initStyleOption(QStyleOptionGraphicsItem *option, const QTransform &worldTransform,
      const QRegion &exposedRegion, bool allItems = false) const;

   QRectF effectiveBoundingRect(QGraphicsItem *topMostEffectItem = nullptr) const;
   QRectF sceneEffectiveBoundingRect() const;

   QRectF effectiveBoundingRect(const QRectF &rect) const;

   virtual void resolveFont(uint inheritedMask) {
      for (int i = 0; i < children.size(); ++i) {
         children.at(i)->d_ptr->resolveFont(inheritedMask);
      }
   }

   virtual void resolvePalette(uint inheritedMask) {
      for (int i = 0; i < children.size(); ++i) {
         children.at(i)->d_ptr->resolvePalette(inheritedMask);
      }
   }

   virtual bool isProxyWidget() const;

   QVariant extra(Extra type) const {
      for (int i = 0; i < extras.size(); ++i) {
         const ExtraStruct &extra = extras.at(i);

         if (extra.m_type == type) {
            return extra.m_value;
         }
      }
      return QVariant();
   }

   void setExtra(Extra type, const QVariant &value) {
      int key = -1;

      for (int i = 0; i < extras.size(); ++i) {
         if (extras.at(i).m_type == type) {
            key = i;
            break;
         }
      }

      if (key == -1) {
         extras << ExtraStruct(type, value);
      } else {
         extras[key].m_value = value;
      }
   }

   void unsetExtra(Extra type) {
      for (int i = 0; i < extras.size(); ++i) {
         if (extras.at(i).m_type == type) {
            extras.removeAt(i);
            return;
         }
      }
   }

   struct ExtraStruct {
      ExtraStruct()
      { }

      ExtraStruct(Extra type, QVariant value)
         : m_type(type), m_value(value) {
      }

      Extra m_type;
      QVariant m_value;

      bool operator<(Extra extra) const {
         return m_type < extra;
      }
   };

   QVector<ExtraStruct> extras;

   QGraphicsItemCache *maybeExtraItemCache() const;
   QGraphicsItemCache *extraItemCache() const;
   void removeExtraItemCache();

   void updatePaintedViewBoundingRects(bool updateChildren);
   void ensureSceneTransformRecursive(QGraphicsItem **topMostDirtyItem);

   inline void ensureSceneTransform() {
      QGraphicsItem *that = q_func();
      ensureSceneTransformRecursive(&that);
   }

   inline bool hasTranslateOnlySceneTransform() {
      ensureSceneTransform();
      return sceneTransformTranslateOnly;
   }

   inline void invalidateChildrenSceneTransform() {
      for (int i = 0; i < children.size(); ++i) {
         children.at(i)->d_ptr->dirtySceneTransform = 1;
      }
   }

   inline qreal calcEffectiveOpacity() const {
      qreal retval = opacity;
      int myFlags  = itemFlags;

      QGraphicsItem *p = parent;

      while (p) {
         int parentFlags = p->d_ptr->itemFlags;

         // If I have a parent and I don't ignore my parent's opacity, and my
         // parent propagates to me, then combine my local opacity with my parent's
         // effective opacity into my effective opacity.

         if ((myFlags & QGraphicsItem::ItemIgnoresParentOpacity)
               || (parentFlags & QGraphicsItem::ItemDoesntPropagateOpacityToChildren)) {
            break;
         }

         retval *= p->d_ptr->opacity;
         p       = p->d_ptr->parent;

         myFlags = parentFlags;
      }

      return retval;
   }

   inline bool isOpacityNull() const {
      return (opacity < qreal(0.001));
   }

   static inline bool isOpacityNull(qreal opacity) {
      return (opacity < qreal(0.001));
   }

   inline bool isFullyTransparent() const {
      if (isOpacityNull()) {
         return true;
      }
      if (!parent) {
         return false;
      }

      return isOpacityNull(calcEffectiveOpacity());
   }

   inline qreal effectiveOpacity() const {
      if (! parent || ! opacity) {
         return opacity;
      }

      return calcEffectiveOpacity();
   }

   inline qreal combineOpacityFromParent(qreal parentOpacity) const {
      if (parent && ! (itemFlags & QGraphicsItem::ItemIgnoresParentOpacity)
            && ! (parent->d_ptr->itemFlags & QGraphicsItem::ItemDoesntPropagateOpacityToChildren)) {
         return parentOpacity * opacity;
      }
      return opacity;
   }

   inline bool childrenCombineOpacity() const {
      if (! children.size()) {
         return true;
      }

      if (itemFlags & QGraphicsItem::ItemDoesntPropagateOpacityToChildren) {
         return false;
      }

      for (int i = 0; i < children.size(); ++i) {
         if (children.at(i)->d_ptr->itemFlags & QGraphicsItem::ItemIgnoresParentOpacity) {
            return false;
         }
      }
      return true;
   }

   inline bool childrenClippedToShape() const {
      return (itemFlags & QGraphicsItem::ItemClipsChildrenToShape) || children.isEmpty();
   }

   inline bool isInvisible() const {
      return !visible || (childrenCombineOpacity() && isFullyTransparent());
   }

   inline void markParentDirty(bool updateBoundingRect = false);

   void setFocusHelper(Qt::FocusReason focusReason, bool climb, bool focusFromHide);
   void clearFocusHelper(bool giveFocusToParent, bool hiddenByParentPanel);
   void setSubFocus(QGraphicsItem *rootItem = nullptr, QGraphicsItem *stopItem = nullptr);
   void clearSubFocus(QGraphicsItem *rootItem = nullptr, QGraphicsItem *stopItem = nullptr);
   void resetFocusProxy();
   virtual void subFocusItemChange();
   virtual void focusScopeItemChange(bool isSubFocusItem);

   static void children_append(QDeclarativeListProperty<QGraphicsObject> *list, QGraphicsObject *item);
   static int children_count(QDeclarativeListProperty<QGraphicsObject> *list);
   static QGraphicsObject *children_at(QDeclarativeListProperty<QGraphicsObject> *list, int);
   static void children_clear(QDeclarativeListProperty<QGraphicsObject> *list);

   inline QTransform transformToParent() const;
   inline void ensureSortedChildren();
   static inline bool insertionOrder(QGraphicsItem *a, QGraphicsItem *b);
   void ensureSequentialSiblingIndex();
   inline void sendScenePosChange();
   virtual void siblingOrderChange();

   // Private Properties
   virtual qreal width() const;
   virtual void setWidth(qreal);
   virtual void resetWidth();

   virtual qreal height() const;
   virtual void setHeight(qreal);
   virtual void resetHeight();

   QRectF childrenBoundingRect;
   QRectF needsRepaint;
   QHash<QWidget *, QRect> paintedViewBoundingRects;
   QPointF pos;
   qreal z;
   qreal opacity;

   QGraphicsScene *scene;
   QGraphicsItem *parent;
   QList<QGraphicsItem *> children;

   struct TransformData;
   TransformData *transformData;
   QGraphicsEffect *graphicsEffect;
   QTransform sceneTransform;
   int index;
   int siblingIndex;
   int itemDepth;  // Lazily calculated when calling depth().
   QGraphicsItem *focusProxy;
   QList<QGraphicsItem **> focusProxyRefs;
   QGraphicsItem *subFocusItem;
   QGraphicsItem *focusScopeItem;
   Qt::InputMethodHints imHints;
   QGraphicsItem::PanelModality panelModality;

#ifndef QT_NO_GESTURES
   QMap<Qt::GestureType, Qt::GestureFlags> gestureContext;
#endif

   // Packed 32 bits
   quint32 acceptedMouseButtons : 5;
   quint32 visible : 1;
   quint32 explicitlyHidden : 1;
   quint32 enabled : 1;
   quint32 explicitlyDisabled : 1;
   quint32 selected : 1;
   quint32 acceptsHover : 1;
   quint32 acceptDrops : 1;
   quint32 isMemberOfGroup : 1;
   quint32 handlesChildEvents : 1;
   quint32 itemDiscovered : 1;
   quint32 hasCursor : 1;
   quint32 ancestorFlags : 5;
   quint32 cacheMode : 2;
   quint32 hasBoundingRegionGranularity : 1;
   quint32 isWidget : 1;
   quint32 dirty : 1;
   quint32 dirtyChildren : 1;
   quint32 localCollisionHack : 1;
   quint32 inSetPosHelper : 1;
   quint32 needSortChildren : 1;
   quint32 allChildrenDirty : 1;
   quint32 fullUpdatePending : 1;

   // Packed 32 bits
   quint32 itemFlags : 20;
   quint32 paintedViewBoundingRectsNeedRepaint : 1;
   quint32 dirtySceneTransform : 1;
   quint32 geometryChanged : 1;
   quint32 inDestructor : 1;
   quint32 isObject : 1;
   quint32 ignoreVisible : 1;
   quint32 ignoreOpacity : 1;
   quint32 acceptTouchEvents : 1;
   quint32 acceptedTouchBeginEvent : 1;
   quint32 filtersDescendantEvents : 1;
   quint32 sceneTransformTranslateOnly : 1;
   quint32 notifyBoundingRectChanged : 1;
   quint32 notifyInvalidated : 1;

   // New 32 bits
   quint32 mouseSetsFocus : 1;
   quint32 explicitActivate : 1;
   quint32 wantsActive : 1;
   quint32 holesInSiblingIndex : 1;
   quint32 sequentialOrdering : 1;
   quint32 updateDueToGraphicsEffect : 1;
   quint32 scenePosDescendants : 1;
   quint32 pendingPolish : 1;
   quint32 mayHaveChildWithGraphicsEffect : 1;
   quint32 isDeclarativeItem : 1;
   quint32 sendParentChangeNotification : 1;
   quint32 dirtyChildrenBoundingRect : 1;
   quint32 padding : 19;

   // Optional stacking order
   int globalStackingOrder;
   QGraphicsItem *q_ptr;
};

struct QGraphicsItemPrivate::TransformData {
   QTransform transform;
   qreal scale;
   qreal rotation;
   qreal xOrigin;
   qreal yOrigin;
   QList<QGraphicsTransform *> graphicsTransforms;
   bool onlyTransform;

   TransformData() :
      scale(1.0), rotation(0.0),
      xOrigin(0.0), yOrigin(0.0),
      onlyTransform(true) {
   }

   QTransform computedFullTransform(QTransform *postmultiplyTransform = nullptr) const {
      if (onlyTransform) {
         if (!postmultiplyTransform || postmultiplyTransform->isIdentity()) {
            return transform;
         }
         if (transform.isIdentity()) {
            return *postmultiplyTransform;
         }
         return transform **postmultiplyTransform;
      }

      QTransform x(transform);
      if (!graphicsTransforms.isEmpty()) {
         QMatrix4x4 m;
         for (int i = 0; i < graphicsTransforms.size(); ++i) {
            graphicsTransforms.at(i)->applyTo(&m);
         }
         x *= m.toTransform();
      }
      x.translate(xOrigin, yOrigin);
      x.rotate(rotation);
      x.scale(scale, scale);
      x.translate(-xOrigin, -yOrigin);
      if (postmultiplyTransform) {
         x *= *postmultiplyTransform;
      }
      return x;
   }
};

struct QGraphicsItemPaintInfo {
   inline QGraphicsItemPaintInfo(const QTransform *const xform1, const QTransform *const xform2,
      const QTransform *const xform3,
      QRegion *r, QWidget *w, QStyleOptionGraphicsItem *opt,
      QPainter *p, qreal o, bool b1, bool b2)
      : viewTransform(xform1), transformPtr(xform2), effectTransform(xform3), exposedRegion(r), widget(w),
        option(opt), painter(p), opacity(o), wasDirtySceneTransform(b1), drawItem(b2) {
   }

   const QTransform *viewTransform;
   const QTransform *transformPtr;
   const QTransform *effectTransform;
   QRegion *exposedRegion;
   QWidget *widget;
   QStyleOptionGraphicsItem *option;
   QPainter *painter;
   qreal opacity;
   quint32 wasDirtySceneTransform : 1;
   quint32 drawItem : 1;
};

#ifndef QT_NO_GRAPHICSEFFECT
class QGraphicsItemEffectSourcePrivate : public QGraphicsEffectSourcePrivate
{
 public:
   QGraphicsItemEffectSourcePrivate(QGraphicsItem *i)
      : QGraphicsEffectSourcePrivate(), item(i), info(nullptr) {
   }

   void detach() override {
      item->d_ptr->graphicsEffect = nullptr;
      item->prepareGeometryChange();
   }

   const QGraphicsItem *graphicsItem() const  override {
      return item;
   }

   const QWidget *widget() const override {
      return nullptr;
   }

   void update() override {
      item->d_ptr->updateDueToGraphicsEffect = true;
      item->update();
      item->d_ptr->updateDueToGraphicsEffect = false;
   }

   void effectBoundingRectChanged() override {
      item->prepareGeometryChange();
   }

   bool isPixmap() const override {
      return item->type() == QGraphicsPixmapItem::Type && ! (item->flags() & QGraphicsItem::ItemIsSelectable)
            && item->d_ptr->children.size() == 0;
   }

   const QStyleOption *styleOption() const override {
      return info ? info->option : nullptr;
   }

   QRect deviceRect() const override {
      if (! info || ! info->widget) {
         qWarning("QGraphicsEffectSource::deviceRect() Not implemented, no device context");
         return QRect();
      }

      return info->widget->rect();
   }

   QRectF boundingRect(Qt::CoordinateSystem system) const override;
   void draw(QPainter *) override;

   QPixmap pixmap(Qt::CoordinateSystem system, QPoint *offset, QGraphicsEffect::PixmapPadMode mode) const override;

   QRect paddedEffectRect(Qt::CoordinateSystem system, QGraphicsEffect::PixmapPadMode mode, const QRectF &sourceRect,
      bool *unpadded = nullptr) const;

   QGraphicsItem *item;
   QGraphicsItemPaintInfo *info;
   QTransform lastEffectTransform;
};
#endif //QT_NO_GRAPHICSEFFECT

inline bool qt_closestItemFirst(const QGraphicsItem *item1, const QGraphicsItem *item2)
{
   // Siblings? Just check their z-values.
   const QGraphicsItemPrivate *d1 = item1->d_ptr.data();
   const QGraphicsItemPrivate *d2 = item2->d_ptr.data();
   if (d1->parent == d2->parent) {
      return qt_closestLeaf(item1, item2);
   }

   // Find common ancestor, and each item's ancestor closest to the common ancestor
   int item1Depth = d1->depth();
   int item2Depth = d2->depth();
   const QGraphicsItem *p = item1;
   const QGraphicsItem *t1 = item1;

   while (item1Depth > item2Depth && (p = p->d_ptr->parent)) {
      if (p == item2) {
         // item2 is one of item1's ancestors; item1 is on top
         return !(t1->d_ptr->itemFlags & QGraphicsItem::ItemStacksBehindParent);
      }
      t1 = p;
      --item1Depth;
   }

   p = item2;
   const QGraphicsItem *t2 = item2;

   while (item2Depth > item1Depth && (p = p->d_ptr->parent)) {
      if (p == item1) {
         // item1 is one of item2's ancestors; item1 is not on top
         return (t2->d_ptr->itemFlags & QGraphicsItem::ItemStacksBehindParent);
      }
      t2 = p;
      --item2Depth;
   }

   // item1Ancestor is now at the same level as item2Ancestor, but not the same.
   const QGraphicsItem *p1 = t1;
   const QGraphicsItem *p2 = t2;
   while (t1 && t1 != t2) {
      p1 = t1;
      p2 = t2;
      t1 = t1->d_ptr->parent;
      t2 = t2->d_ptr->parent;
   }

   // in case we have a common ancestor, we compare the immediate children in the ancestor's path.
   // otherwise we compare the respective items' topLevelItems directly.
   return qt_closestLeaf(p1, p2);
}

inline bool qt_closestItemLast(const QGraphicsItem *item1, const QGraphicsItem *item2)
{
   return qt_closestItemFirst(item2, item1);
}

inline bool qt_closestLeaf(const QGraphicsItem *item1, const QGraphicsItem *item2)
{
   // Return true if sibling item1 is on top of item2.
   const QGraphicsItemPrivate *d1 = item1->d_ptr.data();
   const QGraphicsItemPrivate *d2 = item2->d_ptr.data();

   bool f1 = d1->itemFlags & QGraphicsItem::ItemStacksBehindParent;
   bool f2 = d2->itemFlags & QGraphicsItem::ItemStacksBehindParent;

   if (f1 != f2) {
      return f2;
   }

   if (d1->z != d2->z) {
      return d1->z > d2->z;
   }

   return d1->siblingIndex > d2->siblingIndex;
}

inline bool qt_notclosestLeaf(const QGraphicsItem *item1, const QGraphicsItem *item2)
{
   return qt_closestLeaf(item2, item1);
}

/*
   return the full transform of the item to the parent.  This include the position and all the transform data
*/
inline QTransform QGraphicsItemPrivate::transformToParent() const
{
   QTransform matrix;
   combineTransformToParent(&matrix);
   return matrix;
}

// internal
inline void QGraphicsItemPrivate::ensureSortedChildren()
{
   if (needSortChildren) {
      needSortChildren = 0;
      sequentialOrdering = 1;
      if (children.isEmpty()) {
         return;
      }
      std::sort(children.begin(), children.end(), qt_notclosestLeaf);

      for (int i = 0; i < children.size(); ++i) {
         if (children.at(i)->d_ptr->siblingIndex != i) {
            sequentialOrdering = 0;
            break;
         }
      }
   }
}

// internal
inline bool QGraphicsItemPrivate::insertionOrder(QGraphicsItem *a, QGraphicsItem *b)
{
   return a->d_ptr->siblingIndex < b->d_ptr->siblingIndex;
}

inline void QGraphicsItemPrivate::markParentDirty(bool updateBoundingRect)
{
   QGraphicsItemPrivate *parentp = this;
#ifndef QT_NO_GRAPHICSEFFECT
   if (updateBoundingRect && parentp->graphicsEffect && !parentp->inSetPosHelper) {
      parentp->notifyInvalidated = 1;
      static_cast<QGraphicsItemEffectSourcePrivate *>(parentp->graphicsEffect->d_func()
         ->source->d_func())->invalidateCache();
   }
#endif
   while (parentp->parent) {
      parentp = parentp->parent->d_ptr.data();
      parentp->dirtyChildren = 1;

      if (updateBoundingRect) {
         parentp->dirtyChildrenBoundingRect = 1;
         // ### Only do this if the parent's effect applies to the entire subtree.
         parentp->notifyBoundingRectChanged = 1;
      }
#ifndef QT_NO_GRAPHICSEFFECT
      if (parentp->graphicsEffect) {
         if (updateBoundingRect) {
            static_cast<QGraphicsItemEffectSourcePrivate *>(parentp->graphicsEffect->d_func()
               ->source->d_func())->invalidateCache();
            parentp->notifyInvalidated = 1;
         }
         if (parentp->scene && parentp->graphicsEffect->isEnabled()) {
            parentp->dirty = 1;
            parentp->fullUpdatePending = 1;
         }
      }
#endif
   }
}


#endif // QT_NO_GRAPHICSVIEW

#endif
