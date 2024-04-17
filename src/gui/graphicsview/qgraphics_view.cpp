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

static constexpr const int QGRAPHICSVIEW_REGION_RECT_THRESHOLD  = 50;
static constexpr const int QGRAPHICSVIEW_PREALLOC_STYLE_OPTIONS = 503; // largest prime < 2^9

#include <qgraphicsview.h>
#include <qgraphics_view_p.h>

#ifndef QT_NO_GRAPHICSVIEW

#include <qapplication.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>
#include <qgraphicswidget.h>
#include <qlayout.h>
#include <qmath.h>
#include <qmatrix.h>
#include <qpainter.h>
#include <qscopedvaluerollback.h>
#include <qscrollbar.h>
#include <qstyleoption.h>
#include <qtransform.h>

#include <qevent_p.h>
#include <qgraphics_item_p.h>
#include <qgraphics_scene_p.h>

bool qt_sendSpontaneousEvent(QObject *receiver, QEvent *event);

inline int q_round_bound(qreal d) //### (int)(qreal) INT_MAX != INT_MAX for single precision
{
   if (d <= (qreal) INT_MIN) {
      return INT_MIN;

   } else if (d >= (qreal) INT_MAX) {
      return INT_MAX;
   }

   return d >= 0.0 ? int(d + qreal(0.5)) : int(d - int(d - 1) + qreal(0.5)) + int(d - 1);
}

void QGraphicsViewPrivate::translateTouchEvent(QGraphicsViewPrivate *d, QTouchEvent *touchEvent)
{
   QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();

   for (int i = 0; i < touchPoints.count(); ++i) {
      QTouchEvent::TouchPoint &touchPoint = touchPoints[i];

      // the scene will set the item local pos, startPos, lastPos, and rect before delivering to
      // an item, but for now those functions are returning the view's local coordinates

      touchPoint.setSceneRect(d->mapToScene(touchPoint.rect()));
      touchPoint.setStartScenePos(d->mapToScene(touchPoint.startPos()));
      touchPoint.setLastScenePos(d->mapToScene(touchPoint.lastPos()));

      // screenPos, startScreenPos, lastScreenPos, and screenRect are already set
   }

   touchEvent->setTouchPoints(touchPoints);
}

QGraphicsViewPrivate::QGraphicsViewPrivate()
   : renderHints(QPainter::TextAntialiasing), dragMode(QGraphicsView::NoDrag),
     sceneInteractionAllowed(true), hasSceneRect(false), connectedToScene(false),
     useLastMouseEvent(false), identityMatrix(true), dirtyScroll(true), accelerateScrolling(true),
     keepLastCenterPoint(true), transforming(false), handScrolling(false), mustAllocateStyleOptions(false),
     mustResizeBackgroundPixmap(true), fullUpdatePending(true), hasUpdateClip(false),
     mousePressButton(Qt::NoButton), leftIndent(0), topIndent(0),
     lastMouseEvent(QEvent::None, QPointF(), QPointF(), QPointF(), Qt::NoButton, Qt::EmptyFlag, Qt::EmptyFlag),
     alignment(Qt::AlignCenter),
     transformationAnchor(QGraphicsView::AnchorViewCenter), resizeAnchor(QGraphicsView::NoAnchor),
     viewportUpdateMode(QGraphicsView::MinimalViewportUpdate), optimizationFlags(Qt::EmptyFlag), scene(nullptr),

#ifndef QT_NO_RUBBERBAND
     rubberBanding(false), rubberBandSelectionMode(Qt::IntersectsItemShape),
     rubberBandSelectionOperation(Qt::ReplaceSelection),
#endif

     handScrollMotions(0), cacheMode(Qt::EmptyFlag),

#ifndef QT_NO_CURSOR
     hasStoredOriginalCursor(false),
#endif

     lastDragDropEvent(nullptr), updateSceneSlotReimplementedChecked(false)
{
   styleOptions.reserve(QGRAPHICSVIEW_PREALLOC_STYLE_OPTIONS);
}

QGraphicsViewPrivate::~QGraphicsViewPrivate()
{
}

void QGraphicsViewPrivate::recalculateContentSize()
{
   Q_Q(QGraphicsView);

   QSize maxSize   = q->maximumViewportSize();
   int width       = maxSize.width();
   int height      = maxSize.height();
   QRectF viewRect = matrix.mapRect(q->sceneRect());

   bool frameOnlyAround = (q->style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents, nullptr, q));

   if (frameOnlyAround) {
      if (hbarpolicy == Qt::ScrollBarAlwaysOn) {
         height -= frameWidth * 2;
      }

      if (vbarpolicy == Qt::ScrollBarAlwaysOn) {
         width -= frameWidth * 2;
      }
   }

   // Adjust the maximum width and height of the viewport based on the width
   // of visible scroll bars.
   int scrollBarExtent = q->style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr, q);
   if (frameOnlyAround) {
      scrollBarExtent += frameWidth * 2;
   }

   bool useHorizontalScrollBar = (viewRect.width() > width) && hbarpolicy == Qt::ScrollBarAsNeeded;
   bool useVerticalScrollBar = (viewRect.height() > height) && vbarpolicy == Qt::ScrollBarAsNeeded;

   if (useHorizontalScrollBar && vbarpolicy == Qt::ScrollBarAsNeeded) {
      if (viewRect.height() > height - scrollBarExtent) {
         useVerticalScrollBar = true;
      }
   }

   if (useVerticalScrollBar && hbarpolicy == Qt::ScrollBarAsNeeded) {
      if (viewRect.width() > width - scrollBarExtent) {
         useHorizontalScrollBar = true;
      }
   }

   if (useHorizontalScrollBar) {
      height -= scrollBarExtent;
   }

   if (useVerticalScrollBar) {
      width -= scrollBarExtent;
   }

   // Setting the ranges of these scroll bars can/will cause the values to
   // change, and scrollContentsBy() will be called correspondingly. This
   // will reset the last center point.
   QPointF savedLastCenterPoint = lastCenterPoint;

   // Remember the former indent settings
   qreal oldLeftIndent = leftIndent;
   qreal oldTopIndent = topIndent;

   // If the whole scene fits horizontally, we center the scene horizontally,
   // and ignore the horizontal scroll bars.
   int left =  q_round_bound(viewRect.left());
   int right = q_round_bound(viewRect.right() - width);
   if (left >= right) {
      hbar->setRange(0, 0);

      switch (alignment & Qt::AlignHorizontal_Mask) {
         case Qt::AlignLeft:
            leftIndent = -viewRect.left();
            break;

         case Qt::AlignRight:
            leftIndent = width - viewRect.width() - viewRect.left() - 1;
            break;

         case Qt::AlignHCenter:
         default:
            leftIndent = width / 2 - (viewRect.left() + viewRect.right()) / 2;
            break;
      }

   } else {
      hbar->setRange(left, right);
      hbar->setPageStep(width);
      hbar->setSingleStep(width / 20);
      leftIndent = 0;
   }

   // If the whole scene fits vertically, we center the scene vertically, and
   // ignore the vertical scroll bars.
   int top = q_round_bound(viewRect.top());
   int bottom = q_round_bound(viewRect.bottom()  - height);
   if (top >= bottom) {
      vbar->setRange(0, 0);

      switch (alignment & Qt::AlignVertical_Mask) {
         case Qt::AlignTop:
            topIndent = -viewRect.top();
            break;

         case Qt::AlignBottom:
            topIndent = height - viewRect.height() - viewRect.top() - 1;
            break;

         case Qt::AlignVCenter:
         default:
            topIndent = height / 2 - (viewRect.top() + viewRect.bottom()) / 2;
            break;
      }

   } else {
      vbar->setRange(top, bottom);
      vbar->setPageStep(height);
      vbar->setSingleStep(height / 20);
      topIndent = 0;
   }

   // Restorethe center point from before the ranges changed.
   lastCenterPoint = savedLastCenterPoint;

   // Issue a full update if the indents change.
   // ### If the transform is still the same, we can get away with just a
   // scroll instead.
   if (oldLeftIndent != leftIndent || oldTopIndent != topIndent) {
      dirtyScroll = true;
      updateAll();
   } else if (q->isRightToLeft() && !leftIndent) {
      // In reverse mode, the horizontal scroll always changes after the content
      // size has changed, as the scroll is calculated by summing the min and
      // max values of the range and subtracting the current value. In normal
      // mode the scroll remains unchanged unless the indent has changed.
      dirtyScroll = true;
   }

   if (cacheMode & QGraphicsView::CacheBackground) {
      // Invalidate the background pixmap
      mustResizeBackgroundPixmap = true;
   }
}

void QGraphicsViewPrivate::centerView(QGraphicsView::ViewportAnchor anchor)
{
   Q_Q(QGraphicsView);
   switch (anchor) {
      case QGraphicsView::AnchorUnderMouse: {
         if (q->underMouse()) {
            // Last scene pos: lastMouseMoveScenePoint
            // Current mouse pos:
            QPointF transformationDiff = q->mapToScene(viewport->rect().center())
               - q->mapToScene(viewport->mapFromGlobal(QCursor::pos()));
            q->centerOn(lastMouseMoveScenePoint + transformationDiff);
         } else {
            q->centerOn(lastCenterPoint);
         }
         break;
      }

      case QGraphicsView::AnchorViewCenter:
         q->centerOn(lastCenterPoint);
         break;

      case QGraphicsView::NoAnchor:
         break;
   }
}

void QGraphicsViewPrivate::updateLastCenterPoint()
{
   Q_Q(QGraphicsView);
   lastCenterPoint = q->mapToScene(viewport->rect().center());
}

qint64 QGraphicsViewPrivate::horizontalScroll() const
{
   if (dirtyScroll) {
      const_cast<QGraphicsViewPrivate *>(this)->updateScroll();
   }
   return scrollX;
}

qint64 QGraphicsViewPrivate::verticalScroll() const
{
   if (dirtyScroll) {
      const_cast<QGraphicsViewPrivate *>(this)->updateScroll();
   }
   return scrollY;
}

QRectF QGraphicsViewPrivate::mapRectToScene(const QRect &rect) const
{
   if (dirtyScroll) {
      const_cast<QGraphicsViewPrivate *>(this)->updateScroll();
   }
   QRectF scrolled = QRectF(rect.translated(scrollX, scrollY));
   return identityMatrix ? scrolled : matrix.inverted().mapRect(scrolled);
}

QRectF QGraphicsViewPrivate::mapRectFromScene(const QRectF &rect) const
{
   if (dirtyScroll) {
      const_cast<QGraphicsViewPrivate *>(this)->updateScroll();
   }
   return (identityMatrix ? rect : matrix.mapRect(rect)).translated(-scrollX, -scrollY);
}

void QGraphicsViewPrivate::updateScroll()
{
   Q_Q(QGraphicsView);
   scrollX = qint64(-leftIndent);
   if (q->isRightToLeft()) {
      if (!leftIndent) {
         scrollX += hbar->minimum();
         scrollX += hbar->maximum();
         scrollX -= hbar->value();
      }
   } else {
      scrollX += hbar->value();
   }

   scrollY = qint64(vbar->value() - topIndent);

   dirtyScroll = false;
}

void QGraphicsViewPrivate::replayLastMouseEvent()
{
   if (!useLastMouseEvent || !scene) {
      return;
   }
   mouseMoveEventHandler(&lastMouseEvent);
}

void QGraphicsViewPrivate::storeMouseEvent(QMouseEvent *event)
{
   useLastMouseEvent = true;
   lastMouseEvent = QMouseEvent(QEvent::MouseMove, event->localPos(), event->windowPos(), event->screenPos(),
         event->button(), event->buttons(), event->modifiers());
}

