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

#include <qabstractitemmodel.h>
#include <qabstractitemmodel_p.h>
#include <qdatastream.h>
#include <qstringlist.h>
#include <qsize.h>
#include <qmimedata.h>
#include <qdebug.h>
#include <qvector.h>
#include <qstack.h>
#include <qbitarray.h>

#include <limits.h>

QT_BEGIN_NAMESPACE

QPersistentModelIndexData *QPersistentModelIndexData::create(const QModelIndex &index)
{
   Q_ASSERT(index.isValid());          // we will _never_ insert an invalid index in the list
   QPersistentModelIndexData *d = 0;
   QAbstractItemModel *model = const_cast<QAbstractItemModel *>(index.model());

   QMultiMap<QModelIndex, QPersistentModelIndexData *> &tmpIndex = model->d_func()->persistent.m_indexes;
   const QMultiMap<QModelIndex, QPersistentModelIndexData *>::iterator it = tmpIndex.find(index);

   if (it != tmpIndex.end()) {
      d = (*it);

   } else {
      d = new QPersistentModelIndexData(index);
      tmpIndex.insert(index, d);
   }

   Q_ASSERT(d);
   return d;
}

void QPersistentModelIndexData::destroy(QPersistentModelIndexData *data)
{
   Q_ASSERT(data);
   Q_ASSERT(data->ref.load() == 0);

   QAbstractItemModel *model = const_cast<QAbstractItemModel *>(data->model);

   // a valid persistent model index with a null model pointer can only happen if the model was destroyed
   if (model) {
      QAbstractItemModelPrivate *p = model->d_func();
      Q_ASSERT(p);
      p->removePersistentIndexData(data);
   }
   delete data;
}

QPersistentModelIndex::QPersistentModelIndex()
   : d(0)
{
}

QPersistentModelIndex::QPersistentModelIndex(const QPersistentModelIndex &other)
   : d(other.d)
{
   if (d) {
      d->ref.ref();
   }
}

QPersistentModelIndex::QPersistentModelIndex(const QModelIndex &index)
   : d(0)
{
   if (index.isValid()) {
      d = QPersistentModelIndexData::create(index);
      d->ref.ref();
   }
}

QPersistentModelIndex::~QPersistentModelIndex()
{
   if (d && !d->ref.deref()) {
      QPersistentModelIndexData::destroy(d);
      d = 0;
   }
}

/*!
  Returns true if this persistent model index is equal to the \a other
  persistent model index; otherwise returns false.

  All values in the persistent model index are used when comparing
  with another persistent model index.
*/

bool QPersistentModelIndex::operator==(const QPersistentModelIndex &other) const
{
   if (d && other.d) {
      return d->index == other.d->index;
   }
   return d == other.d;
}

/*!
    \since 4.1

    Returns true if this persistent model index is smaller than the \a other
    persistent model index; otherwise returns false.

    All values in the persistent model index are used when comparing
    with another persistent model index.
*/

bool QPersistentModelIndex::operator<(const QPersistentModelIndex &other) const
{
   if (d && other.d) {
      return d->index < other.d->index;
   }

   return d < other.d;
}

/*!
    \fn bool QPersistentModelIndex::operator!=(const QPersistentModelIndex &other) const
    \since 4.2

    Returns true if this persistent model index is not equal to the \a
    other persistent model index; otherwise returns false.
*/

/*!
    Sets the persistent model index to refer to the same item in a model
    as the \a other persistent model index.
*/

QPersistentModelIndex &QPersistentModelIndex::operator=(const QPersistentModelIndex &other)
{
   if (d == other.d) {
      return *this;
   }
   if (d && !d->ref.deref()) {
      QPersistentModelIndexData::destroy(d);
   }
   d = other.d;
   if (d) {
      d->ref.ref();
   }
   return *this;
}

/*!
    Sets the persistent model index to refer to the same item in a model
    as the \a other model index.
*/

QPersistentModelIndex &QPersistentModelIndex::operator=(const QModelIndex &other)
{
   if (d && !d->ref.deref()) {
      QPersistentModelIndexData::destroy(d);
   }
   if (other.isValid()) {
      d = QPersistentModelIndexData::create(other);
      if (d) {
         d->ref.ref();
      }
   } else {
      d = 0;
   }
   return *this;
}

/*!
  \fn QPersistentModelIndex::operator const QModelIndex&() const

  Cast operator that returns a const QModelIndex&.
*/

QPersistentModelIndex::operator const QModelIndex &() const
{
   static const QModelIndex invalid;
   if (d) {
      return d->index;
   }
   return invalid;
}

/*!
    \fn bool QPersistentModelIndex::operator==(const QModelIndex &other) const

    Returns true if this persistent model index refers to the same location as
    the \a other model index; otherwise returns false.

    All values in the persistent model index are used when comparing with
    another model index.
*/

bool QPersistentModelIndex::operator==(const QModelIndex &other) const
{
   if (d) {
      return d->index == other;
   }
   return !other.isValid();
}

/*!
    \fn bool QPersistentModelIndex::operator!=(const QModelIndex &other) const

    Returns true if this persistent model index does not refer to the same
    location as the \a other model index; otherwise returns false.
*/

bool QPersistentModelIndex::operator!=(const QModelIndex &other) const
{
   if (d) {
      return d->index != other;
   }
   return other.isValid();
}

/*!
    \fn int QPersistentModelIndex::row() const

    Returns the row this persistent model index refers to.
*/

int QPersistentModelIndex::row() const
{
   if (d) {
      return d->index.row();
   }
   return -1;
}

/*!
    \fn int QPersistentModelIndex::column() const

    Returns the column this persistent model index refers to.
*/

int QPersistentModelIndex::column() const
{
   if (d) {
      return d->index.column();
   }
   return -1;
}

/*!
    \fn void *QPersistentModelIndex::internalPointer() const

    \internal

    Returns a \c{void} \c{*} pointer used by the model to associate the index with
    the internal data structure.
*/

void *QPersistentModelIndex::internalPointer() const
{
   if (d) {
      return d->index.internalPointer();
   }
   return 0;
}

/*!
    \fn void *QPersistentModelIndex::internalId() const

    \internal

    Returns a \c{qint64} used by the model to associate the index with
    the internal data structure.
*/

qint64 QPersistentModelIndex::internalId() const
{
   if (d) {
      return d->index.internalId();
   }
   return 0;
}

/*!
    Returns the parent QModelIndex for this persistent index, or an invalid
    QModelIndex if it has no parent.

    \sa child() sibling() model()
*/
QModelIndex QPersistentModelIndex::parent() const
{
   if (d) {
      return d->index.parent();
   }
   return QModelIndex();
}

/*!
    Returns the sibling at \a row and \a column or an invalid QModelIndex if
    there is no sibling at this position.

    \sa parent() child()
*/

QModelIndex QPersistentModelIndex::sibling(int row, int column) const
{
   if (d) {
      return d->index.sibling(row, column);
   }
   return QModelIndex();
}

/*!
    Returns the child of the model index that is stored in the given \a row
    and \a column.

    \sa parent() sibling()
*/

QModelIndex QPersistentModelIndex::child(int row, int column) const
{
   if (d) {
      return d->index.child(row, column);
   }
   return QModelIndex();
}

/*!
    Returns the data for the given \a role for the item referred to by the
    index.

    \sa Qt::ItemDataRole, QAbstractItemModel::setData()
*/
QVariant QPersistentModelIndex::data(int role) const
{
   if (d) {
      return d->index.data(role);
   }
   return QVariant();
}

/*!
    \since 4.2

    Returns the flags for the item referred to by the index.
*/
Qt::ItemFlags QPersistentModelIndex::flags() const
{
   if (d) {
      return d->index.flags();
   }
   return 0;
}

/*!
    Returns the model that the index belongs to.
*/
const QAbstractItemModel *QPersistentModelIndex::model() const
{
   if (d) {
      return d->index.model();
   }
   return 0;
}

/*!
    \fn bool QPersistentModelIndex::isValid() const

    Returns true if this persistent model index is valid; otherwise returns
    false.

    A valid index belongs to a model, and has non-negative row and column
    numbers.

    \sa model(), row(), column()
*/

bool QPersistentModelIndex::isValid() const
{
   return d && d->index.isValid();
}

QDebug operator<<(QDebug dbg, const QModelIndex &idx)
{
   dbg.nospace() << "QModelIndex(" << idx.row() << ',' << idx.column()
                 << ',' << idx.internalPointer() << ',' << idx.model() << ')';
   return dbg.space();
}

