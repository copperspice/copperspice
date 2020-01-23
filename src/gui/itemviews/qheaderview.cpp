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

#include <qheaderview.h>

#ifndef QT_NO_ITEMVIEWS
#include <qbitarray.h>
#include <qbrush.h>
#include <qdebug.h>
#include <qevent.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qvector.h>
#include <qapplication.h>
#include <qvarlengtharray.h>
#include <qabstractitemdelegate.h>
#include <qvariant.h>
#include <qheaderview_p.h>
#include <qabstractitemmodel_p.h>

#include <qdatastream.h>



QDataStream &operator<<(QDataStream &out, const QHeaderViewPrivate::SectionItem &section)
{
   section.write(out);
   return out;
}

QDataStream &operator>>(QDataStream &in, QHeaderViewPrivate::SectionItem &section)
{
   section.read(in);
   return in;
}


static const int maxSizeSection = 1048575; // since section size is in a bitfield (uint 20). See qheaderview_p.h


QHeaderView::QHeaderView(Qt::Orientation orientation, QWidget *parent)
   : QAbstractItemView(*new QHeaderViewPrivate, parent)
{
   Q_D(QHeaderView);

   d->setDefaultValues(orientation);
   initialize();
}

/*!
  \internal
*/
QHeaderView::QHeaderView(QHeaderViewPrivate &dd, Qt::Orientation orientation, QWidget *parent)
   : QAbstractItemView(dd, parent)
{
   Q_D(QHeaderView);

   d->setDefaultValues(orientation);
   initialize();
}


QHeaderView::~QHeaderView()
{
}

/*!
  \internal
*/
void QHeaderView::initialize()
{
   Q_D(QHeaderView);

   setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   setFrameStyle(NoFrame);
   setFocusPolicy(Qt::NoFocus);

   d->viewport->setMouseTracking(true);
   d->viewport->setBackgroundRole(QPalette::Button);
   d->textElideMode = Qt::ElideNone;

   delete d->itemDelegate;
}

/*!
  \reimp
*/
void QHeaderView::setModel(QAbstractItemModel *model)
{
   if (model == this->model()) {
      return;
   }

   Q_D(QHeaderView);
   d->persistentHiddenSections.clear();

   if (d->model && d->model != QAbstractItemModelPrivate::staticEmptyModel()) {

      if (d->orientation == Qt::Horizontal) {
         QObject::disconnect(d->model, &QAbstractItemModel::columnsInserted,         this, &QHeaderView::sectionsInserted);
         QObject::disconnect(d->model, &QAbstractItemModel::columnsAboutToBeRemoved, this, &QHeaderView::sectionsAboutToBeRemoved);
         QObject::disconnect(d->model, &QAbstractItemModel::columnsRemoved,          this, &QHeaderView::_q_sectionsRemoved);
         QObject::disconnect(d->model, &QAbstractItemModel::columnsAboutToBeMoved,   this, &QHeaderView::_q_layoutAboutToBeChanged);

      } else {
         QObject::disconnect(d->model, &QAbstractItemModel::rowsInserted,            this, &QHeaderView::sectionsInserted);
         QObject::disconnect(d->model, &QAbstractItemModel::rowsAboutToBeRemoved,    this, &QHeaderView::sectionsAboutToBeRemoved);
         QObject::disconnect(d->model, &QAbstractItemModel::rowsRemoved,             this, &QHeaderView::_q_sectionsRemoved);
         QObject::disconnect(d->model, &QAbstractItemModel::rowsAboutToBeMoved,      this, &QHeaderView::_q_layoutAboutToBeChanged);
      }

      QObject::disconnect(d->model, &QAbstractItemModel::headerDataChanged,          this, &QHeaderView::headerDataChanged);
      QObject::disconnect(d->model, &QAbstractItemModel::layoutAboutToBeChanged,     this, &QHeaderView::_q_layoutAboutToBeChanged);
   }

   if (model && model != QAbstractItemModelPrivate::staticEmptyModel()) {

      if (d->orientation == Qt::Horizontal) {
         QObject::connect(model, &QAbstractItemModel::columnsInserted,         this, &QHeaderView::sectionsInserted);
         QObject::connect(model, &QAbstractItemModel::columnsAboutToBeRemoved, this, &QHeaderView::sectionsAboutToBeRemoved);
         QObject::connect(model, &QAbstractItemModel::columnsRemoved,          this, &QHeaderView::_q_sectionsRemoved);
         QObject::connect(model, &QAbstractItemModel::columnsAboutToBeMoved,   this, &QHeaderView::_q_layoutAboutToBeChanged);

      } else {
         QObject::connect(model, &QAbstractItemModel::rowsInserted,            this, &QHeaderView::sectionsInserted);
         QObject::connect(model, &QAbstractItemModel::rowsAboutToBeRemoved,    this, &QHeaderView::sectionsAboutToBeRemoved);
         QObject::connect(model, &QAbstractItemModel::rowsRemoved,             this, &QHeaderView::_q_sectionsRemoved);
         QObject::connect(model, &QAbstractItemModel::rowsAboutToBeMoved,      this, &QHeaderView::_q_layoutAboutToBeChanged);
      }

      QObject::connect(model, &QAbstractItemModel::headerDataChanged,          this, &QHeaderView::headerDataChanged);
      QObject::connect(model, &QAbstractItemModel::layoutAboutToBeChanged,     this, &QHeaderView::_q_layoutAboutToBeChanged);
   }

   d->state = QHeaderViewPrivate::NoClear;
   QAbstractItemView::setModel(model);
   d->state = QHeaderViewPrivate::NoState;

   // Users want to set sizes and modes before the widget is shown.
   // Thus, we have to initialize when the model is set,
   // and not lazily like we do in the other views.

   initializeSections();
}


Qt::Orientation QHeaderView::orientation() const
{
   Q_D(const QHeaderView);
   return d->orientation;
}

int QHeaderView::offset() const
{
   Q_D(const QHeaderView);
   return d->offset;
}

/*!
    \fn void QHeaderView::setOffset(int offset)

    Sets the header's offset to \a offset.

    \sa offset(), length()
*/

void QHeaderView::setOffset(int newOffset)
{
   Q_D(QHeaderView);
   if (d->offset == (int)newOffset) {
      return;
   }
   int ndelta = d->offset - newOffset;
   d->offset = newOffset;
   if (d->orientation == Qt::Horizontal) {
      d->viewport->scroll(isRightToLeft() ? -ndelta : ndelta, 0);
   } else {
      d->viewport->scroll(0, ndelta);
   }

   if (d->state == QHeaderViewPrivate::ResizeSection && ! d->preventCursorChangeInSetOffset) {
      QPoint cursorPos = QCursor::pos();
      if (d->orientation == Qt::Horizontal) {
         QCursor::setPos(cursorPos.x() + ndelta, cursorPos.y());
      } else {
         QCursor::setPos(cursorPos.x(), cursorPos.y() + ndelta);
      }
      d->firstPos += ndelta;
      d->lastPos += ndelta;
   }
}


void QHeaderView::setOffsetToSectionPosition(int visualSectionNumber)
{
   Q_D(QHeaderView);
   if (visualSectionNumber > -1 && visualSectionNumber < d->sectionCount()) {
      int position = d->headerSectionPosition(d->adjustedVisualIndex(visualSectionNumber));
      setOffset(position);
   }
}


void QHeaderView::setOffsetToLastSection()
{
   Q_D(const QHeaderView);
   int size = (d->orientation == Qt::Horizontal ? viewport()->width() : viewport()->height());
   int position = length() - size;
   setOffset(position);
}

/*!
    Returns the length along the orientation of the header.

    \sa sizeHint(), setResizeMode(), offset()
*/

int QHeaderView::length() const
{
   Q_D(const QHeaderView);
   d->executePostedLayout();
   d->executePostedResize();
   //Q_ASSERT(d->headerLength() == d->length);
   return d->length;
}

/*!
    Returns a suitable size hint for this header.

    \sa sectionSizeHint()
*/

QSize QHeaderView::sizeHint() const
{
   Q_D(const QHeaderView);
   if (d->cachedSizeHint.isValid()) {
      return d->cachedSizeHint;
   }
   d->cachedSizeHint = QSize(0, 0); //reinitialize the cached size hint
   const int sectionCount = count();

   // get size hint for the first n sections
   int i = 0;
   for (int checked = 0; checked < 100 && i < sectionCount; ++i) {
      if (isSectionHidden(i)) {
         continue;
      }
      checked++;
      QSize hint = sectionSizeFromContents(i);
      d->cachedSizeHint = d->cachedSizeHint.expandedTo(hint);
   }
   // get size hint for the last n sections
   i = qMax(i, sectionCount - 100 );
   for (int j = sectionCount - 1, checked = 0; j >= i && checked < 100; --j) {
      if (isSectionHidden(j)) {
         continue;
      }
      checked++;
      QSize hint = sectionSizeFromContents(j);
      d->cachedSizeHint = d->cachedSizeHint.expandedTo(hint);
   }
   return d->cachedSizeHint;
}


void QHeaderView::setVisible(bool v)
{
   bool actualChange = (v != isVisible());
   QAbstractItemView::setVisible(v);
   if (actualChange) {
      QAbstractScrollArea *parent = qobject_cast<QAbstractScrollArea *>(parentWidget());
      if (parent) {
         parent->updateGeometry();
      }
   }
}


int QHeaderView::sectionSizeHint(int logicalIndex) const
{
   Q_D(const QHeaderView);
   if (isSectionHidden(logicalIndex)) {
      return 0;
   }

   if (logicalIndex < 0 || logicalIndex >= count()) {
      return -1;
   }

   QSize size;
   QVariant value = d->model->headerData(logicalIndex, d->orientation, Qt::SizeHintRole);

   if (value.isValid()) {
      size = qvariant_cast<QSize>(value);
   } else {
      size = sectionSizeFromContents(logicalIndex);
   }

   int hint = d->orientation == Qt::Horizontal ? size.width() : size.height();
   return qBound(minimumSectionSize(), hint, maximumSectionSize());
}

/*!
    Returns the visual index of the section that covers the given \a position
    in the viewport.

    \sa logicalIndexAt()
*/

int QHeaderView::visualIndexAt(int position) const
{
   Q_D(const QHeaderView);
   int vposition = position;
   d->executePostedLayout();
   d->executePostedResize();

   const int count = d->sectionCount();

   if (count < 1) {

      return -1;
   }

   if (d->reverse()) {
      vposition = d->viewport->width() - vposition;
   }
   vposition += d->offset;

   if (vposition > d->length) {
      return -1;
   }
   int visual = d->headerVisualIndexAt(vposition);
   if (visual < 0) {
      return -1;
   }

   while (d->isVisualIndexHidden(visual)) {
      ++visual;
      if (visual >= count) {
         return -1;
      }
   }
   return visual;
}

/*!
    Returns the section that covers the given \a position in the viewport.

    \sa visualIndexAt(), isSectionHidden()
*/

int QHeaderView::logicalIndexAt(int position) const
{
   const int visual = visualIndexAt(position);
   if (visual > -1) {
      return logicalIndex(visual);
   }
   return -1;
}

/*!
    Returns the width (or height for vertical headers) of the given
    \a logicalIndex.

    \sa length(), setResizeMode(), defaultSectionSize()
*/

int QHeaderView::sectionSize(int logicalIndex) const
{
   Q_D(const QHeaderView);
   if (isSectionHidden(logicalIndex)) {
      return 0;
   }
   if (logicalIndex < 0 || logicalIndex >= count()) {
      return 0;
   }
   int visual = visualIndex(logicalIndex);
   if (visual == -1) {
      return 0;
   }
   d->executePostedResize();
   return d->headerSectionSize(visual);
}


int QHeaderView::sectionPosition(int logicalIndex) const
{
   Q_D(const QHeaderView);
   int visual = visualIndex(logicalIndex);
   // in some cases users may change the selections
   // before we have a chance to do the layout
   if (visual == -1) {
      return -1;
   }
   d->executePostedResize();
   return d->headerSectionPosition(visual);
}

/*!
    Returns the section viewport position of the given \a logicalIndex.

    If the section is hidden, the return value is undefined.

    \sa sectionPosition(), isSectionHidden()
*/

int QHeaderView::sectionViewportPosition(int logicalIndex) const
{
   Q_D(const QHeaderView);
   if (logicalIndex >= count()) {
      return -1;
   }
   int position = sectionPosition(logicalIndex);
   if (position < 0) {
      return position;   // the section was hidden
   }
   int offsetPosition = position - d->offset;
   if (d->reverse()) {
      return d->viewport->width() - (offsetPosition + sectionSize(logicalIndex));
   }
   return offsetPosition;
}

