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

#include <qstringlistmodel.h>
#include <qvector.h>

#include <algorithm>

#ifndef QT_NO_STRINGLISTMODEL

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

QModelIndex QStringListModel::sibling(int row, int column, const QModelIndex &idx) const
{
   if (! idx.isValid() || column != 0 || row >= lst.count()) {
      return QModelIndex();
   }

   return createIndex(row, 0);
}

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

Qt::ItemFlags QStringListModel::flags(const QModelIndex &index) const
{
   if (!index.isValid()) {
      return QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled;
   }

   return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool QStringListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   if (index.row() >= 0 && index.row() < lst.size()
         && (role == Qt::EditRole || role == Qt::DisplayRole)) {
      lst.replace(index.row(), value.toString());

      QVector<int> roles;
      roles.reserve(2);
      roles.append(Qt::DisplayRole);
      roles.append(Qt::EditRole);

      emit dataChanged(index, index, roles);
      return true;
   }

   return false;
}

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

void QStringListModel::sort(int, Qt::SortOrder order)
{
   emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), VerticalSortHint);

   QList<QPair<QString, int>> list;
   const int lstCount = lst.count();

   for (int i = 0; i < lstCount; ++i) {
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

   const int numOldIndexes = oldList.count();

   for (int i = 0; i < numOldIndexes; ++i) {
      newList.append(index(forwarding.at(oldList.at(i).row()), 0));
   }
   changePersistentIndexList(oldList, newList);

   emit layoutChanged(QList<QPersistentModelIndex>(), VerticalSortHint);
}

QStringList QStringListModel::stringList() const
{
   return lst;
}

void QStringListModel::setStringList(const QStringList &strings)
{
   beginResetModel();
   lst = strings;
   endResetModel();
}

Qt::DropActions QStringListModel::supportedDropActions() const
{
   return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

#endif // QT_NO_STRINGLISTMODEL
