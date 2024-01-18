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

#include <qidentityproxymodel.h>

#ifndef QT_NO_IDENTITYPROXYMODEL

#include <qitemselectionmodel.h>
#include <qabstractproxymodel_p.h>

class QIdentityProxyModelPrivate : public QAbstractProxyModelPrivate
{
   Q_DECLARE_PUBLIC(QIdentityProxyModel)

   QIdentityProxyModelPrivate()
   { }

   QList<QPersistentModelIndex> layoutChangePersistentIndexes;
   QModelIndexList proxyIndexes;

   void _q_sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
   void _q_sourceRowsInserted(const QModelIndex &parent, int start, int end);
   void _q_sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
   void _q_sourceRowsRemoved(const QModelIndex &parent, int start, int end);
   void _q_sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
         const QModelIndex &destinationParent, int destinationStart);
   void _q_sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
         const QModelIndex &destinationParent, int destinationStart);

   void _q_sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end);
   void _q_sourceColumnsInserted(const QModelIndex &parent, int start, int end);
   void _q_sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
   void _q_sourceColumnsRemoved(const QModelIndex &parent, int start, int end);
   void _q_sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
         const QModelIndex &destinationParent, int destinationStart);
   void _q_sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
         const QModelIndex &destinationParent, int destinationStart);

   void _q_sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
   void _q_sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last);

   void _q_sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents,
         QAbstractItemModel::LayoutChangeHint hint);
   void _q_sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
   void _q_sourceModelAboutToBeReset();
   void _q_sourceModelReset();
};

QIdentityProxyModel::QIdentityProxyModel(QObject *parent)
   : QAbstractProxyModel(*new QIdentityProxyModelPrivate, parent)
{
}

QIdentityProxyModel::QIdentityProxyModel(QIdentityProxyModelPrivate &dd, QObject *parent)
   : QAbstractProxyModel(dd, parent)
{
}

QIdentityProxyModel::~QIdentityProxyModel()
{
}

int QIdentityProxyModel::columnCount(const QModelIndex &parent) const
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);
   Q_D(const QIdentityProxyModel);

   return d->model->columnCount(mapToSource(parent));
}

bool QIdentityProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
   const QModelIndex &parent)
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);

   Q_D(QIdentityProxyModel);

   return d->model->dropMimeData(data, action, row, column, mapToSource(parent));
}

QModelIndex QIdentityProxyModel::index(int row, int column, const QModelIndex &parent) const
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);
   Q_D(const QIdentityProxyModel);
   const QModelIndex sourceParent = mapToSource(parent);
   const QModelIndex sourceIndex = d->model->index(row, column, sourceParent);

   return mapFromSource(sourceIndex);
}

QModelIndex QIdentityProxyModel::sibling(int row, int column, const QModelIndex &idx) const
{
   Q_D(const QIdentityProxyModel);

   return mapFromSource(d->model->sibling(row, column, mapToSource(idx)));
}

bool QIdentityProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);
   Q_D(QIdentityProxyModel);

   return d->model->insertColumns(column, count, mapToSource(parent));
}

bool QIdentityProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);

   Q_D(QIdentityProxyModel);
   return d->model->insertRows(row, count, mapToSource(parent));
}

QModelIndex QIdentityProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
   Q_D(const QIdentityProxyModel);

   if (! d->model || !sourceIndex.isValid()) {
      return QModelIndex();
   }

   Q_ASSERT(sourceIndex.model() == d->model);

   return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
}

QItemSelection QIdentityProxyModel::mapSelectionFromSource(const QItemSelection &selection) const
{
   Q_D(const QIdentityProxyModel);
   QItemSelection proxySelection;

   if (! d->model) {
      return proxySelection;
   }

   QItemSelection::const_iterator it = selection.constBegin();
   const QItemSelection::const_iterator end = selection.constEnd();

   for ( ; it != end; ++it) {
      Q_ASSERT(it->model() == d->model);
      const QItemSelectionRange range(mapFromSource(it->topLeft()), mapFromSource(it->bottomRight()));
      proxySelection.append(range);
   }

   return proxySelection;
}

