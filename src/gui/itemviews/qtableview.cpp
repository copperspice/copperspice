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

#include <qtableview.h>
#include <qtableview_p.h>

#ifndef QT_NO_TABLEVIEW

#include <qabstractbutton.h>
#include <qapplication.h>
#include <qbitarray.h>
#include <qevent.h>
#include <qitemdelegate.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qsize.h>
#include <qstyle.h>

#include <qheaderview_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

// internal - Add a span to the collection. the collection takes the ownership.
void QSpanCollection::addSpan(QSpanCollection::Span *span)
{
   spans.append(span);
   Index::iterator it_y = index.lowerBound(-span->top());

   if (it_y == index.end() || it_y.key() != -span->top()) {
      //there is no spans that starts with the row in the index, so create a sublist for it.
      SubIndex sub_index;
      if (it_y != index.end()) {
         //the previouslist is the list of spans that sarts _before_ the row of the span.
         // and which may intersect this row.
         const SubIndex previousList = it_y.value();

         for (Span *s : previousList) {
            //If a subspans intersect the row, we need to split it into subspans
            if (s->bottom() >= span->top()) {
               sub_index.insert(-s->left(), s);
            }
         }
      }

      it_y = index.insert(-span->top(), sub_index);
      // we will insert span to *it_y in the later loop
   }

   // insert the span as supspan in all the lists that intesects the span
   while (-it_y.key() <= span->bottom()) {
      (*it_y).insert(-span->left(), span);
      if (it_y == index.begin()) {
         break;
      }
      --it_y;
   }
}

// Has to be called after the height and width of a span is changed.
// old_height is the height before the change, if the size of the span is now 0x0 the span will be deleted.
void QSpanCollection::updateSpan(QSpanCollection::Span *span, int old_height)
{
   if (old_height < span->height()) {
      // add the span as subspan in all the lists that intersect the new covered columns
      Index::iterator it_y = index.lowerBound(-(span->top() + old_height - 1));
      Q_ASSERT(it_y != index.end()); //it_y must exist since the span is in the list
      while (-it_y.key() <= span->bottom()) {
         (*it_y).insert(-span->left(), span);
         if (it_y == index.begin()) {
            break;
         }
         --it_y;
      }

   } else if (old_height > span->height()) {
      // remove the span from all the subspans lists that intersect the columns not covered anymore
      Index::iterator it_y = index.lowerBound(-qMax(span->bottom(), span->top())); //qMax useful if height is 0
      Q_ASSERT(it_y != index.end()); //it_y must exist since the span is in the list

      while (-it_y.key() <= span->top() + old_height - 1) {
         if (-it_y.key() > span->bottom()) {
            int removed = (*it_y).remove(-span->left());
            Q_ASSERT(removed == 1);

            if (it_y->isEmpty()) {
               it_y = index.erase(it_y);
            }
         }

         if (it_y == index.begin()) {
            break;
         }

         --it_y;
      }
   }

   if (span->width() == 0 && span->height() == 0) {
      spans.removeOne(span);
      delete span;
   }
}

QSpanCollection::Span *QSpanCollection::spanAt(int x, int y) const
{
   Index::const_iterator it_y = index.lowerBound(-y);
   if (it_y == index.end()) {
      return nullptr;
   }

   SubIndex::const_iterator it_x = (*it_y).lowerBound(-x);
   if (it_x == (*it_y).end()) {
      return nullptr;
   }

   Span *span = *it_x;
   if (span->right() >= x && span->bottom() >= y) {
      return span;
   }

   return nullptr;
}

void QSpanCollection::clear()
{
   qDeleteAll(spans);
   index.clear();
   spans.clear();
}

QList<QSpanCollection::Span *> QSpanCollection::spansInRect(int x, int y, int w, int h) const
{
   QSet<Span *> list;
   Index::const_iterator it_y = index.lowerBound(-y);
   if (it_y == index.end()) {
      --it_y;
   }

   while (-it_y.key() <= y + h) {
      SubIndex::const_iterator it_x = (*it_y).lowerBound(-x);
      if (it_x == (*it_y).end()) {
         --it_x;
      }

      while (-it_x.key() <= x + w) {
         Span *s = *it_x;
         if (s->bottom() >= y && s->right() >= x) {
            list << s;
         }

         if (it_x == (*it_y).begin()) {
            break;
         }
         --it_x;
      }

      if (it_y == index.begin()) {
         break;
      }
      --it_y;
   }
   return list.toList();
}

QDebug operator<<(QDebug str, const QSpanCollection::Span &span)
{
   str << "(" << span.top() << "," << span.left() << "," << span.bottom() << "," << span.right() << ")";
   return str;
}

void QSpanCollection::updateInsertedRows(int start, int end)
{
   if (spans.isEmpty()) {
      return;
   }

   int delta = end - start + 1;

   for (SpanList::iterator it = spans.begin(); it != spans.end(); ++it) {
      Span *span = *it;

      if (span->m_bottom < start) {
         continue;
      }

      if (span->m_top >= start) {
         span->m_top += delta;
      }

      span->m_bottom += delta;
   }

   for (Index::iterator it_y = index.begin(); it_y != index.end(); ) {
      int y = -it_y.key();
      if (y < start) {
         ++it_y;
         continue;
      }

      index.insert(-y - delta, it_y.value());
      it_y = index.erase(it_y);
   }
}

void QSpanCollection::updateInsertedColumns(int start, int end)
{
   if (spans.isEmpty()) {
      return;
   }

   int delta = end - start + 1;

   for (SpanList::iterator it = spans.begin(); it != spans.end(); ++it) {
      Span *span = *it;

      if (span->m_right < start) {
         continue;
      }

      if (span->m_left >= start) {
         span->m_left += delta;
      }

      span->m_right += delta;
   }

   for (Index::iterator it_y = index.begin(); it_y != index.end(); ++it_y) {
      SubIndex &subindex = it_y.value();
      for (SubIndex::iterator it = subindex.begin(); it != subindex.end(); ) {
         int x = -it.key();
         if (x < start) {
            ++it;
            continue;
         }
         subindex.insert(-x - delta, it.value());
         it = subindex.erase(it);
      }
   }

}

bool QSpanCollection::cleanSpanSubIndex(QSpanCollection::SubIndex &subindex, int y, bool update)
{
   if (subindex.isEmpty()) {
      return true;
   }

   bool should_be_deleted = true;
   SubIndex::iterator it = subindex.end();
   do {
      --it;
      int x = -it.key();
      Span *span = it.value();
      if (span->will_be_deleted) {
         it = subindex.erase(it);
         continue;
      }

      if (update && span->m_left != x) {
         subindex.insert(-span->m_left, span);
         it = subindex.erase(it);
      }

      if (should_be_deleted && span->m_top == y) {
         should_be_deleted = false;
      }

   } while (it != subindex.begin());

   return should_be_deleted;
}

void QSpanCollection::updateRemovedRows(int start, int end)
{
   if (spans.isEmpty()) {
      return;
   }

   SpanList spansToBeDeleted;
   int delta = end - start + 1;

   for (SpanList::iterator it = spans.begin(); it != spans.end(); ) {
      Span *span = *it;

      if (span->m_bottom < start) {
         ++it;
         continue;
      }

      if (span->m_top < start) {
         if (span->m_bottom <= end) {
            span->m_bottom = start - 1;
         } else {
            span->m_bottom -= delta;
         }
      } else {
         if (span->m_bottom > end) {
            if (span->m_top <= end) {
               span->m_top = start;
            } else {
               span->m_top -= delta;
            }
            span->m_bottom -= delta;
         } else {
            span->will_be_deleted = true;
         }
      }
      if (span->m_top == span->m_bottom && span->m_left == span->m_right) {
         span->will_be_deleted = true;
      }
      if (span->will_be_deleted) {
         spansToBeDeleted.append(span);
         it = spans.erase(it);
      } else {
         ++it;
      }
   }

   if (spans.isEmpty()) {
      qDeleteAll(spansToBeDeleted);
      index.clear();
      return;
   }

   Index::iterator it_y = index.end();
   do {
      --it_y;
      int y = -it_y.key();
      SubIndex &subindex = it_y.value();
      if (y < start) {
         if (cleanSpanSubIndex(subindex, y)) {
            it_y = index.erase(it_y);
         }

      } else if (y >= start && y <= end) {
         bool span_at_start = false;
         SubIndex spansToBeMoved;
         for (SubIndex::iterator it = subindex.begin(); it != subindex.end(); ++it) {
            Span *span = it.value();
            if (span->will_be_deleted) {
               continue;
            }
            if (!span_at_start && span->m_top == start) {
               span_at_start = true;
            }
            spansToBeMoved.insert(it.key(), span);
         }

         if (y == start && span_at_start) {
            subindex.clear();
         } else {
            it_y = index.erase(it_y);
         }

         if (span_at_start) {
            Index::iterator it_start;
            if (y == start) {
               it_start = it_y;
            } else {
               it_start = index.find(-start);
               if (it_start == index.end()) {
                  it_start = index.insert(-start, SubIndex());
               }
            }
            SubIndex &start_subindex = it_start.value();
            for (SubIndex::iterator it = spansToBeMoved.begin(); it != spansToBeMoved.end(); ++it) {
               start_subindex.insert(it.key(), it.value());
            }
         }

      } else {
         if (y == end + 1) {
            Index::iterator it_top = index.find(-y + delta);
            if (it_top == index.end()) {
               it_top = index.insert(-y + delta, SubIndex());
            }
            for (SubIndex::iterator it = subindex.begin(); it != subindex.end(); ) {
               Span *span = it.value();
               if (!span->will_be_deleted) {
                  it_top.value().insert(it.key(), span);
               }
               ++it;
            }
         } else {
            index.insert(-y + delta, subindex);
         }
         it_y = index.erase(it_y);
      }
   } while (it_y != index.begin());

   qDeleteAll(spansToBeDeleted);
}

void QSpanCollection::updateRemovedColumns(int start, int end)
{
   if (spans.isEmpty()) {
      return;
   }

   SpanList toBeDeleted;
   int delta = end - start + 1;

   for (SpanList::iterator it = spans.begin(); it != spans.end(); ) {
      Span *span = *it;

      if (span->m_right < start) {
         ++it;
         continue;
      }

      if (span->m_left < start) {
         if (span->m_right <= end) {
            span->m_right = start - 1;
         } else {
            span->m_right -= delta;
         }

      } else {
         if (span->m_right > end) {
            if (span->m_left <= end) {
               span->m_left = start;
            } else {
               span->m_left -= delta;
            }
            span->m_right -= delta;
         } else {
            span->will_be_deleted = true;
         }
      }

      if (span->m_top == span->m_bottom && span->m_left == span->m_right) {
         span->will_be_deleted = true;
      }

      if (span->will_be_deleted) {
         toBeDeleted.append(span);
         it = spans.erase(it);
      } else {
         ++it;
      }
   }

   if (spans.isEmpty()) {
      qDeleteAll(toBeDeleted);
      index.clear();
      return;
   }

   for (Index::iterator it_y = index.begin(); it_y != index.end(); ) {
      int y = -it_y.key();
      if (cleanSpanSubIndex(it_y.value(), y, true)) {
         it_y = index.erase(it_y);
      } else {
         ++it_y;
      }
   }

   qDeleteAll(toBeDeleted);
}

class QTableCornerButton : public QAbstractButton
{
   GUI_CS_OBJECT(QTableCornerButton)

 public:
   QTableCornerButton(QWidget *parent) : QAbstractButton(parent) {}

