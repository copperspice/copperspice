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

#ifndef QGRAPHICSSCENELINEARINDEX_P_H
#define QGRAPHICSSCENELINEARINDEX_P_H

#include <QtCore/qglobal.h>

#if !defined(QT_NO_GRAPHICSVIEW)

#include <QtCore/qrect.h>
#include <QtCore/qlist.h>
#include <QtGui/qgraphicsitem.h>
#include <qgraphicssceneindex_p.h>

QT_BEGIN_NAMESPACE

class QGraphicsSceneLinearIndex : public QGraphicsSceneIndex
{
   GUI_CS_OBJECT(QGraphicsSceneLinearIndex)

 public:
   QGraphicsSceneLinearIndex(QGraphicsScene *scene = 0) : QGraphicsSceneIndex(scene) {
   }

   QList<QGraphicsItem *> items(Qt::SortOrder order = Qt::DescendingOrder) const override {
      Q_UNUSED(order);
      return m_items;
   }

   QList<QGraphicsItem *> estimateItems(const QRectF &rect, Qt::SortOrder order) const override {
      Q_UNUSED(rect);
      Q_UNUSED(order);
      return m_items;
   }

 protected :
   void clear() override {
      m_items.clear();
   }

   void addItem(QGraphicsItem *item) override {
      m_items << item;
   }

   void removeItem(QGraphicsItem *item) override {
      m_items.removeOne(item);
   }

 private:
   QList<QGraphicsItem *> m_items;
};

#endif // QT_NO_GRAPHICSVIEW

QT_END_NAMESPACE

#endif // QGRAPHICSSCENELINEARINDEX_H
