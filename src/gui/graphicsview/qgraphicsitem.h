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

#ifndef QGRAPHICSITEM_H
#define QGRAPHICSITEM_H

#include <qglobal.h>
#include <qobject.h>
#include <qpainterpath.h>
#include <qpixmap.h>
#include <qscopedpointer.h>
#include <qtextcursor.h>
#include <qvariant.h>
#include <qrect.h>

// accurate file name
#include <qdeclarativelistproperty.h>

#if ! defined(QT_NO_GRAPHICSVIEW)

#include <qgraphicseffect.h>

class QBrush;
class QCursor;
class QFocusEvent;
class QGraphicsItemGroup;
class QGraphicsObject;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneEvent;
class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;
class QGraphicsScene;
class QGraphicsTransform;
class QGraphicsWidget;
class QStyleOptionGraphicsItem;
class QInputMethodEvent;
class QKeyEvent;
class QMatrix;
class QMenu;
class QPainter;
class QPen;
class QPointF;
class QRectF;

class QGraphicsItemPrivate;
class QAbstractGraphicsShapeItemPrivate;
class QGraphicsPathItemPrivate;
class QGraphicsRectItemPrivate;
class QGraphicsEllipseItemPrivate;
class QGraphicsPolygonItemPrivate;
class QGraphicsLineItemPrivate;
class QGraphicsPixmapItemPrivate;
class QGraphicsTextItemPrivate;
class QGraphicsSimpleTextItemPrivate;
class QGraphicsItemGroupPrivate;

class Q_GUI_EXPORT QGraphicsItem
{
 public:
   static constexpr const int Type = 1;
   static constexpr const int UserType = 65536;

   enum GraphicsItemFlag {
      ItemIsMovable = 0x1,
      ItemIsSelectable = 0x2,
      ItemIsFocusable = 0x4,
      ItemClipsToShape = 0x8,
      ItemClipsChildrenToShape = 0x10,
      ItemIgnoresTransformations = 0x20,
      ItemIgnoresParentOpacity = 0x40,
      ItemDoesntPropagateOpacityToChildren = 0x80,
      ItemStacksBehindParent = 0x100,
      ItemUsesExtendedStyleOption = 0x200,
      ItemHasNoContents = 0x400,
      ItemSendsGeometryChanges = 0x800,
      ItemAcceptsInputMethod = 0x1000,
      ItemNegativeZStacksBehindParent = 0x2000,
      ItemIsPanel = 0x4000,
      ItemIsFocusScope = 0x8000, // internal
      ItemSendsScenePositionChanges = 0x10000,
      ItemStopsClickFocusPropagation = 0x20000,
      ItemStopsFocusHandling = 0x40000,
      ItemContainsChildrenInShape = 0x80000

      // Note:: Do not forget to increase the d_ptr->itemFlags bit field by 1 when adding a new flag
   };
   using GraphicsItemFlags = QFlags<GraphicsItemFlag>;

   enum GraphicsItemChange {
      ItemPositionChange,
      ItemMatrixChange,
      ItemVisibleChange,
      ItemEnabledChange,
      ItemSelectedChange,
      ItemParentChange,
      ItemChildAddedChange,
      ItemChildRemovedChange,
      ItemTransformChange,
      ItemPositionHasChanged,
      ItemTransformHasChanged,
      ItemSceneChange,
      ItemVisibleHasChanged,
      ItemEnabledHasChanged,
      ItemSelectedHasChanged,
      ItemParentHasChanged,
      ItemSceneHasChanged,
      ItemCursorChange,
      ItemCursorHasChanged,
      ItemToolTipChange,
      ItemToolTipHasChanged,
      ItemFlagsChange,
      ItemFlagsHaveChanged,
      ItemZValueChange,
      ItemZValueHasChanged,
      ItemOpacityChange,
      ItemOpacityHasChanged,
      ItemScenePositionHasChanged,
      ItemRotationChange,
      ItemRotationHasChanged,
      ItemScaleChange,
      ItemScaleHasChanged,
      ItemTransformOriginPointChange,
      ItemTransformOriginPointHasChanged
   };

   enum CacheMode {
      NoCache,
      ItemCoordinateCache,
      DeviceCoordinateCache
   };

   enum PanelModality {
      NonModal,
      PanelModal,
      SceneModal
   };

   explicit QGraphicsItem(QGraphicsItem *parent = nullptr);

   QGraphicsItem(const QGraphicsItem &) = delete;
   QGraphicsItem &operator=(const QGraphicsItem &) = delete;

   virtual ~QGraphicsItem();

   QGraphicsScene *scene() const;

   QGraphicsItem *parentItem() const;
   QGraphicsItem *topLevelItem() const;
   QGraphicsObject *parentObject() const;
   QGraphicsWidget *parentWidget() const;
   QGraphicsWidget *topLevelWidget() const;
   QGraphicsWidget *window() const;
   QGraphicsItem *panel() const;
   void setParentItem(QGraphicsItem *parent);

   QList<QGraphicsItem *> childItems() const;
   bool isWidget() const;
   bool isWindow() const;
   bool isPanel() const;

   QGraphicsObject *toGraphicsObject();
   const QGraphicsObject *toGraphicsObject() const;

   QGraphicsItemGroup *group() const;
   void setGroup(QGraphicsItemGroup *group);

   GraphicsItemFlags flags() const;
   void setFlag(GraphicsItemFlag flag, bool enabled = true);
   void setFlags(GraphicsItemFlags flags);

   CacheMode cacheMode() const;
   void setCacheMode(CacheMode mode, const QSize &cacheSize = QSize());