void QGraphicsViewPrivate::mouseMoveEventHandler(QMouseEvent *event)
{
   Q_Q(QGraphicsView);

#ifndef QT_NO_RUBBERBAND
   updateRubberBand(event);
#endif
   storeMouseEvent(event);
   lastMouseEvent.setAccepted(false);

   if (!sceneInteractionAllowed) {
      return;
   }
   if (handScrolling) {
      return;
   }
   if (!scene) {
      return;
   }

   QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
   mouseEvent.setWidget(viewport);
   mouseEvent.setButtonDownScenePos(mousePressButton, mousePressScenePoint);
   mouseEvent.setButtonDownScreenPos(mousePressButton, mousePressScreenPoint);
   mouseEvent.setScenePos(q->mapToScene(event->pos()));
   mouseEvent.setScreenPos(event->globalPos());
   mouseEvent.setLastScenePos(lastMouseMoveScenePoint);
   mouseEvent.setLastScreenPos(lastMouseMoveScreenPoint);
   mouseEvent.setButtons(event->buttons());
   mouseEvent.setButton(event->button());
   mouseEvent.setModifiers(event->modifiers());
   mouseEvent.setSource(event->source());
   mouseEvent.setFlags(event->flags());
   lastMouseMoveScenePoint = mouseEvent.scenePos();
   lastMouseMoveScreenPoint = mouseEvent.screenPos();
   mouseEvent.setAccepted(false);
   if (event->spontaneous()) {
      qt_sendSpontaneousEvent(scene, &mouseEvent);
   } else {
      QApplication::sendEvent(scene, &mouseEvent);
   }

   // Remember whether the last event was accepted or not.
   lastMouseEvent.setAccepted(mouseEvent.isAccepted());

   if (mouseEvent.isAccepted() && mouseEvent.buttons() != 0) {
      // The event was delivered to a mouse grabber; the press is likely to
      // have set a cursor, and we must not change it.
      return;
   }

#ifndef QT_NO_CURSOR
   // If all the items ignore hover events, we don't look-up any items
   // in QGraphicsScenePrivate::dispatchHoverEvent, hence the
   // cachedItemsUnderMouse list will be empty. We therefore do the look-up
   // for cursor items here if not all items use the default cursor.
   if (scene->d_func()->allItemsIgnoreHoverEvents && !scene->d_func()->allItemsUseDefaultCursor
      && scene->d_func()->cachedItemsUnderMouse.isEmpty()) {
      scene->d_func()->cachedItemsUnderMouse = scene->d_func()->itemsAtPosition(mouseEvent.screenPos(),
            mouseEvent.scenePos(),
            mouseEvent.widget());
   }
   // Find the topmost item under the mouse with a cursor.
   for (QGraphicsItem *item : scene->d_func()->cachedItemsUnderMouse) {
      if (item->hasCursor()) {
         _q_setViewportCursor(item->cursor());
         return;
      }
   }

   // No items with cursors found; revert to the view cursor.
   if (hasStoredOriginalCursor) {
      // Restore the original viewport cursor.
      hasStoredOriginalCursor = false;
      viewport->setCursor(originalCursor);
   }
#endif
}

#ifndef QT_NO_RUBBERBAND
QRegion QGraphicsViewPrivate::rubberBandRegion(const QWidget *widget, const QRect &rect) const
{
   QStyleHintReturnMask mask;
   QStyleOptionRubberBand option;
   option.initFrom(widget);
   option.rect = rect;
   option.opaque = false;
   option.shape = QRubberBand::Rectangle;

   QRegion tmp;
   tmp += rect;

   if (widget->style()->styleHint(QStyle::SH_RubberBand_Mask, &option, widget, &mask)) {
      tmp &= mask.region;
   }
   return tmp;
}

void QGraphicsViewPrivate::updateRubberBand(const QMouseEvent *event)
{
   Q_Q(QGraphicsView);
   if (dragMode != QGraphicsView::RubberBandDrag || !sceneInteractionAllowed || !rubberBanding) {
      return;
   }
   // Check for enough drag distance
   if ((mousePressViewPoint - event->pos()).manhattanLength() < QApplication::startDragDistance()) {
      return;
   }

   // Update old rubberband
   if (viewportUpdateMode != QGraphicsView::NoViewportUpdate && !rubberBandRect.isEmpty()) {
      if (viewportUpdateMode != QGraphicsView::FullViewportUpdate) {
         q->viewport()->update(rubberBandRegion(q->viewport(), rubberBandRect));
      } else {
         updateAll();
      }
   }

   // Stop rubber banding if the user has let go of all buttons (even
   // if we didn't get the release events).
   if (!event->buttons()) {
      rubberBanding = false;
      rubberBandSelectionOperation = Qt::ReplaceSelection;
      if (!rubberBandRect.isNull()) {
         rubberBandRect = QRect();
         emit q->rubberBandChanged(rubberBandRect, QPointF(), QPointF());
      }
      return;
   }

   QRect oldRubberband = rubberBandRect;

   // Update rubberband position
   const QPoint mp = q->mapFromScene(mousePressScenePoint);
   const QPoint ep = event->pos();
   rubberBandRect = QRect(qMin(mp.x(), ep.x()), qMin(mp.y(), ep.y()),
         qAbs(mp.x() - ep.x()) + 1, qAbs(mp.y() - ep.y()) + 1);

   if (rubberBandRect != oldRubberband || lastRubberbandScenePoint != lastMouseMoveScenePoint) {
      lastRubberbandScenePoint = lastMouseMoveScenePoint;
      oldRubberband = rubberBandRect;
      emit q->rubberBandChanged(rubberBandRect, mousePressScenePoint, lastRubberbandScenePoint);
   }

   // Update new rubberband
   if (viewportUpdateMode != QGraphicsView::NoViewportUpdate) {
      if (viewportUpdateMode != QGraphicsView::FullViewportUpdate) {
         q->viewport()->update(rubberBandRegion(q->viewport(), rubberBandRect));
      } else {
         updateAll();
      }
   }
   // Set the new selection area
   QPainterPath selectionArea;
   selectionArea.addPolygon(q->mapToScene(rubberBandRect));
   selectionArea.closeSubpath();
   if (scene) {
      scene->setSelectionArea(selectionArea, rubberBandSelectionOperation, rubberBandSelectionMode, q->viewportTransform());
   }
}
#endif

#ifndef QT_NO_CURSOR
void QGraphicsViewPrivate::_q_setViewportCursor(const QCursor &cursor)
{
   if (!hasStoredOriginalCursor) {
      hasStoredOriginalCursor = true;
      originalCursor = viewport->cursor();
   }
   viewport->setCursor(cursor);
}
#endif

#ifndef QT_NO_CURSOR
void QGraphicsViewPrivate::_q_unsetViewportCursor()
{
   Q_Q(QGraphicsView);
   for (QGraphicsItem *item : q->items(lastMouseEvent.pos())) {
      if (item->hasCursor()) {
         _q_setViewportCursor(item->cursor());
         return;
      }
   }

   // Restore the original viewport cursor.
   if (hasStoredOriginalCursor) {
      hasStoredOriginalCursor = false;
      if (dragMode == QGraphicsView::ScrollHandDrag) {
         viewport->setCursor(Qt::OpenHandCursor);
      } else {
         viewport->setCursor(originalCursor);
      }
   }
}
#endif

void QGraphicsViewPrivate::storeDragDropEvent(const QGraphicsSceneDragDropEvent *event)
{
   delete lastDragDropEvent;
   lastDragDropEvent = new QGraphicsSceneDragDropEvent(event->type());
   lastDragDropEvent->setScenePos(event->scenePos());
   lastDragDropEvent->setScreenPos(event->screenPos());
   lastDragDropEvent->setButtons(event->buttons());
   lastDragDropEvent->setModifiers(event->modifiers());
   lastDragDropEvent->setPossibleActions(event->possibleActions());
   lastDragDropEvent->setProposedAction(event->proposedAction());
   lastDragDropEvent->setDropAction(event->dropAction());
   lastDragDropEvent->setMimeData(event->mimeData());
   lastDragDropEvent->setWidget(event->widget());
   lastDragDropEvent->setSource(event->source());
}

void QGraphicsViewPrivate::populateSceneDragDropEvent(QGraphicsSceneDragDropEvent *dest,
   QDropEvent *source)
{
#ifndef QT_NO_DRAGANDDROP
   Q_Q(QGraphicsView);
   dest->setScenePos(q->mapToScene(source->pos()));
   dest->setScreenPos(q->mapToGlobal(source->pos()));
   dest->setButtons(source->mouseButtons());
   dest->setModifiers(source->keyboardModifiers());
   dest->setPossibleActions(source->possibleActions());
   dest->setProposedAction(source->proposedAction());
   dest->setDropAction(source->dropAction());
   dest->setMimeData(source->mimeData());
   dest->setWidget(viewport);
   dest->setSource(qobject_cast<QWidget *>(source->source()));

#endif
}

QRect QGraphicsViewPrivate::mapToViewRect(const QGraphicsItem *item, const QRectF &rect) const
{
   Q_Q(const QGraphicsView);
   if (dirtyScroll) {
      const_cast<QGraphicsViewPrivate *>(this)->updateScroll();
   }

   if (item->d_ptr->itemIsUntransformable()) {
      QTransform itv = item->deviceTransform(q->viewportTransform());
      return itv.mapRect(rect).toAlignedRect();
   }

   // Translate-only
   // COMBINE
   QPointF offset;
   const QGraphicsItem *parentItem = item;
   const QGraphicsItemPrivate *itemd;

   do {
      itemd = parentItem->d_ptr.data();
      if (itemd->transformData) {
         break;
      }

      offset += itemd->pos;

   } while ((parentItem = itemd->parent));

   QRectF baseRect = rect.translated(offset.x(), offset.y());
   if (! parentItem) {
      if (identityMatrix) {
         baseRect.translate(-scrollX, -scrollY);
         return baseRect.toAlignedRect();
      }
      return matrix.mapRect(baseRect).translated(-scrollX, -scrollY).toAlignedRect();
   }

   QTransform tr = parentItem->sceneTransform();
   if (! identityMatrix) {
      tr *= matrix;
   }

   QRectF r = tr.mapRect(baseRect);
   r.translate(-scrollX, -scrollY);

   return r.toAlignedRect();
}

QRegion QGraphicsViewPrivate::mapToViewRegion(const QGraphicsItem *item, const QRectF &rect) const
{
   Q_Q(const QGraphicsView);
   if (dirtyScroll) {
      const_cast<QGraphicsViewPrivate *>(this)->updateScroll();
   }

   // Accurate bounding region
   QTransform itv = item->deviceTransform(q->viewportTransform());
   return item->boundingRegion(itv) & itv.mapRect(rect).toAlignedRect();
}

void QGraphicsViewPrivate::processPendingUpdates()
{
   if (!scene) {
      return;
   }

   if (fullUpdatePending) {
      viewport->update();
   } else if (viewportUpdateMode == QGraphicsView::BoundingRectViewportUpdate) {
      viewport->update(dirtyBoundingRect);
   } else {
      viewport->update(dirtyRegion); // Already adjusted in updateRect/Region.
   }

   dirtyBoundingRect = QRect();
   dirtyRegion = QRegion();
}

static inline bool intersectsViewport(const QRect &r, int width, int height)
{
   return !(r.left() > width) && !(r.right() < 0) && !(r.top() >= height) && !(r.bottom() < 0);
}

static inline bool containsViewport(const QRect &r, int width, int height)
{
   return r.left() <= 0 && r.top() <= 0 && r.right() >= width - 1 && r.bottom() >= height - 1;
}

static inline void QRect_unite(QRect *rect, const QRect &other)
{
   if (rect->isEmpty()) {
      *rect = other;
   } else {
      rect->setCoords(qMin(rect->left(), other.left()), qMin(rect->top(), other.top()),
         qMax(rect->right(), other.right()), qMax(rect->bottom(), other.bottom()));
   }
}

void QGraphicsViewPrivate::setUpdateClip(QGraphicsItem *item)
{
   Q_Q(QGraphicsView);
   // We simply ignore the request if the update mode is either FullViewportUpdate
   // or NoViewportUpdate; in that case there's no point in clipping anything.
   if (!item || viewportUpdateMode == QGraphicsView::NoViewportUpdate
      || viewportUpdateMode == QGraphicsView::FullViewportUpdate) {
      hasUpdateClip = false;
      return;
   }

   // Calculate the clip (item's bounding rect in view coordinates).
   // Optimized version of:
   // QRect clip = item->deviceTransform(q->viewportTransform())
   //              .mapRect(item->boundingRect()).toAlignedRect();
   QRect clip;
   if (item->d_ptr->itemIsUntransformable()) {
      QTransform xform = item->deviceTransform(q->viewportTransform());
      clip = xform.mapRect(item->boundingRect()).toAlignedRect();
   } else if (item->d_ptr->sceneTransformTranslateOnly && identityMatrix) {
      QRectF r(item->boundingRect());
      r.translate(item->d_ptr->sceneTransform.dx() - horizontalScroll(),
         item->d_ptr->sceneTransform.dy() - verticalScroll());
      clip = r.toAlignedRect();
   } else if (!q->isTransformed()) {
      clip = item->d_ptr->sceneTransform.mapRect(item->boundingRect()).toAlignedRect();
   } else {
      QTransform xform = item->d_ptr->sceneTransform;
      xform *= q->viewportTransform();
      clip = xform.mapRect(item->boundingRect()).toAlignedRect();
   }

   if (hasUpdateClip) {
      // Intersect with old clip.
      updateClip &= clip;
   } else {
      updateClip = clip;
      hasUpdateClip = true;
   }
}