template<typename Container>
static void qMoveRange(Container &c, typename Container::size_type rangeStart,
   typename Container::size_type rangeEnd, typename Container::size_type targetPosition)
{
   Q_ASSERT(targetPosition <= c.size());
   Q_ASSERT(targetPosition < rangeStart || targetPosition >= rangeEnd);
   const bool forwardMove = targetPosition > rangeStart;

   typename Container::size_type first = (std::min)(rangeStart, targetPosition);
   typename Container::size_type mid   = forwardMove ? rangeEnd : rangeStart;
   typename Container::size_type last  = forwardMove ? targetPosition + 1 : rangeEnd;

   std::rotate(c.begin() + first, c.begin() + mid, c.begin() + last);
}


void QHeaderView::moveSection(int from, int to)
{
   Q_D(QHeaderView);

   d->executePostedLayout();
   if (from < 0 || from >= d->sectionCount() || to < 0 || to >= d->sectionCount()) {
      return;
   }

   if (from == to) {
      int logical = logicalIndex(from);
      Q_ASSERT(logical != -1);
      updateSection(logical);
      return;
   }

   if (stretchLastSection() &&  to == d->lastVisibleVisualIndex()) {
      d->lastSectionSize = sectionSize(from);
   }

   //int oldHeaderLength = length(); // ### for debugging; remove later
   d->initializeIndexMapping();

   int *visualIndices = d->visualIndices.data();
   int *logicalIndices = d->logicalIndices.data();
   int logical = logicalIndices[from];
   int visual = from;

   if (to > from) {
      while (visual < to) {
         visualIndices[logicalIndices[visual + 1]] = visual;
         logicalIndices[visual] = logicalIndices[visual + 1];
         ++visual;
      }
   } else {
      while (visual > to) {
         visualIndices[logicalIndices[visual - 1]] = visual;
         logicalIndices[visual] = logicalIndices[visual - 1];
         --visual;
      }
   }
   visualIndices[logical] = to;
   logicalIndices[to] = logical;

   qMoveRange(d->sectionItems, from, from + 1, to);

   d->sectionStartposRecalc = true;

   if (d->hasAutoResizeSections()) {
      d->doDelayedResizeSections();
   }
   d->viewport->update();

   emit sectionMoved(logical, from, to);
}


void QHeaderView::swapSections(int first, int second)
{
   Q_D(QHeaderView);

   if (first == second) {
      return;
   }

   d->executePostedLayout();

   if (first < 0 || first >= d->sectionCount() || second < 0 || second >= d->sectionCount()) {
      return;
   }

   int firstSize = d->headerSectionSize(first);
   ResizeMode firstMode = d->headerSectionResizeMode(first);
   int firstLogical = d->logicalIndex(first);

   int secondSize = d->headerSectionSize(second);
   ResizeMode secondMode = d->headerSectionResizeMode(second);
   int secondLogical = d->logicalIndex(second);

   if (d->state == QHeaderViewPrivate::ResizeSection) {
      d->preventCursorChangeInSetOffset = true;
   }
   d->createSectionItems(second, second, firstSize, firstMode);
   d->createSectionItems(first, first, secondSize, secondMode);

   d->initializeIndexMapping();

   d->visualIndices[firstLogical] = second;
   d->logicalIndices[second] = firstLogical;

   d->visualIndices[secondLogical] = first;
   d->logicalIndices[first] = secondLogical;

   if (!d->hiddenSectionSize.isEmpty()) {
      bool firstHidden = d->isVisualIndexHidden(first);
      bool secondHidden = d->isVisualIndexHidden(second);
      d->setVisualIndexHidden(first, secondHidden);
      d->setVisualIndexHidden(second, firstHidden);
   }

   d->viewport->update();
   emit sectionMoved(firstLogical, first, second);
   emit sectionMoved(secondLogical, second, first);
}


void QHeaderView::resizeSection(int logical, int size)
{
   Q_D(QHeaderView);
   if (logical < 0 || logical >= count() || size < 0 || size > maxSizeSection) {
      return;
   }

   if (isSectionHidden(logical)) {
      d->hiddenSectionSize.insert(logical, size);
      return;
   }

   int visual = visualIndex(logical);
   if (visual == -1) {
      return;
   }

   if (d->state == QHeaderViewPrivate::ResizeSection && !d->cascadingResizing && logical != d->section) {
      d->preventCursorChangeInSetOffset = true;
   }

   int oldSize = d->headerSectionSize(visual);
   if (oldSize == size) {
      return;
   }

   d->executePostedLayout();
   d->invalidateCachedSizeHint();

   if (stretchLastSection() && visual == d->lastVisibleVisualIndex()) {
      d->lastSectionSize = size;
   }

   d->createSectionItems(visual, visual, size, d->headerSectionResizeMode(visual));

   if (!updatesEnabled()) {
      if (d->hasAutoResizeSections()) {
         d->doDelayedResizeSections();
      }
      emit sectionResized(logical, oldSize, size);
      return;
   }

   int w = d->viewport->width();
   int h = d->viewport->height();
   int pos = sectionViewportPosition(logical);
   QRect r;

   if (d->orientation == Qt::Horizontal)
      if (isRightToLeft()) {
         r.setRect(0, 0, pos + size, h);
      } else {
         r.setRect(pos, 0, w - pos, h);
      } else {
      r.setRect(0, pos, w, h - pos);
   }

   if (d->hasAutoResizeSections()) {
      d->doDelayedResizeSections();
      r = d->viewport->rect();
   }
   // If the parent is a QAbstractScrollArea with QAbstractScrollArea::AdjustToContents
   // then we want to change the geometry on that widget. Not doing it at once can/will
   // cause scrollbars flicker as they would be shown at first but then removed.
   // In the same situation it will also allow shrinking the whole view when stretchLastSection is set
   // (It is default on QTreeViews - and it wouldn't shrink since the last stretch was made before the
   // viewport was resized)

   QAbstractScrollArea *parent = qobject_cast<QAbstractScrollArea *>(parentWidget());
   if (parent && parent->sizeAdjustPolicy() == QAbstractScrollArea::AdjustToContents) {
      parent->updateGeometry();
   }
   d->viewport->update(r.normalized());
   emit sectionResized(logical, oldSize, size);
}



void QHeaderView::resizeSections(QHeaderView::ResizeMode mode)
{
   Q_D(QHeaderView);
   d->resizeSections(mode, true);
}


bool QHeaderView::isSectionHidden(int logicalIndex) const
{
   Q_D(const QHeaderView);
   d->executePostedLayout();
   if (d->hiddenSectionSize.isEmpty() || logicalIndex < 0 || logicalIndex >= d->sectionCount()) {
      return false;
   }

   int visual = visualIndex(logicalIndex);
   Q_ASSERT(visual != -1);

   return d->isVisualIndexHidden(visual);
}


int QHeaderView::hiddenSectionCount() const
{
   Q_D(const QHeaderView);
   return d->hiddenSectionSize.count();
}

/*!
  If \a hide is true the section specified by \a logicalIndex is hidden;
  otherwise the section is shown.

  \sa isSectionHidden(), hiddenSectionCount()
*/

void QHeaderView::setSectionHidden(int logicalIndex, bool hide)
{
   Q_D(QHeaderView);
   if (logicalIndex < 0 || logicalIndex >= count()) {
      return;
   }

   d->executePostedLayout();
   int visual = visualIndex(logicalIndex);
   Q_ASSERT(visual != -1);
   if (hide == d->isVisualIndexHidden(visual)) {
      return;
   }

   if (hide) {
      int size = d->headerSectionSize(visual);
      if (!d->hasAutoResizeSections()) {
         resizeSection(logicalIndex, 0);
      }
      d->hiddenSectionSize.insert(logicalIndex, size);
      d->setVisualIndexHidden(visual, true);
      if (d->hasAutoResizeSections()) {
         d->doDelayedResizeSections();
      }
   } else {
      int size = d->hiddenSectionSize.value(logicalIndex, d->defaultSectionSize);
      d->hiddenSectionSize.remove(logicalIndex);
      d->setVisualIndexHidden(visual, false);
      resizeSection(logicalIndex, size);
   }
}



int QHeaderView::count() const
{
   Q_D(const QHeaderView);

   // ### this may affect the lazy layout
   d->executePostedLayout();

   return d->sectionCount();
}

/*!
    Returns the visual index position of the section specified by the given
    \a logicalIndex, or -1 otherwise.

    Hidden sections still have valid visual indexes.

    \sa logicalIndex()
*/

int QHeaderView::visualIndex(int logicalIndex) const
{
   Q_D(const QHeaderView);
   if (logicalIndex < 0) {
      return -1;
   }

   d->executePostedLayout();

   if (d->visualIndices.isEmpty()) {
      // nothing has been moved, so we have no mapping
      if (logicalIndex < d->sectionCount()) {
         return logicalIndex;
      }

   } else if (logicalIndex < d->visualIndices.count()) {
      int visual = d->visualIndices.at(logicalIndex);
      Q_ASSERT(visual < d->sectionCount());
      return visual;
   }
   return -1;
}


int QHeaderView::logicalIndex(int visualIndex) const
{
   Q_D(const QHeaderView);
   if (visualIndex < 0 || visualIndex >= d->sectionCount()) {
      return -1;
   }
   return d->logicalIndex(visualIndex);
}



void QHeaderView::setSectionsMovable(bool movable)
{
   Q_D(QHeaderView);
   d->movableSections = movable;
}



bool QHeaderView::sectionsMovable() const
{
   Q_D(const QHeaderView);
   return d->movableSections;
}

void QHeaderView::setSectionsClickable(bool clickable)
{
   Q_D(QHeaderView);
   d->clickableSections = clickable;
}



bool QHeaderView::sectionsClickable() const
{
   Q_D(const QHeaderView);
   return d->clickableSections;
}

void QHeaderView::setHighlightSections(bool highlight)
{
   Q_D(QHeaderView);
   d->highlightSelected = highlight;
}

bool QHeaderView::highlightSections() const
{
   Q_D(const QHeaderView);
   return d->highlightSelected;
}

/*!
    Sets the constraints on how the header can be resized to those described
    by the given \a mode.

    \sa resizeMode(), length(), sectionResized(), sectionAutoResize()
*/

void QHeaderView::setSectionResizeMode(ResizeMode mode)
{
   Q_D(QHeaderView);
   initializeSections();
   d->stretchSections = (mode == Stretch ? count() : 0);
   d->contentsSections =  (mode == ResizeToContents ? count() : 0);
   d->setGlobalHeaderResizeMode(mode);
   if (d->hasAutoResizeSections()) {
      d->doDelayedResizeSections();   // section sizes may change as a result of the new mode
   }
}




void QHeaderView::setSectionResizeMode(int logicalIndex, ResizeMode mode)
{
   Q_D(QHeaderView);
   int visual = visualIndex(logicalIndex);
   Q_ASSERT(visual != -1);

   ResizeMode old = d->headerSectionResizeMode(visual);
   d->setHeaderSectionResizeMode(visual, mode);

   if (mode == Stretch && old != Stretch) {
      ++d->stretchSections;
   } else if (mode == ResizeToContents && old != ResizeToContents) {
      ++d->contentsSections;
   } else if (mode != Stretch && old == Stretch) {
      --d->stretchSections;
   } else if (mode != ResizeToContents && old == ResizeToContents) {
      --d->contentsSections;
   }

   if (d->hasAutoResizeSections() && d->state == QHeaderViewPrivate::NoState) {
      d->doDelayedResizeSections();   // section sizes may change as a result of the new mode
   }

}


QHeaderView::ResizeMode QHeaderView::sectionResizeMode(int logicalIndex) const
{
   Q_D(const QHeaderView);
   int visual = visualIndex(logicalIndex);
   if (visual == -1) {
      return Fixed;   //the default value
   }

   return d->headerSectionResizeMode(visual);
}

void QHeaderView::setResizeContentsPrecision(int precision)
{
   Q_D(QHeaderView);
   d->resizeContentsPrecision = precision;
}
int QHeaderView::resizeContentsPrecision() const
{
   Q_D(const QHeaderView);
   return d->resizeContentsPrecision;
}


int QHeaderView::stretchSectionCount() const
{
   Q_D(const QHeaderView);
   return d->stretchSections;
}

void QHeaderView::setSortIndicatorShown(bool show)
{
   Q_D(QHeaderView);
   if (d->sortIndicatorShown == show) {
      return;
   }

   d->sortIndicatorShown = show;

   if (sortIndicatorSection() < 0 || sortIndicatorSection() > count()) {
      return;
   }

   if (d->headerSectionResizeMode(sortIndicatorSection()) == ResizeToContents) {
      resizeSections();
   }

   d->viewport->update();
}

bool QHeaderView::isSortIndicatorShown() const
{
   Q_D(const QHeaderView);
   return d->sortIndicatorShown;
}

