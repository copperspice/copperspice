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

#ifndef QLISTVIEW_P_H
#define QLISTVIEW_P_H

#include <qabstractitemview_p.h>
#include <qrubberband.h>
#include <qbitarray.h>
#include <qbsptree_p.h>
#include <limits.h>
#include <qscrollbar.h>

#ifndef QT_NO_LISTVIEW

QT_BEGIN_NAMESPACE

class QListView;
class QListViewPrivate;

class QListViewItem
{
   friend class QListViewPrivate;
   friend class QListModeViewBase;
   friend class QIconModeViewBase;

 public:
   inline QListViewItem()
      : x(-1), y(-1), w(0), h(0), indexHint(-1), visited(0xffff) {}

   inline QListViewItem(const QListViewItem &other)
      : x(other.x), y(other.y), w(other.w), h(other.h),
        indexHint(other.indexHint), visited(other.visited) {}

   inline QListViewItem(QRect r, int i)
      : x(r.x()), y(r.y()), w(qMin(r.width(), SHRT_MAX)), h(qMin(r.height(), SHRT_MAX)),
        indexHint(i), visited(0xffff) {}

   inline bool operator==(const QListViewItem &other) const {
      return (x == other.x && y == other.y && w == other.w && h == other.h && indexHint == other.indexHint);
   }
   inline bool operator!=(const QListViewItem &other) const {
      return !(*this == other);
   }
   inline bool isValid() const {
      return rect().isValid() && (indexHint > -1);
   }
   inline void invalidate() {
      x = -1;
      y = -1;
      w = 0;
      h = 0;
   }
   inline void resize(const QSize &size) {
      w = qMin(size.width(), SHRT_MAX);
      h = qMin(size.height(), SHRT_MAX);
   }
   inline void move(const QPoint &position) {
      x = position.x();
      y = position.y();
   }
   inline int width() const {
      return w;
   }
   inline int height() const {
      return h;
   }
 private:
   inline QRect rect() const {
      return QRect(x, y, w, h);
   }
   int x, y;
   short w, h;
   mutable int indexHint;
   uint visited;
};

struct QListViewLayoutInfo {
   QRect bounds;
   QSize grid;
   int spacing;
   int first;
   int last;
   bool wrap;
   QListView::Flow flow;
   int max;
};

class QCommonListViewBase
{
 public:
   inline QCommonListViewBase(QListView *q, QListViewPrivate *d) : dd(d), qq(q), batchStartRow(0), batchSavedDeltaSeg(0) {}
   virtual ~QCommonListViewBase() {}

   //common interface
   virtual int itemIndex(const QListViewItem &item) const = 0;
   virtual QListViewItem indexToListViewItem(const QModelIndex &index) const = 0;
   virtual bool doBatchedItemLayout(const QListViewLayoutInfo &info, int max) = 0;
   virtual void clear() = 0;
   virtual void setRowCount(int) = 0;
   virtual QVector<QModelIndex> intersectingSet(const QRect &area) const = 0;
   virtual void dataChanged(const QModelIndex &, const QModelIndex &) = 0;

   virtual int horizontalScrollToValue(int index, QListView::ScrollHint hint,
                                       bool leftOf, bool rightOf, const QRect &area, const QRect &rect) const;
   virtual int verticalScrollToValue(int index, QListView::ScrollHint hint,
                                     bool above, bool below, const QRect &area, const QRect &rect) const;
   virtual void scrollContentsBy(int dx, int dy, bool scrollElasticBand);
   virtual QRect mapToViewport(const QRect &rect) const {
      return rect;
   }
   virtual int horizontalOffset() const;
   virtual int verticalOffset() const {
      return verticalScrollBar()->value();
   }
   virtual void updateHorizontalScrollBar(const QSize &step);
   virtual void updateVerticalScrollBar(const QSize &step);
   virtual void appendHiddenRow(int row);
   virtual void removeHiddenRow(int row);
   virtual void setPositionForIndex(const QPoint &, const QModelIndex &) { }

#ifndef QT_NO_DRAGANDDROP
   void paintDragDrop(QPainter *painter);
   virtual bool filterDragMoveEvent(QDragMoveEvent *) {
      return false;
   }
   virtual bool filterDragLeaveEvent(QDragLeaveEvent *) {
      return false;
   }
   virtual bool filterDropEvent(QDropEvent *) {
      return false;
   }
   virtual bool filterStartDrag(Qt::DropActions) {
      return false;
   }
#endif

   //other inline members
   inline int spacing() const;
   inline bool isWrapping() const;
   inline QSize gridSize() const;
   inline QListView::Flow flow() const;
   inline QListView::Movement movement() const;

   inline QPoint offset() const;
   inline QPoint pressedPosition() const;
   inline bool uniformItemSizes() const;
   inline int column() const;