bool QGraphicsViewPrivate::updateRegion(const QRectF &rect, const QTransform &xform)
{
   if (rect.isEmpty()) {
      return false;
   }

   if (viewportUpdateMode != QGraphicsView::MinimalViewportUpdate
      && viewportUpdateMode != QGraphicsView::SmartViewportUpdate) {
      // No point in updating with QRegion granularity; use the rect instead.
      return updateRectF(xform.mapRect(rect));
   }

   // Update mode is either Minimal or Smart, so we have to do a potentially slow operation,
   // which is clearly documented here: QGraphicsItem::setBoundingRegionGranularity.
   const QRegion region = xform.map(QRegion(rect.toAlignedRect()));
   QRect viewRect = region.boundingRect();
   const bool dontAdjustForAntialiasing = optimizationFlags & QGraphicsView::DontAdjustForAntialiasing;
   if (dontAdjustForAntialiasing) {
      viewRect.adjust(-1, -1, 1, 1);
   } else {
      viewRect.adjust(-2, -2, 2, 2);
   }
   if (!intersectsViewport(viewRect, viewport->width(), viewport->height())) {
      return false;   // Update region for sure outside viewport.
   }

   const QVector<QRect> &rects = region.rects();
   for (int i = 0; i < rects.size(); ++i) {
      viewRect = rects.at(i);
      if (dontAdjustForAntialiasing) {
         viewRect.adjust(-1, -1, 1, 1);
      } else {
         viewRect.adjust(-2, -2, 2, 2);
      }
      if (hasUpdateClip) {
         viewRect &= updateClip;
      }
      dirtyRegion += viewRect;
   }

   return true;
}

// NB! Assumes the rect 'r' is already aligned and adjusted for antialiasing.
// For QRectF use updateRectF(const QRectF &) to ensure proper adjustments.
bool QGraphicsViewPrivate::updateRect(const QRect &r)
{
   if (fullUpdatePending || viewportUpdateMode == QGraphicsView::NoViewportUpdate
      || !intersectsViewport(r, viewport->width(), viewport->height())) {
      return false;
   }

   switch (viewportUpdateMode) {
      case QGraphicsView::FullViewportUpdate:
         fullUpdatePending = true;
         viewport->update();
         break;

      case QGraphicsView::BoundingRectViewportUpdate:
         if (hasUpdateClip) {
            QRect_unite(&dirtyBoundingRect, r & updateClip);
         } else {
            QRect_unite(&dirtyBoundingRect, r);
         }

         if (containsViewport(dirtyBoundingRect, viewport->width(), viewport->height())) {
            fullUpdatePending = true;
            viewport->update();
         }
         break;

      case QGraphicsView::SmartViewportUpdate:       // ### DEPRECATE
      case QGraphicsView::MinimalViewportUpdate:
         if (hasUpdateClip) {
            dirtyRegion += r & updateClip;
         } else {
            dirtyRegion += r;
         }
         break;

      default:
         break;
   }

   return true;
}

QStyleOptionGraphicsItem *QGraphicsViewPrivate::allocStyleOptionsArray(int numItems)
{
   if (mustAllocateStyleOptions || (numItems > styleOptions.capacity()))
      // too many items, let's allocate on-the-fly
   {
      return new QStyleOptionGraphicsItem[numItems];
   }

   // expand only whenever necessary
   if (numItems > styleOptions.size()) {
      styleOptions.resize(numItems);
   }

   mustAllocateStyleOptions = true;
   return styleOptions.data();
}

void QGraphicsViewPrivate::freeStyleOptionsArray(QStyleOptionGraphicsItem *array)
{
   mustAllocateStyleOptions = false;
   if (array != styleOptions.data()) {
      delete [] array;
   }
}

extern QPainterPath qt_regionToPath(const QRegion &region);

/*!
    ### Adjustments in findItems: mapToScene(QRect) forces us to adjust the
    input rectangle by (0, 0, 1, 1), because it uses QRect::bottomRight()
    (etc) when mapping the rectangle to a polygon (which is _wrong_). In
    addition, as QGraphicsItem::boundingRect() is defined in logical space,
    but the default pen for QPainter is cosmetic with a width of 0, QPainter
    is at risk of painting 1 pixel outside the bounding rect. Therefore we
    must search for items with an adjustment of (-1, -1, 1, 1).
*/
QList<QGraphicsItem *> QGraphicsViewPrivate::findItems(const QRegion &exposedRegion, bool *allItems,
   const QTransform &viewTransform) const
{
   Q_Q(const QGraphicsView);

   // Step 1) If all items are contained within the expose region, then
   // return a list of all visible items. ### the scene's growing bounding
   // rect does not take into account untransformable items.
   const QRectF exposedRegionSceneBounds =
         q->mapToScene(exposedRegion.boundingRect().adjusted(-1, -1, 1, 1)).boundingRect();

   if (exposedRegionSceneBounds.contains(scene->sceneRect())) {
      Q_ASSERT(allItems);
      *allItems = true;

      // All items are guaranteed within the exposed region.
      return scene->items(Qt::AscendingOrder);
   }

   // Step 2) If the expose region is a simple rect and the view is only
   // translated or scaled, search for items using
   // QGraphicsScene::items(QRectF).
   bool simpleRectLookup =  exposedRegion.rectCount() == 1 && matrix.type() <= QTransform::TxScale;

   if (simpleRectLookup) {
      return scene->items(exposedRegionSceneBounds, Qt::IntersectsItemBoundingRect, Qt::AscendingOrder, viewTransform);
   }

   // If the region is complex or the view has a complex transform, adjust
   // the expose region, convert it to a path, and then search for items
   // using QGraphicsScene::items(QPainterPath);

   QRegion adjustedRegion;

   for (const QRect &r : exposedRegion.rects()) {
      adjustedRegion += r.adjusted(-1, -1, 1, 1);
   }

   const QPainterPath exposedScenePath(q->mapToScene(qt_regionToPath(adjustedRegion)));
   return scene->items(exposedScenePath, Qt::IntersectsItemBoundingRect,
         Qt::AscendingOrder, viewTransform);
}

void QGraphicsViewPrivate::updateInputMethodSensitivity()
{
   Q_Q(QGraphicsView);

   QGraphicsItem *focusItem = nullptr;

   bool enabled = scene && (focusItem = scene->focusItem())
      && (focusItem->d_ptr->itemFlags & QGraphicsItem::ItemAcceptsInputMethod);

   q->setAttribute(Qt::WA_InputMethodEnabled, enabled);
   q->viewport()->setAttribute(Qt::WA_InputMethodEnabled, enabled);

   if (! enabled) {
      q->setInputMethodHints(Qt::EmptyFlag);
      return;
   }

   QGraphicsProxyWidget *proxy = focusItem->d_ptr->isWidget && focusItem->d_ptr->isProxyWidget()
            ? static_cast<QGraphicsProxyWidget *>(focusItem) : nullptr;

   if (! proxy) {
      q->setInputMethodHints(focusItem->inputMethodHints());

   } else if (QWidget *widget = proxy->widget()) {
      if (QWidget *fw = widget->focusWidget()) {
         widget = fw;
      }
      q->setInputMethodHints(widget->inputMethodHints());
   } else {
      q->setInputMethodHints(Qt::EmptyFlag);
   }
}

QGraphicsView::QGraphicsView(QWidget *parent)
   : QAbstractScrollArea(*new QGraphicsViewPrivate, parent)
{
   setViewport(nullptr);
   setAcceptDrops(true);
   setBackgroundRole(QPalette::Base);
   // Investigate leaving these disabled by default.
   setAttribute(Qt::WA_InputMethodEnabled);
   viewport()->setAttribute(Qt::WA_InputMethodEnabled);
}

QGraphicsView::QGraphicsView(QGraphicsScene *scene, QWidget *parent)
   : QAbstractScrollArea(*new QGraphicsViewPrivate, parent)
{
   setScene(scene);
   setViewport(nullptr);
   setAcceptDrops(true);
   setBackgroundRole(QPalette::Base);
   // Investigate leaving these disabled by default.
   setAttribute(Qt::WA_InputMethodEnabled);
   viewport()->setAttribute(Qt::WA_InputMethodEnabled);
}

// internal
QGraphicsView::QGraphicsView(QGraphicsViewPrivate &dd, QWidget *parent)
   : QAbstractScrollArea(dd, parent)
{
   setViewport(nullptr);
   setAcceptDrops(true);
   setBackgroundRole(QPalette::Base);
   // Investigate leaving these disabled by default.
   setAttribute(Qt::WA_InputMethodEnabled);
   viewport()->setAttribute(Qt::WA_InputMethodEnabled);
}

QGraphicsView::~QGraphicsView()
{
   Q_D(QGraphicsView);
   if (d->scene) {
      d->scene->d_func()->views.removeAll(this);
   }

   delete d->lastDragDropEvent;
}

QSize QGraphicsView::sizeHint() const
{
   Q_D(const QGraphicsView);
   if (d->scene) {
      QSizeF baseSize = d->matrix.mapRect(sceneRect()).size();
      baseSize += QSizeF(d->frameWidth * 2, d->frameWidth * 2);
      return baseSize.boundedTo((3 * QApplication::desktop()->size()) / 4).toSize();
   }

   return QAbstractScrollArea::sizeHint();
}

QPainter::RenderHints QGraphicsView::renderHints() const
{
   Q_D(const QGraphicsView);
   return d->renderHints;
}

void QGraphicsView::setRenderHints(QPainter::RenderHints hints)
{
   Q_D(QGraphicsView);
   if (hints == d->renderHints) {
      return;
   }

   d->renderHints = hints;
   d->updateAll();
}

void QGraphicsView::setRenderHint(QPainter::RenderHint hint, bool enabled)
{
   Q_D(QGraphicsView);
   QPainter::RenderHints oldHints = d->renderHints;

   if (enabled) {
      d->renderHints |= hint;
   } else {
      d->renderHints &= ~hint;
   }

   if (oldHints != d->renderHints) {
      d->updateAll();
   }
}

Qt::Alignment QGraphicsView::alignment() const
{
   Q_D(const QGraphicsView);
   return d->alignment;
}

void QGraphicsView::setAlignment(Qt::Alignment alignment)
{
   Q_D(QGraphicsView);
   if (d->alignment != alignment) {
      d->alignment = alignment;
      d->recalculateContentSize();
   }
}

QGraphicsView::ViewportAnchor QGraphicsView::transformationAnchor() const
{
   Q_D(const QGraphicsView);
   return d->transformationAnchor;
}

void QGraphicsView::setTransformationAnchor(ViewportAnchor anchor)
{
   Q_D(QGraphicsView);
   d->transformationAnchor = anchor;

   // Ensure mouse tracking is enabled in the case we are using AnchorUnderMouse
   // in order to have up-to-date information for centering the view.
   if (d->transformationAnchor == AnchorUnderMouse) {
      d->viewport->setMouseTracking(true);
   }
}

QGraphicsView::ViewportAnchor QGraphicsView::resizeAnchor() const
{
   Q_D(const QGraphicsView);
   return d->resizeAnchor;
}

void QGraphicsView::setResizeAnchor(ViewportAnchor anchor)
{
   Q_D(QGraphicsView);
   d->resizeAnchor = anchor;

   // Ensure mouse tracking is enabled in the case we are using AnchorUnderMouse
   // in order to have up-to-date information for centering the view.
   if (d->resizeAnchor == AnchorUnderMouse) {
      d->viewport->setMouseTracking(true);
   }
}

QGraphicsView::ViewportUpdateMode QGraphicsView::viewportUpdateMode() const
{
   Q_D(const QGraphicsView);
   return d->viewportUpdateMode;
}
void QGraphicsView::setViewportUpdateMode(ViewportUpdateMode mode)
{
   Q_D(QGraphicsView);
   d->viewportUpdateMode = mode;
}

QGraphicsView::OptimizationFlags QGraphicsView::optimizationFlags() const
{
   Q_D(const QGraphicsView);
   return d->optimizationFlags;
}

void QGraphicsView::setOptimizationFlags(OptimizationFlags flags)
{
   Q_D(QGraphicsView);
   d->optimizationFlags = flags;
}

void QGraphicsView::setOptimizationFlag(OptimizationFlag flag, bool enabled)
{
   Q_D(QGraphicsView);
   if (enabled) {
      d->optimizationFlags |= flag;
   } else {
      d->optimizationFlags &= ~flag;
   }
}

QGraphicsView::DragMode QGraphicsView::dragMode() const
{
   Q_D(const QGraphicsView);
   return d->dragMode;
}