   void paintEvent(QPaintEvent *)  override {
      QStyleOptionHeader opt;
      opt.initFrom(this);
      QStyle::State state = QStyle::State_None;

      if (isEnabled()) {
         state |= QStyle::State_Enabled;
      }
      if (isActiveWindow()) {
         state |= QStyle::State_Active;
      }
      if (isDown()) {
         state |= QStyle::State_Sunken;
      }
      opt.state = state;
      opt.rect = rect();
      opt.position = QStyleOptionHeader::OnlyOneSection;
      QPainter painter(this);
      style()->drawControl(QStyle::CE_Header, &opt, &painter, this);
   }
};

void QTableViewPrivate::init()
{
   Q_Q(QTableView);

   q->setEditTriggers(editTriggers | QAbstractItemView::AnyKeyPressed);

   QHeaderView *vertical = new QHeaderView(Qt::Vertical, q);
   vertical->setSectionsClickable(true);
   vertical->setHighlightSections(true);
   q->setVerticalHeader(vertical);

   QHeaderView *horizontal = new QHeaderView(Qt::Horizontal, q);
   horizontal->setSectionsClickable(true);
   horizontal->setHighlightSections(true);
   q->setHorizontalHeader(horizontal);

   tabKeyNavigation = true;

   QTableCornerButton *tmpObj = new QTableCornerButton(q);
   cornerWidget = tmpObj;
   cornerWidget->setFocusPolicy(Qt::NoFocus);

   QObject::connect(tmpObj, &QTableCornerButton::clicked, q, &QTableView::selectAll);
}

void QTableViewPrivate::trimHiddenSelections(QItemSelectionRange *range) const
{
   Q_ASSERT(range && range->isValid());

   int top = range->top();
   int left = range->left();
   int bottom = range->bottom();
   int right = range->right();

   while (bottom >= top && verticalHeader->isSectionHidden(bottom)) {
      --bottom;
   }

   while (right >= left && horizontalHeader->isSectionHidden(right)) {
      --right;
   }

   if (top > bottom || left > right) { // everything is hidden
      *range = QItemSelectionRange();
      return;
   }

   while (verticalHeader->isSectionHidden(top) && top <= bottom) {
      ++top;
   }

   while (horizontalHeader->isSectionHidden(left) && left <= right) {
      ++left;
   }

   if (top > bottom || left > right) { // everything is hidden
      *range = QItemSelectionRange();
      return;
   }

   QModelIndex bottomRight = model->index(bottom, right, range->parent());
   QModelIndex topLeft = model->index(top, left, range->parent());
   *range = QItemSelectionRange(topLeft, bottomRight);
}

void QTableViewPrivate::setSpan(int row, int column, int rowSpan, int columnSpan)
{
   if (row < 0 || column < 0 || rowSpan <= 0 || columnSpan <= 0) {
      qWarning("QTableView::setSpan() Invalid span (%d, %d, %d, %d)",
            row, column, rowSpan, columnSpan);
      return;
   }

   QSpanCollection::Span *sp = spans.spanAt(column, row);
   if (sp) {
      if (sp->top() != row || sp->left() != column) {
         qWarning("QTableView::setSpan() Span can not overlap");
         return;
      }

      if (rowSpan == 1 && columnSpan == 1) {
         rowSpan = columnSpan = 0;
      }

      const int old_height = sp->height();
      sp->m_bottom = row + rowSpan - 1;
      sp->m_right = column + columnSpan - 1;
      spans.updateSpan(sp, old_height);
      return;

   } else if (rowSpan == 1 && columnSpan == 1) {
      qWarning("QTableView::setSpan() Unable to add a single cell span");
      return;
   }

   sp = new QSpanCollection::Span(row, column, rowSpan, columnSpan);
   spans.addSpan(sp);
}

QSpanCollection::Span QTableViewPrivate::span(int row, int column) const
{
   QSpanCollection::Span *sp = spans.spanAt(column, row);

   if (sp) {
      return *sp;
   }

   return QSpanCollection::Span(row, column, 1, 1);
}

int QTableViewPrivate::sectionSpanEndLogical(const QHeaderView *header, int logical, int span) const
{
   int visual = header->visualIndex(logical);

   for (int i = 1; i < span; ) {
      if (++visual >= header->count()) {
         break;
      }
      logical = header->logicalIndex(visual);
      ++i;
   }
   return logical;
}

int QTableViewPrivate::sectionSpanSize(const QHeaderView *header, int logical, int span) const
{
   int endLogical = sectionSpanEndLogical(header, logical, span);
   return header->sectionPosition(endLogical)
      - header->sectionPosition(logical)
      + header->sectionSize(endLogical);
}

bool QTableViewPrivate::spanContainsSection(const QHeaderView *header, int logical, int spanLogical, int span) const
{
   if (logical == spanLogical) {
      return true;   // it's the start of the span
   }

   int visual = header->visualIndex(spanLogical);

   for (int i = 1; i < span; ) {
      if (++visual >= header->count()) {
         break;
      }

      spanLogical = header->logicalIndex(visual);
      if (logical == spanLogical) {
         return true;
      }

      ++i;
   }

   return false;
}

QRect QTableViewPrivate::visualSpanRect(const QSpanCollection::Span &span) const
{
   Q_Q(const QTableView);

   // vertical
   int row = span.top();
   int rowp = verticalHeader->sectionViewportPosition(row);
   int rowh = rowSpanHeight(row, span.height());

   // horizontal
   int column = span.left();
   int colw = columnSpanWidth(column, span.width());
   if (q->isRightToLeft()) {
      column = span.right();
   }
   int colp = horizontalHeader->sectionViewportPosition(column);

   const int i = showGrid ? 1 : 0;
   if (q->isRightToLeft()) {
      return QRect(colp + i, rowp, colw - i, rowh - i);
   }

   return QRect(colp, rowp, colw - i, rowh - i);
}

void QTableViewPrivate::drawAndClipSpans(const QRegion &area, QPainter *painter,
      const QStyleOptionViewItem &option, QBitArray *drawn,
      int firstVisualRow, int lastVisualRow, int firstVisualColumn, int lastVisualColumn)
{
   bool alternateBase = false;
   QRegion region = viewport->rect();

   QList<QSpanCollection::Span *> visibleSpans;
   bool sectionMoved = verticalHeader->sectionsMoved() || horizontalHeader->sectionsMoved();

   if (!sectionMoved) {
      visibleSpans = spans.spansInRect(logicalColumn(firstVisualColumn), logicalRow(firstVisualRow),
            lastVisualColumn - firstVisualColumn + 1, lastVisualRow - firstVisualRow + 1);
   } else {
      QSet<QSpanCollection::Span *> set;

      for (int x = firstVisualColumn; x <= lastVisualColumn; x++)  {
         for (int y = firstVisualRow; y <= lastVisualRow; y++) {
            set.insert(spans.spanAt(x, y));
         }
      }

      set.remove(nullptr);
      visibleSpans = set.toList();
   }

   for (QSpanCollection::Span *span : visibleSpans) {
      int row = span->top();
      int col = span->left();
      QModelIndex index = model->index(row, col, root);

      if (!index.isValid()) {
         continue;
      }

      QRect rect = visualSpanRect(*span);
      rect.translate(scrollDelayOffset);
      if (!area.intersects(rect)) {
         continue;
      }

      QStyleOptionViewItem opt = option;
      opt.rect = rect;
      alternateBase = alternatingColors && (span->top() & 1);

      if (alternateBase) {
         opt.features |= QStyleOptionViewItem::Alternate;
      } else {
         opt.features &= ~QStyleOptionViewItem::Alternate;
      }

      drawCell(painter, opt, index);
      region -= rect;
      for (int r = span->top(); r <= span->bottom(); ++r) {
         const int vr = visualRow(r);
         if (vr < firstVisualRow || vr > lastVisualRow) {
            continue;
         }

         for (int c = span->left(); c <= span->right(); ++c) {
            const int vc = visualColumn(c);
            if (vc < firstVisualColumn  || vc > lastVisualColumn) {
               continue;
            }

            drawn->setBit((vr - firstVisualRow) * (lastVisualColumn - firstVisualColumn + 1)
               + vc - firstVisualColumn);
         }
      }

   }
   painter->setClipRegion(region);
}

void QTableViewPrivate::_q_updateSpanInsertedRows(const QModelIndex &parent, int start, int end)
{
   (void) parent;
   spans.updateInsertedRows(start, end);
}

void QTableViewPrivate::_q_updateSpanInsertedColumns(const QModelIndex &parent, int start, int end)
{
   (void) parent;
   spans.updateInsertedColumns(start, end);
}

void QTableViewPrivate::_q_updateSpanRemovedRows(const QModelIndex &parent, int start, int end)
{
   (void) parent;
   spans.updateRemovedRows(start, end);
}

void QTableViewPrivate::_q_updateSpanRemovedColumns(const QModelIndex &parent, int start, int end)
{
   (void) parent;
   spans.updateRemovedColumns(start, end);
}

void QTableViewPrivate::drawCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)
{
   Q_Q(QTableView);
   QStyleOptionViewItem opt = option;

   if (selectionModel && selectionModel->isSelected(index)) {
      opt.state |= QStyle::State_Selected;
   }

   if (index == hover) {
      opt.state |= QStyle::State_MouseOver;
   }

   if (option.state & QStyle::State_Enabled) {
      QPalette::ColorGroup cg;
      if ((model->flags(index) & Qt::ItemIsEnabled) == 0) {
         opt.state &= ~QStyle::State_Enabled;
         cg = QPalette::Disabled;
      } else {
         cg = QPalette::Normal;
      }
      opt.palette.setCurrentColorGroup(cg);
   }

   if (index == q->currentIndex()) {
      const bool focus = (q->hasFocus() || viewport->hasFocus()) && q->currentIndex().isValid();
      if (focus) {
         opt.state |= QStyle::State_HasFocus;
      }
   }

   q->style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, painter, q);

   q->itemDelegate(index)->paint(painter, opt, index);
}

int QTableViewPrivate::widthHintForIndex(const QModelIndex &index, int hint, const QStyleOptionViewItem &option) const
{
   Q_Q(const QTableView);
   QWidget *editor = editorForIndex(index).widget.data();
   if (editor && persistent.contains(editor)) {
      hint = qMax(hint, editor->sizeHint().width());
      int min = editor->minimumSize().width();
      int max = editor->maximumSize().width();
      hint = qBound(min, hint, max);
   }

   hint = qMax(hint, q->itemDelegate(index)->sizeHint(option, index).width());

   return hint;
}

int QTableViewPrivate::heightHintForIndex(const QModelIndex &index, int hint, QStyleOptionViewItem &option) const
{
   Q_Q(const QTableView);
   QWidget *editor = editorForIndex(index).widget.data();

   if (editor && persistent.contains(editor)) {
      hint = qMax(hint, editor->sizeHint().height());
      int min = editor->minimumSize().height();
      int max = editor->maximumSize().height();
      hint = qBound(min, hint, max);
   }


   if (wrapItemText) {
      // for wrapping boundaries

      option.rect.setY(q->rowViewportPosition(index.row()));
      int height = q->rowHeight(index.row());

      // if the option.height == 0 then q->itemDelegate(index)->sizeHint(option, index) will be wrong.
      // The option.height == 0 is used to conclude that the text is not wrapped, and hence it will
      // (exactly like widthHintForIndex) return a QSize with a long width (that we don't use) -
      // and the height of the text if it was/is on one line.

      // What we want is a height hint for the current width (and we know that this section is not hidden)
      // therefore we catch this special situation with the following code:

      if (height == 0) {
         height = 1;
      }

      option.rect.setHeight(height);
      option.rect.setX(q->columnViewportPosition(index.column()));
      option.rect.setWidth(q->columnWidth(index.column()));
   }

   hint = qMax(hint, q->itemDelegate(index)->sizeHint(option, index).height());

   return hint;
}

