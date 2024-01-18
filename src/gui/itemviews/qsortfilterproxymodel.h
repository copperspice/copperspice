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

#ifndef QSORTFILTERPROXYMODEL_H
#define QSORTFILTERPROXYMODEL_H

#include <qabstractproxymodel.h>

#ifndef QT_NO_SORTFILTERPROXYMODEL

#include <qregularexpression.h>

class QSortFilterProxyModelPrivate;
class QSortFilterProxyModelLessThan;
class QSortFilterProxyModelGreaterThan;

class Q_GUI_EXPORT QSortFilterProxyModel : public QAbstractProxyModel
{
   GUI_CS_OBJECT(QSortFilterProxyModel)

   GUI_CS_PROPERTY_READ(filterRegExp, filterRegExp)
   GUI_CS_PROPERTY_WRITE(filterRegExp, cs_setFilterRegExp)

   GUI_CS_PROPERTY_READ(filterKeyColumn, filterKeyColumn)
   GUI_CS_PROPERTY_WRITE(filterKeyColumn, setFilterKeyColumn)

   GUI_CS_PROPERTY_READ(dynamicSortFilter, dynamicSortFilter)
   GUI_CS_PROPERTY_WRITE(dynamicSortFilter, setDynamicSortFilter)

   GUI_CS_PROPERTY_READ(filterCaseSensitivity, filterCaseSensitivity)
   GUI_CS_PROPERTY_WRITE(filterCaseSensitivity, setFilterCaseSensitivity)

   GUI_CS_PROPERTY_READ(sortCaseSensitivity, sortCaseSensitivity)
   GUI_CS_PROPERTY_WRITE(sortCaseSensitivity, setSortCaseSensitivity)

   GUI_CS_PROPERTY_READ(isSortLocaleAware, isSortLocaleAware)
   GUI_CS_PROPERTY_WRITE(isSortLocaleAware, setSortLocaleAware)

   GUI_CS_PROPERTY_READ(sortRole, sortRole)
   GUI_CS_PROPERTY_WRITE(sortRole, setSortRole)

   GUI_CS_PROPERTY_READ(filterRole, filterRole)
   GUI_CS_PROPERTY_WRITE(filterRole, setFilterRole)

   friend class QSortFilterProxyModelLessThan;
   friend class QSortFilterProxyModelGreaterThan;

 public:
   QSortFilterProxyModel(QObject *parent = nullptr);

   QSortFilterProxyModel(const QSortFilterProxyModel &) = delete;
   QSortFilterProxyModel &operator=(const QSortFilterProxyModel &) = delete;

   ~QSortFilterProxyModel();

   void setSourceModel(QAbstractItemModel *sourceModel) override;

   QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
   QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

   QItemSelection mapSelectionToSource(const QItemSelection &proxySelection) const override;
   QItemSelection mapSelectionFromSource(const QItemSelection &sourceSelection) const override;

   QRegularExpression filterRegExp() const;
   void setFilterRegExp(const QRegularExpression &regExp);

   // wrapper for overloaded method
   inline void cs_setFilterRegExp(const QRegularExpression &regExp);

   int filterKeyColumn() const;
   void setFilterKeyColumn(int column);

   Qt::CaseSensitivity filterCaseSensitivity() const;
   void setFilterCaseSensitivity(Qt::CaseSensitivity cs);

   Qt::CaseSensitivity sortCaseSensitivity() const;
   void setSortCaseSensitivity(Qt::CaseSensitivity cs);

   bool isSortLocaleAware() const;
   void setSortLocaleAware(bool on);

   int sortColumn() const;
   Qt::SortOrder sortOrder() const;

   bool dynamicSortFilter() const;
   void setDynamicSortFilter(bool enable);

   int sortRole() const;
   void setSortRole(int role);

   int filterRole() const;
   void setFilterRole(int role);

   GUI_CS_SLOT_1(Public, void setFilterRegExp(const QString &pattern))
   GUI_CS_SLOT_OVERLOAD(setFilterRegExp, (const QString &))

   GUI_CS_SLOT_1(Public, void setFilterWildcard(const QString &pattern))
   GUI_CS_SLOT_2(setFilterWildcard)

   GUI_CS_SLOT_1(Public, void setFilterFixedString(const QString &pattern))
   GUI_CS_SLOT_2(setFilterFixedString)

   GUI_CS_SLOT_1(Public, void invalidate())
   GUI_CS_SLOT_2(invalidate)

   using QObject::parent;

   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
   QModelIndex parent(const QModelIndex &child) const override;

   QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;

   int rowCount(const QModelIndex &parent = QModelIndex()) const override;
   int columnCount(const QModelIndex &parent = QModelIndex()) const override;
   bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
   bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

   QMimeData *mimeData(const QModelIndexList &indexes) const override;
   bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

   bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
   bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
   bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
   bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

