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

#include <qabstractproxymodel.h>

#ifndef QT_NO_PROXYMODEL

#include <qitemselectionmodel.h>
#include <qsize.h>
#include <qstringlist.h>

#include <qabstractproxymodel_p.h>

// detects the deletion of the source model
void QAbstractProxyModelPrivate::_q_sourceModelDestroyed()
{
   invalidatePersistentIndexes();
   model = QAbstractItemModelPrivate::staticEmptyModel();
}

QAbstractProxyModel::QAbstractProxyModel(QObject *parent)
   : QAbstractItemModel(*new QAbstractProxyModelPrivate, parent)
{
   setSourceModel(QAbstractItemModelPrivate::staticEmptyModel());
}

// internal (cs)
QAbstractProxyModel::QAbstractProxyModel(QAbstractProxyModelPrivate &dd, QObject *parent)
   : QAbstractItemModel(dd, parent)
{
   setSourceModel(QAbstractItemModelPrivate::staticEmptyModel());
}

void QAbstractProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
   Q_D(QAbstractProxyModel);

   if (sourceModel != d->model) {
      if (d->model) {
         disconnect(d->model, &QObject::destroyed, this, &QAbstractProxyModel::_q_sourceModelDestroyed);
      }

      if (sourceModel) {
         d->model = sourceModel;
         connect(d->model, &QObject::destroyed, this, &QAbstractProxyModel::_q_sourceModelDestroyed);

      } else {
         d->model = QAbstractItemModelPrivate::staticEmptyModel();
      }

      d->roleNames = d->model->roleNames();
      emit sourceModelChanged();
   }
}

void QAbstractProxyModel::resetInternalData()
{
   Q_D(QAbstractProxyModel);
   d->roleNames = d->model->roleNames();
}

QAbstractItemModel *QAbstractProxyModel::sourceModel() const
{
   Q_D(const QAbstractProxyModel);

   if (d->model == QAbstractItemModelPrivate::staticEmptyModel()) {
      return nullptr;
   }

   return d->model;
}

bool QAbstractProxyModel::submit()
{
   Q_D(QAbstractProxyModel);
   return d->model->submit();
}

void QAbstractProxyModel::revert()
{
   Q_D(QAbstractProxyModel);
   d->model->revert();
}

QItemSelection QAbstractProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
   QModelIndexList proxyIndexes = proxySelection.indexes();
   QItemSelection sourceSelection;

   for (int i = 0; i < proxyIndexes.size(); ++i) {
      const QModelIndex proxyIdx = mapToSource(proxyIndexes.at(i));
      if (!proxyIdx.isValid()) {
         continue;
      }
      sourceSelection << QItemSelectionRange(proxyIdx);
   }
   return sourceSelection;
}

QItemSelection QAbstractProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
   QModelIndexList sourceIndexes = sourceSelection.indexes();
   QItemSelection proxySelection;

   for (int i = 0; i < sourceIndexes.size(); ++i) {
      const QModelIndex srcIdx = mapFromSource(sourceIndexes.at(i));
      if (!srcIdx.isValid()) {
         continue;
      }
      proxySelection << QItemSelectionRange(srcIdx);
   }

   return proxySelection;
}

QVariant QAbstractProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
   Q_D(const QAbstractProxyModel);
   return d->model->data(mapToSource(proxyIndex), role);
}

QVariant QAbstractProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_D(const QAbstractProxyModel);
   int sourceSection;

   if (orientation == Qt::Horizontal) {
      const QModelIndex proxyIndex = index(0, section);
      sourceSection = mapToSource(proxyIndex).column();
   } else {
      const QModelIndex proxyIndex = index(section, 0);
      sourceSection = mapToSource(proxyIndex).row();
   }

   return d->model->headerData(sourceSection, orientation, role);
}

QMap<int, QVariant> QAbstractProxyModel::itemData(const QModelIndex &proxyIndex) const
{
   return QAbstractItemModel::itemData(proxyIndex);
}

Qt::ItemFlags QAbstractProxyModel::flags(const QModelIndex &index) const
{
   Q_D(const QAbstractProxyModel);
   return d->model->flags(mapToSource(index));
}

bool QAbstractProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   Q_D(QAbstractProxyModel);
   return d->model->setData(mapToSource(index), value, role);
}

bool QAbstractProxyModel::setItemData(const QModelIndex &index, const QMap< int, QVariant > &roles)
{
   return QAbstractItemModel::setItemData(index, roles);
}

