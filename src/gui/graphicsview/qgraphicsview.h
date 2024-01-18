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

#ifndef QGRAPHICSVIEW_H
#define QGRAPHICSVIEW_H

#include <qpainter.h>
#include <qscrollarea.h>
#include <qgraphicsscene.h>

#if ! defined(QT_NO_GRAPHICSVIEW)

class QGraphicsItem;
class QPainterPath;
class QPolygonF;
class QStyleOptionGraphicsItem;
class QGraphicsViewPrivate;

class Q_GUI_EXPORT QGraphicsView : public QAbstractScrollArea
{
   GUI_CS_OBJECT(QGraphicsView)

   GUI_CS_ENUM(DragMode)
   GUI_CS_ENUM(ViewportAnchor)
   GUI_CS_ENUM(ViewportUpdateMode)
   GUI_CS_ENUM(OptimizationFlag)
   GUI_CS_ENUM(CacheModeFlag)

   GUI_CS_FLAG(OptimizationFlag, OptimizationFlags)
   GUI_CS_FLAG(CacheModeFlag, CacheMode)

   GUI_CS_PROPERTY_READ(backgroundBrush, backgroundBrush)
   GUI_CS_PROPERTY_WRITE(backgroundBrush, setBackgroundBrush)

   GUI_CS_PROPERTY_READ(foregroundBrush, foregroundBrush)
   GUI_CS_PROPERTY_WRITE(foregroundBrush, setForegroundBrush)

   GUI_CS_PROPERTY_READ(interactive, isInteractive)
   GUI_CS_PROPERTY_WRITE(interactive, setInteractive)

   GUI_CS_PROPERTY_READ(sceneRect, sceneRect)
   GUI_CS_PROPERTY_WRITE(sceneRect, cs_setSceneRect)

   GUI_CS_PROPERTY_READ(alignment, alignment)
   GUI_CS_PROPERTY_WRITE(alignment, setAlignment)

   GUI_CS_PROPERTY_READ(renderHints, renderHints)
   GUI_CS_PROPERTY_WRITE(renderHints, setRenderHints)

   GUI_CS_PROPERTY_READ(dragMode, dragMode)
   GUI_CS_PROPERTY_WRITE(dragMode, setDragMode)

   GUI_CS_PROPERTY_READ(cacheMode, cacheMode)
   GUI_CS_PROPERTY_WRITE(cacheMode, setCacheMode)

   GUI_CS_PROPERTY_READ(transformationAnchor, transformationAnchor)
   GUI_CS_PROPERTY_WRITE(transformationAnchor, setTransformationAnchor)

   GUI_CS_PROPERTY_READ(resizeAnchor, resizeAnchor)
   GUI_CS_PROPERTY_WRITE(resizeAnchor, setResizeAnchor)

   GUI_CS_PROPERTY_READ(viewportUpdateMode, viewportUpdateMode)
   GUI_CS_PROPERTY_WRITE(viewportUpdateMode, setViewportUpdateMode)

#ifndef QT_NO_RUBBERBAND
   GUI_CS_PROPERTY_READ(rubberBandSelectionMode, rubberBandSelectionMode)
   GUI_CS_PROPERTY_WRITE(rubberBandSelectionMode, setRubberBandSelectionMode)
#endif

   GUI_CS_PROPERTY_READ(optimizationFlags, optimizationFlags)
   GUI_CS_PROPERTY_WRITE(optimizationFlags, setOptimizationFlags)

 public:
   GUI_CS_REGISTER_ENUM(
      enum ViewportAnchor {
         NoAnchor,
         AnchorViewCenter,
         AnchorUnderMouse
      };
   )

   GUI_CS_REGISTER_ENUM(
      enum CacheModeFlag {
         CacheNone       = 0x0,
         CacheBackground = 0x1
      };
   )
   using CacheMode = QFlags<CacheModeFlag>;

   GUI_CS_REGISTER_ENUM(
      enum DragMode {
         NoDrag,
         ScrollHandDrag,
         RubberBandDrag
      };
   )

   GUI_CS_REGISTER_ENUM(
      enum ViewportUpdateMode {
         FullViewportUpdate,
         MinimalViewportUpdate,
         SmartViewportUpdate,
         NoViewportUpdate,
         BoundingRectViewportUpdate
      };
   )

   GUI_CS_REGISTER_ENUM(
      enum OptimizationFlag {
         DontClipPainter           = 0x1, // obsolete
         DontSavePainterState      = 0x2,
         DontAdjustForAntialiasing = 0x4,
         IndirectPainting          = 0x8
      };
   )
   using OptimizationFlags = QFlags<OptimizationFlag>;

