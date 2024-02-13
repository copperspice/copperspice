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

#ifndef QABSTRACTITEMMODEL_H
#define QABSTRACTITEMMODEL_H

#include <qcontainerfwd.h>
#include <qmultihash.h>
#include <qobject.h>
#include <qscopedpointer.h>
#include <qvariant.h>
#include <qvector.h>

class QAbstractItemModel;
class QAbstractItemModelPrivate;
class QMimeData;
class QModelIndex;
class QPersistentModelIndex;
class QPersistentModelIndexData;

using QModelIndexList = QList<QModelIndex>;

class Q_CORE_EXPORT QModelIndex
{
   friend class QAbstractItemModel;

 public:
   QModelIndex()
      : r(-1), c(-1), p(nullptr), m(nullptr)
   {
   }

   // compiler-generated copy/move constructor/assignment operators are fine

   int row() const {
      return r;
   }

   int column() const {
      return c;
   }

   void *internalPointer() const {
      return p;
   }

   qint64 internalId() const {
      return reinterpret_cast<qint64>(p);
   }

   inline QModelIndex parent() const;
   inline QModelIndex sibling(int row, int column) const;
   inline QModelIndex child(int row, int column) const;
   inline QVariant data(int role = Qt::DisplayRole) const;
   inline Qt::ItemFlags flags() const;

   const QAbstractItemModel *model() const {
      return m;
   }

   bool isValid() const {
      return (r >= 0) && (c >= 0) && (m != nullptr);
   }

   bool operator==(const QModelIndex &other) const {
      return (other.r == r) && (other.p == p) && (other.c == c) && (other.m == m);
   }

   bool operator!=(const QModelIndex &other) const {
      return !(*this == other);
   }

   bool operator<(const QModelIndex &other) const {
      if (r < other.r) {
         return true;
      }

      if (r == other.r) {
         if (c < other.c) {
            return true;
         }

         if (c == other.c) {
            if (p < other.p) {
               return true;
            }

            if (p == other.p) {
               return m < other.m;
            }
         }
      }

      return false;
   }

 private:
   QModelIndex(int row, int column, void *ptr, const QAbstractItemModel *model)
      : r(row), c(column), p(ptr), m(model)
   {
   }

   int r;
   int c;
   void *p;
   const QAbstractItemModel *m;
};

Q_CORE_EXPORT QDebug operator<<(QDebug, const QModelIndex &);
uint qHash(const QPersistentModelIndex &index, uint seed = 0);

class Q_CORE_EXPORT QPersistentModelIndex
{
 public:
   QPersistentModelIndex();
   QPersistentModelIndex(const QModelIndex &index);
   QPersistentModelIndex(const QPersistentModelIndex &other);
   ~QPersistentModelIndex();

   bool operator<(const QPersistentModelIndex &other) const;
   bool operator==(const QPersistentModelIndex &other) const;

   bool operator!=(const QPersistentModelIndex &other) const {
      return !operator==(other);
   }

   QPersistentModelIndex &operator=(const QPersistentModelIndex &other);
   QPersistentModelIndex(QPersistentModelIndex &&other)
      : d(other.d)
   {
      other.d = nullptr;
   }

   QPersistentModelIndex &operator=(QPersistentModelIndex &&other) {
      qSwap(d, other.d);
      return *this;
   }

   void swap(QPersistentModelIndex &other)   {
      qSwap(d, other.d);
   }

   bool operator==(const QModelIndex &other) const;
   bool operator!=(const QModelIndex &other) const;
   QPersistentModelIndex &operator=(const QModelIndex &other);
   operator const QModelIndex &() const;
   int row() const;
   int column() const;
   void *internalPointer() const;

   quintptr internalId() const;

   QModelIndex parent() const;
   QModelIndex sibling(int row, int column) const;
   QModelIndex child(int row, int column) const;
   QVariant data(int role = Qt::DisplayRole) const;
   Qt::ItemFlags flags() const;
   const QAbstractItemModel *model() const;
   bool isValid() const;

 private:
   QPersistentModelIndexData *d;

   friend uint qHash(const QPersistentModelIndex &, uint seed);
   friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QPersistentModelIndex &);
};

inline uint qHash(const QPersistentModelIndex &index, uint seed)
{
   return qHash(index.d, seed);
}

Q_CORE_EXPORT QDebug operator<<(QDebug, const QPersistentModelIndex &);

