/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QGRAPHICS_SCENE_P_H
#define QGRAPHICS_SCENE_P_H

#include <qgraphicsscene.h>

#if !defined(QT_NO_GRAPHICSVIEW)

#include <qbitarray.h>
#include <qfont.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsview.h>
#include <qlist.h>
#include <qmap.h>
#include <qpalette.h>
#include <qset.h>
#include <qstyle.h>
#include <qstyleoption.h>

#include <qgraphics_item_p.h>
#include <qgraphics_view_p.h>

class QGraphicsSceneIndex;
class QGraphicsView;
class QGraphicsWidget;

class QGraphicsScenePrivate
{
   Q_DECLARE_PUBLIC(QGraphicsScene)

 public:
   QGraphicsScenePrivate();
   virtual ~QGraphicsScenePrivate() {};

   void init();

   static QGraphicsScenePrivate *get(QGraphicsScene *q);

   void registerTopLevelItem(QGraphicsItem *item);
   void unregisterTopLevelItem(QGraphicsItem *item);

   void _q_emitUpdated();
   void _q_updateLater();
   void _q_polishItems();
   void _q_processDirtyItems();
   void _q_updateScenePosDescendants();

   void setScenePosItemEnabled(QGraphicsItem *item, bool enabled);
   void registerScenePosItem(QGraphicsItem *item);
   void unregisterScenePosItem(QGraphicsItem *item);

   void removeItemHelper(QGraphicsItem *item);
   void setActivePanelHelper(QGraphicsItem *item, bool duringActivationEvent);
   void setFocusItemHelper(QGraphicsItem *item, Qt::FocusReason focusReason, bool emitFocusChanged = true);

   void addPopup(QGraphicsWidget *widget);
   void removePopup(QGraphicsWidget *widget, bool itemIsDying = false);

   void grabMouse(QGraphicsItem *item, bool implicit = false);
   void ungrabMouse(QGraphicsItem *item, bool itemIsDying = false);
   void clearMouseGrabber();

   void grabKeyboard(QGraphicsItem *item);
   void ungrabKeyboard(QGraphicsItem *item, bool itemIsDying = false);
   void clearKeyboardGrabber();
   void enableMouseTrackingOnViews();

   QList<QGraphicsItem *> itemsAtPosition(const QPoint &screenPos, const QPointF &scenePos, QWidget *widget) const;
   void storeMouseButtonsForMouseGrabber(QGraphicsSceneMouseEvent *event);

   void addView(QGraphicsView *view);
   void removeView(QGraphicsView *view);

   void installSceneEventFilter(QGraphicsItem *watched, QGraphicsItem *filter);
   void removeSceneEventFilter(QGraphicsItem *watched, QGraphicsItem *filter);
   bool filterDescendantEvent(QGraphicsItem *item, QEvent *event);
   bool filterEvent(QGraphicsItem *item, QEvent *event);
   bool sendEvent(QGraphicsItem *item, QEvent *event);

   bool dispatchHoverEvent(QGraphicsSceneHoverEvent *hoverEvent);
   bool itemAcceptsHoverEvents_helper(const QGraphicsItem *item) const;
   void leaveScene(QWidget *viewport);

   void cloneDragDropEvent(QGraphicsSceneDragDropEvent *dest, QGraphicsSceneDragDropEvent *source);
   void sendDragDropEvent(QGraphicsItem *item, QGraphicsSceneDragDropEvent *dragDropEvent);

   void sendHoverEvent(QEvent::Type type, QGraphicsItem *item, QGraphicsSceneHoverEvent *hoverEvent);
   void sendMouseEvent(QGraphicsSceneMouseEvent *mouseEvent);
   void mousePressEventHandler(QGraphicsSceneMouseEvent *mouseEvent);
   QGraphicsWidget *windowForItem(const QGraphicsItem *item) const;

   void drawItemHelper(QGraphicsItem *item, QPainter *painter, const QStyleOptionGraphicsItem *option,
         QWidget *widget, bool painterStateProtection);

   void drawItems(QPainter *painter, const QTransform *const viewTransform,
         QRegion *exposedRegion, QWidget *widget);

   void drawSubtreeRecursive(QGraphicsItem *item, QPainter *painter, const QTransform *const,
         QRegion *exposedRegion, QWidget *widget, qreal parentOpacity = qreal(1.0),
         const QTransform *const effectTransform = nullptr);

   void draw(QGraphicsItem *, QPainter *, const QTransform *const, const QTransform *const,
         QRegion *, QWidget *, qreal, const QTransform *const, bool, bool);

