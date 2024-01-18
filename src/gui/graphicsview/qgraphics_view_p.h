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

#ifndef QGRAPHICS_VIEW_P_H
#define QGRAPHICS_VIEW_P_H

#include <qgraphicsview.h>

#if !defined(QT_NO_GRAPHICSVIEW)

#include <qevent.h>
#include <qcoreapplication.h>
#include <qgraphicssceneevent.h>
#include <qstyleoption.h>

#include <qabstractscrollarea_p.h>
#include <qapplication_p.h>

class Q_GUI_EXPORT QGraphicsViewPrivate : public QAbstractScrollAreaPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsView)

 public:
   QGraphicsViewPrivate();
   ~QGraphicsViewPrivate();

   void recalculateContentSize();
   void centerView(QGraphicsView::ViewportAnchor anchor);

   QPainter::RenderHints renderHints;
   QGraphicsView::DragMode dragMode;

   quint32 sceneInteractionAllowed : 1;
   quint32 hasSceneRect : 1;
   quint32 connectedToScene : 1;
   quint32 useLastMouseEvent : 1;
   quint32 identityMatrix : 1;
   quint32 dirtyScroll : 1;
   quint32 accelerateScrolling : 1;
   quint32 keepLastCenterPoint : 1;
   quint32 transforming : 1;
   quint32 handScrolling : 1;
   quint32 mustAllocateStyleOptions : 1;
   quint32 mustResizeBackgroundPixmap : 1;
   quint32 fullUpdatePending : 1;
   quint32 hasUpdateClip : 1;
   quint32 padding : 18;

   QRectF sceneRect;
   void updateLastCenterPoint();

   qint64 horizontalScroll() const;
   qint64 verticalScroll() const;

   QRectF mapRectToScene(const QRect &rect) const;
   QRectF mapRectFromScene(const QRectF &rect) const;

   QRect updateClip;
   QPointF mousePressItemPoint;
   QPointF mousePressScenePoint;
   QPoint mousePressViewPoint;
   QPoint mousePressScreenPoint;
   QPointF lastMouseMoveScenePoint;
   QPointF lastRubberbandScenePoint;
   QPoint lastMouseMoveScreenPoint;
   QPoint dirtyScrollOffset;
   Qt::MouseButton mousePressButton;
   QTransform matrix;
   qint64 scrollX, scrollY;
   void updateScroll();

   qreal leftIndent;
   qreal topIndent;

   // Replaying mouse events
   QMouseEvent lastMouseEvent;
   void replayLastMouseEvent();
   void storeMouseEvent(QMouseEvent *event);
   void mouseMoveEventHandler(QMouseEvent *event);

   QPointF lastCenterPoint;
   Qt::Alignment alignment;

   QGraphicsView::ViewportAnchor transformationAnchor;
   QGraphicsView::ViewportAnchor resizeAnchor;
   QGraphicsView::ViewportUpdateMode viewportUpdateMode;
   QGraphicsView::OptimizationFlags optimizationFlags;

   QPointer<QGraphicsScene> scene;

#ifndef QT_NO_RUBBERBAND
   QRect rubberBandRect;
   QRegion rubberBandRegion(const QWidget *widget, const QRect &rect) const;
   void updateRubberBand(const QMouseEvent *event);
   bool rubberBanding;
   Qt::ItemSelectionMode rubberBandSelectionMode;
   Qt::ItemSelectionOperation rubberBandSelectionOperation;
#endif

   int handScrollMotions;

   QGraphicsView::CacheMode cacheMode;

   QVector<QStyleOptionGraphicsItem> styleOptions;
   QStyleOptionGraphicsItem *allocStyleOptionsArray(int numItems);
   void freeStyleOptionsArray(QStyleOptionGraphicsItem *array);

   QBrush backgroundBrush;
   QBrush foregroundBrush;
   QPixmap backgroundPixmap;
   QRegion backgroundPixmapExposed;

#ifndef QT_NO_CURSOR
   QCursor originalCursor;
   bool hasStoredOriginalCursor;
   void _q_setViewportCursor(const QCursor &cursor);
   void _q_unsetViewportCursor();
#endif

   QGraphicsSceneDragDropEvent *lastDragDropEvent;
   void storeDragDropEvent(const QGraphicsSceneDragDropEvent *event);
   void populateSceneDragDropEvent(QGraphicsSceneDragDropEvent *dest, QDropEvent *source);

   QRect mapToViewRect(const QGraphicsItem *item, const QRectF &rect) const;
   QRegion mapToViewRegion(const QGraphicsItem *item, const QRectF &rect) const;
   QRegion dirtyRegion;
   QRect dirtyBoundingRect;
   void processPendingUpdates();
   inline void updateAll() {
      viewport->update();
      fullUpdatePending = true;
      dirtyBoundingRect = QRect();
      dirtyRegion = QRegion();
   }

   inline void dispatchPendingUpdateRequests() {


      {
         if (qt_widget_private(viewport)->paintOnScreen()) {
            QCoreApplication::sendPostedEvents(viewport, QEvent::UpdateRequest);
         } else {
            QCoreApplication::sendPostedEvents(viewport->window(), QEvent::UpdateRequest);
         }
      }
   }

   void setUpdateClip(QGraphicsItem *);

   inline bool updateRectF(const QRectF &rect) {
      if (rect.isEmpty()) {
         return false;
      }
      if (optimizationFlags & QGraphicsView::DontAdjustForAntialiasing) {
         return updateRect(rect.toAlignedRect().adjusted(-1, -1, 1, 1));
      }
      return updateRect(rect.toAlignedRect().adjusted(-2, -2, 2, 2));
   }

   bool updateRect(const QRect &rect);
   bool updateRegion(const QRectF &rect, const QTransform &xform);
   bool updateSceneSlotReimplementedChecked;
   QRegion exposedRegion;

   QList<QGraphicsItem *> findItems(const QRegion &exposedRegion, bool *allItems,
      const QTransform &viewTransform) const;

   QPointF mapToScene(const QPointF &point) const;
   QRectF mapToScene(const QRectF &rect) const;
   static void translateTouchEvent(QGraphicsViewPrivate *d, QTouchEvent *touchEvent);
   void updateInputMethodSensitivity();
};


#endif // QT_NO_GRAPHICSVIEW

#endif
