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

#ifndef QGRAPHICSLINEARLAYOUT_H
#define QGRAPHICSLINEARLAYOUT_H

#include <QtGui/qgraphicsitem.h>
#include <QtGui/qgraphicslayout.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_GRAPHICSVIEW)

class QGraphicsLinearLayoutPrivate;

class Q_GUI_EXPORT QGraphicsLinearLayout : public QGraphicsLayout
{
 public:
   QGraphicsLinearLayout(QGraphicsLayoutItem *parent = 0);
   QGraphicsLinearLayout(Qt::Orientation orientation, QGraphicsLayoutItem *parent = 0);
   virtual ~QGraphicsLinearLayout();

   void setOrientation(Qt::Orientation orientation);
   Qt::Orientation orientation() const;

   inline void addItem(QGraphicsLayoutItem *item) {
      insertItem(-1, item);
   }
   inline void addStretch(int stretch = 1) {
      insertStretch(-1, stretch);
   }

   void insertItem(int index, QGraphicsLayoutItem *item);
   void insertStretch(int index, int stretch = 1);

   void removeItem(QGraphicsLayoutItem *item);
   void removeAt(int index);

   void setSpacing(qreal spacing);
   qreal spacing() const;
   void setItemSpacing(int index, qreal spacing);
   qreal itemSpacing(int index) const;

   void setStretchFactor(QGraphicsLayoutItem *item, int stretch);
   int stretchFactor(QGraphicsLayoutItem *item) const;

   void setAlignment(QGraphicsLayoutItem *item, Qt::Alignment alignment);
   Qt::Alignment alignment(QGraphicsLayoutItem *item) const;

   void setGeometry(const QRectF &rect);

   int count() const;
   QGraphicsLayoutItem *itemAt(int index) const;

   void invalidate();
   QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

   void dump(int indent = 0) const;

 private:
   Q_DISABLE_COPY(QGraphicsLinearLayout)
   Q_DECLARE_PRIVATE(QGraphicsLinearLayout)
};

#endif

QT_END_NAMESPACE

#endif

