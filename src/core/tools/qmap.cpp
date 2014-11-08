/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#include <qmap.h>
#include <stdlib.h>

#ifdef QT_QMAP_DEBUG
# include <qstring.h>
# include <qvector.h>
#endif

QT_BEGIN_NAMESPACE

QMapData *QMapData::sharedNull()
{
   static const QMapData shared_null = {
      const_cast<QMapData *>(&shared_null),
      {   const_cast<QMapData *>(&shared_null), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      Q_REFCOUNT_INITIALIZE_STATIC, 0, 0, 0, false, true, false, 0
   };

   return const_cast<QMapData *>(&shared_null);
}

QMapData *QMapData::createData()
{
   return createData(0);
}

QMapData *QMapData::createData(int alignment)
{
   QMapData *d = new QMapData;
   Q_CHECK_PTR(d);
   Node *e = reinterpret_cast<Node *>(d);
   e->backward = e;
   e->forward[0] = e;
   d->ref.initializeOwned();
   d->topLevel = 0;
   d->size = 0;
   d->randomBits = 0;
   d->insertInOrder = false;
   d->sharable = true;
   d->strictAlignment = alignment > 8;
   d->reserved = 0;
   return d;
}

void QMapData::continueFreeData(int offset)
{
   Node *e = reinterpret_cast<Node *>(this);
   Node *cur = e->forward[0];
   Node *prev;
   while (cur != e) {
      prev = cur;
      cur = cur->forward[0];
      if (strictAlignment) {
         qFreeAligned(reinterpret_cast<char *>(prev) - offset);
      } else {
         free(reinterpret_cast<char *>(prev) - offset);
      }
   }
   delete this;
}

QMapData::Node *QMapData::node_create(Node *update[], int offset)
{
   return node_create(update, offset, 0);
}

/*!
    Creates a new node inside the data structure.

    \a update is an array with pointers to the node after which the new node
    should be inserted. Because of the strange skip list data structure there
    could be several pointers to this node on different levels.
    \a offset is an amount of bytes that needs to reserved just before the
    QMapData::Node structure.

    \a alignment dictates the alignment for the data.

    \internal
    \since 4.6
*/
QMapData::Node *QMapData::node_create(Node *update[], int offset, int alignment)
{
   int level = 0;
   uint mask = (1 << Sparseness) - 1;

   while ((randomBits & mask) == mask && level < LastLevel) {
      ++level;
      mask <<= Sparseness;
   }

   if (level > topLevel) {
      Node *e = reinterpret_cast<Node *>(this);
      level = ++topLevel;
      e->forward[level] = e;
      update[level] = e;
   }

   ++randomBits;
   if (level == 3 && !insertInOrder) {
      randomBits = qrand();
   }

   void *concreteNode = strictAlignment ?
                        qMallocAligned(offset + sizeof(Node) + level * sizeof(Node *), alignment) :
                        malloc(offset + sizeof(Node) + level * sizeof(Node *));
   Q_CHECK_PTR(concreteNode);

   Node *abstractNode = reinterpret_cast<Node *>(reinterpret_cast<char *>(concreteNode) + offset);

   abstractNode->backward = update[0];
   update[0]->forward[0]->backward = abstractNode;

   for (int i = level; i >= 0; i--) {
      abstractNode->forward[i] = update[i]->forward[i];
      update[i]->forward[i] = abstractNode;
      update[i] = abstractNode;
   }
   ++size;
   return abstractNode;
}

void QMapData::node_delete(Node *update[], int offset, Node *node)
{
   node->forward[0]->backward = node->backward;

   for (int i = 0; i <= topLevel; ++i) {
      if (update[i]->forward[i] != node) {
         break;
      }
      update[i]->forward[i] = node->forward[i];
   }
   --size;
   if (strictAlignment) {
      qFreeAligned(reinterpret_cast<char *>(node) - offset);
   } else {
      free(reinterpret_cast<char *>(node) - offset);
   }
}

#ifdef QT_QMAP_DEBUG

uint QMapData::adjust_ptr(Node *node)
{
   if (node == reinterpret_cast<Node *>(this)) {
      return (uint)0xDEADBEEF;
   } else {
      return (uint)node;
   }
}

void QMapData::dump()
{
   qDebug("Map data (ref = %d, size = %d, randomBits = %#.8x)", int(ref), size, randomBits);

   QString preOutput;
   QVector<QString> output(topLevel + 1);
   Node *e = reinterpret_cast<Node *>(this);

   QString str;
   str.sprintf("    %.8x", adjust_ptr(reinterpret_cast<Node *>(this)));
   preOutput += str;

   Node *update[LastLevel + 1];
   for (int i = 0; i <= topLevel; ++i) {
      str.sprintf("%d: [%.8x] -", i, adjust_ptr(reinterpret_cast<Node *>(forward[i])));
      output[i] += str;
      update[i] = reinterpret_cast<Node *>(forward[i]);
   }

   Node *node = reinterpret_cast<Node *>(forward[0]);
   while (node != e) {
      int level = 0;
      while (level < topLevel && update[level + 1] == node) {
         ++level;
      }

      str.sprintf("       %.8x", adjust_ptr(node));
      preOutput += str;

      for (int i = 0; i <= level; ++i) {
         str.sprintf("-> [%.8x] -", adjust_ptr(node->forward[i]));
         output[i] += str;
         update[i] = node->forward[i];
      }
      for (int j = level + 1; j <= topLevel; ++j) {
         output[j] += QLatin1String("---------------");
      }
      node = node->forward[0];
   }

   qDebug("%s", preOutput.ascii());
   for (int i = 0; i <= topLevel; ++i) {
      qDebug("%s", output[i].ascii());
   }
}

#endif

QT_END_NAMESPACE
