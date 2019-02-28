/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

   void _q_sourceRowsAboutToBeInserted(QModelIndex parent, int start, int end);
   void _q_sourceRowsInserted(QModelIndex parent, int start, int end);
   void _q_sourceRowsAboutToBeRemoved(QModelIndex parent, int start, int end);
   void _q_sourceRowsRemoved(QModelIndex parent, int start, int end);
   void _q_sourceRowsAboutToBeMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destParent, int dest);
   void _q_sourceRowsMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destParent, int dest);

   void _q_sourceColumnsAboutToBeInserted(QModelIndex parent, int start, int end);
   void _q_sourceColumnsInserted(QModelIndex parent, int start, int end);
   void _q_sourceColumnsAboutToBeRemoved(QModelIndex parent, int start, int end);
   void _q_sourceColumnsRemoved(QModelIndex parent, int start, int end);
   void _q_sourceColumnsAboutToBeMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destParent, int dest);
   void _q_sourceColumnsMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destParent, int dest);

   void _q_sourceDataChanged(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles);
   void _q_sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last);

   void _q_sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
   void _q_sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
   void _q_sourceModelAboutToBeReset();
   void _q_sourceModelReset();

};

/*!
    \since 4.8
    \class QIdentityProxyModel
    \brief The QIdentityProxyModel class proxies its source model unmodified

    \ingroup model-view

    QIdentityProxyModel can be used to forward the structure of a source model exactly, with no sorting, filtering or other transformation.
    This is similar in concept to an identity matrix where A.I = A.

    Because it does no sorting or filtering, this class is most suitable to proxy models which transform the data() of the source model.
    For example, a proxy model could be created to define the font used, or the background colour, or the tooltip etc. This removes the
    need to implement all data handling in the same class that creates the structure of the model, and can also be used to create
    re-usable components.

    This also provides a way to change the data in the case where a source model is supplied by a third party which can not be modified.

    \snippet doc/src/snippets/code/src_gui_itemviews_qidentityproxymodel.cpp 0

    \sa QAbstractProxyModel, {Model/View Programming}, QAbstractItemModel

*/

/*!
    Constructs an identity model with the given \a parent.
*/
QIdentityProxyModel::QIdentityProxyModel(QObject *parent)
   : QAbstractProxyModel(*new QIdentityProxyModelPrivate, parent)
{

}

/*! \internal
 */
QIdentityProxyModel::QIdentityProxyModel(QIdentityProxyModelPrivate &dd, QObject *parent)
   : QAbstractProxyModel(dd, parent)
{

}

/*!
    Destroys this identity model.
*/
QIdentityProxyModel::~QIdentityProxyModel()
{
}

/*!
    \reimp
 */
int QIdentityProxyModel::columnCount(const QModelIndex &parent) const
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);
   Q_D(const QIdentityProxyModel);
   return d->model->columnCount(mapToSource(parent));
}

/*!
    \reimp
 */
bool QIdentityProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
   const QModelIndex &parent)
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);
   Q_D(QIdentityProxyModel);
   return d->model->dropMimeData(data, action, row, column, mapToSource(parent));
}

/*!
    \reimp
 */
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

/*!
    \reimp
 */
bool QIdentityProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);
   Q_D(QIdentityProxyModel);
   return d->model->insertColumns(column, count, mapToSource(parent));
}

/*!
    \reimp
 */
bool QIdentityProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);
   Q_D(QIdentityProxyModel);
   return d->model->insertRows(row, count, mapToSource(parent));
}

/*!
    \reimp
 */
QModelIndex QIdentityProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
   Q_D(const QIdentityProxyModel);
   if (!d->model || !sourceIndex.isValid()) {
      return QModelIndex();
   }

   Q_ASSERT(sourceIndex.model() == d->model);
   return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
}

/*!
    \reimp
 */
