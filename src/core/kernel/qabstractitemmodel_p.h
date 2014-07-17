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

#ifndef QABSTRACTITEMMODEL_P_H
#define QABSTRACTITEMMODEL_P_H

#include <QtCore/qstack.h>
#include <QtCore/qset.h>
#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

class QPersistentModelIndexData
{
 public:
   QPersistentModelIndexData() : model(0) {}
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
   QAbstractItemModelPrivate() : supportedDragActions(-1), roleNames(defaultRoleNames()) {}
   virtual ~QAbstractItemModelPrivate() {}


   void removePersistentIndexData(QPersistentModelIndexData *data);
   void movePersistentIndexes(QVector<QPersistentModelIndexData *> indexes, int change,
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

   inline QModelIndex createIndex(int row, int column, void *data = 0) const {
      return q_func()->createIndex(row, column, data);
   }

   inline QModelIndex createIndex(int row, int column, int id) const {
      return q_func()->createIndex(row, column, id);
   }

   inline bool indexValid(const QModelIndex &index) const {
      return (index.row() >= 0) && (index.column() >= 0) && (index.model() == q_func());
   }

   inline void invalidatePersistentIndexes() {
      foreach (QPersistentModelIndexData * data, persistent.indexes) {
         data->index = QModelIndex();
         data->model = 0;
      }
      persistent.indexes.clear();
   }

   /*!
     \internal
     clean the QPersistentModelIndex relative to the index if there is one.
     To be used before an index is invalided
     */
   inline void invalidatePersistentIndex(const QModelIndex &index) {
      QHash<QModelIndex, QPersistentModelIndexData *>::iterator it = persistent.indexes.find(index);
      if (it != persistent.indexes.end()) {
         QPersistentModelIndexData *data = *it;
         persistent.indexes.erase(it);
         data->index = QModelIndex();
         data->model = 0;
      }
   }

   struct Change {
      Change() : first(-1), last(-1) {}
      Change(const Change &c) : parent(c.parent), first(c.first), last(c.last), needsAdjust(c.needsAdjust) {}
      Change(const QModelIndex &p, int f, int l) : parent(p), first(f), last(l), needsAdjust(false) {}
      QModelIndex parent;
      int first, last;


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
      // The QModelIndex is adjusted to correct that in those cases before reporting it though the
      // rowsMoved signal.
      bool needsAdjust;

      bool isValid() {
         return first >= 0 && last >= 0;
      }
   };
   QStack<Change> changes;

   struct Persistent {
      Persistent() {}
      QHash<QModelIndex, QPersistentModelIndexData *> indexes;
      QStack<QVector<QPersistentModelIndexData *> > moved;
      QStack<QVector<QPersistentModelIndexData *> > invalidated;
      void insertMultiAtEnd(const QModelIndex &key, QPersistentModelIndexData *data);
   } persistent;

   Qt::DropActions supportedDragActions;

   QHash<int, QByteArray> roleNames;
   static const QHash<int, QByteArray> &defaultRoleNames();

 protected:
   QAbstractItemModel *q_ptr;
};

QT_END_NAMESPACE

#endif // QABSTRACTITEMMODEL_P_H