QDebug operator<<(QDebug dbg, const QPersistentModelIndex &idx)
{
   if (idx.d) {
      dbg << idx.d->index;
   } else {
      dbg << QModelIndex();
   }
   return dbg;
}

class QEmptyItemModel : public QAbstractItemModel
{
 public:
   explicit QEmptyItemModel(QObject *parent = nullptr) : QAbstractItemModel(parent) {}
   QModelIndex index(int, int, const QModelIndex &) const override {
      return QModelIndex();
   }
   QModelIndex parent(const QModelIndex &) const override {
      return QModelIndex();
   }
   int rowCount(const QModelIndex &) const  override {
      return 0;
   }
   int columnCount(const QModelIndex &) const  override{
      return 0;
   }
   bool hasChildren(const QModelIndex &) const override {
      return false;
   }
   QVariant data(const QModelIndex &, int) const override{
      return QVariant();
   }
};

Q_GLOBAL_STATIC(QEmptyItemModel, qEmptyModel)

QAbstractItemModel *QAbstractItemModelPrivate::staticEmptyModel()
{
   return qEmptyModel();
}

namespace {
struct DefaultRoleNames : public QHash<int, QByteArray> {
   DefaultRoleNames() {
      (*this)[Qt::DisplayRole] = "display";
      (*this)[Qt::DecorationRole] = "decoration";
      (*this)[Qt::EditRole] = "edit";
      (*this)[Qt::ToolTipRole] = "toolTip";
      (*this)[Qt::StatusTipRole] = "statusTip";
      (*this)[Qt::WhatsThisRole] = "whatsThis";
   }
};
}

Q_GLOBAL_STATIC(DefaultRoleNames, qDefaultRoleNames)

const QHash<int, QByteArray> &QAbstractItemModelPrivate::defaultRoleNames()
{
   return *qDefaultRoleNames();
}


static uint typeOfVariant(const QVariant &value)
{
   //return 0 for integer, 1 for floating point and 2 for other
   switch (value.userType()) {
      case QVariant::Bool:
      case QVariant::Int:
      case QVariant::UInt:
      case QVariant::LongLong:
      case QVariant::ULongLong:
      case QVariant::Char:
      case QMetaType::Short:
      case QMetaType::UShort:
      case QMetaType::UChar:
      case QMetaType::ULong:
      case QMetaType::Long:
         return 0;
      case QVariant::Double:
      case QMetaType::Float:
         return 1;
      default:
         return 2;
   }
}

/*!
    \internal
    return true if \a value contains a numerical type

    This function is used by our Q{Tree,Widget,Table}WidgetModel classes to sort.
*/
bool QAbstractItemModelPrivate::variantLessThan(const QVariant &v1, const QVariant &v2)
{
   switch (qMax(typeOfVariant(v1), typeOfVariant(v2))) {
      case 0: //integer type
         return v1.toLongLong() < v2.toLongLong();
      case 1: //floating point
         return v1.toReal() < v2.toReal();
      default:
         return v1.toString().localeAwareCompare(v2.toString()) < 0;
   }
}

void QAbstractItemModelPrivate::removePersistentIndexData(QPersistentModelIndexData *data)
{
   if (data->index.isValid()) {
      int removed = persistent.m_indexes.remove(data->index);

      Q_ASSERT_X(removed == 1, "QPersistentModelIndex::~QPersistentModelIndex",
                 "persistent model indexes corrupted");

      // This assert may happen if the model use changePersistentIndex in a way that could result on two
      // QPersistentModelIndex pointing to the same index.
      Q_UNUSED(removed);
   }
   // make sure our optimization still works
   for (int i = persistent.moved.count() - 1; i >= 0; --i) {
      int idx = persistent.moved[i].indexOf(data);
      if (idx >= 0) {
         persistent.moved[i].remove(idx);
      }
   }
   // update the references to invalidated persistent indexes
   for (int i = persistent.invalidated.count() - 1; i >= 0; --i) {
      int idx = persistent.invalidated[i].indexOf(data);
      if (idx >= 0) {
         persistent.invalidated[i].remove(idx);
      }
   }

}

void QAbstractItemModelPrivate::rowsAboutToBeInserted(const QModelIndex &parent,
      int first, int last)
{
   Q_Q(QAbstractItemModel);
   Q_UNUSED(last);

   QVector<QPersistentModelIndexData *> persistent_moved;

   if (first < q->rowCount(parent)) {
      for (auto it = persistent.m_indexes.constBegin(); it != persistent.m_indexes.constEnd(); ++it) {
         QPersistentModelIndexData *data = *it;
         const QModelIndex &index = data->index;

         if (index.row() >= first && index.isValid() && index.parent() == parent) {
            persistent_moved.append(data);
         }
      }
   }

   persistent.moved.push(persistent_moved);
}

void QAbstractItemModelPrivate::rowsInserted(const QModelIndex &parent,
      int first, int last)
{
   QVector<QPersistentModelIndexData *> persistent_moved = persistent.moved.pop();
   int count = (last - first) + 1; // it is important to only use the delta, because the change could be nested

   for (auto it = persistent_moved.constBegin(); it != persistent_moved.constEnd(); ++it) {
      QPersistentModelIndexData *data = *it;
      QModelIndex old = data->index;

      auto iter = persistent.m_indexes.find(old);

      if (iter != persistent.m_indexes.end()) {
         persistent.m_indexes.erase(iter);
      }

      data->index = q_func()->index(old.row() + count, old.column(), parent);

      if (data->index.isValid()) {
         persistent.insertMultiAtEnd(data->index, data);
      } else {
         qWarning() << "QAbstractItemModel::endInsertRows:  Invalid index (" << old.row() + count << ','
                  << old.column() << ") in model" << q_func();
      }
   }
}

void QAbstractItemModelPrivate::itemsAboutToBeMoved(const QModelIndex &srcParent, int srcFirst, int srcLast,
      const QModelIndex &destinationParent, int destinationChild, Qt::Orientation orientation)
{
   QVector<QPersistentModelIndexData *> persistent_moved_explicitly;
   QVector<QPersistentModelIndexData *> persistent_moved_in_source;
   QVector<QPersistentModelIndexData *> persistent_moved_in_destination;

   QMultiMap<QModelIndex, QPersistentModelIndexData *>::const_iterator it;

   const QMultiMap<QModelIndex, QPersistentModelIndexData *>::const_iterator begin = persistent.m_indexes.constBegin();
   const QMultiMap<QModelIndex, QPersistentModelIndexData *>::const_iterator end   = persistent.m_indexes.constEnd();

   const bool sameParent = (srcParent == destinationParent);
   const bool movingUp = (srcFirst > destinationChild);

   for ( it = begin; it != end; ++it) {
      QPersistentModelIndexData *data = *it;
      const QModelIndex &index = data->index;
      const QModelIndex &parent = index.parent();
      const bool isSourceIndex = (parent == srcParent);
      const bool isDestinationIndex = (parent == destinationParent);

      int childPosition;
      if (orientation == Qt::Vertical) {
         childPosition = index.row();
      } else {
         childPosition = index.column();
      }

      if (!index.isValid() || !(isSourceIndex || isDestinationIndex ) ) {
         continue;
      }

      if (!sameParent && isDestinationIndex) {
         if (childPosition >= destinationChild) {
            persistent_moved_in_destination.append(data);
         }
         continue;
      }

      if (sameParent && movingUp && childPosition < destinationChild) {
         continue;
      }

      if (sameParent && !movingUp && childPosition < srcFirst ) {
         continue;
      }

      if (!sameParent && childPosition < srcFirst) {
         continue;
      }

      if (sameParent && (childPosition > srcLast) && (childPosition >= destinationChild )) {
         continue;
      }

      if ((childPosition <= srcLast) && (childPosition >= srcFirst)) {
         persistent_moved_explicitly.append(data);
      } else {
         persistent_moved_in_source.append(data);
      }
   }
   persistent.moved.push(persistent_moved_explicitly);
   persistent.moved.push(persistent_moved_in_source);
   persistent.moved.push(persistent_moved_in_destination);
}

