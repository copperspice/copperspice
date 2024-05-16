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

#ifndef QGRAPHICS_ANCHORLAYOUT_P_H
#define QGRAPHICS_ANCHORLAYOUT_P_H

#include <qmultihash.h>
#include <qgraphicswidget.h>
#include <qgraphicsanchorlayout.h>

#include <qgraphics_layout_p.h>
#include <qgraph_p.h>
#include <qsimplex_p.h>

#ifndef QT_NO_GRAPHICSVIEW

// internal
struct AnchorVertex {
   enum Type {
      Normal = 0,
      Pair
   };

   AnchorVertex(QGraphicsLayoutItem *item, Qt::AnchorPoint edge)
      : m_item(item), m_edge(edge), m_type(Normal) {}

   AnchorVertex()
      : m_item(nullptr), m_edge(Qt::AnchorPoint(0)), m_type(Normal) {}

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
   inline QString toString() const;
#endif

   QGraphicsLayoutItem *m_item;
   Qt::AnchorPoint m_edge;
   uint m_type : 1;

   // Current distance from this vertex to the layout edge (Left or Top)
   // Value is calculated from the current anchors sizes.
   qreal distance;
};

// internal
// Represents an edge (anchor) in the internal graph.

struct AnchorData : public QSimplexVariable {
   enum Type {
      Normal = 0,
      Sequential,
      Parallel
   };

   enum Dependency {
      Independent = 0,
      Master,
      Slave
   };

   AnchorData()
      : QSimplexVariable(), from(nullptr), to(nullptr), minSize(0), prefSize(0), maxSize(0),
        minPrefSize(0), maxPrefSize(0), sizeAtMinimum(0), sizeAtPreferred(0),
        sizeAtMaximum(0), item(nullptr), graphicsAnchor(nullptr), type(Normal), isLayoutAnchor(false),
        isCenterAnchor(false), orientation(0), dependency(Independent)
   {
   }

   virtual ~AnchorData();

   virtual void updateChildrenSizes() {}
   void refreshSizeHints(const QLayoutStyleInfo *styleInfo = nullptr);

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
   void dump(int indent = 2);
   inline QString toString() const;
   QString name;
#endif

   // Anchor is semantically directed
   AnchorVertex *from;
   AnchorVertex *to;

   // Nominal sizes
   // These are the intrinsic size restrictions for a given item. They are
   // used as input for the calculation of the actual sizes.
   // These values are filled by the refreshSizeHints method, based on the
   // anchor size policy, the size hints of the item it (possibly) represents
   // and the layout spacing information.
   qreal minSize;
   qreal prefSize;
   qreal maxSize;

   qreal minPrefSize;
   qreal maxPrefSize;

   // Calculated sizes
   // These attributes define which sizes should that anchor be in when the
   // layout is at its minimum, preferred or maximum sizes. Values are
   // calculated by the Simplex solver based on the current layout setup.
   qreal sizeAtMinimum;
   qreal sizeAtPreferred;
   qreal sizeAtMaximum;

   // References to the classes that represent this anchor in the public world
   // An anchor may represent a LayoutItem, it may also be acessible externally
   // through a GraphicsAnchor "handler".
   QGraphicsLayoutItem *item;
   QGraphicsAnchor *graphicsAnchor;

   uint type : 2;            // either Normal, Sequential or Parallel
   uint isLayoutAnchor : 1;  // if this anchor is an internal layout anchor
   uint isCenterAnchor : 1;
   uint orientation : 1;
   uint dependency : 2;      // either Independent, Master or Slave
};

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
inline QString AnchorData::toString() const
{
   return QString("Anchor(%1)").formatArg(name);
}
#endif

struct SequentialAnchorData : public AnchorData {
   SequentialAnchorData(const QVector<AnchorVertex *> &vertices, const QVector<AnchorData *> &edges)
      : AnchorData(), m_children(vertices), m_edges(edges) {
      type = AnchorData::Sequential;
      orientation = m_edges.at(0)->orientation;

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
      name = QString::fromLatin1("%1 -- %2").formatArgs(vertices.first()->toString(), vertices.last()->toString());
#endif
   }

   void updateChildrenSizes() override;
   void calculateSizeHints();

   QVector<AnchorVertex *> m_children;         // list of vertices in the sequence
   QVector<AnchorData *> m_edges;              // keep the list of edges too.
};