void QGraphicsView::setDragMode(DragMode mode)
{
   Q_D(QGraphicsView);
   if (d->dragMode == mode) {
      return;
   }

#ifndef QT_NO_CURSOR
   if (d->dragMode == ScrollHandDrag) {
      viewport()->unsetCursor();
   }
#endif

   // If dragMode is unset while dragging, e.g. via a keyEvent, we
   // don't unset the handScrolling state. When enabling scrolling
   // again the mouseMoveEvent will automatically start scrolling,
   // without a mousePress
   if (d->dragMode == ScrollHandDrag && mode == NoDrag && d->handScrolling) {
      d->handScrolling = false;
   }

   d->dragMode = mode;

#ifndef QT_NO_CURSOR
   if (d->dragMode == ScrollHandDrag) {
      // Forget the stored viewport cursor when we enter scroll hand drag mode.
      d->hasStoredOriginalCursor = false;
      viewport()->setCursor(Qt::OpenHandCursor);
   }
#endif
}

#ifndef QT_NO_RUBBERBAND

Qt::ItemSelectionMode QGraphicsView::rubberBandSelectionMode() const
{
   Q_D(const QGraphicsView);
   return d->rubberBandSelectionMode;
}

void QGraphicsView::setRubberBandSelectionMode(Qt::ItemSelectionMode mode)
{
   Q_D(QGraphicsView);
   d->rubberBandSelectionMode = mode;
}

QRect QGraphicsView::rubberBandRect() const
{
   Q_D(const QGraphicsView);
   if (d->dragMode != QGraphicsView::RubberBandDrag || !d->sceneInteractionAllowed || !d->rubberBanding) {
      return QRect();
   }
   return d->rubberBandRect;
}
#endif

QGraphicsView::CacheMode QGraphicsView::cacheMode() const
{
   Q_D(const QGraphicsView);
   return d->cacheMode;
}

void QGraphicsView::setCacheMode(CacheMode mode)
{
   Q_D(QGraphicsView);
   if (mode == d->cacheMode) {
      return;
   }
   d->cacheMode = mode;
   resetCachedContent();
}

void QGraphicsView::resetCachedContent()
{
   Q_D(QGraphicsView);
   if (d->cacheMode == CacheNone) {
      return;
   }

   if (d->cacheMode & CacheBackground) {
      // Background caching is enabled.
      d->mustResizeBackgroundPixmap = true;
      d->updateAll();
   } else if (d->mustResizeBackgroundPixmap) {
      // Background caching is disabled.
      // Cleanup, free some resources.
      d->mustResizeBackgroundPixmap = false;
      d->backgroundPixmap = QPixmap();
      d->backgroundPixmapExposed = QRegion();
   }
}

void QGraphicsView::invalidateScene(const QRectF &rect, QGraphicsScene::SceneLayers layers)
{
   Q_D(QGraphicsView);

   if ((layers & QGraphicsScene::BackgroundLayer) && !d->mustResizeBackgroundPixmap) {
      QRect viewRect = mapFromScene(rect).boundingRect();
      if (viewport()->rect().intersects(viewRect)) {
         // The updated background area is exposed; schedule this area for
         // redrawing.
         d->backgroundPixmapExposed += viewRect;
         if (d->scene) {
            d->scene->update(rect);
         }
      }
   }
}

bool QGraphicsView::isInteractive() const
{
   Q_D(const QGraphicsView);
   return d->sceneInteractionAllowed;
}

void QGraphicsView::setInteractive(bool allowed)
{
   Q_D(QGraphicsView);
   d->sceneInteractionAllowed = allowed;
}

QGraphicsScene *QGraphicsView::scene() const
{
   Q_D(const QGraphicsView);
   return d->scene;
}

void QGraphicsView::setScene(QGraphicsScene *scene)
{
   Q_D(QGraphicsView);
   if (d->scene == scene) {
      return;
   }

   // Always update the viewport when the scene changes.
   d->updateAll();

   // Remove the previously assigned scene.
   if (d->scene) {
      disconnect(d->scene.data(), &QGraphicsScene::changed,          this, &QGraphicsView::updateScene);
      disconnect(d->scene.data(), &QGraphicsScene::sceneRectChanged, this, &QGraphicsView::updateSceneRect);

      d->scene->d_func()->removeView(this);
      d->connectedToScene = false;

      if (isActiveWindow() && isVisible()) {
         QEvent windowDeactivate(QEvent::WindowDeactivate);
         QApplication::sendEvent(d->scene, &windowDeactivate);
      }
      if (hasFocus()) {
         d->scene->clearFocus();
      }
   }

   // Assign the new scene and update the contents (scrollbars, etc.)).
   if ((d->scene = scene)) {
      connect(d->scene.data(), &QGraphicsScene::sceneRectChanged, this, &QGraphicsView::updateSceneRect);

      d->updateSceneSlotReimplementedChecked = false;
      d->scene->d_func()->addView(this);
      d->recalculateContentSize();
      d->lastCenterPoint = sceneRect().center();
      d->keepLastCenterPoint = true;

      // We are only interested in mouse tracking if items accept
      // hover events or use non-default cursors.
      if (! d->scene->d_func()->allItemsIgnoreHoverEvents || ! d->scene->d_func()->allItemsUseDefaultCursor) {
         d->viewport->setMouseTracking(true);
      }

      // enable touch events if any items is interested in them
      if (! d->scene->d_func()->allItemsIgnoreTouchEvents) {
         d->viewport->setAttribute(Qt::WA_AcceptTouchEvents);
      }

      if (isActiveWindow() && isVisible()) {
         QEvent windowActivate(QEvent::WindowActivate);
         QApplication::sendEvent(d->scene, &windowActivate);
      }

   } else {
      d->recalculateContentSize();
   }

   d->updateInputMethodSensitivity();

   if (d->scene && hasFocus()) {
      d->scene->setFocus();
   }
}

QRectF QGraphicsView::sceneRect() const
{
   Q_D(const QGraphicsView);

   if (d->hasSceneRect) {
      return d->sceneRect;
   }

   if (d->scene) {
      return d->scene->sceneRect();
   }

   return QRectF();
}

void QGraphicsView::setSceneRect(const QRectF &rect)
{
   Q_D(QGraphicsView);
   d->hasSceneRect = !rect.isNull();
   d->sceneRect = rect;
   d->recalculateContentSize();
}

QMatrix QGraphicsView::matrix() const
{
   Q_D(const QGraphicsView);
   return d->matrix.toAffine();
}

void QGraphicsView::setMatrix(const QMatrix &matrix, bool combine)
{
   setTransform(QTransform(matrix), combine);
}

void QGraphicsView::resetMatrix()
{
   resetTransform();
}

void QGraphicsView::rotate(qreal angle)
{
   Q_D(QGraphicsView);
   QTransform matrix = d->matrix;
   matrix.rotate(angle);
   setTransform(matrix);
}

void QGraphicsView::scale(qreal sx, qreal sy)
{
   Q_D(QGraphicsView);
   QTransform matrix = d->matrix;
   matrix.scale(sx, sy);
   setTransform(matrix);
}

void QGraphicsView::shear(qreal sh, qreal sv)
{
   Q_D(QGraphicsView);
   QTransform matrix = d->matrix;
   matrix.shear(sh, sv);
   setTransform(matrix);
}

void QGraphicsView::translate(qreal dx, qreal dy)
{
   Q_D(QGraphicsView);
   QTransform matrix = d->matrix;
   matrix.translate(dx, dy);
   setTransform(matrix);
}

void QGraphicsView::centerOn(const QPointF &pos)
{
   Q_D(QGraphicsView);
   qreal width = viewport()->width();
   qreal height = viewport()->height();
   QPointF viewPoint = d->matrix.map(pos);
   QPointF oldCenterPoint = pos;

   if (!d->leftIndent) {
      if (isRightToLeft()) {
         qint64 horizontal = 0;
         horizontal += horizontalScrollBar()->minimum();
         horizontal += horizontalScrollBar()->maximum();
         horizontal -= int(viewPoint.x() - width / qreal(2.0));
         horizontalScrollBar()->setValue(horizontal);
      } else {
         horizontalScrollBar()->setValue(int(viewPoint.x() - width / qreal(2.0)));
      }
   }
   if (!d->topIndent) {
      verticalScrollBar()->setValue(int(viewPoint.y() - height / qreal(2.0)));
   }
   d->lastCenterPoint = oldCenterPoint;
}

void QGraphicsView::centerOn(const QGraphicsItem *item)
{
   centerOn(item->sceneBoundingRect().center());
}

void QGraphicsView::ensureVisible(const QRectF &rect, int xmargin, int ymargin)
{
   Q_D(QGraphicsView);
   qreal width = viewport()->width();
   qreal height = viewport()->height();
   QRectF viewRect = d->matrix.mapRect(rect);

   qreal left = d->horizontalScroll();
   qreal right = left + width;
   qreal top = d->verticalScroll();
   qreal bottom = top + height;

   if (viewRect.left() <= left + xmargin) {
      // need to scroll from the left
      if (!d->leftIndent) {
         horizontalScrollBar()->setValue(int(viewRect.left() - xmargin - qreal(0.5)));
      }
   }
   if (viewRect.right() >= right - xmargin) {
      // need to scroll from the right
      if (!d->leftIndent) {
         horizontalScrollBar()->setValue(int(viewRect.right() - width + xmargin + qreal(0.5)));
      }
   }
   if (viewRect.top() <= top + ymargin) {
      // need to scroll from the top
      if (!d->topIndent) {
         verticalScrollBar()->setValue(int(viewRect.top() - ymargin - qreal(0.5)));
      }
   }
   if (viewRect.bottom() >= bottom - ymargin) {
      // need to scroll from the bottom
      if (!d->topIndent) {
         verticalScrollBar()->setValue(int(viewRect.bottom() - height + ymargin + qreal(0.5)));
      }
   }
}

void QGraphicsView::ensureVisible(const QGraphicsItem *item, int xmargin, int ymargin)
{
   ensureVisible(item->sceneBoundingRect(), xmargin, ymargin);
}

void QGraphicsView::fitInView(const QRectF &rect, Qt::AspectRatioMode aspectRatioMode)
{
   Q_D(QGraphicsView);
   if (!d->scene || rect.isNull()) {
      return;
   }

   // Reset the view scale to 1:1.
   QRectF unity = d->matrix.mapRect(QRectF(0, 0, 1, 1));
   if (unity.isEmpty()) {
      return;
   }
   scale(1 / unity.width(), 1 / unity.height());

   // Find the ideal x / y scaling ratio to fit \a rect in the view.
   int margin = 2;
   QRectF viewRect = viewport()->rect().adjusted(margin, margin, -margin, -margin);
   if (viewRect.isEmpty()) {
      return;
   }
   QRectF sceneRect = d->matrix.mapRect(rect);
   if (sceneRect.isEmpty()) {
      return;
   }
   qreal xratio = viewRect.width() / sceneRect.width();
   qreal yratio = viewRect.height() / sceneRect.height();

   // Respect the aspect ratio mode.
   switch (aspectRatioMode) {
      case Qt::KeepAspectRatio:
         xratio = yratio = qMin(xratio, yratio);
         break;

      case Qt::KeepAspectRatioByExpanding:
         xratio = yratio = qMax(xratio, yratio);
         break;

      case Qt::IgnoreAspectRatio:
         break;
   }

   // Scale and center on the center of \a rect.
   scale(xratio, yratio);
   centerOn(rect.center());
}

void QGraphicsView::fitInView(const QGraphicsItem *item, Qt::AspectRatioMode aspectRatioMode)
{
   QPainterPath path = item->isClipped() ? item->clipPath() : item->shape();
   if (item->d_ptr->hasTranslateOnlySceneTransform()) {
      path.translate(item->d_ptr->sceneTransform.dx(), item->d_ptr->sceneTransform.dy());
      fitInView(path.boundingRect(), aspectRatioMode);
   } else {
      fitInView(item->d_ptr->sceneTransform.map(path).boundingRect(), aspectRatioMode);
   }
}

