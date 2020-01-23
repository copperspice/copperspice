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

#ifndef QGRIDLAYOUTENGINE_P_H
#define QGRIDLAYOUTENGINE_P_H

#include <qalgorithms.h>
#include <qbitarray.h>
#include <qlist.h>
#include <qmap.h>
#include <qpair.h>
#include <qvector.h>
#include <qgraphicslayout_p.h>

#include <qlayoutpolicy_p.h>
#include <qabstractlayoutstyleinfo_p.h>

#include <float.h>

class QStyle;
class QWidget;

class QGraphicsLayoutItem;
class QGridLayoutRowInfo;
class QGridLayoutEngine;

// ### deal with Descent in a similar way
enum {
   MinimumSize = Qt::MinimumSize,
   PreferredSize = Qt::PreferredSize,
   MaximumSize = Qt::MaximumSize,
   NSizes
};

// do not reorder
enum {
   Hor,
   Ver,
   NOrientations
};

// do not reorder
enum LayoutSide {
   Left,
   Top,
   Right,
   Bottom
};

enum {
   NoConstraint,
   HorizontalConstraint,   // Width depends on the height
   VerticalConstraint,     // Height depends on the width
   UnknownConstraint,      // need to update cache
   UnfeasibleConstraint    // not feasible, it be has some items with Vertical and others with Horizontal constraints
};

template <typename T>
class QLayoutParameter
{
 public:
   enum State { Default, User, Cached };

   inline QLayoutParameter() : q_value(T()), q_state(Default) {}
   inline QLayoutParameter(T value, State state = Default) : q_value(value), q_state(state) {}

   inline void setUserValue(T value) {
      q_value = value;
      q_state = User;
   }
   inline void setCachedValue(T value) const {
      if (q_state != User) {
         q_value = value;
         q_state = Cached;
      }
   }

   inline T value() const {
      return q_value;
   }

   inline T value(T defaultValue) const {
      return isUser() ? q_value : defaultValue;
   }

   inline bool isDefault() const {
      return q_state == Default;
   }

   inline bool isUser() const {
      return q_state == User;
   }
   inline bool isCached() const {
      return q_state == Cached;
   }

 private:
   mutable T q_value;
   mutable State q_state;
};

class QStretchParameter : public QLayoutParameter<int>
{
 public:
   QStretchParameter() : QLayoutParameter<int>(-1) {}

};

class QGridLayoutBox
{
 public:
   inline QGridLayoutBox()
      : q_minimumSize(0), q_preferredSize(0), q_maximumSize(FLT_MAX),
        q_minimumDescent(-1), q_minimumAscent(-1) {}

   void add(const QGridLayoutBox &other, int stretch, qreal spacing);
   void combine(const QGridLayoutBox &other);
   void normalize();


   // This code could use the union-struct-array trick, but a compiler
   // bug prevents this from working.
   qreal q_minimumSize;
   qreal q_preferredSize;
   qreal q_maximumSize;
   qreal q_minimumDescent;
   qreal q_minimumAscent;

   inline qreal &q_sizes(int which) {
      qreal *t;
      switch (which) {
         case Qt::MinimumSize:
            t = &q_minimumSize;
            break;
         case Qt::PreferredSize:
            t = &q_preferredSize;
            break;
         case Qt::MaximumSize:
            t = &q_maximumSize;
            break;
         case Qt::MinimumDescent:
            t = &q_minimumDescent;
            break;
         case (Qt::MinimumDescent + 1):
            t = &q_minimumAscent;
            break;
         default:
            t = 0;
            break;
      }
      return *t;
   }
   inline const qreal &q_sizes(int which) const {
      const qreal *t;
      switch (which) {
         case Qt::MinimumSize:
            t = &q_minimumSize;
            break;
         case Qt::PreferredSize:
            t = &q_preferredSize;
            break;
         case Qt::MaximumSize:
            t = &q_maximumSize;
            break;
         case Qt::MinimumDescent:
            t = &q_minimumDescent;
            break;
         case (Qt::MinimumDescent + 1):
            t = &q_minimumAscent;
            break;
         default:
            t = 0;
            break;
      }
      return *t;
   }
};

bool operator==(const QGridLayoutBox &box1, const QGridLayoutBox &box2);

inline bool operator!=(const QGridLayoutBox &box1, const QGridLayoutBox &box2)
{
   return !operator==(box1, box2);
}

class QGridLayoutMultiCellData
{
 public:
   inline QGridLayoutMultiCellData() : q_stretch(-1) {}

   QGridLayoutBox q_box;
   int q_stretch;
};

typedef QMap<QPair<int, int>, QGridLayoutMultiCellData> MultiCellMap;

class QGridLayoutRowData
{
 public:
   void reset(int count);
   void distributeMultiCells(const QGridLayoutRowInfo &rowInfo, bool snapToPixelGrid);
   void calculateGeometries(int start, int end, qreal targetSize, qreal *positions, qreal *sizes,
      qreal *descents, const QGridLayoutBox &totalBox,
      const QGridLayoutRowInfo &rowInfo, bool snapToPixelGrid);