QItemSelection QIdentityProxyModel::mapSelectionFromSource(const QItemSelection &selection) const
{
   Q_D(const QIdentityProxyModel);
   QItemSelection proxySelection;

   if (!d->model) {
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

/*!
    \reimp
 */
QItemSelection QIdentityProxyModel::mapSelectionToSource(const QItemSelection &selection) const
{
   Q_D(const QIdentityProxyModel);
   QItemSelection sourceSelection;

   if (!d->model) {
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

/*!
    \reimp
 */
QModelIndex QIdentityProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
   Q_D(const QIdentityProxyModel);
   if (!d->model || !proxyIndex.isValid()) {
      return QModelIndex();
   }
   Q_ASSERT(proxyIndex.model() == this);
   return d->model->createIndex(proxyIndex.row(), proxyIndex.column(), proxyIndex.internalPointer());
}

/*!
    \reimp
 */
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

/*!
    \reimp
 */
QModelIndex QIdentityProxyModel::parent(const QModelIndex &child) const
{
   Q_ASSERT(child.isValid() ? child.model() == this : true);
   const QModelIndex sourceIndex = mapToSource(child);
   const QModelIndex sourceParent = sourceIndex.parent();
   return mapFromSource(sourceParent);
}

/*!
    \reimp
 */
bool QIdentityProxyModel::removeColumns(int column, int count, const QModelIndex &parent)
{
   Q_ASSERT(parent.isValid() ? parent.model() == this : true);
   Q_D(QIdentityProxyModel);
   return d->model->removeColumns(column, count, mapToSource(parent));
}

/*!
    \reimp
 */
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
      disconnect(sourceModel(), SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)),
         this, SLOT(_q_sourceRowsAboutToBeInserted(QModelIndex, int, int)));
      disconnect(sourceModel(), SIGNAL(rowsInserted(QModelIndex, int, int)),
         this, SLOT(_q_sourceRowsInserted(QModelIndex, int, int)));
      disconnect(sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
         this, SLOT(_q_sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
      disconnect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex, int, int)),
         this, SLOT(_q_sourceRowsRemoved(QModelIndex, int, int)));
      disconnect(sourceModel(), SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)),
         this, SLOT(_q_sourceRowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
      disconnect(sourceModel(), SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)),
         this, SLOT(_q_sourceRowsMoved(QModelIndex, int, int, QModelIndex, int)));
      disconnect(sourceModel(), SIGNAL(columnsAboutToBeInserted(QModelIndex, int, int)),
         this, SLOT(_q_sourceColumnsAboutToBeInserted(QModelIndex, int, int)));
      disconnect(sourceModel(), SIGNAL(columnsInserted(QModelIndex, int, int)),
         this, SLOT(_q_sourceColumnsInserted(QModelIndex, int, int)));
      disconnect(sourceModel(), SIGNAL(columnsAboutToBeRemoved(QModelIndex, int, int)),
         this, SLOT(_q_sourceColumnsAboutToBeRemoved(QModelIndex, int, int)));

      disconnect(sourceModel(), SIGNAL(columnsRemoved(QModelIndex, int, int)),
         this, SLOT(_q_sourceColumnsRemoved(QModelIndex, int, int)));

      disconnect(sourceModel(), SIGNAL(columnsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)),
         this, SLOT(_q_sourceColumnsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));

      disconnect(sourceModel(), SIGNAL(columnsMoved(QModelIndex, int, int, QModelIndex, int)),
         this, SLOT(_q_sourceColumnsMoved(QModelIndex, int, int, QModelIndex, int)));

      disconnect(sourceModel(), SIGNAL(modelAboutToBeReset()),
         this, SLOT(_q_sourceModelAboutToBeReset()));

      disconnect(sourceModel(), SIGNAL(modelReset()),
         this, SLOT(_q_sourceModelReset()));
      disconnect(sourceModel(), SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)),
         this, SLOT(_q_sourceDataChanged(QModelIndex, QModelIndex, QVector<int>)));
      disconnect(sourceModel(), SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
         this, SLOT(_q_sourceHeaderDataChanged(Qt::Orientation, int, int)));
      disconnect(sourceModel(), SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>, QAbstractItemModel::LayoutChangeHint)),
         this, SLOT(_q_sourceLayoutAboutToBeChanged(QList<QPersistentModelIndex>, QAbstractItemModel::LayoutChangeHint)));
      disconnect(sourceModel(), SIGNAL(layoutChanged(QList<QPersistentModelIndex>, QAbstractItemModel::LayoutChangeHint)),
         this, SLOT(_q_sourceLayoutChanged(QList<QPersistentModelIndex>, QAbstractItemModel::LayoutChangeHint)));
   }

   QAbstractProxyModel::setSourceModel(newSourceModel);

   if (sourceModel()) {
      connect(sourceModel(), SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)), this,
         SLOT(_q_sourceRowsAboutToBeInserted(QModelIndex, int, int)));

      connect(sourceModel(), SIGNAL(rowsInserted(QModelIndex, int, int)), this,
         SLOT(_q_sourceRowsInserted(QModelIndex, int, int)));

      connect(sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)), this,
         SLOT(_q_sourceRowsAboutToBeRemoved(QModelIndex, int, int)));

      connect(sourceModel(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this,
         SLOT(_q_sourceRowsRemoved(QModelIndex, int, int)));

      connect(sourceModel(), SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)), this,
         SLOT(_q_sourceRowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));

      connect(sourceModel(), SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)), this,
         SLOT(_q_sourceRowsMoved(QModelIndex, int, int, QModelIndex, int)));

      connect(sourceModel(), SIGNAL(columnsAboutToBeInserted(QModelIndex, int, int)), this,
         SLOT(_q_sourceColumnsAboutToBeInserted(QModelIndex, int, int)));

      connect(sourceModel(), SIGNAL(columnsInserted(QModelIndex, int, int)), this,
         SLOT(_q_sourceColumnsInserted(QModelIndex, int, int)));

      connect(sourceModel(), SIGNAL(columnsAboutToBeRemoved(QModelIndex, int, int)), this,
         SLOT(_q_sourceColumnsAboutToBeRemoved(QModelIndex, int, int)));

      connect(sourceModel(), SIGNAL(columnsRemoved(QModelIndex, int, int)), this,
         SLOT(_q_sourceColumnsRemoved(QModelIndex, int, int)));

      connect(sourceModel(), SIGNAL(columnsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)), this,
         SLOT(_q_sourceColumnsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));

      connect(sourceModel(), SIGNAL(columnsMoved(QModelIndex, int, int, QModelIndex, int)), this,
         SLOT(_q_sourceColumnsMoved(QModelIndex, int, int, QModelIndex, int)));

      connect(sourceModel(), SIGNAL(modelAboutToBeReset()), this, SLOT(_q_sourceModelAboutToBeReset()));

      connect(sourceModel(), SIGNAL(modelReset()), this, SLOT(_q_sourceModelReset()));

      connect(sourceModel(), SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)), this,
         SLOT(_q_sourceDataChanged(QModelIndex, QModelIndex, QVector<int>)));

      connect(sourceModel(), SIGNAL(headerDataChanged(Qt::Orientation, int, int)), this,
         SLOT(_q_sourceHeaderDataChanged(Qt::Orientation, int, int)));

      connect(sourceModel(), SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>, QAbstractItemModel::LayoutChangeHint)),  this,
         SLOT(_q_sourceLayoutAboutToBeChanged(QList<QPersistentModelIndex>, QAbstractItemModel::LayoutChangeHint)));

      connect(sourceModel(), SIGNAL(layoutChanged(QList<QPersistentModelIndex>, QAbstractItemModel::LayoutChangeHint)), this,
         SLOT(_q_sourceLayoutChanged(QList<QPersistentModelIndex>, QAbstractItemModel::LayoutChangeHint)));
   }

   endResetModel();
}