void QHeaderView::setSortIndicator(int logicalIndex, Qt::SortOrder order)
{
   Q_D(QHeaderView);

   // This is so that people can set the position of the sort indicator before the fill the model
   int old = d->sortIndicatorSection;
   if (old == logicalIndex && order == d->sortIndicatorOrder) {
      return;
   }

   d->sortIndicatorSection = logicalIndex;
   d->sortIndicatorOrder = order;

   if (logicalIndex >= d->sectionCount()) {
      emit sortIndicatorChanged(logicalIndex, order);
      return; // nothing to do
   }

   if (old != logicalIndex
      && ((logicalIndex >= 0 && sectionResizeMode(logicalIndex) == ResizeToContents)
         || old >= d->sectionCount() || (old >= 0 && sectionResizeMode(old) == ResizeToContents))) {

      resizeSections();
      d->viewport->update();
   } else {
      if (old >= 0 && old != logicalIndex) {
         updateSection(old);
      }
      if (logicalIndex >= 0) {
         updateSection(logicalIndex);
      }
   }

   emit sortIndicatorChanged(logicalIndex, order);
}



int QHeaderView::sortIndicatorSection() const
{
   Q_D(const QHeaderView);
   return d->sortIndicatorSection;
}



Qt::SortOrder QHeaderView::sortIndicatorOrder() const
{
   Q_D(const QHeaderView);
   return d->sortIndicatorOrder;
}

bool QHeaderView::stretchLastSection() const
{
   Q_D(const QHeaderView);
   return d->stretchLastSection;
}

void QHeaderView::setStretchLastSection(bool stretch)
{
   Q_D(QHeaderView);

   if (d->stretchLastSection == stretch) {
      return;
   }

   d->stretchLastSection = stretch;
   if (d->state != QHeaderViewPrivate::NoState) {
      return;
   }
   if (stretch) {
      resizeSections();
   } else if (count()) {
      resizeSection(count() - 1, d->defaultSectionSize);
   }
}


bool QHeaderView::cascadingSectionResizes() const
{
   Q_D(const QHeaderView);
   return d->cascadingResizing;
}

void QHeaderView::setCascadingSectionResizes(bool enable)
{
   Q_D(QHeaderView);
   d->cascadingResizing = enable;
}


int QHeaderView::defaultSectionSize() const
{
   Q_D(const QHeaderView);
   return d->defaultSectionSize;
}

void QHeaderView::setDefaultSectionSize(int size)
{
   Q_D(QHeaderView);
   if (size < 0 || size > maxSizeSection) {
      return;
   }
   d->setDefaultSectionSize(size);
}

void QHeaderView::resetDefaultSectionSize()
{
   Q_D(QHeaderView);
   if (d->customDefaultSectionSize) {
      d->updateDefaultSectionSizeFromStyle();
      d->customDefaultSectionSize = false;
   }
}

int QHeaderView::minimumSectionSize() const
{
   Q_D(const QHeaderView);
   if (d->minimumSectionSize == -1) {
      QSize strut = QApplication::globalStrut();
      int margin = style()->pixelMetric(QStyle::PM_HeaderMargin, 0, this);
      if (d->orientation == Qt::Horizontal) {
         return qMax(strut.width(), (fontMetrics().maxWidth() + margin));
      }
      return qMax(strut.height(), (fontMetrics().height() + margin));
   }
   return d->minimumSectionSize;
}

void QHeaderView::setMinimumSectionSize(int size)
{
   Q_D(QHeaderView);

   if (size < -1 || size > maxSizeSection) {
      return;
   }
   d->minimumSectionSize = size;
   if (d->minimumSectionSize > maximumSectionSize()) {
      d->maximumSectionSize = size;
   }
}
int QHeaderView::maximumSectionSize() const
{
   Q_D(const QHeaderView);
   if (d->maximumSectionSize == -1) {
      return maxSizeSection;
   }
   return d->maximumSectionSize;
}
void QHeaderView::setMaximumSectionSize(int size)
{
   Q_D(QHeaderView);
   if (size == -1) {
      d->maximumSectionSize = maxSizeSection;
      return;
   }
   if (size < 0 || size > maxSizeSection) {
      return;
   }
   if (minimumSectionSize() > size) {
      d->minimumSectionSize = size;
   }
   d->maximumSectionSize = size;
}

/*!
    \since 4.1
    \property QHeaderView::defaultAlignment
    \brief the default alignment of the text in each header section
*/

Qt::Alignment QHeaderView::defaultAlignment() const
{
   Q_D(const QHeaderView);
   return d->defaultAlignment;
}

void QHeaderView::setDefaultAlignment(Qt::Alignment alignment)
{
   Q_D(QHeaderView);
   if (d->defaultAlignment == alignment) {
      return;
   }

   d->defaultAlignment = alignment;
   d->viewport->update();
}

/*!
    \internal
*/
void QHeaderView::doItemsLayout()
{
   initializeSections();
   QAbstractItemView::doItemsLayout();
}

/*!
    Returns true if sections in the header has been moved; otherwise returns
    false;

    \sa moveSection()
*/
bool QHeaderView::sectionsMoved() const
{
   Q_D(const QHeaderView);
   return !d->visualIndices.isEmpty();
}


bool QHeaderView::sectionsHidden() const
{
   Q_D(const QHeaderView);
   return !d->hiddenSectionSize.isEmpty();
}


QByteArray QHeaderView::saveState() const
{
   Q_D(const QHeaderView);
   QByteArray data;
   QDataStream stream(&data, QIODevice::WriteOnly);
   stream << QHeaderViewPrivate::VersionMarker;
   stream << 0; // current version is 0
   d->write(stream);
   return data;
}


bool QHeaderView::restoreState(const QByteArray &state)
{
   Q_D(QHeaderView);
   if (state.isEmpty()) {
      return false;
   }
   QByteArray data = state;
   QDataStream stream(&data, QIODevice::ReadOnly);
   int marker;
   int ver;
   stream >> marker;
   stream >> ver;
   if (stream.status() != QDataStream::Ok || marker != QHeaderViewPrivate::VersionMarker || ver != 0) {

      return false;
   }

   if (d->read(stream)) {
      emit sortIndicatorChanged(d->sortIndicatorSection, d->sortIndicatorOrder );
      d->viewport->update();
      return true;
   }
   return false;
}


/*!
  \reimp
*/
void QHeaderView::reset()
{
   QAbstractItemView::reset();
   // it would be correct to call clear, but some apps rely
   // on the header keeping the sections, even after calling reset
   //d->clear();
   initializeSections();
}


void QHeaderView::headerDataChanged(Qt::Orientation orientation, int logicalFirst, int logicalLast)
{
   Q_D(QHeaderView);
   if (d->orientation != orientation) {
      return;
   }

   if (logicalFirst < 0 || logicalLast < 0 || logicalFirst >= count() || logicalLast >= count()) {
      return;
   }

   d->invalidateCachedSizeHint();

   int firstVisualIndex = INT_MAX, lastVisualIndex = -1;

   for (int section = logicalFirst; section <= logicalLast; ++section) {
      const int visual = visualIndex(section);
      firstVisualIndex = qMin(firstVisualIndex, visual);
      lastVisualIndex =  qMax(lastVisualIndex,  visual);
   }

   d->executePostedResize();
   const int first = d->headerSectionPosition(firstVisualIndex),
             last = d->headerSectionPosition(lastVisualIndex)
                + d->headerSectionSize(lastVisualIndex);

   if (orientation == Qt::Horizontal) {
      d->viewport->update(first, 0, last - first, d->viewport->height());
   } else {
      d->viewport->update(0, first, d->viewport->width(), last - first);
   }
}



void QHeaderView::updateSection(int logicalIndex)
{
   Q_D(QHeaderView);
   if (d->orientation == Qt::Horizontal)
      d->viewport->update(QRect(sectionViewportPosition(logicalIndex),
            0, sectionSize(logicalIndex), d->viewport->height()));
   else
      d->viewport->update(QRect(0, sectionViewportPosition(logicalIndex),
            d->viewport->width(), sectionSize(logicalIndex)));
}

/*!
    Resizes the sections according to their size hints. Normally, you do not
    have to call this function.
*/

void QHeaderView::resizeSections()
{
   Q_D(QHeaderView);
   if (d->hasAutoResizeSections()) {
      d->resizeSections(Interactive, false);   // no global resize mode
   }
}



void QHeaderView::sectionsInserted(const QModelIndex &parent,
   int logicalFirst, int logicalLast)
{
   Q_D(QHeaderView);
   if (parent != d->root) {
      return;   // we only handle changes in the top level
   }

   int oldCount = d->sectionCount();

   d->invalidateCachedSizeHint();

   if (d->state == QHeaderViewPrivate::ResizeSection) {
      d->preventCursorChangeInSetOffset = true;
   }
   // add the new sections
   int insertAt = logicalFirst;

   int insertCount = logicalLast - logicalFirst + 1;
   QHeaderViewPrivate::SectionItem section(d->defaultSectionSize, d->globalResizeMode);
   d->sectionStartposRecalc = true;

   if (d->sectionItems.isEmpty() || insertAt >= d->sectionItems.count()) {
      int insertLength = d->defaultSectionSize * insertCount;
      d->length += insertLength;
      d->sectionItems.insert(d->sectionItems.count(), insertCount, section); // append
   } else {
      // separate them out into their own sections
      int insertLength = d->defaultSectionSize * insertCount;
      d->length += insertLength;
      d->sectionItems.insert(insertAt, insertCount, section);
   }

   // update sorting column
   if (d->sortIndicatorSection >= logicalFirst) {
      d->sortIndicatorSection += insertCount;
   }

   // update resize mode section counts
   if (d->globalResizeMode == Stretch) {
      d->stretchSections = d->sectionCount();
   } else if (d->globalResizeMode == ResizeToContents) {
      d->contentsSections = d->sectionCount();
   }

   // clear selection cache
   d->sectionSelected.clear();

   // update mapping
   if (!d->visualIndices.isEmpty() && !d->logicalIndices.isEmpty()) {
      Q_ASSERT(d->visualIndices.count() == d->logicalIndices.count());
      int mappingCount = d->visualIndices.count();
      for (int i = 0; i < mappingCount; ++i) {
         if (d->visualIndices.at(i) >= logicalFirst) {
            d->visualIndices[i] += insertCount;
         }
         if (d->logicalIndices.at(i) >= logicalFirst) {
            d->logicalIndices[i] += insertCount;
         }
      }
      for (int j = logicalFirst; j <= logicalLast; ++j) {
         d->visualIndices.insert(j, j);
         d->logicalIndices.insert(j, j);
      }
   }


   // insert sections into hiddenSectionSize
   QHash<int, int> newHiddenSectionSize; // from logical index to section size
   for (auto it = d->hiddenSectionSize.cbegin(), end = d->hiddenSectionSize.cend(); it != end; ++it) {

      const int oldIndex = it.key();
      const int newIndex = (oldIndex < logicalFirst) ? oldIndex : oldIndex + insertCount;

      newHiddenSectionSize[newIndex] = it.value();
   }

   d->hiddenSectionSize.swap(newHiddenSectionSize);

   d->doDelayedResizeSections();
   emit sectionCountChanged(oldCount, count());

   // if the new sections were not updated by resizing, we need to update now
   if (! d->hasAutoResizeSections()) {
      d->viewport->update();
   }
}

/*!
    This slot is called when sections are removed from the \a parent.
    \a logicalFirst and \a logicalLast signify where the sections were removed.

    If only one section is removed, \a logicalFirst and \a logicalLast will
    be the same.
*/

void QHeaderView::sectionsAboutToBeRemoved(const QModelIndex &parent, int logicalFirst, int logicalLast)
{

}

void QHeaderViewPrivate::updateHiddenSections(int logicalFirst, int logicalLast)
{
   Q_Q(QHeaderView);
   const int changeCount = logicalLast - logicalFirst + 1;

   // remove sections from hiddenSectionSize
   QHash<int, int> newHiddenSectionSize; // from logical index to section size

   for (int i = 0; i < logicalFirst; ++i) {

      if (q->isSectionHidden(i)) {
         newHiddenSectionSize[i] = hiddenSectionSize[i];
      }
   }

   for (int j = logicalLast + 1; j < sectionCount(); ++j) {
      if (q->isSectionHidden(j)) {
         newHiddenSectionSize[j - changeCount] = hiddenSectionSize[j];
      }
   }

   hiddenSectionSize = newHiddenSectionSize;
}

