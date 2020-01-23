/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QGRAPHICSGRIDLAYOUTENGINE_P_H
#define QGRAPHICSGRIDLAYOUTENGINE_P_H

#include <qgridlayoutengine_p.h>

#ifndef QT_NO_GRAPHICSVIEW

#include <qsizepolicy.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qgraphicslayoutitem.h>

QT_BEGIN_NAMESPACE

class QGraphicsLayoutPrivate;

class QGraphicsGridLayoutEngineItem : public QGridLayoutItem
{
 public:
   QGraphicsGridLayoutEngineItem(QGraphicsLayoutItem *item, int row, int columns, int rowSpan = 1, int columnSpan = 1,
      Qt::Alignment alignment = 0)
      : QGridLayoutItem(row, columns, rowSpan, columnSpan, alignment), q_layoutItem(item) {}

   virtual QLayoutPolicy::Policy sizePolicy(Qt::Orientation orientation) const override {
      QSizePolicy sizePolicy(q_layoutItem->sizePolicy());
      return (QLayoutPolicy::Policy)((orientation == Qt::Horizontal) ? sizePolicy.horizontalPolicy()
            : sizePolicy.verticalPolicy());
   }

   virtual QLayoutPolicy::ControlTypes controlTypes(LayoutSide) const override {
      const QSizePolicy::ControlType ct = q_layoutItem->sizePolicy().controlType();
      return (QLayoutPolicy::ControlTypes)ct;
   }

   virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const override {
      return q_layoutItem->effectiveSizeHint(which, constraint);
   }

   bool isHidden() const;

   virtual bool isIgnored() const override;

   virtual void setGeometry(const QRectF &rect) override {
      q_layoutItem->setGeometry(rect);
   }

   virtual bool hasDynamicConstraint() const override;
   virtual Qt::Orientation dynamicConstraintOrientation() const override;

   QGraphicsLayoutItem *layoutItem() const {
      return q_layoutItem;
   }

 protected:
   QGraphicsLayoutItem *q_layoutItem;
};


class QGraphicsGridLayoutEngine : public QGridLayoutEngine
{
 public:
   QGraphicsGridLayoutEngineItem *findLayoutItem(QGraphicsLayoutItem *layoutItem) const {
      const int index = indexOf(layoutItem);
      if (index < 0) {
         return 0;
      }
      return static_cast<QGraphicsGridLayoutEngineItem *>(q_items.at(index));
   }

   int indexOf(QGraphicsLayoutItem *item) const {
      for (int i = 0; i < q_items.count(); ++i) {
         if (item == static_cast<QGraphicsGridLayoutEngineItem *>(q_items.at(i))->layoutItem()) {
            return i;
         }
      }
      return -1;
   }

   void setAlignment(QGraphicsLayoutItem *graphicsLayoutItem, Qt::Alignment alignment);
   Qt::Alignment alignment(QGraphicsLayoutItem *graphicsLayoutItem) const;

   void setStretchFactor(QGraphicsLayoutItem *layoutItem, int stretch, Qt::Orientation orientation);
   int stretchFactor(QGraphicsLayoutItem *layoutItem, Qt::Orientation orientation) const;

};

QT_END_NAMESPACE

#endif // QT_NO_GRAPHICSVIEW

#endif // QGRAPHICSGRIDLAYOUTENGINE_P_H
