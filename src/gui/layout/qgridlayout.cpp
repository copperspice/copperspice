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

#include <qgridlayout.h>

#include <qapplication.h>
#include <qlist.h>
#include <qsizepolicy.h>
#include <qvarlengtharray.h>
#include <qvector.h>
#include <qwidget.h>

#include <qlayoutengine_p.h>
#include <qlayout_p.h>

struct QGridLayoutSizeTriple {
   QSize minS;
   QSize hint;
   QSize maxS;
};

/*
  Three internal classes related to QGridLayout: (1) QGridBox is a
  QLayoutItem with (row, column) information and (torow, tocolumn) information; (3) QGridLayoutData is
  the internal representation of a QGridLayout.
*/

class QGridBox
{
 public:
   QGridBox(QLayoutItem *item) {
      m_item = item;
   }

   QGridBox(const QLayout *layout, QWidget *wid) {
      m_item = QLayoutPrivate::createWidgetItem(layout, wid);
   }

   ~QGridBox() {
      delete m_item;
   }

   QSize sizeHint() const {
      return m_item->sizeHint();
   }

   QSize minimumSize() const {
      return m_item->minimumSize();
   }

   QSize maximumSize() const {
      return m_item->maximumSize();
   }

   Qt::Orientations expandingDirections() const {
      return m_item->expandingDirections();
   }

   bool isEmpty() const {
      return m_item->isEmpty();
   }

   bool hasHeightForWidth() const {
      return m_item->hasHeightForWidth();
   }

   int heightForWidth(int w) const {
      return m_item->heightForWidth(w);
   }

   void setAlignment(Qt::Alignment a) {
      m_item->setAlignment(a);
   }

   void setGeometry(const QRect &r) {
      m_item->setGeometry(r);
   }

   Qt::Alignment alignment() const {
      return m_item->alignment();
   }

   QLayoutItem *item() {
      return m_item;
   }

   void setItem(QLayoutItem *newItem) {
      m_item = newItem;
   }

   QLayoutItem *takeItem() {
      QLayoutItem *tmp = m_item;
      m_item = nullptr;

      return tmp;
   }

   int hStretch() {
      return m_item->widget() ? m_item->widget()->sizePolicy().horizontalStretch() : 0;
   }

   int vStretch() {
      return m_item->widget() ? m_item->widget()->sizePolicy().verticalStretch() : 0;
   }

 private:
   friend class QGridLayoutPrivate;
   friend class QGridLayout;

   int toRow(int rr) const {
      return torow >= 0 ? torow : rr - 1;
   }

   int toCol(int cc) const {
      return tocol >= 0 ? tocol : cc - 1;
   }

   QLayoutItem *m_item;
   int row, col;
   int torow, tocol;
};

class QGridLayoutPrivate : public QLayoutPrivate
{
   Q_DECLARE_PUBLIC(QGridLayout)

 public:
   QGridLayoutPrivate();

   void add(QGridBox *, int row, int col);
   void add(QGridBox *, int row1, int row2, int col1, int col2);
   QSize sizeHint(int hSpacing, int vSpacing) const;
   QSize minimumSize(int hSpacing, int vSpacing) const;
   QSize maximumSize(int hSpacing, int vSpacing) const;

   Qt::Orientations expandingDirections(int hSpacing, int vSpacing) const;

   void distribute(QRect rect, int hSpacing, int vSpacing);

   inline int numRows() const {
      return rr;
   }

   inline int numCols() const {
      return cc;
   }

   inline void expand(int rows, int cols) {

      setSize(qMax(rows, rr), qMax(cols, cc));
   }

   inline void setRowStretch(int r, int s) {
      expand(r + 1, 0);
      rStretch[r] = s;
      setDirty();
   }

   inline void setColStretch(int c, int s) {
      expand(0, c + 1);
      cStretch[c] = s;
      setDirty();
   }

   inline int rowStretch(int r) const {
      return rStretch.at(r);
   }
   inline int colStretch(int c) const {
      return cStretch.at(c);
   }
   inline void setRowMinimumHeight(int r, int s) {
      expand(r + 1, 0);
      rMinHeights[r] = s;
      setDirty();
   }
   inline void setColumnMinimumWidth(int c, int s) {
      expand(0, c + 1);
      cMinWidths[c] = s;
      setDirty();
   }
   inline int rowSpacing(int r) const {
      return rMinHeights.at(r);
   }
   inline int colSpacing(int c) const {
      return cMinWidths.at(c);
   }

   inline void setReversed(bool r, bool c) {
      hReversed = c;
      vReversed = r;
   }

   inline bool horReversed() const {
      return hReversed;
   }

   inline bool verReversed() const {
      return vReversed;
   }

   inline void setDirty() {
      needRecalc = true;
      hfw_width = -1;
   }

   inline bool isDirty() const {
      return needRecalc;
   }

   bool hasHeightForWidth(int hSpacing, int vSpacing);
   int heightForWidth(int width, int hSpacing, int vSpacing);
   int minimumHeightForWidth(int width, int hSpacing, int vSpacing);

   inline void getNextPos(int &row, int &col) {
      row = nextR;
      col = nextC;
   }

   inline int count() const {
      return things.count();
   }

   QRect cellRect(int row, int col) const;

   inline QLayoutItem *itemAt(int index) const {
      if (index < things.count()) {
         return things.at(index)->item();
      } else {
         return nullptr;
      }
   }