void QIdentityProxyModelPrivate::_q_sourceColumnsAboutToBeInserted(QModelIndex parent, int start, int end)
{
   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);

   q->beginInsertColumns(q->mapFromSource(parent), start, end);
}

void QIdentityProxyModelPrivate::_q_sourceColumnsAboutToBeMoved(QModelIndex sourceParent, int sourceStart,
   int sourceEnd, QModelIndex destParent, int dest)
{
   Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == model : true);
   Q_ASSERT(destParent.isValid() ? destParent.model() == model : true);
   Q_Q(QIdentityProxyModel);
   q->beginMoveColumns(q->mapFromSource(sourceParent), sourceStart, sourceEnd, q->mapFromSource(destParent), dest);
}

void QIdentityProxyModelPrivate::_q_sourceColumnsAboutToBeRemoved(QModelIndex parent, int start, int end)
{
   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);
   q->beginRemoveColumns(q->mapFromSource(parent), start, end);
}

void QIdentityProxyModelPrivate::_q_sourceColumnsInserted(QModelIndex parent, int start, int end)
{
   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);
   Q_UNUSED(parent)
   Q_UNUSED(start)
   Q_UNUSED(end)
   q->endInsertColumns();
}

void QIdentityProxyModelPrivate::_q_sourceColumnsMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd,
   QModelIndex destParent, int dest)
{
   Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == model : true);
   Q_ASSERT(destParent.isValid() ? destParent.model() == model : true);
   Q_Q(QIdentityProxyModel);
   Q_UNUSED(sourceParent)
   Q_UNUSED(sourceStart)
   Q_UNUSED(sourceEnd)
   Q_UNUSED(destParent)
   Q_UNUSED(dest)
   q->endMoveColumns();
}