   inline QScrollBar *verticalScrollBar() const;
   inline QScrollBar *horizontalScrollBar() const;
   inline QListView::ScrollMode verticalScrollMode() const;
   inline QListView::ScrollMode horizontalScrollMode() const;

   inline QModelIndex modelIndex(int row) const;
   inline int rowCount() const;

   inline QStyleOptionViewItemV4 viewOptions() const;
   inline QWidget *viewport() const;
   inline QRect clipRect() const;

   inline QSize cachedItemSize() const;
   inline QRect viewItemRect(const QListViewItem &item) const;
   inline QSize itemSize(const QStyleOptionViewItemV2 &opt, const QModelIndex &idx) const;
   inline QAbstractItemDelegate *delegate(const QModelIndex &idx) const;

   inline bool isHidden(int row) const;
   inline int hiddenCount() const;

   inline bool isRightToLeft() const;

   QListViewPrivate *dd;
   QListView *qq;
   QSize contentsSize;
   int batchStartRow;
   int batchSavedDeltaSeg;
};

class QListModeViewBase : public QCommonListViewBase
{
 public:
   QListModeViewBase(QListView *q, QListViewPrivate *d) : QCommonListViewBase(q, d) {}

   QVector<int> flowPositions;
   QVector<int> segmentPositions;
   QVector<int> segmentStartRows;
   QVector<int> segmentExtents;
   QVector<int> scrollValueMap;

   // used when laying out in batches
   int batchSavedPosition;

   //reimplementations
   int itemIndex(const QListViewItem &item) const override{
      return item.indexHint;
   }

   QListViewItem indexToListViewItem(const QModelIndex &index) const override;
   bool doBatchedItemLayout(const QListViewLayoutInfo &info, int max) override;
   void clear() override;

   void setRowCount(int rowCount) override{
      flowPositions.resize(rowCount);
   }

   QVector<QModelIndex> intersectingSet(const QRect &area) const override;
   void dataChanged(const QModelIndex &, const QModelIndex &) override;

   int horizontalScrollToValue(int index, QListView::ScrollHint hint,
                  bool leftOf, bool rightOf, const QRect &area, const QRect &rect) const override;

   int verticalScrollToValue(int index, QListView::ScrollHint hint,
                  bool above, bool below, const QRect &area, const QRect &rect) const override;

   void scrollContentsBy(int dx, int dy, bool scrollElasticBand) override;
   QRect mapToViewport(const QRect &rect) const override;
   int horizontalOffset() const override;
   int verticalOffset() const override;
   void updateHorizontalScrollBar(const QSize &step) override;
   void updateVerticalScrollBar(const QSize &step) override;

#ifndef QT_NO_DRAGANDDROP
   // The next two methods are to be used on LefToRight flow only.
   // WARNING: Plenty of duplicated code from QAbstractItemView{,Private}.
   QAbstractItemView::DropIndicatorPosition position(const QPoint &pos, const QRect &rect, const QModelIndex &idx) const;
   void dragMoveEvent(QDragMoveEvent *e);
   bool dropOn(QDropEvent *event, int *row, int *col, QModelIndex *index);
#endif

 private:
   QPoint initStaticLayout(const QListViewLayoutInfo &info);
   void doStaticLayout(const QListViewLayoutInfo &info);

   int perItemScrollToValue(int index, int value, int height, QAbstractItemView::ScrollHint hint,
                  Qt::Orientation orientation, bool wrap, int extent) const;

   int perItemScrollingPageSteps(int length, int bounds, bool wrap) const;
};

class QIconModeViewBase : public QCommonListViewBase
{
 public:
   QIconModeViewBase(QListView *q, QListViewPrivate *d) : QCommonListViewBase(q, d), interSectingVector(0) {}

   QBspTree tree;
   QVector<QListViewItem> items;
   QBitArray moved;

   QVector<QModelIndex> draggedItems; // indices to the tree.itemVector
   mutable QPoint draggedItemsPos;

   // used when laying out in batches
   QVector<QModelIndex> *interSectingVector; //used from within intersectingSet

   //reimplementations
   int itemIndex(const QListViewItem &item) const override;
   QListViewItem indexToListViewItem(const QModelIndex &index) const override;
   bool doBatchedItemLayout(const QListViewLayoutInfo &info, int max) override;
   void clear() override;
   void setRowCount(int rowCount) override;
   QVector<QModelIndex> intersectingSet(const QRect &area) const override;

   void scrollContentsBy(int dx, int dy, bool scrollElasticBand) override;
   void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) override;
   void appendHiddenRow(int row) override;
   void removeHiddenRow(int row) override;
   void setPositionForIndex(const QPoint &position, const QModelIndex &index) override;