   PanelModality panelModality() const;
   void setPanelModality(PanelModality panelModality);
   bool isBlockedByModalPanel(QGraphicsItem **blockingPanel = nullptr) const;

#ifndef QT_NO_TOOLTIP
   QString toolTip() const;
   void setToolTip(const QString &toolTip);
#endif

#ifndef QT_NO_CURSOR
   QCursor cursor() const;
   void setCursor(const QCursor &cursor);
   bool hasCursor() const;
   void unsetCursor();
#endif

   bool isVisible() const;
   bool isVisibleTo(const QGraphicsItem *graphicsItem) const;
   void setVisible(bool visible);

   inline void hide() {
      setVisible(false);
   }

   inline void show() {
      setVisible(true);
   }

   bool isEnabled() const;
   void setEnabled(bool enabled);

   bool isSelected() const;
   void setSelected(bool selected);

   bool acceptDrops() const;
   void setAcceptDrops(bool on);

   qreal opacity() const;
   qreal effectiveOpacity() const;
   void setOpacity(qreal opacity);

#ifndef QT_NO_GRAPHICSEFFECT
   QGraphicsEffect *graphicsEffect() const;
   void setGraphicsEffect(QGraphicsEffect *effect);
#endif

   Qt::MouseButtons acceptedMouseButtons() const;
   void setAcceptedMouseButtons(Qt::MouseButtons buttons);

   bool acceptHoverEvents() const;
   void setAcceptHoverEvents(bool enabled);
   bool acceptTouchEvents() const;
   void setAcceptTouchEvents(bool enabled);

   bool filtersChildEvents() const;
   void setFiltersChildEvents(bool enabled);

   bool handlesChildEvents() const;
   void setHandlesChildEvents(bool enabled);

   bool isActive() const;
   void setActive(bool active);

   bool hasFocus() const;
   void setFocus(Qt::FocusReason focusReason = Qt::OtherFocusReason);
   void clearFocus();

   QGraphicsItem *focusProxy() const;
   void setFocusProxy(QGraphicsItem *graphicsItem);

   QGraphicsItem *focusItem() const;
   QGraphicsItem *focusScopeItem() const;

   void grabMouse();
   void ungrabMouse();
   void grabKeyboard();
   void ungrabKeyboard();

   // Positioning in scene coordinates
   QPointF pos() const;
   qreal x() const {
      return pos().x();
   }

   void setX(qreal x);
   qreal y() const {
      return pos().y();
   }

   void setY(qreal y);
   QPointF scenePos() const;

   void setPos(const QPointF &pos);
   inline void setPos(qreal x, qreal y);

   // wrapper for static method
   void cs_setPos(const QPointF &pos) {
      setPos(pos);
   }

   void moveBy(qreal dx, qreal dy) {
      setPos(pos().x() + dx, pos().y() + dy);
   }

   void ensureVisible(const QRectF &rectF = QRectF(), int xmargin = 50, int ymargin = 50);
   inline void ensureVisible(qreal x, qreal y, qreal width, qreal height, int xmargin = 50, int ymargin = 50);

   // Local transformation
   QMatrix matrix() const;
   QMatrix sceneMatrix() const;
   void setMatrix(const QMatrix &matrix, bool combine = false);
   void resetMatrix();
   QTransform transform() const;
   QTransform sceneTransform() const;
   QTransform deviceTransform(const QTransform &viewportTransform) const;
   QTransform itemTransform(const QGraphicsItem *graphicsItem, bool *ok = nullptr) const;
   void setTransform(const QTransform &matrix, bool combine = false);
   void resetTransform();

   void setRotation(qreal angle);
   qreal rotation() const;

   void setScale(qreal factor);
   qreal scale() const;

   // wrapper for static method
   qreal cs_scale() const {
      return scale();
   }

   QList<QGraphicsTransform *> transformations() const;
   void setTransformations(const QList<QGraphicsTransform *> &transformations);

   QPointF transformOriginPoint() const;
   void setTransformOriginPoint(const QPointF &origin);

   inline void setTransformOriginPoint(qreal x, qreal y) {
      setTransformOriginPoint(QPointF(x, y));
   }

   // wrapper for overloaded method
   void cs_setTransformOriginPoint(const QPointF &origin) {
      setTransformOriginPoint(origin);
   }

   virtual void advance(int phase);

   // Stacking order
   qreal zValue() const;
   void setZValue(qreal z);
   void stackBefore(const QGraphicsItem *graphicsItem);

   // Hit test
   virtual QRectF boundingRect() const = 0;
   QRectF childrenBoundingRect() const;
   QRectF sceneBoundingRect() const;
   virtual QPainterPath shape() const;
   bool isClipped() const;
   QPainterPath clipPath() const;