void QGraphicsView::render(QPainter *painter, const QRectF &target, const QRect &source,
   Qt::AspectRatioMode aspectRatioMode)
{
   // ### Switch to using the recursive rendering algorithm instead.

   Q_D(QGraphicsView);
   if (!d->scene || !(painter && painter->isActive())) {
      return;
   }

   // Default source rect = viewport rect
   QRect sourceRect = source;
   if (source.isNull()) {
      sourceRect = viewport()->rect();
   }

   // Default target rect = device rect
   QRectF targetRect = target;
   if (target.isNull()) {
      if (painter->device()->devType() == QInternal::Picture) {
         targetRect = sourceRect;
      } else {
         targetRect.setRect(0, 0, painter->device()->width(), painter->device()->height());
      }
   }

   // Find the ideal x / y scaling ratio to fit \a source into \a target.
   qreal xratio = targetRect.width() / sourceRect.width();
   qreal yratio = targetRect.height() / sourceRect.height();

   // Scale according to the aspect ratio mode.
   switch (aspectRatioMode) {
      case Qt::KeepAspectRatio:
         xratio = yratio = qMin(xratio, yratio);
         break;

      case Qt::KeepAspectRatioByExpanding:
         xratio = yratio = qMax(xratio, yratio);
         break;

      case Qt::IgnoreAspectRatio:
         break;
   }

   // Find all items to draw, and reverse the list (we want to draw
   // in reverse order).
   QPolygonF sourceScenePoly = mapToScene(sourceRect.adjusted(-1, -1, 1, 1));
   QList<QGraphicsItem *> itemList = d->scene->items(sourceScenePoly,
         Qt::IntersectsItemBoundingRect);

   QGraphicsItem **itemArray = new QGraphicsItem *[itemList.size()];
   int numItems = itemList.size();

   for (int i = 0; i < numItems; ++i) {
      itemArray[numItems - i - 1] = itemList.at(i);
   }
   itemList.clear();

   // Setup painter matrix.
   QTransform moveMatrix = QTransform::fromTranslate(-d->horizontalScroll(), -d->verticalScroll());
   QTransform painterMatrix = d->matrix * moveMatrix;

   painterMatrix *= QTransform()
      .translate(targetRect.left(), targetRect.top())
      .scale(xratio, yratio)
      .translate(-sourceRect.left(), -sourceRect.top());

   // Generate the style options
   QStyleOptionGraphicsItem *styleOptionArray = d->allocStyleOptionsArray(numItems);
   for (int i = 0; i < numItems; ++i) {
      itemArray[i]->d_ptr->initStyleOption(&styleOptionArray[i], painterMatrix, targetRect.toRect());
   }

   painter->save();

   // Clip in device coordinates to avoid QRegion transformations.
   painter->setClipRect(targetRect);
   QPainterPath path;
   path.addPolygon(sourceScenePoly);
   path.closeSubpath();
   painter->setClipPath(painterMatrix.map(path), Qt::IntersectClip);

   // Transform the painter.
   painter->setTransform(painterMatrix, true);

   // Render the scene.
   QRectF sourceSceneRect = sourceScenePoly.boundingRect();
   drawBackground(painter, sourceSceneRect);
   drawItems(painter, numItems, itemArray, styleOptionArray);
   drawForeground(painter, sourceSceneRect);

   delete [] itemArray;
   d->freeStyleOptionsArray(styleOptionArray);

   painter->restore();
}

QList<QGraphicsItem *> QGraphicsView::items() const
{
   Q_D(const QGraphicsView);
   if (! d->scene) {
      return QList<QGraphicsItem *>();
   }
   return d->scene->items();
}

QList<QGraphicsItem *> QGraphicsView::items(const QPoint &pos) const
{
   Q_D(const QGraphicsView);
   if (!d->scene) {
      return QList<QGraphicsItem *>();
   }
   // ### Unify these two, and use the items(QPointF) version in
   // QGraphicsScene instead. The scene items function could use the viewport
   // transform to map the point to a rect/polygon.
   if ((d->identityMatrix || d->matrix.type() <= QTransform::TxScale)) {
      // Use the rect version
      QTransform xinv = viewportTransform().inverted();
      return d->scene->items(xinv.mapRect(QRectF(pos.x(), pos.y(), 1, 1)),
            Qt::IntersectsItemShape,
            Qt::DescendingOrder,
            viewportTransform());
   }
   // Use the polygon version
   return d->scene->items(mapToScene(pos.x(), pos.y(), 1, 1),
         Qt::IntersectsItemShape,
         Qt::DescendingOrder,
         viewportTransform());
}

QList<QGraphicsItem *> QGraphicsView::items(const QRect &rect, Qt::ItemSelectionMode mode) const
{
   Q_D(const QGraphicsView);
   if (!d->scene) {
      return QList<QGraphicsItem *>();
   }
   return d->scene->items(mapToScene(rect), mode, Qt::DescendingOrder, viewportTransform());
}

QList<QGraphicsItem *> QGraphicsView::items(const QPolygon &polygon, Qt::ItemSelectionMode mode) const
{
   Q_D(const QGraphicsView);
   if (!d->scene) {
      return QList<QGraphicsItem *>();
   }
   return d->scene->items(mapToScene(polygon), mode, Qt::DescendingOrder, viewportTransform());
}

QList<QGraphicsItem *> QGraphicsView::items(const QPainterPath &path, Qt::ItemSelectionMode mode) const
{
   Q_D(const QGraphicsView);
   if (!d->scene) {
      return QList<QGraphicsItem *>();
   }
   return d->scene->items(mapToScene(path), mode, Qt::DescendingOrder, viewportTransform());
}

QGraphicsItem *QGraphicsView::itemAt(const QPoint &pos) const
{
   Q_D(const QGraphicsView);
   if (!d->scene) {
      return nullptr;
   }

   QList<QGraphicsItem *> itemsAtPos = items(pos);
   return itemsAtPos.isEmpty() ? nullptr : itemsAtPos.first();
}

QPointF QGraphicsView::mapToScene(const QPoint &point) const
{
   Q_D(const QGraphicsView);
   QPointF p = point;
   p.rx() += d->horizontalScroll();
   p.ry() += d->verticalScroll();
   return d->identityMatrix ? p : d->matrix.inverted().map(p);
}

QPolygonF QGraphicsView::mapToScene(const QRect &rect) const
{
   Q_D(const QGraphicsView);
   if (!rect.isValid()) {
      return QPolygonF();
   }

   QPointF scrollOffset(d->horizontalScroll(), d->verticalScroll());
   QRect r = rect.adjusted(0, 0, 1, 1);
   QPointF tl = scrollOffset + r.topLeft();
   QPointF tr = scrollOffset + r.topRight();
   QPointF br = scrollOffset + r.bottomRight();
   QPointF bl = scrollOffset + r.bottomLeft();

   QPolygonF poly(4);
   if (!d->identityMatrix) {
      QTransform x = d->matrix.inverted();
      poly[0] = x.map(tl);
      poly[1] = x.map(tr);
      poly[2] = x.map(br);
      poly[3] = x.map(bl);
   } else {
      poly[0] = tl;
      poly[1] = tr;
      poly[2] = br;
      poly[3] = bl;
   }
   return poly;
}

QPolygonF QGraphicsView::mapToScene(const QPolygon &polygon) const
{
   QPolygonF poly;

   for (const QPoint &point : polygon) {
      poly << mapToScene(point);
   }
   return poly;
}

QPainterPath QGraphicsView::mapToScene(const QPainterPath &path) const
{
   Q_D(const QGraphicsView);
   QTransform matrix = QTransform::fromTranslate(d->horizontalScroll(), d->verticalScroll());
   matrix *= d->matrix.inverted();
   return matrix.map(path);
}

QPoint QGraphicsView::mapFromScene(const QPointF &point) const
{
   Q_D(const QGraphicsView);
   QPointF p = d->identityMatrix ? point : d->matrix.map(point);
   p.rx() -= d->horizontalScroll();
   p.ry() -= d->verticalScroll();
   return p.toPoint();
}

QPolygon QGraphicsView::mapFromScene(const QRectF &rect) const
{
   Q_D(const QGraphicsView);
   QPointF tl;
   QPointF tr;
   QPointF br;
   QPointF bl;
   if (!d->identityMatrix) {
      const QTransform &x = d->matrix;
      tl = x.map(rect.topLeft());
      tr = x.map(rect.topRight());
      br = x.map(rect.bottomRight());
      bl = x.map(rect.bottomLeft());
   } else {
      tl = rect.topLeft();
      tr = rect.topRight();
      br = rect.bottomRight();
      bl = rect.bottomLeft();
   }
   QPointF scrollOffset(d->horizontalScroll(), d->verticalScroll());
   tl -= scrollOffset;
   tr -= scrollOffset;
   br -= scrollOffset;
   bl -= scrollOffset;

   QPolygon poly(4);
   poly[0] = tl.toPoint();
   poly[1] = tr.toPoint();
   poly[2] = br.toPoint();
   poly[3] = bl.toPoint();
   return poly;
}

QPolygon QGraphicsView::mapFromScene(const QPolygonF &polygon) const
{
   QPolygon poly;
   for (const QPointF &point : polygon) {
      poly << mapFromScene(point);
   }
   return poly;
}

QPainterPath QGraphicsView::mapFromScene(const QPainterPath &path) const
{
   Q_D(const QGraphicsView);
   QTransform matrix = d->matrix;
   matrix *= QTransform::fromTranslate(-d->horizontalScroll(), -d->verticalScroll());
   return matrix.map(path);
}

QVariant QGraphicsView::inputMethodQuery(Qt::InputMethodQuery query) const
{
   Q_D(const QGraphicsView);
   if (!d->scene) {
      return QVariant();
   }

   QVariant value = d->scene->inputMethodQuery(query);
   if (value.type() == QVariant::RectF) {
      value = d->mapRectFromScene(value.toRectF());
   } else if (value.type() == QVariant::PointF) {
      value = mapFromScene(value.toPointF());
   } else if (value.type() == QVariant::Rect) {
      value = d->mapRectFromScene(value.toRect()).toRect();
   } else if (value.type() == QVariant::Point) {
      value = mapFromScene(value.toPoint());
   }
   return value;
}

QBrush QGraphicsView::backgroundBrush() const
{
   Q_D(const QGraphicsView);
   return d->backgroundBrush;
}
void QGraphicsView::setBackgroundBrush(const QBrush &brush)
{
   Q_D(QGraphicsView);
   d->backgroundBrush = brush;
   d->updateAll();

   if (d->cacheMode & CacheBackground) {
      // Invalidate the background pixmap
      d->mustResizeBackgroundPixmap = true;
   }
}

QBrush QGraphicsView::foregroundBrush() const
{
   Q_D(const QGraphicsView);
   return d->foregroundBrush;
}
void QGraphicsView::setForegroundBrush(const QBrush &brush)
{
   Q_D(QGraphicsView);
   d->foregroundBrush = brush;
   d->updateAll();
}

void QGraphicsView::updateScene(const QList<QRectF> &rects)
{
   // this slot is only called if the user explicitly
   // establishes a connection between the scene and the view, as the scene
   // and view are no longer connected. We need to keep it working (basically
   // leave it as it is), but the new delivery path is through QGraphicsScenePrivate::itemUpdate().
   Q_D(QGraphicsView);
   if (d->fullUpdatePending || d->viewportUpdateMode == QGraphicsView::NoViewportUpdate) {
      return;
   }

   // Extract and reset dirty scene rect info.
   QVector<QRect> dirtyViewportRects;
   const QVector<QRect> &dirtyRects = d->dirtyRegion.rects();
   for (int i = 0; i < dirtyRects.size(); ++i) {
      dirtyViewportRects += dirtyRects.at(i);
   }

   d->dirtyRegion = QRegion();
   d->dirtyBoundingRect = QRect();

   bool fullUpdate = !d->accelerateScrolling || d->viewportUpdateMode == QGraphicsView::FullViewportUpdate;
   bool boundingRectUpdate = (d->viewportUpdateMode == QGraphicsView::BoundingRectViewportUpdate)
      || (d->viewportUpdateMode == QGraphicsView::SmartViewportUpdate
         && ((dirtyViewportRects.size() + rects.size()) >= QGRAPHICSVIEW_REGION_RECT_THRESHOLD));

   QRegion updateRegion;
   QRect boundingRect;
   QRect viewportRect = viewport()->rect();
   bool redraw = false;
   QTransform transform = viewportTransform();

   // Convert scene rects to viewport rects.
   for (const QRectF &rect : rects) {
      QRect xrect = transform.mapRect(rect).toAlignedRect();
      if (!(d->optimizationFlags & DontAdjustForAntialiasing)) {
         xrect.adjust(-2, -2, 2, 2);
      } else {
         xrect.adjust(-1, -1, 1, 1);
      }
      if (!viewportRect.intersects(xrect)) {
         continue;
      }
      dirtyViewportRects << xrect;
   }

   for (const QRect &rect : dirtyViewportRects) {
      // Add the exposed rect to the update region. In rect update
      // mode, we only count the bounding rect of items.
      if (! boundingRectUpdate) {
         updateRegion += rect;
      } else {
         boundingRect |= rect;
      }

      redraw = true;
      if (fullUpdate) {
         // If fullUpdate is true and we found a visible dirty rect,
         // we're done.
         break;
      }
   }

   if (! redraw) {
      return;
   }

   if (fullUpdate) {
      viewport()->update();

   } else if (boundingRectUpdate) {
      viewport()->update(boundingRect);

   } else {
      viewport()->update(updateRegion);
   }
}

void QGraphicsView::updateSceneRect(const QRectF &rect)
{
   Q_D(QGraphicsView);
   if (!d->hasSceneRect) {
      d->sceneRect = rect;
      d->recalculateContentSize();
   }
}