#ifndef QT_NO_DRAGANDDROP
   bool filterDragMoveEvent(QDragMoveEvent *) override;
   bool filterDragLeaveEvent(QDragLeaveEvent *) override;
   bool filterDropEvent(QDropEvent *e) override;
   bool filterStartDrag(Qt::DropActions) override;
#endif

 private:
   void initBspTree(const QSize &contents);
   QPoint initDynamicLayout(const QListViewLayoutInfo &info);
   void doDynamicLayout(const QListViewLayoutInfo &info);
   static void addLeaf(QVector<int> &leaf, const QRect &area,uint visited, QBspTree::Data data);
   QRect itemsRect(const QVector<QModelIndex> &indexes) const;
   QRect draggedItemsRect() const;
   QPoint snapToGrid(const QPoint &pos) const;
   void updateContentsSize();
   QPoint draggedItemsDelta() const;
   void drawItems(QPainter *painter, const QVector<QModelIndex> &indexes) const;
   void moveItem(int index, const QPoint &dest);

};

class QListViewPrivate: public QAbstractItemViewPrivate
{
   Q_DECLARE_PUBLIC(QListView)

 public:
   QListViewPrivate();
   ~QListViewPrivate();

   void clear();
   void prepareItemsLayout();

   bool doItemsLayout(int num);

   inline QVector<QModelIndex> intersectingSet(const QRect &area, bool doLayout = true) const {
      if (doLayout) {
         executePostedLayout();
      }
      QRect a = (q_func()->isRightToLeft() ? flipX(area.normalized()) : area.normalized());
      return commonListView->intersectingSet(a);
   }

   inline void resetBatchStartRow() {
      commonListView->batchStartRow = 0;
   }
   inline int batchStartRow() const {
      return commonListView->batchStartRow;
   }
   inline QSize contentsSize() const {
      return commonListView->contentsSize;
   }
   inline void setContentsSize(int w, int h) {
      commonListView->contentsSize = QSize(w, h);
   }

   inline int flipX(int x) const {
      return qMax(viewport->width(), contentsSize().width()) - x;
   }
   inline QPoint flipX(const QPoint &p) const {
      return QPoint(flipX(p.x()), p.y());
   }
   inline QRect flipX(const QRect &r) const {
      return QRect(flipX(r.x()) - r.width(), r.y(), r.width(), r.height());
   }
   inline QRect viewItemRect(const QListViewItem &item) const {
      if (q_func()->isRightToLeft()) {
         return flipX(item.rect());
      }
      return item.rect();
   }

   QListViewItem indexToListViewItem(const QModelIndex &index) const;
   inline QModelIndex listViewItemToIndex(const QListViewItem &item) const {
      return model->index(commonListView->itemIndex(item), column, root);
   }

   QRect rectForIndex(const QModelIndex &index) const {
      if (!isIndexValid(index) || index.parent() != root || index.column() != column || isHidden(index.row())) {
         return QRect();
      }
      executePostedLayout();
      return viewItemRect(indexToListViewItem(index));
   }

   void viewUpdateGeometries() {
      q_func()->updateGeometries();
   }


   QRect mapToViewport(const QRect &rect, bool extend = true) const;

   QModelIndex closestIndex(const QRect &target, const QVector<QModelIndex> &candidates) const;
   QSize itemSize(const QStyleOptionViewItem &option, const QModelIndex &index) const;

   bool selectionAllowed(const QModelIndex &index) const override {
      if (viewMode == QListView::ListMode && !showElasticBand) {
         return index.isValid();
      }
      return true;
   }

   int horizontalScrollToValue(const QModelIndex &index, const QRect &rect, QListView::ScrollHint hint) const;
   int verticalScrollToValue(const QModelIndex &index, const QRect &rect, QListView::ScrollHint hint) const;

   QItemSelection selection(const QRect &rect) const;
   void selectAll(QItemSelectionModel::SelectionFlags command) override;

#ifndef QT_NO_DRAGANDDROP
   virtual QAbstractItemView::DropIndicatorPosition position(const QPoint &pos, const QRect &rect,
                  const QModelIndex &idx) const override;

   bool dropOn(QDropEvent *event, int *row, int *col, QModelIndex *index) override;
#endif

   inline void setGridSize(const QSize &size) {
      grid = size;
   }
   inline QSize gridSize() const {
      return grid;
   }
   inline void setWrapping(bool b) {
      wrap = b;
   }
   inline bool isWrapping() const {
      return wrap;
   }
   inline void setSpacing(int s) {
      space = s;
   }
   inline int spacing() const {
      return space;
   }
   inline void setSelectionRectVisible(bool visible) {
      showElasticBand = visible;
   }
   inline bool isSelectionRectVisible() const {
      return showElasticBand;
   }