class Q_CORE_EXPORT QAbstractItemModel : public QObject
{
   CORE_CS_OBJECT(QAbstractItemModel)

   CORE_CS_ENUM(LayoutChangeHint)

 public:
   enum LayoutChangeHint {
      NoLayoutChangeHint,
      VerticalSortHint,
      HorizontalSortHint
   };

   explicit QAbstractItemModel(QObject *parent = nullptr);

   QAbstractItemModel(const QAbstractItemModel &) = delete;
   QAbstractItemModel &operator=(const QAbstractItemModel &) = delete;

   virtual ~QAbstractItemModel();

   bool hasIndex(int row, int column, const QModelIndex &parent = QModelIndex()) const;
   virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const = 0;
   virtual QModelIndex parent(const QModelIndex &index) const = 0;
   virtual QModelIndex sibling(int row, int column, const QModelIndex &index) const;

   virtual int rowCount(const QModelIndex &parent = QModelIndex()) const = 0;
   virtual int columnCount(const QModelIndex &parent = QModelIndex()) const = 0;
   virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

   virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const = 0;
   virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

   virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
   virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);

   virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
   virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);

   virtual QStringList mimeTypes() const;
   virtual QMimeData *mimeData(const QModelIndexList &indexes) const;

   virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action,
         int row, int column, const QModelIndex &parent) const;

   virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
         int row, int column, const QModelIndex &parent);

   virtual Qt::DropActions supportedDropActions() const;
   virtual Qt::DropActions supportedDragActions() const;

   virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
   virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
   virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
   virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

   virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
         const QModelIndex &destinationParent, int destinationChild);
   virtual bool moveColumns(const QModelIndex &sourceParent, int sourceColumn, int count,
         const QModelIndex &destinationParent, int destinationChild);

   inline bool insertRow(int row, const QModelIndex &parent = QModelIndex());
   inline bool insertColumn(int column, const QModelIndex &parent = QModelIndex());
   inline bool removeRow(int row, const QModelIndex &parent = QModelIndex());
   inline bool removeColumn(int column, const QModelIndex &parent = QModelIndex());

   inline bool moveRow(const QModelIndex &sourceParent, int sourceRow,
         const QModelIndex &destinationParent, int destinationChild);
   inline bool moveColumn(const QModelIndex &sourceParent, int sourceColumn,
         const QModelIndex &destinationParent, int destinationChild);

   virtual void fetchMore(const QModelIndex &parent);
   virtual bool canFetchMore(const QModelIndex &parent) const;
   virtual Qt::ItemFlags flags(const QModelIndex &index) const;
   virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
   virtual QModelIndex buddy(const QModelIndex &index) const;

   virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1,
         Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;

   virtual QSize span(const QModelIndex &index) const;

   virtual QMultiHash<int, QString> roleNames() const;

   using QObject::parent;

   CORE_CS_SIGNAL_1(Public, void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
               const QVector<int> &roles = QVector<int>()))
   CORE_CS_SIGNAL_2(dataChanged, topLeft, bottomRight, roles)

   CORE_CS_SIGNAL_1(Public, void headerDataChanged(Qt::Orientation orientation, int first, int last))
   CORE_CS_SIGNAL_2(headerDataChanged, orientation, first, last)

   CORE_CS_SIGNAL_1(Public, void layoutChanged(const QList<QPersistentModelIndex> &parents = QList<QPersistentModelIndex>(),
               QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint))
   CORE_CS_SIGNAL_2(layoutChanged, parents, hint)

   CORE_CS_SIGNAL_1(Public, void layoutAboutToBeChanged(const QList<QPersistentModelIndex> &parents = QList<QPersistentModelIndex>(),
               QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint))
   CORE_CS_SIGNAL_2(layoutAboutToBeChanged, parents, hint)

   CORE_CS_SIGNAL_1(Public, void rowsAboutToBeInserted(const QModelIndex &parent, int first, int last))
   CORE_CS_SIGNAL_2(rowsAboutToBeInserted, parent, first, last)

   CORE_CS_SIGNAL_1(Public, void rowsInserted(const QModelIndex &parent, int first, int last))
   CORE_CS_SIGNAL_2(rowsInserted, parent, first, last)

   CORE_CS_SIGNAL_1(Public, void rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last))
   CORE_CS_SIGNAL_2(rowsAboutToBeRemoved, parent, first, last)

   CORE_CS_SIGNAL_1(Public, void rowsRemoved(const QModelIndex &parent, int first, int last))
   CORE_CS_SIGNAL_2(rowsRemoved, parent, first, last)

   CORE_CS_SIGNAL_1(Public, void columnsAboutToBeInserted(const QModelIndex &parent, int first, int last))
   CORE_CS_SIGNAL_2(columnsAboutToBeInserted, parent, first, last)

   CORE_CS_SIGNAL_1(Public, void columnsInserted(const QModelIndex &parent, int first, int last))
   CORE_CS_SIGNAL_2(columnsInserted, parent, first, last)

   CORE_CS_SIGNAL_1(Public, void columnsAboutToBeRemoved(const QModelIndex &parent, int first, int last))
   CORE_CS_SIGNAL_2(columnsAboutToBeRemoved, parent, first, last)

   CORE_CS_SIGNAL_1(Public, void columnsRemoved(const QModelIndex &parent, int first, int last))
   CORE_CS_SIGNAL_2(columnsRemoved, parent, first, last)

   CORE_CS_SIGNAL_1(Public, void modelAboutToBeReset())
   CORE_CS_SIGNAL_2(modelAboutToBeReset)

   CORE_CS_SIGNAL_1(Public, void modelReset())
   CORE_CS_SIGNAL_2(modelReset)

   CORE_CS_SIGNAL_1(Public, void rowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
               const QModelIndex &destinationParent, int destinationRow))
   CORE_CS_SIGNAL_2(rowsAboutToBeMoved, sourceParent, sourceStart, sourceEnd, destinationParent, destinationRow)

   CORE_CS_SIGNAL_1(Public, void rowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
               const QModelIndex &destinationParent, int destinationRow))
   CORE_CS_SIGNAL_2(rowsMoved, sourceParent, sourceStart, sourceEnd, destinationParent, destinationRow)

   CORE_CS_SIGNAL_1(Public, void columnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
               const QModelIndex &destinationParent, int destinationColumn))
   CORE_CS_SIGNAL_2(columnsAboutToBeMoved, sourceParent, sourceStart, sourceEnd, destinationParent, destinationColumn)

   CORE_CS_SIGNAL_1(Public, void columnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
               const QModelIndex &destinationParent, int destinationColumn))
   CORE_CS_SIGNAL_2(columnsMoved, sourceParent, sourceStart, sourceEnd, destinationParent, destinationColumn)

   //
   CORE_CS_SLOT_1(Public, virtual bool submit())
   CORE_CS_SLOT_2(submit)

   CORE_CS_SLOT_1(Public, virtual void revert())
   CORE_CS_SLOT_2(revert)

 protected:
   QAbstractItemModel(QAbstractItemModelPrivate &dd, QObject *parent = nullptr);

   inline QModelIndex createIndex(int row, int column, void *data = nullptr) const;
   inline QModelIndex createIndex(int row, int column, quintptr id) const;

   void encodeData(const QModelIndexList &indexes, QDataStream &stream) const;
   bool decodeData(int row, int column, const QModelIndex &parent, QDataStream &stream);

   void beginInsertRows(const QModelIndex &parent, int first, int last);
   void endInsertRows();

   void beginRemoveRows(const QModelIndex &parent, int first, int last);
   void endRemoveRows();

   bool beginMoveRows(const QModelIndex &sourceParent, int sourceFirst, int sourceLast,
         const QModelIndex &destinationParent, int destinationRow);
   void endMoveRows();

   void beginInsertColumns(const QModelIndex &parent, int first, int last);
   void endInsertColumns();

   void beginRemoveColumns(const QModelIndex &parent, int first, int last);
   void endRemoveColumns();

   bool beginMoveColumns(const QModelIndex &sourceParent, int sourceFirst, int sourceLast,
         const QModelIndex &destinationParent, int destinationColumn);

   void endMoveColumns();
   void beginResetModel();
   void endResetModel();

   void changePersistentIndex(const QModelIndex &from, const QModelIndex &to);
   void changePersistentIndexList(const QModelIndexList &from, const QModelIndexList &to);
   QModelIndexList persistentIndexList() const;

   CORE_CS_SLOT_1(Protected, void resetInternalData())
   CORE_CS_SLOT_2(resetInternalData)

   QScopedPointer<QAbstractItemModelPrivate> d_ptr;

 private:
   void doSetRoleNames(const QMultiHash<int, QString> &roleNames);
   void doSetSupportedDragActions(Qt::DropActions actions);
   Q_DECLARE_PRIVATE(QAbstractItemModel)

   friend class QPersistentModelIndexData;
   friend class QAbstractItemViewPrivate;
   friend class QIdentityProxyModel;
};