   QGridLayoutBox totalBox(int start, int end) const;
   void stealBox(int start, int end, int which, qreal *positions, qreal *sizes);



   QBitArray ignore;   // ### rename q_
   QVector<QGridLayoutBox> boxes;
   MultiCellMap multiCellMap;
   QVector<int> stretches;
   QVector<qreal> spacings;
   bool hasIgnoreFlag;
};

class QGridLayoutRowInfo
{
 public:
   inline QGridLayoutRowInfo() : count(0) {}

   void insertOrRemoveRows(int row, int delta);

#ifdef QGRIDLAYOUTENGINE_DEBUG
   void dump(int indent = 0) const;
#endif

   int count;
   QVector<QStretchParameter> stretches;
   QVector<QLayoutParameter<qreal>> spacings;
   QVector<Qt::Alignment> alignments;
   QVector<QGridLayoutBox> boxes;
};


class QGridLayoutItem
{
 public:
   QGridLayoutItem(int row, int column, int rowSpan = 1, int columnSpan = 1,
      Qt::Alignment alignment = 0);

   virtual ~QGridLayoutItem() {}

   inline int firstRow() const {
      return q_firstRows[Ver];
   }
   inline int firstColumn() const {
      return q_firstRows[Hor];
   }
   inline int rowSpan() const {
      return q_rowSpans[Ver];
   }
   inline int columnSpan() const {
      return q_rowSpans[Hor];
   }
   inline int lastRow() const {
      return firstRow() + rowSpan() - 1;
   }
   inline int lastColumn() const {
      return firstColumn() + columnSpan() - 1;
   }

   int firstRow(Qt::Orientation orientation) const;
   int firstColumn(Qt::Orientation orientation) const;
   int lastRow(Qt::Orientation orientation) const;
   int lastColumn(Qt::Orientation orientation) const;
   int rowSpan(Qt::Orientation orientation) const;
   int columnSpan(Qt::Orientation orientation) const;
   void setFirstRow(int row, Qt::Orientation orientation = Qt::Vertical);
   void setRowSpan(int rowSpan, Qt::Orientation orientation = Qt::Vertical);

   int stretchFactor(Qt::Orientation orientation) const;
   void setStretchFactor(int stretch, Qt::Orientation orientation);

   inline Qt::Alignment alignment() const {
      return q_alignment;
   }

   inline void setAlignment(Qt::Alignment alignment) {
      q_alignment = alignment;
   }

   virtual QLayoutPolicy::Policy sizePolicy(Qt::Orientation orientation) const = 0;
   virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const = 0;
   virtual bool isIgnored() const {
      return false;
   }

   virtual void setGeometry(const QRectF &rect) = 0;
   virtual bool hasDynamicConstraint() const {
      return false;
   }
   virtual Qt::Orientation dynamicConstraintOrientation() const {
      return Qt::Horizontal;
   }

   virtual QLayoutPolicy::ControlTypes controlTypes(LayoutSide side) const;

   QRectF geometryWithin(qreal x, qreal y, qreal width, qreal height, qreal rowDescent, Qt::Alignment align) const;
   QGridLayoutBox box(Qt::Orientation orientation, qreal constraint = -1.0) const;


   void transpose();
   void insertOrRemoveRows(int row, int delta, Qt::Orientation orientation = Qt::Vertical);
   QSizeF effectiveMaxSize(const QSizeF &constraint) const;


 private:
   int q_firstRows[NOrientations];
   int q_rowSpans[NOrientations];
   int q_stretches[NOrientations];
   Qt::Alignment q_alignment;
};


class QGridLayoutEngine
{
 public:
   QGridLayoutEngine(Qt::Alignment defaultAlignment = Qt::Alignment(0), bool snapToPixelGrid = false);
   ~QGridLayoutEngine() {
      qDeleteAll(q_items);
   }

   int rowCount(Qt::Orientation orientation) const;
   int columnCount(Qt::Orientation orientation) const;

   int rowCount() const {
      return q_infos[Ver].count;
   }

   int columnCount() const {
      return q_infos[Hor].count;
   }
   // returns the number of items inserted, which may be less than (rowCount * columnCount)
   int itemCount() const;
   QGridLayoutItem *itemAt(int index) const;

   int effectiveFirstRow(Qt::Orientation orientation = Qt::Vertical) const;
   int effectiveLastRow(Qt::Orientation orientation = Qt::Vertical) const;

   void setSpacing(qreal spacing, Qt::Orientations orientations);
   qreal spacing(Qt::Orientation orientation, const QAbstractLayoutStyleInfo *styleInfo) const;
   // ### setSpacingAfterRow(), spacingAfterRow()
   void setRowSpacing(int row, qreal spacing, Qt::Orientation orientation = Qt::Vertical);
   qreal rowSpacing(int row, Qt::Orientation orientation = Qt::Vertical) const;

   void setRowStretchFactor(int row, int stretch, Qt::Orientation orientation = Qt::Vertical);
   int rowStretchFactor(int row, Qt::Orientation orientation = Qt::Vertical) const;

   void setRowSizeHint(Qt::SizeHint which, int row, qreal size,
      Qt::Orientation orientation = Qt::Vertical);
   qreal rowSizeHint(Qt::SizeHint which, int row,
      Qt::Orientation orientation = Qt::Vertical) const;