   inline QLayoutItem *takeAt(int index) {
      Q_Q(QGridLayout);

      if (index < things.count()) {
         if (QGridBox *b = things.takeAt(index)) {
            QLayoutItem *item = b->takeItem();

            if (QLayout *l = item->layout()) {
               // sanity check in case the user passed something weird to QObject::setParent()
               if (l->parent() == q) {
                  l->setParent(nullptr);
               }
            }

            delete b;
            return item;
         }
      }

      return nullptr;
   }

   QLayoutItem *replaceAt(int index, QLayoutItem *newitem) override {
      if (!newitem) {
         return nullptr;
      }

      QLayoutItem *item = nullptr;
      QGridBox *b = things.value(index);

      if (b) {
         item = b->takeItem();
         b->setItem(newitem);
      }
      return item;
   }

   void getItemPosition(int index, int *row, int *column, int *rowSpan, int *columnSpan) const {
      if (index < things.count()) {
         const QGridBox *b =  things.at(index);
         int toRow   = b->toRow(rr);
         int toCol   = b->toCol(cc);
         *row        = b->row;
         *column     = b->col;
         *rowSpan    = toRow - *row + 1;
         *columnSpan = toCol - *column + 1;
      }
   }

   void deleteAll();

 private:
   void setNextPosAfter(int r, int c);
   void recalcHFW(int w);
   void addHfwData(QGridBox *box, int width);
   void init();
   QSize findSize(int QLayoutStruct::*, int hSpacing, int vSpacing) const;
   void addData(QGridBox *b, const QGridLayoutSizeTriple &sizes, bool r, bool c);
   void setSize(int rows, int cols);
   void setupSpacings(QVector<QLayoutStruct> &chain, QGridBox *grid[], int fixedSpacing,
      Qt::Orientation orientation);
   void setupLayoutData(int hSpacing, int vSpacing);
   void setupHfwLayoutData();
   void effectiveMargins(int *left, int *top, int *right, int *bottom) const;

   int rr;
   int cc;
   QVector<QLayoutStruct> rowData;
   QVector<QLayoutStruct> colData;
   QVector<QLayoutStruct> *hfwData;
   QVector<int> rStretch;
   QVector<int> cStretch;
   QVector<int> rMinHeights;
   QVector<int> cMinWidths;
   QList<QGridBox *> things;

   int hfw_width;
   int hfw_height;
   int hfw_minheight;
   int nextR;
   int nextC;

   int horizontalSpacing;
   int verticalSpacing;
   int leftMargin;
   int topMargin;
   int rightMargin;
   int bottomMargin;

   uint hReversed : 1;
   uint vReversed : 1;
   uint needRecalc : 1;
   uint has_hfw : 1;
   uint addVertical : 1;
};

void QGridLayoutPrivate::effectiveMargins(int *left, int *top, int *right, int *bottom) const
{
   int l = leftMargin;
   int t = topMargin;
   int r = rightMargin;
   int b = bottomMargin;

#ifdef Q_OS_DARWIN
   int leftMost = INT_MAX;
   int topMost = INT_MAX;
   int rightMost = 0;
   int bottomMost = 0;

   QWidget *w = nullptr;
   const int n = things.count();

   for (int i = 0; i < n; ++i) {
      QGridBox *box    = things.at(i);
      QLayoutItem *itm = box->item();
      w = itm->widget();

      if (w) {
         bool visualHReversed = hReversed != (w->layoutDirection() == Qt::RightToLeft);
         QRect lir = itm->geometry();
         QRect wr = w->geometry();

         if (box->col <= leftMost) {
            if (box->col < leftMost) {
               // we found an item even closer to the margin, discard.
               leftMost = box->col;
               if (visualHReversed) {
                  r = rightMargin;
               } else {
                  l = leftMargin;
               }
            }
            if (visualHReversed) {
               r = qMax(r, wr.right() - lir.right());
            } else {
               l = qMax(l, lir.left() - wr.left());
            }
         }
         if (box->row <= topMost) {
            if (box->row < topMost) {
               // we found an item even closer to the margin, discard.
               topMost = box->row;
               if (vReversed) {
                  b = bottomMargin;
               } else {
                  t = topMargin;
               }
            }
            if (vReversed) {
               b = qMax(b, wr.bottom() - lir.bottom());
            } else {
               t = qMax(t, lir.top() - wr.top());
            }
         }
         if (box->toCol(cc) >= rightMost) {
            if (box->toCol(cc) > rightMost) {
               // we found an item even closer to the margin, discard.
               rightMost = box->toCol(cc);
               if (visualHReversed) {
                  l = leftMargin;
               } else {
                  r = rightMargin;
               }
            }
            if (visualHReversed) {
               l = qMax(l, lir.left() - wr.left());
            } else {
               r = qMax(r, wr.right() - lir.right());
            }

         }
         if (box->toRow(rr) >= bottomMost) {
            if (box->toRow(rr) > bottomMost) {
               // we found an item even closer to the margin, discard.
               bottomMost = box->toRow(rr);
               if (vReversed) {
                  t = topMargin;
               } else {
                  b = bottomMargin;
               }
            }
            if (vReversed) {
               t = qMax(t, lir.top() - wr.top());
            } else {
               b = qMax(b, wr.bottom() - lir.bottom());
            }
         }
      }
   }

#endif

   if (left) {
      *left = l;
   }
   if (top) {
      *top = t;
   }
   if (right) {
      *right = r;
   }
   if (bottom) {
      *bottom = b;
   }
}