struct ParallelAnchorData : public AnchorData {
   ParallelAnchorData(AnchorData *first, AnchorData *second)
      : AnchorData(), firstEdge(first), secondEdge(second) {
      type = AnchorData::Parallel;
      orientation = first->orientation;

      // This assert whether the child anchors share their vertices
      Q_ASSERT(((first->from == second->from) && (first->to == second->to)) ||
         ((first->from == second->to) && (first->to == second->from)));

      // Our convention will be that the parallel group anchor will have the same
      // direction as the first anchor.
      from = first->from;
      to = first->to;

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
      name = QString("%1 | %2").formatArgs(first->toString(), second->toString());
#endif

   }

   void updateChildrenSizes() override;
   bool calculateSizeHints();

   bool secondForward() const {
      // We have the convention that the first children will define the direction of the
      // pararell group. Note that we can't rely on 'this->from' or 'this->to'  because they
      // might be changed by vertex simplification.
      return firstEdge->from == secondEdge->from;
   }

   AnchorData *firstEdge;
   AnchorData *secondEdge;

   QList<QSimplexConstraint *> m_firstConstraints;
   QList<QSimplexConstraint *> m_secondConstraints;
};

struct AnchorVertexPair : public AnchorVertex {
   AnchorVertexPair(AnchorVertex *v1, AnchorVertex *v2, AnchorData *data)
      : AnchorVertex(), m_first(v1), m_second(v2), m_removedAnchor(data) {
      m_type = AnchorVertex::Pair;
   }

   AnchorVertex *m_first;
   AnchorVertex *m_second;

   AnchorData *m_removedAnchor;
   QList<AnchorData *> m_firstAnchors;
   QList<AnchorData *> m_secondAnchors;
};

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
inline QString AnchorVertex::toString() const
{
   if (m_type == Pair) {
      const AnchorVertexPair *vp = static_cast<const AnchorVertexPair *>(this);
      return QString::fromLatin1("(%1, %2)").formatArg(vp->m_first->toString()).formatArg(vp->m_second->toString());
   } else if (!m_item) {
      return QString::fromLatin1("NULL_%1").formatArg(quintptr(this));
   }

   QString edge;
   switch (m_edge) {
      case Qt::AnchorLeft:
         edge = QLatin1String("Left");
         break;
      case Qt::AnchorHorizontalCenter:
         edge = QLatin1String("HorizontalCenter");
         break;
      case Qt::AnchorRight:
         edge = QLatin1String("Right");
         break;
      case Qt::AnchorTop:
         edge = QLatin1String("Top");
         break;
      case Qt::AnchorVerticalCenter:
         edge = QLatin1String("VerticalCenter");
         break;
      case Qt::AnchorBottom:
         edge = QLatin1String("Bottom");
         break;
      default:
         edge = QLatin1String("None");
         break;
   }

   QString itemName;
   if (m_item->isLayout()) {
      itemName = QLatin1String("layout");
   } else {
      if (QGraphicsItem *item = m_item->graphicsItem()) {
         itemName = item->data(0).toString();
      }
   }
   edge.insert(0, QLatin1String("%1_"));
   return edge.formatArg(itemName);
}
#endif

class GraphPath
{
 public:
   GraphPath() {}

   QSimplexConstraint *constraint(const GraphPath &path) const;

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
   QString toString() const;
#endif

   QSet<AnchorData *> positives;
   QSet<AnchorData *> negatives;
};

class QGraphicsAnchorLayoutPrivate;

// internal
class QGraphicsAnchorPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsAnchor)

 public:
   explicit QGraphicsAnchorPrivate();
   virtual ~QGraphicsAnchorPrivate();

   void setSpacing(qreal value);
   void unsetSpacing();
   qreal spacing() const;

   void setSizePolicy(QSizePolicy::Policy policy);

   QGraphicsAnchorLayoutPrivate *layoutPrivate;
   AnchorData *data;

   // Size information for user controlled anchor
   QSizePolicy::Policy sizePolicy;
   qreal preferredSize;

   uint hasSize : 1;         // if false, get size from style.

 protected:
   QGraphicsAnchor *q_ptr;

};

