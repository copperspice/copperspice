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

#ifndef QSTANDARDITEMMODEL_P_H
#define QSTANDARDITEMMODEL_P_H

#include <qabstractitemmodel_p.h>

#ifndef QT_NO_STANDARDITEMMODEL

#include <qlist.h>
#include <qpair.h>
#include <qstack.h>
#include <qvariant.h>
#include <qvector.h>

class QStandardItemData
{
 public:
   inline QStandardItemData() : role(-1) {}
   inline QStandardItemData(int r, const QVariant &v) : role(r), value(v) {}
   int role;
   QVariant value;
   inline bool operator==(const QStandardItemData &other) const {
      return role == other.role && value == other.value;
   }
};

inline QDataStream &operator>>(QDataStream &in, QStandardItemData &data)
{
   in >> data.role;
   in >> data.value;
   return in;
}

inline QDataStream &operator<<(QDataStream &out, const QStandardItemData &data)
{
   out << data.role;
   out << data.value;
   return out;
}

class QStandardItemPrivate
{
   Q_DECLARE_PUBLIC(QStandardItem)

 public:
   inline QStandardItemPrivate()
      : model(nullptr), parent(nullptr), rows(0), columns(0), q_ptr(nullptr), lastIndexOf(2)
   {
   }

   virtual ~QStandardItemPrivate();

   inline int childIndex(int row, int column) const {
      if ((row < 0) || (column < 0)
         || (row >= rowCount()) || (column >= columnCount())) {
         return -1;
      }
      return (row * columnCount()) + column;
   }
   inline int childIndex(const QStandardItem *child) {
      int start = qMax(0, lastIndexOf - 2);
      lastIndexOf = children.indexOf(const_cast<QStandardItem *>(child), start);
      if (lastIndexOf == -1 && start != 0) {
         lastIndexOf = children.lastIndexOf(const_cast<QStandardItem *>(child), start);
      }
      return lastIndexOf;
   }

   QPair<int, int> position() const;
   void setChild(int row, int column, QStandardItem *item,
      bool emitChanged = false);
   inline int rowCount() const {
      return rows;
   }

   inline int columnCount() const {
      return columns;
   }

   void childDeleted(QStandardItem *child);
   void setModel(QStandardItemModel *mod);

   inline void setParentAndModel(
      QStandardItem *par,
      QStandardItemModel *mod) {
      setModel(mod);
      parent = par;
   }

   void changeFlags(bool enable, Qt::ItemFlags f);
   void setItemData(const QMap<int, QVariant> &roles);
   const QMap<int, QVariant> itemData() const;

   bool insertRows(int row, int count, const QList<QStandardItem *> &items);
   bool insertRows(int row, const QList<QStandardItem *> &items);
   bool insertColumns(int column, int count, const QList<QStandardItem *> &items);

   void sortChildren(int column, Qt::SortOrder order);

   QStandardItemModel *model;
   QStandardItem *parent;
   QVector<QStandardItemData> values;
   QVector<QStandardItem *> children;
   int rows;
   int columns;

   QStandardItem *q_ptr;

   int lastIndexOf;
};

class QStandardItemModelPrivate : public QAbstractItemModelPrivate
{
   Q_DECLARE_PUBLIC(QStandardItemModel)

 public:
   QStandardItemModelPrivate();
   virtual ~QStandardItemModelPrivate();

   void init();

   inline QStandardItem *createItem() const {
      return itemPrototype ? itemPrototype->clone() : new QStandardItem;
   }

   inline QStandardItem *itemFromIndex(const QModelIndex &index) const {
      Q_Q(const QStandardItemModel);

      if (!index.isValid()) {
         return root.data();
      }

      if (index.model() != q) {
         return nullptr;
      }

      QStandardItem *parent = static_cast<QStandardItem *>(index.internalPointer());
      if (parent == nullptr) {
         return nullptr;
      }

      return parent->child(index.row(), index.column());
   }

   void sort(QStandardItem *parent, int column, Qt::SortOrder order);
   void itemChanged(QStandardItem *item);
   void rowsAboutToBeInserted(QStandardItem *parent, int start, int end);
   void columnsAboutToBeInserted(QStandardItem *parent, int start, int end);
   void rowsAboutToBeRemoved(QStandardItem *parent, int start, int end);
   void columnsAboutToBeRemoved(QStandardItem *parent, int start, int end);
   void rowsInserted(QStandardItem *parent, int row, int count);
   void columnsInserted(QStandardItem *parent, int column, int count);
   void rowsRemoved(QStandardItem *parent, int row, int count);
   void columnsRemoved(QStandardItem *parent, int column, int count);

   void _q_emitItemChanged(const QModelIndex &topLeft,
      const QModelIndex &bottomRight);

   void decodeDataRecursive(QDataStream &stream, QStandardItem *item);

   QVector<QStandardItem *> columnHeaderItems;
   QVector<QStandardItem *> rowHeaderItems;
   QScopedPointer<QStandardItem> root;
   const QStandardItem *itemPrototype;
   int sortRole;
};



#endif // QT_NO_STANDARDITEMMODEL

#endif // QSTANDARDITEMMODEL_P_H