QGridLayoutPrivate::QGridLayoutPrivate()
{
   addVertical = false;
   setDirty();
   rr        = 0;
   cc        = 0;
   nextR     = 0;
   nextC     = 0;
   hfwData   = nullptr;
   hReversed = false;
   vReversed = false;

   horizontalSpacing = -1;
   verticalSpacing   = -1;
}

void QGridLayoutPrivate::deleteAll()
{
   while (!things.isEmpty()) {
      delete things.takeFirst();
   }
   delete hfwData;
}

bool QGridLayoutPrivate::hasHeightForWidth(int hSpacing, int vSpacing)
{
   setupLayoutData(hSpacing, vSpacing);
   return has_hfw;
}

void QGridLayoutPrivate::recalcHFW(int w)
{
   //  Go through all children, using colData and heightForWidth()
   //  and put the results in hfwData.

   if (! hfwData) {
      hfwData = new QVector<QLayoutStruct>(rr);
   }
   setupHfwLayoutData();
   QVector<QLayoutStruct> &rData = *hfwData;

   int h = 0;
   int mh = 0;
   for (int r = 0; r < rr; r++) {
      int spacing = rData.at(r).spacing;
      h += rData.at(r).sizeHint + spacing;
      mh += rData.at(r).minimumSize + spacing;
   }

   hfw_width = w;
   hfw_height = qMin(QLAYOUTSIZE_MAX, h);
   hfw_minheight = qMin(QLAYOUTSIZE_MAX, mh);
}

int QGridLayoutPrivate::heightForWidth(int w, int hSpacing, int vSpacing)
{
   setupLayoutData(hSpacing, vSpacing);
   if (!has_hfw) {
      return -1;
   }
   int left, top, right, bottom;
   effectiveMargins(&left, &top, &right, &bottom);

   int hMargins = left + right;
   if (w - hMargins != hfw_width) {
      qGeomCalc(colData, 0, cc, 0, w - hMargins);
      recalcHFW(w - hMargins);
   }
   return hfw_height + top + bottom;
}

int QGridLayoutPrivate::minimumHeightForWidth(int w, int hSpacing, int vSpacing)
{
   (void)heightForWidth(w, hSpacing, vSpacing);
   if (!has_hfw) {
      return -1;
   }

   int top, bottom;
   effectiveMargins(nullptr, &top, nullptr, &bottom);

   return hfw_minheight + top + bottom;
}

QSize QGridLayoutPrivate::findSize(int QLayoutStruct::*size, int hSpacing, int vSpacing) const
{
   QGridLayoutPrivate *self = const_cast<QGridLayoutPrivate *>(this);
   self->setupLayoutData(hSpacing, vSpacing);

   int w = 0;
   int h = 0;

   for (int r = 0; r < rr; r++) {
      h += rowData.at(r).*size + rowData.at(r).spacing;
   }

   for (int c = 0; c < cc; c++) {
      w += colData.at(c).*size + colData.at(c).spacing;
   }

   w = qMin(QLAYOUTSIZE_MAX, w);
   h = qMin(QLAYOUTSIZE_MAX, h);

   return QSize(w, h);
}

Qt::Orientations QGridLayoutPrivate::expandingDirections(int hSpacing, int vSpacing) const
{
   QGridLayoutPrivate *self = const_cast<QGridLayoutPrivate *>(this);
   self->setupLayoutData(hSpacing, vSpacing);

   Qt::Orientations ret;

   for (int r = 0; r < rr; r++) {
      if (rowData.at(r).expansive) {
         ret |= Qt::Vertical;
         break;
      }
   }

   for (int c = 0; c < cc; c++) {
      if (colData.at(c).expansive) {
         ret |= Qt::Horizontal;
         break;
      }
   }

   return ret;
}

QSize QGridLayoutPrivate::sizeHint(int hSpacing, int vSpacing) const
{
   return findSize(&QLayoutStruct::sizeHint, hSpacing, vSpacing);
}

QSize QGridLayoutPrivate::maximumSize(int hSpacing, int vSpacing) const
{
   return findSize(&QLayoutStruct::maximumSize, hSpacing, vSpacing);
}

QSize QGridLayoutPrivate::minimumSize(int hSpacing, int vSpacing) const
{
   return findSize(&QLayoutStruct::minimumSize, hSpacing, vSpacing);
}

void QGridLayoutPrivate::setSize(int r, int c)
{
   if ((int)rowData.size() < r) {
      int newR = qMax(r, rr * 2);
      rowData.resize(newR);
      rStretch.resize(newR);
      rMinHeights.resize(newR);

      for (int i = rr; i < newR; i++) {
         rowData[i].init();
         rowData[i].maximumSize = 0;
         rowData[i].pos = 0;
         rowData[i].size = 0;
         rStretch[i] = 0;
         rMinHeights[i] = 0;
      }
   }

   if ((int)colData.size() < c) {
      int newC = qMax(c, cc * 2);
      colData.resize(newC);
      cStretch.resize(newC);
      cMinWidths.resize(newC);
      for (int i = cc; i < newC; i++) {
         colData[i].init();
         colData[i].maximumSize = 0;
         colData[i].pos = 0;
         colData[i].size = 0;
         cStretch[i] = 0;
         cMinWidths[i] = 0;
      }
   }

   if (hfwData && (int)hfwData->size() < r) {
      delete hfwData;
      hfwData   = nullptr;
      hfw_width = -1;
   }
   rr = r;
   cc = c;
}