/*!
  \internal

  Moves persistent indexes \a indexes by amount \a change. The change will be either a change in row value or a change in
  column value depending on the value of \a orientation. The indexes may also be moved to a different parent if \a parent
  differs from the existing parent for the index.
*/
void QAbstractItemModelPrivate::movePersistentIndexes(QVector<QPersistentModelIndexData *> indexes, int change,
      const QModelIndex &parent, Qt::Orientation orientation)
{
   QVector<QPersistentModelIndexData *>::const_iterator it;
   const QVector<QPersistentModelIndexData *>::const_iterator begin = indexes.constBegin();
   const QVector<QPersistentModelIndexData *>::const_iterator end   = indexes.constEnd();

   for (it = begin; it != end; ++it) {
      QPersistentModelIndexData *data = *it;

      int row = data->index.row();
      int column = data->index.column();

      if (Qt::Vertical == orientation) {
         row += change;
      } else {
         column += change;
      }

      auto iter = persistent.m_indexes.find(data->index);

      if (iter != persistent.m_indexes.end()) {
         persistent.m_indexes.erase(iter);
      }

      data->index = q_func()->index(row, column, parent);

      if (data->index.isValid()) {
         persistent.insertMultiAtEnd(data->index, data);
      } else {
         qWarning() << "QAbstractItemModel::endMoveRows:  Invalid index (" << row << "," << column << ") in model" << q_func();
      }
   }
}

void QAbstractItemModelPrivate::itemsMoved(const QModelIndex &sourceParent, int sourceFirst, int sourceLast,
      const QModelIndex &destinationParent, int destinationChild, Qt::Orientation orientation)
{
   QVector<QPersistentModelIndexData *> moved_in_destination = persistent.moved.pop();
   QVector<QPersistentModelIndexData *> moved_in_source = persistent.moved.pop();
   QVector<QPersistentModelIndexData *> moved_explicitly = persistent.moved.pop();

   const bool sameParent = (sourceParent == destinationParent);
   const bool movingUp = (sourceFirst > destinationChild);

   const int explicit_change = (! sameParent || movingUp) ? destinationChild - sourceFirst : destinationChild - sourceLast - 1;
   const int source_change   = (! sameParent || ! movingUp) ? -1 * (sourceLast - sourceFirst + 1) : sourceLast - sourceFirst + 1;
   const int destination_change = sourceLast - sourceFirst + 1;

   movePersistentIndexes(moved_explicitly, explicit_change, destinationParent, orientation);
   movePersistentIndexes(moved_in_source, source_change, sourceParent, orientation);
   movePersistentIndexes(moved_in_destination, destination_change, destinationParent, orientation);
}

void QAbstractItemModelPrivate::rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
   QVector<QPersistentModelIndexData *>  persistent_moved;
   QVector<QPersistentModelIndexData *>  persistent_invalidated;

   // find the persistent indexes that are affected by the change, either by being in the removed subtree
   // or by being on the same level and below the removed rows

   for (auto it = persistent.m_indexes.constBegin(); it != persistent.m_indexes.constEnd(); ++it) {
      QPersistentModelIndexData *data = *it;
      bool level_changed = false;
      QModelIndex current = data->index;

      while (current.isValid()) {
         QModelIndex current_parent = current.parent();

         if (current_parent == parent) { // on the same level as the change
            if (!level_changed && current.row() > last) { // below the removed rows
               persistent_moved.append(data);
            } else if (current.row() <= last && current.row() >= first) { // in the removed subtree
               persistent_invalidated.append(data);
            }
            break;
         }

         current = current_parent;
         level_changed = true;
      }
   }

   persistent.moved.push(persistent_moved);
   persistent.invalidated.push(persistent_invalidated);
}

void QAbstractItemModelPrivate::rowsRemoved(const QModelIndex &parent, int first, int last)
{
   QVector<QPersistentModelIndexData *> persistent_moved = persistent.moved.pop();
   int count = (last - first) + 1; // it is important to only use the delta, because the change could be nested

   for (auto it = persistent_moved.constBegin(); it != persistent_moved.constEnd(); ++it) {

      QPersistentModelIndexData *data = *it;
      QModelIndex old = data->index;

      auto iter = persistent.m_indexes.find(old);

      if (iter != persistent.m_indexes.end()) {
         persistent.m_indexes.erase(iter);
      }

      data->index = q_func()->index(old.row() - count, old.column(), parent);

      if (data->index.isValid()) {
         persistent.insertMultiAtEnd(data->index, data);
      } else {
         qWarning() << "QAbstractItemModel::endRemoveRows: Invalid index ("
                  << old.row() - count << ',' << old.column() << ") in model" << q_func();
      }
   }

   QVector<QPersistentModelIndexData *> persistent_invalidated = persistent.invalidated.pop();

   for (auto it = persistent_invalidated.constBegin(); it != persistent_invalidated.constEnd(); ++it) {
      QPersistentModelIndexData *data = *it;

      auto iter = persistent.m_indexes.find(data->index);

      if (iter != persistent.m_indexes.end()) {
         persistent.m_indexes.erase(iter);
      }

      data->index = QModelIndex();
      data->model = 0;
   }
}

void QAbstractItemModelPrivate::columnsAboutToBeInserted(const QModelIndex &parent,
      int first, int last)
{
   Q_Q(QAbstractItemModel);
   Q_UNUSED(last);

   QVector<QPersistentModelIndexData *> persistent_moved;

   if (first < q->columnCount(parent)) {
      for (auto it = persistent.m_indexes.constBegin(); it != persistent.m_indexes.constEnd(); ++it) {
         QPersistentModelIndexData *data = *it;
         const QModelIndex &index = data->index;

         if (index.column() >= first && index.isValid() && index.parent() == parent) {
            persistent_moved.append(data);
         }
      }
   }

   persistent.moved.push(persistent_moved);
}

void QAbstractItemModelPrivate::columnsInserted(const QModelIndex &parent, int first, int last)
{
   QVector<QPersistentModelIndexData *> persistent_moved = persistent.moved.pop();
   int count = (last - first) + 1; // it is important to only use the delta, because the change could be nested

   for (auto it = persistent_moved.constBegin(); it != persistent_moved.constEnd(); ++it) {

      QPersistentModelIndexData *data = *it;
      QModelIndex old = data->index;

      auto iter = persistent.m_indexes.find(old);

      if (iter != persistent.m_indexes.end()) {
         persistent.m_indexes.erase(iter);
      }

      data->index = q_func()->index(old.row(), old.column() + count, parent);

      if (data->index.isValid()) {
         persistent.insertMultiAtEnd(data->index, data);
      } else {
         qWarning() << "QAbstractItemModel::endInsertColumns:  Invalid index (" << old.row() << ',' << old.column() + count <<
                    ") in model" << q_func();
      }
   }
}

void QAbstractItemModelPrivate::columnsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
   QVector<QPersistentModelIndexData *> persistent_moved;
   QVector<QPersistentModelIndexData *> persistent_invalidated;

   // find the persistent indexes that are affected by the change, either by being in the removed subtree
   // or by being on the same level and to the right of the removed columns

   for (auto it = persistent.m_indexes.constBegin(); it != persistent.m_indexes.constEnd(); ++it) {
      QPersistentModelIndexData *data = *it;
      bool level_changed  = false;
      QModelIndex current = data->index;

      while (current.isValid()) {
         QModelIndex current_parent = current.parent();

         if (current_parent == parent) { // on the same level as the change
            if (!level_changed && current.column() > last) { // right of the removed columns
               persistent_moved.append(data);
            } else if (current.column() <= last && current.column() >= first) { // in the removed subtree
               persistent_invalidated.append(data);
            }
            break;
         }
         current = current_parent;
         level_changed = true;
      }
   }

   persistent.moved.push(persistent_moved);
   persistent.invalidated.push(persistent_invalidated);
}

void QAbstractItemModelPrivate::columnsRemoved(const QModelIndex &parent, int first, int last)
{
   QVector<QPersistentModelIndexData *> persistent_moved = persistent.moved.pop();
   int count = (last - first) + 1; // it is important to only use the delta, because the change could be nested

   for (auto it = persistent_moved.constBegin(); it != persistent_moved.constEnd(); ++it) {

      QPersistentModelIndexData *data = *it;
      QModelIndex old = data->index;

      auto iter = persistent.m_indexes.find(old);

      if (iter != persistent.m_indexes.end()) {
         persistent.m_indexes.erase(iter);
      }

      data->index = q_func()->index(old.row(), old.column() - count, parent);

      if (data->index.isValid()) {
         persistent.insertMultiAtEnd(data->index, data);
      } else {
         qWarning() << "QAbstractItemModel::endRemoveColumns:  Invalid index (" << old.row() << ',' << old.column() - count <<
                    ") in model" << q_func();
      }
   }

   QVector<QPersistentModelIndexData *> persistent_invalidated = persistent.invalidated.pop();

   for (auto it = persistent_invalidated.constBegin();it != persistent_invalidated.constEnd(); ++it) {
      QPersistentModelIndexData *data = *it;

      auto iter = persistent.m_indexes.find(data->index);

      if (iter != persistent.m_indexes.end()) {
         persistent.m_indexes.erase(iter);
      }

      data->index = QModelIndex();
      data->model = 0;
   }
}