void QGraphicsView::setupViewport(QWidget *widget)
{
   Q_D(QGraphicsView);

   if (!widget) {
      qWarning("QGraphicsView::setupViewport() Unable to initialize an invalid widget (nullptr)");
      return;
   }

   const bool isGLWidget = widget->inherits("QGLWidget") || widget->inherits("QOpenGLWidget");

   d->accelerateScrolling = !(isGLWidget);

   widget->setFocusPolicy(Qt::StrongFocus);

   if (!isGLWidget) {
      // autoFillBackground enables scroll acceleration.
      widget->setAutoFillBackground(true);
   }

   // We are only interested in mouse tracking if items
   // accept hover events or use non-default cursors or if
   // AnchorUnderMouse is used as transformation or resize anchor.
   if ((d->scene && (!d->scene->d_func()->allItemsIgnoreHoverEvents
            || !d->scene->d_func()->allItemsUseDefaultCursor))
      || d->transformationAnchor == AnchorUnderMouse
      || d->resizeAnchor == AnchorUnderMouse) {
      widget->setMouseTracking(true);
   }

   // enable touch events if any items is interested in them
   if (d->scene && !d->scene->d_func()->allItemsIgnoreTouchEvents) {
      widget->setAttribute(Qt::WA_AcceptTouchEvents);
   }

#ifndef QT_NO_GESTURES
   if (d->scene) {
      for (Qt::GestureType gesture : d->scene->d_func()->grabbedGestures.keys()) {
         widget->grabGesture(gesture);
      }
   }
#endif

   widget->setAcceptDrops(acceptDrops());
}

bool QGraphicsView::event(QEvent *event)
{
   Q_D(QGraphicsView);

   if (d->sceneInteractionAllowed) {
      switch (event->type()) {
         case QEvent::ShortcutOverride:
            if (d->scene) {
               return QApplication::sendEvent(d->scene, event);
            }
            break;

         case QEvent::KeyPress:
            if (d->scene) {
               QKeyEvent *k = static_cast<QKeyEvent *>(event);
               if (k->key() == Qt::Key_Tab || k->key() == Qt::Key_Backtab) {
                  // Send the key events to the scene. This will invoke the
                  // scene's tab focus handling, and if the event is
                  // accepted, we return (prevent further event delivery),
                  // and the base implementation will call QGraphicsView's
                  // focusNextPrevChild() function. If the event is ignored,
                  // we fall back to standard tab focus handling.

                  QApplication::sendEvent(d->scene, event);
                  if (event->isAccepted()) {
                     return true;
                  }

                  // Ensure the event doesn't propagate just because the
                  // scene ignored it. If the event propagates, then tab
                  // handling will be called twice (this and parent).
                  event->accept();
               }
            }
            break;

         default:
            break;
      }
   }

   return QAbstractScrollArea::event(event);
}

bool QGraphicsView::viewportEvent(QEvent *event)
{
   Q_D(QGraphicsView);

   if (! d->scene) {
      return QAbstractScrollArea::viewportEvent(event);
   }

   switch (event->type()) {
      case QEvent::Enter:
         QApplication::sendEvent(d->scene, event);
         break;

      case QEvent::WindowActivate:
         QApplication::sendEvent(d->scene, event);
         break;

      case QEvent::WindowDeactivate:
         // ### This is a temporary fix until we have proper mouse grab events
         // mouseGrabberItem should be set to 0 if we lose he mouse grab.
         // Remove all popups when the scene loses focus.

         if (! d->scene->d_func()->popupWidgets.isEmpty()) {
            d->scene->d_func()->removePopup(d->scene->d_func()->popupWidgets.first());
         }
         QApplication::sendEvent(d->scene, event);
         break;

      case QEvent::Show:
         if (d->scene && isActiveWindow()) {
            QEvent windowActivate(QEvent::WindowActivate);
            QApplication::sendEvent(d->scene, &windowActivate);
         }
         break;

      case QEvent::Hide:
         // spontaneous event will generate a WindowDeactivate.
         if (!event->spontaneous() && d->scene && isActiveWindow()) {
            QEvent windowDeactivate(QEvent::WindowDeactivate);
            QApplication::sendEvent(d->scene, &windowDeactivate);
         }
         break;

      case QEvent::Leave: {
         // ### This is a temporary fix until we have proper mouse grab events
         //  activeMouseGrabberItem should be set to 0 if we lose the mouse grab.

         if ((QApplication::activePopupWidget() && QApplication::activePopupWidget() != window())
               || (QApplication::activeModalWidget() && QApplication::activeModalWidget() != window())
               || (QApplication::activeWindow() != window())) {

            if (! d->scene->d_func()->popupWidgets.isEmpty()) {
               d->scene->d_func()->removePopup(d->scene->d_func()->popupWidgets.first());
            }
         }

         d->useLastMouseEvent = false;
         // a hack to pass a viewport pointer to the scene inside the leave event
         Q_ASSERT(event->d == nullptr);

         QScopedValueRollback<QEventPrivate *> rb(event->d);
         event->d = reinterpret_cast<QEventPrivate *>(viewport());
         QApplication::sendEvent(d->scene, event);

         break;
      }

#ifndef QT_NO_TOOLTIP
      case QEvent::ToolTip: {
         QHelpEvent *toolTip = static_cast<QHelpEvent *>(event);
         QGraphicsSceneHelpEvent helpEvent(QEvent::GraphicsSceneHelp);
         helpEvent.setWidget(viewport());
         helpEvent.setScreenPos(toolTip->globalPos());
         helpEvent.setScenePos(mapToScene(toolTip->pos()));
         QApplication::sendEvent(d->scene, &helpEvent);
         toolTip->setAccepted(helpEvent.isAccepted());
         return true;
      }
#endif

      case QEvent::Paint:
         // Reset full update
         d->fullUpdatePending = false;
         d->dirtyScrollOffset = QPoint();

         if (d->scene) {
            // Check if this view reimplements the updateScene slot. If it does we can not
            // direct update delivery and have to fall back to connecting the changed signal.

            if (! d->updateSceneSlotReimplementedChecked) {
               d->updateSceneSlotReimplementedChecked = true;
               const QMetaObject *mo = metaObject();

               if (mo != &QGraphicsView::staticMetaObject()) {

                  if (mo->indexOfSlot("updateScene(const QList<QRectF> &)") !=
                        QGraphicsView::staticMetaObject().indexOfSlot("updateScene(const QList<QRectF> &)")) {

                     connect(d->scene.data(), &QGraphicsScene::changed, this, &QGraphicsView::updateScene);
                  }
               }
            }
         }

         break;

      case QEvent::TouchBegin:
      case QEvent::TouchUpdate:
      case QEvent::TouchEnd: {
         if (! isEnabled()) {
            return false;
         }

         if (d->scene && d->sceneInteractionAllowed) {
            // Convert and deliver the touch event to the scene.
            QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
            touchEvent->setTarget(viewport());

            QGraphicsViewPrivate::translateTouchEvent(d, touchEvent);
            (void) QApplication::sendEvent(d->scene, touchEvent);

         } else {
            event->ignore();
         }

         return true;
      }
#ifndef QT_NO_GESTURES
      case QEvent::Gesture:
      case QEvent::GestureOverride: {
         if (! isEnabled()) {
            return false;
         }

         if (d->scene && d->sceneInteractionAllowed) {
            QGestureEvent *gestureEvent = static_cast<QGestureEvent *>(event);
            gestureEvent->setWidget(viewport());
            (void) QApplication::sendEvent(d->scene, gestureEvent);
         }
         return true;
      }
#endif

      default:
         break;
   }

   return QAbstractScrollArea::viewportEvent(event);
}

#ifndef QT_NO_CONTEXTMENU
void QGraphicsView::contextMenuEvent(QContextMenuEvent *event)
{
   Q_D(QGraphicsView);
   if (!d->scene || !d->sceneInteractionAllowed) {
      return;
   }

   d->mousePressViewPoint = event->pos();
   d->mousePressScenePoint = mapToScene(d->mousePressViewPoint);
   d->mousePressScreenPoint = event->globalPos();
   d->lastMouseMoveScenePoint = d->mousePressScenePoint;
   d->lastMouseMoveScreenPoint = d->mousePressScreenPoint;

   QGraphicsSceneContextMenuEvent contextEvent(QEvent::GraphicsSceneContextMenu);
   contextEvent.setWidget(viewport());
   contextEvent.setScenePos(d->mousePressScenePoint);
   contextEvent.setScreenPos(d->mousePressScreenPoint);
   contextEvent.setModifiers(event->modifiers());
   contextEvent.setReason((QGraphicsSceneContextMenuEvent::Reason)(event->reason()));
   contextEvent.setAccepted(event->isAccepted());
   QApplication::sendEvent(d->scene, &contextEvent);
   event->setAccepted(contextEvent.isAccepted());
}
#endif


void QGraphicsView::dropEvent(QDropEvent *event)
{
#ifndef QT_NO_DRAGANDDROP
   Q_D(QGraphicsView);

   if (!d->scene || !d->sceneInteractionAllowed) {
      return;
   }

   // Generate a scene event.
   QGraphicsSceneDragDropEvent sceneEvent(QEvent::GraphicsSceneDrop);
   d->populateSceneDragDropEvent(&sceneEvent, event);

   // Send it to the scene.
   QApplication::sendEvent(d->scene, &sceneEvent);

   // Accept the originating event if the scene accepted the scene event.
   event->setAccepted(sceneEvent.isAccepted());
   if (sceneEvent.isAccepted()) {
      event->setDropAction(sceneEvent.dropAction());
   }

   delete d->lastDragDropEvent;
   d->lastDragDropEvent = nullptr;

#else
   (void) event;
#endif
}

void QGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
#ifndef QT_NO_DRAGANDDROP
   Q_D(QGraphicsView);
   if (!d->scene || !d->sceneInteractionAllowed) {
      return;
   }

   // Disable replaying of mouse move events.
   d->useLastMouseEvent = false;

   // Generate a scene event.
   QGraphicsSceneDragDropEvent sceneEvent(QEvent::GraphicsSceneDragEnter);
   d->populateSceneDragDropEvent(&sceneEvent, event);

   // Store it for later use.
   d->storeDragDropEvent(&sceneEvent);

   // Send it to the scene.
   QApplication::sendEvent(d->scene, &sceneEvent);

   // Accept the originating event if the scene accepted the scene event.
   if (sceneEvent.isAccepted()) {
      event->setAccepted(true);
      event->setDropAction(sceneEvent.dropAction());
   }
#endif
}

void QGraphicsView::dragLeaveEvent(QDragLeaveEvent *event)
{
#ifndef QT_NO_DRAGANDDROP
   Q_D(QGraphicsView);
   if (!d->scene || !d->sceneInteractionAllowed) {
      return;
   }
   if (!d->lastDragDropEvent) {
      qWarning("QGraphicsView::dragLeaveEvent() Drag leave event was received before a drag enter event");
      return;
   }

   // Generate a scene event.
   QGraphicsSceneDragDropEvent sceneEvent(QEvent::GraphicsSceneDragLeave);
   sceneEvent.setScenePos(d->lastDragDropEvent->scenePos());
   sceneEvent.setScreenPos(d->lastDragDropEvent->screenPos());
   sceneEvent.setButtons(d->lastDragDropEvent->buttons());
   sceneEvent.setModifiers(d->lastDragDropEvent->modifiers());
   sceneEvent.setPossibleActions(d->lastDragDropEvent->possibleActions());
   sceneEvent.setProposedAction(d->lastDragDropEvent->proposedAction());
   sceneEvent.setDropAction(d->lastDragDropEvent->dropAction());
   sceneEvent.setMimeData(d->lastDragDropEvent->mimeData());
   sceneEvent.setWidget(d->lastDragDropEvent->widget());
   sceneEvent.setSource(d->lastDragDropEvent->source());
   delete d->lastDragDropEvent;
   d->lastDragDropEvent = nullptr;

   // Send it to the scene.
   QApplication::sendEvent(d->scene, &sceneEvent);

   // Accept the originating event if the scene accepted the scene event.
   if (sceneEvent.isAccepted()) {
      event->setAccepted(true);
   }
#else
   (void) event;
#endif
}

void QGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
#ifndef QT_NO_DRAGANDDROP
   Q_D(QGraphicsView);
   if (!d->scene || !d->sceneInteractionAllowed) {
      return;
   }

   // Generate a scene event.
   QGraphicsSceneDragDropEvent sceneEvent(QEvent::GraphicsSceneDragMove);
   d->populateSceneDragDropEvent(&sceneEvent, event);

   // Store it for later use.
   d->storeDragDropEvent(&sceneEvent);

   // Send it to the scene.
   QApplication::sendEvent(d->scene, &sceneEvent);

   // Ignore the originating event if the scene ignored the scene event.
   event->setAccepted(sceneEvent.isAccepted());
   if (sceneEvent.isAccepted()) {
      event->setDropAction(sceneEvent.dropAction());
   }