void QHeaderViewPrivate::_q_sectionsRemoved(const QModelIndex &parent, int logicalFirst, int logicalLast)
{
   Q_Q(QHeaderView);
   if (parent != root) {
      return;   // we only handle changes in the top level
   }

   if (qMin(logicalFirst, logicalLast) < 0 || qMax(logicalLast, logicalFirst) >= sectionCount()) {
      return;
   }

   int oldCount = q->count();
   int changeCount = logicalLast - logicalFirst + 1;

   if (state == QHeaderViewPrivate::ResizeSection) {
      preventCursorChangeInSetOffset = true;
   }
   updateHiddenSections(logicalFirst, logicalLast);

   if (visualIndices.isEmpty() && logicalIndices.isEmpty()) {
      // Q_ASSERT(headerSectionCount() == sectionCount);
      removeSectionsFromSectionItems(logicalFirst, logicalLast);

   } else {
      if (logicalFirst == logicalLast) {
         // Remove just one index

         int l      = logicalFirst;
         int visual = visualIndices.at(l);

         Q_ASSERT(sectionCount() == logicalIndices.count());

         for (int v = 0; v < sectionCount(); ++v) {

            if (v > visual) {
               int logical = logicalIndices.at(v);
               --(visualIndices[logical]);
            }

            if (logicalIndex(v) > l) {
               // no need to move the positions before l
               --(logicalIndices[v]);
            }
         }

         logicalIndices.remove(visual);
         visualIndices.remove(l);

         // Q_ASSERT(headerSectionCount() == sectionCount);
         removeSectionsFromSectionItems(visual, visual);

      } else {
         sectionStartposRecalc = true; // We will need to recalc positions after removing items
         for (int u = 0; u < sectionItems.count(); ++u) { // Store section info
            sectionItems.at(u).tmpLogIdx = logicalIndices.at(u);
         }
         for (int v = sectionItems.count() - 1; v >= 0; --v) {  // Remove the sections
            if (logicalFirst <= sectionItems.at(v).tmpLogIdx && sectionItems.at(v).tmpLogIdx <= logicalLast) {
               removeSectionsFromSectionItems(v, v);
            }
         }
         visualIndices.resize(sectionItems.count());
         logicalIndices.resize(sectionItems.count());
         int *visual_data = visualIndices.data();
         int *logical_data = logicalIndices.data();
         for (int w = 0; w < sectionItems.count(); ++w) { // Restore visual and logical indexes
            int logindex = sectionItems.at(w).tmpLogIdx;
            if (logindex > logicalFirst) {
               logindex -= changeCount;
            }
            visual_data[logindex] = w;
            logical_data[w] = logindex;
         }
      }
      // ### handle sectionSelection, sectionHidden
   }


   // update sorting column
   if (sortIndicatorSection >= logicalFirst) {
      if (sortIndicatorSection <= logicalLast) {
         sortIndicatorSection = -1;
      } else {
         sortIndicatorSection -= changeCount;
      }
   }

   // if we only have the last section (the "end" position) left, the header is empty
   if (sectionCount() <= 0) {
      clear();
   }

   invalidateCachedSizeHint();
   emit q->sectionCountChanged(oldCount, q->count());
   viewport->update();
}

void QHeaderViewPrivate::_q_layoutAboutToBeChanged()
{
   //if there is no row/column we can't have mapping for columns
   //because no QModelIndex in the model would be valid
   // ### this is far from being bullet-proof and we would need a real system to
   // ### map columns or rows persistently
   if ((orientation == Qt::Horizontal && model->rowCount(root) == 0)
      || model->columnCount(root) == 0) {
      return;
   }

   if (hiddenSectionSize.count() == 0) {
      return;
   }

   for (int i = 0; i < sectionItems.count(); ++i)

      if (isVisualIndexHidden(i))  {
         // ### note that we are using column or row 0

         persistentHiddenSections.append(orientation == Qt::Horizontal
            ? model->index(0, logicalIndex(i), root)
            : model->index(logicalIndex(i), 0, root));
      }
}

void QHeaderViewPrivate::_q_layoutChanged()
{
   Q_Q(QHeaderView);
   viewport->update();
   if (persistentHiddenSections.isEmpty() || modelIsEmpty()) {
      if (modelSectionCount() != sectionCount()) {
         q->initializeSections();
      }
      persistentHiddenSections.clear();
      return;
   }

   QBitArray oldSectionHidden = sectionsHiddenToBitVector();
   oldSectionHidden.resize(sectionItems.size());

   bool sectionCountChanged = false;

   for (int i = 0; i < persistentHiddenSections.count(); ++i) {
      QModelIndex index = persistentHiddenSections.at(i);
      if (index.isValid()) {
         const int logical = (orientation == Qt::Horizontal
               ? index.column()
               : index.row());
         q->setSectionHidden(logical, true);
         oldSectionHidden.setBit(logical, false);
      } else if (!sectionCountChanged && (modelSectionCount() != sectionCount())) {
         sectionCountChanged = true;
         break;
      }
   }
   persistentHiddenSections.clear();

   for (int i = 0; i < oldSectionHidden.count(); ++i) {
      if (oldSectionHidden.testBit(i)) {
         q->setSectionHidden(i, false);
      }
   }

   // the number of sections changed; we need to reread the state of the model
   if (sectionCountChanged) {
      q->initializeSections();
   }
}

/*!
  \internal
*/

void QHeaderView::initializeSections()
{
   Q_D(QHeaderView);
   const int oldCount = d->sectionCount();
   const int newCount = d->modelSectionCount();

   if (newCount <= 0) {
      d->clear();
      emit sectionCountChanged(oldCount, 0);

   } else if (newCount != oldCount) {
      const int min = qBound(0, oldCount, newCount - 1);
      initializeSections(min, newCount - 1);

      if (stretchLastSection()) { // we've already gotten the size hint
         d->lastSectionSize = sectionSize(logicalIndex(d->sectionCount() - 1));
      }

      //make sure we update the hidden sections
      if (newCount < oldCount) {
         d->updateHiddenSections(0, newCount - 1);
      }
   }
}


void QHeaderView::initializeSections(int start, int end)
{
   Q_D(QHeaderView);

   Q_ASSERT(start >= 0);
   Q_ASSERT(end >= 0);

   d->invalidateCachedSizeHint();
   int oldCount = d->sectionCount();

   if (end + 1 < d->sectionCount()) {
      int newCount = end + 1;
      d->removeSectionsFromSectionItems(newCount, d->sectionCount() - 1);

      if (!d->hiddenSectionSize.isEmpty()) {
         if (oldCount - newCount > d->hiddenSectionSize.count()) {
            for (int i = end + 1; i < d->sectionCount(); ++i) {
               d->hiddenSectionSize.remove(i);
            }

         } else {
            QHash<int, int>::iterator it = d->hiddenSectionSize.begin();

            while (it != d->hiddenSectionSize.end()) {
               if (it.key() > end) {
                  it = d->hiddenSectionSize.erase(it);
               } else {
                  ++it;
               }
            }
         }
      }
   }

   int newSectionCount = end + 1;

   if (!d->logicalIndices.isEmpty()) {
      if (oldCount <= newSectionCount) {
         d->logicalIndices.resize(newSectionCount);
         d->visualIndices.resize(newSectionCount);
         for (int i = oldCount; i < newSectionCount; ++i) {
            d->logicalIndices[i] = i;
            d->visualIndices[i] = i;
         }
      } else {
         int j = 0;
         for (int i = 0; i < oldCount; ++i) {
            int v = d->logicalIndices.at(i);
            if (v < newSectionCount) {
               d->logicalIndices[j] = v;
               d->visualIndices[v] = j;
               j++;
            }
         }
         d->logicalIndices.resize(newSectionCount);
         d->visualIndices.resize(newSectionCount);
      }
   }

   if (d->globalResizeMode == Stretch) {
      d->stretchSections = newSectionCount;
   } else if (d->globalResizeMode == ResizeToContents) {
      d->contentsSections = newSectionCount;
   }

   if (newSectionCount > oldCount) {
      d->createSectionItems(start, end, (end - start + 1) * d->defaultSectionSize, d->globalResizeMode);
   }
   //Q_ASSERT(d->headerLength() == d->length);

   if (d->sectionCount() != oldCount) {
      emit sectionCountChanged(oldCount,  d->sectionCount());
   }
   d->viewport->update();
}

/*!
  \reimp
*/

void QHeaderView::currentChanged(const QModelIndex &current, const QModelIndex &old)
{
   Q_D(QHeaderView);

   if (d->orientation == Qt::Horizontal && current.column() != old.column()) {
      if (old.isValid() && old.parent() == d->root)
         d->viewport->update(QRect(sectionViewportPosition(old.column()), 0,
               sectionSize(old.column()), d->viewport->height()));
      if (current.isValid() && current.parent() == d->root)
         d->viewport->update(QRect(sectionViewportPosition(current.column()), 0,
               sectionSize(current.column()), d->viewport->height()));
   } else if (d->orientation == Qt::Vertical && current.row() != old.row()) {
      if (old.isValid() && old.parent() == d->root)
         d->viewport->update(QRect(0, sectionViewportPosition(old.row()),
               d->viewport->width(), sectionSize(old.row())));
      if (current.isValid() && current.parent() == d->root)
         d->viewport->update(QRect(0, sectionViewportPosition(current.row()),
               d->viewport->width(), sectionSize(current.row())));
   }
}


/*!
  \reimp
*/

bool QHeaderView::event(QEvent *e)
{
   Q_D(QHeaderView);
   switch (e->type()) {
      case QEvent::HoverEnter: {
         QHoverEvent *he = static_cast<QHoverEvent *>(e);
         d->hover = logicalIndexAt(he->pos());
         if (d->hover != -1) {
            updateSection(d->hover);
         }
         break;
      }
      case QEvent::Leave:
      case QEvent::HoverLeave: {
         if (d->hover != -1) {
            updateSection(d->hover);
         }
         d->hover = -1;
         break;
      }
      case QEvent::HoverMove: {
         QHoverEvent *he = static_cast<QHoverEvent *>(e);
         int oldHover = d->hover;
         d->hover = logicalIndexAt(he->pos());

         if (d->hover != oldHover) {
            if (oldHover != -1) {
               updateSection(oldHover);
            }

            if (d->hover != -1) {
               updateSection(d->hover);
            }
         }
         break;
      }

      case QEvent::Timer: {
         QTimerEvent *te = static_cast<QTimerEvent *>(e);
         if (te->timerId() == d->delayedResize.timerId()) {
            d->delayedResize.stop();
            resizeSections();
         }
         break;
      }

      case QEvent::StyleChange:
         if (!d->customDefaultSectionSize) {
            d->updateDefaultSectionSizeFromStyle();
         }
         break;

      default:
         break;
   }

   return QAbstractItemView::event(e);
}

/*!
  \reimp
*/

void QHeaderView::paintEvent(QPaintEvent *e)
{
   Q_D(QHeaderView);

   if (count() == 0) {
      return;
   }

   QPainter painter(d->viewport);
   const QPoint offset = d->scrollDelayOffset;
   QRect translatedEventRect = e->rect();
   translatedEventRect.translate(offset);

   int start = -1;
   int end = -1;
   if (d->orientation == Qt::Horizontal) {
      start = visualIndexAt(translatedEventRect.left());
      end = visualIndexAt(translatedEventRect.right());
   } else {
      start = visualIndexAt(translatedEventRect.top());
      end = visualIndexAt(translatedEventRect.bottom());
   }

   if (d->reverse()) {
      start = (start == -1 ? count() - 1 : start);
      end = (end == -1 ? 0 : end);
   } else {
      start = (start == -1 ? 0 : start);
      end = (end == -1 ? count() - 1 : end);
   }

   int tmp = start;
   start = qMin(start, end);
   end = qMax(tmp, end);

   d->prepareSectionSelected(); // clear and resize the bit array

   QRect currentSectionRect;
   int logical;
   const int width = d->viewport->width();
   const int height = d->viewport->height();
   for (int i = start; i <= end; ++i) {
      if (d->isVisualIndexHidden(i)) {
         continue;
      }
      painter.save();
      logical = logicalIndex(i);
      if (d->orientation == Qt::Horizontal) {
         currentSectionRect.setRect(sectionViewportPosition(logical), 0, sectionSize(logical), height);
      } else {
         currentSectionRect.setRect(0, sectionViewportPosition(logical), width, sectionSize(logical));
      }
      currentSectionRect.translate(offset);

      QVariant variant = d->model->headerData(logical, d->orientation, Qt::FontRole);

      if (variant.isValid() && variant.canConvert<QFont>()) {
         QFont sectionFont = qvariant_cast<QFont>(variant);
         painter.setFont(sectionFont);
      }

      paintSection(&painter, currentSectionRect, logical);
      painter.restore();
   }

   QStyleOption opt;
   opt.init(this);
   // Paint the area beyond where there are indexes
   if (d->reverse()) {
      opt.state |= QStyle::State_Horizontal;
      if (currentSectionRect.left() > translatedEventRect.left()) {
         opt.rect = QRect(translatedEventRect.left(), 0,
               currentSectionRect.left() - translatedEventRect.left(), height);
         style()->drawControl(QStyle::CE_HeaderEmptyArea, &opt, &painter, this);
      }
   } else if (currentSectionRect.right() < translatedEventRect.right()) {
      // paint to the right
      opt.state |= QStyle::State_Horizontal;
      opt.rect = QRect(currentSectionRect.right() + 1, 0,
            translatedEventRect.right() - currentSectionRect.right(), height);
      style()->drawControl(QStyle::CE_HeaderEmptyArea, &opt, &painter, this);
   } else if (currentSectionRect.bottom() < translatedEventRect.bottom()) {
      // paint the bottom section
      opt.state &= ~QStyle::State_Horizontal;
      opt.rect = QRect(0, currentSectionRect.bottom() + 1,
            width, height - currentSectionRect.bottom() - 1);
      style()->drawControl(QStyle::CE_HeaderEmptyArea, &opt, &painter, this);
   }


}