class QGraphicsAnchorLayoutPrivate : public QGraphicsLayoutPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsAnchorLayout)

 public:
   // When the layout geometry is different from its Minimum, Preferred
   // or Maximum values, interpolation is used to calculate the geometries
   // of the items.
   //
   // Interval represents which interpolation interval are we operating in.
   enum Interval {
      MinimumToMinPreferred = 0,
      MinPreferredToPreferred,
      PreferredToMaxPreferred,
      MaxPreferredToMaximum
   };

   // Several structures internal to the layout are duplicated to handle
   // both Horizontal and Vertical restrictions.
   //
   // Orientation is used to reference the right structure in each context
   enum Orientation {
      Horizontal = 0,
      Vertical,
      NOrientations
   };

   QGraphicsAnchorLayoutPrivate();

   static QGraphicsAnchorLayoutPrivate *get(QGraphicsAnchorLayout *q) {
      return q ? q->d_func() : nullptr;
   }

   static Qt::AnchorPoint oppositeEdge(
      Qt::AnchorPoint edge);

   static Orientation edgeOrientation(Qt::AnchorPoint edge);

   static Qt::AnchorPoint pickEdge(Qt::AnchorPoint edge, Orientation orientation) {
      if (orientation == Vertical && int(edge) <= 2) {
         return (Qt::AnchorPoint)(edge + 3);
      } else if (orientation == Horizontal && int(edge) >= 3) {
         return (Qt::AnchorPoint)(edge - 3);
      }
      return edge;
   }

   // Init methods
   void createLayoutEdges();
   void deleteLayoutEdges();
   void createItemEdges(QGraphicsLayoutItem *item);
   void createCenterAnchors(QGraphicsLayoutItem *item, Qt::AnchorPoint centerEdge);
   void removeCenterAnchors(QGraphicsLayoutItem *item, Qt::AnchorPoint centerEdge, bool substitute = true);
   void removeCenterConstraints(QGraphicsLayoutItem *item, Orientation orientation);

   QGraphicsAnchor *acquireGraphicsAnchor(AnchorData *data) {
      Q_Q(QGraphicsAnchorLayout);
      if (!data->graphicsAnchor) {
         data->graphicsAnchor = new QGraphicsAnchor(q);
         data->graphicsAnchor->d_func()->data = data;
      }
      return data->graphicsAnchor;
   }

   // function used by the 4 API functions
   QGraphicsAnchor *addAnchor(QGraphicsLayoutItem *firstItem, Qt::AnchorPoint firstEdge, QGraphicsLayoutItem *secondItem,
      Qt::AnchorPoint secondEdge, qreal *spacing = nullptr);

   // Helper for Anchor Manipulation methods
   void addAnchor_helper(QGraphicsLayoutItem *firstItem, Qt::AnchorPoint firstEdge, QGraphicsLayoutItem *secondItem,
      Qt::AnchorPoint secondEdge, AnchorData *data);

   QGraphicsAnchor *getAnchor(QGraphicsLayoutItem *firstItem, Qt::AnchorPoint firstEdge,
      QGraphicsLayoutItem *secondItem, Qt::AnchorPoint secondEdge);

   void removeAnchor(AnchorVertex *firstVertex, AnchorVertex *secondVertex);
   void removeAnchor_helper(AnchorVertex *v1, AnchorVertex *v2);

   void removeAnchors(QGraphicsLayoutItem *item);

   void removeVertex(QGraphicsLayoutItem *item, Qt::AnchorPoint edge);

   void correctEdgeDirection(QGraphicsLayoutItem *&firstItem, Qt::AnchorPoint &firstEdge, QGraphicsLayoutItem *&secondItem,
      Qt::AnchorPoint &secondEdge);

   QLayoutStyleInfo &styleInfo() const;

   AnchorData *addAnchorMaybeParallel(AnchorData *newAnchor, bool *feasible);

   // Activation
   void calculateGraphs();
   void calculateGraphs(Orientation orientation);

   // Simplification
   bool simplifyGraph(Orientation orientation);
   bool simplifyVertices(Orientation orientation);
   bool simplifyGraphIteration(Orientation orientation, bool *feasible);

   bool replaceVertex(Orientation orientation, AnchorVertex *oldV, AnchorVertex *newV, const QList<AnchorData *> &edges);

   void restoreSimplifiedGraph(Orientation orientation);
   void restoreSimplifiedAnchor(AnchorData *edge);
   void restoreSimplifiedConstraints(ParallelAnchorData *parallel);
   void restoreVertices(Orientation orientation);

   bool calculateTrunk(Orientation orientation, const GraphPath &trunkPath, const QList<QSimplexConstraint *> &constraints,
      const QList<AnchorData *> &variables);

   bool calculateNonTrunk(const QList<QSimplexConstraint *> &constraints, const QList<AnchorData *> &variables);

   // Support functions for calculateGraph()
   void refreshAllSizeHints(Orientation orientation);
   void findPaths(Orientation orientation);
   void constraintsFromPaths(Orientation orientation);
   void updateAnchorSizes(Orientation orientation);

   QList<QSimplexConstraint *> constraintsFromSizeHints(const QList<AnchorData *> &anchors);
   QList<QList<QSimplexConstraint *>> getGraphParts(Orientation orientation);
   void identifyFloatItems(const QSet<AnchorData *> &visited, Orientation orientation);
   void identifyNonFloatItems_helper(const AnchorData *ad, QSet<QGraphicsLayoutItem *> *nonFloatingItemsIdentifiedSoFar);

   AnchorVertex *internalVertex(const QPair<QGraphicsLayoutItem *, Qt::AnchorPoint> &itemEdge) const {
      return m_vertexList.value(itemEdge).first;
   }

   AnchorVertex *internalVertex(const QGraphicsLayoutItem *item, Qt::AnchorPoint edge) const {
      return internalVertex(qMakePair(const_cast<QGraphicsLayoutItem *>(item), edge));
   }

   void changeLayoutVertex(Orientation orientation, AnchorVertex *oldV, AnchorVertex *newV) {
      if (layoutFirstVertex[orientation] == oldV) {
         layoutFirstVertex[orientation] = newV;

      } else if (layoutCentralVertex[orientation] == oldV) {
         layoutCentralVertex[orientation] = newV;

      } else if (layoutLastVertex[orientation] == oldV) {
         layoutLastVertex[orientation] = newV;
      }
   }


   AnchorVertex *addInternalVertex(QGraphicsLayoutItem *item, Qt::AnchorPoint edge);
   void removeInternalVertex(QGraphicsLayoutItem *item, Qt::AnchorPoint edge);

   // Geometry interpolation methods
   void setItemsGeometries(const QRectF &geom);

   void calculateVertexPositions(Orientation orientation);
   void setupEdgesInterpolation(Orientation orientation);
   void interpolateEdge(AnchorVertex *base, AnchorData *edge);

   // Linear Programming solver methods
   bool solveMinMax(const QList<QSimplexConstraint *> &constraints,
      GraphPath path, qreal *min, qreal *max);
   bool solvePreferred(const QList<QSimplexConstraint *> &constraints,
      const QList<AnchorData *> &variables);
   bool hasConflicts() const;

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
   void dumpGraph(const QString &name = QString());