   virtual bool contains(const QPointF &point) const;
   virtual bool collidesWithItem(const QGraphicsItem *graphicsItem, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;
   virtual bool collidesWithPath(const QPainterPath &path, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;
   QList<QGraphicsItem *> collidingItems(Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;

   bool isObscured(const QRectF &rectF = QRectF()) const;
   inline bool isObscured(qreal x, qreal y, qreal width, qreal height) const;
   virtual bool isObscuredBy(const QGraphicsItem *graphicsItem) const;
   virtual QPainterPath opaqueArea() const;

   QRegion boundingRegion(const QTransform &itemToDeviceTransform) const;
   qreal boundingRegionGranularity() const;
   void setBoundingRegionGranularity(qreal granularity);

   // Drawing
   virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) = 0;
   void update(const QRectF &rectF = QRectF());
   inline void update(qreal x, qreal y, qreal width, qreal height);
   void scroll(qreal dx, qreal dy, const QRectF &rectF = QRectF());

   // Coordinate mapping
   QPointF mapToItem(const QGraphicsItem *graphicsItem, const QPointF &point) const;
   QPointF mapToParent(const QPointF &point) const;
   QPointF mapToScene(const QPointF &point) const;
   QPolygonF mapToItem(const QGraphicsItem *graphicsItem, const QRectF &rectF) const;
   QPolygonF mapToParent(const QRectF &rectF) const;
   QPolygonF mapToScene(const QRectF &rectF) const;
   QRectF mapRectToItem(const QGraphicsItem *graphicsItem, const QRectF &rectF) const;
   QRectF mapRectToParent(const QRectF &rectF) const;
   QRectF mapRectToScene(const QRectF &rectF) const;
   QPolygonF mapToItem(const QGraphicsItem *graphicsItem, const QPolygonF &polygon) const;
   QPolygonF mapToParent(const QPolygonF &polygon) const;
   QPolygonF mapToScene(const QPolygonF &polygon) const;
   QPainterPath mapToItem(const QGraphicsItem *graphicsItem, const QPainterPath &path) const;
   QPainterPath mapToParent(const QPainterPath &path) const;
   QPainterPath mapToScene(const QPainterPath &path) const;
   QPointF mapFromItem(const QGraphicsItem *graphicsItem, const QPointF &point) const;
   QPointF mapFromParent(const QPointF &point) const;
   QPointF mapFromScene(const QPointF &point) const;
   QPolygonF mapFromItem(const QGraphicsItem *graphicsItem, const QRectF &rectF) const;
   QPolygonF mapFromParent(const QRectF &rectF) const;
   QPolygonF mapFromScene(const QRectF &rectF) const;
   QRectF mapRectFromItem(const QGraphicsItem *graphicsItem, const QRectF &rectF) const;
   QRectF mapRectFromParent(const QRectF &rectF) const;
   QRectF mapRectFromScene(const QRectF &rectF) const;
   QPolygonF mapFromItem(const QGraphicsItem *graphicsItem, const QPolygonF &polygon) const;
   QPolygonF mapFromParent(const QPolygonF &polygon) const;
   QPolygonF mapFromScene(const QPolygonF &polygon) const;
   QPainterPath mapFromItem(const QGraphicsItem *graphicsItem, const QPainterPath &path) const;
   QPainterPath mapFromParent(const QPainterPath &path) const;
   QPainterPath mapFromScene(const QPainterPath &path) const;

   inline QPointF mapToItem(const QGraphicsItem *graphicsItem, qreal x, qreal y) const;
   inline QPointF mapToParent(qreal x, qreal y) const;
   inline QPointF mapToScene(qreal x, qreal y) const;
   inline QPolygonF mapToItem(const QGraphicsItem *graphicsItem, qreal x, qreal y, qreal width, qreal height) const;
   inline QPolygonF mapToParent(qreal x, qreal y, qreal width, qreal height) const;
   inline QPolygonF mapToScene(qreal x, qreal y, qreal width, qreal height) const;
   inline QRectF mapRectToItem(const QGraphicsItem *graphicsItem, qreal x, qreal y, qreal width, qreal height) const;
   inline QRectF mapRectToParent(qreal x, qreal y, qreal width, qreal height) const;
   inline QRectF mapRectToScene(qreal x, qreal y, qreal width, qreal height) const;
   inline QPointF mapFromItem(const QGraphicsItem *graphicsItem, qreal x, qreal y) const;
   inline QPointF mapFromParent(qreal x, qreal y) const;
   inline QPointF mapFromScene(qreal x, qreal y) const;
   inline QPolygonF mapFromItem(const QGraphicsItem *graphicsItem, qreal x, qreal y, qreal width, qreal height) const;
   inline QPolygonF mapFromParent(qreal x, qreal y, qreal width, qreal height) const;
   inline QPolygonF mapFromScene(qreal x, qreal y, qreal width, qreal height) const;
   inline QRectF mapRectFromItem(const QGraphicsItem *graphicsItem, qreal x, qreal y, qreal width, qreal height) const;
   inline QRectF mapRectFromParent(qreal x, qreal y, qreal width, qreal height) const;
   inline QRectF mapRectFromScene(qreal x, qreal y, qreal width, qreal height) const;

   bool isAncestorOf(const QGraphicsItem *graphicsItem) const;
   QGraphicsItem *commonAncestorItem(const QGraphicsItem *graphicsItem) const;
   bool isUnderMouse() const;

   // Custom data
   QVariant data(int key) const;
   void setData(int key, const QVariant &value);

   Qt::InputMethodHints inputMethodHints() const;
   void setInputMethodHints(Qt::InputMethodHints hints);

   virtual int type() const;

   void installSceneEventFilter(QGraphicsItem *filterItem);
   void removeSceneEventFilter(QGraphicsItem *filterItem);

 protected:
   void updateMicroFocus();
   virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
   virtual bool sceneEvent(QEvent *event);
   virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
   virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
   virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
   virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
   virtual void dropEvent(QGraphicsSceneDragDropEvent *event);
   virtual void focusInEvent(QFocusEvent *event);
   virtual void focusOutEvent(QFocusEvent *event);
   virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
   virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
   virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
   virtual void keyPressEvent(QKeyEvent *event);
   virtual void keyReleaseEvent(QKeyEvent *event);
   virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
   virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
   virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
   virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
   virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
   virtual void inputMethodEvent(QInputMethodEvent *event);
   virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

   virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

   enum Extension {
      UserExtension = 0x80000000
   };

   virtual bool supportsExtension(Extension extension) const;
   virtual void setExtension(Extension extension, const QVariant &variant);
   virtual QVariant extension(const QVariant &variant) const;

   QGraphicsItem(QGraphicsItemPrivate &dd, QGraphicsItem *parent);
   QScopedPointer<QGraphicsItemPrivate> d_ptr;

   void addToIndex();
   void removeFromIndex();
   void prepareGeometryChange();

 private:
   Q_DECLARE_PRIVATE(QGraphicsItem)

   friend class QGraphicsItemGroup;
   friend class QGraphicsScene;
   friend class QGraphicsScenePrivate;
   friend class QGraphicsSceneFindItemBspTreeVisitor;
   friend class QGraphicsSceneBspTree;
   friend class QGraphicsView;
   friend class QGraphicsViewPrivate;
   friend class QGraphicsObject;
   friend class QGraphicsWidget;
   friend class QGraphicsWidgetPrivate;
   friend class QGraphicsProxyWidgetPrivate;
   friend class QGraphicsSceneIndex;
   friend class QGraphicsSceneIndexPrivate;
   friend class QGraphicsSceneBspTreeIndex;
   friend class QGraphicsSceneBspTreeIndexPrivate;
   friend class QGraphicsItemEffectSourcePrivate;
   friend class QGraphicsTransformPrivate;

#ifndef QT_NO_GESTURES
   friend class QGestureManager;
#endif

   friend bool qt_closestLeaf(const QGraphicsItem *, const QGraphicsItem *);
   friend bool qt_closestItemFirst(const QGraphicsItem *, const QGraphicsItem *);
};

inline void QGraphicsItem::setPos(qreal x, qreal y)
{
   setPos(QPointF(x, y));
}

inline void QGraphicsItem::ensureVisible(qreal x, qreal y, qreal width, qreal height, int xmargin, int ymargin)
{
   ensureVisible(QRectF(x, y, width, height), xmargin, ymargin);
}

inline void QGraphicsItem::update(qreal x, qreal y, qreal width, qreal height)
{
   update(QRectF(x, y, width, height));
}

inline bool QGraphicsItem::isObscured(qreal x, qreal y, qreal width, qreal height) const
{
   return isObscured(QRectF(x, y, width, height));
}

inline QPointF QGraphicsItem::mapToItem(const QGraphicsItem *graphicsItem, qreal x, qreal y) const
{
   return mapToItem(graphicsItem, QPointF(x, y));
}

inline QPointF QGraphicsItem::mapToParent(qreal x, qreal y) const
{
   return mapToParent(QPointF(x, y));
}

inline QPointF QGraphicsItem::mapToScene(qreal x, qreal y) const
{
   return mapToScene(QPointF(x, y));
}

inline QPointF QGraphicsItem::mapFromItem(const QGraphicsItem *graphicsItem, qreal x, qreal y) const
{
   return mapFromItem(graphicsItem, QPointF(x, y));
}

inline QPointF QGraphicsItem::mapFromParent(qreal x, qreal y) const
{
   return mapFromParent(QPointF(x, y));
}

inline QPointF QGraphicsItem::mapFromScene(qreal x, qreal y) const
{
   return mapFromScene(QPointF(x, y));
}

inline QPolygonF QGraphicsItem::mapToItem(const QGraphicsItem *graphicsItem, qreal x, qreal y, qreal width, qreal height) const
{
   return mapToItem(graphicsItem, QRectF(x, y, width, height));
}

inline QPolygonF QGraphicsItem::mapToParent(qreal x, qreal y, qreal width, qreal height) const
{
   return mapToParent(QRectF(x, y, width, height));
}

inline QPolygonF QGraphicsItem::mapToScene(qreal x, qreal y, qreal width, qreal height) const
{
   return mapToScene(QRectF(x, y, width, height));
}

inline QRectF QGraphicsItem::mapRectToItem(const QGraphicsItem *graphicsItem, qreal x, qreal y, qreal width, qreal height) const
{
   return mapRectToItem(graphicsItem, QRectF(x, y, width, height));
}

inline QRectF QGraphicsItem::mapRectToParent(qreal x, qreal y, qreal width, qreal height) const
{
   return mapRectToParent(QRectF(x, y, width, height));
}

inline QRectF QGraphicsItem::mapRectToScene(qreal x, qreal y, qreal width, qreal height) const
{
   return mapRectToScene(QRectF(x, y, width, height));
}

inline QPolygonF QGraphicsItem::mapFromItem(const QGraphicsItem *graphicsItem, qreal x, qreal y, qreal width, qreal height) const
{
   return mapFromItem(graphicsItem, QRectF(x, y, width, height));
}

inline QPolygonF QGraphicsItem::mapFromParent(qreal x, qreal y, qreal width, qreal height) const
{
   return mapFromParent(QRectF(x, y, width, height));
}

inline QPolygonF QGraphicsItem::mapFromScene(qreal x, qreal y, qreal width, qreal height) const
{
   return mapFromScene(QRectF(x, y, width, height));
}

inline QRectF QGraphicsItem::mapRectFromItem(const QGraphicsItem *graphicsItem, qreal x, qreal y, qreal width, qreal height) const
{
   return mapRectFromItem(graphicsItem, QRectF(x, y, width, height));
}

inline QRectF QGraphicsItem::mapRectFromParent(qreal x, qreal y, qreal width, qreal height) const
{
   return mapRectFromParent(QRectF(x, y, width, height));
}

inline QRectF QGraphicsItem::mapRectFromScene(qreal x, qreal y, qreal width, qreal height) const
{
   return mapRectFromScene(QRectF(x, y, width, height));
}

class Q_GUI_EXPORT QGraphicsObject : public QObject, public QGraphicsItem
{
   GUI_CS_OBJECT_MULTIPLE(QGraphicsObject, QObject)

   GUI_CS_PROPERTY_READ(parent, parentObject)
   GUI_CS_PROPERTY_WRITE(parent, setParentItem)
   GUI_CS_PROPERTY_NOTIFY(parent, parentChanged)
   GUI_CS_PROPERTY_DESIGNABLE(parent, false)

   GUI_CS_PROPERTY_READ(opacity, opacity)
   GUI_CS_PROPERTY_WRITE(opacity, setOpacity)
   GUI_CS_PROPERTY_NOTIFY(opacity, opacityChanged)
   GUI_CS_PROPERTY_FINAL(opacity)

   GUI_CS_PROPERTY_READ(enabled, isEnabled)
   GUI_CS_PROPERTY_WRITE(enabled, setEnabled)
   GUI_CS_PROPERTY_NOTIFY(enabled, enabledChanged)

   GUI_CS_PROPERTY_READ(visible, isVisible)
   GUI_CS_PROPERTY_WRITE(visible, setVisible)
   GUI_CS_PROPERTY_NOTIFY(visible, visibleChanged)
   GUI_CS_PROPERTY_FINAL(visible)

   GUI_CS_PROPERTY_READ(pos, pos)
   GUI_CS_PROPERTY_WRITE(pos, cs_setPos)
   GUI_CS_PROPERTY_FINAL(pos)

   GUI_CS_PROPERTY_READ(x, x)
   GUI_CS_PROPERTY_WRITE(x, setX)
   GUI_CS_PROPERTY_NOTIFY(x, xChanged)
   GUI_CS_PROPERTY_FINAL(x)

   GUI_CS_PROPERTY_READ(y, y)
   GUI_CS_PROPERTY_WRITE(y, setY)
   GUI_CS_PROPERTY_NOTIFY(y, yChanged)
   GUI_CS_PROPERTY_FINAL(y)

   GUI_CS_PROPERTY_READ(z, zValue)
   GUI_CS_PROPERTY_WRITE(z, setZValue)
   GUI_CS_PROPERTY_NOTIFY(z, zChanged)
   GUI_CS_PROPERTY_FINAL(z)

   GUI_CS_PROPERTY_READ(rotation, rotation)
   GUI_CS_PROPERTY_WRITE(rotation, setRotation)
   GUI_CS_PROPERTY_NOTIFY(rotation, rotationChanged)

   GUI_CS_PROPERTY_READ(scale, cs_scale)
   GUI_CS_PROPERTY_WRITE(scale, setScale)
   GUI_CS_PROPERTY_NOTIFY(scale, scaleChanged)

   GUI_CS_PROPERTY_READ(transformOriginPoint, transformOriginPoint)
   GUI_CS_PROPERTY_WRITE(transformOriginPoint, cs_setTransformOriginPoint)

#ifndef QT_NO_GRAPHICSEFFECT
   GUI_CS_PROPERTY_READ(effect, graphicsEffect)
   GUI_CS_PROPERTY_WRITE(effect, setGraphicsEffect)
#endif

   GUI_CS_PROPERTY_READ(children, cs_childrenList)
   GUI_CS_PROPERTY_DESIGNABLE(children, false)
   GUI_CS_PROPERTY_NOTIFY(children, childrenChanged)

   GUI_CS_PROPERTY_READ(width, cs_width)
   GUI_CS_PROPERTY_WRITE(width, cs_setWidth)
   GUI_CS_PROPERTY_NOTIFY(width, widthChanged)
   GUI_CS_PROPERTY_RESET(width, cs_resetWidth)
   GUI_CS_PROPERTY_FINAL(width)

   GUI_CS_PROPERTY_READ(height, cs_height)
   GUI_CS_PROPERTY_WRITE(height, cs_setHeight)
   GUI_CS_PROPERTY_NOTIFY(height, heightChanged)
   GUI_CS_PROPERTY_RESET(height, cs_resetHeight)
   GUI_CS_PROPERTY_FINAL(height)

   GUI_CS_CLASSINFO("DefaultProperty", "children")
   CS_INTERFACES(QGraphicsItem)

 public:
   explicit QGraphicsObject(QGraphicsItem *parent = nullptr);
   ~QGraphicsObject();
   using QObject::children;

#ifndef QT_NO_GESTURES
   void grabGesture(Qt::GestureType gesture, Qt::GestureFlags flags = Qt::GestureFlags());
   void ungrabGesture(Qt::GestureType gesture);
#endif

   GUI_CS_SIGNAL_1(Public, void parentChanged())
   GUI_CS_SIGNAL_2(parentChanged)

   GUI_CS_SIGNAL_1(Public, void opacityChanged())
   GUI_CS_SIGNAL_2(opacityChanged)

   GUI_CS_SIGNAL_1(Public, void visibleChanged())
   GUI_CS_SIGNAL_2(visibleChanged)

   GUI_CS_SIGNAL_1(Public, void enabledChanged())
   GUI_CS_SIGNAL_2(enabledChanged)

   GUI_CS_SIGNAL_1(Public, void xChanged())
   GUI_CS_SIGNAL_2(xChanged)

   GUI_CS_SIGNAL_1(Public, void yChanged())
   GUI_CS_SIGNAL_2(yChanged)

   GUI_CS_SIGNAL_1(Public, void zChanged())
   GUI_CS_SIGNAL_2(zChanged)

   GUI_CS_SIGNAL_1(Public, void rotationChanged())
   GUI_CS_SIGNAL_2(rotationChanged)

   GUI_CS_SIGNAL_1(Public, void scaleChanged())
   GUI_CS_SIGNAL_2(scaleChanged)

   GUI_CS_SIGNAL_1(Public, void childrenChanged())
   GUI_CS_SIGNAL_2(childrenChanged)

   GUI_CS_SIGNAL_1(Public, void widthChanged())
   GUI_CS_SIGNAL_2(widthChanged)

   GUI_CS_SIGNAL_1(Public, void heightChanged())
   GUI_CS_SIGNAL_2(heightChanged)

 protected:
   QGraphicsObject(QGraphicsItemPrivate &dd, QGraphicsItem *parent);

   bool event(QEvent *event) override;

   GUI_CS_SLOT_1(Protected, void updateMicroFocus())
   GUI_CS_SLOT_2(updateMicroFocus)

 private:
   friend class QGraphicsItem;
   friend class QGraphicsItemPrivate;

   // wrapper for overloaded method
   QDeclarativeListProperty<QGraphicsObject> cs_childrenList() const;

   // wrapper for overloaded method
   qreal cs_width() const;

   // wrapper for overloaded method
   void cs_setWidth(qreal width);

   // wrapper for overloaded method
   void cs_resetWidth();

   // wrapper for overloaded method
   qreal cs_height() const;

   // wrapper for overloaded method
   void cs_setHeight(qreal height);

   // wrapper for overloaded method
   void cs_resetHeight();

};

class Q_GUI_EXPORT QAbstractGraphicsShapeItem : public QGraphicsItem
{
 public:
   explicit QAbstractGraphicsShapeItem(QGraphicsItem *parent = nullptr);

   QAbstractGraphicsShapeItem(const QAbstractGraphicsShapeItem &) = delete;
   QAbstractGraphicsShapeItem &operator=(const QAbstractGraphicsShapeItem &) = delete;

   ~QAbstractGraphicsShapeItem();

   QPen pen() const;
   void setPen(const QPen &pen);

   QBrush brush() const;
   void setBrush(const QBrush &brush);

   bool isObscuredBy(const QGraphicsItem *graphicsItem) const override;
   QPainterPath opaqueArea() const override;

 protected:
   QAbstractGraphicsShapeItem(QAbstractGraphicsShapeItemPrivate &dd, QGraphicsItem *parent);

 private:
   Q_DECLARE_PRIVATE(QAbstractGraphicsShapeItem)
};

class Q_GUI_EXPORT QGraphicsPathItem : public QAbstractGraphicsShapeItem
{
 public:
   explicit QGraphicsPathItem(QGraphicsItem *parent = nullptr);
   explicit QGraphicsPathItem(const QPainterPath &path, QGraphicsItem *parent = nullptr);

   QGraphicsPathItem(const QGraphicsPathItem &) = delete;
   QGraphicsPathItem &operator=(const QGraphicsPathItem &) = delete;

   ~QGraphicsPathItem();

   QPainterPath path() const;
   void setPath(const QPainterPath &path);

   QRectF boundingRect() const override;
   QPainterPath shape() const override;
   bool contains(const QPointF &point) const override;

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

   bool isObscuredBy(const QGraphicsItem *graphicsItem) const override;
   QPainterPath opaqueArea() const override;

   static constexpr const int Type = 2;
   int type() const override;

 protected:
   bool supportsExtension(Extension extension) const override;
   void setExtension(Extension extension, const QVariant &variant) override;
   QVariant extension(const QVariant &variant) const override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsPathItem)
};

class Q_GUI_EXPORT QGraphicsRectItem : public QAbstractGraphicsShapeItem
{
 public:
   explicit QGraphicsRectItem(QGraphicsItem *parent = nullptr);
   explicit QGraphicsRectItem(const QRectF &rectF, QGraphicsItem *parent = nullptr);
   explicit QGraphicsRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);

