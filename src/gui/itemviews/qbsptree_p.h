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

#ifndef QBSPTREE_P_H
#define QBSPTREE_P_H

#include <qrect.h>
#include <qvector.h>

class QBspTree
{
 public:

   struct Node {
      enum Type {
         None            = 0,
         VerticalPlane   = 1,
         HorizontalPlane = 2,
         Both            = 3
      };

      Node()
         : pos(0), type(None)
      { }

      int pos;
      Type type;
   };

   typedef Node::Type NodeType;

   struct Data {
      Data(void *p)
         : ptr(p)
      { }

      Data(int n)
         : i(n)
      { }

      union {
         void *ptr;
         int i;
      };
   };

   typedef QBspTree::Data QBspTreeData;
   typedef void callback(QVector<int> &leaf, const QRect &area, uint visited, QBspTreeData data);

   QBspTree();

   void create(int n, int d = -1);
   void destroy();

   void init(const QRect &area, NodeType type) {
      init(area, depth, type, 0);
   }

   void climbTree(const QRect &rect, callback *function, QBspTreeData data);

   int leafCount() const {
      return leaves.count();
   }

   QVector<int> &leaf(int i) {
      return leaves[i];
   }

   void insertLeaf(const QRect &r, int i) {
      climbTree(r, &insert, i, 0);
   }

   void removeLeaf(const QRect &r, int i) {
      climbTree(r, &remove, i, 0);
   }

 protected:
   void init(const QRect &area, int depth, NodeType type, int index);
   void climbTree(const QRect &rect, callback *function, QBspTreeData data, int index);

   int parentIndex(int i) const {
      return (i & 1) ? ((i - 1) / 2) : ((i - 2) / 2);
   }

   int firstChildIndex(int i) const {
      return ((i * 2) + 1);
   }

   static void insert(QVector<int> &leaf, const QRect &area, uint visited, QBspTreeData data);
   static void remove(QVector<int> &leaf, const QRect &area, uint visited, QBspTreeData data);

 private:
   uint depth;
   mutable uint visited;
   QVector<Node> nodes;
   mutable QVector< QVector<int>> leaves; // the leaves are just indices into the items
};

#endif // QBSPTREE_P_H