QItemSelection QIdentityProxyModel::mapSelectionToSource(const QItemSelection &selection) const
{
   Q_D(const QIdentityProxyModel);
   QItemSelection sourceSelection;

   if (! d->model) {
      return sourceSelection;
   }

   QItemSelection::const_iterator it = selection.constBegin();
   const QItemSelection::const_iterator end = selection.constEnd();

   for ( ; it != end; ++it) {
      Q_ASSERT(it->model() == this);
      const QItemSelectionRange range(mapToSource(it->topLeft()), mapToSource(it->bottomRight()));
      sourceSelection.append(range);
   }

   return sourceSelection;
}

QModelIndex QIdentityProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
   Q_D(const QIdentityProxyModel);

   if (! d->model || !proxyIndex.isValid()) {
      return QModelIndex();
   }

   Q_ASSERT(proxyIndex.model() == this);

   return d->model->createIndex(proxyIndex.row(), proxyIndex.column(), proxyIndex.internalPointer());
}

QModelIndexList QIdentityProxyModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
   Qt::MatchFlags flags) const
{
   Q_D(const QIdentityProxyModel);

   Q_ASSERT(start.isValid() ? start.model() == this : true);
   if (!d->model) {
      return QModelIndexList();
   }

   const QModelIndexList sourceList = d->model->match(mapToSource(start), role, value, hits, flags);
   QModelIndexList::const_iterator it = sourceList.constBegin();

   const QModelIndexList::const_iterator end = sourceList.constEnd();
   QModelIndexList proxyList;

   for ( ; it != end; ++it) {
      proxyList.append(mapFromSource(*it));
   }

   return proxyList;
}

QModelIndex QIdentityProxyModel::parent(const QModelIndex &child) const
{
   Q_ASSERT(child.isValid() ? child.model() == this : true);
   const QModelIndex sourceIndex = mapToSource(child);
   const QModelIndex sourceParent = sourceIndex.parent();
   return mapFromSource(sourceParent);
}

bool QIdentityProxyModel::removeColumns(int column, int count, const QModelIndex &parent)
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);
   Q_D(QIdentityProxyModel);
   return d->model->removeColumns(column, count, mapToSource(parent));
}

bool QIdentityProxyModel::removeRows(int row, int count, const QModelIndex &parent)
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);
   Q_D(QIdentityProxyModel);

   return d->model->removeRows(row, count, mapToSource(parent));
}

int QIdentityProxyModel::rowCount(const QModelIndex &parent) const
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);
   Q_D(const QIdentityProxyModel);

   return d->model->rowCount(mapToSource(parent));
}

QVariant QIdentityProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_D(const QIdentityProxyModel);

   return d->model->headerData(section, orientation, role);
}

void QIdentityProxyModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
   beginResetModel();

   if (sourceModel()) {
      disconnect(sourceModel(), &QAbstractItemModel::rowsAboutToBeInserted,    this,
            &QIdentityProxyModel::_q_sourceRowsAboutToBeInserted);
      disconnect(sourceModel(), &QAbstractItemModel::rowsInserted,             this,
            &QIdentityProxyModel::_q_sourceRowsInserted);
      disconnect(sourceModel(), &QAbstractItemModel::rowsAboutToBeRemoved,     this,
            &QIdentityProxyModel::_q_sourceRowsAboutToBeRemoved);
      disconnect(sourceModel(), &QAbstractItemModel::rowsRemoved,              this,
            &QIdentityProxyModel::_q_sourceRowsRemoved);
      disconnect(sourceModel(), &QAbstractItemModel::rowsAboutToBeMoved,       this,
            &QIdentityProxyModel::_q_sourceRowsAboutToBeMoved);
      disconnect(sourceModel(), &QAbstractItemModel::rowsMoved,                this,
            &QIdentityProxyModel::_q_sourceRowsMoved);

      disconnect(sourceModel(), &QAbstractItemModel::columnsAboutToBeInserted, this,
            &QIdentityProxyModel::_q_sourceColumnsAboutToBeInserted);
      disconnect(sourceModel(), &QAbstractItemModel::columnsInserted,          this,
            &QIdentityProxyModel::_q_sourceColumnsInserted);
      disconnect(sourceModel(), &QAbstractItemModel::columnsAboutToBeRemoved,  this,
            &QIdentityProxyModel::_q_sourceColumnsAboutToBeRemoved);
      disconnect(sourceModel(), &QAbstractItemModel::columnsRemoved,           this,
            &QIdentityProxyModel::_q_sourceColumnsRemoved);
      disconnect(sourceModel(), &QAbstractItemModel::columnsAboutToBeMoved,    this,
            &QIdentityProxyModel::_q_sourceColumnsAboutToBeMoved);
      disconnect(sourceModel(), &QAbstractItemModel::columnsMoved,             this,
            &QIdentityProxyModel::_q_sourceColumnsMoved);

      disconnect(sourceModel(), &QAbstractItemModel::modelAboutToBeReset,      this,
            &QIdentityProxyModel::_q_sourceModelAboutToBeReset);
      disconnect(sourceModel(), &QAbstractItemModel::modelReset,               this,
            &QIdentityProxyModel::_q_sourceModelReset);
      disconnect(sourceModel(), &QAbstractItemModel::dataChanged,              this,
            &QIdentityProxyModel::_q_sourceDataChanged);
      disconnect(sourceModel(), &QAbstractItemModel::headerDataChanged,        this,
            &QIdentityProxyModel::_q_sourceHeaderDataChanged);
      disconnect(sourceModel(), &QAbstractItemModel::layoutAboutToBeChanged,   this,
            &QIdentityProxyModel::_q_sourceLayoutAboutToBeChanged);
      disconnect(sourceModel(), &QAbstractItemModel::layoutChanged,            this,
            &QIdentityProxyModel::_q_sourceLayoutChanged);
   }

   QAbstractProxyModel::setSourceModel(newSourceModel);

   if (sourceModel()) {
      connect(sourceModel(), &QAbstractItemModel::rowsAboutToBeInserted,    this,
            &QIdentityProxyModel::_q_sourceRowsAboutToBeInserted);
      connect(sourceModel(), &QAbstractItemModel::rowsInserted,             this,
            &QIdentityProxyModel::_q_sourceRowsInserted);
      connect(sourceModel(), &QAbstractItemModel::rowsAboutToBeRemoved,     this,
            &QIdentityProxyModel::_q_sourceRowsAboutToBeRemoved);
      connect(sourceModel(), &QAbstractItemModel::rowsRemoved,              this,
            &QIdentityProxyModel::_q_sourceRowsRemoved);
      connect(sourceModel(), &QAbstractItemModel::rowsAboutToBeMoved,       this,
            &QIdentityProxyModel::_q_sourceRowsAboutToBeMoved);
      connect(sourceModel(), &QAbstractItemModel::rowsMoved,                this,
            &QIdentityProxyModel::_q_sourceRowsMoved);

      connect(sourceModel(), &QAbstractItemModel::columnsAboutToBeInserted, this,
            &QIdentityProxyModel::_q_sourceColumnsAboutToBeInserted);
      connect(sourceModel(), &QAbstractItemModel::columnsInserted,          this,
            &QIdentityProxyModel::_q_sourceColumnsInserted);
      connect(sourceModel(), &QAbstractItemModel::columnsAboutToBeRemoved,  this,
            &QIdentityProxyModel::_q_sourceColumnsAboutToBeRemoved);
      connect(sourceModel(), &QAbstractItemModel::columnsRemoved,           this,
            &QIdentityProxyModel::_q_sourceColumnsRemoved);
      connect(sourceModel(), &QAbstractItemModel::columnsAboutToBeMoved,    this,
            &QIdentityProxyModel::_q_sourceColumnsAboutToBeMoved);
      connect(sourceModel(), &QAbstractItemModel::columnsMoved,             this,
            &QIdentityProxyModel::_q_sourceColumnsMoved);

      connect(sourceModel(), &QAbstractItemModel::modelAboutToBeReset,      this,
            &QIdentityProxyModel::_q_sourceModelAboutToBeReset);
      connect(sourceModel(), &QAbstractItemModel::modelReset,               this,
            &QIdentityProxyModel::_q_sourceModelReset);
      connect(sourceModel(), &QAbstractItemModel::dataChanged,              this,
            &QIdentityProxyModel::_q_sourceDataChanged);
      connect(sourceModel(), &QAbstractItemModel::headerDataChanged,        this,
            &QIdentityProxyModel::_q_sourceHeaderDataChanged);
      connect(sourceModel(), &QAbstractItemModel::layoutAboutToBeChanged,   this,
            &QIdentityProxyModel::_q_sourceLayoutAboutToBeChanged);
      connect(sourceModel(), &QAbstractItemModel::layoutChanged,            this,
            &QIdentityProxyModel::_q_sourceLayoutChanged);
   }

   endResetModel();
}

void QIdentityProxyModelPrivate::_q_sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);

   q->beginInsertColumns(q->mapFromSource(parent), start, end);
}