   QGraphicsRectItem(const QGraphicsRectItem &) = delete;
   QGraphicsRectItem &operator=(const QGraphicsRectItem &) = delete;

   ~QGraphicsRectItem();

   QRectF rect() const;
   void setRect(const QRectF &rectF);
   inline void setRect(qreal x, qreal y, qreal width, qreal height);

   QRectF boundingRect() const override;
   QPainterPath shape() const override;
   bool contains(const QPointF &point) const override;

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

   bool isObscuredBy(const QGraphicsItem *graphicsItem) const override;
   QPainterPath opaqueArea() const override;

   static constexpr const int Type = 3;
   int type() const override;

 protected:
   bool supportsExtension(Extension extension) const override;
   void setExtension(Extension extension, const QVariant &variant) override;
   QVariant extension(const QVariant &variant) const override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsRectItem)
};

inline void QGraphicsRectItem::setRect(qreal x, qreal y, qreal width, qreal height)
{
   setRect(QRectF(x, y, width, height));
}

class Q_GUI_EXPORT QGraphicsEllipseItem : public QAbstractGraphicsShapeItem
{
 public:
   explicit QGraphicsEllipseItem(QGraphicsItem *parent = nullptr);
   explicit QGraphicsEllipseItem(const QRectF &rectF, QGraphicsItem *parent = nullptr);
   explicit QGraphicsEllipseItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);

   QGraphicsEllipseItem(const QGraphicsEllipseItem &other) = delete;
   QGraphicsEllipseItem &operator=(const QGraphicsEllipseItem &other) = delete;

   ~QGraphicsEllipseItem();

   QRectF rect() const;
   void setRect(const QRectF &rectF);
   inline void setRect(qreal x, qreal y, qreal width, qreal height);

   int startAngle() const;
   void setStartAngle(int angle);

   int spanAngle() const;
   void setSpanAngle(int angle);

   QRectF boundingRect() const override;
   QPainterPath shape() const override;
   bool contains(const QPointF &point) const override;

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

   bool isObscuredBy(const QGraphicsItem *graphicsItem) const override;
   QPainterPath opaqueArea() const override;

   static constexpr const int Type = 4;
   int type() const override;

 protected:
   bool supportsExtension(Extension extension) const override;
   void setExtension(Extension extension, const QVariant &variant) override;
   QVariant extension(const QVariant &variant) const override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsEllipseItem)
};

