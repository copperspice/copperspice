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

#ifndef QGRAPHICSWIDGET_H
#define QGRAPHICSWIDGET_H

#include <qfont.h>
#include <qgraphicslayout.h>
#include <qgraphicslayoutitem.h>
#include <qgraphicsitem.h>
#include <qpalette.h>

class QFont;
class QFontMetrics;
class QGraphicsSceneMoveEvent;
class QGraphicsWidgetPrivate;
class QGraphicsSceneResizeEvent;
class QStyle;
class QStyleOption;

#if ! defined(QT_NO_GRAPHICSVIEW)

class QGraphicsWidgetPrivate;

class Q_GUI_EXPORT QGraphicsWidget : public QGraphicsObject, public QGraphicsLayoutItem
{
   GUI_CS_OBJECT_MULTIPLE(QGraphicsWidget, QGraphicsObject)
   CS_INTERFACES(QGraphicsItem, QGraphicsLayoutItem)

   GUI_CS_PROPERTY_READ(palette, palette)
   GUI_CS_PROPERTY_WRITE(palette, setPalette)

   GUI_CS_PROPERTY_READ(font, font)
   GUI_CS_PROPERTY_WRITE(font, setFont)

   GUI_CS_PROPERTY_READ(layoutDirection, layoutDirection)
   GUI_CS_PROPERTY_WRITE(layoutDirection, setLayoutDirection)
   GUI_CS_PROPERTY_RESET(layoutDirection, unsetLayoutDirection)

   GUI_CS_PROPERTY_READ(size, size)
   GUI_CS_PROPERTY_WRITE(size, cs_resize)
   GUI_CS_PROPERTY_NOTIFY(size, geometryChanged)

   GUI_CS_PROPERTY_READ(minimumSize, minimumSize)
   GUI_CS_PROPERTY_WRITE(minimumSize, cs_setMinimumSize)

   GUI_CS_PROPERTY_READ(preferredSize, preferredSize)
   GUI_CS_PROPERTY_WRITE(preferredSize, cs_setPreferredSize)

   GUI_CS_PROPERTY_READ(maximumSize, maximumSize)
   GUI_CS_PROPERTY_WRITE(maximumSize, cs_setMaximumSize)

   GUI_CS_PROPERTY_READ(sizePolicy, sizePolicy)
   GUI_CS_PROPERTY_WRITE(sizePolicy, cs_setSizePolicy)

   GUI_CS_PROPERTY_READ(focusPolicy, focusPolicy)
   GUI_CS_PROPERTY_WRITE(focusPolicy, setFocusPolicy)

   GUI_CS_PROPERTY_READ(windowFlags, windowFlags)
   GUI_CS_PROPERTY_WRITE(windowFlags, setWindowFlags)

   GUI_CS_PROPERTY_READ(windowTitle, windowTitle)
   GUI_CS_PROPERTY_WRITE(windowTitle, setWindowTitle)

   GUI_CS_PROPERTY_READ(geometry, geometry)
   GUI_CS_PROPERTY_WRITE(geometry, cs_setGeometry)
   GUI_CS_PROPERTY_NOTIFY(geometry, geometryChanged)

   GUI_CS_PROPERTY_READ(autoFillBackground, autoFillBackground)
   GUI_CS_PROPERTY_WRITE(autoFillBackground, setAutoFillBackground)

   GUI_CS_PROPERTY_READ(layout, layout)
   GUI_CS_PROPERTY_WRITE(layout, setLayout)
   GUI_CS_PROPERTY_NOTIFY(layout, layoutChanged)