QTableView::QTableView(QWidget *parent)
   : QAbstractItemView(*new QTableViewPrivate, parent)
{
   Q_D(QTableView);
   d->init();
}

QTableView::QTableView(QTableViewPrivate &dd, QWidget *parent)
   : QAbstractItemView(dd, parent)
{
   Q_D(QTableView);
   d->init();
}

QTableView::~QTableView()
{
}

QSize QTableView::viewportSizeHint() const
{
   Q_D(const QTableView);

   QSize result( (d->verticalHeader->isHidden() ? 0 : d->verticalHeader->width()) + d->horizontalHeader->length(),
      (d->horizontalHeader->isHidden() ? 0 : d->horizontalHeader->height()) + d->verticalHeader->length());
   result += QSize(verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0,
         horizontalScrollBar()->isVisible() ? horizontalScrollBar()->height() : 0);

   return result;
}

void QTableView::setModel(QAbstractItemModel *model)
{
   Q_D(QTableView);

   if (model == d->model) {
      return;
   }

   // disconnect from the old model
   if (d->model && d->model != QAbstractItemModelPrivate::staticEmptyModel()) {
      disconnect(d->model, &QAbstractItemModel::rowsInserted,    this, &QTableView::_q_updateSpanInsertedRows);
      disconnect(d->model, &QAbstractItemModel::columnsInserted, this, &QTableView::_q_updateSpanInsertedColumns);
      disconnect(d->model, &QAbstractItemModel::rowsRemoved,     this, &QTableView::_q_updateSpanRemovedRows);
      disconnect(d->model, &QAbstractItemModel::columnsRemoved,  this, &QTableView::_q_updateSpanRemovedColumns);
   }

   if (d->selectionModel) {
      // support row editing
      disconnect(d->selectionModel.data(), &QItemSelectionModel::currentRowChanged, d->model, &QAbstractItemModel::submit);
   }

   if (model) {
      // connect to the new one
      connect(model, &QAbstractItemModel::rowsInserted,    this, &QTableView::_q_updateSpanInsertedRows);
      connect(model, &QAbstractItemModel::columnsInserted, this, &QTableView::_q_updateSpanInsertedColumns);
      connect(model, &QAbstractItemModel::rowsRemoved,     this, &QTableView::_q_updateSpanRemovedRows);
      connect(model, &QAbstractItemModel::columnsRemoved,  this, &QTableView::_q_updateSpanRemovedColumns);
   }

   d->verticalHeader->setModel(model);
   d->horizontalHeader->setModel(model);
   QAbstractItemView::setModel(model);
}

void QTableView::setRootIndex(const QModelIndex &index)
{
   Q_D(QTableView);

   if (index == d->root) {
      viewport()->update();
      return;
   }

   d->verticalHeader->setRootIndex(index);
   d->horizontalHeader->setRootIndex(index);
   QAbstractItemView::setRootIndex(index);
}

void QTableView::doItemsLayout()
{
   Q_D(QTableView);

   QAbstractItemView::doItemsLayout();
   d->verticalHeader->d_func()->setScrollOffset(verticalScrollBar(), verticalScrollMode());

   if (!d->verticalHeader->updatesEnabled()) {
      d->verticalHeader->setUpdatesEnabled(true);
   }
}

void QTableView::setSelectionModel(QItemSelectionModel *selectionModel)
{
   Q_D(QTableView);
   Q_ASSERT(selectionModel);

   if (d->selectionModel) {
      // support row editing
      disconnect(d->selectionModel.data(), &QItemSelectionModel::currentRowChanged, d->model, &QAbstractItemModel::submit);
   }

   d->verticalHeader->setSelectionModel(selectionModel);
   d->horizontalHeader->setSelectionModel(selectionModel);
   QAbstractItemView::setSelectionModel(selectionModel);

   if (d->selectionModel) {
      // support row editing
      connect(d->selectionModel.data(), &QItemSelectionModel::currentRowChanged, d->model, &QAbstractItemModel::submit);
   }
}

QHeaderView *QTableView::horizontalHeader() const
{
   Q_D(const QTableView);
   return d->horizontalHeader;
}

QHeaderView *QTableView::verticalHeader() const
{
   Q_D(const QTableView);
   return d->verticalHeader;
}

void QTableView::setHorizontalHeader(QHeaderView *header)
{
   Q_D(QTableView);

   if (!header || header == d->horizontalHeader) {
      return;
   }

   if (d->horizontalHeader && d->horizontalHeader->parent() == this) {
      delete d->horizontalHeader;
   }

   d->horizontalHeader = header;
   d->horizontalHeader->setParent(this);
   d->horizontalHeader->d_func()->setAllowUserMoveOfSection0(true);

   if (!d->horizontalHeader->model()) {
      d->horizontalHeader->setModel(d->model);

      if (d->selectionModel) {
         d->horizontalHeader->setSelectionModel(d->selectionModel);
      }
   }


   connect(d->horizontalHeader, &QHeaderView::sectionResized,             this, &QTableView::columnResized);
   connect(d->horizontalHeader, &QHeaderView::sectionMoved,               this, &QTableView::columnMoved);
   connect(d->horizontalHeader, &QHeaderView::sectionCountChanged,        this, &QTableView::columnCountChanged);
   connect(d->horizontalHeader, &QHeaderView::sectionPressed,             this, &QTableView::selectColumn);
   connect(d->horizontalHeader, &QHeaderView::sectionEntered,             this, &QTableView::_q_selectColumn);
   connect(d->horizontalHeader, &QHeaderView::sectionHandleDoubleClicked, this, &QTableView::resizeColumnToContents);
   connect(d->horizontalHeader, &QHeaderView::geometriesChanged,          this, &QTableView::updateGeometries);

   //update the sorting enabled states on the new header
   setSortingEnabled(d->sortingEnabled);
}

void QTableView::setVerticalHeader(QHeaderView *header)
{
   Q_D(QTableView);

   if (!header || header == d->verticalHeader) {
      return;
   }

   if (d->verticalHeader && d->verticalHeader->parent() == this) {
      delete d->verticalHeader;
   }

   d->verticalHeader = header;
   d->verticalHeader->setParent(this);
   d->verticalHeader->d_func()->setAllowUserMoveOfSection0(true);

   if (! d->verticalHeader->model()) {
      d->verticalHeader->setModel(d->model);
      if (d->selectionModel) {
         d->verticalHeader->setSelectionModel(d->selectionModel);
      }
   }

   connect(d->verticalHeader, &QHeaderView::sectionResized,             this, &QTableView::rowResized);
   connect(d->verticalHeader, &QHeaderView::sectionMoved,               this, &QTableView::rowMoved);
   connect(d->verticalHeader, &QHeaderView::sectionCountChanged,        this, &QTableView::rowCountChanged);
   connect(d->verticalHeader, &QHeaderView::sectionPressed,             this, &QTableView::selectRow);
   connect(d->verticalHeader, &QHeaderView::sectionEntered,             this, &QTableView::_q_selectRow);
   connect(d->verticalHeader, &QHeaderView::sectionHandleDoubleClicked, this, &QTableView::resizeRowToContents);
   connect(d->verticalHeader, &QHeaderView::geometriesChanged,          this, &QTableView::updateGeometries);
}

void QTableView::scrollContentsBy(int dx, int dy)
{
   Q_D(QTableView);

   d->delayedAutoScroll.stop(); // auto scroll was canceled by the user scrolling

   dx = isRightToLeft() ? -dx : dx;

   if (dx) {
      int oldOffset = d->horizontalHeader->offset();
      d->horizontalHeader->d_func()->setScrollOffset(horizontalScrollBar(), horizontalScrollMode());
      if (horizontalScrollMode() == QAbstractItemView::ScrollPerItem) {
         int newOffset = d->horizontalHeader->offset();
         dx = isRightToLeft() ? newOffset - oldOffset : oldOffset - newOffset;
      }
   }

   if (dy) {
      int oldOffset = d->verticalHeader->offset();
      d->verticalHeader->d_func()->setScrollOffset(verticalScrollBar(), verticalScrollMode());
      if (verticalScrollMode() == QAbstractItemView::ScrollPerItem) {
         int newOffset = d->verticalHeader->offset();
         dy = oldOffset - newOffset;
      }
   }
   d->scrollContentsBy(dx, dy);

   if (d->showGrid) {
      //we need to update the first line of the previous top item in the view
      //because it has the grid drawn if the header is invisible.
      //It is strictly related to what's done at then end of the paintEvent
      if (dy > 0 && d->horizontalHeader->isHidden() && d->verticalScrollMode == ScrollPerItem) {
         d->viewport->update(0, dy, d->viewport->width(), dy);
      }
      if (dx > 0 && d->verticalHeader->isHidden() && d->horizontalScrollMode == ScrollPerItem) {
         d->viewport->update(dx, 0, dx, d->viewport->height());
      }
   }
}

QStyleOptionViewItem QTableView::viewOptions() const
{
   QStyleOptionViewItem option = QAbstractItemView::viewOptions();
   option.showDecorationSelected = true;
   return option;
}