   QGraphicsView(QWidget *parent = nullptr);
   QGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr);

   QGraphicsView(const QGraphicsView &) = delete;
   QGraphicsView &operator=(const QGraphicsView &) = delete;

   ~QGraphicsView();

   QSize sizeHint() const override;

   QPainter::RenderHints renderHints() const;
   void setRenderHint(QPainter::RenderHint hint, bool enabled = true);
   void setRenderHints(QPainter::RenderHints hints);

   Qt::Alignment alignment() const;
   void setAlignment(Qt::Alignment alignment);

   ViewportAnchor transformationAnchor() const;
   void setTransformationAnchor(ViewportAnchor anchor);

   ViewportAnchor resizeAnchor() const;
   void setResizeAnchor(ViewportAnchor anchor);

   ViewportUpdateMode viewportUpdateMode() const;
   void setViewportUpdateMode(ViewportUpdateMode mode);

   OptimizationFlags optimizationFlags() const;
   void setOptimizationFlag(OptimizationFlag flag, bool enabled = true);
   void setOptimizationFlags(OptimizationFlags flags);

   DragMode dragMode() const;
   void setDragMode(DragMode mode);

#ifndef QT_NO_RUBBERBAND
   Qt::ItemSelectionMode rubberBandSelectionMode() const;
   void setRubberBandSelectionMode(Qt::ItemSelectionMode mode);
   QRect rubberBandRect() const;
