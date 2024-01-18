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

#ifndef QGRAPHICS_SCENELINEARINDEX_P_H
#define QGRAPHICS_SCENELINEARINDEX_P_H

#include <qglobal.h>

#if ! defined(QT_NO_GRAPHICSVIEW)

#include <qrect.h>
#include <qlist.h>
#include <qgraphicsitem.h>

#include <qgraphics_sceneindex_p.h>

class QGraphicsSceneLinearIndex : public QGraphicsSceneIndex
{
   GUI_CS_OBJECT(QGraphicsSceneLinearIndex)

 public:
   QGraphicsSceneLinearIndex(QGraphicsScene *scene = nullptr)
      : QGraphicsSceneIndex(scene)
   {
   }

   QList<QGraphicsItem *> items(Qt::SortOrder order = Qt::DescendingOrder) const override {
      (void) order;
      return m_items;
   }

   QList<QGraphicsItem *> estimateItems(const QRectF &rect, Qt::SortOrder order) const override {
      (void) rect;
      (void) order;

      return m_items;
   }

 protected :
   void clear() override {
      m_items.clear();
      m_numSortedElements = 0;
   }

   void addItem(QGraphicsItem *item) override {
      m_items << item;
   }

   void removeItem(QGraphicsItem *item) override {
      // Sort m_items if needed
      if (m_numSortedElements < m_items.size()) {
         std::sort(m_items.begin() + m_numSortedElements, m_items.end() );
         std::inplace_merge(m_items.begin(), m_items.begin() + m_numSortedElements, m_items.end());
         m_numSortedElements = m_items.size();
      }

      QList<QGraphicsItem *>::iterator element = std::lower_bound(m_items.begin(), m_items.end(), item);
      if (element != m_items.end() && *element == item) {
         m_items.erase(element);
         --m_numSortedElements;
      }
   }

 private:
   QList<QGraphicsItem *> m_items;
   int m_numSortedElements;
};

#endif // QT_NO_GRAPHICSVIEW

#endif // QGRAPHICSSCENELINEARINDEX_H