inline void QGraphicsEllipseItem::setRect(qreal x, qreal y, qreal width, qreal height)
{
   setRect(QRectF(x, y, width, height));
}

class Q_GUI_EXPORT QGraphicsPolygonItem : public QAbstractGraphicsShapeItem
{
 public:
   explicit QGraphicsPolygonItem(QGraphicsItem *parent = nullptr);
   explicit QGraphicsPolygonItem(const QPolygonF &polygon, QGraphicsItem *parent = nullptr);

   QGraphicsPolygonItem(const QGraphicsPolygonItem &) = delete;
   QGraphicsPolygonItem &operator=(const QGraphicsPolygonItem &) = delete;

   ~QGraphicsPolygonItem();

   QPolygonF polygon() const;
   void setPolygon(const QPolygonF &polygon);

   Qt::FillRule fillRule() const;
   void setFillRule(Qt::FillRule rule);

   QRectF boundingRect() const override;
   QPainterPath shape() const override;
   bool contains(const QPointF &point) const override;

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

   bool isObscuredBy(const QGraphicsItem *graphicsItem) const override;
   QPainterPath opaqueArea() const override;

   static constexpr const int Type = 5;
   int type() const override;

 protected:
   bool supportsExtension(Extension extension) const override;
   void setExtension(Extension extension, const QVariant &variant) override;
   QVariant extension(const QVariant &variant) const override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsPolygonItem)
};