   void markDirty(QGraphicsItem *item, const QRectF &rect = QRectF(), bool invalidateChildren = false,
         bool force = false, bool ignoreOpacity = false, bool removingItemFromScene = false,
         bool updateBoundingRect = false);

   void processDirtyItemsRecursive(QGraphicsItem *item, bool dirtyAncestorContainsChildren = false,
      qreal parentOpacity = qreal(1.0));

   void resetDirtyItem(QGraphicsItem *item, bool recursive = false) {
      Q_ASSERT(item);
      item->d_ptr->dirty = 0;
      item->d_ptr->paintedViewBoundingRectsNeedRepaint = 0;
      item->d_ptr->geometryChanged = 0;

      if (!item->d_ptr->dirtyChildren) {
         recursive = false;
      }

      item->d_ptr->dirtyChildren = 0;
      item->d_ptr->needsRepaint = QRectF();
      item->d_ptr->allChildrenDirty = 0;
      item->d_ptr->fullUpdatePending = 0;
      item->d_ptr->ignoreVisible = 0;
      item->d_ptr->ignoreOpacity = 0;

#ifndef QT_NO_GRAPHICSEFFECT
      QGraphicsEffect::ChangeFlags flags;

      if (item->d_ptr->notifyBoundingRectChanged) {
         flags |= QGraphicsEffect::SourceBoundingRectChanged;
         item->d_ptr->notifyBoundingRectChanged = 0;
      }

      if (item->d_ptr->notifyInvalidated) {
         flags |= QGraphicsEffect::SourceInvalidated;
         item->d_ptr->notifyInvalidated = 0;
      }
#endif

      if (recursive) {
         for (int i = 0; i < item->d_ptr->children.size(); ++i) {
            resetDirtyItem(item->d_ptr->children.at(i), recursive);
         }
      }

#ifndef QT_NO_GRAPHICSEFFECT
      if (flags && item->d_ptr->graphicsEffect) {
         item->d_ptr->graphicsEffect->sourceChanged(flags);
      }
#endif

   }

   void ensureSortedTopLevelItems() {
      if (needSortTopLevelItems) {
         std::sort(topLevelItems.begin(), topLevelItems.end(), qt_notclosestLeaf);
         topLevelSequentialOrdering = false;
         needSortTopLevelItems = false;
      }
   }

   void ensureSequentialTopLevelSiblingIndexes();

   void setFont_helper(const QFont &font);
   void resolveFont();
   void updateFont(const QFont &font);
   void setPalette_helper(const QPalette &palette);
   void resolvePalette();
   void updatePalette(const QPalette &palette);

   static void updateTouchPointsForItem(QGraphicsItem *item, QTouchEvent *touchEvent);
   int findClosestTouchPointId(const QPointF &scenePos);
   void touchEventHandler(QTouchEvent *touchEvent);
   bool sendTouchBeginEvent(QGraphicsItem *item, QTouchEvent *touchEvent);
   void enableTouchEventsOnViews();

   void updateInputMethodSensitivityInViews();

   void enterModal(QGraphicsItem *item, QGraphicsItem::PanelModality panelModality = QGraphicsItem::NonModal);
   void leaveModal(QGraphicsItem *item);

   int activationRefCount;
   int childExplicitActivation;
   int lastItemCount;
   int selectionChanging;

   qreal minimumRenderSize;
   quint32 rectAdjust;

   quint32 allItemsIgnoreHoverEvents : 1;
   quint32 allItemsIgnoreTouchEvents : 1;
   quint32 allItemsUseDefaultCursor : 1;
   quint32 calledEmitUpdated : 1;
   quint32 dirtyGrowingItemsBoundingRect : 1;
   quint32 hasFocus : 1;
   quint32 hasSceneRect : 1;
   quint32 holesInTopLevelSiblingIndex : 1;
   quint32 lastMouseGrabberItemHasImplicitMouseGrab : 1;
   quint32 needSortTopLevelItems : 1;
   quint32 painterStateProtection : 1;
   quint32 processDirtyItemsEmitted : 1;
   quint32 scenePosDescendantsUpdatePending : 1;
   quint32 stickyFocus : 1;
   quint32 topLevelSequentialOrdering : 1;
   quint32 updateAll : 1;
   quint32 padding : 15;

   QBrush backgroundBrush;
   QBrush foregroundBrush;

   QStyle *style;
   QFont m_sceneFont;
   QPalette m_scenePalette;
   QPainterPath selectionArea;