/*!
  \reimp
*/

void QHeaderView::mousePressEvent(QMouseEvent *e)
{
   Q_D(QHeaderView);
   if (d->state != QHeaderViewPrivate::NoState || e->button() != Qt::LeftButton) {
      return;
   }

   int pos = d->orientation == Qt::Horizontal ? e->x() : e->y();
   int handle = d->sectionHandleAt(pos);
   d->originalSize = -1; // clear the stored original size
   if (handle == -1) {
      d->pressed = logicalIndexAt(pos);
      if (d->clickableSections) {
         emit sectionPressed(d->pressed);
      }
      bool acceptMoveSection = d->movableSections;
      if (acceptMoveSection && d->pressed == 0 && !d->allowUserMoveOfSection0) {
         acceptMoveSection = false;   // Do not allow moving the tree nod
      }
      if (acceptMoveSection) {
         d->section = d->target = d->pressed;
         if (d->section == -1) {
            return;
         }
         d->state = QHeaderViewPrivate::MoveSection;
         d->setupSectionIndicator(d->section, pos);
      } else if (d->clickableSections && d->pressed != -1) {
         updateSection(d->pressed);
         d->state = QHeaderViewPrivate::SelectSections;
      }
   } else if (sectionResizeMode(handle) == Interactive) {
      d->originalSize = sectionSize(handle);
      d->state = QHeaderViewPrivate::ResizeSection;
      d->section = handle;
      d->preventCursorChangeInSetOffset = false;
   }

   d->firstPos = pos;
   d->lastPos = pos;

   d->clearCascadingSections();
}

/*!
  \reimp
*/

void QHeaderView::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QHeaderView);
   int pos = d->orientation == Qt::Horizontal ? e->x() : e->y();
   if (pos < 0 && d->state != QHeaderViewPrivate::SelectSections) {
      return;
   }

   if (e->buttons() == Qt::NoButton) {

      // Under Cocoa, when the mouse button is released, may include an extra
      // simulated mouse moved event. The state of the buttons when this event
      // is generated is already "no button" and the code below gets executed
      // just before the mouseReleaseEvent and resets the state. This prevents
      // column dragging from working. So this code is disabled under Cocoa.

      d->state = QHeaderViewPrivate::NoState;
      d->pressed = -1;
   }

   switch (d->state) {
      case QHeaderViewPrivate::ResizeSection: {
         Q_ASSERT(d->originalSize != -1);
         if (d->cascadingResizing) {
            int delta = d->reverse() ? d->lastPos - pos : pos - d->lastPos;
            int visual = visualIndex(d->section);
            d->cascadingResize(visual, d->headerSectionSize(visual) + delta);
         } else {
            int delta = d->reverse() ? d->firstPos - pos : pos - d->firstPos;
            int newsize = qBound(minimumSectionSize(), d->originalSize + delta, maximumSectionSize());
            resizeSection(d->section, newsize);
         }
         d->lastPos = pos;
         return;
      }
      case QHeaderViewPrivate::MoveSection: {
         if (d->shouldAutoScroll(e->pos())) {
            d->startAutoScroll();
         }

         if (qAbs(pos - d->firstPos) >= QApplication::startDragDistance()
            || !d->sectionIndicator->isHidden()) {
            int visual = visualIndexAt(pos);

            if (visual == -1) {
               return;
            }

            if (visual == 0 && logicalIndex(0) == 0 && !d->allowUserMoveOfSection0) {
               return;
            }

            int posThreshold = d->headerSectionPosition(visual) - d->offset + d->headerSectionSize(visual) / 2;
            int moving = visualIndex(d->section);

            if (visual < moving) {
               if (pos < posThreshold) {
                  d->target = d->logicalIndex(visual);
               } else {
                  d->target = d->logicalIndex(visual + 1);
               }
            } else if (visual > moving) {
               if (pos > posThreshold) {
                  d->target = d->logicalIndex(visual);
               } else {
                  d->target = d->logicalIndex(visual - 1);
               }

            } else {
               d->target = d->section;
            }
            d->updateSectionIndicator(d->section, pos);
         }
         return;
      }

      case QHeaderViewPrivate::SelectSections: {
         int logical = logicalIndexAt(qMax(-d->offset, pos));
         if (logical == -1 && pos > 0) {
            logical = logicalIndex(d->lastVisibleVisualIndex());
         }

         if (logical == d->pressed) {
            return;   // nothing to do
         } else if (d->pressed != -1) {
            updateSection(d->pressed);
         }
         d->pressed = logical;
         if (d->clickableSections && logical != -1) {
            emit sectionEntered(d->pressed);
            updateSection(d->pressed);
         }
         return;
      }

      case QHeaderViewPrivate::NoState: {

#ifndef QT_NO_CURSOR
         int handle = d->sectionHandleAt(pos);
         bool hasCursor = testAttribute(Qt::WA_SetCursor);
         if (handle != -1 && (sectionResizeMode(handle) == Interactive)) {
            if (!hasCursor) {
               setCursor(d->orientation == Qt::Horizontal ? Qt::SplitHCursor : Qt::SplitVCursor);
            }
         } else if (hasCursor) {
            unsetCursor();
         }
#endif
         return;
      }
      default:
         break;
   }
}

/*!
  \reimp
*/

void QHeaderView::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QHeaderView);
   int pos = d->orientation == Qt::Horizontal ? e->x() : e->y();
   switch (d->state) {
      case QHeaderViewPrivate::MoveSection:
         if (! d->sectionIndicator->isHidden()) {
            // moving
            int from = visualIndex(d->section);
            Q_ASSERT(from != -1);

            int to = visualIndex(d->target);
            Q_ASSERT(to != -1);

            moveSection(from, to);
            d->section = d->target = -1;
            d->updateSectionIndicator(d->section, pos);
            break;
         }

         [[fallthrough]];

      case QHeaderViewPrivate::SelectSections:
         if (!d->clickableSections) {
            int section = logicalIndexAt(pos);
            updateSection(section);
         }

         [[fallthrough]];

      case QHeaderViewPrivate::NoState:
         if (d->clickableSections) {
            int section = logicalIndexAt(pos);
            if (section != -1 && section == d->pressed) {
               d->flipSortIndicator(section);
               emit sectionClicked(section);
            }
            if (d->pressed != -1) {
               updateSection(d->pressed);
            }
         }
         break;
      case QHeaderViewPrivate::ResizeSection:
         d->originalSize = -1;
         d->clearCascadingSections();
         break;
      default:
         break;
   }
   d->state = QHeaderViewPrivate::NoState;
   d->pressed = -1;
}

/*!
  \reimp
*/

void QHeaderView::mouseDoubleClickEvent(QMouseEvent *e)
{
   Q_D(QHeaderView);
   int pos = d->orientation == Qt::Horizontal ? e->x() : e->y();
   int handle = d->sectionHandleAt(pos);
   if (handle > -1 && sectionResizeMode(handle) == Interactive) {
      emit sectionHandleDoubleClicked(handle);
#ifndef QT_NO_CURSOR
      Qt::CursorShape splitCursor = (d->orientation == Qt::Horizontal)
         ? Qt::SplitHCursor : Qt::SplitVCursor;
      if (cursor().shape() == splitCursor) {
         // signal handlers may have changed the section size
         handle = d->sectionHandleAt(pos);
         if (!(handle > -1 && sectionResizeMode(handle) == Interactive)) {
            setCursor(Qt::ArrowCursor);
         }
      }
#endif
   } else {
      emit sectionDoubleClicked(logicalIndexAt(e->pos()));
   }
}

/*!
  \reimp
*/

bool QHeaderView::viewportEvent(QEvent *e)
{
   Q_D(QHeaderView);
   switch (e->type()) {

#ifndef QT_NO_TOOLTIP
      case QEvent::ToolTip: {
         QHelpEvent *he = static_cast<QHelpEvent *>(e);
         int logical = logicalIndexAt(he->pos());
         if (logical != -1) {
            QVariant variant = d->model->headerData(logical, d->orientation, Qt::ToolTipRole);
            if (variant.isValid()) {
               QToolTip::showText(he->globalPos(), variant.toString(), this);
               return true;
            }
         }
         break;
      }
#endif

#ifndef QT_NO_WHATSTHIS
      case QEvent::QueryWhatsThis: {
         QHelpEvent *he = static_cast<QHelpEvent *>(e);
         int logical = logicalIndexAt(he->pos());
         if (logical != -1
            && d->model->headerData(logical, d->orientation, Qt::WhatsThisRole).isValid()) {
            return true;
         }
         break;
      }

      case QEvent::WhatsThis: {
         QHelpEvent *he = static_cast<QHelpEvent *>(e);
         int logical = logicalIndexAt(he->pos());
         if (logical != -1) {
            QVariant whatsthis = d->model->headerData(logical, d->orientation,
                  Qt::WhatsThisRole);
            if (whatsthis.isValid()) {
               QWhatsThis::showText(he->globalPos(), whatsthis.toString(), this);
               return true;
            }
         }
         break;
      }
#endif

#ifndef QT_NO_STATUSTIP
      case QEvent::StatusTip: {
         QHelpEvent *he = static_cast<QHelpEvent *>(e);
         int logical = logicalIndexAt(he->pos());
         if (logical != -1) {
            QString statustip = d->model->headerData(logical, d->orientation, Qt::StatusTipRole).toString();
            if (!statustip.isEmpty()) {
               setStatusTip(statustip);
            }
         }
         return true;
      }
#endif // QT_NO_STATUSTIP

      case QEvent::FontChange:
      case QEvent::StyleChange:
         d->invalidateCachedSizeHint();
      // Fall through

      case QEvent::Hide:
      case QEvent::Show: {
         QAbstractScrollArea *parent = qobject_cast<QAbstractScrollArea *>(parentWidget());
         if (parent && parent->isVisible()) { // Only resize if we have a visible parent
            resizeSections();
         }
         emit geometriesChanged();
         break;
      }

      case QEvent::ContextMenu: {
         d->state = QHeaderViewPrivate::NoState;
         d->pressed = d->section = d->target = -1;
         d->updateSectionIndicator(d->section, -1);
         break;
      }

      case QEvent::Wheel: {
         QAbstractScrollArea *asa = qobject_cast<QAbstractScrollArea *>(parentWidget());
         if (asa) {
            return QApplication::sendEvent(asa->viewport(), e);
         }
         break;
      }

      default:
         break;
   }
   return QAbstractItemView::viewportEvent(e);
}

/*!
    Paints the section specified by the given \a logicalIndex, using the given
    \a painter and \a rect.

    Normally, you do not have to call this function.
*/

void QHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
   Q_D(const QHeaderView);
   if (!rect.isValid()) {
      return;
   }
   // get the state of the section
   QStyleOptionHeader opt;
   initStyleOption(&opt);
   QStyle::State state = QStyle::State_None;
   if (isEnabled()) {
      state |= QStyle::State_Enabled;
   }
   if (window()->isActiveWindow()) {
      state |= QStyle::State_Active;
   }
   if (d->clickableSections) {
      if (logicalIndex == d->hover) {
         state |= QStyle::State_MouseOver;
      }
      if (logicalIndex == d->pressed) {
         state |= QStyle::State_Sunken;
      } else if (d->highlightSelected) {
         if (d->sectionIntersectsSelection(logicalIndex)) {
            state |= QStyle::State_On;
         }
         if (d->isSectionSelected(logicalIndex)) {
            state |= QStyle::State_Sunken;
         }
      }

   }
   if (isSortIndicatorShown() && sortIndicatorSection() == logicalIndex)
      opt.sortIndicator = (sortIndicatorOrder() == Qt::AscendingOrder)
         ? QStyleOptionHeader::SortDown : QStyleOptionHeader::SortUp;

   // setup the style options structure
   QVariant textAlignment = d->model->headerData(logicalIndex, d->orientation, Qt::TextAlignmentRole);
   opt.rect    = rect;
   opt.section = logicalIndex;
   opt.state  |= state;

   opt.textAlignment = Qt::Alignment(textAlignment.isValid()
         ? Qt::Alignment(textAlignment.toInt())
         : d->defaultAlignment);

   opt.iconAlignment = Qt::AlignVCenter;
   opt.text = d->model->headerData(logicalIndex, d->orientation, Qt::DisplayRole).toString();

   int margin = 2 * style()->pixelMetric(QStyle::PM_HeaderMargin, 0, this);

   const Qt::Alignment headerArrowAlignment = static_cast<Qt::Alignment>(style()->styleHint(QStyle::SH_Header_ArrowAlignment, 0, this));
   const bool isHeaderArrowOnTheSide = headerArrowAlignment & Qt::AlignVCenter;
   if (isSortIndicatorShown() && sortIndicatorSection() == logicalIndex && isHeaderArrowOnTheSide) {
      margin += style()->pixelMetric(QStyle::PM_HeaderMarkSize, 0, this);
   }
   if (d->textElideMode != Qt::ElideNone) {
      opt.text = opt.fontMetrics.elidedText(opt.text, d->textElideMode, rect.width() - margin);
   }

   QVariant variant = d->model->headerData(logicalIndex, d->orientation, Qt::DecorationRole);
   opt.icon = qvariant_cast<QIcon>(variant);

   if (opt.icon.isNull()) {
      opt.icon = qvariant_cast<QPixmap>(variant);
   }
   QVariant foregroundBrush = d->model->headerData(logicalIndex, d->orientation,
         Qt::ForegroundRole);
   if (foregroundBrush.canConvert<QBrush>()) {
      opt.palette.setBrush(QPalette::ButtonText, qvariant_cast<QBrush>(foregroundBrush));
   }

   QPointF oldBO = painter->brushOrigin();
   QVariant backgroundBrush = d->model->headerData(logicalIndex, d->orientation,
         Qt::BackgroundRole);
   if (backgroundBrush.canConvert<QBrush>()) {
      opt.palette.setBrush(QPalette::Button, qvariant_cast<QBrush>(backgroundBrush));
      opt.palette.setBrush(QPalette::Window, qvariant_cast<QBrush>(backgroundBrush));
      painter->setBrushOrigin(opt.rect.topLeft());
   }

   // the section position
   int visual = visualIndex(logicalIndex);
   Q_ASSERT(visual != -1);
   bool first = d->isFirstVisibleSection(visual);
   bool last = d->isLastVisibleSection(visual);
   if (first && last) {
      opt.position = QStyleOptionHeader::OnlyOneSection;
   } else if (first) {
      opt.position = QStyleOptionHeader::Beginning;
   } else if (last) {
      opt.position = QStyleOptionHeader::End;
   } else {
      opt.position = QStyleOptionHeader::Middle;
   }

   opt.orientation = d->orientation;

   // the selected position
   bool previousSelected = d->isSectionSelected(this->logicalIndex(visual - 1));
   bool nextSelected =  d->isSectionSelected(this->logicalIndex(visual + 1));

   if (previousSelected && nextSelected) {
      opt.selectedPosition = QStyleOptionHeader::NextAndPreviousAreSelected;
   } else if (previousSelected) {
      opt.selectedPosition = QStyleOptionHeader::PreviousIsSelected;
   } else if (nextSelected) {
      opt.selectedPosition = QStyleOptionHeader::NextIsSelected;
   } else {
      opt.selectedPosition = QStyleOptionHeader::NotAdjacent;
   }

   // draw the section
   style()->drawControl(QStyle::CE_Header, &opt, painter, this);

   painter->setBrushOrigin(oldBO);
}

/*!
    Returns the size of the contents of the section specified by the given
    \a logicalIndex.

    \sa defaultSectionSize()
*/

QSize QHeaderView::sectionSizeFromContents(int logicalIndex) const
{
   Q_D(const QHeaderView);
   Q_ASSERT(logicalIndex >= 0);

   ensurePolished();

   // use SizeHintRole
   QVariant variant = d->model->headerData(logicalIndex, d->orientation, Qt::SizeHintRole);
   if (variant.isValid()) {
      return qvariant_cast<QSize>(variant);
   }

   // otherwise use the contents
   QStyleOptionHeader opt;
   initStyleOption(&opt);
   opt.section = logicalIndex;
   QVariant var = d->model->headerData(logicalIndex, d->orientation, Qt::FontRole);
   QFont fnt;

   if (var.isValid() && var.canConvert<QFont>()) {
      fnt = qvariant_cast<QFont>(var);
   } else {
      fnt = font();
   }
   fnt.setBold(true);
   opt.fontMetrics = QFontMetrics(fnt);
   opt.text = d->model->headerData(logicalIndex, d->orientation,
         Qt::DisplayRole).toString();
   variant = d->model->headerData(logicalIndex, d->orientation, Qt::DecorationRole);
   opt.icon = qvariant_cast<QIcon>(variant);
   if (opt.icon.isNull()) {
      opt.icon = qvariant_cast<QPixmap>(variant);
   }

   if (isSortIndicatorShown()) {
      opt.sortIndicator = QStyleOptionHeader::SortDown;
   }

   return style()->sizeFromContents(QStyle::CT_HeaderSection, &opt, QSize(), this);
}



int QHeaderView::horizontalOffset() const
{
   Q_D(const QHeaderView);
   if (d->orientation == Qt::Horizontal) {
      return d->offset;
   }
   return 0;
}

/*!
    Returns the vertical offset of the header. This is 0 for horizontal
    headers.

    \sa offset()
*/

int QHeaderView::verticalOffset() const
{
   Q_D(const QHeaderView);
   if (d->orientation == Qt::Vertical) {
      return d->offset;
   }
   return 0;
}

/*!
    \reimp
    \internal
*/

void QHeaderView::updateGeometries()
{
   Q_D(QHeaderView);
   d->layoutChildren();
   if (d->hasAutoResizeSections()) {
      d->doDelayedResizeSections();
   }
}

/*!
    \reimp
    \internal
*/

void QHeaderView::scrollContentsBy(int dx, int dy)
{
   Q_D(QHeaderView);
   d->scrollDirtyRegion(dx, dy);
}

/*!
    \reimp
    \internal
*/
void QHeaderView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
   Q_D(QHeaderView);

   d->invalidateCachedSizeHint();

   if (d->hasAutoResizeSections()) {
      bool resizeRequired = d->globalResizeMode == ResizeToContents;
      int first = orientation() == Qt::Horizontal ? topLeft.column() : topLeft.row();
      int last = orientation() == Qt::Horizontal ? bottomRight.column() : bottomRight.row();

      for (int i = first; i <= last && !resizeRequired; ++i) {
         resizeRequired = (sectionResizeMode(i) == ResizeToContents);
      }

      if (resizeRequired) {
         d->doDelayedResizeSections();
      }
   }
}

/*!
    \reimp
    \internal

    Empty implementation because the header doesn't show QModelIndex items.
*/
void QHeaderView::rowsInserted(const QModelIndex &, int, int)
{
   // do nothing
}

/*!
    \reimp
    \internal

    Empty implementation because the header doesn't show QModelIndex items.
*/

QRect QHeaderView::visualRect(const QModelIndex &) const
{
   return QRect();
}

/*!
    \reimp
    \internal

    Empty implementation because the header doesn't show QModelIndex items.
*/

void QHeaderView::scrollTo(const QModelIndex &, ScrollHint)
{
   // do nothing - the header only displays sections
}

/*!
    \reimp
    \internal

    Empty implementation because the header doesn't show QModelIndex items.
*/

QModelIndex QHeaderView::indexAt(const QPoint &) const
{
   return QModelIndex();
}

/*!
    \reimp
    \internal

    Empty implementation because the header doesn't show QModelIndex items.
*/

bool QHeaderView::isIndexHidden(const QModelIndex &) const
{
   return true; // the header view has no items, just sections
}

/*!
    \reimp
    \internal

    Empty implementation because the header doesn't show QModelIndex items.
*/

QModelIndex QHeaderView::moveCursor(CursorAction, Qt::KeyboardModifiers)
{
   return QModelIndex();
}

/*!
    \reimp

    Selects the items in the given \a rect according to the specified
    \a flags.

    The base class implementation does nothing.
*/

void QHeaderView::setSelection(const QRect &, QItemSelectionModel::SelectionFlags)
{
   // do nothing
}

/*!
    \internal
*/

QRegion QHeaderView::visualRegionForSelection(const QItemSelection &selection) const
{
   Q_D(const QHeaderView);
   const int max = d->modelSectionCount();
   if (d->orientation == Qt::Horizontal) {
      int logicalLeft = max;
      int logicalRight = 0;

      if (d->visualIndices.empty()) {
         // If no reordered sections, skip redundant visual-to-logical transformations
         for (int i = 0; i < selection.count(); ++i) {
            const QItemSelectionRange &r = selection.at(i);
            if (r.parent().isValid() || !r.isValid()) {
               continue;   // we only know about toplevel items and we don't want invalid ranges
            }
            if (r.left() < logicalLeft) {
               logicalLeft = r.left();
            }
            if (r.right() > logicalRight) {
               logicalRight = r.right();
            }
         }
      } else {
         int left = max;
         int right = 0;
         for (int i = 0; i < selection.count(); ++i) {
            const QItemSelectionRange &r = selection.at(i);
            if (r.parent().isValid() || !r.isValid()) {
               continue;   // we only know about toplevel items and we don't want invalid ranges
            }
            for (int k = r.left(); k <= r.right(); ++k) {
               int visual = visualIndex(k);
               if (visual == -1) { // in some cases users may change the selections
                  continue;   // before we have a chance to do the layout
               }
               if (visual < left) {
                  left = visual;
               }
               if (visual > right) {
                  right = visual;
               }
            }
         }
         logicalLeft = logicalIndex(left);
         logicalRight = logicalIndex(right);
      }



      if (logicalLeft < 0  || logicalLeft >= count() ||
         logicalRight < 0 || logicalRight >= count()) {
         return QRegion();
      }

      int leftPos = sectionViewportPosition(logicalLeft);
      int rightPos = sectionViewportPosition(logicalRight);
      rightPos += sectionSize(logicalRight);
      return QRect(leftPos, 0, rightPos - leftPos, height());
   }

   // orientation() == Qt::Vertical
   int logicalTop = max;
   int logicalBottom = 0;

   if (d->visualIndices.empty()) {
      // If no reordered sections, skip redundant visual-to-logical transformations
      for (int i = 0; i < selection.count(); ++i) {
         const QItemSelectionRange &r = selection.at(i);
         if (r.parent().isValid() || !r.isValid()) {
            continue;   // we only know about toplevel items and we don't want invalid ranges
         }
         if (r.top() < logicalTop) {
            logicalTop = r.top();
         }
         if (r.bottom() > logicalBottom) {
            logicalBottom = r.bottom();
         }
      }
   } else {
      int top = max;
      int bottom = 0;

      for (int i = 0; i < selection.count(); ++i) {
         const QItemSelectionRange &r = selection.at(i);
         if (r.parent().isValid() || !r.isValid()) {
            continue;   // we only know about toplevel items and we don't want invalid ranges
         }
         for (int k = r.top(); k <= r.bottom(); ++k) {
            int visual = visualIndex(k);
            if (visual == -1) { // in some cases users may change the selections
               continue;   // before we have a chance to do the layout
            }
            if (visual < top) {
               top = visual;
            }
            if (visual > bottom) {
               bottom = visual;
            }
         }
      }

      logicalTop = logicalIndex(top);
      logicalBottom = logicalIndex(bottom);
   }


   if (logicalTop < 0 || logicalTop >= count() || logicalBottom < 0 || logicalBottom >= count()) {
      return QRegion();
   }

   int topPos = sectionViewportPosition(logicalTop);
   int bottomPos = sectionViewportPosition(logicalBottom) + sectionSize(logicalBottom);

   return QRect(0, topPos, width(), bottomPos - topPos);
}


// private implementation

int QHeaderViewPrivate::sectionHandleAt(int position)
{
   Q_Q(QHeaderView);
   int visual = q->visualIndexAt(position);
   if (visual == -1) {
      return -1;
   }

   int log = logicalIndex(visual);
   int pos = q->sectionViewportPosition(log);
   int grip = q->style()->pixelMetric(QStyle::PM_HeaderGripMargin, 0, q);

   bool atLeft = position < pos + grip;
   bool atRight = (position > pos + q->sectionSize(log) - grip);
   if (reverse()) {
      qSwap(atLeft, atRight);
   }

   if (atLeft) {
      //grip at the beginning of the section
      while (visual > -1) {
         int logical = q->logicalIndex(--visual);
         if (!q->isSectionHidden(logical)) {
            return logical;
         }
      }
   } else if (atRight) {
      //grip at the end of the section
      return log;
   }
   return -1;
}