   void setRowAlignment(int row, Qt::Alignment alignment, Qt::Orientation orientation);
   Qt::Alignment rowAlignment(int row, Qt::Orientation orientation) const;

   Qt::Alignment effectiveAlignment(const QGridLayoutItem *layoutItem) const;


   void insertItem(QGridLayoutItem *item, int index);
   void addItem(QGridLayoutItem *item);
   void removeItem(QGridLayoutItem *item);

   void deleteItems() {
      const QList<QGridLayoutItem *> oldItems = q_items;
      q_items.clear();    // q_items are used as input when the grid is regenerated in removeRows
      // The following calls to removeRows are suboptimal
      int rows = rowCount(Qt::Vertical);
      removeRows(0, rows, Qt::Vertical);
      rows = rowCount(Qt::Horizontal);
      removeRows(0, rows, Qt::Horizontal);
      qDeleteAll(oldItems);
   }
   QGridLayoutItem *itemAt(int row, int column, Qt::Orientation orientation = Qt::Vertical) const;
   inline void insertRow(int row, Qt::Orientation orientation = Qt::Vertical) {
      insertOrRemoveRows(row, +1, orientation);
   }
   inline void removeRows(int row, int count, Qt::Orientation orientation) {
      insertOrRemoveRows(row, -count, orientation);
   }

   void invalidate();
   void setGeometries(const QRectF &contentsGeometry, const QAbstractLayoutStyleInfo *styleInfo);
   QRectF cellRect(const QRectF &contentsGeometry, int row, int column, int rowSpan, int columnSpan,
      const QAbstractLayoutStyleInfo *styleInfo) const;
   QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint,
      const QAbstractLayoutStyleInfo *styleInfo) const;

   // heightForWidth / widthForHeight support
   QSizeF dynamicallyConstrainedSizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
   bool ensureDynamicConstraint() const;
   bool hasDynamicConstraint() const;
   Qt::Orientation constraintOrientation() const;


   QLayoutPolicy::ControlTypes controlTypes(LayoutSide side) const;
   void transpose();
   void setVisualDirection(Qt::LayoutDirection direction);
   Qt::LayoutDirection visualDirection() const;


 private:
   static int grossRoundUp(int n) {
      return ((n + 2) | 0x3) - 2;
   }

   void maybeExpandGrid(int row, int column, Qt::Orientation orientation = Qt::Vertical);
   void regenerateGrid();
   inline int internalGridRowCount() const {
      return grossRoundUp(rowCount());
   }

   inline int internalGridColumnCount() const {
      return grossRoundUp(columnCount());
   }
   void setItemAt(int row, int column, QGridLayoutItem *item);
   void insertOrRemoveRows(int row, int delta, Qt::Orientation orientation = Qt::Vertical);

   void fillRowData(QGridLayoutRowData *rowData,
      const qreal *colPositions, const qreal *colSizes,
      Qt::Orientation orientation,
      const QAbstractLayoutStyleInfo *styleInfo) const;

   void ensureEffectiveFirstAndLastRows() const;

   void ensureColumnAndRowData(QGridLayoutRowData *rowData, QGridLayoutBox *totalBox,
      const qreal *colPositions, const qreal *colSizes,
      Qt::Orientation orientation,
      const QAbstractLayoutStyleInfo *styleInfo) const;

   void ensureGeometries(const QSizeF &size, const QAbstractLayoutStyleInfo *styleInfo) const;

 protected:
   QList<QGridLayoutItem *> q_items;

 private:
   // User input
   QVector<QGridLayoutItem *> q_grid;
   QLayoutParameter<qreal> q_defaultSpacings[NOrientations];
   QGridLayoutRowInfo q_infos[NOrientations];
   Qt::LayoutDirection m_visualDirection;

   // Configuration
   Qt::Alignment m_defaultAlignment;
   unsigned m_snapToPixelGrid : 1;
   // Lazily computed from the above user input
   mutable int q_cachedEffectiveFirstRows[NOrientations];
   mutable int q_cachedEffectiveLastRows[NOrientations];
   mutable quint8 q_cachedConstraintOrientation : 3;

   // this is useful to cache
   mutable QGridLayoutBox q_totalBoxes[NOrientations];
   enum {
      NotCached = -2,             // Cache is empty. Happens when the engine is invalidated.
      CachedWithNoConstraint = -1 // cache has a totalBox without any HFW/WFH constraints.
         // >= 0                     // cache has a totalBox with this specific constraint.
   };
   mutable qreal q_totalBoxCachedConstraints[NOrientations];   // holds the constraint used for the cached totalBox
   mutable QGridLayoutRowData q_columnData;
   mutable QGridLayoutRowData q_rowData;

   // Output
   mutable QSizeF q_cachedSize;
   mutable QVector<qreal> q_xx;
   mutable QVector<qreal> q_yy;
   mutable QVector<qreal> q_widths;
   mutable QVector<qreal> q_heights;
   mutable QVector<qreal> q_descents;

   friend class QGridLayoutItem;
};

#endif
