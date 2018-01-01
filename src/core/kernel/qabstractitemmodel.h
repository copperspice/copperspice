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

#ifndef QABSTRACTITEMMODEL_H
#define QABSTRACTITEMMODEL_H

#include <QtCore/qvariant.h>
#include <QtCore/qobject.h>
#include <QtCore/qhash.h>
#include <QtCore/qset.h>
#include <QScopedPointer>
#include <QtCore/qcontainerfwd.h>

QT_BEGIN_NAMESPACE

class QAbstractItemModel;
class QPersistentModelIndex;
class QPersistentModelIndexData;
class QModelIndex;
class QMimeData;
class QAbstractItemModelPrivate;

typedef QList<QModelIndex> QModelIndexList;

class Q_CORE_EXPORT QModelIndex
{
   friend class QAbstractItemModel;
   friend class QProxyModel;

 public:
   inline QModelIndex() : r(-1), c(-1), p(0), m(0) {}

   inline QModelIndex(const QModelIndex &other)
      : r(other.r), c(other.c), p(other.p), m(other.m) {}

   inline ~QModelIndex() {
      p = 0;
      m = 0;
   }
   inline int row() const {
      return r;
   }
   inline int column() const {
      return c;
   }
   inline void *internalPointer() const {
      return p;
   }
   inline qint64 internalId() const {
      return reinterpret_cast<qint64>(p);
   }
   inline QModelIndex parent() const;
   inline QModelIndex sibling(int row, int column) const;
   inline QModelIndex child(int row, int column) const;
   inline QVariant data(int role = Qt::DisplayRole) const;
   inline Qt::ItemFlags flags() const;
   inline const QAbstractItemModel *model() const {
      return m;
   }
   inline bool isValid() const {
      return (r >= 0) && (c >= 0) && (m != 0);
   }
   inline bool operator==(const QModelIndex &other) const {
      return (other.r == r) && (other.p == p) && (other.c == c) && (other.m == m);
   }
   inline bool operator!=(const QModelIndex &other) const {
      return !(*this == other);
   }
   inline bool operator<(const QModelIndex &other) const {
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
   inline QModelIndex(int row, int column, void *ptr, const QAbstractItemModel *model);
   int r, c;
   void *p;
   const QAbstractItemModel *m;
};
Q_DECLARE_TYPEINFO(QModelIndex, Q_MOVABLE_TYPE);

Q_CORE_EXPORT QDebug operator<<(QDebug, const QModelIndex &);

class Q_CORE_EXPORT QPersistentModelIndex
{
 public:
   QPersistentModelIndex();
   QPersistentModelIndex(const QModelIndex &index);
   QPersistentModelIndex(const QPersistentModelIndex &other);
   ~QPersistentModelIndex();
   bool operator<(const QPersistentModelIndex &other) const;
   bool operator==(const QPersistentModelIndex &other) const;
   inline bool operator!=(const QPersistentModelIndex &other) const {
      return !operator==(other);
   }
   QPersistentModelIndex &operator=(const QPersistentModelIndex &other);
   bool operator==(const QModelIndex &other) const;
   bool operator!=(const QModelIndex &other) const;
   QPersistentModelIndex &operator=(const QModelIndex &other);
   operator const QModelIndex &() const;
   int row() const;
   int column() const;
   void *internalPointer() const;
   qint64 internalId() const;
   QModelIndex parent() const;
   QModelIndex sibling(int row, int column) const;
   QModelIndex child(int row, int column) const;
   QVariant data(int role = Qt::DisplayRole) const;
   Qt::ItemFlags flags() const;
   const QAbstractItemModel *model() const;
   bool isValid() const;

 private:
   QPersistentModelIndexData *d;

   friend uint qHash(const QPersistentModelIndex &);
   friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QPersistentModelIndex &);
};
Q_DECLARE_TYPEINFO(QPersistentModelIndex, Q_MOVABLE_TYPE);

inline uint qHash(const QPersistentModelIndex &index)
{
   return qHash(index.d);
}

Q_CORE_EXPORT QDebug operator<<(QDebug, const QPersistentModelIndex &);

class Q_CORE_EXPORT QAbstractItemModel : public QObject
{
   CORE_CS_OBJECT(QAbstractItemModel)

   friend class QPersistentModelIndexData;
   friend class QAbstractItemViewPrivate;
   friend class QIdentityProxyModel;