#endif
}
void QGraphicsView::focusInEvent(QFocusEvent *event)
{
   Q_D(QGraphicsView);
   d->updateInputMethodSensitivity();
   QAbstractScrollArea::focusInEvent(event);
   if (d->scene) {
      QApplication::sendEvent(d->scene, event);
   }
   // Pass focus on if the scene cannot accept focus.
   if (!d->scene || !event->isAccepted()) {
      QAbstractScrollArea::focusInEvent(event);
   }
}

bool QGraphicsView::focusNextPrevChild(bool next)
{
   return QAbstractScrollArea::focusNextPrevChild(next);
}

void QGraphicsView::focusOutEvent(QFocusEvent *event)
{
   Q_D(QGraphicsView);
   QAbstractScrollArea::focusOutEvent(event);

   if (d->scene) {
      QApplication::sendEvent(d->scene, event);
   }
}

void QGraphicsView::keyPressEvent(QKeyEvent *event)
{
   Q_D(QGraphicsView);
   if (!d->scene || !d->sceneInteractionAllowed) {
      QAbstractScrollArea::keyPressEvent(event);
      return;
   }
   QApplication::sendEvent(d->scene, event);
   if (!event->isAccepted()) {
      QAbstractScrollArea::keyPressEvent(event);
   }
}

void QGraphicsView::keyReleaseEvent(QKeyEvent *event)
{
   Q_D(QGraphicsView);
   if (!d->scene || !d->sceneInteractionAllowed) {
      return;
   }
   QApplication::sendEvent(d->scene, event);
   if (!event->isAccepted()) {
      QAbstractScrollArea::keyReleaseEvent(event);
   }
}

void QGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
   Q_D(QGraphicsView);
   if (!d->scene || !d->sceneInteractionAllowed) {
      return;
   }

   d->storeMouseEvent(event);
   d->mousePressViewPoint = event->pos();
   d->mousePressScenePoint = mapToScene(d->mousePressViewPoint);
   d->mousePressScreenPoint = event->globalPos();
   d->lastMouseMoveScenePoint = d->mousePressScenePoint;
   d->lastMouseMoveScreenPoint = d->mousePressScreenPoint;
   d->mousePressButton = event->button();

   QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseDoubleClick);
   mouseEvent.setWidget(viewport());
   mouseEvent.setButtonDownScenePos(d->mousePressButton, d->mousePressScenePoint);
   mouseEvent.setButtonDownScreenPos(d->mousePressButton, d->mousePressScreenPoint);
   mouseEvent.setScenePos(mapToScene(d->mousePressViewPoint));
   mouseEvent.setScreenPos(d->mousePressScreenPoint);
   mouseEvent.setLastScenePos(d->lastMouseMoveScenePoint);
   mouseEvent.setLastScreenPos(d->lastMouseMoveScreenPoint);
   mouseEvent.setButtons(event->buttons());

   mouseEvent.setAccepted(false);
   mouseEvent.setButton(event->button());
   mouseEvent.setModifiers(event->modifiers());
   mouseEvent.setSource(event->source());
   mouseEvent.setFlags(event->flags());

   if (event->spontaneous()) {
      qt_sendSpontaneousEvent(d->scene, &mouseEvent);
   } else {
      QApplication::sendEvent(d->scene, &mouseEvent);
   }
   const bool isAccepted = mouseEvent.isAccepted();
   event->setAccepted(isAccepted);
   // Update the last mouse event accepted state.
   d->lastMouseEvent.setAccepted(isAccepted);
}

void QGraphicsView::mousePressEvent(QMouseEvent *event)
{
   Q_D(QGraphicsView);

   // Store this event for replaying, finding deltas, and for
   // scroll-dragging; even in non-interactive mode, scroll hand dragging is
   // allowed, so we store the event at the very top of this function.
   d->storeMouseEvent(event);
   d->lastMouseEvent.setAccepted(false);

   if (d->sceneInteractionAllowed) {
      // Store some of the event's button-down data.
      d->mousePressViewPoint = event->pos();
      d->mousePressScenePoint = mapToScene(d->mousePressViewPoint);
      d->mousePressScreenPoint = event->globalPos();
      d->lastMouseMoveScenePoint = d->mousePressScenePoint;
      d->lastMouseMoveScreenPoint = d->mousePressScreenPoint;
      d->mousePressButton = event->button();

      if (d->scene) {
         // Convert and deliver the mouse event to the scene.
         QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
         mouseEvent.setWidget(viewport());
         mouseEvent.setButtonDownScenePos(d->mousePressButton, d->mousePressScenePoint);
         mouseEvent.setButtonDownScreenPos(d->mousePressButton, d->mousePressScreenPoint);
         mouseEvent.setScenePos(d->mousePressScenePoint);
         mouseEvent.setScreenPos(d->mousePressScreenPoint);
         mouseEvent.setLastScenePos(d->lastMouseMoveScenePoint);
         mouseEvent.setLastScreenPos(d->lastMouseMoveScreenPoint);
         mouseEvent.setButtons(event->buttons());
         mouseEvent.setButton(event->button());
         mouseEvent.setModifiers(event->modifiers());
         mouseEvent.setSource(event->source());
         mouseEvent.setFlags(event->flags());
         mouseEvent.setAccepted(false);

         if (event->spontaneous()) {
            qt_sendSpontaneousEvent(d->scene, &mouseEvent);
         } else {
            QApplication::sendEvent(d->scene, &mouseEvent);
         }

         // Update the original mouse event accepted state.
         bool isAccepted = mouseEvent.isAccepted();
         event->setAccepted(isAccepted);

         // Update the last mouse event accepted state.
         d->lastMouseEvent.setAccepted(isAccepted);

         if (isAccepted) {
            return;
         }
      }
   }

#ifndef QT_NO_RUBBERBAND
   if (d->dragMode == QGraphicsView::RubberBandDrag && !d->rubberBanding) {
      if (d->sceneInteractionAllowed) {
         // Rubberbanding is only allowed in interactive mode.
         event->accept();
         d->rubberBanding = true;
         d->rubberBandRect = QRect();
         if (d->scene) {
            bool extendSelection = (event->modifiers() & Qt::ControlModifier) != 0;
            // Initiating a rubber band always clears the selection.
            if (extendSelection) {
               d->rubberBandSelectionOperation = Qt::AddToSelection;
            } else {
               d->rubberBandSelectionOperation = Qt::ReplaceSelection;
               d->scene->clearSelection();
            }
         }
      }
   } else
#endif
      if (d->dragMode == QGraphicsView::ScrollHandDrag && event->button() == Qt::LeftButton) {
         // Left-button press in scroll hand mode initiates hand scrolling.
         event->accept();
         d->handScrolling = true;
         d->handScrollMotions = 0;
#ifndef QT_NO_CURSOR
         viewport()->setCursor(Qt::ClosedHandCursor);
#endif
      }
}

void QGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
   Q_D(QGraphicsView);

   if (d->dragMode == QGraphicsView::ScrollHandDrag) {
      if (d->handScrolling) {
         QScrollBar *hBar = horizontalScrollBar();
         QScrollBar *vBar = verticalScrollBar();
         QPoint delta = event->pos() - d->lastMouseEvent.pos();
         hBar->setValue(hBar->value() + (isRightToLeft() ? delta.x() : -delta.x()));
         vBar->setValue(vBar->value() - delta.y());

         // Detect how much we've scrolled to disambiguate scrolling from
         // clicking.
         ++d->handScrollMotions;
      }
   }

   d->mouseMoveEventHandler(event);
}

void QGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
   Q_D(QGraphicsView);

#ifndef QT_NO_RUBBERBAND
   if (d->dragMode == QGraphicsView::RubberBandDrag && d->sceneInteractionAllowed && !event->buttons()) {
      if (d->rubberBanding) {
         if (d->viewportUpdateMode != QGraphicsView::NoViewportUpdate) {
            if (d->viewportUpdateMode != FullViewportUpdate) {
               viewport()->update(d->rubberBandRegion(viewport(), d->rubberBandRect));
            } else {
               d->updateAll();
            }
         }
         d->rubberBanding = false;
         d->rubberBandSelectionOperation = Qt::ReplaceSelection;
         if (!d->rubberBandRect.isNull()) {
            d->rubberBandRect = QRect();
            emit rubberBandChanged(d->rubberBandRect, QPointF(), QPointF());
         }
      }
   } else
#endif

      if (d->dragMode == QGraphicsView::ScrollHandDrag && event->button() == Qt::LeftButton) {
#ifndef QT_NO_CURSOR
         // Restore the open hand cursor. ### There might be items
         // under the mouse that have a valid cursor at this time, so
         // we could repeat the steps from mouseMoveEvent().
         viewport()->setCursor(Qt::OpenHandCursor);
#endif
         d->handScrolling = false;

         if (d->scene && d->sceneInteractionAllowed && !d->lastMouseEvent.isAccepted() && d->handScrollMotions <= 6) {
            // If we've detected very little motion during the hand drag, and
            // no item accepted the last event, we'll interpret that as a
            // click to the scene, and reset the selection.
            d->scene->clearSelection();
         }
      }

   d->storeMouseEvent(event);

   if (!d->sceneInteractionAllowed) {
      return;
   }

   if (!d->scene) {
      return;
   }

   QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
   mouseEvent.setWidget(viewport());
   mouseEvent.setButtonDownScenePos(d->mousePressButton, d->mousePressScenePoint);
   mouseEvent.setButtonDownScreenPos(d->mousePressButton, d->mousePressScreenPoint);
   mouseEvent.setScenePos(mapToScene(event->pos()));
   mouseEvent.setScreenPos(event->globalPos());
   mouseEvent.setLastScenePos(d->lastMouseMoveScenePoint);
   mouseEvent.setLastScreenPos(d->lastMouseMoveScreenPoint);
   mouseEvent.setButtons(event->buttons());
   mouseEvent.setButton(event->button());
   mouseEvent.setModifiers(event->modifiers());
   mouseEvent.setSource(event->source());
   mouseEvent.setFlags(event->flags());
   mouseEvent.setAccepted(false);
   if (event->spontaneous()) {
      qt_sendSpontaneousEvent(d->scene, &mouseEvent);
   } else {
      QApplication::sendEvent(d->scene, &mouseEvent);
   }

   // Update the last mouse event selected state.
   d->lastMouseEvent.setAccepted(mouseEvent.isAccepted());

#ifndef QT_NO_CURSOR
   if (mouseEvent.isAccepted() && mouseEvent.buttons() == 0 && viewport()->testAttribute(Qt::WA_SetCursor)) {
      // The last mouse release on the viewport will trigger clearing the cursor.
      d->_q_unsetViewportCursor();
   }
#endif
}

#ifndef QT_NO_WHEELEVENT
void QGraphicsView::wheelEvent(QWheelEvent *event)
{
   Q_D(QGraphicsView);
   if (!d->scene || !d->sceneInteractionAllowed) {
      QAbstractScrollArea::wheelEvent(event);
      return;
   }

   event->ignore();

   QGraphicsSceneWheelEvent wheelEvent(QEvent::GraphicsSceneWheel);
   wheelEvent.setWidget(viewport());
   wheelEvent.setScenePos(mapToScene(event->pos()));
   wheelEvent.setScreenPos(event->globalPos());
   wheelEvent.setButtons(event->buttons());
   wheelEvent.setModifiers(event->modifiers());
   wheelEvent.setDelta(event->delta());
   wheelEvent.setOrientation(event->orientation());
   wheelEvent.setAccepted(false);

   QApplication::sendEvent(d->scene, &wheelEvent);
   event->setAccepted(wheelEvent.isAccepted());

   if (! event->isAccepted()) {
      QAbstractScrollArea::wheelEvent(event);
   }
}
#endif

