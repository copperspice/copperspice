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

#ifndef QHEADERVIEW_H
#define QHEADERVIEW_H

#include <qabstractitemview.h>

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

   GUI_CS_PROPERTY_READ(defaultSectionSize,  defaultSectionSize)
   GUI_CS_PROPERTY_WRITE(defaultSectionSize, setDefaultSectionSize)
   GUI_CS_PROPERTY_RESET(defaultSectionSize, resetDefaultSectionSize)

   GUI_CS_PROPERTY_READ(minimumSectionSize, minimumSectionSize)
   GUI_CS_PROPERTY_WRITE(minimumSectionSize, setMinimumSectionSize)

   GUI_CS_PROPERTY_READ(maximumSectionSize, maximumSectionSize)
   GUI_CS_PROPERTY_WRITE(maximumSectionSize, setMaximumSectionSize)

   GUI_CS_PROPERTY_READ(defaultAlignment, defaultAlignment)
   GUI_CS_PROPERTY_WRITE(defaultAlignment, setDefaultAlignment)

   GUI_CS_ENUM(ResizeMode)

 public:
   GUI_CS_REGISTER_ENUM(
      enum ResizeMode {
         Interactive,
         Stretch,
         Fixed,
         ResizeToContents,
         Custom = Fixed
      };
   )

   explicit QHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

   QHeaderView(const QHeaderView &) = delete;
   QHeaderView &operator=(const QHeaderView &) = delete;

   virtual ~QHeaderView();

   void setModel(QAbstractItemModel *model) override;

   Qt::Orientation orientation() const;
   int offset() const;
   int length() const;
   QSize sizeHint() const override;
   void setVisible(bool visible) override;

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

   void setSectionsMovable(bool movable);
   bool sectionsMovable() const;

   void setSectionsClickable(bool clickable);
   bool sectionsClickable() const;

   void setHighlightSections(bool highlight);
   bool highlightSections() const;

   ResizeMode sectionResizeMode(int logicalIndex) const;
   void setSectionResizeMode(ResizeMode mode);
   void setSectionResizeMode(int logicalIndex, ResizeMode mode);
   void setResizeContentsPrecision(int precision);
   int  resizeContentsPrecision() const;

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
   void resetDefaultSectionSize();

   int minimumSectionSize() const;
   void setMinimumSectionSize(int size);
   int maximumSectionSize() const;
   void setMaximumSectionSize(int size);

   Qt::Alignment defaultAlignment() const;
   void setDefaultAlignment(Qt::Alignment alignment);

   void doItemsLayout() override;
   bool sectionsMoved() const;
   bool sectionsHidden() const;

   QByteArray saveState() const;
   bool restoreState(const QByteArray &state);

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

   GUI_CS_SIGNAL_1(Public, void geometriesChanged())
   GUI_CS_SIGNAL_2(geometriesChanged)

   GUI_CS_SIGNAL_1(Public, void sortIndicatorChanged(int logicalIndex, Qt::SortOrder order))
   GUI_CS_SIGNAL_2(sortIndicatorChanged, logicalIndex, order)

 protected:
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

   bool event(QEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mouseDoubleClickEvent(QMouseEvent *event) override;
   bool viewportEvent(QEvent *event) override;

   virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
   virtual QSize sectionSizeFromContents(int logicalIndex) const;

   int horizontalOffset() const override;
   int verticalOffset() const override;
   void updateGeometries() override;
   void scrollContentsBy(int dx, int dy) override;

   void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;
   void rowsInserted(const QModelIndex &parent, int start, int end) override;

   QRect visualRect(const QModelIndex &index) const override;
   void scrollTo(const QModelIndex &index, ScrollHint hint) override;

   QModelIndex indexAt(const QPoint &p) const override;
   bool isIndexHidden(const QModelIndex &index) const override;

   QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers) override;
   void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) override;
   QRegion visualRegionForSelection(const QItemSelection &selection) const override;
   void initStyleOption(QStyleOptionHeader *option) const;

   friend class QTableView;
   friend class QTreeView;

 private:
   Q_DECLARE_PRIVATE(QHeaderView)

   GUI_CS_SLOT_1(Private, void _q_sectionsRemoved(const QModelIndex &parent, int logicalFirst, int logicalLast))
   GUI_CS_SLOT_2(_q_sectionsRemoved)

   GUI_CS_SLOT_1(Private, void _q_layoutAboutToBeChanged())
   GUI_CS_SLOT_2(_q_layoutAboutToBeChanged)
};

inline int QHeaderView::logicalIndexAt(int x, int y) const
{
   return orientation() == Qt::Horizontal ? logicalIndexAt(x) : logicalIndexAt(y);
}

inline int QHeaderView::logicalIndexAt(const QPoint &pos) const
{
   return logicalIndexAt(pos.x(), pos.y());
}

inline void QHeaderView::hideSection(int logicalIndex)
{
   setSectionHidden(logicalIndex, true);
}

inline void QHeaderView::showSection(int logicalIndex)
{
   setSectionHidden(logicalIndex, false);
}

#endif // QT_NO_ITEMVIEWS

#endif