void QAbstractItemModel::resetInternalData()
{
}


/*!
    Constructs an abstract item model with the given \a parent.
*/
QAbstractItemModel::QAbstractItemModel(QObject *parent)
   : QObject(parent), d_ptr(new QAbstractItemModelPrivate)
{
   d_ptr->q_ptr = this;
}

/*!
  \internal
*/
QAbstractItemModel::QAbstractItemModel(QAbstractItemModelPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QAbstractItemModel::~QAbstractItemModel()
{
   d_func()->invalidatePersistentIndexes();
}

bool QAbstractItemModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
   if (row < 0 || column < 0) {
      return false;
   }
   return row < rowCount(parent) && column < columnCount(parent);
}

bool QAbstractItemModel::hasChildren(const QModelIndex &parent) const
{
   return (rowCount(parent) > 0) && (columnCount(parent) > 0);
}

QMap<int, QVariant> QAbstractItemModel::itemData(const QModelIndex &index) const
{
   QMap<int, QVariant> roles;
   for (int i = 0; i < Qt::UserRole; ++i) {
      QVariant variantData = data(index, i);
      if (variantData.isValid()) {
         roles.insert(i, variantData);
      }
   }
   return roles;
}

/*!
    Sets the \a role data for the item at \a index to \a value.

    Returns true if successful; otherwise returns false.

    The dataChanged() signal should be emitted if the data was successfully
    set.

    The base class implementation returns false. This function and data() must
    be reimplemented for editable models.

    \sa Qt::ItemDataRole, data(), itemData()
*/
bool QAbstractItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   Q_UNUSED(index);
   Q_UNUSED(value);
   Q_UNUSED(role);

   return false;
}

bool QAbstractItemModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
   bool b = true;
   for (QMap<int, QVariant>::ConstIterator it = roles.begin(); it != roles.end(); ++it) {
      b = b && setData(index, it.value(), it.key());
   }
   return b;
}

QStringList QAbstractItemModel::mimeTypes() const
{
   QStringList types;
   types << QLatin1String("application/x-qabstractitemmodeldatalist");
   return types;
}

QMimeData *QAbstractItemModel::mimeData(const QModelIndexList &indexes) const
{
   if (indexes.count() <= 0) {
      return 0;
   }

   QStringList types = mimeTypes();
   if (types.isEmpty()) {
      return 0;
   }

   QMimeData *data = new QMimeData();
   QString format = types.at(0);
   QByteArray encoded;
   QDataStream stream(&encoded, QIODevice::WriteOnly);

   encodeData(indexes, stream);
   data->setData(format, encoded);
   return data;
}

bool QAbstractItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                      int row, int column, const QModelIndex &parent)
{
   // check if the action is supported
   if (!data || !(action == Qt::CopyAction || action == Qt::MoveAction)) {
      return false;
   }
   // check if the format is supported
   QStringList types = mimeTypes();
   if (types.isEmpty()) {
      return false;
   }
   QString format = types.at(0);
   if (!data->hasFormat(format)) {
      return false;
   }
   if (row > rowCount(parent)) {
      row = rowCount(parent);
   }
   if (row == -1) {
      row = rowCount(parent);
   }
   if (column == -1) {
      column = 0;
   }
   // decode and insert
   QByteArray encoded = data->data(format);
   QDataStream stream(&encoded, QIODevice::ReadOnly);
   return decodeData(row, column, parent, stream);
}

/*!
    \since 4.2

    Returns the drop actions supported by this model.

    The default implementation returns Qt::CopyAction. Reimplement this
    function if you wish to support additional actions. You must also
    reimplement the dropMimeData() function to handle the additional
    operations.

    \sa dropMimeData(), Qt::DropActions, {Using drag and drop with item
    views}
*/
Qt::DropActions QAbstractItemModel::supportedDropActions() const
{
   return Qt::CopyAction;
}

Qt::DropActions QAbstractItemModel::supportedDragActions() const
{
   // ### Qt5 make this virtual or these properties
   Q_D(const QAbstractItemModel);

   if (d->supportedDragActions != -1) {
      return d->supportedDragActions;
   }

   return supportedDropActions();
}

void QAbstractItemModel::setSupportedDragActions(Qt::DropActions actions)
{
   Q_D(QAbstractItemModel);
   d->supportedDragActions = actions;
}

/*!
    \note The base class implementation of this function does nothing and
    returns false.

    On models that support this, inserts \a count rows into the model before
    the given \a row. Items in the new row will be children of the item
    represented by the \a parent model index.

    If \a row is 0, the rows are prepended to any existing rows in the parent.

    If \a row is rowCount(), the rows are appended to any existing rows in the
    parent.

    If \a parent has no children, a single column with \a count rows is
    inserted.

    Returns true if the rows were successfully inserted; otherwise returns
    false.

    If you implement your own model, you can reimplement this function if you
    want to support insertions. Alternatively, you can provide your own API for
    altering the data. In either case, you will need to call
    beginInsertRows() and endInsertRows() to notify other components that the
    model has changed.

    \sa insertColumns(), removeRows(), beginInsertRows(), endInsertRows()
*/
bool QAbstractItemModel::insertRows(int, int, const QModelIndex &)
{
   return false;
}

/*!
    On models that support this, inserts \a count new columns into the model
    before the given \a column. The items in each new column will be children
    of the item represented by the \a parent model index.

    If \a column is 0, the columns are prepended to any existing columns.

    If \a column is columnCount(), the columns are appended to any existing
    columns.

    If \a parent has no children, a single row with \a count columns is
    inserted.

    Returns true if the columns were successfully inserted; otherwise returns
    false.

    The base class implementation does nothing and returns false.

    If you implement your own model, you can reimplement this function if you
    want to support insertions. Alternatively, you can provide your own API for
    altering the data.

    \sa insertRows(), removeColumns(), beginInsertColumns(), endInsertColumns()
*/
bool QAbstractItemModel::insertColumns(int, int, const QModelIndex &)
{
   return false;
}

/*!
    On models that support this, removes \a count rows starting with the given
    \a row under parent \a parent from the model.

    Returns true if the rows were successfully removed; otherwise returns
    false.

    The base class implementation does nothing and returns false.

    If you implement your own model, you can reimplement this function if you
    want to support removing. Alternatively, you can provide your own API for
    altering the data.

    \sa removeRow(), removeColumns(), insertColumns(), beginRemoveRows(),
        endRemoveRows()
*/
bool QAbstractItemModel::removeRows(int, int, const QModelIndex &)
{
   return false;
}

/*!
    On models that support this, removes \a count columns starting with the
    given \a column under parent \a parent from the model.

    Returns true if the columns were successfully removed; otherwise returns
    false.

    The base class implementation does nothing and returns false.

    If you implement your own model, you can reimplement this function if you
    want to support removing. Alternatively, you can provide your own API for
    altering the data.

    \sa removeColumn(), removeRows(), insertColumns(), beginRemoveColumns(),
        endRemoveColumns()
*/
bool QAbstractItemModel::removeColumns(int, int, const QModelIndex &)
{
   return false;
}

/*!
    Fetches any available data for the items with the parent specified by the
    \a parent index.

    Reimplement this if you are populating your model incrementally.

    The default implementation does nothing.

    \sa canFetchMore()
*/
void QAbstractItemModel::fetchMore(const QModelIndex &)
{
   // do nothing
}

/*!
    Returns true if there is more data available for \a parent; otherwise
    returns false.

    The default implementation always returns false.

    If canFetchMore() returns true, QAbstractItemView will call fetchMore().
    However, the fetchMore() function is only called when the model is being
    populated incrementally.

    \sa fetchMore()
*/
bool QAbstractItemModel::canFetchMore(const QModelIndex &) const
{
   return false;
}