class Q_GUI_EXPORT QGraphicsLineItem : public QGraphicsItem
{
 public:
   explicit QGraphicsLineItem(QGraphicsItem *parent = nullptr);
   explicit QGraphicsLineItem(const QLineF &line, QGraphicsItem *parent = nullptr);
   explicit QGraphicsLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent = nullptr);

   QGraphicsLineItem(const QGraphicsLineItem &) = delete;
   QGraphicsLineItem &operator=(const QGraphicsLineItem &) = delete;

   ~QGraphicsLineItem();

   QPen pen() const;
   void setPen(const QPen &pen);

   QLineF line() const;
   void setLine(const QLineF &line);
   inline void setLine(qreal x1, qreal y1, qreal x2, qreal y2) {
      setLine(QLineF(x1, y1, x2, y2));
   }

   QRectF boundingRect() const override;
   QPainterPath shape() const override;
   bool contains(const QPointF &point) const override;

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

   bool isObscuredBy(const QGraphicsItem *graphicsItem) const override;
   QPainterPath opaqueArea() const override;

   static constexpr const int Type = 6;
   int type() const override;

 protected:
   bool supportsExtension(Extension extension) const override;
   void setExtension(Extension extension, const QVariant &variant) override;
   QVariant extension(const QVariant &variant) const override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsLineItem)
};