   void fetchMore(const QModelIndex &parent) override;
   bool canFetchMore(const QModelIndex &parent) const override;
   Qt::ItemFlags flags(const QModelIndex &index) const override;

   QModelIndex buddy(const QModelIndex &index) const override;
   QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1,
      Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const override;

   QSize span(const QModelIndex &index) const override;
   void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

   QStringList mimeTypes() const override;
   Qt::DropActions supportedDropActions() const override;

 protected:
   virtual bool filterAcceptsRow(int source_row, const QModelIndex &sourceParent) const;
   virtual bool filterAcceptsColumn(int source_column, const QModelIndex &sourceParent) const;
   virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

   void invalidateFilter();

 private:
   Q_DECLARE_PRIVATE(QSortFilterProxyModel)

   GUI_CS_SLOT_1(Private, void _q_sourceDataChanged(const QModelIndex &source_top_left,
         const QModelIndex &source_bottom_right, const QVector<int> &roles))
   GUI_CS_SLOT_2(_q_sourceDataChanged)

   GUI_CS_SLOT_1(Private, void _q_sourceHeaderDataChanged(Qt::Orientation orientation, int start, int end))
   GUI_CS_SLOT_2(_q_sourceHeaderDataChanged)

   GUI_CS_SLOT_1(Private, void _q_sourceAboutToBeReset())
   GUI_CS_SLOT_2(_q_sourceAboutToBeReset)

   GUI_CS_SLOT_1(Private, void _q_sourceReset())
   GUI_CS_SLOT_2(_q_sourceReset)

   GUI_CS_SLOT_1(Private, void _q_sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents,
         QAbstractItemModel::LayoutChangeHint hint))
   GUI_CS_SLOT_2(_q_sourceLayoutAboutToBeChanged)

   GUI_CS_SLOT_1(Private, void _q_sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents,
         QAbstractItemModel::LayoutChangeHint hint))
   GUI_CS_SLOT_2(_q_sourceLayoutChanged)

   GUI_CS_SLOT_1(Private, void _q_sourceRowsAboutToBeInserted(const QModelIndex &sourceParent, int start, int end))
   GUI_CS_SLOT_2(_q_sourceRowsAboutToBeInserted)

   GUI_CS_SLOT_1(Private, void _q_sourceRowsInserted(const QModelIndex &sourceParent, int start, int end))
   GUI_CS_SLOT_2(_q_sourceRowsInserted)

   GUI_CS_SLOT_1(Private, void _q_sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end))
   GUI_CS_SLOT_2(_q_sourceRowsAboutToBeRemoved)

   GUI_CS_SLOT_1(Private, void _q_sourceRowsRemoved(const QModelIndex &sourceParent, int start, int end))
   GUI_CS_SLOT_2(_q_sourceRowsRemoved)

   GUI_CS_SLOT_1(Private, void _q_sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int, int,
         const QModelIndex &, int))
   GUI_CS_SLOT_2(_q_sourceRowsAboutToBeMoved)

   GUI_CS_SLOT_1(Private, void _q_sourceRowsMoved(const QModelIndex &sourceParent, int, int,
         const QModelIndex &, int))
   GUI_CS_SLOT_2(_q_sourceRowsMoved)

   GUI_CS_SLOT_1(Private, void _q_sourceColumnsAboutToBeInserted(const QModelIndex &sourceParent, int start, int end))
   GUI_CS_SLOT_2(_q_sourceColumnsAboutToBeInserted)

   GUI_CS_SLOT_1(Private, void _q_sourceColumnsInserted(const QModelIndex &sourceParent, int start, int end))
   GUI_CS_SLOT_2(_q_sourceColumnsInserted)

   GUI_CS_SLOT_1(Private, void _q_sourceColumnsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end))
   GUI_CS_SLOT_2(_q_sourceColumnsAboutToBeRemoved)

   GUI_CS_SLOT_1(Private, void _q_sourceColumnsRemoved(const QModelIndex &sourceParent, int start, int end))
   GUI_CS_SLOT_2(_q_sourceColumnsRemoved)

   GUI_CS_SLOT_1(Private, void _q_sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int, int,
         const QModelIndex &, int))
   GUI_CS_SLOT_2(_q_sourceColumnsAboutToBeMoved)

   GUI_CS_SLOT_1(Private, void _q_sourceColumnsMoved(const QModelIndex &sourceParent, int, int,
         const QModelIndex &, int))
   GUI_CS_SLOT_2(_q_sourceColumnsMoved)

   GUI_CS_SLOT_1(Private, void _q_clearMapping())
   GUI_CS_SLOT_2(_q_clearMapping)
};

void QSortFilterProxyModel::cs_setFilterRegExp(const QRegularExpression &regExp)
{
   setFilterRegExp(regExp);
}

#endif // QT_NO_SORTFILTERPROXYMODEL

#endif // QSORTFILTERPROXYMODEL_H