/*!
    Returns the item flags for the given \a index.

    The base class implementation returns a combination of flags that enables
    the item (\c ItemIsEnabled) and allows it to be selected
    (\c ItemIsSelectable).

    \sa Qt::ItemFlags
*/
Qt::ItemFlags QAbstractItemModel::flags(const QModelIndex &index) const
{
   Q_D(const QAbstractItemModel);
   if (!d->indexValid(index)) {
      return 0;
   }

   return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void QAbstractItemModel::sort(int column, Qt::SortOrder order)
{
   Q_UNUSED(column);
   Q_UNUSED(order);
   // do nothing
}

QModelIndex QAbstractItemModel::buddy(const QModelIndex &index) const
{
   return index;
}

QModelIndexList QAbstractItemModel::match(const QModelIndex &start, int role,
                  const QVariant &value, int hits, Qt::MatchFlags flags) const
{
   QModelIndexList result;
   uint matchType = flags & 0x0F;
   Qt::CaseSensitivity cs = flags & Qt::MatchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
   bool recurse = flags & Qt::MatchRecursive;
   bool wrap = flags & Qt::MatchWrap;
   bool allHits = (hits == -1);

   QString text; // only convert to a string if it is needed
   QModelIndex p = parent(start);

   int from = start.row();
   int to = rowCount(p);

   // iterates twice if wrapping
   for (int i = 0; (wrap && i < 2) || (!wrap && i < 1); ++i) {
      for (int r = from; (r < to) && (allHits || result.count() < hits); ++r) {
         QModelIndex idx = index(r, start.column(), p);
         if (!idx.isValid()) {
            continue;
         }
         QVariant v = data(idx, role);
         // QVariant based matching
         if (matchType == Qt::MatchExactly) {
            if (value == v) {
               result.append(idx);
            }
         } else { // QString based matching
            if (text.isEmpty()) { // lazy conversion
               text = value.toString();
            }
            QString t = v.toString();
            switch (matchType) {
               case Qt::MatchRegExp:
                  if (QRegExp(text, cs).exactMatch(t)) {
                     result.append(idx);
                  }
                  break;
               case Qt::MatchWildcard:
                  if (QRegExp(text, cs, QRegExp::Wildcard).exactMatch(t)) {
                     result.append(idx);
                  }
                  break;
               case Qt::MatchStartsWith:
                  if (t.startsWith(text, cs)) {
                     result.append(idx);
                  }
                  break;
               case Qt::MatchEndsWith:
                  if (t.endsWith(text, cs)) {
                     result.append(idx);
                  }
                  break;
               case Qt::MatchFixedString:
                  if (t.compare(text, cs) == 0) {
                     result.append(idx);
                  }
                  break;
               case Qt::MatchContains:
               default:
                  if (t.contains(text, cs)) {
                     result.append(idx);
                  }
            }
         }
         if (recurse && hasChildren(idx)) { // search the hierarchy
            result += match(index(0, idx.column(), idx), role,
                            (text.isEmpty() ? value : text),
                            (allHits ? -1 : hits - result.count()), flags);
         }
      }
      // prepare for the next iteration
      from = 0;
      to = start.row();
   }
   return result;
}

/*!
    Returns the row and column span of the item represented by \a index.

    \note Currently, span is not used.
*/

QSize QAbstractItemModel::span(const QModelIndex &) const
{
   return QSize(1, 1);
}

/*!
    \since 4.6

    Sets the model's role names to \a roleNames.

    This function allows mapping of role identifiers to role property names in
    Declarative UI.  This function must be called before the model is used.
    Modifying the role names after the model has been set may result in
    undefined behaviour.

    \sa roleNames()
*/
void QAbstractItemModel::setRoleNames(const QHash<int, QByteArray> &roleNames)
{
   Q_D(QAbstractItemModel);
   d->roleNames = roleNames;
}

/*!
    \since 4.6

    Returns the model's role names.

    \sa setRoleNames()
*/
const QHash<int, QByteArray> &QAbstractItemModel::roleNames() const
{
   Q_D(const QAbstractItemModel);
   return d->roleNames;
}

/*!
    Lets the model know that it should submit cached information to permanent
    storage. This function is typically used for row editing.

    Returns true if there is no error; otherwise returns false.

    \sa revert()
*/

bool QAbstractItemModel::submit()
{
   return true;
}

/*!
    Lets the model know that it should discard cached information. This
    function is typically used for row editing.

    \sa submit()
*/

void QAbstractItemModel::revert()
{
}

QVariant QAbstractItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_UNUSED(orientation);
   if (role == Qt::DisplayRole) {
      return section + 1;
   }
   return QVariant();
}

bool QAbstractItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
   Q_UNUSED(section);
   Q_UNUSED(orientation);
   Q_UNUSED(value);
   Q_UNUSED(role);
   return false;
}

/*!
  \internal
*/
void QAbstractItemModel::encodeData(const QModelIndexList &indexes, QDataStream &stream) const
{
   QModelIndexList::ConstIterator it = indexes.begin();

   for (; it != indexes.end(); ++it) {
      stream << (*it).row() << (*it).column() << itemData(*it);
   }
}

/*!
  \internal
 */
bool QAbstractItemModel::decodeData(int row, int column, const QModelIndex &parent, QDataStream &stream)
{
   int top = INT_MAX;
   int left = INT_MAX;
   int bottom = 0;
   int right = 0;
   QVector<int> rows, columns;
   QVector<QMap<int, QVariant> > data;

   while (!stream.atEnd()) {
      int r, c;
      QMap<int, QVariant> v;
      stream >> r >> c >> v;
      rows.append(r);
      columns.append(c);
      data.append(v);
      top = qMin(r, top);
      left = qMin(c, left);
      bottom = qMax(r, bottom);
      right = qMax(c, right);
   }

   // insert the dragged items into the table, use a bit array to avoid overwriting items,
   // since items from different tables can have the same row and column
   int dragRowCount = 0;
   int dragColumnCount = right - left + 1;

   // Compute the number of continuous rows upon insertion and modify the rows to match
   QVector<int> rowsToInsert(bottom + 1);
   for (int i = 0; i < rows.count(); ++i) {
      rowsToInsert[rows.at(i)] = 1;
   }
   for (int i = 0; i < rowsToInsert.count(); ++i) {
      if (rowsToInsert[i] == 1) {
         rowsToInsert[i] = dragRowCount;
         ++dragRowCount;
      }
   }
   for (int i = 0; i < rows.count(); ++i) {
      rows[i] = top + rowsToInsert[rows[i]];
   }

   QBitArray isWrittenTo(dragRowCount * dragColumnCount);

   // make space in the table for the dropped data
   int colCount = columnCount(parent);
   if (colCount == 0) {
      insertColumns(colCount, dragColumnCount - colCount, parent);
      colCount = columnCount(parent);
   }
   insertRows(row, dragRowCount, parent);

   row = qMax(0, row);
   column = qMax(0, column);

   QVector<QPersistentModelIndex> newIndexes(data.size());
   // set the data in the table
   for (int j = 0; j < data.size(); ++j) {
      int relativeRow = rows.at(j) - top;
      int relativeColumn = columns.at(j) - left;
      int destinationRow = relativeRow + row;
      int destinationColumn = relativeColumn + column;
      int flat = (relativeRow * dragColumnCount) + relativeColumn;
      // if the item was already written to, or we just can't fit it in the table, create a new row
      if (destinationColumn >= colCount || isWrittenTo.testBit(flat)) {
         destinationColumn = qBound(column, destinationColumn, colCount - 1);
         destinationRow = row + dragRowCount;
         insertRows(row + dragRowCount, 1, parent);
         flat = (dragRowCount * dragColumnCount) + relativeColumn;
         isWrittenTo.resize(++dragRowCount * dragColumnCount);
      }

      if (!isWrittenTo.testBit(flat)) {
         newIndexes[j] = index(destinationRow, destinationColumn, parent);
         isWrittenTo.setBit(flat);
      }
   }

   for (int k = 0; k < newIndexes.size(); k++) {
      if (newIndexes.at(k).isValid()) {
         setItemData(newIndexes.at(k), data.at(k));
      }
   }

   return true;
}

void QAbstractItemModel::beginInsertRows(const QModelIndex &parent, int first, int last)
{
   Q_ASSERT(first >= 0);
   Q_ASSERT(last >= first);
   Q_D(QAbstractItemModel);
   d->changes.push(QAbstractItemModelPrivate::Change(parent, first, last));
   emit rowsAboutToBeInserted(parent, first, last);
   d->rowsAboutToBeInserted(parent, first, last);
}

void QAbstractItemModel::endInsertRows()
{
   Q_D(QAbstractItemModel);
   QAbstractItemModelPrivate::Change change = d->changes.pop();
   d->rowsInserted(change.parent, change.first, change.last);
   emit rowsInserted(change.parent, change.first, change.last);
}

void QAbstractItemModel::beginRemoveRows(const QModelIndex &parent, int first, int last)
{
   Q_ASSERT(first >= 0);
   Q_ASSERT(last >= first);
   Q_D(QAbstractItemModel);

   d->changes.push(QAbstractItemModelPrivate::Change(parent, first, last));
   emit rowsAboutToBeRemoved(parent, first, last);
   d->rowsAboutToBeRemoved(parent, first, last);
}