void QIdentityProxyModelPrivate::_q_sourceColumnsRemoved(QModelIndex parent, int start, int end)
{
   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);
   Q_UNUSED(parent)
   Q_UNUSED(start)
   Q_UNUSED(end)
   q->endRemoveColumns();
}

void QIdentityProxyModelPrivate::_q_sourceDataChanged(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles)
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

   for (const QPersistentModelIndex &proxyPersistentIndex : q->persistentIndexList()) {
      proxyIndexes << proxyPersistentIndex;
      Q_ASSERT(proxyPersistentIndex.isValid());
      const QPersistentModelIndex srcPersistentIndex = q->mapToSource(proxyPersistentIndex);
      Q_ASSERT(srcPersistentIndex.isValid());
      layoutChangePersistentIndexes << srcPersistentIndex;
   }

   QList<QPersistentModelIndex> parents;

   for (const QPersistentModelIndex &parent : sourceParents) {
      if (!parent.isValid()) {
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
      if (!parent.isValid()) {
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

void QIdentityProxyModelPrivate::_q_sourceRowsAboutToBeInserted(QModelIndex parent, int start, int end)
{
   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);
   q->beginInsertRows(q->mapFromSource(parent), start, end);
}

void QIdentityProxyModelPrivate::_q_sourceRowsAboutToBeMoved(QModelIndex sourceParent, int sourceStart,
   int sourceEnd, QModelIndex destParent, int dest)
{
   Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == model : true);
   Q_ASSERT(destParent.isValid() ? destParent.model() == model : true);
   Q_Q(QIdentityProxyModel);
   q->beginMoveRows(q->mapFromSource(sourceParent), sourceStart, sourceEnd, q->mapFromSource(destParent), dest);
}

void QIdentityProxyModelPrivate::_q_sourceRowsAboutToBeRemoved(QModelIndex parent, int start, int end)
{
   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);
   q->beginRemoveRows(q->mapFromSource(parent), start, end);
}

void QIdentityProxyModelPrivate::_q_sourceRowsInserted(QModelIndex parent, int start, int end)
{
   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);
   Q_UNUSED(parent)
   Q_UNUSED(start)
   Q_UNUSED(end)
   q->endInsertRows();
}