#endif

   qreal spacings[NOrientations];

   // Size hints from simplex engine
   qreal sizeHints[2][3];

   // Items
   QVector<QGraphicsLayoutItem *> items;

   // Mapping between high level anchorage points (Item, Edge) to low level
   // ones (Graph Vertices)
   QHash<QPair<QGraphicsLayoutItem *, Qt::AnchorPoint>, QPair<AnchorVertex *, int>> m_vertexList;

   // Internal graph of anchorage points and anchors, for both orientations
   Graph<AnchorVertex, AnchorData> graph[2];

   AnchorVertex *layoutFirstVertex[2];
   AnchorVertex *layoutCentralVertex[2];
   AnchorVertex *layoutLastVertex[2];

   // Combined anchors in order of creation
   QList<AnchorVertexPair *> simplifiedVertices[2];
   QList<AnchorData *> anchorsFromSimplifiedVertices[2];

   // Graph paths and constraints, for both orientations
   QMultiHash<AnchorVertex *, GraphPath> graphPaths[2];
   QList<QSimplexConstraint *> constraints[2];
   QList<QSimplexConstraint *> itemCenterConstraints[2];

   // The interpolation interval and progress based on the current size
   // as well as the key values (minimum, preferred and maximum)
   Interval interpolationInterval[2];
   qreal interpolationProgress[2];

   bool graphHasConflicts[2];
   QSet<QGraphicsLayoutItem *> m_floatItems[2];

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
   bool lastCalculationUsedSimplex[2];
#endif

   uint calculateGraphCacheDirty : 1;
   mutable uint styleInfoDirty : 1;
   mutable QLayoutStyleInfo cachedStyleInfo;

   friend class QGraphicsAnchorPrivate;
};

#endif //QT_NO_GRAPHICSVIEW

#endif
