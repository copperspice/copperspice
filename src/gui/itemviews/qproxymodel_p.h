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

#ifndef QPROXYMODEL_P_H
#define QPROXYMODEL_P_H

#include <QtCore/qabstractitemmodel.h>
#include <qabstractitemmodel_p.h>

#ifndef QT_NO_PROXYMODEL

QT_BEGIN_NAMESPACE

class QEmptyModel : public QAbstractItemModel
{
 public:
   explicit QEmptyModel(QObject *parent = nullptr) : QAbstractItemModel(parent) {}

   QModelIndex index(int, int, const QModelIndex &) const override {
      return QModelIndex();
   }

   QModelIndex parent(const QModelIndex &) const override {
      return QModelIndex();
   }

   int rowCount(const QModelIndex &) const override {
      return 0;
   }

   int columnCount(const QModelIndex &) const override {
      return 0;
   }

   bool hasChildren(const QModelIndex &) const override {
      return false;
   }

   QVariant data(const QModelIndex &, int) const override {
      return QVariant();
   }
};

class QProxyModelPrivate : private QAbstractItemModelPrivate
{
   Q_DECLARE_PUBLIC(QProxyModel)

 public:
   void _q_sourceDataChanged(const QModelIndex &tl, const QModelIndex &br);
   void _q_sourceRowsAboutToBeInserted(const QModelIndex &parent, int first , int last);
   void _q_sourceRowsInserted(const QModelIndex &parent, int first , int last);
   void _q_sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
   void _q_sourceRowsRemoved(const QModelIndex &parent, int first, int last);
   void _q_sourceColumnsAboutToBeInserted(const QModelIndex &parent, int first, int last);
   void _q_sourceColumnsInserted(const QModelIndex &parent, int first, int last);
   void _q_sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
   void _q_sourceColumnsRemoved(const QModelIndex &parent, int first, int last);

   QProxyModelPrivate() : QAbstractItemModelPrivate(), model(0) {}
   QAbstractItemModel *model;
   QEmptyModel empty;
};

QT_END_NAMESPACE

#endif // QT_NO_PROXYMODEL

#endif // QPROXYMODEL_P_H