void QTableView::paintEvent(QPaintEvent *event)
{
   Q_D(QTableView);

   // setup temp variables for the painting
   QStyleOptionViewItem option = d->viewOptions();
   const QPoint offset = d->scrollDelayOffset;
   const bool showGrid = d->showGrid;
   const int gridSize = showGrid ? 1 : 0;
   const int gridHint = style()->styleHint(QStyle::SH_Table_GridLineColor, &option, this);
   const QColor gridColor = static_cast<QRgb>(gridHint);
   const QPen gridPen = QPen(gridColor, 0, d->gridStyle);
   const QHeaderView *verticalHeader = d->verticalHeader;
   const QHeaderView *horizontalHeader = d->horizontalHeader;
   const bool alternate = d->alternatingColors;
   const bool rightToLeft = isRightToLeft();

   QPainter painter(d->viewport);

   // if there's nothing to do, clear the area and return
   if (horizontalHeader->count() == 0 || verticalHeader->count() == 0 || ! d->itemDelegate) {
      return;
   }

   uint x = horizontalHeader->length() - horizontalHeader->offset() - (rightToLeft ? 0 : 1);
   uint y = verticalHeader->length() - verticalHeader->offset() - 1;

   const QRegion region = event->region().translated(offset);
   const QVector<QRect> rects = region.rects();

   //firstVisualRow is the visual index of the first visible row.  lastVisualRow is the visual index of the last visible Row.
   //same goes for ...VisualColumn
   int firstVisualRow = qMax(verticalHeader->visualIndexAt(0), 0);
   int lastVisualRow = verticalHeader->visualIndexAt(verticalHeader->viewport()->height());
   if (lastVisualRow == -1) {
      lastVisualRow = d->model->rowCount(d->root) - 1;
   }

   int firstVisualColumn = horizontalHeader->visualIndexAt(0);
   int lastVisualColumn = horizontalHeader->visualIndexAt(horizontalHeader->viewport()->width());
   if (rightToLeft) {
      qSwap(firstVisualColumn, lastVisualColumn);
   }

   if (firstVisualColumn == -1) {
      firstVisualColumn = 0;
   }

   if (lastVisualColumn == -1) {
      lastVisualColumn = horizontalHeader->count() - 1;
   }

   QBitArray drawn((lastVisualRow - firstVisualRow + 1) * (lastVisualColumn - firstVisualColumn + 1));

   if (d->hasSpans()) {
      d->drawAndClipSpans(region, &painter, option, &drawn,
         firstVisualRow, lastVisualRow, firstVisualColumn, lastVisualColumn);
   }

   for (int i = 0; i < rects.size(); ++i) {
      QRect dirtyArea = rects.at(i);
      dirtyArea.setBottom(qMin(dirtyArea.bottom(), int(y)));
      if (rightToLeft) {
         dirtyArea.setLeft(qMax(dirtyArea.left(), d->viewport->width() - int(x)));
      } else {
         dirtyArea.setRight(qMin(dirtyArea.right(), int(x)));
      }

      // get the horizontal start and end visual sections
      int left = horizontalHeader->visualIndexAt(dirtyArea.left());
      int right = horizontalHeader->visualIndexAt(dirtyArea.right());

      if (rightToLeft) {
         qSwap(left, right);
      }

      if (left == -1) {
         left = 0;
      }

      if (right == -1) {
         right = horizontalHeader->count() - 1;
      }

      // get the vertical start and end visual sections and if alternate color
      int bottom = verticalHeader->visualIndexAt(dirtyArea.bottom());
      if (bottom == -1) {
         bottom = verticalHeader->count() - 1;
      }

      int top = 0;
      bool alternateBase = false;

      if (alternate && verticalHeader->sectionsHidden()) {
         uint verticalOffset = verticalHeader->offset();
         int row = verticalHeader->logicalIndex(top);
         for (int y = 0;
            ((uint)(y += verticalHeader->sectionSize(top)) <= verticalOffset) && (top < bottom);
            ++top) {
            row = verticalHeader->logicalIndex(top);
            if (alternate && !verticalHeader->isSectionHidden(row)) {
               alternateBase = !alternateBase;
            }
         }
      } else {
         top = verticalHeader->visualIndexAt(dirtyArea.top());
         alternateBase = (top & 1) && alternate;
      }
      if (top == -1 || top > bottom) {
         continue;
      }

      // Paint each row item
      for (int visualRowIndex = top; visualRowIndex <= bottom; ++visualRowIndex) {
         int row = verticalHeader->logicalIndex(visualRowIndex);
         if (verticalHeader->isSectionHidden(row)) {
            continue;
         }

         int rowY = rowViewportPosition(row);
         rowY += offset.y();
         int rowh = rowHeight(row) - gridSize;

         // Paint each column item
         for (int visualColumnIndex = left; visualColumnIndex <= right; ++visualColumnIndex) {
            int currentBit = (visualRowIndex - firstVisualRow) * (lastVisualColumn - firstVisualColumn + 1)
               + visualColumnIndex - firstVisualColumn;

            if (currentBit < 0 || currentBit >= drawn.size() || drawn.testBit(currentBit)) {
               continue;
            }
            drawn.setBit(currentBit);

            int col = horizontalHeader->logicalIndex(visualColumnIndex);
            if (horizontalHeader->isSectionHidden(col)) {
               continue;
            }

            int colp = columnViewportPosition(col);
            colp += offset.x();
            int colw = columnWidth(col) - gridSize;

            const QModelIndex index = d->model->index(row, col, d->root);
            if (index.isValid()) {
               option.rect = QRect(colp + (showGrid && rightToLeft ? 1 : 0), rowY, colw, rowh);
               if (alternate) {
                  if (alternateBase) {
                     option.features |= QStyleOptionViewItem::Alternate;
                  } else {
                     option.features &= ~QStyleOptionViewItem::Alternate;
                  }
               }
               d->drawCell(&painter, option, index);
            }
         }
         alternateBase = !alternateBase && alternate;
      }

      if (showGrid) {
         // Find the bottom right (the last rows/columns might be hidden)
         while (verticalHeader->isSectionHidden(verticalHeader->logicalIndex(bottom))) {
            --bottom;
         }

         QPen old = painter.pen();
         painter.setPen(gridPen);

         // Paint each row
         for (int visualIndex = top; visualIndex <= bottom; ++visualIndex) {
            int row = verticalHeader->logicalIndex(visualIndex);
            if (verticalHeader->isSectionHidden(row)) {
               continue;
            }
            int rowY = rowViewportPosition(row);
            rowY += offset.y();
            int rowh = rowHeight(row) - gridSize;
            painter.drawLine(dirtyArea.left(), rowY + rowh, dirtyArea.right(), rowY + rowh);
         }

         // Paint each column
         for (int h = left; h <= right; ++h) {
            int col = horizontalHeader->logicalIndex(h);
            if (horizontalHeader->isSectionHidden(col)) {
               continue;
            }
            int colp = columnViewportPosition(col);
            colp += offset.x();
            if (!rightToLeft) {
               colp +=  columnWidth(col) - gridSize;
            }
            painter.drawLine(colp, dirtyArea.top(), colp, dirtyArea.bottom());
         }

         //draw the top & left grid lines if the headers are not visible.
         //We do update this line when subsequent scroll happen (see scrollContentsBy)
         if (horizontalHeader->isHidden() && verticalScrollMode() == ScrollPerItem) {
            painter.drawLine(dirtyArea.left(), 0, dirtyArea.right(), 0);
         }
         if (verticalHeader->isHidden() && horizontalScrollMode() == ScrollPerItem) {
            painter.drawLine(0, dirtyArea.top(), 0, dirtyArea.bottom());
         }
         painter.setPen(old);
      }
   }

#ifndef QT_NO_DRAGANDDROP
   // Paint the dropIndicator
   d->paintDropIndicator(&painter);
#endif
}

QModelIndex QTableView::indexAt(const QPoint &pos) const
{
   Q_D(const QTableView);
   d->executePostedLayout();
   int r = rowAt(pos.y());
   int c = columnAt(pos.x());
   if (r >= 0 && c >= 0) {
      if (d->hasSpans()) {
         QSpanCollection::Span span = d->span(r, c);
         r = span.top();
         c = span.left();
      }
      return d->model->index(r, c, d->root);
   }
   return QModelIndex();
}

int QTableView::horizontalOffset() const
{
   Q_D(const QTableView);
   return d->horizontalHeader->offset();
}

int QTableView::verticalOffset() const
{
   Q_D(const QTableView);
   return d->verticalHeader->offset();
}

QModelIndex QTableView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
   Q_D(QTableView);
   (void) modifiers;

   int bottom = d->model->rowCount(d->root) - 1;

   // make sure that bottom is the bottommost *visible* row
   while (bottom >= 0 && isRowHidden(d->logicalRow(bottom))) {
      --bottom;
   }

   int right = d->model->columnCount(d->root) - 1;

   while (right >= 0 && isColumnHidden(d->logicalColumn(right))) {
      --right;
   }

   if (bottom == -1 || right == -1) {
      return QModelIndex();   // model is empty
   }

   QModelIndex current = currentIndex();

   if (!current.isValid()) {
      int row = 0;
      int column = 0;
      while (column < right && isColumnHidden(d->logicalColumn(column))) {
         ++column;
      }
      while (isRowHidden(d->logicalRow(row)) && row < bottom) {
         ++row;
      }
      d->visualCursor = QPoint(column, row);
      return d->model->index(d->logicalRow(row), d->logicalColumn(column), d->root);
   }

   // Update visual cursor if current index has changed.
   QPoint visualCurrent(d->visualColumn(current.column()), d->visualRow(current.row()));
   if (visualCurrent != d->visualCursor) {
      if (d->hasSpans()) {
         QSpanCollection::Span span = d->span(current.row(), current.column());
         if (span.top() > d->visualCursor.y() || d->visualCursor.y() > span.bottom()
            || span.left() > d->visualCursor.x() || d->visualCursor.x() > span.right()) {
            d->visualCursor = visualCurrent;
         }
      } else {
         d->visualCursor = visualCurrent;
      }
   }

   int visualRow = d->visualCursor.y();
   if (visualRow > bottom) {
      visualRow = bottom;
   }
   Q_ASSERT(visualRow != -1);
   int visualColumn = d->visualCursor.x();
   if (visualColumn > right) {
      visualColumn = right;
   }
   Q_ASSERT(visualColumn != -1);

   if (isRightToLeft()) {
      if (cursorAction == MoveLeft) {
         cursorAction = MoveRight;
      } else if (cursorAction == MoveRight) {
         cursorAction = MoveLeft;
      }
   }

   switch (cursorAction) {
      case MoveUp: {
         int originalRow = visualRow;
#ifdef QT_KEYPAD_NAVIGATION
         if (QApplication::keypadNavigationEnabled() && visualRow == 0) {
            visualRow = d->visualRow(model()->rowCount() - 1) + 1;
         }
         // FIXME? visualRow = bottom + 1;
#endif
         int r = d->logicalRow(visualRow);
         int c = d->logicalColumn(visualColumn);
         if (r != -1 && d->hasSpans()) {
            QSpanCollection::Span span = d->span(r, c);
            if (span.width() > 1 || span.height() > 1) {
               visualRow = d->visualRow(span.top());
            }
         }
         while (visualRow >= 0) {
            --visualRow;
            r = d->logicalRow(visualRow);
            c = d->logicalColumn(visualColumn);
            if (r == -1 || (!isRowHidden(r) && d->isCellEnabled(r, c))) {
               break;
            }
         }
         if (visualRow < 0) {
            visualRow = originalRow;
         }
         break;
      }
      case MoveDown: {
         int originalRow = visualRow;
         if (d->hasSpans()) {
            QSpanCollection::Span span = d->span(current.row(), current.column());
            visualRow = d->visualRow(d->rowSpanEndLogical(span.top(), span.height()));
         }
#ifdef QT_KEYPAD_NAVIGATION
         if (QApplication::keypadNavigationEnabled() && visualRow >= bottom) {
            visualRow = -1;
         }
#endif
         int r = d->logicalRow(visualRow);
         int c = d->logicalColumn(visualColumn);
         if (r != -1 && d->hasSpans()) {
            QSpanCollection::Span span = d->span(r, c);
            if (span.width() > 1 || span.height() > 1) {
               visualRow = d->visualRow(d->rowSpanEndLogical(span.top(), span.height()));
            }
         }
         while (visualRow <= bottom) {
            ++visualRow;
            r = d->logicalRow(visualRow);
            c = d->logicalColumn(visualColumn);
            if (r == -1 || (!isRowHidden(r) && d->isCellEnabled(r, c))) {
               break;
            }
         }
         if (visualRow > bottom) {
            visualRow = originalRow;
         }
         break;
      }
      case MovePrevious:
      case MoveLeft: {
         int originalRow = visualRow;
         int originalColumn = visualColumn;
         bool firstTime = true;
         bool looped = false;
         bool wrapped = false;
         do {
            int r = d->logicalRow(visualRow);
            int c = d->logicalColumn(visualColumn);
            if (firstTime && c != -1 && d->hasSpans()) {
               firstTime = false;
               QSpanCollection::Span span = d->span(r, c);
               if (span.width() > 1 || span.height() > 1) {
                  visualColumn = d->visualColumn(span.left());
               }
            }
            while (visualColumn >= 0) {
               --visualColumn;
               r = d->logicalRow(visualRow);
               c = d->logicalColumn(visualColumn);
               if (r == -1 || c == -1 || (!isRowHidden(r) && !isColumnHidden(c) && d->isCellEnabled(r, c))) {
                  break;
               }
               if (wrapped && (originalRow < visualRow || (originalRow == visualRow && originalColumn <= visualColumn))) {
                  looped = true;
                  break;
               }
            }
            if (cursorAction == MoveLeft || visualColumn >= 0) {
               break;
            }
            visualColumn = right + 1;
            if (visualRow == 0) {
               wrapped = true;
               visualRow = bottom;
            } else {
               --visualRow;
            }
         } while (!looped);
         if (visualColumn < 0) {
            visualColumn = originalColumn;
         }
         break;
      }
      case MoveNext:
      case MoveRight: {
         int originalRow = visualRow;
         int originalColumn = visualColumn;
         bool firstTime = true;
         bool looped = false;
         bool wrapped = false;
         do {
            int r = d->logicalRow(visualRow);
            int c = d->logicalColumn(visualColumn);
            if (firstTime && c != -1 && d->hasSpans()) {
               firstTime = false;
               QSpanCollection::Span span = d->span(r, c);
               if (span.width() > 1 || span.height() > 1) {
                  visualColumn = d->visualColumn(d->columnSpanEndLogical(span.left(), span.width()));
               }
            }
            while (visualColumn <= right) {
               ++visualColumn;
               r = d->logicalRow(visualRow);
               c = d->logicalColumn(visualColumn);
               if (r == -1 || c == -1 || (!isRowHidden(r) && !isColumnHidden(c) && d->isCellEnabled(r, c))) {
                  break;
               }
               if (wrapped && (originalRow > visualRow || (originalRow == visualRow && originalColumn >= visualColumn))) {
                  looped = true;
                  break;
               }
            }
            if (cursorAction == MoveRight || visualColumn <= right) {
               break;
            }
            visualColumn = -1;
            if (visualRow == bottom) {
               wrapped = true;
               visualRow = 0;
            } else {
               ++visualRow;
            }
         } while (!looped);
         if (visualColumn > right) {
            visualColumn = originalColumn;
         }
         break;
      }
      case MoveHome:
         visualColumn = 0;
         while (visualColumn < right && d->isVisualColumnHiddenOrDisabled(visualRow, visualColumn)) {
            ++visualColumn;
         }
         if (modifiers & Qt::ControlModifier) {
            visualRow = 0;
            while (visualRow < bottom && d->isVisualRowHiddenOrDisabled(visualRow, visualColumn)) {
               ++visualRow;
            }
         }
         break;
      case MoveEnd:
         visualColumn = right;
         if (modifiers & Qt::ControlModifier) {
            visualRow = bottom;
         }
         break;
      case MovePageUp: {
         int newRow = rowAt(visualRect(current).bottom() - d->viewport->height());
         if (newRow == -1) {
            newRow = d->logicalRow(0);
         }
         return d->model->index(newRow, current.column(), d->root);
      }
      case MovePageDown: {
         int newRow = rowAt(visualRect(current).top() + d->viewport->height());
         if (newRow == -1) {
            newRow = d->logicalRow(bottom);
         }
         return d->model->index(newRow, current.column(), d->root);
      }
   }

   d->visualCursor = QPoint(visualColumn, visualRow);
   int logicalRow = d->logicalRow(visualRow);
   int logicalColumn = d->logicalColumn(visualColumn);
   if (!d->model->hasIndex(logicalRow, logicalColumn, d->root)) {
      return QModelIndex();
   }

   QModelIndex result = d->model->index(logicalRow, logicalColumn, d->root);
   if (!d->isRowHidden(logicalRow) && !d->isColumnHidden(logicalColumn) && d->isIndexEnabled(result)) {
      if (d->hasSpans()) {
         QSpanCollection::Span span = d->span(result.row(), result.column());
         if (span.width() > 1 || span.height() > 1) {
            result = d->model->sibling(span.top(), span.left(), result);
         }
      }
      return result;
   }

   return QModelIndex();
}