void QHeaderViewPrivate::setupSectionIndicator(int section, int position)
{
   Q_Q(QHeaderView);
   if (!sectionIndicator) {
      sectionIndicator = new QLabel(viewport);
   }

   int w, h;
   int p = q->sectionViewportPosition(section);
   if (orientation == Qt::Horizontal) {
      w = q->sectionSize(section);
      h = viewport->height();
   } else {
      w = viewport->width();
      h = q->sectionSize(section);
   }
   sectionIndicator->resize(w, h);

   QPixmap pm(w, h);
   pm.fill(QColor(0, 0, 0, 45));
   QRect rect(0, 0, w, h);

   QPainter painter(&pm);
   painter.setOpacity(0.75);
   q->paintSection(&painter, rect, section);
   painter.end();

   sectionIndicator->setPixmap(pm);
   sectionIndicatorOffset = position - qMax(p, 0);
}

void QHeaderViewPrivate::updateSectionIndicator(int section, int position)
{
   if (!sectionIndicator) {
      return;
   }

   if (section == -1 || target == -1) {
      sectionIndicator->hide();
      return;
   }

   if (orientation == Qt::Horizontal) {
      sectionIndicator->move(position - sectionIndicatorOffset, 0);
   } else {
      sectionIndicator->move(0, position - sectionIndicatorOffset);
   }

   sectionIndicator->show();
}


void QHeaderView::initStyleOption(QStyleOptionHeader *option) const
{
   Q_D(const QHeaderView);
   option->initFrom(this);
   option->state = QStyle::State_None | QStyle::State_Raised;
   option->orientation = d->orientation;
   if (d->orientation == Qt::Horizontal) {
      option->state |= QStyle::State_Horizontal;
   }

   if (isEnabled()) {
      option->state |= QStyle::State_Enabled;
   }
   option->section = 0;
}

bool QHeaderViewPrivate::isSectionSelected(int section) const
{
   int i = section * 2;
   if (i < 0 || i >= sectionSelected.count()) {
      return false;
   }
   if (sectionSelected.testBit(i)) { // if the value was cached
      return sectionSelected.testBit(i + 1);
   }
   bool s = false;
   if (orientation == Qt::Horizontal) {
      s = isColumnSelected(section);
   } else {
      s = isRowSelected(section);
   }
   sectionSelected.setBit(i + 1, s); // selection state
   sectionSelected.setBit(i, true); // cache state
   return s;
}

bool QHeaderViewPrivate::isFirstVisibleSection(int section) const
{
   if (sectionStartposRecalc) {
      recalcSectionStartPos();
   }
   const SectionItem &item = sectionItems.at(section);
   return item.size > 0 && item.calculated_startpos == 0;
}
bool QHeaderViewPrivate::isLastVisibleSection(int section) const
{
   if (sectionStartposRecalc) {
      recalcSectionStartPos();
   }
   const SectionItem &item = sectionItems.at(section);
   return item.size > 0 && item.calculatedEndPos() == length;
}
/*!
    \internal
    Returns the last visible (ie. not hidden) visual index
*/
int QHeaderViewPrivate::lastVisibleVisualIndex() const
{
   Q_Q(const QHeaderView);
   for (int visual = q->count() - 1; visual >= 0; --visual) {
      if (!q->isSectionHidden(q->logicalIndex(visual))) {
         return visual;
      }
   }

   //default value if no section is actually visible
   return -1;
}

/*!
    \internal
    Go through and resize all of the sections applying stretchLastSection,
    manualy stretches, sizes, and useGlobalMode.

    The different resize modes are:
    Interactive - the user decides the size
    Stretch - take up whatever space is left
    Fixed - the size is set programmatically outside the header
    ResizeToContentes - the size is set based on the contents of the row or column in the parent view

    The resize mode will not affect the last section if stretchLastSection is true.
*/
void QHeaderViewPrivate::resizeSections(QHeaderView::ResizeMode globalMode, bool useGlobalMode)
{
   Q_Q(QHeaderView);
   //stop the timer in case it is delayed
   delayedResize.stop();

   executePostedLayout();
   if (sectionCount() == 0) {
      return;
   }

   if (resizeRecursionBlock) {
      return;
   }
   resizeRecursionBlock = true;

   invalidateCachedSizeHint();

   const int lastVisibleSection = lastVisibleVisualIndex();

   // find stretchLastSection if we have it
   int stretchSection = -1;
   if (stretchLastSection && !useGlobalMode) {
      stretchSection = lastVisibleVisualIndex();
   }

   // count up the number of strected sections and how much space left for them
   int lengthToStretch = (orientation == Qt::Horizontal ? viewport->width() : viewport->height());
   int numberOfStretchedSections = 0;
   QList<int> section_sizes;

   for (int i = 0; i < sectionCount(); ++i) {
      if (isVisualIndexHidden(i)) {
         continue;
      }

      QHeaderView::ResizeMode resizeMode;
      if (useGlobalMode && (i != stretchSection)) {
         resizeMode = globalMode;
      } else {
         resizeMode = (i == stretchSection ? QHeaderView::Stretch : headerSectionResizeMode(i));
      }

      if (resizeMode == QHeaderView::Stretch) {
         ++numberOfStretchedSections;
         section_sizes.append(headerSectionSize(i));
         continue;
      }

      // because it isn't stretch, determine its width and remove that from lengthToStrech
      int sectionSize = 0;
      if (resizeMode == QHeaderView::Interactive || resizeMode == QHeaderView::Fixed) {
         sectionSize = headerSectionSize(i);
      } else { // resizeMode == QHeaderView::ResizeToContents
         int logicalIndex = q->logicalIndex(i);
         sectionSize = qMax(viewSectionSizeHint(logicalIndex),
               q->sectionSizeHint(logicalIndex));
         if (sectionSize > q->maximumSectionSize()) {
            sectionSize = q->maximumSectionSize();
         }
      }
      section_sizes.append(sectionSize);
      lengthToStretch -= sectionSize;
   }

   // calculate the new length for all of the stretched sections
   int stretchSectionLength = -1;
   int pixelReminder = 0;

   if (numberOfStretchedSections > 0 && lengthToStretch > 0) {
      // we have room to stretch in
      int hintLengthForEveryStretchedSection = lengthToStretch / numberOfStretchedSections;
      stretchSectionLength = qMax(hintLengthForEveryStretchedSection, q->minimumSectionSize());
      pixelReminder = lengthToStretch % numberOfStretchedSections;
   }

   int spanStartSection = 0;
   int previousSectionLength = 0;

   QHeaderView::ResizeMode previousSectionResizeMode = QHeaderView::Interactive;

   // resize each section along the total length
   for (int i = 0; i < sectionCount(); ++i) {
      int oldSectionLength = headerSectionSize(i);
      int newSectionLength = -1;
      QHeaderView::ResizeMode newSectionResizeMode = headerSectionResizeMode(i);

      if (isVisualIndexHidden(i)) {
         newSectionLength = 0;
      } else {
         QHeaderView::ResizeMode resizeMode;
         if (useGlobalMode) {
            resizeMode = globalMode;
         } else
            resizeMode = (i == stretchSection
                  ? QHeaderView::Stretch
                  : newSectionResizeMode);
         if (resizeMode == QHeaderView::Stretch && stretchSectionLength != -1) {
            if (i == lastVisibleSection) {
               newSectionLength = qMax(stretchSectionLength, lastSectionSize);
            } else {
               newSectionLength = stretchSectionLength;
            }
            if (pixelReminder > 0) {
               newSectionLength += 1;
               --pixelReminder;
            }
            section_sizes.removeFirst();
         } else {
            newSectionLength = section_sizes.front();
            section_sizes.removeFirst();
         }
      }

      //Q_ASSERT(newSectionLength > 0);
      if ((previousSectionResizeMode != newSectionResizeMode
            || previousSectionLength != newSectionLength) && i > 0) {
         int spanLength = (i - spanStartSection) * previousSectionLength;
         createSectionItems(spanStartSection, i - 1, spanLength, previousSectionResizeMode);
         //Q_ASSERT(headerLength() == length);
         spanStartSection = i;
      }

      if (newSectionLength != oldSectionLength) {
         emit q->sectionResized(logicalIndex(i), oldSectionLength, newSectionLength);
      }

      previousSectionLength = newSectionLength;
      previousSectionResizeMode = newSectionResizeMode;
   }

   createSectionItems(spanStartSection, sectionCount() - 1,
      (sectionCount() - spanStartSection) * previousSectionLength,
      previousSectionResizeMode);
   //Q_ASSERT(headerLength() == length);
   resizeRecursionBlock = false;
   viewport->update();
}

void QHeaderViewPrivate::createSectionItems(int start, int end, int size, QHeaderView::ResizeMode mode)
{
   int sizePerSection = size / (end - start + 1);
   if (end >= sectionItems.count()) {
      sectionItems.resize(end + 1);
      sectionStartposRecalc = true;
   }

   SectionItem *sectiondata = sectionItems.data();
   for (int i = start; i <= end; ++i) {
      length += (sizePerSection - sectiondata[i].size);
      sectionStartposRecalc |= (sectiondata[i].size != sizePerSection);
      sectiondata[i].size = sizePerSection;
      sectiondata[i].resizeMode = mode;
   }
}

void QHeaderViewPrivate::removeSectionsFromSectionItems(int start, int end)
{
   // remove sections
   sectionStartposRecalc |= (end != sectionItems.count() - 1);
   int removedlength = 0;
   for (int u = start; u <= end; ++u) {
      removedlength += sectionItems.at(u).size;
   }
   length -= removedlength;
   sectionItems.remove(start, end - start + 1);
}

void QHeaderViewPrivate::clear()
{
   if (state != NoClear) {
      length = 0;
      visualIndices.clear();
      logicalIndices.clear();
      sectionSelected.clear();
      hiddenSectionSize.clear();
      sectionItems.clear();
      invalidateCachedSizeHint();
   }
}

void QHeaderViewPrivate::flipSortIndicator(int section)
{
   Q_Q(QHeaderView);

   Qt::SortOrder sortOrder;

   if (sortIndicatorSection == section) {
      sortOrder = (sortIndicatorOrder == Qt::DescendingOrder) ? Qt::AscendingOrder : Qt::DescendingOrder;

   } else {
      const QVariant value = model->headerData(section, orientation, Qt::InitialSortOrderRole);

      if (value.canConvert(QVariant::Int)) {
         sortOrder = static_cast<Qt::SortOrder>(value.toInt());
      } else {
         sortOrder = Qt::AscendingOrder;
      }
   }

   q->setSortIndicator(section, sortOrder);
}

void QHeaderViewPrivate::cascadingResize(int visual, int newSize)
{
   Q_Q(QHeaderView);
   const int minimumSize = q->minimumSectionSize();
   const int oldSize = headerSectionSize(visual);
   int delta = newSize - oldSize;

   if (delta > 0) { // larger
      bool sectionResized = false;

      // restore old section sizes
      for (int i = firstCascadingSection; i < visual; ++i) {
         if (cascadingSectionSize.contains(i)) {
            int currentSectionSize = headerSectionSize(i);
            int originalSectionSize = cascadingSectionSize.value(i);

            if (currentSectionSize < originalSectionSize) {
               int newSectionSize = currentSectionSize + delta;
               resizeSectionItem(i, currentSectionSize, newSectionSize);

               if (newSectionSize >= originalSectionSize && false) {
                  cascadingSectionSize.remove(i);   // the section is now restored
               }

               sectionResized = true;
               break;
            }
         }

      }

      // resize the section
      if (! sectionResized) {
         newSize = qMax(newSize, minimumSize);

         if (oldSize != newSize) {
            resizeSectionItem(visual, oldSize, newSize);
         }
      }

      // cascade the section size change
      for (int i = visual + 1; i < sectionCount(); ++i) {
         if (!sectionIsCascadable(i)) {
            continue;
         }
         int currentSectionSize = headerSectionSize(i);
         if (currentSectionSize <= minimumSize) {
            continue;
         }

         int newSectionSize = qMax(currentSectionSize - delta, minimumSize);
         //qDebug() << "### cascading to" << i << newSectionSize - currentSectionSize << delta;

         resizeSectionItem(i, currentSectionSize, newSectionSize);
         saveCascadingSectionSize(i, currentSectionSize);
         delta = delta - (currentSectionSize - newSectionSize);

         //qDebug() << "new delta" << delta;
         //if (newSectionSize != minimumSize)

         if (delta <= 0) {
            break;
         }
      }

   } else { // smaller
      bool sectionResized = false;

      // restore old section sizes
      for (int i = lastCascadingSection; i > visual; --i) {
         if (!cascadingSectionSize.contains(i)) {
            continue;
         }

         int currentSectionSize = headerSectionSize(i);
         int originalSectionSize = cascadingSectionSize.value(i);
         if (currentSectionSize >= originalSectionSize) {
            continue;
         }

         int newSectionSize = currentSectionSize - delta;
         resizeSectionItem(i, currentSectionSize, newSectionSize);
         if (newSectionSize >= originalSectionSize && false) {
            //qDebug() << "section" << i << "restored to" << originalSectionSize;
            cascadingSectionSize.remove(i); // the section is now restored
         }
         sectionResized = true;
         break;
      }

      // resize the section
      resizeSectionItem(visual, oldSize, qMax(newSize, minimumSize));

      // cascade the section size change
      if (delta < 0 && newSize < minimumSize) {
         for (int i = visual - 1; i >= 0; --i) {
            if (!sectionIsCascadable(i)) {
               continue;
            }

            int sectionSize = headerSectionSize(i);
            if (sectionSize <= minimumSize) {
               continue;
            }

            resizeSectionItem(i, sectionSize, qMax(sectionSize + delta, minimumSize));
            saveCascadingSectionSize(i, sectionSize);
            break;
         }
      }

      // let the next section get the space from the resized section
      if (!sectionResized) {
         for (int i = visual + 1; i < sectionCount(); ++i) {
            if (!sectionIsCascadable(i)) {
               continue;
            }
            int currentSectionSize = headerSectionSize(i);
            int newSectionSize = qMax(currentSectionSize - delta, minimumSize);
            resizeSectionItem(i, currentSectionSize, newSectionSize);
            break;
         }
      }
   }

   if (hasAutoResizeSections()) {
      doDelayedResizeSections();
   }

   viewport->update();
}

