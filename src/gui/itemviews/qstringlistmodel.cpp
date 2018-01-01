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

/*
    A simple model that uses a QStringList as its data source.
*/

#include <algorithm>

#include <qstringlistmodel.h>

#ifndef QT_NO_STRINGLISTMODEL

QT_BEGIN_NAMESPACE

QStringListModel::QStringListModel(QObject *parent)
   : QAbstractListModel(parent)
{
}

QStringListModel::QStringListModel(const QStringList &strings, QObject *parent)
   : QAbstractListModel(parent), lst(strings)
{
}

int QStringListModel::rowCount(const QModelIndex &parent) const
{
   if (parent.isValid()) {
      return 0;
   }

   return lst.count();
}

/*!
    Returns data for the specified \a role, from the item with the
    given \a index.

    If the view requests an invalid index, an invalid variant is returned.

    \sa setData()
*/

QVariant QStringListModel::data(const QModelIndex &index, int role) const
{
   if (index.row() < 0 || index.row() >= lst.size()) {
      return QVariant();
   }

   if (role == Qt::DisplayRole || role == Qt::EditRole) {
      return lst.at(index.row());
   }

   return QVariant();
}

/*!
    Returns the flags for the item with the given \a index.

    Valid items are enabled, selectable, editable, drag enabled and drop enabled.

    \sa QAbstractItemModel::flags()
*/

Qt::ItemFlags QStringListModel::flags(const QModelIndex &index) const
{
   if (!index.isValid()) {
      return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;
   }

   return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

/*!
    Sets the data for the specified \a role in the item with the given
    \a index in the model, to the provided \a value.

    The dataChanged() signal is emitted if the item is changed.

    \sa Qt::ItemDataRole, data()
*/

bool QStringListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   if (index.row() >= 0 && index.row() < lst.size()
         && (role == Qt::EditRole || role == Qt::DisplayRole)) {
      lst.replace(index.row(), value.toString());
      emit dataChanged(index, index);
      return true;
   }
   return false;
}

/*!
    Inserts \a count rows into the model, beginning at the given \a row.

    The \a parent index of the rows is optional and is only used for
    consistency with QAbstractItemModel. By default, a null index is
    specified, indicating that the rows are inserted in the top level of
    the model.

    \sa QAbstractItemModel::insertRows()
*/

bool QStringListModel::insertRows(int row, int count, const QModelIndex &parent)
{
   if (count < 1 || row < 0 || row > rowCount(parent)) {
      return false;
   }

   beginInsertRows(QModelIndex(), row, row + count - 1);

   for (int r = 0; r < count; ++r) {
      lst.insert(row, QString());
   }

   endInsertRows();

   return true;
}

/*!
    Removes \a count rows from the model, beginning at the given \a row.

    The \a parent index of the rows is optional and is only used for
    consistency with QAbstractItemModel. By default, a null index is
    specified, indicating that the rows are removed in the top level of
    the model.

    \sa QAbstractItemModel::removeRows()
*/

bool QStringListModel::removeRows(int row, int count, const QModelIndex &parent)
{
   if (count <= 0 || row < 0 || (row + count) > rowCount(parent)) {
      return false;
   }

   beginRemoveRows(QModelIndex(), row, row + count - 1);

   for (int r = 0; r < count; ++r) {
      lst.removeAt(row);
   }

   endRemoveRows();

   return true;
}

static bool ascendingLessThan(const QPair<QString, int> &s1, const QPair<QString, int> &s2)
{
   return s1.first < s2.first;
}

static bool decendingLessThan(const QPair<QString, int> &s1, const QPair<QString, int> &s2)
{
   return s1.first > s2.first;
}

/*!
  \reimp
*/
void QStringListModel::sort(int, Qt::SortOrder order)
{
   emit layoutAboutToBeChanged();

   QList<QPair<QString, int> > list;
   for (int i = 0; i < lst.count(); ++i) {
      list.append(QPair<QString, int>(lst.at(i), i));
   }

   if (order == Qt::AscendingOrder) {
      std::sort(list.begin(), list.end(), ascendingLessThan);
   } else {
      std::sort(list.begin(), list.end(), decendingLessThan);
   }

   lst.clear();
   QVector<int> forwarding(list.count());
   for (int i = 0; i < list.count(); ++i) {
      lst.append(list.at(i).first);
      forwarding[list.at(i).second] = i;
   }

   QModelIndexList oldList = persistentIndexList();
   QModelIndexList newList;
   for (int i = 0; i < oldList.count(); ++i) {
      newList.append(index(forwarding.at(oldList.at(i).row()), 0));
   }
   changePersistentIndexList(oldList, newList);

   emit layoutChanged();
}

/*!
    Returns the string list used by the model to store data.
*/
QStringList QStringListModel::stringList() const
{
   return lst;
}

/*!
    Sets the model's internal string list to \a strings. The model will
    notify any attached views that its underlying data has changed.

    \sa dataChanged()
*/
void QStringListModel::setStringList(const QStringList &strings)
{
   emit beginResetModel();
   lst = strings;
   emit endResetModel();
}

/*!
  \reimp
*/
Qt::DropActions QStringListModel::supportedDropActions() const
{
   return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

QT_END_NAMESPACE

#endif // QT_NO_STRINGLISTMODEL