bool QAbstractProxyModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
   Q_D(QAbstractProxyModel);

   int sourceSection;

   if (orientation == Qt::Horizontal) {
      const QModelIndex proxyIndex = index(0, section);
      sourceSection = mapToSource(proxyIndex).column();
   } else {
      const QModelIndex proxyIndex = index(section, 0);
      sourceSection = mapToSource(proxyIndex).row();
   }
   return d->model->setHeaderData(sourceSection, orientation, value, role);
}

QModelIndex QAbstractProxyModel::buddy(const QModelIndex &index) const
{
   Q_D(const QAbstractProxyModel);
   return mapFromSource(d->model->buddy(mapToSource(index)));
}

bool QAbstractProxyModel::canFetchMore(const QModelIndex &parent) const
{
   Q_D(const QAbstractProxyModel);
   return d->model->canFetchMore(mapToSource(parent));
}

void QAbstractProxyModel::fetchMore(const QModelIndex &parent)
{
   Q_D(QAbstractProxyModel);
   d->model->fetchMore(mapToSource(parent));
}

void QAbstractProxyModel::sort(int column, Qt::SortOrder order)
{
   Q_D(QAbstractProxyModel);
   d->model->sort(column, order);
}

QSize QAbstractProxyModel::span(const QModelIndex &index) const
{
   Q_D(const QAbstractProxyModel);
   return d->model->span(mapToSource(index));
}

bool QAbstractProxyModel::hasChildren(const QModelIndex &parent) const
{
   Q_D(const QAbstractProxyModel);
   return d->model->hasChildren(mapToSource(parent));
}

QModelIndex QAbstractProxyModel::sibling(int row, int column, const QModelIndex &idx) const
{
   return index(row, column, idx.parent());
}

QMimeData *QAbstractProxyModel::mimeData(const QModelIndexList &indexes) const
{
   Q_D(const QAbstractProxyModel);
   QModelIndexList list;

   for (const QModelIndex &index : indexes) {
      list << mapToSource(index);
   }

   return d->model->mimeData(list);
}

void QAbstractProxyModelPrivate::mapDropCoordinatesToSource(int row, int column, const QModelIndex &parent,
   int *sourceRow, int *sourceColumn, QModelIndex *sourceParent) const
{
   Q_Q(const QAbstractProxyModel);
   *sourceRow = -1;
   *sourceColumn = -1;

   if (row == -1 && column == -1) {
      *sourceParent = q->mapToSource(parent);

   } else if (row == q->rowCount(parent)) {
      *sourceParent = q->mapToSource(parent);
      *sourceRow = model->rowCount(*sourceParent);

   } else {
      QModelIndex proxyIndex = q->index(row, column, parent);
      QModelIndex sourceIndex = q->mapToSource(proxyIndex);
      *sourceRow = sourceIndex.row();
      *sourceColumn = sourceIndex.column();
      *sourceParent = sourceIndex.parent();
   }
}

bool QAbstractProxyModel::canDropMimeData(const QMimeData *data, Qt::DropAction action,
   int row, int column, const QModelIndex &parent) const
{
   Q_D(const QAbstractProxyModel);

   int sourceDestinationRow;
   int sourceDestinationColumn;
   QModelIndex sourceParent;
   d->mapDropCoordinatesToSource(row, column, parent, &sourceDestinationRow, &sourceDestinationColumn, &sourceParent);

   return d->model->canDropMimeData(data, action, sourceDestinationRow, sourceDestinationColumn, sourceParent);
}

bool QAbstractProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
   int row, int column, const QModelIndex &parent)
{
   Q_D(QAbstractProxyModel);
   int sourceDestinationRow;
   int sourceDestinationColumn;
   QModelIndex sourceParent;
   d->mapDropCoordinatesToSource(row, column, parent, &sourceDestinationRow, &sourceDestinationColumn, &sourceParent);
   return d->model->dropMimeData(data, action, sourceDestinationRow, sourceDestinationColumn, sourceParent);
}

QStringList QAbstractProxyModel::mimeTypes() const
{
   Q_D(const QAbstractProxyModel);
   return d->model->mimeTypes();
}

Qt::DropActions QAbstractProxyModel::supportedDragActions() const
{
   Q_D(const QAbstractProxyModel);
   return d->model->supportedDragActions();
}

Qt::DropActions QAbstractProxyModel::supportedDropActions() const
{
   Q_D(const QAbstractProxyModel);
   return d->model->supportedDropActions();
}

void QAbstractProxyModel::_q_sourceModelDestroyed()
{
   Q_D(QAbstractProxyModel);
   d->_q_sourceModelDestroyed();
}

#endif // QT_NO_PROXYMODEL