#endif

   CacheMode cacheMode() const;
   void setCacheMode(CacheMode mode);
   void resetCachedContent();

   bool isInteractive() const;
   void setInteractive(bool allowed);

   QGraphicsScene *scene() const;
   void setScene(QGraphicsScene *scene);

   QRectF sceneRect() const;
   void setSceneRect(const QRectF &rect);
   inline void setSceneRect(qreal x, qreal y, qreal width, qreal height);

   // wrapper for overloaded method
   inline void cs_setSceneRect(const QRectF &rect);

   QMatrix matrix() const;
   void setMatrix(const QMatrix &matrix, bool combine = false);
   void resetMatrix();
   QTransform transform() const;
   QTransform viewportTransform() const;
   bool isTransformed() const;
   void setTransform(const QTransform &matrix, bool combine = false);
   void resetTransform();
   void rotate(qreal angle);
   void scale(qreal sx, qreal sy);
   void shear(qreal sh, qreal sv);
   void translate(qreal dx, qreal dy);

   void centerOn(const QPointF &pos);
   inline void centerOn(qreal x, qreal y);
   void centerOn(const QGraphicsItem *item);

   void ensureVisible(const QRectF &rect, int xmargin = 50, int ymargin = 50);
   inline void ensureVisible(qreal x, qreal y, qreal width, qreal height, int xmargin = 50, int ymargin = 50);
   void ensureVisible(const QGraphicsItem *item, int xmargin = 50, int ymargin = 50);

   void fitInView(const QRectF &rect, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio);

   inline void fitInView(qreal x, qreal y, qreal width, qreal height, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio);
   void fitInView(const QGraphicsItem *item, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio);

   void render(QPainter *painter, const QRectF &target = QRectF(), const QRect &source = QRect(),
      Qt::AspectRatioMode mode = Qt::KeepAspectRatio);

   QList<QGraphicsItem *> items() const;
   QList<QGraphicsItem *> items(const QPoint &pos) const;
   inline QList<QGraphicsItem *> items(int x, int y) const;
   QList<QGraphicsItem *> items(const QRect &rect, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;
   inline QList<QGraphicsItem *> items(int x, int y, int width, int height, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;
   QList<QGraphicsItem *> items(const QPolygon &polygon, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;

   QList<QGraphicsItem *> items(const QPainterPath &path, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;
   QGraphicsItem *itemAt(const QPoint &pos) const;
   inline QGraphicsItem *itemAt(int x, int y) const;

   QPointF mapToScene(const QPoint &point) const;
   QPolygonF mapToScene(const QRect &rect) const;
   QPolygonF mapToScene(const QPolygon &polygon) const;
   QPainterPath mapToScene(const QPainterPath &path) const;
   QPoint mapFromScene(const QPointF &point) const;
   QPolygon mapFromScene(const QRectF &rect) const;
   QPolygon mapFromScene(const QPolygonF &polygon) const;
   QPainterPath mapFromScene(const QPainterPath &path) const;
   inline QPointF mapToScene(int x, int y) const;
   inline QPolygonF mapToScene(int x, int y, int width, int height) const;
   inline QPoint mapFromScene(qreal x, qreal y) const;
   inline QPolygon mapFromScene(qreal x, qreal y, qreal width, qreal height) const;

   QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

   QBrush backgroundBrush() const;
   void setBackgroundBrush(const QBrush &brush);

   QBrush foregroundBrush() const;
   void setForegroundBrush(const QBrush &brush);

   GUI_CS_SLOT_1(Public, void updateScene(const QList <QRectF> &rects))
   GUI_CS_SLOT_2(updateScene)

   GUI_CS_SLOT_1(Public, void invalidateScene(const QRectF &rect = QRectF(),
         QGraphicsScene::SceneLayers layers = QGraphicsScene::AllLayers))
   GUI_CS_SLOT_2(invalidateScene)

   GUI_CS_SLOT_1(Public, void updateSceneRect(const QRectF &rect))
   GUI_CS_SLOT_2(updateSceneRect)

#ifndef QT_NO_RUBBERBAND
   GUI_CS_SIGNAL_1(Public, void rubberBandChanged(QRect viewportRect, QPointF fromScenePoint, QPointF toScenePoint))
   GUI_CS_SIGNAL_2(rubberBandChanged, viewportRect, fromScenePoint, toScenePoint)
#endif

 protected:
   GUI_CS_SLOT_1(Protected, void setupViewport(QWidget *widget) override)
   GUI_CS_SLOT_2(setupViewport)

   QGraphicsView(QGraphicsViewPrivate &, QWidget *parent = nullptr);
   bool event(QEvent *event) override;
   bool viewportEvent(QEvent *event) override;

#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QContextMenuEvent *event) override;
#endif

   void dragEnterEvent(QDragEnterEvent *event) override;
   void dragLeaveEvent(QDragLeaveEvent *event) override;
   void dragMoveEvent(QDragMoveEvent *event) override;
   void dropEvent(QDropEvent *event) override;
   void focusInEvent(QFocusEvent *event) override;
   bool focusNextPrevChild(bool next) override;
   void focusOutEvent(QFocusEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void keyReleaseEvent(QKeyEvent *event) override;
   void mouseDoubleClickEvent(QMouseEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *event) override;
#endif

   void paintEvent(QPaintEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void scrollContentsBy(int dx, int dy) override;
   void showEvent(QShowEvent *event) override;
   void inputMethodEvent(QInputMethodEvent *event) override;

   virtual void drawBackground(QPainter *painter, const QRectF &rect);
   virtual void drawForeground(QPainter *painter, const QRectF &rect);
   virtual void drawItems(QPainter *painter, int numItems, QGraphicsItem *items[], const QStyleOptionGraphicsItem options[]);

 private:
   Q_DECLARE_PRIVATE(QGraphicsView)

#ifndef QT_NO_CURSOR
   GUI_CS_SLOT_1(Private, void _q_setViewportCursor(const QCursor &cursor))
   GUI_CS_SLOT_2(_q_setViewportCursor)

   GUI_CS_SLOT_1(Private, void _q_unsetViewportCursor())
   GUI_CS_SLOT_2(_q_unsetViewportCursor)
#endif

   friend class QGraphicsSceneWidget;
   friend class QGraphicsScene;
   friend class QGraphicsScenePrivate;
   friend class QGraphicsItemPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGraphicsView::CacheMode)
Q_DECLARE_OPERATORS_FOR_FLAGS(QGraphicsView::OptimizationFlags)

inline void QGraphicsView::cs_setSceneRect(const QRectF &rect)
{
   setSceneRect(rect);
}

inline void QGraphicsView::setSceneRect(qreal x, qreal y, qreal width, qreal height)
{
   setSceneRect(QRectF(x, y, width, height));
}

inline void QGraphicsView::centerOn(qreal x, qreal y)
{
   centerOn(QPointF(x, y));
}

inline void QGraphicsView::ensureVisible(qreal x, qreal y, qreal width, qreal height, int xmargin, int ymargin)
{
   ensureVisible(QRectF(x, y, width, height), xmargin, ymargin);
}

inline void QGraphicsView::fitInView(qreal x, qreal y, qreal width, qreal height, Qt::AspectRatioMode mode)
{
   fitInView(QRectF(x, y, width, height), mode);
}

inline QList<QGraphicsItem *> QGraphicsView::items(int x, int y) const
{
   return items(QPoint(x, y));
}

inline QList<QGraphicsItem *> QGraphicsView::items(int x, int y, int width, int height, Qt::ItemSelectionMode mode) const
{
   return items(QRect(x, y, width, height), mode);
}

inline QGraphicsItem *QGraphicsView::itemAt(int x, int y) const
{
   return itemAt(QPoint(x, y));
}

inline QPointF QGraphicsView::mapToScene(int x, int y) const
{
   return mapToScene(QPoint(x, y));
}

inline QPolygonF QGraphicsView::mapToScene(int x, int y, int width, int height) const
{
   return mapToScene(QRect(x, y, width, height));
}

inline QPoint QGraphicsView::mapFromScene(qreal x, qreal y) const
{
   return mapFromScene(QPointF(x, y));
}

inline QPolygon QGraphicsView::mapFromScene(qreal x, qreal y, qreal width, qreal height) const
{
   return mapFromScene(QRectF(x, y, width, height));
}

#endif // QT_NO_GRAPHICSVIEW


#endif
