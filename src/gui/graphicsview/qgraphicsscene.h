/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QGRAPHICSSCENE_H
#define QGRAPHICSSCENE_H

#include <QtCore/qobject.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtGui/qbrush.h>
#include <QtGui/qfont.h>
#include <QtGui/qtransform.h>
#include <QtGui/qmatrix.h>
#include <QtGui/qpen.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

#if ! defined(QT_NO_GRAPHICSVIEW)

template<typename T> class QList;
class QFocusEvent;
class QFont;
class QFontMetrics;
class QGraphicsEllipseItem;
class QGraphicsItem;
class QGraphicsItemGroup;
class QGraphicsLineItem;
class QGraphicsPathItem;
class QGraphicsPixmapItem;
class QGraphicsPolygonItem;
class QGraphicsProxyWidget;
class QGraphicsRectItem;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneEvent;
class QGraphicsSceneHelpEvent;
class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;
class QGraphicsSimpleTextItem;
class QGraphicsTextItem;
class QGraphicsView;
class QGraphicsWidget;
class QGraphicsSceneIndex;
class QHelpEvent;
class QInputMethodEvent;
class QKeyEvent;
class QLineF;
class QPainterPath;
class QPixmap;
class QPointF;
class QPolygonF;
class QRectF;
class QSizeF;
class QStyle;
class QStyleOptionGraphicsItem;
class QGraphicsScenePrivate;

class Q_GUI_EXPORT QGraphicsScene : public QObject
{
   GUI_CS_OBJECT(QGraphicsScene)

   GUI_CS_PROPERTY_READ(backgroundBrush, backgroundBrush)
   GUI_CS_PROPERTY_WRITE(backgroundBrush, setBackgroundBrush)

   GUI_CS_PROPERTY_READ(foregroundBrush, foregroundBrush)
   GUI_CS_PROPERTY_WRITE(foregroundBrush, setForegroundBrush)

   GUI_CS_PROPERTY_READ(itemIndexMethod, itemIndexMethod)
   GUI_CS_PROPERTY_WRITE(itemIndexMethod, setItemIndexMethod)

   GUI_CS_PROPERTY_READ(sceneRect, sceneRect)
   GUI_CS_PROPERTY_WRITE(sceneRect, cs_setSceneRect)

   GUI_CS_PROPERTY_READ(bspTreeDepth, bspTreeDepth)
   GUI_CS_PROPERTY_WRITE(bspTreeDepth, setBspTreeDepth)

   GUI_CS_PROPERTY_READ(palette, palette)
   GUI_CS_PROPERTY_WRITE(palette, setPalette)

   GUI_CS_PROPERTY_READ(font, font)
   GUI_CS_PROPERTY_WRITE(font, setFont)

   GUI_CS_PROPERTY_READ(stickyFocus, stickyFocus)
   GUI_CS_PROPERTY_WRITE(stickyFocus, setStickyFocus)

 public:
   enum ItemIndexMethod {
      BspTreeIndex,
      NoIndex = -1
   };

   enum SceneLayer {
      ItemLayer = 0x1,
      BackgroundLayer = 0x2,
      ForegroundLayer = 0x4,
      AllLayers = 0xffff
   };
   using SceneLayers = QFlags<SceneLayer>;