void QTableView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
   Q_D(QTableView);
   QModelIndex tl = indexAt(QPoint(isRightToLeft() ? qMax(rect.left(), rect.right())
            : qMin(rect.left(), rect.right()), qMin(rect.top(), rect.bottom())));
   QModelIndex br = indexAt(QPoint(isRightToLeft() ? qMin(rect.left(), rect.right()) :
            qMax(rect.left(), rect.right()), qMax(rect.top(), rect.bottom())));
   if (!d->selectionModel || !tl.isValid() || !br.isValid() || !d->isIndexEnabled(tl) || !d->isIndexEnabled(br)) {
      return;
   }

   bool verticalMoved = verticalHeader()->sectionsMoved();
   bool horizontalMoved = horizontalHeader()->sectionsMoved();

   QItemSelection selection;

   if (d->hasSpans()) {
      bool expanded;
      int top = qMin(d->visualRow(tl.row()), d->visualRow(br.row()));
      int left = qMin(d->visualColumn(tl.column()), d->visualColumn(br.column()));
      int bottom = qMax(d->visualRow(tl.row()), d->visualRow(br.row()));
      int right = qMax(d->visualColumn(tl.column()), d->visualColumn(br.column()));

      do {
         expanded = false;

         for (QSpanCollection::Span *it : d->spans.spans) {
            const QSpanCollection::Span &span = *it;

            int t = d->visualRow(span.top());
            int l = d->visualColumn(span.left());
            int b = d->visualRow(d->rowSpanEndLogical(span.top(), span.height()));
            int r = d->visualColumn(d->columnSpanEndLogical(span.left(), span.width()));
            if ((t > bottom) || (l > right) || (top > b) || (left > r)) {
               continue;   // no intersect
            }

            if (t < top) {
               top = t;
               expanded = true;
            }
            if (l < left) {
               left = l;
               expanded = true;
            }
            if (b > bottom) {
               bottom = b;
               expanded = true;
            }
            if (r > right) {
               right = r;
               expanded = true;
            }
            if (expanded) {
               break;
            }
         }

      } while (expanded);

      for (int horizontal = left; horizontal <= right; ++horizontal) {
         int column = d->logicalColumn(horizontal);
         for (int vertical = top; vertical <= bottom; ++vertical) {
            int row = d->logicalRow(vertical);
            QModelIndex index = d->model->index(row, column, d->root);
            selection.append(QItemSelectionRange(index));
         }
      }
   } else if (verticalMoved && horizontalMoved) {
      int top = d->visualRow(tl.row());
      int left = d->visualColumn(tl.column());
      int bottom = d->visualRow(br.row());
      int right = d->visualColumn(br.column());

      for (int horizontal = left; horizontal <= right; ++horizontal) {
         int column = d->logicalColumn(horizontal);
         for (int vertical = top; vertical <= bottom; ++vertical) {
            int row = d->logicalRow(vertical);
            QModelIndex index = d->model->index(row, column, d->root);
            selection.append(QItemSelectionRange(index));
         }
      }
   } else if (horizontalMoved) {
      int left = d->visualColumn(tl.column());
      int right = d->visualColumn(br.column());

      for (int visual = left; visual <= right; ++visual) {
         int column = d->logicalColumn(visual);
         QModelIndex topLeft = d->model->index(tl.row(), column, d->root);
         QModelIndex bottomRight = d->model->index(br.row(), column, d->root);
         selection.append(QItemSelectionRange(topLeft, bottomRight));
      }

   } else if (verticalMoved) {
      int top = d->visualRow(tl.row());
      int bottom = d->visualRow(br.row());

      for (int visual = top; visual <= bottom; ++visual) {
         int row = d->logicalRow(visual);
         QModelIndex topLeft = d->model->index(row, tl.column(), d->root);
         QModelIndex bottomRight = d->model->index(row, br.column(), d->root);
         selection.append(QItemSelectionRange(topLeft, bottomRight));
      }

   } else { // nothing moved
      QItemSelectionRange range(tl, br);

      if (!range.isEmpty()) {
         selection.append(range);
      }
   }

   d->selectionModel->select(selection, command);
}

QRegion QTableView::visualRegionForSelection(const QItemSelection &selection) const
{
   Q_D(const QTableView);

   if (selection.isEmpty()) {
      return QRegion();
   }

   QRegion selectionRegion;
   const QRect &viewportRect = d->viewport->rect();
   bool verticalMoved = verticalHeader()->sectionsMoved();
   bool horizontalMoved = horizontalHeader()->sectionsMoved();

   if ((verticalMoved && horizontalMoved) || (d->hasSpans() && (verticalMoved || horizontalMoved))) {
      for (int i = 0; i < selection.count(); ++i) {
         QItemSelectionRange range = selection.at(i);
         if (range.parent() != d->root || !range.isValid()) {
            continue;
         }
         for (int r = range.top(); r <= range.bottom(); ++r)
            for (int c = range.left(); c <= range.right(); ++c) {
               const QRect &rangeRect = visualRect(d->model->index(r, c, d->root));
               if (viewportRect.intersects(rangeRect)) {
                  selectionRegion += rangeRect;
               }
            }
      }
   } else if (horizontalMoved) {
      for (int i = 0; i < selection.count(); ++i) {
         QItemSelectionRange range = selection.at(i);
         if (range.parent() != d->root || !range.isValid()) {
            continue;
         }
         int top = rowViewportPosition(range.top());
         int bottom = rowViewportPosition(range.bottom()) + rowHeight(range.bottom());
         if (top > bottom) {
            qSwap<int>(top, bottom);
         }
         int height = bottom - top;
         for (int c = range.left(); c <= range.right(); ++c) {
            const QRect rangeRect(columnViewportPosition(c), top, columnWidth(c), height);
            if (viewportRect.intersects(rangeRect)) {
               selectionRegion += rangeRect;
            }
         }
      }

   } else if (verticalMoved) {
      for (int i = 0; i < selection.count(); ++i) {
         QItemSelectionRange range = selection.at(i);
         if (range.parent() != d->root || !range.isValid()) {
            continue;
         }
         int left = columnViewportPosition(range.left());
         int right = columnViewportPosition(range.right()) + columnWidth(range.right());
         if (left > right) {
            qSwap<int>(left, right);
         }
         int width = right - left;
         for (int r = range.top(); r <= range.bottom(); ++r) {
            const QRect rangeRect(left, rowViewportPosition(r), width, rowHeight(r));
            if (viewportRect.intersects(rangeRect)) {
               selectionRegion += rangeRect;
            }
         }
      }

   } else { // nothing moved
      const int gridAdjust = showGrid() ? 1 : 0;

      for (int i = 0; i < selection.count(); ++i) {
         QItemSelectionRange range = selection.at(i);
         if (range.parent() != d->root || !range.isValid()) {
            continue;
         }
         d->trimHiddenSelections(&range);

         const int rtop = rowViewportPosition(range.top());
         const int rbottom = rowViewportPosition(range.bottom()) + rowHeight(range.bottom());
         int rleft;
         int rright;

         if (isLeftToRight()) {
            rleft = columnViewportPosition(range.left());
            rright = columnViewportPosition(range.right()) + columnWidth(range.right());
         } else {
            rleft = columnViewportPosition(range.right());
            rright = columnViewportPosition(range.left()) + columnWidth(range.left());
         }

         const QRect rangeRect(QPoint(rleft, rtop), QPoint(rright - 1 - gridAdjust, rbottom - 1 - gridAdjust));
         if (viewportRect.intersects(rangeRect)) {
            selectionRegion += rangeRect;
         }

         if (d->hasSpans()) {
            for (QSpanCollection::Span *s :
               d->spans.spansInRect(range.left(), range.top(), range.width(), range.height())) {

               if (range.contains(s->top(), s->left(), range.parent())) {
                  const QRect &visualSpanRect = d->visualSpanRect(*s);
                  if (viewportRect.intersects(visualSpanRect)) {
                     selectionRegion += visualSpanRect;
                  }
               }
            }
         }
      }
   }

   return selectionRegion;
}