void QIdentityProxyModelPrivate::_q_sourceRowsMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd,
   QModelIndex destParent, int dest)
{
   Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == model : true);
   Q_ASSERT(destParent.isValid() ? destParent.model() == model : true);
   Q_Q(QIdentityProxyModel);
   Q_UNUSED(sourceParent)
   Q_UNUSED(sourceStart)
   Q_UNUSED(sourceEnd)
   Q_UNUSED(destParent)
   Q_UNUSED(dest)
   q->endMoveRows();
}

void QIdentityProxyModelPrivate::_q_sourceRowsRemoved(QModelIndex parent, int start, int end)
{
   Q_ASSERT(parent.isValid() ? parent.model() == model : true);
   Q_Q(QIdentityProxyModel);
   Q_UNUSED(parent)
   Q_UNUSED(start)
   Q_UNUSED(end)
   q->endRemoveRows();
}

void QIdentityProxyModel::_q_sourceRowsAboutToBeInserted(QModelIndex un_named_arg1, int un_named_arg2, int un_named_arg3)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceRowsAboutToBeInserted(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QIdentityProxyModel::_q_sourceRowsInserted(QModelIndex un_named_arg1, int un_named_arg2, int un_named_arg3)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceRowsInserted(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QIdentityProxyModel::_q_sourceRowsAboutToBeRemoved(QModelIndex un_named_arg1, int un_named_arg2,
   int un_named_arg3)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceRowsAboutToBeRemoved(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QIdentityProxyModel::_q_sourceRowsRemoved(QModelIndex un_named_arg1, int un_named_arg2, int un_named_arg3)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceRowsRemoved(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QIdentityProxyModel::_q_sourceRowsAboutToBeMoved(QModelIndex un_named_arg1, int un_named_arg2,
   int un_named_arg3, QModelIndex un_named_arg4, int un_named_arg5)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceRowsAboutToBeMoved(un_named_arg1, un_named_arg2, un_named_arg3, un_named_arg4, un_named_arg5);
}

void QIdentityProxyModel::_q_sourceRowsMoved(QModelIndex un_named_arg1, int un_named_arg2, int un_named_arg3,
   QModelIndex un_named_arg4, int un_named_arg5)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceRowsMoved(un_named_arg1, un_named_arg2, un_named_arg3, un_named_arg4, un_named_arg5);
}

void QIdentityProxyModel::_q_sourceColumnsAboutToBeInserted(QModelIndex un_named_arg1, int un_named_arg2,
   int un_named_arg3)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceColumnsAboutToBeInserted(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QIdentityProxyModel::_q_sourceColumnsInserted(QModelIndex un_named_arg1, int un_named_arg2,
   int un_named_arg3)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceColumnsInserted(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QIdentityProxyModel::_q_sourceColumnsAboutToBeRemoved(QModelIndex un_named_arg1, int un_named_arg2,
   int un_named_arg3)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceColumnsAboutToBeRemoved(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QIdentityProxyModel::_q_sourceColumnsRemoved(QModelIndex un_named_arg1, int un_named_arg2,
   int un_named_arg3)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceColumnsRemoved(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QIdentityProxyModel::_q_sourceColumnsAboutToBeMoved(QModelIndex un_named_arg1, int un_named_arg2,
   int un_named_arg3, QModelIndex un_named_arg4, int un_named_arg5)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceColumnsAboutToBeMoved(un_named_arg1, un_named_arg2, un_named_arg3, un_named_arg4, un_named_arg5);
}

void QIdentityProxyModel::_q_sourceColumnsMoved(QModelIndex un_named_arg1, int un_named_arg2, int un_named_arg3,
   QModelIndex un_named_arg4, int un_named_arg5)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceColumnsMoved(un_named_arg1, un_named_arg2, un_named_arg3, un_named_arg4, un_named_arg5);
}

void QIdentityProxyModel::_q_sourceDataChanged(QModelIndex un_named_arg1, QModelIndex un_named_arg2, QVector<int> roles)
{
   Q_D(QIdentityProxyModel);
   d->_q_sourceDataChanged(un_named_arg1, un_named_arg2, roles);
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
