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

#ifndef QGRAPHICSLAYOUTITEM_H
#define QGRAPHICSLAYOUTITEM_H

#include <qscopedpointer.h>
#include <qsizepolicy.h>
#include <qevent.h>

#if ! defined(QT_NO_GRAPHICSVIEW)

class QGraphicsLayoutItemPrivate;
class QGraphicsItem;

class Q_GUI_EXPORT QGraphicsLayoutItem
{
 public:
   QGraphicsLayoutItem(QGraphicsLayoutItem *parent = nullptr, bool isLayout = false);
   virtual ~QGraphicsLayoutItem();

   void setSizePolicy(const QSizePolicy &policy);
   void setSizePolicy(QSizePolicy::Policy hPolicy, QSizePolicy::Policy vPolicy,
      QSizePolicy::ControlType controlType = QSizePolicy::DefaultType);

   QSizePolicy sizePolicy() const;

   void setMinimumSize(const QSizeF &size);
   inline void setMinimumSize(qreal w, qreal h);
   QSizeF minimumSize() const;
   void setMinimumWidth(qreal width);
   inline qreal minimumWidth() const;
   void setMinimumHeight(qreal height);
   inline qreal minimumHeight() const;

   void setPreferredSize(const QSizeF &size);
   inline void setPreferredSize(qreal w, qreal h);
   QSizeF preferredSize() const;
   void setPreferredWidth(qreal width);
   inline qreal preferredWidth() const;
   void setPreferredHeight(qreal height);
   inline qreal preferredHeight() const;

   void setMaximumSize(const QSizeF &size);
   inline void setMaximumSize(qreal w, qreal h);
   QSizeF maximumSize() const;
   void setMaximumWidth(qreal width);
   inline qreal maximumWidth() const;
   void setMaximumHeight(qreal height);
   inline qreal maximumHeight() const;

   virtual void setGeometry(const QRectF &rect);
   QRectF geometry() const;
   virtual void getContentsMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const;
   QRectF contentsRect() const;

   QSizeF effectiveSizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

   virtual void updateGeometry();  //### rename to sizeHintChanged()

   QGraphicsLayoutItem *parentLayoutItem() const;
   void setParentLayoutItem(QGraphicsLayoutItem *parent);

   bool isLayout() const;
   // TODO: Make automatic reparenting work regardless of item/object/widget type.
   QGraphicsItem *graphicsItem() const;
   bool ownedByLayout() const;

 protected:
   void setGraphicsItem(QGraphicsItem *item);
   void setOwnedByLayout(bool owner);
   QGraphicsLayoutItem(QGraphicsLayoutItemPrivate &dd);

   virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const = 0;
   QScopedPointer<QGraphicsLayoutItemPrivate> d_ptr;

 private:
   QSizeF *effectiveSizeHints(const QSizeF &constraint) const;
   Q_DECLARE_PRIVATE(QGraphicsLayoutItem)

   friend class QGraphicsLayout;
};

CS_DECLARE_INTERFACE(QGraphicsLayoutItem, "com.copperspice.QGraphicsLayoutItem")

inline void QGraphicsLayoutItem::setMinimumSize(qreal w, qreal h)
{
   setMinimumSize(QSizeF(w, h));
}

inline void QGraphicsLayoutItem::setPreferredSize(qreal w, qreal h)
{
   setPreferredSize(QSizeF(w, h));
}

inline void QGraphicsLayoutItem::setMaximumSize(qreal w, qreal h)
{
   setMaximumSize(QSizeF(w, h));
}

inline qreal QGraphicsLayoutItem::minimumWidth() const
{
   return effectiveSizeHint(Qt::MinimumSize).width();
}

inline qreal QGraphicsLayoutItem::minimumHeight() const
{
   return effectiveSizeHint(Qt::MinimumSize).height();
}

inline qreal QGraphicsLayoutItem::preferredWidth() const
{
   return effectiveSizeHint(Qt::PreferredSize).width();
}

inline qreal QGraphicsLayoutItem::preferredHeight() const
{
   return effectiveSizeHint(Qt::PreferredSize).height();
}

inline qreal QGraphicsLayoutItem::maximumWidth() const
{
   return effectiveSizeHint(Qt::MaximumSize).width();
}

inline qreal QGraphicsLayoutItem::maximumHeight() const
{
   return effectiveSizeHint(Qt::MaximumSize).height();
}

#endif


#endif
