/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QHEADERVIEW_H
#define QHEADERVIEW_H

#include <QtGui/qabstractitemview.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ITEMVIEWS

class QHeaderViewPrivate;
class QStyleOptionHeader;

class Q_GUI_EXPORT QHeaderView : public QAbstractItemView
{
   GUI_CS_OBJECT(QHeaderView)

   GUI_CS_PROPERTY_READ(showSortIndicator, isSortIndicatorShown)
   GUI_CS_PROPERTY_WRITE(showSortIndicator, setSortIndicatorShown)
   GUI_CS_PROPERTY_READ(highlightSections, highlightSections)
   GUI_CS_PROPERTY_WRITE(highlightSections, setHighlightSections)
   GUI_CS_PROPERTY_READ(stretchLastSection, stretchLastSection)
   GUI_CS_PROPERTY_WRITE(stretchLastSection, setStretchLastSection)
   GUI_CS_PROPERTY_READ(cascadingSectionResizes, cascadingSectionResizes)
   GUI_CS_PROPERTY_WRITE(cascadingSectionResizes, setCascadingSectionResizes)
   GUI_CS_PROPERTY_READ(defaultSectionSize, defaultSectionSize)
   GUI_CS_PROPERTY_WRITE(defaultSectionSize, setDefaultSectionSize)
   GUI_CS_PROPERTY_READ(minimumSectionSize, minimumSectionSize)
   GUI_CS_PROPERTY_WRITE(minimumSectionSize, setMinimumSectionSize)
   GUI_CS_PROPERTY_READ(defaultAlignment, defaultAlignment)
   GUI_CS_PROPERTY_WRITE(defaultAlignment, setDefaultAlignment)

   GUI_CS_ENUM(ResizeMode)

 public:

   enum ResizeMode {
      Interactive,
      Stretch,
      Fixed,
      ResizeToContents,
      Custom = Fixed
   };

   explicit QHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
   virtual ~QHeaderView();

   void setModel(QAbstractItemModel *model) override;

   Qt::Orientation orientation() const;
   int offset() const;
   int length() const;
   QSize sizeHint() const override;
   int sectionSizeHint(int logicalIndex) const;

   int visualIndexAt(int position) const;
   int logicalIndexAt(int position) const;

   inline int logicalIndexAt(int x, int y) const;
   inline int logicalIndexAt(const QPoint &pos) const;

   int sectionSize(int logicalIndex) const;
   int sectionPosition(int logicalIndex) const;
   int sectionViewportPosition(int logicalIndex) const;

   void moveSection(int from, int to);
   void swapSections(int first, int second);
   void resizeSection(int logicalIndex, int size);
   void resizeSections(QHeaderView::ResizeMode mode);

   bool isSectionHidden(int logicalIndex) const;
   void setSectionHidden(int logicalIndex, bool hide);
   int hiddenSectionCount() const;

   inline void hideSection(int logicalIndex);
   inline void showSection(int logicalIndex);

   int count() const;
   int visualIndex(int logicalIndex) const;
   int logicalIndex(int visualIndex) const;

   void setMovable(bool movable);
   bool isMovable() const;

   void setClickable(bool clickable);
   bool isClickable() const;

   void setHighlightSections(bool highlight);
   bool highlightSections() const;

   void setResizeMode(ResizeMode mode);
   void setResizeMode(int logicalIndex, ResizeMode mode);
   ResizeMode resizeMode(int logicalIndex) const;
   int stretchSectionCount() const;

   void setSortIndicatorShown(bool show);
   bool isSortIndicatorShown() const;

   void setSortIndicator(int logicalIndex, Qt::SortOrder order);
   int sortIndicatorSection() const;
   Qt::SortOrder sortIndicatorOrder() const;

   bool stretchLastSection() const;
   void setStretchLastSection(bool stretch);

   bool cascadingSectionResizes() const;
   void setCascadingSectionResizes(bool enable);

   int defaultSectionSize() const;
   void setDefaultSectionSize(int size);

   int minimumSectionSize() const;
   void setMinimumSectionSize(int size);

   Qt::Alignment defaultAlignment() const;
   void setDefaultAlignment(Qt::Alignment alignment);

   void doItemsLayout() override;
   bool sectionsMoved() const;
   bool sectionsHidden() const;

#ifndef QT_NO_DATASTREAM
   QByteArray saveState() const;
   bool restoreState(const QByteArray &state);
#endif

   void reset() override;

   GUI_CS_SLOT_1(Public, void setOffset(int offset))
   GUI_CS_SLOT_2(setOffset)
   GUI_CS_SLOT_1(Public, void setOffsetToSectionPosition(int visualIndex))
   GUI_CS_SLOT_2(setOffsetToSectionPosition)
   GUI_CS_SLOT_1(Public, void setOffsetToLastSection())
   GUI_CS_SLOT_2(setOffsetToLastSection)
   GUI_CS_SLOT_1(Public, void headerDataChanged(Qt::Orientation orientation, int logicalFirst, int logicalLast))
   GUI_CS_SLOT_2(headerDataChanged)

