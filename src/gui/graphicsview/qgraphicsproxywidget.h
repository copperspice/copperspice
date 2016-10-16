/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QGRAPHICSPROXYWIDGET_H
#define QGRAPHICSPROXYWIDGET_H

#include <QtGui/qgraphicswidget.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_GRAPHICSVIEW)

class QGraphicsProxyWidgetPrivate;

class Q_GUI_EXPORT QGraphicsProxyWidget : public QGraphicsWidget
{
   GUI_CS_OBJECT(QGraphicsProxyWidget)

 public:
   QGraphicsProxyWidget(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
   ~QGraphicsProxyWidget();

   void setWidget(QWidget *widget);
   QWidget *widget() const;

   QRectF subWidgetRect(const QWidget *widget) const;

   void setGeometry(const QRectF &rect) override;

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

   enum {
      Type = 12
   };
   int type() const override;

   QGraphicsProxyWidget *createProxyForChildWidget(QWidget *child);

 protected:
   QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

   bool event(QEvent *event) override;
   bool eventFilter(QObject *object, QEvent *event) override;

   void showEvent(QShowEvent *event) override;
   void hideEvent(QHideEvent *event) override;

#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
#endif

#ifndef QT_NO_DRAGANDDROP
   void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
   void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
   void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
   void dropEvent(QGraphicsSceneDragDropEvent *event) override;
#endif

   void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
   void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
   void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
   void grabMouseEvent(QEvent *event) override;
   void ungrabMouseEvent(QEvent *event) override;

   void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
   void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
   void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QGraphicsSceneWheelEvent *event) override;
#endif

   void keyPressEvent(QKeyEvent *event) override;
   void keyReleaseEvent(QKeyEvent *event) override;

   void focusInEvent(QFocusEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   bool focusNextPrevChild(bool next) override;
   // ### Qt 4.5:
   // QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
   // void inputMethodEvent(QInputMethodEvent *event);

   QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override;
   void resizeEvent(QGraphicsSceneResizeEvent *event) override;

   GUI_CS_SLOT_1(Protected, QGraphicsProxyWidget *newProxyWidget(const QWidget *un_named_arg1))
   GUI_CS_SLOT_2(newProxyWidget)

 private:
   Q_DISABLE_COPY(QGraphicsProxyWidget)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QGraphicsProxyWidget)

   GUI_CS_SLOT_1(Private, void _q_removeWidgetSlot())
   GUI_CS_SLOT_2(_q_removeWidgetSlot)

   friend class QWidget;
   friend class QWidgetPrivate;
   friend class QGraphicsItem;
};

#endif

QT_END_NAMESPACE

#endif