void QGridLayoutPrivate::setNextPosAfter(int row, int col)
{
   if (addVertical) {
      if (col > nextC || (col == nextC && row >= nextR)) {
         nextR = row + 1;
         nextC = col;
         if (nextR >= rr) {
            nextR = 0;
            nextC++;
         }
      }
   } else {
      if (row > nextR || (row == nextR && col >= nextC)) {
         nextR = row;
         nextC = col + 1;
         if (nextC >= cc) {
            nextC = 0;
            nextR++;
         }
      }
   }
}

void QGridLayoutPrivate::add(QGridBox *box, int row, int col)
{
   expand(row + 1, col + 1);
   box->row = box->torow = row;
   box->col = box->tocol = col;
   things.append(box);
   setDirty();
   setNextPosAfter(row, col);
}

void QGridLayoutPrivate::add(QGridBox *box, int row1, int row2, int col1, int col2)
{
   if (row2 >= 0 && row2 < row1) {
      qWarning("QGridLayout:add() Multi cell fromRow is greater than toRow");
   }
   if (col2 >= 0 && col2 < col1) {
      qWarning("QGridLayout:add() Multi cell fromCol is greater than toCol");
   }
   if (row1 == row2 && col1 == col2) {
      add(box, row1, col1);
      return;
   }

   expand(qMax(row1, row2) + 1, qMax(col1, col2) + 1);

   box->row = row1;
   box->col = col1;

   box->torow = row2;
   box->tocol = col2;

   things.append(box);
   setDirty();
   if (col2 < 0) {
      col2 = cc - 1;
   }

   setNextPosAfter(row2, col2);
}

void QGridLayoutPrivate::addData(QGridBox *box, const QGridLayoutSizeTriple &sizes, bool r, bool c)
{
   const QWidget *widget = box->item()->widget();

   if (box->isEmpty() && widget) {
      return;
   }

   if (c) {
      QLayoutStruct *data = &colData[box->col];
      if (!cStretch.at(box->col)) {
         data->stretch = qMax(data->stretch, box->hStretch());
      }
      data->sizeHint = qMax(sizes.hint.width(), data->sizeHint);
      data->minimumSize = qMax(sizes.minS.width(), data->minimumSize);

      qMaxExpCalc(data->maximumSize, data->expansive, data->empty, sizes.maxS.width(),
         box->expandingDirections() & Qt::Horizontal, box->isEmpty());
   }
   if (r) {
      QLayoutStruct *data = &rowData[box->row];
      if (!rStretch.at(box->row)) {
         data->stretch = qMax(data->stretch, box->vStretch());
      }
      data->sizeHint = qMax(sizes.hint.height(), data->sizeHint);
      data->minimumSize = qMax(sizes.minS.height(), data->minimumSize);

      qMaxExpCalc(data->maximumSize, data->expansive, data->empty, sizes.maxS.height(),
         box->expandingDirections() & Qt::Vertical, box->isEmpty());
   }
}

static void initEmptyMultiBox(QVector<QLayoutStruct> &chain, int start, int end)
{
   for (int i = start; i <= end; i++) {
      QLayoutStruct *data = &chain[i];

      if (data->empty && data->maximumSize == 0) {
         // truly empty box
         data->maximumSize = QWIDGETSIZE_MAX;
      }

      data->empty = false;
   }
}

static void distributeMultiBox(QVector<QLayoutStruct> &chain, int start, int end, int minSize,
   int sizeHint, QVector<int> &stretchArray, int stretch)
{
   int i;
   int w = 0;
   int wh = 0;
   int max = 0;

   for (i = start; i <= end; i++) {
      QLayoutStruct *data = &chain[i];
      w += data->minimumSize;
      wh += data->sizeHint;
      max += data->maximumSize;
      if (stretchArray.at(i) == 0) {
         data->stretch = qMax(data->stretch, stretch);
      }

      if (i != end) {
         int spacing = data->spacing;
         w += spacing;
         wh += spacing;
         max += spacing;
      }
   }

   if (max < minSize) { // implies w < minSize
      /*
        We must increase the maximum size of at least one of the
        items. qGeomCalc() will put the extra space in between the
        items. We must recover that extra space and put it
        somewhere. It does not really matter where, since the user
        can always specify stretch factors and avoid this code.
      */
      qGeomCalc(chain, start, end - start + 1, 0, minSize);
      int pos = 0;

      for (i = start; i <= end; i++) {
         QLayoutStruct *data = &chain[i];
         int nextPos = (i == end) ? minSize : chain.at(i + 1).pos;
         int realSize = nextPos - pos;
         if (i != end) {
            realSize -= data->spacing;
         }
         if (data->minimumSize < realSize) {
            data->minimumSize = realSize;
         }
         if (data->maximumSize < data->minimumSize) {
            data->maximumSize = data->minimumSize;
         }
         pos = nextPos;
      }

   } else if (w < minSize) {
      qGeomCalc(chain, start, end - start + 1, 0, minSize);

      for (i = start; i <= end; i++) {
         QLayoutStruct *data = &chain[i];
         if (data->minimumSize < data->size) {
            data->minimumSize = data->size;
         }
      }
   }

   if (wh < sizeHint) {
      qGeomCalc(chain, start, end - start + 1, 0, sizeHint);
      for (i = start; i <= end; i++) {
         QLayoutStruct *data = &chain[i];
         if (data->sizeHint < data->size) {
            data->sizeHint = data->size;
         }
      }
   }
}