   inline QModelIndex modelIndex(int row) const {
      return model->index(row, column, root);
   }
   inline bool isHidden(int row) const {
      QModelIndex idx = model->index(row, 0, root);
      return isPersistent(idx) && hiddenRows.contains(idx);
   }
   inline bool isHiddenOrDisabled(int row) const {
      return isHidden(row) || !isIndexEnabled(modelIndex(row));
   }

   inline void removeCurrentAndDisabled(QVector<QModelIndex> *indexes, const QModelIndex &current) const {
      QVector<QModelIndex>::iterator it = indexes->begin();
      while (it != indexes->end()) {
         if (!isIndexEnabled(*it) || (*it) == current) {
            indexes->erase(it);
         } else {
            ++it;
         }
      }
   }

   void scrollElasticBandBy(int dx, int dy);

   QItemViewPaintPairs draggablePaintPairs(const QModelIndexList &indexes, QRect *r) const override;

   void emitIndexesMoved(const QModelIndexList &indexes) {
      emit q_func()->indexesMoved(indexes);
   }

   QCommonListViewBase *commonListView;

   // ### FIXME: see if we can move the members into the dynamic/static classes
   bool wrap;
   int space;
   QSize grid;

   QListView::Flow flow;
   QListView::Movement movement;
   QListView::ResizeMode resizeMode;
   QListView::LayoutMode layoutMode;
   QListView::ViewMode viewMode;

   // the properties controlling the icon- or list-view modes
   enum ModeProperties {
      Wrap = 1,
      Spacing = 2,
      GridSize = 4,
      Flow = 8,
      Movement = 16,
      ResizeMode = 32,
      SelectionRectVisible = 64
   };

   uint modeProperties : 8;

   QRect layoutBounds;

   // timers
   QBasicTimer batchLayoutTimer;

   // used for hidden items
   QSet<QPersistentModelIndex> hiddenRows;

   int column;
   bool uniformItemSizes;
   mutable QSize cachedItemSize;
   int batchSize;

   QRect elasticBand;
   bool showElasticBand;
};

// inline implementations

inline int QCommonListViewBase::spacing() const
{
   return dd->spacing();
}
inline bool QCommonListViewBase::isWrapping() const
{
   return dd->isWrapping();
}
inline QSize QCommonListViewBase::gridSize() const
{
   return dd->gridSize();
}
inline QListView::Flow QCommonListViewBase::flow() const
{
   return dd->flow;
}
inline QListView::Movement QCommonListViewBase::movement() const
{
   return dd->movement;
}

inline QPoint QCommonListViewBase::offset() const
{
   return dd->offset();
}
inline QPoint QCommonListViewBase::pressedPosition() const
{
   return dd->pressedPosition;
}
inline bool QCommonListViewBase::uniformItemSizes() const
{
   return dd->uniformItemSizes;
}
inline int QCommonListViewBase::column() const
{
   return dd->column;
}

inline QScrollBar *QCommonListViewBase::verticalScrollBar() const
{
   return qq->verticalScrollBar();
}
inline QScrollBar *QCommonListViewBase::horizontalScrollBar() const
{
   return qq->horizontalScrollBar();
}
inline QListView::ScrollMode QCommonListViewBase::verticalScrollMode() const
{
   return qq->verticalScrollMode();
}
inline QListView::ScrollMode QCommonListViewBase::horizontalScrollMode() const
{
   return qq->horizontalScrollMode();
}

inline QModelIndex QCommonListViewBase::modelIndex(int row) const
{
   return dd->model->index(row, dd->column, dd->root);
}
inline int QCommonListViewBase::rowCount() const
{
   return dd->model->rowCount(dd->root);
}

inline QStyleOptionViewItemV4 QCommonListViewBase::viewOptions() const
{
   return dd->viewOptionsV4();
}

inline QWidget *QCommonListViewBase::viewport() const
{
   return dd->viewport;
}

inline QRect QCommonListViewBase::clipRect() const
{
   return dd->clipRect();
}

inline QSize QCommonListViewBase::cachedItemSize() const
{
   return dd->cachedItemSize;
}

inline QRect QCommonListViewBase::viewItemRect(const QListViewItem &item) const
{
   return dd->viewItemRect(item);
}

inline QSize QCommonListViewBase::itemSize(const QStyleOptionViewItemV2 &opt, const QModelIndex &idx) const
{
   return dd->itemSize(opt, idx);
}

inline QAbstractItemDelegate *QCommonListViewBase::delegate(const QModelIndex &idx) const
{
   return dd->delegateForIndex(idx);
}

inline bool QCommonListViewBase::isHidden(int row) const
{
   return dd->isHidden(row);
}
inline int QCommonListViewBase::hiddenCount() const
{
   return dd->hiddenRows.count();
}

inline bool QCommonListViewBase::isRightToLeft() const
{
   return qq->isRightToLeft();
}

QT_END_NAMESPACE

#endif // QT_NO_LISTVIEW

#endif // QLISTVIEW_P_H