inline bool QAbstractItemModel::insertRow(int row, const QModelIndex &parent)
{
   return insertRows(row, 1, parent);
}

inline bool QAbstractItemModel::insertColumn(int column, const QModelIndex &parent)
{
   return insertColumns(column, 1, parent);
}

inline bool QAbstractItemModel::removeRow(int row, const QModelIndex &parent)
{
   return removeRows(row, 1, parent);
}

inline bool QAbstractItemModel::removeColumn(int column, const QModelIndex &parent)
{
   return removeColumns(column, 1, parent);
}

inline bool QAbstractItemModel::moveRow(const QModelIndex &sourceParent, int sourceRow,
      const QModelIndex &destinationParent, int destinationChild)
{
   return moveRows(sourceParent, sourceRow, 1, destinationParent, destinationChild);
}

inline bool QAbstractItemModel::moveColumn(const QModelIndex &sourceParent, int sourceColumn,
      const QModelIndex &destinationParent, int destinationChild)
{
   return moveColumns(sourceParent, sourceColumn, 1, destinationParent, destinationChild);
}

inline QModelIndex QAbstractItemModel::createIndex(int row, int column, void *data) const
{
   return QModelIndex(row, column, data, this);
}

inline QModelIndex QAbstractItemModel::createIndex(int row, int column, quintptr id) const
{
   return QModelIndex(row, column, reinterpret_cast<void *>(id), this);
}

