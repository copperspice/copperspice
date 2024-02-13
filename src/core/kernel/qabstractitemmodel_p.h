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

#ifndef QABSTRACTITEMMODEL_P_H
#define QABSTRACTITEMMODEL_P_H

#include <qmultihash.h>
#include <qstack.h>
#include <qset.h>

#include <optional>

class QPersistentModelIndexData
{
 public:
   QPersistentModelIndexData() : model(nullptr) {}
   QPersistentModelIndexData(const QModelIndex &idx) : index(idx), model(idx.model()) {}
   QModelIndex index;
   QAtomicInt ref;
   const QAbstractItemModel *model;
   static QPersistentModelIndexData *create(const QModelIndex &index);
   static void destroy(QPersistentModelIndexData *data);
};

class Q_CORE_EXPORT QAbstractItemModelPrivate
{
   Q_DECLARE_PUBLIC(QAbstractItemModel)

 public:
   QAbstractItemModelPrivate()
      : roleNames(defaultRoleNames())
   {
   }

   virtual ~QAbstractItemModelPrivate();

   void removePersistentIndexData(QPersistentModelIndexData *data);
   void movePersistentIndexes(const QVector<QPersistentModelIndexData *> &indexes, int change,
         const QModelIndex &parent, Qt::Orientation orientation);

   void rowsAboutToBeInserted(const QModelIndex &parent, int first, int last);
   void rowsInserted(const QModelIndex &parent, int first, int last);
   void rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
   void rowsRemoved(const QModelIndex &parent, int first, int last);
   void columnsAboutToBeInserted(const QModelIndex &parent, int first, int last);
   void columnsInserted(const QModelIndex &parent, int first, int last);
   void columnsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
   void columnsRemoved(const QModelIndex &parent, int first, int last);
   static QAbstractItemModel *staticEmptyModel();
   static bool variantLessThan(const QVariant &v1, const QVariant &v2);

   void itemsAboutToBeMoved(const QModelIndex &srcParent, int srcFirst, int srcLast,
         const QModelIndex &destinationParent, int destinationChild, Qt::Orientation);

   void itemsMoved(const QModelIndex &srcParent, int srcFirst, int srcLast,
         const QModelIndex &destinationParent, int destinationChild, Qt::Orientation orientation);

   bool allowMove(const QModelIndex &srcParent, int srcFirst, int srcLast,
         const QModelIndex &destinationParent, int destinationChild, Qt::Orientation orientation);

   QModelIndex createIndex(int row, int column, void *data = nullptr) const {
      return q_func()->createIndex(row, column, data);
   }

   QModelIndex createIndex(int row, int column, int id) const {
      return q_func()->createIndex(row, column, id);
   }

   bool indexValid(const QModelIndex &index) const {
      return (index.row() >= 0) && (index.column() >= 0) && (index.model() == q_func());
   }

   void invalidatePersistentIndexes() {
      for (QPersistentModelIndexData *data : persistent.m_indexes) {
         data->index = QModelIndex();
         data->model = nullptr;
      }

      persistent.m_indexes.clear();
   }

   void invalidatePersistentIndex(const QModelIndex &index) {
      QMultiMap<QModelIndex, QPersistentModelIndexData *>::iterator it = persistent.m_indexes.find(index);

      if (it != persistent.m_indexes.end()) {
         QPersistentModelIndexData *data = *it;
         persistent.m_indexes.erase(it);
         data->index = QModelIndex();
         data->model = nullptr;
      }
   }

   struct Change {
      Change() : first(-1), last(-1) {}
      Change(const Change &c) : parent(c.parent), first(c.first), last(c.last), needsAdjust(c.needsAdjust) {}
      Change(const QModelIndex &p, int f, int l) : parent(p), first(f), last(l), needsAdjust(false) {}

      QModelIndex parent;
      int first;
      int last;

      // In cases such as this:
      // - A
      // - B
      // - C
      // - - D
      // - - E
      // - - F
      //
      // If B is moved to above E, C is the source parent in the signal and its row is 2. When the move is
      // completed however, C is at row 1 and there is no row 2 at the same level in the model at all.
      // The QModelIndex is adjusted to correct that in those cases before reporting it though the rowsMoved signal.
      bool needsAdjust;

      bool isValid() {
         return first >= 0 && last >= 0;
      }
   };
   QStack<Change> changes;

   struct Persistent {
      Persistent() {}
      QMultiMap<QModelIndex, QPersistentModelIndexData *> m_indexes;

      QStack<QVector<QPersistentModelIndexData *>> moved;
      QStack<QVector<QPersistentModelIndexData *>> invalidated;

      void insertMultiAtEnd(const QModelIndex &key, QPersistentModelIndexData *data);
   } persistent;

   std::optional<Qt::DropActions> supportedDragActions;

   QMultiHash<int, QString> roleNames;
   static const QMultiHash<int, QString> &defaultRoleNames();

 protected:
   QAbstractItemModel *q_ptr;
};

#endif