static QGridBox *&gridAt(QGridBox *grid[], int r, int c, int cc,
   Qt::Orientation orientation = Qt::Vertical)
{
   if (orientation == Qt::Horizontal) {
      qSwap(r, c);
   }
   return grid[(r * cc) + c];
}

void QGridLayoutPrivate::setupSpacings(QVector<QLayoutStruct> &chain, QGridBox *grid[],
            int fixedSpacing, Qt::Orientation orientation)
{
   Q_Q(QGridLayout);
   int numRows = rr;       // or columns if orientation is horizontal
   int numColumns = cc;    // or rows if orientation is horizontal

   if (orientation == Qt::Horizontal) {
      qSwap(numRows, numColumns);
   }

   QStyle *style = nullptr;
   if (fixedSpacing < 0) {
      if (QWidget *parentWidget = q->parentWidget()) {
         style = parentWidget->style();
      }
   }

   for (int c = 0; c < numColumns; ++c) {
      QGridBox *previousBox = nullptr;

      int previousRow = -1;       // previous *non-empty* row

      for (int r = 0; r < numRows; ++r) {
         if (chain.at(r).empty) {
            continue;
         }

         QGridBox *box = gridAt(grid, r, c, cc, orientation);

         if (previousRow != -1 && (!box || previousBox != box)) {
            int spacing = fixedSpacing;

            if (spacing < 0) {
               QSizePolicy::ControlTypes controlTypes1 = QSizePolicy::DefaultType;
               QSizePolicy::ControlTypes controlTypes2 = QSizePolicy::DefaultType;
               if (previousBox) {
                  controlTypes1 = previousBox->item()->controlTypes();
               }
               if (box) {
                  controlTypes2 = box->item()->controlTypes();
               }

               if ((orientation == Qt::Horizontal && hReversed)
                  || (orientation == Qt::Vertical && vReversed)) {
                  qSwap(controlTypes1, controlTypes2);
               }

               if (style)
                  spacing = style->combinedLayoutSpacing(controlTypes1, controlTypes2,
                        orientation, nullptr, q->parentWidget());

            } else {
               if (orientation == Qt::Vertical) {
                  QGridBox *sibling = vReversed ? previousBox : box;

                  if (sibling) {
                     QWidget *wid = sibling->item()->widget();
                     if (wid) {
                        spacing = qMax(spacing, sibling->item()->geometry().top() - wid->geometry().top() );
                     }
                  }
               }
            }

            if (spacing > chain.at(previousRow).spacing) {
               chain[previousRow].spacing = spacing;
            }
         }

         previousBox = box;
         previousRow = r;
      }
   }
}

//#define QT_LAYOUT_DISABLE_CACHING

void QGridLayoutPrivate::setupLayoutData(int hSpacing, int vSpacing)
{
   Q_Q(QGridLayout);

#ifndef QT_LAYOUT_DISABLE_CACHING
   if (!needRecalc) {
      return;
   }
#endif
   has_hfw = false;
   int i;

   for (i = 0; i < rr; i++) {
      rowData[i].init(rStretch.at(i), rMinHeights.at(i));
      rowData[i].maximumSize = rStretch.at(i) ? QLAYOUTSIZE_MAX : rMinHeights.at(i);
   }
   for (i = 0; i < cc; i++) {
      colData[i].init(cStretch.at(i), cMinWidths.at(i));
      colData[i].maximumSize = cStretch.at(i) ? QLAYOUTSIZE_MAX : cMinWidths.at(i);
   }

   int n = things.size();
   QVarLengthArray<QGridLayoutSizeTriple> sizes(n);

   bool has_multi = false;

   /*
       Grid of items. We use it to determine which items are
       adjacent to which and compute the spacings correctly.
   */
   QVarLengthArray<QGridBox *> grid(rr * cc);
   memset(grid.data(), 0, rr * cc * sizeof(QGridBox *));

   /*
       Initialize 'sizes' and 'grid' data structures, and insert
       non-spanning items to our row and column data structures.
   */
   for (i = 0; i < n; ++i) {
      QGridBox *const box = things.at(i);
      sizes[i].minS = box->minimumSize();
      sizes[i].hint = box->sizeHint();
      sizes[i].maxS = box->maximumSize();

      if (box->hasHeightForWidth()) {
         has_hfw = true;
      }

      if (box->row == box->toRow(rr)) {
         addData(box, sizes[i], true, false);
      } else {
         initEmptyMultiBox(rowData, box->row, box->toRow(rr));
         has_multi = true;
      }

      if (box->col == box->toCol(cc)) {
         addData(box, sizes[i], false, true);
      } else {
         initEmptyMultiBox(colData, box->col, box->toCol(cc));
         has_multi = true;
      }

      for (int r = box->row; r <= box->toRow(rr); ++r) {
         for (int c = box->col; c <= box->toCol(cc); ++c) {
            gridAt(grid.data(), r, c, cc) = box;
         }
      }
   }

   setupSpacings(colData, grid.data(), hSpacing, Qt::Horizontal);
   setupSpacings(rowData, grid.data(), vSpacing, Qt::Vertical);

   /*
       Insert multicell items to our row and column data structures.
       This must be done after the non-spanning items to obtain a
       better distribution in distributeMultiBox().
   */
   if (has_multi) {
      for (i = 0; i < n; ++i) {
         QGridBox *const box = things.at(i);

         if (box->row != box->toRow(rr))
            distributeMultiBox(rowData, box->row, box->toRow(rr), sizes[i].minS.height(),
               sizes[i].hint.height(), rStretch, box->vStretch());
         if (box->col != box->toCol(cc))
            distributeMultiBox(colData, box->col, box->toCol(cc), sizes[i].minS.width(),
               sizes[i].hint.width(), cStretch, box->hStretch());
      }
   }

   for (i = 0; i < rr; i++) {
      rowData[i].expansive = rowData.at(i).expansive || rowData.at(i).stretch > 0;
   }
   for (i = 0; i < cc; i++) {
      colData[i].expansive = colData.at(i).expansive || colData.at(i).stretch > 0;
   }

   q->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

   needRecalc = false;
}

