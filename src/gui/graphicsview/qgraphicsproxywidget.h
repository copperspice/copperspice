/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

   void setGeometry(const QRectF &rect);

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

   enum {
      Type = 12
   };
   int type() const;

   QGraphicsProxyWidget *createProxyForChildWidget(QWidget *child);

 protected:
   QVariant itemChange(GraphicsItemChange change, const QVariant &value);

   bool event(QEvent *event);
   bool eventFilter(QObject *object, QEvent *event);

   void showEvent(QShowEvent *event);
   void hideEvent(QHideEvent *event);

#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
#endif

#ifndef QT_NO_DRAGANDDROP
   void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
   void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
   void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
   void dropEvent(QGraphicsSceneDragDropEvent *event);
#endif

   void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
   void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
   void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
   void grabMouseEvent(QEvent *event);
   void ungrabMouseEvent(QEvent *event);

   void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
   void mousePressEvent(QGraphicsSceneMouseEvent *event);
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
   void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QGraphicsSceneWheelEvent *event);
#endif

   void keyPressEvent(QKeyEvent *event);
   void keyReleaseEvent(QKeyEvent *event);

   void focusInEvent(QFocusEvent *event);
   void focusOutEvent(QFocusEvent *event);
   bool focusNextPrevChild(bool next);
   // ### Qt 4.5:
   // QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
   // void inputMethodEvent(QInputMethodEvent *event);

   QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
   void resizeEvent(QGraphicsSceneResizeEvent *event);

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

