/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QABSTRACTPROXYMODEL_H
#define QABSTRACTPROXYMODEL_H

#include <QtCore/qabstractitemmodel.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PROXYMODEL

class QAbstractProxyModelPrivate;
class QItemSelection;

class Q_GUI_EXPORT QAbstractProxyModel : public QAbstractItemModel
{
   CS_OBJECT(QAbstractProxyModel)

 public:
   QAbstractProxyModel(QObject *parent = 0);
   ~QAbstractProxyModel();

   virtual void setSourceModel(QAbstractItemModel *sourceModel);
   QAbstractItemModel *sourceModel() const;

   virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const = 0;
   virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const = 0;

   virtual QItemSelection mapSelectionToSource(const QItemSelection &selection) const;
   virtual QItemSelection mapSelectionFromSource(const QItemSelection &selection) const;

   bool submit();
   void revert();

   QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const;
   QVariant headerData(int section, Qt::Orientation orientation, int role) const;
   QMap<int, QVariant> itemData(const QModelIndex &index) const;
   Qt::ItemFlags flags(const QModelIndex &index) const;

   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
   bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
   bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);

   QModelIndex buddy(const QModelIndex &index) const;
   bool canFetchMore(const QModelIndex &parent) const;
   void fetchMore(const QModelIndex &parent);
   void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
   QSize span(const QModelIndex &index) const;
   bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

   QMimeData *mimeData(const QModelIndexList &indexes) const;
   QStringList mimeTypes() const;
   Qt::DropActions supportedDropActions() const;

 protected:
   QAbstractProxyModel(QAbstractProxyModelPrivate &, QObject *parent);

 private:
   Q_DECLARE_PRIVATE(QAbstractProxyModel)
   Q_DISABLE_COPY(QAbstractProxyModel)

   GUI_CS_SLOT_1(Private, void _q_sourceModelDestroyed())
   GUI_CS_SLOT_2(_q_sourceModelDestroyed)
};

#endif // QT_NO_PROXYMODEL

QT_END_NAMESPACE

#endif // QABSTRACTPROXYMODEL_H
