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

#include <qabstractproxymodel.h>

#ifndef QT_NO_PROXYMODEL

#include <qitemselectionmodel.h>
#include <qabstractproxymodel_p.h>
#include <QtCore/QSize>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

//detects the deletion of the source model
void QAbstractProxyModelPrivate::_q_sourceModelDestroyed()
{
   invalidatePersistentIndexes();
   model = QAbstractItemModelPrivate::staticEmptyModel();
}

/*!
    Constructs a proxy model with the given \a parent.
*/

QAbstractProxyModel::QAbstractProxyModel(QObject *parent)
   : QAbstractItemModel(*new QAbstractProxyModelPrivate, parent)
{
   setSourceModel(QAbstractItemModelPrivate::staticEmptyModel());
}

/*!
    \internal
*/

QAbstractProxyModel::QAbstractProxyModel(QAbstractProxyModelPrivate &dd, QObject *parent)
   : QAbstractItemModel(dd, parent)
{
   setSourceModel(QAbstractItemModelPrivate::staticEmptyModel());
}

/*!
    Destroys the proxy model.
*/
QAbstractProxyModel::~QAbstractProxyModel()
{

}

/*!
    Sets the given \a sourceModel to be processed by the proxy model.
*/
void QAbstractProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
   Q_D(QAbstractProxyModel);
   if (d->model) {
      disconnect(d->model, SIGNAL(destroyed()), this, SLOT(_q_sourceModelDestroyed()));
   }

   if (sourceModel) {
      d->model = sourceModel;
      connect(d->model, SIGNAL(destroyed()), this, SLOT(_q_sourceModelDestroyed()));
   } else {
      d->model = QAbstractItemModelPrivate::staticEmptyModel();
   }
   d->roleNames = d->model->roleNames();
}

/*!
    Returns the model that contains the data that is available through the proxy model.
*/
QAbstractItemModel *QAbstractProxyModel::sourceModel() const
{
   Q_D(const QAbstractProxyModel);
   if (d->model == QAbstractItemModelPrivate::staticEmptyModel()) {
      return 0;
   }
   return d->model;
}

/*!
    \reimp
 */
bool QAbstractProxyModel::submit()
{
   Q_D(QAbstractProxyModel);
   return d->model->submit();
}

/*!
    \reimp
 */
void QAbstractProxyModel::revert()
{
   Q_D(QAbstractProxyModel);
   d->model->revert();
}


/*!
  \fn QModelIndex QAbstractProxyModel::mapToSource(const QModelIndex &proxyIndex) const

  Reimplement this function to return the model index in the source model that
  corresponds to the \a proxyIndex in the proxy model.

  \sa mapFromSource()
*/

/*!
  \fn QModelIndex QAbstractProxyModel::mapFromSource(const QModelIndex &sourceIndex) const

  Reimplement this function to return the model index in the proxy model that
  corresponds to the \a sourceIndex from the source model.

  \sa mapToSource()
*/

/*!
  Returns a source selection mapped from the specified \a proxySelection.

  Reimplement this method to map proxy selections to source selections.
 */
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

/*!
  Returns a proxy selection mapped from the specified \a sourceSelection.

  Reimplement this method to map source selections to proxy selections.
*/
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

/*!
    \reimp
 */
QVariant QAbstractProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
   Q_D(const QAbstractProxyModel);
   return d->model->data(mapToSource(proxyIndex), role);
}

/*!
    \reimp
 */
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

/*!
    \reimp
 */
QMap<int, QVariant> QAbstractProxyModel::itemData(const QModelIndex &proxyIndex) const
{
   return QAbstractItemModel::itemData(proxyIndex);
}

/*!
    \reimp
 */
Qt::ItemFlags QAbstractProxyModel::flags(const QModelIndex &index) const
{
   Q_D(const QAbstractProxyModel);
   return d->model->flags(mapToSource(index));
}

/*!
    \reimp
 */
bool QAbstractProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   Q_D(QAbstractProxyModel);
   return d->model->setData(mapToSource(index), value, role);
}

/*!
    \reimp
 */
bool QAbstractProxyModel::setItemData(const QModelIndex &index, const QMap< int, QVariant > &roles)
{
   return QAbstractItemModel::setItemData(index, roles);
}

/*!
    \reimp
 */
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

/*!
    \reimp
    \since 4.8
 */
QModelIndex QAbstractProxyModel::buddy(const QModelIndex &index) const
{
   Q_D(const QAbstractProxyModel);
   return mapFromSource(d->model->buddy(mapToSource(index)));
}

/*!
    \reimp
    \since 4.8
 */
bool QAbstractProxyModel::canFetchMore(const QModelIndex &parent) const
{
   Q_D(const QAbstractProxyModel);
   return d->model->canFetchMore(mapToSource(parent));
}

/*!
    \reimp
    \since 4.8
 */
void QAbstractProxyModel::fetchMore(const QModelIndex &parent)
{
   Q_D(QAbstractProxyModel);
   d->model->fetchMore(mapToSource(parent));
}

/*!
    \reimp
    \since 4.8
 */
void QAbstractProxyModel::sort(int column, Qt::SortOrder order)
{
   Q_D(QAbstractProxyModel);
   d->model->sort(column, order);
}

/*!
    \reimp
    \since 4.8
 */
QSize QAbstractProxyModel::span(const QModelIndex &index) const
{
   Q_D(const QAbstractProxyModel);
   return d->model->span(mapToSource(index));
}

/*!
    \reimp
    \since 4.8
 */
bool QAbstractProxyModel::hasChildren(const QModelIndex &parent) const
{
   Q_D(const QAbstractProxyModel);
   return d->model->hasChildren(mapToSource(parent));
}

/*!
    \reimp
    \since 4.8
 */
QMimeData *QAbstractProxyModel::mimeData(const QModelIndexList &indexes) const
{
   Q_D(const QAbstractProxyModel);
   QModelIndexList list;
  
   for (const QModelIndex & index : indexes) {
      list << mapToSource(index);
   }

   return d->model->mimeData(list);
}

/*!
    \reimp
    \since 4.8
 */
QStringList QAbstractProxyModel::mimeTypes() const
{
   Q_D(const QAbstractProxyModel);
   return d->model->mimeTypes();
}

/*!
    \reimp
    \since 4.8
 */
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


QT_END_NAMESPACE

#endif // QT_NO_PROXYMODEL