 public:
   QGraphicsWidget(QGraphicsItem *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   QGraphicsWidget(const QGraphicsWidget &) = delete;
   QGraphicsWidget &operator=(const QGraphicsWidget &) = delete;

   ~QGraphicsWidget();

   QGraphicsLayout *layout() const;
   void setLayout(QGraphicsLayout *layout);
   void adjustSize();

   Qt::LayoutDirection layoutDirection() const;
   void setLayoutDirection(Qt::LayoutDirection direction);
   void unsetLayoutDirection();

   QStyle *style() const;
   void setStyle(QStyle *style);

   QFont font() const;
   void setFont(const QFont &font);

   QPalette palette() const;
   void setPalette(const QPalette &palette);

   bool autoFillBackground() const;
   void setAutoFillBackground(bool enabled);

   void resize(const QSizeF &size);
   void resize(qreal width, qreal height) {
      resize(QSizeF(width, height));
   }

   QSizeF size() const;

   // wrapper for overloaded method
   inline void cs_resize(const QSizeF &size);

   void setGeometry(const QRectF &rect) override;
   inline void setGeometry(qreal x, qreal y, qreal width, qreal height);

   QRectF rect() const {
      return QRectF(QPointF(), size());
   }

   // wrapper for overloaded method
   inline void cs_setGeometry(const QRectF &size);

   // wrapper for overloaded method
   inline void cs_setMinimumSize(const QSizeF &size);

   // wrapper for overloaded method
   inline void cs_setPreferredSize(const QSizeF &size);

   // wrapper for overloaded method
   inline void cs_setMaximumSize(const QSizeF &size);

   // wrapper for overloaded method
   inline void cs_setSizePolicy(const QSizePolicy &policy);

   void setContentsMargins(qreal left, qreal top, qreal right, qreal bottom);
   void getContentsMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const override;

   void setWindowFrameMargins(qreal left, qreal top, qreal right, qreal bottom);
   void getWindowFrameMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const;
   void unsetWindowFrameMargins();
   QRectF windowFrameGeometry() const;
   QRectF windowFrameRect() const;

   // Window handling
   Qt::WindowFlags windowFlags() const;
   Qt::WindowType windowType() const;
   void setWindowFlags(Qt::WindowFlags flags);
   bool isActiveWindow() const;
   void setWindowTitle(const QString &title);
   QString windowTitle() const;

   // Focus handling
   Qt::FocusPolicy focusPolicy() const;
   void setFocusPolicy(Qt::FocusPolicy policy);
   static void setTabOrder(QGraphicsWidget *first, QGraphicsWidget *second);
   QGraphicsWidget *focusWidget() const;

#ifndef QT_NO_SHORTCUT
   int grabShortcut(const QKeySequence &sequence, Qt::ShortcutContext context = Qt::WindowShortcut);
   void releaseShortcut(int id);
   void setShortcutEnabled(int id, bool enabled = true);
   void setShortcutAutoRepeat(int id, bool enabled = true);
#endif

#ifndef QT_NO_ACTION
   void addAction(QAction *action);
   void addActions(const QList<QAction *> &actions);
   void insertAction(QAction *before, QAction *action);
   void insertActions(QAction *before, QList<QAction *> actions);
   void removeAction(QAction *action);
   QList<QAction *> actions() const;
#endif

   void setAttribute(Qt::WidgetAttribute attribute, bool on = true);
   bool testAttribute(Qt::WidgetAttribute attribute) const;

   static constexpr const int Type = 11;

   int type() const override;

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
   virtual void paintWindowFrame(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
   QRectF boundingRect() const override;
   QPainterPath shape() const override;

   using QObject::children;

   GUI_CS_SIGNAL_1(Public, void geometryChanged())
   GUI_CS_SIGNAL_2(geometryChanged)

   GUI_CS_SIGNAL_1(Public, void layoutChanged())
   GUI_CS_SIGNAL_2(layoutChanged)

   GUI_CS_SLOT_1(Public, bool close())
   GUI_CS_SLOT_2(close)

 protected:
   QGraphicsWidget(QGraphicsWidgetPrivate &, QGraphicsItem *parent, Qt::WindowFlags flags = Qt::EmptyFlag);

   virtual void initStyleOption(QStyleOption *option) const;

   QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override;
   void updateGeometry() override;

   // Notification
   QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
   virtual QVariant propertyChange(const QString &propertyName, const QVariant &value);

   // Scene events
   bool sceneEvent(QEvent *event) override;
   virtual bool windowFrameEvent(QEvent *event);
   virtual Qt::WindowFrameSection windowFrameSectionAt(const QPointF &pos) const;

   // Base event handlers
   bool event(QEvent *event) override;
   // virtual void actionEvent(QActionEvent *event);
   virtual void changeEvent(QEvent *event);
   virtual void closeEvent(QCloseEvent *event);

   // void create(WId window = 0, bool initializeWindow = true, bool destroyOldWindow = true);
   // void destroy(bool destroyWindow = true, bool destroySubWindows = true);

   void focusInEvent(QFocusEvent *event) override;
   virtual bool focusNextPrevChild(bool next);
   void focusOutEvent(QFocusEvent *event) override;
   virtual void hideEvent(QHideEvent *event);

   // virtual bool macEvent(EventHandlerCallRef caller, EventRef event);
   // virtual int metric(PaintDeviceMetric m ) const;

   virtual void moveEvent(QGraphicsSceneMoveEvent *event);
   virtual void polishEvent();

   // virtual bool qwsEvent(QWSEvent *event);
   // void resetInputContext ();

   virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
   virtual void showEvent(QShowEvent *event);

   // virtual void tabletEvent(QTabletEvent *event);
   // virtual bool winEvent(MSG *message, long *result);
   // virtual bool x11Event(XEvent *event);

   void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
   void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

   virtual void grabMouseEvent(QEvent *event);
   virtual void ungrabMouseEvent(QEvent *event);
   virtual void grabKeyboardEvent(QEvent *event);
   virtual void ungrabKeyboardEvent(QEvent *event);

 private:
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QGraphicsWidget)

   friend class QGraphicsScene;
   friend class QGraphicsScenePrivate;
   friend class QGraphicsView;
   friend class QGraphicsItem;
   friend class QGraphicsItemPrivate;
   friend class QGraphicsLayout;
   friend class QWidget;
   friend class QApplication;
};

inline void QGraphicsWidget::setGeometry(qreal x, qreal y, qreal width, qreal height)
{
   setGeometry(QRectF(x, y, width, height));
}

inline void QGraphicsWidget::cs_resize(const QSizeF &size)
{
   resize(size);
}

inline void QGraphicsWidget::cs_setGeometry(const QRectF &size)
{
   setGeometry(size);
}

inline void QGraphicsWidget::cs_setMinimumSize(const QSizeF &size)
{
   setMinimumSize(size);
}

inline void QGraphicsWidget::cs_setPreferredSize(const QSizeF &size)
{
   setPreferredSize(size);
}

inline void QGraphicsWidget::cs_setMaximumSize(const QSizeF &size)
{
   setMaximumSize(size);
}

inline void QGraphicsWidget::cs_setSizePolicy(const QSizePolicy &policy)
{
   setSizePolicy(policy);
}

#endif

#endif

