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

#ifndef QGRAPHICS_SCENE_BSP_P_H
#define QGRAPHICS_SCENE_BSP_P_H

#include <qlist.h>

#if !defined(QT_NO_GRAPHICSVIEW)

#include <qrect.h>
#include <qset.h>
#include <qvector.h>

class QGraphicsItem;
class QGraphicsSceneBspTreeVisitor;
class QGraphicsSceneInsertItemBspTreeVisitor;
class QGraphicsSceneRemoveItemBspTreeVisitor;
class QGraphicsSceneFindItemBspTreeVisitor;

class QGraphicsSceneBspTree
{
 public:
   struct Node {
      enum Type {
         Horizontal,
         Vertical,
         Leaf
      };

      union {
         qreal offset;
         int leafIndex;
      };

      Type type;
   };

   QGraphicsSceneBspTree();
   ~QGraphicsSceneBspTree();

   void initialize(const QRectF &rect, int depth);
   void clear();

   void insertItem(QGraphicsItem *item, const QRectF &rect);
   void removeItem(QGraphicsItem *item, const QRectF &rect);
   void removeItems(const QSet<QGraphicsItem *> &items);

   QList<QGraphicsItem *> items(const QRectF &rect, bool onlyTopLevelItems = false) const;
   int leafCount() const;

   inline int firstChildIndex(int index) const {
      return index * 2 + 1;
   }

   inline int parentIndex(int index) const {
      return index > 0 ? ((index & 1) ? ((index - 1) / 2) : ((index - 2) / 2)) : -1;
   }

   QString debug(int index) const;

 private:
   void initialize(const QRectF &rect, int depth, int index);
   void climbTree(QGraphicsSceneBspTreeVisitor *visitor, const QRectF &rect, int index = 0) const;
   QRectF rectForIndex(int index) const;

   QVector<Node> nodes;
   QVector<QList<QGraphicsItem *>> leaves;
   int leafCnt;
   QRectF rect;

   QGraphicsSceneInsertItemBspTreeVisitor *insertVisitor;
   QGraphicsSceneRemoveItemBspTreeVisitor *removeVisitor;
   QGraphicsSceneFindItemBspTreeVisitor *findVisitor;
};

class QGraphicsSceneBspTreeVisitor
{
 public:
   virtual ~QGraphicsSceneBspTreeVisitor() { }
   virtual void visit(QList<QGraphicsItem *> *items) = 0;
};
#endif // QT_NO_GRAPHICSVIEW

#endif // QGRAPHICSSCENEBSPTREE_P_H