void QHeaderViewPrivate::setDefaultSectionSize(int size)
{
   Q_Q(QHeaderView);
   executePostedLayout();
   invalidateCachedSizeHint();
   defaultSectionSize = size;
   customDefaultSectionSize = true;
   if (state == QHeaderViewPrivate::ResizeSection) {
      preventCursorChangeInSetOffset = true;
   }
   for (int i = 0; i < sectionItems.count(); ++i) {
      QHeaderViewPrivate::SectionItem &section = sectionItems[i];
      if (hiddenSectionSize.isEmpty() || !isVisualIndexHidden(i)) { // resize on not hidden.
         const int newSize = size;
         if (newSize != section.size) {
            length += newSize - section.size; //the whole length is changed
            const int oldSectionSize = section.sectionSize();
            section.size = size;
            emit q->sectionResized(logicalIndex(i), oldSectionSize, size);
         }
      }
   }
   sectionStartposRecalc = true;
   if (hasAutoResizeSections()) {
      doDelayedResizeSections();
   }
   viewport->update();
}
void QHeaderViewPrivate::updateDefaultSectionSizeFromStyle()
{
   Q_Q(QHeaderView);
   if (orientation == Qt::Horizontal) {
      defaultSectionSize = q->style()->pixelMetric(QStyle::PM_HeaderDefaultSectionSizeHorizontal, 0, q);
   } else {
      defaultSectionSize = qMax(q->minimumSectionSize(),
            q->style()->pixelMetric(QStyle::PM_HeaderDefaultSectionSizeVertical, 0, q));
   }
}

void QHeaderViewPrivate::recalcSectionStartPos() const // linear (but fast)
{
   int pixelpos = 0;
   for (QVector<SectionItem>::const_iterator i = sectionItems.constBegin(); i != sectionItems.constEnd(); ++i) {
      i->calculated_startpos = pixelpos; // write into const mutable
      pixelpos += i->size;
   }
   sectionStartposRecalc = false;
}
void QHeaderViewPrivate::resizeSectionItem(int visualIndex, int oldSize, int newSize)
{
   Q_Q(QHeaderView);
   QHeaderView::ResizeMode mode = headerSectionResizeMode(visualIndex);
   createSectionItems(visualIndex, visualIndex, newSize, mode);
   emit q->sectionResized(logicalIndex(visualIndex), oldSize, newSize);
}

int QHeaderViewPrivate::headerSectionSize(int visual) const
{
   if (visual < sectionCount() && visual >= 0) {
      return sectionItems.at(visual).sectionSize();
   }
   return -1;
}

int QHeaderViewPrivate::headerSectionPosition(int visual) const
{
   if (visual < sectionCount() && visual >= 0) {
      if (sectionStartposRecalc) {
         recalcSectionStartPos();
      }
      return sectionItems.at(visual).calculated_startpos;
   }
   return -1;
}

int QHeaderViewPrivate::headerVisualIndexAt(int position) const
{
   // ### silly iteration
   if (sectionStartposRecalc) {
      recalcSectionStartPos();
   }

   int startidx = 0;
   int endidx = sectionItems.count() - 1;

   while (startidx <= endidx) {
      int middle = (endidx + startidx) / 2;
      if (sectionItems.at(middle).calculated_startpos > position) {
         endidx = middle - 1;
      } else {
         if (sectionItems.at(middle).calculatedEndPos() <= position) {
            startidx = middle + 1;
         } else { // we found it.
            return middle;
         }
      }
   }
   return -1;
}

void QHeaderViewPrivate::setHeaderSectionResizeMode(int visual, QHeaderView::ResizeMode mode)
{
   int size = headerSectionSize(visual);
   createSectionItems(visual, visual, size, mode);
}

QHeaderView::ResizeMode QHeaderViewPrivate::headerSectionResizeMode(int visual) const
{
   if (visual < 0 || visual >= sectionItems.count()) {
      return globalResizeMode;
   }
   return static_cast<QHeaderView::ResizeMode>(sectionItems.at(visual).resizeMode);

}

void QHeaderViewPrivate::setGlobalHeaderResizeMode(QHeaderView::ResizeMode mode)
{
   globalResizeMode = mode;
   for (int i = 0; i < sectionItems.count(); ++i) {
      sectionItems[i].resizeMode = mode;
   }
}

int QHeaderViewPrivate::viewSectionSizeHint(int logical) const
{
   Q_Q(const QHeaderView);

   if (QAbstractItemView *view = qobject_cast<QAbstractItemView *>( q->parent() )) {
      return (orientation == Qt::Horizontal
            ? view->sizeHintForColumn(logical)
            : view->sizeHintForRow(logical));
   }
   return 0;
}

int QHeaderViewPrivate::adjustedVisualIndex(int visualIndex) const
{
   if (!hiddenSectionSize.isEmpty()) {
      int adjustedVisualIndex = visualIndex;
      int currentVisualIndex = 0;
      for (int i = 0; i < sectionItems.count(); ++i) {
         if (isVisualIndexHidden(i)) {
            ++adjustedVisualIndex;
         } else {
            ++currentVisualIndex;
         }
         if (currentVisualIndex >= visualIndex) {
            break;
         }
      }
      visualIndex = adjustedVisualIndex;
   }
   return visualIndex;
}
void QHeaderViewPrivate::setScrollOffset(const QScrollBar *scrollBar, QAbstractItemView::ScrollMode scrollMode)
{
   Q_Q(QHeaderView);
   if (scrollMode == QAbstractItemView::ScrollPerItem) {
      if (scrollBar->maximum() > 0 && scrollBar->value() == scrollBar->maximum()) {
         q->setOffsetToLastSection();
      } else {
         q->setOffsetToSectionPosition(scrollBar->value());
      }
   } else {
      q->setOffset(scrollBar->value());
   }
}


void QHeaderViewPrivate::write(QDataStream &out) const
{
   out << int(orientation);
   out << int(sortIndicatorOrder);
   out << sortIndicatorSection;
   out << sortIndicatorShown;

   out << visualIndices;
   out << logicalIndices;

   out << sectionsHiddenToBitVector();
   out << hiddenSectionSize;

   out << length;
   out << sectionCount();
   out << movableSections;
   out << clickableSections;
   out << highlightSelected;
   out << stretchLastSection;
   out << cascadingResizing;
   out << stretchSections;
   out << contentsSections;
   out << defaultSectionSize;
   out << minimumSectionSize;

   out << int(defaultAlignment);
   out << int(globalResizeMode);

   out << sectionItems;
   out << resizeContentsPrecision;
   out << customDefaultSectionSize;
}

bool QHeaderViewPrivate::read(QDataStream &in)
{
   int orient, order, align, global;
   int sortIndicatorSectionIn;
   bool sortIndicatorShownIn;
   int lengthIn;
   QVector<int> visualIndicesIn;
   QVector<int> logicalIndicesIn;
   QHash<int, int> hiddenSectionSizeIn;
   bool movableSectionsIn;
   bool clickableSectionsIn;
   bool highlightSelectedIn;
   bool stretchLastSectionIn;
   bool cascadingResizingIn;
   int stretchSectionsIn;
   int contentsSectionsIn;
   int defaultSectionSizeIn;
   int minimumSectionSizeIn;
   QVector<SectionItem> sectionItemsIn;
   in >> orient;

   in >> order;

   in >> sortIndicatorSectionIn;
   in >> sortIndicatorShownIn;

   in >> visualIndicesIn;
   in >> logicalIndicesIn;

   QBitArray sectionHidden;
   in >> sectionHidden;
   in >> hiddenSectionSizeIn;

   in >> lengthIn;
   int unusedSectionCount; // For compatibility
   in >> unusedSectionCount;
   if (in.status() != QDataStream::Ok || lengthIn < 0) {
      return false;
   }
   in >> movableSectionsIn;
   in >> clickableSectionsIn;
   in >> highlightSelectedIn;
   in >> stretchLastSectionIn;
   in >> cascadingResizingIn;
   in >> stretchSectionsIn;
   in >> contentsSectionsIn;
   in >> defaultSectionSizeIn;
   in >> minimumSectionSizeIn;

   in >> align;

   in >> global;

   in >> sectionItemsIn;
   // In Qt4 we had a vector of spans where one span could hold information on more sections.
   // Now we have an itemvector where one items contains information about one section
   // For backward compatibility with Qt4 we do the following
   QVector<SectionItem> newSectionItems;
   for (int u = 0; u < sectionItemsIn.count(); ++u) {
      int count = sectionItemsIn.at(u).tmpDataStreamSectionCount;

      if (count > 0) {
         sectionItemsIn[u].size /= count;
      }
      for (int n = 0; n < count; ++n) {
         newSectionItems.append(sectionItemsIn[u]);
      }
   }

   int sectionItemsLengthTotal = 0;
   for (const SectionItem &section : newSectionItems) {
      sectionItemsLengthTotal += section.size;
   }

   if (sectionItemsLengthTotal != lengthIn) {
      return false;
   }

   orientation = static_cast<Qt::Orientation>(orient);
   sortIndicatorOrder   = static_cast<Qt::SortOrder>(order);
   sortIndicatorSection = sortIndicatorSectionIn;
   sortIndicatorShown   = sortIndicatorShownIn;
   visualIndices        = visualIndicesIn;
   logicalIndices       = logicalIndicesIn;
   hiddenSectionSize    = hiddenSectionSizeIn;
   length               = lengthIn;
   movableSections      = movableSectionsIn;
   clickableSections    = clickableSectionsIn;
   highlightSelected    = highlightSelectedIn;
   stretchLastSection   = stretchLastSectionIn;
   cascadingResizing    = cascadingResizingIn;
   stretchSections      = stretchSectionsIn;
   contentsSections     = contentsSectionsIn;
   defaultSectionSize   = defaultSectionSizeIn;
   minimumSectionSize   = minimumSectionSizeIn;

   defaultAlignment = Qt::Alignment(align);
   globalResizeMode = static_cast<QHeaderView::ResizeMode>(global);

   sectionItems = newSectionItems;
   setHiddenSectionsFromBitVector(sectionHidden);
   recalcSectionStartPos();

   int tmpint;
   in >> tmpint;
   if (in.status() == QDataStream::Ok) { // we haven't read past end
      resizeContentsPrecision = tmpint;
   }

   bool tmpbool;
   in >> tmpbool;
   if (in.status() == QDataStream::Ok) {  // we haven't read past end
      customDefaultSectionSize = tmpbool;
      if (!customDefaultSectionSize) {
         updateDefaultSectionSizeFromStyle();
      }
   }

   return true;

}

void QHeaderView::_q_sectionsRemoved(const QModelIndex &parent, int logicalFirst, int logicalLast)
{
   Q_D(QHeaderView);
   d->_q_sectionsRemoved(parent, logicalFirst, logicalLast);
}

void QHeaderView::_q_layoutAboutToBeChanged()
{
   Q_D(QHeaderView);
   d->_q_layoutAboutToBeChanged();
}

#endif // QT_NO_ITEMVIEWS