   QGraphicsScene(QObject *parent = nullptr);
   QGraphicsScene(const QRectF &sceneRect, QObject *parent = nullptr);
   QGraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);
   virtual ~QGraphicsScene();

   QRectF sceneRect() const;
   inline qreal width() const {
      return sceneRect().width();
   }

   inline qreal height() const {
      return sceneRect().height();
   }

   void setSceneRect(const QRectF &rect);
   inline void setSceneRect(qreal x, qreal y, qreal w, qreal h) {
      setSceneRect(QRectF(x, y, w, h));
   }

   // wrapper for overloaded method
   inline void cs_setSceneRect(const QRectF &rect);

   void render(QPainter *painter, const QRectF &target = QRectF(), const QRectF &source = QRectF(),
               Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio);

   ItemIndexMethod itemIndexMethod() const;
   void setItemIndexMethod(ItemIndexMethod method);

   bool isSortCacheEnabled() const;
   void setSortCacheEnabled(bool enabled);

   int bspTreeDepth() const;
   void setBspTreeDepth(int depth);

   QRectF itemsBoundingRect() const;

   QList<QGraphicsItem *> items() const;

   // ### Qt5/unify
   QList<QGraphicsItem *> items(Qt::SortOrder order) const;

   QList<QGraphicsItem *> items(const QPointF &pos, Qt::ItemSelectionMode mode,
                                Qt::SortOrder order, const QTransform &deviceTransform = QTransform()) const;
   QList<QGraphicsItem *> items(const QRectF &rect, Qt::ItemSelectionMode mode,
                                Qt::SortOrder order, const QTransform &deviceTransform = QTransform()) const;
   QList<QGraphicsItem *> items(const QPolygonF &polygon, Qt::ItemSelectionMode mode,
                                Qt::SortOrder order, const QTransform &deviceTransform = QTransform()) const;
   QList<QGraphicsItem *> items(const QPainterPath &path, Qt::ItemSelectionMode mode,
                                Qt::SortOrder order, const QTransform &deviceTransform = QTransform()) const;

   // ### obsolete
   QList<QGraphicsItem *> items(const QPointF &pos) const;

   // ### obsolete
   QList<QGraphicsItem *> items(const QRectF &rect, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;

   // ### obsolete
   QList<QGraphicsItem *> items(const QPolygonF &polygon, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;

   // ### obsolete
   QList<QGraphicsItem *> items(const QPainterPath &path, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;

   QList<QGraphicsItem *> collidingItems(const QGraphicsItem *item, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;

   // ### obsolete
   QGraphicsItem *itemAt(const QPointF &pos) const;

   QGraphicsItem *itemAt(const QPointF &pos, const QTransform &deviceTransform) const;

   // ### obsolete
   inline QList<QGraphicsItem *> items(qreal x, qreal y, qreal w, qreal h,
                                       Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const {
      return items(QRectF(x, y, w, h), mode);
   }

   inline QList<QGraphicsItem *> items(qreal x, qreal y, qreal w, qreal h, Qt::ItemSelectionMode mode, Qt::SortOrder order,
                                       const QTransform &deviceTransform = QTransform()) const {
      return items(QRectF(x, y, w, h), mode, order, deviceTransform);
   }

   // ### obsolete
   inline QGraphicsItem *itemAt(qreal x, qreal y) const {
      return itemAt(QPointF(x, y));
   }

   inline QGraphicsItem *itemAt(qreal x, qreal y, const QTransform &deviceTransform) const {
      return itemAt(QPointF(x, y), deviceTransform);
   }

   QList<QGraphicsItem *> selectedItems() const;
   QPainterPath selectionArea() const;
   void setSelectionArea(const QPainterPath &path); // ### obsolete
   void setSelectionArea(const QPainterPath &path, const QTransform &deviceTransform);
   void setSelectionArea(const QPainterPath &path, Qt::ItemSelectionMode mode); // ### obsolete
   void setSelectionArea(const QPainterPath &path, Qt::ItemSelectionMode mode, const QTransform &deviceTransform);

   QGraphicsItemGroup *createItemGroup(const QList<QGraphicsItem *> &items);
   void destroyItemGroup(QGraphicsItemGroup *group);

   void addItem(QGraphicsItem *item);
   QGraphicsEllipseItem *addEllipse(const QRectF &rect, const QPen &pen = QPen(), const QBrush &brush = QBrush());
   QGraphicsLineItem *addLine(const QLineF &line, const QPen &pen = QPen());
   QGraphicsPathItem *addPath(const QPainterPath &path, const QPen &pen = QPen(), const QBrush &brush = QBrush());
   QGraphicsPixmapItem *addPixmap(const QPixmap &pixmap);
   QGraphicsPolygonItem *addPolygon(const QPolygonF &polygon, const QPen &pen = QPen(), const QBrush &brush = QBrush());
   QGraphicsRectItem *addRect(const QRectF &rect, const QPen &pen = QPen(), const QBrush &brush = QBrush());
   QGraphicsTextItem *addText(const QString &text, const QFont &font = QFont());
   QGraphicsSimpleTextItem *addSimpleText(const QString &text, const QFont &font = QFont());
   QGraphicsProxyWidget *addWidget(QWidget *widget, Qt::WindowFlags wFlags = 0);

   inline QGraphicsEllipseItem *addEllipse(qreal x, qreal y, qreal w, qreal h, const QPen &pen = QPen(), const QBrush &brush = QBrush()) {
      return addEllipse(QRectF(x, y, w, h), pen, brush);
   }

   inline QGraphicsLineItem *addLine(qreal x1, qreal y1, qreal x2, qreal y2, const QPen &pen = QPen()) {
      return addLine(QLineF(x1, y1, x2, y2), pen);
   }

   inline QGraphicsRectItem *addRect(qreal x, qreal y, qreal w, qreal h, const QPen &pen = QPen(),
                                     const QBrush &brush = QBrush()) {
      return addRect(QRectF(x, y, w, h), pen, brush);
   }

   void removeItem(QGraphicsItem *item);

   QGraphicsItem *focusItem() const;
   void setFocusItem(QGraphicsItem *item, Qt::FocusReason focusReason = Qt::OtherFocusReason);
   bool hasFocus() const;
   void setFocus(Qt::FocusReason focusReason = Qt::OtherFocusReason);
   void clearFocus();

   void setStickyFocus(bool enabled);
   bool stickyFocus() const;

   QGraphicsItem *mouseGrabberItem() const;

   QBrush backgroundBrush() const;
   void setBackgroundBrush(const QBrush &brush);

   QBrush foregroundBrush() const;
   void setForegroundBrush(const QBrush &brush);

   virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

   QList <QGraphicsView *> views() const;

   inline void update(qreal x, qreal y, qreal w, qreal h) {
      update(QRectF(x, y, w, h));
   }
   inline void invalidate(qreal x, qreal y, qreal w, qreal h, SceneLayers layers = AllLayers) {
      invalidate(QRectF(x, y, w, h), layers);
   }

   QStyle *style() const;
   void setStyle(QStyle *style);

   QFont font() const;
   void setFont(const QFont &font);

   QPalette palette() const;
   void setPalette(const QPalette &palette);

   bool isActive() const;
   QGraphicsItem *activePanel() const;
   void setActivePanel(QGraphicsItem *item);
   QGraphicsWidget *activeWindow() const;
   void setActiveWindow(QGraphicsWidget *widget);

   bool sendEvent(QGraphicsItem *item, QEvent *event);

   GUI_CS_SLOT_1(Public, void update(const QRectF &rect = QRectF()))
   GUI_CS_SLOT_OVERLOAD(update, (const QRectF &))

   GUI_CS_SLOT_1(Public, void invalidate(const QRectF &rect = QRectF(), SceneLayers layers = AllLayers))
   GUI_CS_SLOT_OVERLOAD(invalidate, (const QRectF &, SceneLayers))

   GUI_CS_SLOT_1(Public, void advance())
   GUI_CS_SLOT_2(advance)

   GUI_CS_SLOT_1(Public, void clearSelection())
   GUI_CS_SLOT_2(clearSelection)

   GUI_CS_SLOT_1(Public, void clear())
   GUI_CS_SLOT_2(clear)

   GUI_CS_SIGNAL_1(Public, void changed(const QList <QRectF> &region))
   GUI_CS_SIGNAL_2(changed, region)

   GUI_CS_SIGNAL_1(Public, void sceneRectChanged(const QRectF &rect))
   GUI_CS_SIGNAL_2(sceneRectChanged, rect)

   GUI_CS_SIGNAL_1(Public, void selectionChanged())
   GUI_CS_SIGNAL_2(selectionChanged)

 protected:
   bool event(QEvent *event) override;
   bool eventFilter(QObject *watched, QEvent *event) override;

   virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
   virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
   virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
   virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
   virtual void dropEvent(QGraphicsSceneDragDropEvent *event);
   virtual void focusInEvent(QFocusEvent *event);
   virtual void focusOutEvent(QFocusEvent *event);
   virtual void helpEvent(QGraphicsSceneHelpEvent *event);
   virtual void keyPressEvent(QKeyEvent *event);
   virtual void keyReleaseEvent(QKeyEvent *event);
   virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
   virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
   virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
   virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
   virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
   virtual void inputMethodEvent(QInputMethodEvent *event);

   virtual void drawBackground(QPainter *painter, const QRectF &rect);
   virtual void drawForeground(QPainter *painter, const QRectF &rect);
   virtual void drawItems(QPainter *painter, int numItems, QGraphicsItem *items[],
                          const QStyleOptionGraphicsItem options[], QWidget *widget = 0);

   QScopedPointer<QGraphicsScenePrivate> d_ptr;

   GUI_CS_SLOT_1(Protected, bool focusNextPrevChild(bool next))
   GUI_CS_SLOT_2(focusNextPrevChild)

 private:
   Q_DECLARE_PRIVATE(QGraphicsScene)
   Q_DISABLE_COPY(QGraphicsScene)

   GUI_CS_SLOT_1(Private, void _q_emitUpdated())
   GUI_CS_SLOT_2(_q_emitUpdated)

   GUI_CS_SLOT_1(Private, void _q_polishItems())
   GUI_CS_SLOT_2(_q_polishItems)

   GUI_CS_SLOT_1(Private, void _q_processDirtyItems())
   GUI_CS_SLOT_2(_q_processDirtyItems)

   GUI_CS_SLOT_1(Private, void _q_updateScenePosDescendants())
   GUI_CS_SLOT_2(_q_updateScenePosDescendants)

   friend class QGraphicsItem;
   friend class QGraphicsItemPrivate;
   friend class QGraphicsObject;
   friend class QGraphicsView;
   friend class QGraphicsViewPrivate;
   friend class QGraphicsWidget;
   friend class QGraphicsWidgetPrivate;
   friend class QGraphicsEffect;
   friend class QGraphicsSceneIndex;
   friend class QGraphicsSceneIndexPrivate;
   friend class QGraphicsSceneBspTreeIndex;
   friend class QGraphicsSceneBspTreeIndexPrivate;
   friend class QGraphicsItemEffectSourcePrivate;

#ifndef QT_NO_GESTURES
   friend class QGesture;
#endif
  
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGraphicsScene::SceneLayers)

void QGraphicsScene::cs_setSceneRect(const QRectF &rect)
{
   setSceneRect(rect);
}

#endif // QT_NO_GRAPHICSVIEW

QT_END_NAMESPACE

#endif