void QIdentityProxyModelPrivate::_q_sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart,
   int sourceEnd, const QModelIndex &destinationParent, int destinationStart)
{
   Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == model : true);
   Q_ASSERT(destinationParent.isValid() ? destinationParent.model() == model : true);

   Q_Q(QIdentityProxyModel);

   q->beginMoveColumns(q->mapFromSource(sourceParent), sourceStart,
         sourceEnd, q->mapFromSource(destinationParent), destinationStart);
}

void QIdentityProxyModelPrivate::_q_sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
   Q_ASSERT(parent.isValid() ? parent.model() == model : true);

   Q_Q(QIdentityProxyModel);
   q->beginRemoveColumns(q->mapFromSource(parent), start, end);
}

void QIdentityProxyModelPrivate::_q_sourceColumnsInserted(const QModelIndex &parent, int start, int end)
{
   (void) parent;
   (void) start;
   (void) end;

   Q_ASSERT(parent.isValid() ? parent.model() == model : true);

   Q_Q(QIdentityProxyModel);
   q->endInsertColumns();
}

void QIdentityProxyModelPrivate::_q_sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
   const QModelIndex &destinationParent, int destinationStart)
{
   (void) sourceParent;
   (void) sourceStart;
   (void) sourceEnd;
   (void) destinationParent;
   (void) destinationStart;

   Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == model : true);
   Q_ASSERT(destinationParent.isValid() ? destinationParent.model() == model : true);

   Q_Q(QIdentityProxyModel);
   q->endMoveColumns();
}

void QIdentityProxyModelPrivate::_q_sourceColumnsRemoved(const QModelIndex &parent, int start, int end)
{
   (void) parent;
   (void) start;
   (void) end;

   Q_ASSERT(parent.isValid() ? parent.model() == model : true);

   Q_Q(QIdentityProxyModel);
   q->endRemoveColumns();
}

void QIdentityProxyModelPrivate::_q_sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
      const QVector<int> &roles)
{
   Q_ASSERT(topLeft.isValid() ? topLeft.model() == model : true);
   Q_ASSERT(bottomRight.isValid() ? bottomRight.model() == model : true);

   Q_Q(QIdentityProxyModel);
   q->dataChanged(q->mapFromSource(topLeft), q->mapFromSource(bottomRight), roles);
}

void QIdentityProxyModelPrivate::_q_sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
   Q_Q(QIdentityProxyModel);
   q->headerDataChanged(orientation, first, last);
}

void QIdentityProxyModelPrivate::_q_sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents,
   QAbstractItemModel::LayoutChangeHint hint)
{
   Q_Q(QIdentityProxyModel);

   for (const QModelIndex &proxyPersistentIndex : q->persistentIndexList()) {
      proxyIndexes << proxyPersistentIndex;
      Q_ASSERT(proxyPersistentIndex.isValid());

      const QPersistentModelIndex srcPersistentIndex = q->mapToSource(proxyPersistentIndex);
      Q_ASSERT(srcPersistentIndex.isValid());

      layoutChangePersistentIndexes << srcPersistentIndex;
   }

   QList<QPersistentModelIndex> parents;

   for (const QPersistentModelIndex &parent : sourceParents) {
      if (! parent.isValid()) {
         parents << QPersistentModelIndex();
         continue;
      }

      const QModelIndex mappedParent = q->mapFromSource(parent);
      Q_ASSERT(mappedParent.isValid());
      parents << mappedParent;
   }

   q->layoutAboutToBeChanged(parents, hint);
}

void QIdentityProxyModelPrivate::_q_sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents,
   QAbstractItemModel::LayoutChangeHint hint)
{
   Q_Q(QIdentityProxyModel);

   for (int i = 0; i < proxyIndexes.size(); ++i) {
      q->changePersistentIndex(proxyIndexes.at(i), q->mapFromSource(layoutChangePersistentIndexes.at(i)));
   }

   layoutChangePersistentIndexes.clear();
   proxyIndexes.clear();

   QList<QPersistentModelIndex> parents;

   for (const QPersistentModelIndex &parent : sourceParents) {
      if (! parent.isValid()) {
         parents << QPersistentModelIndex();
         continue;
      }

      const QModelIndex mappedParent = q->mapFromSource(parent);
      Q_ASSERT(mappedParent.isValid());
      parents << mappedParent;
   }

   q->layoutChanged(parents, hint);
}