 public:
   explicit QAbstractItemModel(QObject *parent = nullptr);
   virtual ~QAbstractItemModel();

   bool hasIndex(int row, int column, const QModelIndex &parent = QModelIndex()) const;
   virtual QModelIndex index(int row, int column,
                             const QModelIndex &parent = QModelIndex()) const = 0;
   virtual QModelIndex parent(const QModelIndex &child) const = 0;

   QModelIndex sibling(int row, int column, const QModelIndex &idx) const {
      return index(row, column, parent(idx));
   }

   virtual int rowCount(const QModelIndex &parent = QModelIndex()) const = 0;
   virtual int columnCount(const QModelIndex &parent = QModelIndex()) const = 0;
   virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

   virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const = 0;
   virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

   virtual QVariant headerData(int section, Qt::Orientation orientation,
                               int role = Qt::DisplayRole) const;
   virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                              int role = Qt::EditRole);

   virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
   virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);

   virtual QStringList mimeTypes() const;
   virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
   virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                             int row, int column, const QModelIndex &parent);
   virtual Qt::DropActions supportedDropActions() const;

   Qt::DropActions supportedDragActions() const;
   void setSupportedDragActions(Qt::DropActions);

   virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
   virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
   virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
   virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

   inline bool insertRow(int row, const QModelIndex &parent = QModelIndex());
   inline bool insertColumn(int column, const QModelIndex &parent = QModelIndex());
   inline bool removeRow(int row, const QModelIndex &parent = QModelIndex());
   inline bool removeColumn(int column, const QModelIndex &parent = QModelIndex());

   virtual void fetchMore(const QModelIndex &parent);
   virtual bool canFetchMore(const QModelIndex &parent) const;
   virtual Qt::ItemFlags flags(const QModelIndex &index) const;
   virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
   virtual QModelIndex buddy(const QModelIndex &index) const;

   virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value,
                                 int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;

   virtual QSize span(const QModelIndex &index) const;

   const QHash<int, QByteArray> &roleNames() const;

   using QObject::parent;

   CORE_CS_SIGNAL_1(Public, void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                    const QSet<int> &roles = QSet<int>()) )

   CORE_CS_SIGNAL_2(dataChanged, topLeft, bottomRight, roles)

   CORE_CS_SIGNAL_1(Public, void headerDataChanged(Qt::Orientation orientation, int first, int last))
   CORE_CS_SIGNAL_2(headerDataChanged, orientation, first, last)

   CORE_CS_SIGNAL_1(Public, void layoutChanged())
   CORE_CS_SIGNAL_2(layoutChanged)

   CORE_CS_SIGNAL_1(Public, void layoutAboutToBeChanged())
   CORE_CS_SIGNAL_2(layoutAboutToBeChanged)

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

   CORE_CS_SIGNAL_1(Public, void rowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart,
                    int sourceEnd, const QModelIndex &destinationParent, int destinationRow))
   CORE_CS_SIGNAL_2(rowsAboutToBeMoved, sourceParent, sourceStart, sourceEnd, destinationParent, destinationRow)

   CORE_CS_SIGNAL_1(Public, void rowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination,
                                           int row))
   CORE_CS_SIGNAL_2(rowsMoved, parent, start, end, destination, row)

   CORE_CS_SIGNAL_1(Public, void columnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                    const QModelIndex &destinationParent, int destinationColumn))
   CORE_CS_SIGNAL_2(columnsAboutToBeMoved, sourceParent, sourceStart, sourceEnd, destinationParent, destinationColumn)

   CORE_CS_SIGNAL_1(Public, void columnsMoved(const QModelIndex &parent, int start, int end,
                    const QModelIndex &destination, int column))
   CORE_CS_SIGNAL_2(columnsMoved, parent, start, end, destination, column)

   //
   CORE_CS_SLOT_1(Public, virtual bool submit())
   CORE_CS_SLOT_2(submit)

   CORE_CS_SLOT_1(Public, virtual void revert())
   CORE_CS_SLOT_2(revert)

 protected:
   QAbstractItemModel(QAbstractItemModelPrivate &dd, QObject *parent = nullptr);

   inline QModelIndex createIndex(int row, int column, void *data = 0) const;
   inline QModelIndex createIndex(int row, int column, int id) const;
   inline QModelIndex createIndex(int row, int column, quint32 id) const;

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

   void reset();

   void beginResetModel();
   void endResetModel();

   void changePersistentIndex(const QModelIndex &from, const QModelIndex &to);
   void changePersistentIndexList(const QModelIndexList &from, const QModelIndexList &to);
   QModelIndexList persistentIndexList() const;

   void setRoleNames(const QHash<int, QByteArray> &roleNames);

   CORE_CS_SLOT_1(Protected, void resetInternalData())
   CORE_CS_SLOT_2(resetInternalData)

   QScopedPointer<QAbstractItemModelPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QAbstractItemModel)
   Q_DISABLE_COPY(QAbstractItemModel)
   
};