   GUI_CS_SIGNAL_1(Public, void sectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex))
   GUI_CS_SIGNAL_2(sectionMoved, logicalIndex, oldVisualIndex, newVisualIndex)
   GUI_CS_SIGNAL_1(Public, void sectionResized(int logicalIndex, int oldSize, int newSize))
   GUI_CS_SIGNAL_2(sectionResized, logicalIndex, oldSize, newSize)
   GUI_CS_SIGNAL_1(Public, void sectionPressed(int logicalIndex))
   GUI_CS_SIGNAL_2(sectionPressed, logicalIndex)
   GUI_CS_SIGNAL_1(Public, void sectionClicked(int logicalIndex))
   GUI_CS_SIGNAL_2(sectionClicked, logicalIndex)
   GUI_CS_SIGNAL_1(Public, void sectionEntered(int logicalIndex))
   GUI_CS_SIGNAL_2(sectionEntered, logicalIndex)
   GUI_CS_SIGNAL_1(Public, void sectionDoubleClicked(int logicalIndex))
   GUI_CS_SIGNAL_2(sectionDoubleClicked, logicalIndex)
   GUI_CS_SIGNAL_1(Public, void sectionCountChanged(int oldCount, int newCount))
   GUI_CS_SIGNAL_2(sectionCountChanged, oldCount, newCount)
   GUI_CS_SIGNAL_1(Public, void sectionHandleDoubleClicked(int logicalIndex))
   GUI_CS_SIGNAL_2(sectionHandleDoubleClicked, logicalIndex)
   GUI_CS_SIGNAL_1(Public, void sectionAutoResize(int logicalIndex, QHeaderView::ResizeMode mode))
   GUI_CS_SIGNAL_2(sectionAutoResize, logicalIndex, mode)
   GUI_CS_SIGNAL_1(Public, void geometriesChanged())
   GUI_CS_SIGNAL_2(geometriesChanged)
   GUI_CS_SIGNAL_1(Public, void sortIndicatorChanged(int logicalIndex, Qt::SortOrder order))
   GUI_CS_SIGNAL_2(sortIndicatorChanged, logicalIndex, order)

 protected :
   GUI_CS_SLOT_1(Protected, void updateSection(int logicalIndex))
   GUI_CS_SLOT_2(updateSection)

   GUI_CS_SLOT_1(Protected, void resizeSections())
   GUI_CS_SLOT_OVERLOAD(resizeSections, ())

   GUI_CS_SLOT_1(Protected, void sectionsInserted(const QModelIndex &parent, int logicalFirst, int logicalLast))
   GUI_CS_SLOT_2(sectionsInserted)

   GUI_CS_SLOT_1(Protected, void sectionsAboutToBeRemoved(const QModelIndex &parent, int logicalFirst, int logicalLast))
   GUI_CS_SLOT_2(sectionsAboutToBeRemoved)

   QHeaderView(QHeaderViewPrivate &dd, Qt::Orientation orientation, QWidget *parent = nullptr);
   void initialize();

   void initializeSections();
   void initializeSections(int start, int end);
   void currentChanged(const QModelIndex &current, const QModelIndex &old) override;

   bool event(QEvent *e) override;
   void paintEvent(QPaintEvent *e) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseMoveEvent(QMouseEvent *e) override;
   void mouseReleaseEvent(QMouseEvent *e) override;
   void mouseDoubleClickEvent(QMouseEvent *e) override;
   bool viewportEvent(QEvent *e) override;

   virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
   virtual QSize sectionSizeFromContents(int logicalIndex) const;

   int horizontalOffset() const override;
   int verticalOffset() const override;
   void updateGeometries() override;
   void scrollContentsBy(int dx, int dy) override;

   void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) override;
   void rowsInserted(const QModelIndex &parent, int start, int end) override;

   QRect visualRect(const QModelIndex &index) const override;
   void scrollTo(const QModelIndex &index, ScrollHint hint) override;

   QModelIndex indexAt(const QPoint &p) const override;
   bool isIndexHidden(const QModelIndex &index) const override;

   QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers) override;
   void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) override;
   QRegion visualRegionForSelection(const QItemSelection &selection) const override;
   void initStyleOption(QStyleOptionHeader *option) const;

 private:
   Q_DECLARE_PRIVATE(QHeaderView)
   Q_DISABLE_COPY(QHeaderView)

   GUI_CS_SLOT_1(Private, void _q_sectionsRemoved(const QModelIndex &parent, int logicalFirst, int logicalLast))
   GUI_CS_SLOT_2(_q_sectionsRemoved)

   GUI_CS_SLOT_1(Private, void _q_layoutAboutToBeChanged())
   GUI_CS_SLOT_2(_q_layoutAboutToBeChanged)
};

inline int QHeaderView::logicalIndexAt(int ax, int ay) const
{
   return orientation() == Qt::Horizontal ? logicalIndexAt(ax) : logicalIndexAt(ay);
}

inline int QHeaderView::logicalIndexAt(const QPoint &apos) const
{
   return logicalIndexAt(apos.x(), apos.y());
}

inline void QHeaderView::hideSection(int alogicalIndex)
{
   setSectionHidden(alogicalIndex, true);
}

inline void QHeaderView::showSection(int alogicalIndex)
{
   setSectionHidden(alogicalIndex, false);
}

#endif // QT_NO_ITEMVIEWS

QT_END_NAMESPACE

#endif // QHEADERVIEW_H