class Q_GUI_EXPORT QGraphicsPixmapItem : public QGraphicsItem
{
 public:
   enum ShapeMode {
      MaskShape,
      BoundingRectShape,
      HeuristicMaskShape
   };

   explicit QGraphicsPixmapItem(QGraphicsItem *parent = nullptr);
   explicit QGraphicsPixmapItem(const QPixmap &pixmap, QGraphicsItem *parent = nullptr);

   QGraphicsPixmapItem(const QGraphicsPixmapItem &) = delete;
   QGraphicsPixmapItem &operator=(const QGraphicsPixmapItem &) = delete;

   ~QGraphicsPixmapItem();

   QPixmap pixmap() const;
   void setPixmap(const QPixmap &pixmap);

   Qt::TransformationMode transformationMode() const;
   void setTransformationMode(Qt::TransformationMode mode);

   QPointF offset() const;
   void setOffset(const QPointF &offset);
   inline void setOffset(qreal x, qreal y);

   QRectF boundingRect() const override;
   QPainterPath shape() const override;
   bool contains(const QPointF &point) const override;

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

   bool isObscuredBy(const QGraphicsItem *graphicsItem) const override;
   QPainterPath opaqueArea() const override;

   static constexpr const int Type = 7;
   int type() const override;

   ShapeMode shapeMode() const;
   void setShapeMode(ShapeMode mode);

 protected:
   bool supportsExtension(Extension extension) const override;
   void setExtension(Extension extension, const QVariant &variant) override;
   QVariant extension(const QVariant &variant) const override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsPixmapItem)
};

inline void QGraphicsPixmapItem::setOffset(qreal x, qreal y)
{
   setOffset(QPointF(x, y));
}

class Q_GUI_EXPORT QGraphicsTextItem : public QGraphicsObject
{
   GUI_CS_OBJECT(QGraphicsTextItem)

   GUI_CS_PROPERTY_READ(openExternalLinks, openExternalLinks)
   GUI_CS_PROPERTY_WRITE(openExternalLinks, setOpenExternalLinks)

   GUI_CS_PROPERTY_READ(textCursor, textCursor)
   GUI_CS_PROPERTY_WRITE(textCursor, setTextCursor)

 public:
   explicit QGraphicsTextItem(QGraphicsItem *parent = nullptr);
   explicit QGraphicsTextItem(const QString &text, QGraphicsItem *parent = nullptr);

   QGraphicsTextItem(const QGraphicsTextItem &other) = delete;
   QGraphicsTextItem &operator=(const QGraphicsTextItem &other) = delete;

   ~QGraphicsTextItem();

   QString toHtml() const;
   void setHtml(const QString &text);

   QString toPlainText() const;
   void setPlainText(const QString &text);

   QFont font() const;
   void setFont(const QFont &font);

   void setDefaultTextColor(const QColor &color);
   QColor defaultTextColor() const;

   QRectF boundingRect() const override;
   QPainterPath shape() const override;
   bool contains(const QPointF &point) const override;

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

   bool isObscuredBy(const QGraphicsItem *graphicsItem) const override;
   QPainterPath opaqueArea() const override;