void QGridLayoutPrivate::addHfwData(QGridBox *box, int width)
{
   QVector<QLayoutStruct> &rData = *hfwData;
   if (box->hasHeightForWidth()) {
      int hint = box->heightForWidth(width);
      rData[box->row].sizeHint = qMax(hint, rData.at(box->row).sizeHint);
      rData[box->row].minimumSize = qMax(hint, rData.at(box->row).minimumSize);
   } else {
      QSize hint = box->sizeHint();
      QSize minS = box->minimumSize();
      rData[box->row].sizeHint = qMax(hint.height(), rData.at(box->row).sizeHint);
      rData[box->row].minimumSize = qMax(minS.height(), rData.at(box->row).minimumSize);
   }
}

void QGridLayoutPrivate::setupHfwLayoutData()
{
   QVector<QLayoutStruct> &rData = *hfwData;
   for (int i = 0; i < rr; i++) {
      rData[i] = rowData.at(i);
      rData[i].minimumSize = rData[i].sizeHint = rMinHeights.at(i);
   }

   for (int pass = 0; pass < 2; ++pass) {
      for (int i = 0; i < things.size(); ++i) {
         QGridBox *box = things.at(i);
         int r1 = box->row;
         int c1 = box->col;
         int r2 = box->toRow(rr);
         int c2 = box->toCol(cc);
         int w = colData.at(c2).pos + colData.at(c2).size - colData.at(c1).pos;

         if (r1 == r2) {
            if (pass == 0) {
               addHfwData(box, w);
            }
         } else {
            if (pass == 0) {
               initEmptyMultiBox(rData, r1, r2);
            } else {
               QSize hint = box->sizeHint();
               QSize min = box->minimumSize();
               if (box->hasHeightForWidth()) {
                  int hfwh = box->heightForWidth(w);
                  if (hfwh > hint.height()) {
                     hint.setHeight(hfwh);
                  }
                  if (hfwh > min.height()) {
                     min.setHeight(hfwh);
                  }
               }
               distributeMultiBox(rData, r1, r2, min.height(), hint.height(),
                  rStretch, box->vStretch());
            }
         }
      }
   }
   for (int i = 0; i < rr; i++) {
      rData[i].expansive = rData.at(i).expansive || rData.at(i).stretch > 0;
   }
}

void QGridLayoutPrivate::distribute(QRect r, int hSpacing, int vSpacing)
{
   Q_Q(QGridLayout);
   bool visualHReversed = hReversed;
   QWidget *parent = q->parentWidget();
   if (parent && parent->isRightToLeft()) {
      visualHReversed = !visualHReversed;
   }

   setupLayoutData(hSpacing, vSpacing);

   int left, top, right, bottom;
   effectiveMargins(&left, &top, &right, &bottom);
   r.adjust(+left, +top, -right, -bottom);

   qGeomCalc(colData, 0, cc, r.x(), r.width());
   QVector<QLayoutStruct> *rDataPtr;
   if (has_hfw) {
      recalcHFW(r.width());
      qGeomCalc(*hfwData, 0, rr, r.y(), r.height());
      rDataPtr = hfwData;
   } else {
      qGeomCalc(rowData, 0, rr, r.y(), r.height());
      rDataPtr = &rowData;
   }
   QVector<QLayoutStruct> &rData = *rDataPtr;
   int i;

   bool reverse = ((r.bottom() > rect.bottom()) || (r.bottom() == rect.bottom()
            && ((r.right() > rect.right()) != visualHReversed)));
   int n = things.size();
   for (i = 0; i < n; ++i) {
      QGridBox *box = things.at(reverse ? n - i - 1 : i);
      int r2 = box->toRow(rr);
      int c2 = box->toCol(cc);

      int x = colData.at(box->col).pos;
      int y = rData.at(box->row).pos;
      int x2p = colData.at(c2).pos + colData.at(c2).size; // x2+1
      int y2p = rData.at(r2).pos + rData.at(r2).size;    // y2+1
      int w = x2p - x;
      int h = y2p - y;

      if (visualHReversed) {
         x = r.left() + r.right() - x - w + 1;
      }
      if (vReversed) {
         y = r.top() + r.bottom() - y - h + 1;
      }

      box->setGeometry(QRect(x, y, w, h));
   }
}

QRect QGridLayoutPrivate::cellRect(int row, int col) const
{
   if (row < 0 || row >= rr || col < 0 || col >= cc) {
      return QRect();
   }

   const QVector<QLayoutStruct> *rDataPtr;
   if (has_hfw && hfwData) {
      rDataPtr = hfwData;
   } else {
      rDataPtr = &rowData;
   }
   return QRect(colData.at(col).pos, rDataPtr->at(row).pos,
         colData.at(col).size, rDataPtr->at(row).size);
}

QGridLayout::QGridLayout(QWidget *parent)
   : QLayout(*new QGridLayoutPrivate, nullptr, parent)
{
   Q_D(QGridLayout);
   d->expand(1, 1);
}

