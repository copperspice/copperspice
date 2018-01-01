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

#ifndef QIDENTITYPROXYMODEL_H
#define QIDENTITYPROXYMODEL_H

#include <QtGui/qabstractproxymodel.h>

#ifndef QT_NO_IDENTITYPROXYMODEL

QT_BEGIN_NAMESPACE

class QIdentityProxyModelPrivate;

class Q_GUI_EXPORT QIdentityProxyModel : public QAbstractProxyModel
{
   GUI_CS_OBJECT(QIdentityProxyModel)

 public:
   explicit QIdentityProxyModel(QObject *parent = nullptr);
   ~QIdentityProxyModel();

   int columnCount(const QModelIndex &parent = QModelIndex()) const override;
   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
   QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
   QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
   QModelIndex parent(const QModelIndex &child) const override;
   int rowCount(const QModelIndex &parent = QModelIndex()) const override;
   bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

   QItemSelection mapSelectionFromSource(const QItemSelection &selection) const override;
   QItemSelection mapSelectionToSource(const QItemSelection &selection) const override;
   QModelIndexList match(const QModelIndex &start, int role, const QVariant &value,
                         int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const override;
   void setSourceModel(QAbstractItemModel *sourceModel) override;

   bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
   bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
   bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
   bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

 protected:
   QIdentityProxyModel(QIdentityProxyModelPrivate &dd, QObject *parent);

 private:
   Q_DECLARE_PRIVATE(QIdentityProxyModel)
   Q_DISABLE_COPY(QIdentityProxyModel)

   GUI_CS_SLOT_1(Private, void _q_sourceRowsAboutToBeInserted(const QModelIndex &un_named_arg1, int un_named_arg2,
                 int un_named_arg3))
   GUI_CS_SLOT_2(_q_sourceRowsAboutToBeInserted)

   GUI_CS_SLOT_1(Private, void _q_sourceRowsInserted(const QModelIndex &un_named_arg1, int un_named_arg2, int un_named_arg3))
   GUI_CS_SLOT_2(_q_sourceRowsInserted)

   GUI_CS_SLOT_1(Private, void _q_sourceRowsAboutToBeRemoved(const QModelIndex &un_named_arg1, int un_named_arg2,
                 int un_named_arg3))
   GUI_CS_SLOT_2(_q_sourceRowsAboutToBeRemoved)

   GUI_CS_SLOT_1(Private, void _q_sourceRowsRemoved(const QModelIndex &un_named_arg1, int un_named_arg2,int un_named_arg3))
   GUI_CS_SLOT_2(_q_sourceRowsRemoved)

   GUI_CS_SLOT_1(Private, void _q_sourceRowsAboutToBeMoved(const QModelIndex &un_named_arg1, int un_named_arg2,
                 int un_named_arg3, const QModelIndex &un_named_arg4, int un_named_arg5))
   GUI_CS_SLOT_2(_q_sourceRowsAboutToBeMoved)

   GUI_CS_SLOT_1(Private, void _q_sourceRowsMoved(const QModelIndex &un_named_arg1, int un_named_arg2,
                 int un_named_arg3, const QModelIndex &un_named_arg4, int un_named_arg5))
   GUI_CS_SLOT_2(_q_sourceRowsMoved)

   GUI_CS_SLOT_1(Private, void _q_sourceColumnsAboutToBeInserted(const QModelIndex &un_named_arg1, int un_named_arg2,
                 int un_named_arg3))
   GUI_CS_SLOT_2(_q_sourceColumnsAboutToBeInserted)

   GUI_CS_SLOT_1(Private, void _q_sourceColumnsInserted(const QModelIndex &un_named_arg1, int un_named_arg2,
                 int un_named_arg3))
   GUI_CS_SLOT_2(_q_sourceColumnsInserted)

   GUI_CS_SLOT_1(Private, void _q_sourceColumnsAboutToBeRemoved(const QModelIndex &un_named_arg1, int un_named_arg2, int un_named_arg3))
   GUI_CS_SLOT_2(_q_sourceColumnsAboutToBeRemoved)

   GUI_CS_SLOT_1(Private, void _q_sourceColumnsRemoved(const QModelIndex &un_named_arg1, int un_named_arg2,
                 int un_named_arg3))
   GUI_CS_SLOT_2(_q_sourceColumnsRemoved)

   GUI_CS_SLOT_1(Private, void _q_sourceColumnsAboutToBeMoved(const QModelIndex &un_named_arg1, int un_named_arg2,
                 int un_named_arg3, const QModelIndex &un_named_arg4, int un_named_arg5))
   GUI_CS_SLOT_2(_q_sourceColumnsAboutToBeMoved)

   GUI_CS_SLOT_1(Private, void _q_sourceColumnsMoved(const QModelIndex &un_named_arg1, int un_named_arg2,
                 int un_named_arg3, const QModelIndex &un_named_arg4, int un_named_arg5))
   GUI_CS_SLOT_2(_q_sourceColumnsMoved)

   GUI_CS_SLOT_1(Private, void _q_sourceDataChanged(const QModelIndex &un_named_arg1, const QModelIndex &un_named_arg2))
   GUI_CS_SLOT_2(_q_sourceDataChanged)

   GUI_CS_SLOT_1(Private, void _q_sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last))
   GUI_CS_SLOT_2(_q_sourceHeaderDataChanged)

   GUI_CS_SLOT_1(Private, void _q_sourceLayoutAboutToBeChanged())
   GUI_CS_SLOT_2(_q_sourceLayoutAboutToBeChanged)

   GUI_CS_SLOT_1(Private, void _q_sourceLayoutChanged())
   GUI_CS_SLOT_2(_q_sourceLayoutChanged)

   GUI_CS_SLOT_1(Private, void _q_sourceModelAboutToBeReset())
   GUI_CS_SLOT_2(_q_sourceModelAboutToBeReset)

   GUI_CS_SLOT_1(Private, void _q_sourceModelReset())
   GUI_CS_SLOT_2(_q_sourceModelReset)

};

QT_END_NAMESPACE

#endif // QT_NO_IDENTITYPROXYMODEL

#endif // QIDENTITYPROXYMODEL_H