QModelIndexList QTableView::selectedIndexes() const
{
   Q_D(const QTableView);
   QModelIndexList viewSelected;
   QModelIndexList modelSelected;

   if (d->selectionModel) {
      modelSelected = d->selectionModel->selectedIndexes();
   }

   for (int i = 0; i < modelSelected.count(); ++i) {
      QModelIndex index = modelSelected.at(i);

      if (!isIndexHidden(index) && index.parent() == d->root) {
         viewSelected.append(index);
      }
   }

   return viewSelected;
}

void QTableView::rowCountChanged(int oldCount, int newCount )
{
   Q_D(QTableView);

   // when removing rows, we need to disable updates for the header until the geometries have been
   // updated and the offset has been adjusted, or we risk calling paintSection for all the sections
   if (newCount < oldCount) {
      d->verticalHeader->setUpdatesEnabled(false);
   }

   d->doDelayedItemsLayout();
}

void QTableView::columnCountChanged(int, int)
{
   Q_D(QTableView);

   updateGeometries();
   if (horizontalScrollMode() == QAbstractItemView::ScrollPerItem) {
      d->horizontalHeader->setOffsetToSectionPosition(horizontalScrollBar()->value());
   } else {
      d->horizontalHeader->setOffset(horizontalScrollBar()->value());
   }
   d->viewport->update();
}

void QTableView::updateGeometries()
{
   Q_D(QTableView);

   if (d->geometryRecursionBlock) {
      return;
   }
   d->geometryRecursionBlock = true;

   int width = 0;
   if (!d->verticalHeader->isHidden()) {
      width = qMax(d->verticalHeader->minimumWidth(), d->verticalHeader->sizeHint().width());
      width = qMin(width, d->verticalHeader->maximumWidth());
   }
   int height = 0;
   if (!d->horizontalHeader->isHidden()) {
      height = qMax(d->horizontalHeader->minimumHeight(), d->horizontalHeader->sizeHint().height());
      height = qMin(height, d->horizontalHeader->maximumHeight());
   }
   bool reverse = isRightToLeft();
   if (reverse) {
      setViewportMargins(0, height, width, 0);
   } else {
      setViewportMargins(width, height, 0, 0);
   }

   // update headers

   QRect vg = d->viewport->geometry();

   int verticalLeft = reverse ? vg.right() + 1 : (vg.left() - width);
   d->verticalHeader->setGeometry(verticalLeft, vg.top(), width, vg.height());
   if (d->verticalHeader->isHidden()) {
      QMetaObject::invokeMethod(d->verticalHeader, "updateGeometries");
   }

   int horizontalTop = vg.top() - height;
   d->horizontalHeader->setGeometry(vg.left(), horizontalTop, vg.width(), height);
   if (d->horizontalHeader->isHidden()) {
      QMetaObject::invokeMethod(d->horizontalHeader, "updateGeometries");
   }

   // update cornerWidget
   if (d->horizontalHeader->isHidden() || d->verticalHeader->isHidden()) {
      d->cornerWidget->setHidden(true);
   } else {
      d->cornerWidget->setHidden(false);
      d->cornerWidget->setGeometry(verticalLeft, horizontalTop, width, height);
   }

   // update scroll bars

   // ### move this block into the if
   QSize vsize = d->viewport->size();
   QSize max = maximumViewportSize();
   uint horizontalLength = d->horizontalHeader->length();
   uint verticalLength = d->verticalHeader->length();
   if ((uint)max.width() >= horizontalLength && (uint)max.height() >= verticalLength) {
      vsize = max;
   }

   // horizontal scroll bar
   const int columnCount = d->horizontalHeader->count();
   const int viewportWidth = vsize.width();
   int columnsInViewport = 0;
   for (int width = 0, column = columnCount - 1; column >= 0; --column) {
      int logical = d->horizontalHeader->logicalIndex(column);
      if (!d->horizontalHeader->isSectionHidden(logical)) {
         width += d->horizontalHeader->sectionSize(logical);
         if (width > viewportWidth) {
            break;
         }
         ++columnsInViewport;
      }
   }
   columnsInViewport = qMax(columnsInViewport, 1); //there must be always at least 1 column

   if (horizontalScrollMode() == QAbstractItemView::ScrollPerItem) {
      const int visibleColumns = columnCount - d->horizontalHeader->hiddenSectionCount();
      horizontalScrollBar()->setRange(0, visibleColumns - columnsInViewport);
      horizontalScrollBar()->setPageStep(columnsInViewport);
      if (columnsInViewport >= visibleColumns) {
         d->horizontalHeader->setOffset(0);
      }
      horizontalScrollBar()->setSingleStep(1);
   } else { // ScrollPerPixel
      horizontalScrollBar()->setPageStep(vsize.width());
      horizontalScrollBar()->setRange(0, horizontalLength - vsize.width());
      horizontalScrollBar()->setSingleStep(qMax(vsize.width() / (columnsInViewport + 1), 2));
   }

   // vertical scroll bar
   const int rowCount = d->verticalHeader->count();
   const int viewportHeight = vsize.height();
   int rowsInViewport = 0;
   for (int height = 0, row = rowCount - 1; row >= 0; --row) {
      int logical = d->verticalHeader->logicalIndex(row);
      if (!d->verticalHeader->isSectionHidden(logical)) {
         height += d->verticalHeader->sectionSize(logical);
         if (height > viewportHeight) {
            break;
         }
         ++rowsInViewport;
      }
   }
   rowsInViewport = qMax(rowsInViewport, 1); //there must be always at least 1 row

   if (verticalScrollMode() == QAbstractItemView::ScrollPerItem) {
      const int visibleRows = rowCount - d->verticalHeader->hiddenSectionCount();
      verticalScrollBar()->setRange(0, visibleRows - rowsInViewport);
      verticalScrollBar()->setPageStep(rowsInViewport);
      if (rowsInViewport >= visibleRows) {
         d->verticalHeader->setOffset(0);
      }
      verticalScrollBar()->setSingleStep(1);
   } else { // ScrollPerPixel
      verticalScrollBar()->setPageStep(vsize.height());
      verticalScrollBar()->setRange(0, verticalLength - vsize.height());
      verticalScrollBar()->setSingleStep(qMax(vsize.height() / (rowsInViewport + 1), 2));
   }

   d->geometryRecursionBlock = false;
   QAbstractItemView::updateGeometries();
}

int QTableView::sizeHintForRow(int row) const
{
   Q_D(const QTableView);

   if (! model()) {
      return -1;
   }

   ensurePolished();
   const int maximumProcessCols = d->verticalHeader->resizeContentsPrecision();

   int left  = qMax(0, d->horizontalHeader->visualIndexAt(0));
   int right = d->horizontalHeader->visualIndexAt(d->viewport->width());

   if (right == -1) {
      // not have enough columns to fill the viewport
      right = d->model->columnCount(d->root) - 1;
   }

   QStyleOptionViewItem option = d->viewOptions();

   int hint = 0;
   QModelIndex index;

   int columnsProcessed = 0;
   int column = left;

   for (; column <= right; ++column) {
      int logicalColumn = d->horizontalHeader->logicalIndex(column);
      if (d->horizontalHeader->isSectionHidden(logicalColumn)) {
         continue;
      }

      index = d->model->index(row, logicalColumn, d->root);
      hint  = d->heightHintForIndex(index, hint, option);

      ++columnsProcessed;

      if (columnsProcessed == maximumProcessCols) {
         break;
      }
   }

   int actualRight = d->model->columnCount(d->root) - 1;
   int idxLeft     = left;
   int idxRight    = column - 1;

   if (maximumProcessCols == 0) {
      columnsProcessed = 0;   // skip the while loop
   }

   while (columnsProcessed != maximumProcessCols && (idxLeft > 0 || idxRight < actualRight)) {
      int logicalIdx  = -1;

      if ((columnsProcessed % 2 && idxLeft > 0) || idxRight == actualRight) {
         while (idxLeft > 0) {
            --idxLeft;
            int logcol = d->horizontalHeader->logicalIndex(idxLeft);

            if (d->horizontalHeader->isSectionHidden(logcol)) {
               continue;
            }

            logicalIdx = logcol;
            break;
         }

      } else {
         while (idxRight < actualRight) {
            ++idxRight;
            int logcol = d->horizontalHeader->logicalIndex(idxRight);

            if (d->horizontalHeader->isSectionHidden(logcol)) {
               continue;
            }

            logicalIdx = logcol;
            break;
         }
      }

      if (logicalIdx < 0) {
         continue;
      }

      index = d->model->index(row, logicalIdx, d->root);
      hint  = d->heightHintForIndex(index, hint, option);
      ++columnsProcessed;
   }

   return d->showGrid ? hint + 1 : hint;
}

int QTableView::sizeHintForColumn(int column) const
{
   Q_D(const QTableView);

   if (! model()) {
      return -1;
   }

   ensurePolished();
   const int maximumProcessRows = d->horizontalHeader->resizeContentsPrecision();

   int top = qMax(0, d->verticalHeader->visualIndexAt(0));
   int bottom = d->verticalHeader->visualIndexAt(d->viewport->height());
   if (!isVisible() || bottom == -1) {
      // the table does not have enough rows to fill the viewport
      bottom = d->model->rowCount(d->root) - 1;
   }

   QStyleOptionViewItem option = d->viewOptions();

   int hint = 0;
   int rowsProcessed = 0;

   QModelIndex index;
   int row = top;

   for (; row <= bottom; ++row) {
      int logicalRow = d->verticalHeader->logicalIndex(row);
      if (d->verticalHeader->isSectionHidden(logicalRow)) {
         continue;
      }
      index = d->model->index(logicalRow, column, d->root);

      hint = d->widthHintForIndex(index, hint, option);
      ++rowsProcessed;
      if (rowsProcessed == maximumProcessRows) {
         break;
      }
   }

   int actualBottom = d->model->rowCount(d->root) - 1;
   int idxTop       = top;
   int idxBottom    = row - 1;

   if (maximumProcessRows == 0) {
      rowsProcessed = 0;   // skip the while loop
   }

   while (rowsProcessed != maximumProcessRows && (idxTop > 0 || idxBottom < actualBottom)) {
      int logicalIdx  = -1;

      if ((rowsProcessed % 2 && idxTop > 0) || idxBottom == actualBottom) {
         while (idxTop > 0) {
            --idxTop;
            int logrow = d->verticalHeader->logicalIndex(idxTop);
            if (d->verticalHeader->isSectionHidden(logrow)) {
               continue;
            }
            logicalIdx = logrow;
            break;
         }
      } else {
         while (idxBottom < actualBottom) {
            ++idxBottom;
            int logrow = d->verticalHeader->logicalIndex(idxBottom);
            if (d->verticalHeader->isSectionHidden(logrow)) {
               continue;
            }
            logicalIdx = logrow;
            break;
         }
      }
      if (logicalIdx < 0) {
         continue;
      }

      index = d->model->index(logicalIdx, column, d->root);
      hint = d->widthHintForIndex(index, hint, option);
      ++rowsProcessed;
   }
   return d->showGrid ? hint + 1 : hint;
}

int QTableView::rowViewportPosition(int row) const
{
   Q_D(const QTableView);
   return d->verticalHeader->sectionViewportPosition(row);
}

int QTableView::rowAt(int y) const
{
   Q_D(const QTableView);
   return d->verticalHeader->logicalIndexAt(y);
}

