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

#ifndef QGRAPHICSLAYOUT_H
#define QGRAPHICSLAYOUT_H

#include <QtGui/qgraphicslayoutitem.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_GRAPHICSVIEW)

class QGraphicsLayoutPrivate;
class QGraphicsLayoutItem;
class QGraphicsWidget;

class Q_GUI_EXPORT QGraphicsLayout : public QGraphicsLayoutItem
{
 public:
   QGraphicsLayout(QGraphicsLayoutItem *parent = 0);
   ~QGraphicsLayout();

   void setContentsMargins(qreal left, qreal top, qreal right, qreal bottom);
   void getContentsMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const override;

   void activate();
   bool isActivated() const;
   virtual void invalidate();
   void updateGeometry() override;

   virtual void widgetEvent(QEvent *e);

   virtual int count() const = 0;
   virtual QGraphicsLayoutItem *itemAt(int i) const = 0;
   virtual void removeAt(int index) = 0;

   static void setInstantInvalidatePropagation(bool enable);
   static bool instantInvalidatePropagation();

 protected:
   QGraphicsLayout(QGraphicsLayoutPrivate &, QGraphicsLayoutItem *);
   void addChildLayoutItem(QGraphicsLayoutItem *layoutItem);

 private:
   Q_DISABLE_COPY(QGraphicsLayout)
   Q_DECLARE_PRIVATE(QGraphicsLayout)
   friend class QGraphicsWidget;
};

CS_DECLARE_INTERFACE(QGraphicsLayout, "com.copperspice.QGraphicsLayout")

#endif

QT_END_NAMESPACE

#endif