void QIdentityProxyModelPrivate::_q_sourceModelAboutToBeReset()
{
   Q_Q(QIdentityProxyModel);
   q->beginResetModel();
}

void QIdentityProxyModelPrivate::_q_sourceModelReset()
{
   Q_Q(QIdentityProxyModel);
   q->endResetModel();
}

void QIdentityProxyModelPrivate::_q_sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);

   q->beginInsertRows(q->mapFromSource(parent), start, end);
}

void QIdentityProxyModelPrivate::_q_sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart,
   int sourceEnd, const QModelIndex &destinationParent, int destinationStart)
{
   Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == model : true);
   Q_ASSERT(destinationParent.isValid() ? destinationParent.model() == model : true);

   Q_Q(QIdentityProxyModel);

   q->beginMoveRows(q->mapFromSource(sourceParent), sourceStart, sourceEnd,
         q->mapFromSource(destinationParent), destinationStart);
}

void QIdentityProxyModelPrivate::_q_sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);

   q->beginRemoveRows(q->mapFromSource(parent), start, end);
}

void QIdentityProxyModelPrivate::_q_sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
   (void) start;
   (void) end;

   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);

   q->endInsertRows();
}

void QIdentityProxyModelPrivate::_q_sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
   const QModelIndex &destinationParent, int destinationStart)
{
   (void) sourceStart;
   (void) sourceEnd;
   (void) destinationStart;

   Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == model : true);
   Q_ASSERT(destinationParent.isValid() ? destinationParent.model() == model : true);

   Q_Q(QIdentityProxyModel);

   q->endMoveRows();
}

void QIdentityProxyModelPrivate::_q_sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
   (void) start;
   (void) end;

   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);

   q->endRemoveRows();
}

void QIdentityProxyModel::_q_sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceRowsAboutToBeInserted(parent, start, end);
}

void QIdentityProxyModel::_q_sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceRowsInserted(parent, start, end);
}

void QIdentityProxyModel::_q_sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceRowsAboutToBeRemoved(parent, start, end);
}

void QIdentityProxyModel::_q_sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceRowsRemoved(parent, start, end);
}

void QIdentityProxyModel::_q_sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
         const QModelIndex &destinationParent, int destinationStart)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceRowsAboutToBeMoved(sourceParent, sourceStart, sourceEnd, destinationParent, destinationStart);
}

void QIdentityProxyModel::_q_sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
         const QModelIndex &destinationParent, int destinationStart)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceRowsMoved(sourceParent, sourceStart, sourceEnd, destinationParent, destinationStart);
}

void QIdentityProxyModel::_q_sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceColumnsAboutToBeInserted(parent, start, end);
}

void QIdentityProxyModel::_q_sourceColumnsInserted(const QModelIndex &parent, int start, int end)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceColumnsInserted(parent, start, end);
}

void QIdentityProxyModel::_q_sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceColumnsAboutToBeRemoved(parent, start, end);
}

void QIdentityProxyModel::_q_sourceColumnsRemoved(const QModelIndex &parent, int start, int end)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceColumnsRemoved(parent, start, end);
}

void QIdentityProxyModel::_q_sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
         const QModelIndex &destinationParent, int destinationStart)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceColumnsAboutToBeMoved(sourceParent, sourceStart, sourceEnd, destinationParent, destinationStart);
}

void QIdentityProxyModel::_q_sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
         const QModelIndex &destinationParent, int destinationStart)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceColumnsMoved(sourceParent, sourceStart, sourceEnd, destinationParent, destinationStart);
}

void QIdentityProxyModel::_q_sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
      const QVector<int> &roles)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceDataChanged(topLeft, bottomRight, roles);
}

void QIdentityProxyModel::_q_sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceHeaderDataChanged(orientation, first, last);
}

void QIdentityProxyModel::_q_sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents,
   QAbstractItemModel::LayoutChangeHint hint)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceLayoutAboutToBeChanged(sourceParents, hint);
}

void QIdentityProxyModel::_q_sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents,
   QAbstractItemModel::LayoutChangeHint hint)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceLayoutChanged(sourceParents, hint);
}

void QIdentityProxyModel::_q_sourceModelAboutToBeReset()
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceModelAboutToBeReset();
}

void QIdentityProxyModel::_q_sourceModelReset()
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceModelReset();
}

#endif // QT_NO_IDENTITYPROXYMODEL