void QTableView::setRowHeight(int row, int height)
{
   Q_D(const QTableView);
   d->verticalHeader->resizeSection(row, height);
}

int QTableView::rowHeight(int row) const
{
   Q_D(const QTableView);
   return d->verticalHeader->sectionSize(row);
}

int QTableView::columnViewportPosition(int column) const
{
   Q_D(const QTableView);
   return d->horizontalHeader->sectionViewportPosition(column);
}

int QTableView::columnAt(int x) const
{
   Q_D(const QTableView);
   return d->horizontalHeader->logicalIndexAt(x);
}

void QTableView::setColumnWidth(int column, int width)
{
   Q_D(const QTableView);
   d->horizontalHeader->resizeSection(column, width);
}

int QTableView::columnWidth(int column) const
{
   Q_D(const QTableView);
   return d->horizontalHeader->sectionSize(column);
}

bool QTableView::isRowHidden(int row) const
{
   Q_D(const QTableView);
   return d->verticalHeader->isSectionHidden(row);
}

void QTableView::setRowHidden(int row, bool hide)
{
   Q_D(QTableView);

   if (row < 0 || row >= d->verticalHeader->count()) {
      return;
   }

   d->verticalHeader->setSectionHidden(row, hide);
}

bool QTableView::isColumnHidden(int column) const
{
   Q_D(const QTableView);
   return d->horizontalHeader->isSectionHidden(column);
}

void QTableView::setColumnHidden(int column, bool hide)
{
   Q_D(QTableView);

   if (column < 0 || column >= d->horizontalHeader->count()) {
      return;
   }

   d->horizontalHeader->setSectionHidden(column, hide);
}

void QTableView::setSortingEnabled(bool enable)
{
   Q_D(QTableView);

   d->sortingEnabled = enable;
   horizontalHeader()->setSortIndicatorShown(enable);

   if (enable) {
      disconnect(d->horizontalHeader, &QHeaderView::sectionEntered,       this, &QTableView::_q_selectColumn);
      disconnect(horizontalHeader(),  &QHeaderView::sectionPressed,       this, &QTableView::selectColumn);

      connect(horizontalHeader(),     &QHeaderView::sortIndicatorChanged,
            this, cs_mp_cast<int>(&QTableView::sortByColumn), Qt::UniqueConnection);

      sortByColumn(horizontalHeader()->sortIndicatorSection(), horizontalHeader()->sortIndicatorOrder());

   } else {
      connect(d->horizontalHeader,   &QHeaderView::sectionEntered,
            this, &QTableView::_q_selectColumn, Qt::UniqueConnection);

      connect(horizontalHeader(),    &QHeaderView::sectionPressed,
            this, &QTableView::selectColumn, Qt::UniqueConnection);

      disconnect(horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, cs_mp_cast<int>(&QTableView::sortByColumn));
   }
}

bool QTableView::isSortingEnabled() const
{
   Q_D(const QTableView);
   return d->sortingEnabled;
}

bool QTableView::showGrid() const
{
   Q_D(const QTableView);
   return d->showGrid;
}

void QTableView::setShowGrid(bool show)
{
   Q_D(QTableView);
   if (d->showGrid != show) {
      d->showGrid = show;
      d->viewport->update();
   }
}

Qt::PenStyle QTableView::gridStyle() const
{
   Q_D(const QTableView);
   return d->gridStyle;
}

void QTableView::setGridStyle(Qt::PenStyle style)
{
   Q_D(QTableView);
   if (d->gridStyle != style) {
      d->gridStyle = style;
      d->viewport->update();
   }
}

void QTableView::setWordWrap(bool on)
{
   Q_D(QTableView);
   if (d->wrapItemText == on) {
      return;
   }

   d->wrapItemText = on;
   QMetaObject::invokeMethod(d->verticalHeader, "resizeSections");
   QMetaObject::invokeMethod(d->horizontalHeader, "resizeSections");
}

bool QTableView::wordWrap() const
{
   Q_D(const QTableView);
   return d->wrapItemText;
}

void QTableView::setCornerButtonEnabled(bool enable)
{
   Q_D(QTableView);
   d->cornerWidget->setEnabled(enable);
}

bool QTableView::isCornerButtonEnabled() const
{
   Q_D(const QTableView);
   return d->cornerWidget->isEnabled();
}

QRect QTableView::visualRect(const QModelIndex &index) const
{
   Q_D(const QTableView);

   if (! d->isIndexValid(index) || index.parent() != d->root || (! d->hasSpans() && isIndexHidden(index))) {
      return QRect();
   }

   d->executePostedLayout();

   if (d->hasSpans()) {
      QSpanCollection::Span span = d->span(index.row(), index.column());
      return d->visualSpanRect(span);
   }

   int rowp = rowViewportPosition(index.row());
   int rowh = rowHeight(index.row());
   int colp = columnViewportPosition(index.column());
   int colw = columnWidth(index.column());

   const int i = showGrid() ? 1 : 0;
   return QRect(colp, rowp, colw - i, rowh - i);
}

void QTableView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
   Q_D(QTableView);

   // check if we really need to do anything
   if (! d->isIndexValid(index) || (d->model->parent(index) != d->root) || isRowHidden(index.row())
         || isColumnHidden(index.column())) {
      return;
   }

   QSpanCollection::Span span;
   if (d->hasSpans()) {
      span = d->span(index.row(), index.column());
   }

   // Adjust horizontal position

   int viewportWidth = d->viewport->width();
   int horizontalOffset = d->horizontalHeader->offset();
   int horizontalPosition = d->horizontalHeader->sectionPosition(index.column());
   int horizontalIndex = d->horizontalHeader->visualIndex(index.column());

   int cellWidth = d->hasSpans()
      ? d->columnSpanWidth(index.column(), span.width())
      : d->horizontalHeader->sectionSize(index.column());

   if (horizontalScrollMode() == QAbstractItemView::ScrollPerItem) {

      bool positionAtLeft = (horizontalPosition - horizontalOffset < 0);
      bool positionAtRight = (horizontalPosition - horizontalOffset + cellWidth > viewportWidth);

      if (hint == PositionAtCenter || positionAtRight) {
         int w = (hint == PositionAtCenter ? viewportWidth / 2 : viewportWidth);
         int x = cellWidth;
         while (horizontalIndex > 0) {
            x += columnWidth(d->horizontalHeader->logicalIndex(horizontalIndex - 1));
            if (x > w) {
               break;
            }
            --horizontalIndex;
         }
      }

      if (positionAtRight || hint == PositionAtCenter || positionAtLeft) {
         int hiddenSections = 0;
         if (d->horizontalHeader->sectionsHidden()) {
            for (int s = horizontalIndex - 1; s >= 0; --s) {
               int column = d->horizontalHeader->logicalIndex(s);
               if (d->horizontalHeader->isSectionHidden(column)) {
                  ++hiddenSections;
               }
            }
         }
         horizontalScrollBar()->setValue(horizontalIndex - hiddenSections);
      }

   } else { // ScrollPerPixel
      if (hint == PositionAtCenter) {
         horizontalScrollBar()->setValue(horizontalPosition - ((viewportWidth - cellWidth) / 2));
      } else {
         if (horizontalPosition - horizontalOffset < 0 || cellWidth > viewportWidth) {
            horizontalScrollBar()->setValue(horizontalPosition);
         } else if (horizontalPosition - horizontalOffset + cellWidth > viewportWidth) {
            horizontalScrollBar()->setValue(horizontalPosition - viewportWidth + cellWidth);
         }
      }
   }

   // Adjust vertical position

   int viewportHeight = d->viewport->height();
   int verticalOffset = d->verticalHeader->offset();
   int verticalPosition = d->verticalHeader->sectionPosition(index.row());
   int verticalIndex = d->verticalHeader->visualIndex(index.row());
   int cellHeight = d->hasSpans()
      ? d->rowSpanHeight(index.row(), span.height())
      : d->verticalHeader->sectionSize(index.row());

   if (verticalPosition - verticalOffset < 0 || cellHeight > viewportHeight) {
      if (hint == EnsureVisible) {
         hint = PositionAtTop;
      }

   } else if (verticalPosition - verticalOffset + cellHeight > viewportHeight) {
      if (hint == EnsureVisible) {
         hint = PositionAtBottom;
      }
   }

   if (verticalScrollMode() == QAbstractItemView::ScrollPerItem) {

      if (hint == PositionAtBottom || hint == PositionAtCenter) {
         int h = (hint == PositionAtCenter ? viewportHeight / 2 : viewportHeight);
         int y = cellHeight;
         while (verticalIndex > 0) {
            int row = d->verticalHeader->logicalIndex(verticalIndex - 1);
            y += d->verticalHeader->sectionSize(row);

            if (y > h) {
               break;
            }
            --verticalIndex;
         }
      }

      if (hint == PositionAtBottom || hint == PositionAtCenter || hint == PositionAtTop) {
         int hiddenSections = 0;
         if (d->verticalHeader->sectionsHidden()) {
            for (int s = verticalIndex - 1; s >= 0; --s) {
               int row = d->verticalHeader->logicalIndex(s);
               if (d->verticalHeader->isSectionHidden(row)) {
                  ++hiddenSections;
               }
            }
         }
         verticalScrollBar()->setValue(verticalIndex - hiddenSections);
      }

   } else { // ScrollPerPixel
      if (hint == PositionAtTop) {
         verticalScrollBar()->setValue(verticalPosition);
      } else if (hint == PositionAtBottom) {
         verticalScrollBar()->setValue(verticalPosition - viewportHeight + cellHeight);
      } else if (hint == PositionAtCenter) {
         verticalScrollBar()->setValue(verticalPosition - ((viewportHeight - cellHeight) / 2));
      }
   }

   update(index);
}

void QTableView::rowResized(int row, int, int)
{
   Q_D(QTableView);

   d->rowsToUpdate.append(row);
   if (d->rowResizeTimerID == 0) {
      d->rowResizeTimerID = startTimer(0);
   }
}

void QTableView::columnResized(int column, int, int)
{
   Q_D(QTableView);

   d->columnsToUpdate.append(column);
   if (d->columnResizeTimerID == 0) {
      d->columnResizeTimerID = startTimer(0);
   }
}

void QTableView::timerEvent(QTimerEvent *event)
{
   Q_D(QTableView);

   if (event->timerId() == d->columnResizeTimerID) {
      updateGeometries();
      killTimer(d->columnResizeTimerID);
      d->columnResizeTimerID = 0;

      QRect rect;
      int viewportHeight = d->viewport->height();
      int viewportWidth  = d->viewport->width();

      if (d->hasSpans()) {
         rect = QRect(0, 0, viewportWidth, viewportHeight);
      } else {
         for (int i = d->columnsToUpdate.size() - 1; i >= 0; --i) {
            int column = d->columnsToUpdate.at(i);
            int x = columnViewportPosition(column);
            if (isRightToLeft()) {
               rect |= QRect(0, 0, x + columnWidth(column), viewportHeight);
            } else {
               rect |= QRect(x, 0, viewportWidth - x, viewportHeight);
            }
         }
      }

      d->viewport->update(rect.normalized());
      d->columnsToUpdate.clear();
   }

   if (event->timerId() == d->rowResizeTimerID) {
      updateGeometries();
      killTimer(d->rowResizeTimerID);
      d->rowResizeTimerID = 0;

      int viewportHeight = d->viewport->height();
      int viewportWidth = d->viewport->width();
      int top;

      if (d->hasSpans()) {
         top = 0;
      } else {
         top = viewportHeight;
         for (int i = d->rowsToUpdate.size() - 1; i >= 0; --i) {
            int y = rowViewportPosition(d->rowsToUpdate.at(i));
            top = qMin(top, y);
         }
      }

      d->viewport->update(QRect(0, top, viewportWidth, viewportHeight - top));
      d->rowsToUpdate.clear();
   }

   QAbstractItemView::timerEvent(event);
}