inline bool QAbstractItemModel::insertRow(int arow, const QModelIndex &aparent)
{
   return insertRows(arow, 1, aparent);
}

inline bool QAbstractItemModel::insertColumn(int acolumn, const QModelIndex &aparent)
{
   return insertColumns(acolumn, 1, aparent);
}

inline bool QAbstractItemModel::removeRow(int arow, const QModelIndex &aparent)
{
   return removeRows(arow, 1, aparent);
}

inline bool QAbstractItemModel::removeColumn(int acolumn, const QModelIndex &aparent)
{
   return removeColumns(acolumn, 1, aparent);
}

inline QModelIndex QAbstractItemModel::createIndex(int arow, int acolumn, void *adata) const
{
   return QModelIndex(arow, acolumn, adata, this);
}

inline QModelIndex QAbstractItemModel::createIndex(int arow, int acolumn, int aid) const
{
   return QModelIndex(arow, acolumn, reinterpret_cast<void *>(aid), this);
}

inline QModelIndex QAbstractItemModel::createIndex(int arow, int acolumn, quint32 aid) const
{
   return QModelIndex(arow, acolumn, reinterpret_cast<void *>(aid), this);
}


class Q_CORE_EXPORT QAbstractTableModel : public QAbstractItemModel
{
   CORE_CS_OBJECT(QAbstractTableModel)

 public:
   explicit QAbstractTableModel(QObject *parent = nullptr);
   ~QAbstractTableModel();

   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
   bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

 protected:
   QAbstractTableModel(QAbstractItemModelPrivate &dd, QObject *parent);

 private:
   Q_DISABLE_COPY(QAbstractTableModel)
   QModelIndex parent(const QModelIndex &child) const override;
   bool hasChildren(const QModelIndex &parent) const override;
};

class Q_CORE_EXPORT QAbstractListModel : public QAbstractItemModel
{
   CORE_CS_OBJECT(QAbstractListModel)

 public:
   explicit QAbstractListModel(QObject *parent = nullptr);
   ~QAbstractListModel();

   QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;
   bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

 protected:
   QAbstractListModel(QAbstractItemModelPrivate &dd, QObject *parent);

 private:
   Q_DISABLE_COPY(QAbstractListModel)
   QModelIndex parent(const QModelIndex &child) const override;
   int columnCount(const QModelIndex &parent) const override;
   bool hasChildren(const QModelIndex &parent) const override;
};

// inline implementations

inline QModelIndex::QModelIndex(int arow, int acolumn, void *adata, const QAbstractItemModel *amodel)
   : r(arow), c(acolumn), p(adata), m(amodel) {}

inline QModelIndex QModelIndex::parent() const
{
   return m ? m->parent(*this) : QModelIndex();
}

inline QModelIndex QModelIndex::sibling(int arow, int acolumn) const
{
   return m ? (r == arow && c == acolumn) ? *this : m->index(arow, acolumn, m->parent(*this)) : QModelIndex();
}

inline QModelIndex QModelIndex::child(int arow, int acolumn) const
{
   return m ? m->index(arow, acolumn, *this) : QModelIndex();
}

inline QVariant QModelIndex::data(int arole) const
{
   return m ? m->data(*this, arole) : QVariant();
}

inline Qt::ItemFlags QModelIndex::flags() const
{
   return m ? m->flags(*this) : Qt::ItemFlags(0);
}

inline uint qHash(const QModelIndex &index)
{
   return uint((index.row() << 4) + index.column() + index.internalId());
}

QT_END_NAMESPACE


#endif // QABSTRACTITEMMODEL_H
