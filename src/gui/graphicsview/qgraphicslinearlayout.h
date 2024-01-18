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

#ifndef QGRAPHICSLINEARLAYOUT_H
#define QGRAPHICSLINEARLAYOUT_H

#include <qgraphicsitem.h>
#include <qgraphicslayout.h>

#if ! defined(QT_NO_GRAPHICSVIEW)

class QGraphicsLinearLayoutPrivate;

class Q_GUI_EXPORT QGraphicsLinearLayout : public QGraphicsLayout
{
 public:
   QGraphicsLinearLayout(QGraphicsLayoutItem *parent = nullptr);
   QGraphicsLinearLayout(Qt::Orientation orientation, QGraphicsLayoutItem *parent = nullptr);

   QGraphicsLinearLayout(const QGraphicsLinearLayout &) = delete;
   QGraphicsLinearLayout &operator=(const QGraphicsLinearLayout &) = delete;

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
   void removeAt(int index) override;

   void setSpacing(qreal spacing);
   qreal spacing() const;
   void setItemSpacing(int index, qreal spacing);
   qreal itemSpacing(int index) const;

   void setStretchFactor(QGraphicsLayoutItem *item, int stretch);
   int stretchFactor(QGraphicsLayoutItem *item) const;

   void setAlignment(QGraphicsLayoutItem *item, Qt::Alignment alignment);
   Qt::Alignment alignment(QGraphicsLayoutItem *item) const;

   void setGeometry(const QRectF &rect) override;

   int count() const override;
   QGraphicsLayoutItem *itemAt(int index) const override;

   void invalidate() override;
   QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override;

   void dump(int indent = 0) const;

 private:
   Q_DECLARE_PRIVATE(QGraphicsLinearLayout)
};

#endif

#endif