/*!
    Ends a row removal operation.

    When reimplementing removeRows() in a subclass, you must call this function
    \e after removing data from the model's underlying data store.

    \sa beginRemoveRows()
*/
void QAbstractItemModel::endRemoveRows()
{
   Q_D(QAbstractItemModel);
   QAbstractItemModelPrivate::Change change = d->changes.pop();
   d->rowsRemoved(change.parent, change.first, change.last);
   emit rowsRemoved(change.parent, change.first, change.last);
}

/*!
    Returns whether a move operation is valid.

    A move operation is not allowed if it moves a continuous range of rows to a destination within
    itself, or if it attempts to move a row to one of its own descendants.

    \internal
*/
bool QAbstractItemModelPrivate::allowMove(const QModelIndex &srcParent, int start, int end,
      const QModelIndex &destinationParent, int destinationStart, Qt::Orientation orientation)
{
   // Don't move the range within itself.
   if (destinationParent == srcParent) {
      return !(destinationStart >= start && destinationStart <= end + 1);
   }

   QModelIndex destinationAncestor = destinationParent;
   int pos = (Qt::Vertical == orientation) ? destinationAncestor.row() : destinationAncestor.column();
   forever {
      if (destinationAncestor == srcParent)
      {
         if (pos >= start && pos <= end) {
            return false;
         }
         break;
      }

      if (!destinationAncestor.isValid())
      {
         break;
      }

      pos = (Qt::Vertical == orientation) ? destinationAncestor.row() : destinationAncestor.column();
      destinationAncestor = destinationAncestor.parent();
   }

   return true;
}

/*!
    \since 4.6

    Begins a row move operation.

    When reimplementing a subclass, this method simplifies moving
    entities in your model. This method is responsible for moving
    persistent indexes in the model, which you would otherwise be
    required to do yourself. Using beginMoveRows and endMoveRows
    is an alternative to emitting layoutAboutToBeChanged and
    layoutChanged directly along with changePersistentIndexes.
    layoutAboutToBeChanged is emitted by this method for compatibility
    reasons.

    The \a sourceParent index corresponds to the parent from which the
    rows are moved; \a sourceFirst and \a sourceLast are the first and last
    row numbers of the rows to be moved. The \a destinationParent index
    corresponds to the parent into which those rows are moved. The \a
    destinationChild is the row to which the rows will be moved.  That
    is, the index at row \a sourceFirst in \a sourceParent will become
    row \a destinationChild in \a destinationParent, followed by all other
    rows up to \a sourceLast.

    However, when moving rows down in the same parent (\a sourceParent
    and \a destinationParent are equal), the rows will be placed before the
    \a destinationChild index. That is, if you wish to move rows 0 and 1 so
    they will become rows 1 and 2, \a destinationChild should be 3. In this
    case, the new index for the source row \c i (which is between
    \a sourceFirst and \a sourceLast) is equal to
    \c {(destinationChild-sourceLast-1+i)}.

    Note that if \a sourceParent and \a destinationParent are the same,
    you must ensure that the \a destinationChild is not within the range
    of \a sourceFirst and \a sourceLast + 1.  You must also ensure that you
    do not attempt to move a row to one of its own children or ancestors.
    This method returns false if either condition is true, in which case you
    should abort your move operation.

    \table 80%
    \row
        \o  \inlineimage modelview-move-rows-1.png Moving rows to another parent
        \o  Specify the first and last row numbers for the span of rows in
            the source parent you want to move in the model. Also specify
            the row in the destination parent to move the span to.

            For example, as shown in the diagram, we move three rows from
            row 2 to 4 in the source, so \a sourceFirst is 2 and \a sourceLast is 4.
            We move those items to above row 2 in the destination, so \a destinationChild is 2.

            \snippet doc/src/snippets/code/src_corelib_kernel_qabstractitemmodel.cpp 6

            This moves the three rows rows 2, 3, and 4 in the source to become 2, 3 and 4 in
            the destination. Other affected siblings are displaced accordingly.
    \row
        \o  \inlineimage modelview-move-rows-2.png Moving rows to append to another parent
        \o  To append rows to another parent, move them to after the last row.

            For example, as shown in the diagram, we move three rows to a
            collection of 6 existing rows (ending in row 5), so \a destinationChild is 6:

            \snippet doc/src/snippets/code/src_corelib_kernel_qabstractitemmodel.cpp 7

            This moves the target rows to the end of the target parent as 6, 7 and 8.
    \row
        \o  \inlineimage modelview-move-rows-3.png Moving rows in the same parent up
        \o  To move rows within the same parent, specify the row to move them to.

            For example, as shown in the diagram, we move one item from row 2 to row 0,
            so \a sourceFirst and \a sourceLast are 2 and \a destinationChild is 0.

            \snippet doc/src/snippets/code/src_corelib_kernel_qabstractitemmodel.cpp 8

            Note that other rows may be displaced accordingly. Note also that when moving
            items within the same parent you should not attempt invalid or no-op moves. In
            the above example, item 2 is at row 2 before the move, so it can not be moved
            to row 2 (where it is already) or row 3 (no-op as row 3 means above row 3, where
            it is already)

    \row
        \o  \inlineimage modelview-move-rows-4.png Moving rows in the same parent down
        \o  To move rows within the same parent, specify the row to move them to.

            For example, as shown in the diagram, we move one item from row 2 to row 4,
            so \a sourceFirst and \a sourceLast are 2 and \a destinationChild is 4.

            \snippet doc/src/snippets/code/src_corelib_kernel_qabstractitemmodel.cpp 9

            Note that other rows may be displaced accordingly.
    \endtable

    \sa endMoveRows()
*/
bool QAbstractItemModel::beginMoveRows(const QModelIndex &sourceParent, int sourceFirst, int sourceLast,
                                       const QModelIndex &destinationParent, int destinationChild)
{
   Q_ASSERT(sourceFirst >= 0);
   Q_ASSERT(sourceLast >= sourceFirst);
   Q_ASSERT(destinationChild >= 0);
   Q_D(QAbstractItemModel);

   if (!d->allowMove(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, Qt::Vertical)) {
      return false;
   }

   QAbstractItemModelPrivate::Change sourceChange(sourceParent, sourceFirst, sourceLast);
   sourceChange.needsAdjust = sourceParent.isValid() && sourceParent.row() >= destinationChild &&
                              sourceParent.parent() == destinationParent;
   d->changes.push(sourceChange);
   int destinationLast = destinationChild + (sourceLast - sourceFirst);
   QAbstractItemModelPrivate::Change destinationChange(destinationParent, destinationChild, destinationLast);
   destinationChange.needsAdjust = destinationParent.isValid() && destinationParent.row() >= sourceLast &&
                                   destinationParent.parent() == sourceParent;
   d->changes.push(destinationChange);

   emit rowsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild);
   emit layoutAboutToBeChanged();
   d->itemsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, Qt::Vertical);
   return true;
}

/*!
    Ends a row move operation.

    When implementing a subclass, you must call this
    function \e after moving data within the model's underlying data
    store.

    layoutChanged is emitted by this method for compatibility reasons.

    \sa beginMoveRows()

    \since 4.6
*/
void QAbstractItemModel::endMoveRows()
{
   Q_D(QAbstractItemModel);

   QAbstractItemModelPrivate::Change insertChange = d->changes.pop();
   QAbstractItemModelPrivate::Change removeChange = d->changes.pop();

   QModelIndex adjustedSource = removeChange.parent;
   QModelIndex adjustedDestination = insertChange.parent;

   const int numMoved = removeChange.last - removeChange.first + 1;
   if (insertChange.needsAdjust) {
      adjustedDestination = createIndex(adjustedDestination.row() - numMoved, adjustedDestination.column(),
                                        adjustedDestination.internalPointer());
   }

   if (removeChange.needsAdjust) {
      adjustedSource = createIndex(adjustedSource.row() + numMoved, adjustedSource.column(),
                                   adjustedSource.internalPointer());
   }

   d->itemsMoved(adjustedSource, removeChange.first, removeChange.last, adjustedDestination, insertChange.first,
                 Qt::Vertical);

   emit rowsMoved(adjustedSource, removeChange.first, removeChange.last, adjustedDestination, insertChange.first);
   emit layoutChanged();
}