   static constexpr const int Type = 8;
   int type() const override;

   void setTextWidth(qreal width);
   qreal textWidth() const;

   void adjustSize();

   void setDocument(QTextDocument *document);
   QTextDocument *document() const;

   void setTextInteractionFlags(Qt::TextInteractionFlags flags);
   Qt::TextInteractionFlags textInteractionFlags() const;

   void setTabChangesFocus(bool b);
   bool tabChangesFocus() const;

   void setOpenExternalLinks(bool open);
   bool openExternalLinks() const;

   void setTextCursor(const QTextCursor &cursor);
   QTextCursor textCursor() const;

   GUI_CS_SIGNAL_1(Public, void linkActivated(const QString &link))
   GUI_CS_SIGNAL_2(linkActivated, link)

   GUI_CS_SIGNAL_1(Public, void linkHovered(const QString &link))
   GUI_CS_SIGNAL_2(linkHovered, link)

 protected:
   bool sceneEvent(QEvent *event) override;
   void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
   void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
   void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
   void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void keyReleaseEvent(QKeyEvent *event) override;
   void focusInEvent(QFocusEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
   void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
   void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
   void dropEvent(QGraphicsSceneDragDropEvent *event) override;
   void inputMethodEvent(QInputMethodEvent *event) override;
   void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
   void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
   void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

   QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

   bool supportsExtension(Extension extension) const override;
   void setExtension(Extension extension, const QVariant &variant) override;
   QVariant extension(const QVariant &variant) const override;

 private:
   QGraphicsTextItemPrivate *dd;

   GUI_CS_SLOT_1(Private, void _q_updateBoundingRect(const QSizeF &sizeF))
   GUI_CS_SLOT_2(_q_updateBoundingRect)

   GUI_CS_SLOT_1(Private, void _q_update(const QRectF &rectF))
   GUI_CS_SLOT_2(_q_update)

   GUI_CS_SLOT_1(Private, void _q_ensureVisible(const QRectF &rectF))
   GUI_CS_SLOT_2(_q_ensureVisible)

   friend class QGraphicsTextItemPrivate;
};

class Q_GUI_EXPORT QGraphicsSimpleTextItem : public QAbstractGraphicsShapeItem
{

 public:
   explicit QGraphicsSimpleTextItem(QGraphicsItem *parent = nullptr);
   explicit QGraphicsSimpleTextItem(const QString &text, QGraphicsItem *parent = nullptr);

   QGraphicsSimpleTextItem(const QGraphicsSimpleTextItem &) = delete;
   QGraphicsSimpleTextItem &operator=(const QGraphicsSimpleTextItem &) = delete;

   ~QGraphicsSimpleTextItem();

   void setText(const QString &text);
   QString text() const;

   void setFont(const QFont &font);
   QFont font() const;

   QRectF boundingRect() const override;
   QPainterPath shape() const override;
   bool contains(const QPointF &point) const override;

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

   bool isObscuredBy(const QGraphicsItem *graphicsItem) const override;
   QPainterPath opaqueArea() const override;

   static constexpr const int Type = 9;
   int type() const override;

 protected:
   bool supportsExtension(Extension extension) const override;
   void setExtension(Extension extension, const QVariant &variant) override;
   QVariant extension(const QVariant &variant) const override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsSimpleTextItem)
};

class Q_GUI_EXPORT QGraphicsItemGroup : public QGraphicsItem
{
 public:
   explicit QGraphicsItemGroup(QGraphicsItem *parent = nullptr);

   QGraphicsItemGroup(const QGraphicsItemGroup &) = delete;
   QGraphicsItemGroup &operator=(const QGraphicsItemGroup &) = delete;

   ~QGraphicsItemGroup();

   void addToGroup(QGraphicsItem *graphicsItem);
   void removeFromGroup(QGraphicsItem *graphicsItem);

   QRectF boundingRect() const override;
   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

   bool isObscuredBy(const QGraphicsItem *graphicsItem) const override;
   QPainterPath opaqueArea() const override;

   static constexpr const int Type = 10;
   int type() const override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsItemGroup)
};

template <class T>
T qgraphicsitem_cast(QGraphicsItem *item)
{
   using Item = typename std::remove_cv<typename std::remove_pointer<T>::type>::type;

   return int(Item::Type) == int(QGraphicsItem::Type)
      || (item && int(Item::Type) == item->type()) ? static_cast<T>(item) : nullptr;
}

template <class T>
T qgraphicsitem_cast(const QGraphicsItem *item)
{
   using Item = typename std::remove_cv<typename std::remove_pointer<T>::type>::type;

   return int(Item::Type) == int(QGraphicsItem::Type)
      || (item && int(Item::Type) == item->type()) ? static_cast<T>(item) : nullptr;
}

Q_GUI_EXPORT QDebug operator<<(QDebug debug, const QGraphicsItem *item);
Q_GUI_EXPORT QDebug operator<<(QDebug debug, const QGraphicsObject *item);

Q_GUI_EXPORT QDebug operator<<(QDebug debug, QGraphicsItem::GraphicsItemChange change);
Q_GUI_EXPORT QDebug operator<<(QDebug debug, QGraphicsItem::GraphicsItemFlag flag);
Q_GUI_EXPORT QDebug operator<<(QDebug debug, QGraphicsItem::GraphicsItemFlags flags);

Q_DECLARE_OPERATORS_FOR_FLAGS(QGraphicsItem::GraphicsItemFlags)
CS_DECLARE_INTERFACE(QGraphicsItem, "com.copperspice.QGraphicsItem")

CS_DECLARE_METATYPE(QGraphicsItem)

#endif // QT_NO_GRAPHICSVIEW

#endif