void QGraphicsView::paintEvent(QPaintEvent *event)
{
   Q_D(QGraphicsView);
   if (!d->scene) {
      QAbstractScrollArea::paintEvent(event);
      return;
   }

   // Set up painter state protection.
   d->scene->d_func()->painterStateProtection = !(d->optimizationFlags & DontSavePainterState);

   // Determine the exposed region
   d->exposedRegion = event->region();
   QRectF exposedSceneRect = mapToScene(d->exposedRegion.boundingRect()).boundingRect();

   // Set up the painter
   QPainter painter(viewport());

#ifndef QT_NO_RUBBERBAND
   if (d->rubberBanding && !d->rubberBandRect.isEmpty()) {
      painter.save();
   }
#endif

   // Set up render hints
   painter.setRenderHints(painter.renderHints(), false);
   painter.setRenderHints(d->renderHints, true);

   // Set up viewport transform
   const bool viewTransformed = isTransformed();
   if (viewTransformed) {
      painter.setWorldTransform(viewportTransform());
   }
   const QTransform viewTransform = painter.worldTransform();

   // Draw background
   if ((d->cacheMode & CacheBackground)

   ) {
      // Recreate the background pixmap, and flag the whole background as
      // exposed.
      if (d->mustResizeBackgroundPixmap) {
         d->backgroundPixmap = QPixmap(viewport()->size());
         QBrush bgBrush = viewport()->palette().brush(viewport()->backgroundRole());
         if (!bgBrush.isOpaque()) {
            d->backgroundPixmap.fill(Qt::transparent);
         }
         QPainter p(&d->backgroundPixmap);
         p.fillRect(0, 0, d->backgroundPixmap.width(), d->backgroundPixmap.height(), bgBrush);
         d->backgroundPixmapExposed = QRegion(viewport()->rect());
         d->mustResizeBackgroundPixmap = false;
      }

      // Redraw exposed areas
      if (!d->backgroundPixmapExposed.isEmpty()) {
         QPainter backgroundPainter(&d->backgroundPixmap);
         backgroundPainter.setClipRegion(d->backgroundPixmapExposed, Qt::ReplaceClip);
         if (viewTransformed) {
            backgroundPainter.setTransform(viewTransform);
         }
         QRectF backgroundExposedSceneRect = mapToScene(d->backgroundPixmapExposed.boundingRect()).boundingRect();
         drawBackground(&backgroundPainter, backgroundExposedSceneRect);
         d->backgroundPixmapExposed = QRegion();
      }

      // Blit the background from the background pixmap
      if (viewTransformed) {
         painter.setWorldTransform(QTransform());
         painter.drawPixmap(QPoint(), d->backgroundPixmap);
         painter.setWorldTransform(viewTransform);
      } else {
         painter.drawPixmap(QPoint(), d->backgroundPixmap);
      }

   } else {
      if (!(d->optimizationFlags & DontSavePainterState)) {
         painter.save();
      }
      drawBackground(&painter, exposedSceneRect);
      if (!(d->optimizationFlags & DontSavePainterState)) {
         painter.restore();
      }
   }

   // Items
   if (!(d->optimizationFlags & IndirectPainting)) {
      const quint32 oldRectAdjust = d->scene->d_func()->rectAdjust;
      if (d->optimizationFlags & QGraphicsView::DontAdjustForAntialiasing) {
         d->scene->d_func()->rectAdjust = 1;
      } else {
         d->scene->d_func()->rectAdjust = 2;
      }
      d->scene->d_func()->drawItems(&painter, viewTransformed ? &viewTransform : nullptr,
         &d->exposedRegion, viewport());
      d->scene->d_func()->rectAdjust = oldRectAdjust;
      // Make sure the painter's world transform is restored correctly when
      // drawing without painter state protection (DontSavePainterState).
      // We only change the worldTransform() so there's no need to do a full-blown
      // save() and restore(). Also note that we don't have to do this in case of
      // IndirectPainting (the else branch), because in that case we always save()
      // and restore() in QGraphicsScene::drawItems().
      if (!d->scene->d_func()->painterStateProtection) {
         painter.setOpacity(1.0);
      }
      painter.setWorldTransform(viewTransform);

   } else {
      // Make sure we don't have unpolished items before we draw
      if (!d->scene->d_func()->unpolishedItems.isEmpty()) {
         d->scene->d_func()->_q_polishItems();
      }
      // We reset updateAll here (after we've issued polish events)
      // so that we can discard update requests coming from polishEvent().
      d->scene->d_func()->updateAll = false;

      // Find all exposed items
      bool allItems = false;
      QList<QGraphicsItem *> itemList = d->findItems(d->exposedRegion, &allItems, viewTransform);

      if (!itemList.isEmpty()) {
         // Generate the style options.
         const int numItems = itemList.size();
         QGraphicsItem **itemArray = &itemList[0]; // Relies on QList internals, but is perfectly valid.
         QStyleOptionGraphicsItem *styleOptionArray = d->allocStyleOptionsArray(numItems);
         QTransform transform(Qt::NoData);

         for (int i = 0; i < numItems; ++i) {
            QGraphicsItem *item = itemArray[i];
            QGraphicsItemPrivate *itemd = item->d_ptr.data();
            itemd->initStyleOption(&styleOptionArray[i], viewTransform, d->exposedRegion, allItems);
            // Cache the item's area in view coordinates.
            // Note that we have to do this here in case the base class implementation
            // (QGraphicsScene::drawItems) is not called. If it is, we'll do this
            // operation twice, but that's the price one has to pay for using indirect
            // painting :-/.
            const QRectF brect = adjustedItemEffectiveBoundingRect(item);
            if (!itemd->itemIsUntransformable()) {
               transform = item->sceneTransform();
               if (viewTransformed) {
                  transform *= viewTransform;
               }
            } else {
               transform = item->deviceTransform(viewTransform);
            }
            itemd->paintedViewBoundingRects.insert(d->viewport, transform.mapRect(brect).toRect());
         }
         // Draw the items.
         drawItems(&painter, numItems, itemArray, styleOptionArray);
         d->freeStyleOptionsArray(styleOptionArray);
      }
   }

   // Foreground
   drawForeground(&painter, exposedSceneRect);

#ifndef QT_NO_RUBBERBAND
   // Rubberband
   if (d->rubberBanding && !d->rubberBandRect.isEmpty()) {
      painter.restore();
      QStyleOptionRubberBand option;
      option.initFrom(viewport());
      option.rect = d->rubberBandRect;
      option.shape = QRubberBand::Rectangle;

      QStyleHintReturnMask mask;
      if (viewport()->style()->styleHint(QStyle::SH_RubberBand_Mask, &option, viewport(), &mask)) {
         // painter clipping for masked rubberbands
         painter.setClipRegion(mask.region, Qt::IntersectClip);
      }

      viewport()->style()->drawControl(QStyle::CE_RubberBand, &option, &painter, viewport());
   }
#endif

   painter.end();

   // Restore painter state protection.
   d->scene->d_func()->painterStateProtection = true;
}

void QGraphicsView::resizeEvent(QResizeEvent *event)
{
   Q_D(QGraphicsView);
   // Save the last center point - the resize may scroll the view, which
   // changes the center point.
   QPointF oldLastCenterPoint = d->lastCenterPoint;

   QAbstractScrollArea::resizeEvent(event);
   d->recalculateContentSize();

   // Restore the center point again.
   if (d->resizeAnchor == NoAnchor && !d->keepLastCenterPoint) {
      d->updateLastCenterPoint();
   } else {
      d->lastCenterPoint = oldLastCenterPoint;
   }
   d->centerView(d->resizeAnchor);
   d->keepLastCenterPoint = false;

   if (d->cacheMode & CacheBackground) {
      // Invalidate the background pixmap
      d->mustResizeBackgroundPixmap = true;
   }
}

void QGraphicsView::scrollContentsBy(int dx, int dy)
{
   Q_D(QGraphicsView);
   d->dirtyScroll = true;
   if (d->transforming) {
      return;
   }
   if (isRightToLeft()) {
      dx = -dx;
   }

   if (d->viewportUpdateMode != QGraphicsView::NoViewportUpdate) {
      if (d->viewportUpdateMode != QGraphicsView::FullViewportUpdate) {
         if (d->accelerateScrolling) {
#ifndef QT_NO_RUBBERBAND
            // Update new and old rubberband regions
            if (!d->rubberBandRect.isEmpty()) {
               QRegion rubberBandRegion(d->rubberBandRegion(viewport(), d->rubberBandRect));
               rubberBandRegion += rubberBandRegion.translated(-dx, -dy);
               viewport()->update(rubberBandRegion);
            }
#endif
            d->dirtyScrollOffset.rx() += dx;
            d->dirtyScrollOffset.ry() += dy;
            d->dirtyRegion.translate(dx, dy);
            viewport()->scroll(dx, dy);
         } else {
            d->updateAll();
         }
      } else {
         d->updateAll();
      }
   }

   d->updateLastCenterPoint();

   if ((d->cacheMode & CacheBackground)

   ) {
      // Scroll the background pixmap
      QRegion exposed;
      if (!d->backgroundPixmap.isNull()) {
         d->backgroundPixmap.scroll(dx, dy, d->backgroundPixmap.rect(), &exposed);
      }

      // Invalidate the background pixmap
      d->backgroundPixmapExposed.translate(dx, dy);
      d->backgroundPixmapExposed += exposed;
   }

   // Always replay on scroll.
   if (d->sceneInteractionAllowed) {
      d->replayLastMouseEvent();
   }
}

void QGraphicsView::showEvent(QShowEvent *event)
{
   Q_D(QGraphicsView);
   d->recalculateContentSize();
   d->centerView(d->transformationAnchor);
   QAbstractScrollArea::showEvent(event);
}

void QGraphicsView::inputMethodEvent(QInputMethodEvent *event)
{
   Q_D(QGraphicsView);
   if (d->scene) {
      QApplication::sendEvent(d->scene, event);
   }
}

void QGraphicsView::drawBackground(QPainter *painter, const QRectF &rect)
{
   Q_D(QGraphicsView);
   if (d->scene && d->backgroundBrush.style() == Qt::NoBrush) {
      d->scene->drawBackground(painter, rect);
      return;
   }

   painter->fillRect(rect, d->backgroundBrush);
}

void QGraphicsView::drawForeground(QPainter *painter, const QRectF &rect)
{
   Q_D(QGraphicsView);
   if (d->scene && d->foregroundBrush.style() == Qt::NoBrush) {
      d->scene->drawForeground(painter, rect);
      return;
   }

   painter->fillRect(rect, d->foregroundBrush);
}

// obsolete
void QGraphicsView::drawItems(QPainter *painter, int numItems,
   QGraphicsItem *items[],
   const QStyleOptionGraphicsItem options[])
{
   Q_D(QGraphicsView);
   if (d->scene) {
      QWidget *widget = painter->device() == viewport() ? viewport() : nullptr;
      d->scene->drawItems(painter, numItems, items, options, widget);
   }
}

QTransform QGraphicsView::transform() const
{
   Q_D(const QGraphicsView);
   return d->matrix;
}

QTransform QGraphicsView::viewportTransform() const
{
   Q_D(const QGraphicsView);
   QTransform moveMatrix = QTransform::fromTranslate(-d->horizontalScroll(), -d->verticalScroll());
   return d->identityMatrix ? moveMatrix : d->matrix * moveMatrix;
}

bool QGraphicsView::isTransformed() const
{
   Q_D(const QGraphicsView);
   return !d->identityMatrix || d->horizontalScroll() || d->verticalScroll();
}

void QGraphicsView::setTransform(const QTransform &matrix, bool combine )
{
   Q_D(QGraphicsView);

   QTransform oldMatrix = d->matrix;
   if (!combine) {
      d->matrix = matrix;
   } else {
      d->matrix = matrix * d->matrix;
   }
   if (oldMatrix == d->matrix) {
      return;
   }

   d->identityMatrix = d->matrix.isIdentity();
   d->transforming = true;
   if (d->scene) {
      d->recalculateContentSize();
      d->centerView(d->transformationAnchor);
   } else {
      d->updateLastCenterPoint();
   }

   if (d->sceneInteractionAllowed) {
      d->replayLastMouseEvent();
   }
   d->transforming = false;

   // Any matrix operation requires a full update.
   d->updateAll();
}

void QGraphicsView::resetTransform()
{
   setTransform(QTransform());
}

QPointF QGraphicsViewPrivate::mapToScene(const QPointF &point) const
{
   QPointF p = point;
   p.rx() += horizontalScroll();
   p.ry() += verticalScroll();
   return identityMatrix ? p : matrix.inverted().map(p);
}

QRectF QGraphicsViewPrivate::mapToScene(const QRectF &rect) const
{
   QPointF scrollOffset(horizontalScroll(), verticalScroll());
   QPointF tl = scrollOffset + rect.topLeft();
   QPointF tr = scrollOffset + rect.topRight();
   QPointF br = scrollOffset + rect.bottomRight();
   QPointF bl = scrollOffset + rect.bottomLeft();

   QPolygonF poly(4);
   if (!identityMatrix) {
      QTransform x = matrix.inverted();
      poly[0] = x.map(tl);
      poly[1] = x.map(tr);
      poly[2] = x.map(br);
      poly[3] = x.map(bl);
   } else {
      poly[0] = tl;
      poly[1] = tr;
      poly[2] = br;
      poly[3] = bl;
   }
   return poly.boundingRect();
}

#ifndef QT_NO_CURSOR
void QGraphicsView::_q_setViewportCursor(const QCursor &cursor)
{
   Q_D(QGraphicsView);
   d->_q_setViewportCursor(cursor);
}

void QGraphicsView::_q_unsetViewportCursor()
{
   Q_D(QGraphicsView);
   d->_q_unsetViewportCursor();
}
#endif

#endif // QT_NO_GRAPHICSVIEW
