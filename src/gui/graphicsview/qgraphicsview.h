/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <QtCore/qmetatype.h>
#include <QtGui/qpainter.h>
#include <QtGui/qscrollarea.h>
#include <QtGui/qgraphicsscene.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_GRAPHICSVIEW)

class QGraphicsItem;
class QPainterPath;
class QPolygonF;
class QStyleOptionGraphicsItem;
class QGraphicsViewPrivate;

class Q_GUI_EXPORT QGraphicsView : public QAbstractScrollArea
{
   GUI_CS_OBJECT(QGraphicsView)

   GUI_CS_FLAG(OptimizationFlag, OptimizationFlags)
   GUI_CS_FLAG(CacheModeFlag, CacheMode)

   GUI_CS_ENUM(ViewportAnchor)
   GUI_CS_ENUM(DragMode)
   GUI_CS_ENUM(ViewportUpdateMode)

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
   enum ViewportAnchor {
      NoAnchor,
      AnchorViewCenter,
      AnchorUnderMouse
   };

   enum CacheModeFlag {
      CacheNone = 0x0,
      CacheBackground = 0x1
   };
   using CacheMode = QFlags<CacheModeFlag>;

   enum DragMode {
      NoDrag,
      ScrollHandDrag,
      RubberBandDrag
   };

   enum ViewportUpdateMode {
      FullViewportUpdate,
      MinimalViewportUpdate,
      SmartViewportUpdate,
      NoViewportUpdate,
      BoundingRectViewportUpdate
   };

   enum OptimizationFlag {
      DontClipPainter = 0x1, // obsolete
      DontSavePainterState = 0x2,
      DontAdjustForAntialiasing = 0x4,
      IndirectPainting = 0x8
   };
   using OptimizationFlags = QFlags<OptimizationFlag>;

   QGraphicsView(QWidget *parent = nullptr);
   QGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr);
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
   inline void setSceneRect(qreal x, qreal y, qreal w, qreal h);

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
   inline void ensureVisible(qreal x, qreal y, qreal w, qreal h, int xmargin = 50, int ymargin = 50);
   void ensureVisible(const QGraphicsItem *item, int xmargin = 50, int ymargin = 50);

   void fitInView(const QRectF &rect, Qt::AspectRatioMode aspectRadioMode = Qt::IgnoreAspectRatio);

   inline void fitInView(qreal x, qreal y, qreal w, qreal h, Qt::AspectRatioMode aspectRadioMode = Qt::IgnoreAspectRatio);
   void fitInView(const QGraphicsItem *item, Qt::AspectRatioMode aspectRadioMode = Qt::IgnoreAspectRatio);

   void render(QPainter *painter, const QRectF &target = QRectF(), const QRect &source = QRect(),
               Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio);

   QList<QGraphicsItem *> items() const;
   QList<QGraphicsItem *> items(const QPoint &pos) const;
   inline QList<QGraphicsItem *> items(int x, int y) const;
   QList<QGraphicsItem *> items(const QRect &rect, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;
   inline QList<QGraphicsItem *> items(int x, int y, int w, int h,
                                       Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;
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
   inline QPolygonF mapToScene(int x, int y, int w, int h) const;
   inline QPoint mapFromScene(qreal x, qreal y) const;
   inline QPolygon mapFromScene(qreal x, qreal y, qreal w, qreal h) const;

   QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

   QBrush backgroundBrush() const;
   void setBackgroundBrush(const QBrush &brush);

   QBrush foregroundBrush() const;
   void setForegroundBrush(const QBrush &brush);

 public :
   GUI_CS_SLOT_1(Public, void updateScene(const QList <QRectF> &rects))
   GUI_CS_SLOT_2(updateScene)

   GUI_CS_SLOT_1(Public, void invalidateScene(const QRectF &rect = QRectF(),
                 QGraphicsScene::SceneLayers layers = QGraphicsScene::AllLayers))
   GUI_CS_SLOT_2(invalidateScene)

   GUI_CS_SLOT_1(Public, void updateSceneRect(const QRectF &rect))
   GUI_CS_SLOT_2(updateSceneRect)

 protected :
   GUI_CS_SLOT_1(Protected, void setupViewport(QWidget *widget))
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
   Q_DISABLE_COPY(QGraphicsView)

#ifndef QT_NO_CURSOR
   GUI_CS_SLOT_1(Private, void _q_setViewportCursor(const QCursor &un_named_arg1))
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

void QGraphicsView::cs_setSceneRect(const QRectF &rect)
{
   setSceneRect(rect);
}

void QGraphicsView::setSceneRect(qreal ax, qreal ay, qreal aw, qreal ah)
{
   setSceneRect(QRectF(ax, ay, aw, ah));
}

void QGraphicsView::centerOn(qreal ax, qreal ay)
{
   centerOn(QPointF(ax, ay));
}

void QGraphicsView::ensureVisible(qreal ax, qreal ay, qreal aw, qreal ah, int xmargin, int ymargin)
{
   ensureVisible(QRectF(ax, ay, aw, ah), xmargin, ymargin);
}

void QGraphicsView::fitInView(qreal ax, qreal ay, qreal w, qreal h, Qt::AspectRatioMode mode)
{
   fitInView(QRectF(ax, ay, w, h), mode);
}

QList<QGraphicsItem *> QGraphicsView::items(int ax, int ay) const
{
   return items(QPoint(ax, ay));
}

QList<QGraphicsItem *> QGraphicsView::items(int ax, int ay, int w, int h, Qt::ItemSelectionMode mode) const
{
   return items(QRect(ax, ay, w, h), mode);
}

QGraphicsItem *QGraphicsView::itemAt(int ax, int ay) const
{
   return itemAt(QPoint(ax, ay));
}

QPointF QGraphicsView::mapToScene(int ax, int ay) const
{
   return mapToScene(QPoint(ax, ay));
}

QPolygonF QGraphicsView::mapToScene(int ax, int ay, int w, int h) const
{
   return mapToScene(QRect(ax, ay, w, h));
}

QPoint QGraphicsView::mapFromScene(qreal ax, qreal ay) const
{
   return mapFromScene(QPointF(ax, ay));
}

QPolygon QGraphicsView::mapFromScene(qreal ax, qreal ay, qreal w, qreal h) const
{
   return mapFromScene(QRectF(ax, ay, w, h));
}

#endif // QT_NO_GRAPHICSVIEW

QT_END_NAMESPACE

#endif // QGRAPHICSVIEW_H