/*!
    Begins a column insertion operation.

    When reimplementing insertColumns() in a subclass, you must call this
    function \e before inserting data into the model's underlying data store.

    The \a parent index corresponds to the parent into which the new columns
    are inserted; \a first and \a last are the column numbers of the new
    columns will have after they have been inserted.

    \table 80%
    \row
        \o  \inlineimage modelview-begin-insert-columns.png Inserting columns
        \o  Specify the first and last column numbers for the span of columns
            you want to insert into an item in a model.

            For example, as shown in the diagram, we insert three columns
            before column 4, so \a first is 4 and \a last is 6:

            \snippet doc/src/snippets/code/src_corelib_kernel_qabstractitemmodel.cpp 3

            This inserts the three new columns as columns 4, 5, and 6.
    \row
        \o  \inlineimage modelview-begin-append-columns.png Appending columns
        \o  To append columns, insert them after the last column.

            For example, as shown in the diagram, we append three columns to a
            collection of six existing columns (ending in column 5), so
            \a first is 6 and \a last is 8:

            \snippet doc/src/snippets/code/src_corelib_kernel_qabstractitemmodel.cpp 4

            This appends the two new columns as columns 6, 7, and 8.
    \endtable

    \note This function emits the columnsAboutToBeInserted() signal which
    connected views (or proxies) must handle before the data is inserted.
    Otherwise, the views may end up in an invalid state.

    \sa endInsertColumns()
*/
void QAbstractItemModel::beginInsertColumns(const QModelIndex &parent, int first, int last)
{
   Q_ASSERT(first >= 0);
   Q_ASSERT(last >= first);
   Q_D(QAbstractItemModel);
   d->changes.push(QAbstractItemModelPrivate::Change(parent, first, last));
   emit columnsAboutToBeInserted(parent, first, last);
   d->columnsAboutToBeInserted(parent, first, last);
}

/*!
    Ends a column insertion operation.

    When reimplementing insertColumns() in a subclass, you must call this
    function \e after inserting data into the model's underlying data
    store.

    \sa beginInsertColumns()
*/
void QAbstractItemModel::endInsertColumns()
{
   Q_D(QAbstractItemModel);
   QAbstractItemModelPrivate::Change change = d->changes.pop();
   d->columnsInserted(change.parent, change.first, change.last);
   emit columnsInserted(change.parent, change.first, change.last);
}

/*!
    Begins a column removal operation.

    When reimplementing removeColumns() in a subclass, you must call this
    function \e before removing data from the model's underlying data store.

    The \a parent index corresponds to the parent from which the new columns
    are removed; \a first and \a last are the column numbers of the first and
    last columns to be removed.

    \table 80%
    \row
        \o  \inlineimage modelview-begin-remove-columns.png Removing columns
        \o  Specify the first and last column numbers for the span of columns
            you want to remove from an item in a model.

            For example, as shown in the diagram, we remove the three columns
            from column 4 to column 6, so \a first is 4 and \a last is 6:

            \snippet doc/src/snippets/code/src_corelib_kernel_qabstractitemmodel.cpp 5
    \endtable

    \note This function emits the columnsAboutToBeRemoved() signal which
    connected views (or proxies) must handle before the data is removed.
    Otherwise, the views may end up in an invalid state.

    \sa endRemoveColumns()
*/
void QAbstractItemModel::beginRemoveColumns(const QModelIndex &parent, int first, int last)
{
   Q_ASSERT(first >= 0);
   Q_ASSERT(last >= first);
   Q_D(QAbstractItemModel);
   d->changes.push(QAbstractItemModelPrivate::Change(parent, first, last));
   emit columnsAboutToBeRemoved(parent, first, last);
   d->columnsAboutToBeRemoved(parent, first, last);
}

/*!
    Ends a column removal operation.

    When reimplementing removeColumns() in a subclass, you must call this
    function \e after removing data from the model's underlying data store.

    \sa beginRemoveColumns()
*/
void QAbstractItemModel::endRemoveColumns()
{
   Q_D(QAbstractItemModel);
   QAbstractItemModelPrivate::Change change = d->changes.pop();
   d->columnsRemoved(change.parent, change.first, change.last);
   emit columnsRemoved(change.parent, change.first, change.last);
}

/*!
    Begins a column move operation.

    When reimplementing a subclass, this method simplifies moving
    entities in your model. This method is responsible for moving
    persistent indexes in the model, which you would otherwise be
    required to do yourself. Using beginMoveRows and endMoveRows
    is an alternative to emitting layoutAboutToBeChanged and
    layoutChanged directly along with changePersistentIndexes.
    layoutAboutToBeChanged is emitted by this method for compatibility
    reasons.

    The \a sourceParent index corresponds to the parent from which the
    columns are moved; \a sourceFirst and \a sourceLast are the first and last
    column numbers of the columns to be moved. The \a destinationParent index
    corresponds to the parent into which those columns are moved. The \a
    destinationChild is the column to which the columns will be moved.  That
    is, the index at column \a sourceFirst in \a sourceParent will become
    column \a destinationChild in \a destinationParent, followed by all other
    columns up to \a sourceLast.

    However, when moving columns down in the same parent (\a sourceParent
    and \a destinationParent are equal), the columnss will be placed before the
    \a destinationChild index. That is, if you wish to move columns 0 and 1 so
    they will become columns 1 and 2, \a destinationChild should be 3. In this
    case, the new index for the source column \c i (which is between
    \a sourceFirst and \a sourceLast) is equal to
    \c {(destinationChild-sourceLast-1+i)}.

    Note that if \a sourceParent and \a destinationParent are the same,
    you must ensure that the \a destinationChild is not within the range
    of \a sourceFirst and \a sourceLast + 1.  You must also ensure that you
    do not attempt to move a column to one of its own children or ancestors.
    This method returns false if either condition is true, in which case you
    should abort your move operation.

    \sa endMoveColumns()

    \since 4.6
*/
bool QAbstractItemModel::beginMoveColumns(const QModelIndex &sourceParent, int sourceFirst, int sourceLast,
      const QModelIndex &destinationParent, int destinationChild)
{
   Q_ASSERT(sourceFirst >= 0);
   Q_ASSERT(sourceLast >= sourceFirst);
   Q_ASSERT(destinationChild >= 0);
   Q_D(QAbstractItemModel);

   if (!d->allowMove(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, Qt::Horizontal)) {
      return false;
   }

   QAbstractItemModelPrivate::Change sourceChange(sourceParent, sourceFirst, sourceLast);
   sourceChange.needsAdjust = sourceParent.isValid() && sourceParent.row() >= destinationChild &&
                              sourceParent.parent() == destinationParent;
   d->changes.push(sourceChange);
   int destinationLast = destinationChild + (sourceLast - sourceFirst);
   QAbstractItemModelPrivate::Change destinationChange(destinationParent, destinationChild, destinationLast);
   destinationChange.needsAdjust = destinationParent.isValid() && destinationParent.row() >= sourceLast &&
                                   destinationParent.parent() == sourceParent;
   d->changes.push(destinationChange);

   d->itemsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, Qt::Horizontal);

   emit columnsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild);
   emit layoutAboutToBeChanged();
   return true;
}

/*!
    Ends a column move operation.

    When implementing a subclass, you must call this
    function \e after moving data within the model's underlying data
    store.

    layoutChanged is emitted by this method for compatibility reasons.

    \sa beginMoveColumns()

    \since 4.6
*/
void QAbstractItemModel::endMoveColumns()
{
   Q_D(QAbstractItemModel);

   QAbstractItemModelPrivate::Change insertChange = d->changes.pop();
   QAbstractItemModelPrivate::Change removeChange = d->changes.pop();

   QModelIndex adjustedSource = removeChange.parent;
   QModelIndex adjustedDestination = insertChange.parent;

   const int numMoved = removeChange.last - removeChange.first + 1;
   if (insertChange.needsAdjust) {
      adjustedDestination = createIndex(adjustedDestination.row(), adjustedDestination.column() - numMoved,
                                        adjustedDestination.internalPointer());
   }

   if (removeChange.needsAdjust) {
      adjustedSource = createIndex(adjustedSource.row(), adjustedSource.column() + numMoved,
                                   adjustedSource.internalPointer());
   }

   d->itemsMoved(adjustedSource, removeChange.first, removeChange.last, adjustedDestination, insertChange.first,
                 Qt::Horizontal);

   emit columnsMoved(adjustedSource, removeChange.first, removeChange.last, adjustedDestination, insertChange.first);
   emit layoutChanged();
}

/*!
    Resets the model to its original state in any attached views.

    \note Use beginResetModel() and endResetModel() instead whenever possible.
    Use this method only if there is no way to call beginResetModel() before invalidating the model.
    Otherwise it could lead to unexpected behaviour, especially when used with proxy models.
*/
void QAbstractItemModel::reset()
{
   Q_D(QAbstractItemModel);
   emit modelAboutToBeReset();
   d->invalidatePersistentIndexes();
   QMetaObject::invokeMethod(this, "resetInternalData");
   emit modelReset();
}