void QTableView::rowMoved(int, int oldIndex, int newIndex)
{
   Q_D(QTableView);

   updateGeometries();
   int logicalOldIndex = d->verticalHeader->logicalIndex(oldIndex);
   int logicalNewIndex = d->verticalHeader->logicalIndex(newIndex);

   if (d->hasSpans()) {
      d->viewport->update();
   } else {
      int oldTop = rowViewportPosition(logicalOldIndex);
      int newTop = rowViewportPosition(logicalNewIndex);
      int oldBottom = oldTop + rowHeight(logicalOldIndex);
      int newBottom = newTop + rowHeight(logicalNewIndex);
      int top = qMin(oldTop, newTop);
      int bottom = qMax(oldBottom, newBottom);
      int height = bottom - top;
      d->viewport->update(0, top, d->viewport->width(), height);
   }
}

void QTableView::columnMoved(int, int oldIndex, int newIndex)
{
   Q_D(QTableView);

   updateGeometries();
   int logicalOldIndex = d->horizontalHeader->logicalIndex(oldIndex);
   int logicalNewIndex = d->horizontalHeader->logicalIndex(newIndex);
   if (d->hasSpans()) {
      d->viewport->update();
   } else {
      int oldLeft = columnViewportPosition(logicalOldIndex);
      int newLeft = columnViewportPosition(logicalNewIndex);
      int oldRight = oldLeft + columnWidth(logicalOldIndex);
      int newRight = newLeft + columnWidth(logicalNewIndex);
      int left = qMin(oldLeft, newLeft);
      int right = qMax(oldRight, newRight);
      int width = right - left;
      d->viewport->update(left, 0, width, d->viewport->height());
   }
}

void QTableView::selectRow(int row)
{
   Q_D(QTableView);
   d->selectRow(row, true);
}

void QTableView::selectColumn(int column)
{
   Q_D(QTableView);
   d->selectColumn(column, true);
}

void QTableView::hideRow(int row)
{
   Q_D(QTableView);
   d->verticalHeader->hideSection(row);
}

void QTableView::hideColumn(int column)
{
   Q_D(QTableView);
   d->horizontalHeader->hideSection(column);
}

void QTableView::showRow(int row)
{
   Q_D(QTableView);
   d->verticalHeader->showSection(row);
}

void QTableView::showColumn(int column)
{
   Q_D(QTableView);
   d->horizontalHeader->showSection(column);
}

void QTableView::resizeRowToContents(int row)
{
   Q_D(QTableView);
   int content = sizeHintForRow(row);
   int header = d->verticalHeader->sectionSizeHint(row);
   d->verticalHeader->resizeSection(row, qMax(content, header));
}

void QTableView::resizeRowsToContents()
{
   Q_D(QTableView);
   d->verticalHeader->resizeSections(QHeaderView::ResizeToContents);
}

void QTableView::resizeColumnToContents(int column)
{
   Q_D(QTableView);
   int content = sizeHintForColumn(column);
   int header = d->horizontalHeader->sectionSizeHint(column);
   d->horizontalHeader->resizeSection(column, qMax(content, header));
}

void QTableView::resizeColumnsToContents()
{
   Q_D(QTableView);
   d->horizontalHeader->resizeSections(QHeaderView::ResizeToContents);
}

void QTableView::sortByColumn(int column)
{
   Q_D(QTableView);
   if (column == -1) {
      return;
   }

   d->model->sort(column, d->horizontalHeader->sortIndicatorOrder());
}

void QTableView::sortByColumn(int column, Qt::SortOrder order)
{
   Q_D(QTableView);
   d->horizontalHeader->setSortIndicator(column, order);
   sortByColumn(column);
}

void QTableView::verticalScrollbarAction(int action)
{
   QAbstractItemView::verticalScrollbarAction(action);
}

void QTableView::horizontalScrollbarAction(int action)
{
   QAbstractItemView::horizontalScrollbarAction(action);
}

bool QTableView::isIndexHidden(const QModelIndex &index) const
{
   Q_D(const QTableView);

   Q_ASSERT(d->isIndexValid(index));

   if (isRowHidden(index.row()) || isColumnHidden(index.column())) {
      return true;
   }

   if (d->hasSpans()) {
      QSpanCollection::Span span = d->span(index.row(), index.column());
      return !((span.top() == index.row()) && (span.left() == index.column()));
   }

   return false;
}

void QTableView::setSpan(int row, int column, int rowSpan, int columnSpan)
{
   Q_D(QTableView);
   if (row < 0 || column < 0 || rowSpan < 0 || columnSpan < 0) {
      return;
   }
   d->setSpan(row, column, rowSpan, columnSpan);
   d->viewport->update();
}

int QTableView::rowSpan(int row, int column) const
{
   Q_D(const QTableView);
   return d->rowSpan(row, column);
}

int QTableView::columnSpan(int row, int column) const
{
   Q_D(const QTableView);
   return d->columnSpan(row, column);
}

void QTableView::clearSpans()
{
   Q_D(QTableView);
   d->spans.clear();
   d->viewport->update();
}

void QTableViewPrivate::_q_selectRow(int row)
{
   selectRow(row, false);
}

void QTableViewPrivate::_q_selectColumn(int column)
{
   selectColumn(column, false);
}

void QTableViewPrivate::selectRow(int row, bool anchor)
{
   Q_Q(QTableView);

   if (q->selectionBehavior() == QTableView::SelectColumns || (q->selectionMode() == QTableView::SingleSelection
         && q->selectionBehavior() == QTableView::SelectItems)) {
      return;
   }

   if (row >= 0 && row < model->rowCount(root)) {
      int column = horizontalHeader->logicalIndexAt(q->isRightToLeft() ? viewport->width() : 0);
      QModelIndex index = model->index(row, column, root);

      QItemSelectionModel::SelectionFlags command = q->selectionCommand(index);
      selectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);

      if ((anchor && !(command & QItemSelectionModel::Current))
            || (q->selectionMode() == QTableView::SingleSelection)) {
         rowSectionAnchor = row;
      }

      if (q->selectionMode() != QTableView::SingleSelection
            && command.testFlag(QItemSelectionModel::Toggle)) {

         if (anchor) {
            ctrlDragSelectionFlag = verticalHeader->selectionModel()->selectedRows().contains(index)
                  ? QItemSelectionModel::Deselect : QItemSelectionModel::Select;
         }

         command &= ~QItemSelectionModel::Toggle;
         command |= ctrlDragSelectionFlag;

         if (! anchor) {
            command |= QItemSelectionModel::Current;
         }
      }

      QModelIndex upper = model->index(qMin(rowSectionAnchor, row), column, root);
      QModelIndex lower = model->index(qMax(rowSectionAnchor, row), column, root);

      if ((verticalHeader->sectionsMoved() && upper.row() != lower.row())) {
         q->setSelection(q->visualRect(upper) | q->visualRect(lower), command | QItemSelectionModel::Rows);
      } else {
         selectionModel->select(QItemSelection(upper, lower), command | QItemSelectionModel::Rows);
      }
   }
}

void QTableViewPrivate::selectColumn(int column, bool anchor)
{
   Q_Q(QTableView);

   if (q->selectionBehavior() == QTableView::SelectRows
         || (q->selectionMode() == QTableView::SingleSelection
         && q->selectionBehavior() == QTableView::SelectItems)) {
      return;
   }

   if (column >= 0 && column < model->columnCount(root)) {
      int row = verticalHeader->logicalIndexAt(0);
      QModelIndex index = model->index(row, column, root);

      QItemSelectionModel::SelectionFlags command = q->selectionCommand(index);
      selectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);

      if ((anchor && !(command & QItemSelectionModel::Current))
            || (q->selectionMode() == QTableView::SingleSelection)) {
         columnSectionAnchor = column;
      }

      if (q->selectionMode() != QTableView::SingleSelection
            && command.testFlag(QItemSelectionModel::Toggle)) {

         if (anchor) {
            ctrlDragSelectionFlag = horizontalHeader->selectionModel()->selectedColumns().contains(index)
               ? QItemSelectionModel::Deselect : QItemSelectionModel::Select;
         }

         command &= ~QItemSelectionModel::Toggle;
         command |= ctrlDragSelectionFlag;

         if (! anchor) {
            command |= QItemSelectionModel::Current;
         }
      }

      QModelIndex left  = model->index(row, qMin(columnSectionAnchor, column), root);
      QModelIndex right = model->index(row, qMax(columnSectionAnchor, column), root);

      if ((horizontalHeader->sectionsMoved() && left.column() != right.column())) {
         q->setSelection(q->visualRect(left) | q->visualRect(right), command | QItemSelectionModel::Columns);
      } else {
         selectionModel->select(QItemSelection(left, right), command | QItemSelectionModel::Columns);
      }
   }
}

void QTableView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
#ifndef QT_NO_ACCESSIBILITY
   if (QAccessible::isActive()) {
      if (current.isValid()) {

         Q_D(QTableView);
         int entry = d->accessibleTable2Index(current);
         QAccessibleEvent event(this, QAccessible::Focus);
         event.setChild(entry);
         QAccessible::updateAccessibility(&event);

      }
   }
#endif

   QAbstractItemView::currentChanged(current, previous);
}

void QTableView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
   Q_D(QTableView);

#ifndef QT_NO_ACCESSIBILITY

   if (QAccessible::isActive()) {
      // ### does not work properly for selection ranges.
      QModelIndex sel = selected.indexes().value(0);

      if (sel.isValid()) {
         int entry = d->accessibleTable2Index(sel);
         QAccessibleEvent event(this, QAccessible::SelectionAdd);

         event.setChild(entry);
         QAccessible::updateAccessibility(&event);
      }
      QModelIndex desel = deselected.indexes().value(0);
      if (desel.isValid()) {

         int entry = d->accessibleTable2Index(desel);
         QAccessibleEvent event(this, QAccessible::SelectionRemove);
         event.setChild(entry);

         QAccessible::updateAccessibility(&event);

      }
   }
#endif

   QAbstractItemView::selectionChanged(selected, deselected);
}

int QTableView::visualIndex(const QModelIndex &index) const
{
   return index.row();
}

void QTableView::_q_selectRow(int row)
{
   Q_D(QTableView);
   d->_q_selectRow(row);
}

void QTableView::_q_selectColumn(int column)
{
   Q_D(QTableView);
   d->_q_selectColumn(column);
}

void QTableView::_q_updateSpanInsertedRows(const QModelIndex &parent, int start, int end)
{
   Q_D(QTableView);
   d->_q_updateSpanInsertedRows(parent, start, end);
}

void QTableView::_q_updateSpanInsertedColumns(const QModelIndex &parent, int start, int end)
{
   Q_D(QTableView);
   d->_q_updateSpanInsertedColumns(parent, start, end);
}

void QTableView::_q_updateSpanRemovedRows(const QModelIndex &parent, int start, int end)
{
   Q_D(QTableView);
   d->_q_updateSpanRemovedRows(parent, start, end);
}

void QTableView::_q_updateSpanRemovedColumns(const QModelIndex &parent, int start, int end)
{
   Q_D(QTableView);
   d->_q_updateSpanRemovedColumns(parent, start, end);
}

#endif // QT_NO_TABLEVIEW