   QStyleOptionGraphicsItem m_sceneStyleOption;

   QMetaMethod changedSignalMethod;
   QMetaMethod processDirtyItemsMethod;
   QMetaMethod polishItemsMethod;

   QGraphicsScene::ItemIndexMethod indexMethod;
   QGraphicsSceneIndex *m_graphicsSceneIndex;

   QPointF lastSceneMousePos;

   QRectF growingItemsBoundingRect;
   QRectF sceneRect;
   QList<QRectF> updatedRects;

   QList<QGraphicsItem *> cachedItemsUnderMouse;
   QList<QGraphicsItem *> hoverItems;
   QList<QGraphicsItem *> keyboardGrabberItems;
   QList<QGraphicsItem *> modalPanels;
   QList<QGraphicsItem *> mouseGrabberItems;
   QList<QGraphicsItem *> topLevelItems;

   QHash<QGraphicsItem *, QPointF> movingItemsInitialPositions;

   QSet<QGraphicsItem *> selectedItems;
   QSet<QGraphicsItem *> scenePosItems;

   QVector<QGraphicsItem *> unpolishedItems;
   QMultiMap<QGraphicsItem *, QGraphicsItem *> sceneEventFilters;

   QList<QGraphicsObject *> cachedTargetItems;
   QList<QGraphicsView *> views;
   QList<QGraphicsWidget *> popupWidgets;

   Qt::DropAction lastDropAction;

   QMap<Qt::MouseButton, QPointF> mouseGrabberButtonDownPos;
   QMap<Qt::MouseButton, QPointF> mouseGrabberButtonDownScenePos;
   QMap<Qt::MouseButton, QPoint>  mouseGrabberButtonDownScreenPos;

   QGraphicsItem *activePanel;
   QGraphicsItem *dragDropItem;
   QGraphicsItem *focusItem;
   QGraphicsItem *lastActivePanel;
   QGraphicsItem *lastFocusItem;
   QGraphicsItem *lastMouseGrabberItem;
   QGraphicsItem *passiveFocusItem;

   QGraphicsWidget *tabFocusFirst;
   QGraphicsWidget *enterWidget;

   QMap<int, QTouchEvent::TouchPoint> sceneCurrentTouchPoints;
   QMap<int, QGraphicsItem *> itemForTouchPointId;

#ifndef QT_NO_GESTURES
   void gestureEventHandler(QGestureEvent *event);

   void gestureTargetsAtHotSpots(const QSet<QGesture *> &gestures, Qt::GestureFlag flag,
         QHash<QGraphicsObject *, QSet<QGesture *>> *targets, QSet<QGraphicsObject *> *itemsSet = nullptr,
         QSet<QGesture *> *normal = nullptr, QSet<QGesture *> *conflicts = nullptr);

   void cancelGesturesForChildren(QGesture *original);
   void grabGesture(QGraphicsItem *, Qt::GestureType gesture);
   void ungrabGesture(QGraphicsItem *, Qt::GestureType gesture);

   QHash<QGraphicsObject *, QSet<QGesture *>> cachedItemGestures;
   QHash<QGraphicsObject *, QSet<QGesture *>> cachedAlreadyDeliveredGestures;

   QHash<QGesture *, QGraphicsObject *> gestureTargets;
   QHash<Qt::GestureType, int>  grabbedGestures;
#endif

 protected:
   QGraphicsScene *q_ptr;

};

// QRectF::intersects() returns false always if either the source or target
// rectangle's width or height are 0. This works around that problem.
static inline void _q_adjustRect(QRectF *rect)
{
   Q_ASSERT(rect);

   if (! rect->width()) {
      rect->adjust(qreal(-0.00001), 0, qreal(0.00001), 0);
   }
   if (! rect->height()) {
      rect->adjust(0, qreal(-0.00001), 0, qreal(0.00001));
   }
}

static inline QRectF adjustedItemBoundingRect(const QGraphicsItem *item)
{
   Q_ASSERT(item);

   QRectF boundingRect(item->boundingRect());
   _q_adjustRect(&boundingRect);
   return boundingRect;
}

static inline QRectF adjustedItemEffectiveBoundingRect(const QGraphicsItem *item)
{
   Q_ASSERT(item);

   QRectF boundingRect(QGraphicsItemPrivate::get(item)->effectiveBoundingRect());
   _q_adjustRect(&boundingRect);
   return boundingRect;
}


#endif // QT_NO_GRAPHICSVIEW

#endif