/*!
    Begins a model reset operation.

    A reset operation resets the model to its current state in any attached views.

    \note Any views attached to this model will be reset as well.

    When a model is reset it means that any previous data reported from the
    model is now invalid and has to be queried for again. This also means that
    the current item and any selected items will become invalid.

    When a model radically changes its data it can sometimes be easier to just
    call this function rather than emit dataChanged() to inform other
    components when the underlying data source, or its structure, has changed.

    You must call this function before resetting any internal data structures in your model
    or proxy model.

    \sa modelAboutToBeReset(), modelReset(), endResetModel()
    \since 4.6
*/
void QAbstractItemModel::beginResetModel()
{
   emit modelAboutToBeReset();
}

/*!
    Completes a model reset operation.

    You must call this function after resetting any internal data structure in your model
    or proxy model.

    \sa beginResetModel()
    \since 4.6
*/
void QAbstractItemModel::endResetModel()
{
   Q_D(QAbstractItemModel);
   d->invalidatePersistentIndexes();
   QMetaObject::invokeMethod(this, "resetInternalData");
   emit modelReset();
}

void QAbstractItemModel::changePersistentIndex(const QModelIndex &from, const QModelIndex &to)
{
   Q_D(QAbstractItemModel);

   if (d->persistent.m_indexes.isEmpty()) {
      return;
   }

   // find the data and reinsert it sorted
   const QMultiMap<QModelIndex, QPersistentModelIndexData *>::iterator it = d->persistent.m_indexes.find(from);

   if (it != d->persistent.m_indexes.end()) {
      QPersistentModelIndexData *data = *it;
      d->persistent.m_indexes.erase(it);
      data->index = to;

      if (to.isValid()) {
         d->persistent.insertMultiAtEnd(to, data);
      } else {
         data->model = 0;
      }
   }
}

void QAbstractItemModel::changePersistentIndexList(const QModelIndexList &from,
      const QModelIndexList &to)
{
   Q_D(QAbstractItemModel);

   if (d->persistent.m_indexes.isEmpty()) {
      return;
   }

   QVector<QPersistentModelIndexData *> toBeReinserted;
   toBeReinserted.reserve(to.count());

   for (int i = 0; i < from.count(); ++i) {
      if (from.at(i) == to.at(i)) {
         continue;
      }

      const QMultiMap<QModelIndex, QPersistentModelIndexData *>::iterator it = d->persistent.m_indexes.find(from.at(i));

      if (it != d->persistent.m_indexes.end()) {
         QPersistentModelIndexData *data = *it;
         d->persistent.m_indexes.erase(it);
         data->index = to.at(i);

         if (data->index.isValid()) {
            toBeReinserted << data;
         } else {
            data->model = 0;
         }
      }
   }

   for (auto it = toBeReinserted.constBegin(); it != toBeReinserted.constEnd() ; ++it) {
      QPersistentModelIndexData *data = *it;
      d->persistent.insertMultiAtEnd(data->index, data);
   }
}

QModelIndexList QAbstractItemModel::persistentIndexList() const
{
   Q_D(const QAbstractItemModel);
   QModelIndexList result;

   for (auto it = d->persistent.m_indexes.constBegin(); it != d->persistent.m_indexes.constEnd(); ++it) {
      QPersistentModelIndexData *data = *it;
      result.append(data->index);
   }
   return result;
}

QAbstractTableModel::QAbstractTableModel(QObject *parent)
   : QAbstractItemModel(parent)
{

}

QAbstractTableModel::QAbstractTableModel(QAbstractItemModelPrivate &dd, QObject *parent)
   : QAbstractItemModel(dd, parent)
{

}

QAbstractTableModel::~QAbstractTableModel()
{
}

QModelIndex QAbstractTableModel::index(int row, int column, const QModelIndex &parent) const
{
   return hasIndex(row, column, parent) ? createIndex(row, column, 0) : QModelIndex();
}

QModelIndex QAbstractTableModel::parent(const QModelIndex &) const
{
   return QModelIndex();
}

bool QAbstractTableModel::hasChildren(const QModelIndex &parent) const
{
   if (parent.model() == this || !parent.isValid()) {
      return rowCount(parent) > 0 && columnCount(parent) > 0;
   }
   return false;
}

QAbstractListModel::QAbstractListModel(QObject *parent)
   : QAbstractItemModel(parent)
{
}

QAbstractListModel::QAbstractListModel(QAbstractItemModelPrivate &dd, QObject *parent)
   : QAbstractItemModel(dd, parent)
{
}

QAbstractListModel::~QAbstractListModel()
{
}

QModelIndex QAbstractListModel::index(int row, int column, const QModelIndex &parent) const
{
   return hasIndex(row, column, parent) ? createIndex(row, column, 0) : QModelIndex();
}

QModelIndex QAbstractListModel::parent(const QModelIndex & /* index */) const
{
   return QModelIndex();
}

/*!
    \internal

    Returns the number of columns in the list with the given \a parent.

    \sa rowCount()
*/

int QAbstractListModel::columnCount(const QModelIndex &parent) const
{
   return parent.isValid() ? 0 : 1;
}

bool QAbstractListModel::hasChildren(const QModelIndex &parent) const
{
   return parent.isValid() ? false : (rowCount() > 0);
}

bool QAbstractTableModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                  int row, int column, const QModelIndex &parent)
{
   if (! data || ! (action == Qt::CopyAction || action == Qt::MoveAction)) {
      return false;
   }

   QStringList types = mimeTypes();
   if (types.isEmpty()) {
      return false;
   }

   QString format = types.at(0);
   if (!data->hasFormat(format)) {
      return false;
   }

   QByteArray encoded = data->data(format);
   QDataStream stream(&encoded, QIODevice::ReadOnly);

   // if the drop is on an item, replace the data in the items
   if (parent.isValid() && row == -1 && column == -1) {
      int top = INT_MAX;
      int left = INT_MAX;
      QVector<int> rows, columns;
      QVector<QMap<int, QVariant> > data;

      while (!stream.atEnd()) {
         int r, c;
         QMap<int, QVariant> v;
         stream >> r >> c >> v;
         rows.append(r);
         columns.append(c);
         data.append(v);
         top = qMin(r, top);
         left = qMin(c, left);
      }

      for (int i = 0; i < data.size(); ++i) {
         int r = (rows.at(i) - top) + parent.row();
         int c = (columns.at(i) - left) + parent.column();
         if (hasIndex(r, c)) {
            setItemData(index(r, c), data.at(i));
         }
      }

      return true;
   }

   // otherwise insert new rows for the data
   return decodeData(row, column, parent, stream);
}

/*!
  \reimp
*/
bool QAbstractListModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                  int row, int column, const QModelIndex &parent)
{
   if (!data || !(action == Qt::CopyAction || action == Qt::MoveAction)) {
      return false;
   }

   QStringList types = mimeTypes();
   if (types.isEmpty()) {
      return false;
   }
   QString format = types.at(0);
   if (!data->hasFormat(format)) {
      return false;
   }

   QByteArray encoded = data->data(format);
   QDataStream stream(&encoded, QIODevice::ReadOnly);

   // if the drop is on an item, replace the data in the items
   if (parent.isValid() && row == -1 && column == -1) {
      int top = INT_MAX;
      int left = INT_MAX;
      QVector<int> rows, columns;
      QVector<QMap<int, QVariant> > data;

      while (!stream.atEnd()) {
         int r, c;
         QMap<int, QVariant> v;
         stream >> r >> c >> v;
         rows.append(r);
         columns.append(c);
         data.append(v);
         top = qMin(r, top);
         left = qMin(c, left);
      }

      for (int i = 0; i < data.size(); ++i) {
         int r = (rows.at(i) - top) + parent.row();
         if (columns.at(i) == left && hasIndex(r, 0)) {
            setItemData(index(r), data.at(i));
         }
      }

      return true;
   }

   if (row == -1) {
      row = rowCount(parent);
   }

   // otherwise insert new rows for the data
   return decodeData(row, column, parent, stream);
}

void QAbstractItemModelPrivate::Persistent::insertMultiAtEnd(const QModelIndex &key, QPersistentModelIndexData *data)
{
   QMultiMap<QModelIndex, QPersistentModelIndexData *>::iterator newIt = m_indexes.insertMulti(key, data);
   QMultiMap<QModelIndex, QPersistentModelIndexData *>::iterator it    = newIt + 1;

   while (it != m_indexes.end() && it.key() == key) {
      qSwap(*newIt, *it);
      newIt = it;
      ++it;
   }
}

QT_END_NAMESPACE
