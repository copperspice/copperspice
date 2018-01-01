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

#ifndef QABSTRACTPROXYMODEL_H
#define QABSTRACTPROXYMODEL_H

#include <QtCore/qabstractitemmodel.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PROXYMODEL

class QAbstractProxyModelPrivate;
class QItemSelection;

class Q_GUI_EXPORT QAbstractProxyModel : public QAbstractItemModel
{
   GUI_CS_OBJECT(QAbstractProxyModel)

 public:
   QAbstractProxyModel(QObject *parent = nullptr);
   ~QAbstractProxyModel();

   virtual void setSourceModel(QAbstractItemModel *sourceModel);
   QAbstractItemModel *sourceModel() const;

   virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const = 0;
   virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const = 0;

   virtual QItemSelection mapSelectionToSource(const QItemSelection &selection) const;
   virtual QItemSelection mapSelectionFromSource(const QItemSelection &selection) const;

   bool submit() override;
   void revert() override;

   QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const override;
   QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
   QMap<int, QVariant> itemData(const QModelIndex &index) const override;
   Qt::ItemFlags flags(const QModelIndex &index) const override;

   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
   bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) override;
   bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

   QModelIndex buddy(const QModelIndex &index) const override;
   bool canFetchMore(const QModelIndex &parent) const override;
   void fetchMore(const QModelIndex &parent) override;
   void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
   QSize span(const QModelIndex &index) const override;
   bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

   QMimeData *mimeData(const QModelIndexList &indexes) const override;
   QStringList mimeTypes() const override;
   Qt::DropActions supportedDropActions() const override;

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