class Q_CORE_EXPORT QAbstractTableModel : public QAbstractItemModel
{
   CORE_CS_OBJECT(QAbstractTableModel)

 public:
   explicit QAbstractTableModel(QObject *parent = nullptr);

   QAbstractTableModel(const QAbstractTableModel &) = delete;
   QAbstractTableModel &operator=(const QAbstractTableModel &) = delete;

   ~QAbstractTableModel();

   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
   QModelIndex sibling(int row, int column, const QModelIndex &index) const override;
   bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

   Qt::ItemFlags flags(const QModelIndex &index) const override;
   using QObject::parent;

 protected:
   QAbstractTableModel(QAbstractItemModelPrivate &dd, QObject *parent);

 private:
   QModelIndex parent(const QModelIndex &child) const override;
   bool hasChildren(const QModelIndex &parent) const override;
};

class Q_CORE_EXPORT QAbstractListModel : public QAbstractItemModel
{
   CORE_CS_OBJECT(QAbstractListModel)

 public:
   explicit QAbstractListModel(QObject *parent = nullptr);

   QAbstractListModel(const QAbstractListModel &) = delete;
   QAbstractListModel &operator=(const QAbstractListModel &) = delete;

   ~QAbstractListModel();

   QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;
   QModelIndex sibling(int row, int column, const QModelIndex &index) const override;

   bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

   Qt::ItemFlags flags(const QModelIndex &index) const override;
   using QObject::parent;

 protected:
   QAbstractListModel(QAbstractItemModelPrivate &dd, QObject *parent);

 private:
   QModelIndex parent(const QModelIndex &child) const override;
   int columnCount(const QModelIndex &parent) const override;
   bool hasChildren(const QModelIndex &parent) const override;
};

inline QModelIndex QModelIndex::parent() const
{
   return m ? m->parent(*this) : QModelIndex();
}

inline QModelIndex QModelIndex::sibling(int row, int column) const
{
   return m ? (r == row && c == column) ? *this : m->sibling(row, column, *this) : QModelIndex();
}

inline QModelIndex QModelIndex::child(int row, int column) const
{
   return m ? m->index(row, column, *this) : QModelIndex();
}

inline QVariant QModelIndex::data(int role) const
{
   return m ? m->data(*this, role) : QVariant();
}

inline Qt::ItemFlags QModelIndex::flags() const
{
   return m ? m->flags(*this) : Qt::ItemFlags();
}

inline uint qHash(const QModelIndex &index)
{
   return uint((index.row() << 4) + index.column() + index.internalId());
}

#endif