QGridLayout::QGridLayout()
   : QLayout(*new QGridLayoutPrivate, nullptr, nullptr)
{
   Q_D(QGridLayout);
   d->expand(1, 1);
}

void QGridLayout::setDefaultPositioning(int n, Qt::Orientation orient)
{
   Q_D(QGridLayout);
   if (orient == Qt::Horizontal) {
      d->expand(1, n);
      d->addVertical = false;
   } else {
      d->expand(n, 1);
      d->addVertical = true;
   }
}

QGridLayout::~QGridLayout()
{
   Q_D(QGridLayout);
   d->deleteAll();
}

void QGridLayout::setHorizontalSpacing(int spacing)
{
   Q_D(QGridLayout);
   d->horizontalSpacing = spacing;
   invalidate();
}

int QGridLayout::horizontalSpacing() const
{
   Q_D(const QGridLayout);
   if (d->horizontalSpacing >= 0) {
      return d->horizontalSpacing;
   } else {
      return qSmartSpacing(this, QStyle::PM_LayoutHorizontalSpacing);
   }
}

void QGridLayout::setVerticalSpacing(int spacing)
{
   Q_D(QGridLayout);
   d->verticalSpacing = spacing;
   invalidate();
}

int QGridLayout::verticalSpacing() const
{
   Q_D(const QGridLayout);
   if (d->verticalSpacing >= 0) {
      return d->verticalSpacing;
   } else {
      return qSmartSpacing(this, QStyle::PM_LayoutVerticalSpacing);
   }
}

void QGridLayout::setSpacing(int spacing)
{
   Q_D(QGridLayout);
   d->horizontalSpacing = d->verticalSpacing = spacing;
   invalidate();
}

int QGridLayout::spacing() const
{
   int hSpacing = horizontalSpacing();
   if (hSpacing == verticalSpacing()) {
      return hSpacing;
   } else {
      return -1;
   }
}

int QGridLayout::rowCount() const
{
   Q_D(const QGridLayout);
   return d->numRows();
}

int QGridLayout::columnCount() const
{
   Q_D(const QGridLayout);
   return d->numCols();
}

QSize QGridLayout::sizeHint() const
{
   Q_D(const QGridLayout);
   QSize result(d->sizeHint(horizontalSpacing(), verticalSpacing()));
   int left, top, right, bottom;
   d->effectiveMargins(&left, &top, &right, &bottom);
   result += QSize(left + right, top + bottom);
   return result;
}

QSize QGridLayout::minimumSize() const
{
   Q_D(const QGridLayout);
   QSize result(d->minimumSize(horizontalSpacing(), verticalSpacing()));
   int left, top, right, bottom;
   d->effectiveMargins(&left, &top, &right, &bottom);
   result += QSize(left + right, top + bottom);
   return result;
}

QSize QGridLayout::maximumSize() const
{
   Q_D(const QGridLayout);

   QSize s = d->maximumSize(horizontalSpacing(), verticalSpacing());
   int left, top, right, bottom;
   d->effectiveMargins(&left, &top, &right, &bottom);
   s += QSize(left + right, top + bottom);
   s = s.boundedTo(QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX));

   if (alignment() & Qt::AlignHorizontal_Mask) {
      s.setWidth(QLAYOUTSIZE_MAX);
   }
   if (alignment() & Qt::AlignVertical_Mask) {
      s.setHeight(QLAYOUTSIZE_MAX);
   }
   return s;
}

bool QGridLayout::hasHeightForWidth() const
{
   return const_cast<QGridLayout *>(this)->d_func()->hasHeightForWidth(horizontalSpacing(), verticalSpacing());
}

int QGridLayout::heightForWidth(int w) const
{
   Q_D(const QGridLayout);
   QGridLayoutPrivate *dat = const_cast<QGridLayoutPrivate *>(d);
   return dat->heightForWidth(w, horizontalSpacing(), verticalSpacing());
}

int QGridLayout::minimumHeightForWidth(int w) const
{
   Q_D(const QGridLayout);
   QGridLayoutPrivate *dat = const_cast<QGridLayoutPrivate *>(d);
   return dat->minimumHeightForWidth(w, horizontalSpacing(), verticalSpacing());
}

int QGridLayout::count() const
{
   Q_D(const QGridLayout);
   return d->count();
}

QLayoutItem *QGridLayout::itemAt(int index) const
{
   Q_D(const QGridLayout);
   return d->itemAt(index);
}

QLayoutItem *QGridLayout::itemAtPosition(int row, int column) const
{
   Q_D(const QGridLayout);
   int n = d->things.count();

   for (int i = 0; i < n; ++i) {
      QGridBox *box = d->things.at(i);
      if (row >= box->row && row <= box->toRow(d->rr)
         && column >= box->col && column <= box->toCol(d->cc)) {
         return box->item();
      }
   }

   return nullptr;
}

QLayoutItem *QGridLayout::takeAt(int index)
{
   Q_D(QGridLayout);
   return d->takeAt(index);
}

void QGridLayout::getItemPosition(int index, int *row, int *column, int *rowSpan, int *columnSpan) const
{
   Q_D(const QGridLayout);
   d->getItemPosition(index, row, column, rowSpan, columnSpan);
}

