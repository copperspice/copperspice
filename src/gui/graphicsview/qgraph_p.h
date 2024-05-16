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

#ifndef QGRAPH_P_H
#define QGRAPH_P_H

#include <qhash.h>
#include <qqueue.h>
#include <qstring.h>
#include <qdebug.h>

#include <float.h>

template <typename Vertex, typename EdgeData>
class Graph
{
 public:
   Graph() {}

   class const_iterator
   {
    public:
      const_iterator(const Graph *graph, bool begin) : g(graph) {
         if (begin) {
            row = g->m_graph.constBegin();
            //test if the graph is empty
            if (row != g->m_graph.constEnd()) {
               column = (*row)->constBegin();
            }
         } else {
            row = g->m_graph.constEnd();
         }
      }

      inline Vertex *operator*() {
         return column.key();
      }

      inline Vertex *from() const {
         return row.key();
      }

      inline Vertex *to() const {
         return column.key();
      }

      inline bool operator==(const const_iterator &o) const {
         return !(*this != o);
      }
      inline bool operator!=(const const_iterator &o) const {
         if (row ==  g->m_graph.end()) {
            return row != o.row;
         } else {
            return row != o.row || column != o.column;
         }
      }
      inline const_iterator &operator=(const const_iterator &o) const {
         row = o.row;
         column = o.column;
         return *this;
      }

      // prefix
      const_iterator &operator++() {
         if (row != g->m_graph.constEnd()) {
            ++column;
            if (column == (*row)->constEnd()) {
               ++row;
               if (row != g->m_graph.constEnd()) {
                  column = (*row)->constBegin();
               }
            }
         }
         return *this;
      }

    private:
      const Graph *g;
      typename QHash<Vertex *, QHash<Vertex *, EdgeData *> * >::const_iterator row;
      typename QHash<Vertex *, EdgeData *>::const_iterator column;
   };

   const_iterator constBegin() const {
      return const_iterator(this, true);
   }

   const_iterator constEnd() const {
      return const_iterator(this, false);
   }

   EdgeData *edgeData(Vertex *first, Vertex *second) {
      QHash<Vertex *, EdgeData *> *row = m_graph.value(first);
      return row ? row->value(second) : nullptr;
   }

   void createEdge(Vertex *first, Vertex *second, EdgeData *data) {
      // Creates a bidirectional edge

      createDirectedEdge(first, second, data);
      createDirectedEdge(second, first, data);
   }

   void removeEdge(Vertex *first, Vertex *second) {
      // Removes a bidirectional edge

      EdgeData *data = edgeData(first, second);
      removeDirectedEdge(first, second);
      removeDirectedEdge(second, first);
      if (data) {
         delete data;
      }
   }

   EdgeData *takeEdge(Vertex *first, Vertex *second) {
      // Removes a bidirectional edge
      EdgeData *data = edgeData(first, second);
      if (data) {
         removeDirectedEdge(first, second);
         removeDirectedEdge(second, first);
      }
      return data;
   }

   QList<Vertex *> adjacentVertices(Vertex *vertex) const {
      QHash<Vertex *, EdgeData *> *row = m_graph.value(vertex);
      QList<Vertex *> l;
      if (row) {
         l = row->keys();
      }
      return l;
   }

   QSet<Vertex *> vertices() const {
      QSet<Vertex *> setOfVertices;
      for (const_iterator it = constBegin(); it != constEnd(); ++it) {
         setOfVertices.insert(*it);
      }
      return setOfVertices;
   }

   QList<QPair<Vertex *, Vertex *>> connections() const {
      QList<QPair<Vertex *, Vertex *>> conns;
      for (const_iterator it = constBegin(); it != constEnd(); ++it) {
         Vertex *from = it.from();
         Vertex *to = it.to();
         // do not return (from,to) *and* (to,from)
         if (from < to) {
            conns.append(qMakePair(from, to));
         }
      }
      return conns;
   }

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
   QString serializeToDot() {
      // traversal

      QString strVertices;
      QString edges;

      QSet<Vertex *> setOfVertices = vertices();
      for (typename QSet<Vertex *>::const_iterator it = setOfVertices.begin(); it != setOfVertices.end(); ++it) {
         Vertex *v = *it;
         QList<Vertex *> adjacents = adjacentVertices(v);

         for (int i = 0; i < adjacents.count(); ++i) {
            Vertex *v1 = adjacents.at(i);
            EdgeData *data = edgeData(v, v1);
            bool forward = data->from == v;

            if (forward) {
               edges += QString::fromLatin1("\"%1\"->\"%2\" [label=\"[%3,%4,%5,%6,%7]\" color=\"#000000\"] \n")
                  .formatArg(v->toString())
                  .formatArg(v1->toString())
                  .formatArg(data->minSize)
                  .formatArg(data->minPrefSize)
                  .formatArg(data->prefSize)
                  .formatArg(data->maxPrefSize)
                  .formatArg(data->maxSize)
                  ;
            }
         }
         strVertices += QString::fromLatin1("\"%1\" [label=\"%2\"]\n").formatArg(v->toString()).formatArg(v->toString());
      }
      return QString::fromLatin1("%1\n%2\n").formatArg(strVertices).formatArg(edges);
   }
#endif

 protected:
   void createDirectedEdge(Vertex *from, Vertex *to, EdgeData *data) {
      QHash<Vertex *, EdgeData *> *adjacentToFirst = m_graph.value(from);
      if (!adjacentToFirst) {
         adjacentToFirst = new QHash<Vertex *, EdgeData *>();
         m_graph.insert(from, adjacentToFirst);
      }
      adjacentToFirst->insert(to, data);
   }

   void removeDirectedEdge(Vertex *from, Vertex *to) {
      QHash<Vertex *, EdgeData *> *adjacentToFirst = m_graph.value(from);
      Q_ASSERT(adjacentToFirst);

      adjacentToFirst->remove(to);
      if (adjacentToFirst->isEmpty()) {
         //nobody point to 'from' so we can remove it from the graph
         m_graph.remove(from);
         delete adjacentToFirst;
      }
   }

 private:
   QHash<Vertex *, QHash<Vertex *, EdgeData *> *> m_graph;
};



#endif