void QGridLayout::setGeometry(const QRect &rect)
{
   Q_D(QGridLayout);
   if (d->isDirty() || rect != geometry()) {
      QRect cr = alignment() ? alignmentRect(rect) : rect;
      d->distribute(cr, horizontalSpacing(), verticalSpacing());
      QLayout::setGeometry(rect);
   }
}

QRect QGridLayout::cellRect(int row, int column) const
{
   Q_D(const QGridLayout);
   return d->cellRect(row, column);
}

void QGridLayout::addItem(QLayoutItem *item)
{
   Q_D(QGridLayout);
   int r, c;
   d->getNextPos(r, c);
   addItem(item, r, c);
}

void QGridLayout::addItem(QLayoutItem *item, int row, int column, int rowSpan, int columnSpan, Qt::Alignment alignment)
{
   Q_D(QGridLayout);
   QGridBox *b = new QGridBox(item);
   b->setAlignment(alignment);
   d->add(b, row, (rowSpan < 0) ? -1 : row + rowSpan - 1, column, (columnSpan < 0) ? -1 : column + columnSpan - 1);
   invalidate();
}

void QGridLayout::addWidget(QWidget *widget, int row, int column, Qt::Alignment alignment)
{
   Q_D(QGridLayout);

   if (! d->checkWidget(widget)) {
      return;
   }

   if (row < 0 || column < 0) {
      qWarning("QGridLayout::addWidget() Unable to add %s/%s to %s/%s at row %d column %d",
         csPrintable(widget->metaObject()->className()), csPrintable(widget->objectName()),
         csPrintable(metaObject()->className()), csPrintable(objectName()), row, column);

      return;
   }

   addChildWidget(widget);
   QWidgetItem *b = QLayoutPrivate::createWidgetItem(this, widget);
   addItem(b, row, column, 1, 1, alignment);
}

void QGridLayout::addWidget(QWidget *widget, int fromRow, int fromColumn,
   int rowSpan, int columnSpan, Qt::Alignment alignment)
{
   Q_D(QGridLayout);

   if (!d->checkWidget(widget)) {
      return;
   }

   int toRow = (rowSpan < 0) ? -1 : fromRow + rowSpan - 1;
   int toColumn = (columnSpan < 0) ? -1 : fromColumn + columnSpan - 1;
   addChildWidget(widget);
   QGridBox *b = new QGridBox(this, widget);
   b->setAlignment(alignment);
   d->add(b, fromRow, toRow, fromColumn, toColumn);
   invalidate();
}

void QGridLayout::addLayout(QLayout *layout, int row, int column, Qt::Alignment alignment)
{
   Q_D(QGridLayout);

   if (!d->checkLayout(layout)) {
      return;
   }

   if (!adoptLayout(layout)) {
      return;
   }
   QGridBox *b = new QGridBox(layout);
   b->setAlignment(alignment);
   d->add(b, row, column);
}

void QGridLayout::addLayout(QLayout *layout, int row, int column,
      int rowSpan, int columnSpan, Qt::Alignment alignment)
{
   Q_D(QGridLayout);
   if (!d->checkLayout(layout)) {
      return;
   }

   if (!adoptLayout(layout)) {
      return;
   }
   QGridBox *b = new QGridBox(layout);
   b->setAlignment(alignment);
   d->add(b, row, (rowSpan < 0) ? -1 : row + rowSpan - 1, column, (columnSpan < 0) ? -1 : column + columnSpan - 1);
}

void QGridLayout::setRowStretch(int row, int stretch)
{
   Q_D(QGridLayout);
   d->setRowStretch(row, stretch);
   invalidate();
}

int QGridLayout::rowStretch(int row) const
{
   Q_D(const QGridLayout);
   return d->rowStretch(row);
}

int QGridLayout::columnStretch(int column) const
{
   Q_D(const QGridLayout);
   return d->colStretch(column);
}

void QGridLayout::setColumnStretch(int column, int stretch)
{
   Q_D(QGridLayout);
   d->setColStretch(column, stretch);
   invalidate();
}

void QGridLayout::setRowMinimumHeight(int row, int minSize)
{
   Q_D(QGridLayout);
   d->setRowMinimumHeight(row, minSize);
   invalidate();
}

int QGridLayout::rowMinimumHeight(int row) const
{
   Q_D(const QGridLayout);
   return d->rowSpacing(row);
}

void QGridLayout::setColumnMinimumWidth(int column, int minSize)
{
   Q_D(QGridLayout);
   d->setColumnMinimumWidth(column, minSize);
   invalidate();
}

int QGridLayout::columnMinimumWidth(int column) const
{
   Q_D(const QGridLayout);
   return d->colSpacing(column);
}

Qt::Orientations QGridLayout::expandingDirections() const
{
   Q_D(const QGridLayout);
   return d->expandingDirections(horizontalSpacing(), verticalSpacing());
}

void QGridLayout::setOriginCorner(Qt::Corner corner)
{
   Q_D(QGridLayout);
   d->setReversed(corner == Qt::BottomLeftCorner || corner == Qt::BottomRightCorner,
      corner == Qt::TopRightCorner || corner == Qt::BottomRightCorner);
}

Qt::Corner QGridLayout::originCorner() const
{
   Q_D(const QGridLayout);
   if (d->horReversed()) {
      return d->verReversed() ? Qt::BottomRightCorner : Qt::TopRightCorner;
   } else {
      return d->verReversed() ? Qt::BottomLeftCorner : Qt::TopLeftCorner;
   }
}

void QGridLayout::invalidate()
{
   Q_D(QGridLayout);
   d->setDirty();
   QLayout::invalidate();
}
