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

#include <qsqlquerymodel.h>

#include <qdebug.h>
#include <qsqldriver.h>
#include <qsqlfield.h>

#include <qsqlquerymodel_p.h>

QT_BEGIN_NAMESPACE

#define QSQL_PREFETCH 255

void QSqlQueryModelPrivate::prefetch(int limit)
{
   Q_Q(QSqlQueryModel);

   if (atEnd || limit <= bottom.row() || bottom.column() == -1) {
      return;
   }

   QModelIndex newBottom;
   const int oldBottomRow = qMax(bottom.row(), 0);

   // try to seek directly
   if (query.seek(limit)) {
      newBottom = q->createIndex(limit, bottom.column());
   } else {
      // have to seek back to our old position for MS Access
      int i = oldBottomRow;
      if (query.seek(i)) {
         while (query.next()) {
            ++i;
         }
         newBottom = q->createIndex(i, bottom.column());
      } else {
         // empty or invalid query
         newBottom = q->createIndex(-1, bottom.column());
      }
      atEnd = true; // this is the end.
   }
   if (newBottom.row() >= 0 && newBottom.row() > bottom.row()) {
      q->beginInsertRows(QModelIndex(), bottom.row() + 1, newBottom.row());
      bottom = newBottom;
      q->endInsertRows();
   } else {
      bottom = newBottom;
   }
}

QSqlQueryModelPrivate::~QSqlQueryModelPrivate()
{
}

void QSqlQueryModelPrivate::initColOffsets(int size)
{
   colOffsets.resize(size);
   memset(colOffsets.data(), 0, colOffsets.size() * sizeof(int));
}

/*!
    \class QSqlQueryModel
    \brief The QSqlQueryModel class provides a read-only data model for SQL
    result sets.

    \ingroup database
    \inmodule QtSql

    QSqlQueryModel is a high-level interface for executing SQL
    statements and traversing the result set. It is built on top of
    the lower-level QSqlQuery and can be used to provide data to
    view classes such as QTableView. For example:

    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 16

    We set the model's query, then we set up the labels displayed in
    the view header.

    QSqlQueryModel can also be used to access a database
    programmatically, without binding it to a view:

    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 21

    The code snippet above extracts the \c salary field from record 4 in
    the result set of the query \c{SELECT * from employee}. Assuming
    that \c salary is column 2, we can rewrite the last line as follows:

    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 22

    The model is read-only by default. To make it read-write, you
    must subclass it and reimplement setData() and flags(). Another
    option is to use QSqlTableModel, which provides a read-write
    model based on a single database table.

    The \l{sql/querymodel} example illustrates how to use
    QSqlQueryModel to display the result of a query. It also shows
    how to subclass QSqlQueryModel to customize the contents of the
    data before showing it to the user, and how to create a
    read-write model based on QSqlQueryModel.

    If the database doesn't return the number of selected rows in
    a query, the model will fetch rows incrementally.
    See fetchMore() for more information.

    \sa QSqlTableModel, QSqlRelationalTableModel, QSqlQuery,
        {Model/View Programming}, {Query Model Example}
*/

/*!
    Creates an empty QSqlQueryModel with the given \a parent.
 */
QSqlQueryModel::QSqlQueryModel(QObject *parent)
   : QAbstractTableModel(*new QSqlQueryModelPrivate, parent)
{
}

/*! \internal
 */
QSqlQueryModel::QSqlQueryModel(QSqlQueryModelPrivate &dd, QObject *parent)
   : QAbstractTableModel(dd, parent)
{
}

/*!
    Destroys the object and frees any allocated resources.

    \sa clear()
*/
QSqlQueryModel::~QSqlQueryModel()
{
}

/*!
    \since 4.1

    Fetches more rows from a database.
    This only affects databases that don't report back the size of a query
    (see QSqlDriver::hasFeature()).

    To force fetching of the entire result set, you can use the following:

    \snippet doc/src/snippets/code/src_sql_models_qsqlquerymodel.cpp 0

    \a parent should always be an invalid QModelIndex.

    \sa canFetchMore()
*/
void QSqlQueryModel::fetchMore(const QModelIndex &parent)
{
   Q_D(QSqlQueryModel);
   if (parent.isValid()) {
      return;
   }
   d->prefetch(qMax(d->bottom.row(), 0) + QSQL_PREFETCH);
}

/*!
    \since 4.1

    Returns true if it is possible to read more rows from the database.
    This only affects databases that don't report back the size of a query
    (see QSqlDriver::hasFeature()).

    \a parent should always be an invalid QModelIndex.

    \sa fetchMore()
 */
bool QSqlQueryModel::canFetchMore(const QModelIndex &parent) const
{
   Q_D(const QSqlQueryModel);
   return (!parent.isValid() && !d->atEnd);
}

/*! \fn int QSqlQueryModel::rowCount(const QModelIndex &parent) const
    \since 4.1

    If the database supports returning the size of a query
    (see QSqlDriver::hasFeature()), the number of rows of the current
    query is returned. Otherwise, returns the number of rows
    currently cached on the client.

    \a parent should always be an invalid QModelIndex.

    \sa canFetchMore(), QSqlDriver::hasFeature()
 */
int QSqlQueryModel::rowCount(const QModelIndex &index) const
{
   Q_D(const QSqlQueryModel);
   return index.isValid() ? 0 : d->bottom.row() + 1;
}

/*! \reimp
 */
int QSqlQueryModel::columnCount(const QModelIndex &index) const
{
   Q_D(const QSqlQueryModel);
   return index.isValid() ? 0 : d->rec.count();
}

/*!
    Returns the value for the specified \a item and \a role.

    If \a item is out of bounds or if an error occurred, an invalid
    QVariant is returned.

    \sa lastError()
*/
QVariant QSqlQueryModel::data(const QModelIndex &item, int role) const
{
   Q_D(const QSqlQueryModel);
   if (!item.isValid()) {
      return QVariant();
   }

   QVariant v;
   if (role & ~(Qt::DisplayRole | Qt::EditRole)) {
      return v;
   }

   if (!d->rec.isGenerated(item.column())) {
      return v;
   }
   QModelIndex dItem = indexInQuery(item);
   if (dItem.row() > d->bottom.row()) {
      const_cast<QSqlQueryModelPrivate *>(d)->prefetch(dItem.row());
   }

   if (!d->query.seek(dItem.row())) {
      d->error = d->query.lastError();
      return v;
   }

   return d->query.value(dItem.column());
}

/*!
    Returns the header data for the given \a role in the \a section
    of the header with the specified \a orientation.
*/
QVariant QSqlQueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_D(const QSqlQueryModel);
   if (orientation == Qt::Horizontal) {
      QVariant val = d->headers.value(section).value(role);
      if (role == Qt::DisplayRole && !val.isValid()) {
         val = d->headers.value(section).value(Qt::EditRole);
      }
      if (val.isValid()) {
         return val;
      }

      // See if it's an inserted column (iiq.column() != -1)
      QModelIndex dItem = indexInQuery(createIndex(0, section));

      if (role == Qt::DisplayRole && d->rec.count() > section && dItem.column() != -1) {
         return d->rec.fieldName(section);
      }
   }
   return QAbstractItemModel::headerData(section, orientation, role);
}

/*!
    This virtual function is called whenever the query changes. The
    default implementation does nothing.

    query() returns the new query.

    \sa query(), setQuery()
 */
void QSqlQueryModel::queryChange()
{
   // do nothing
}

/*!
    Resets the model and sets the data provider to be the given \a
    query. Note that the query must be active and must not be
    isForwardOnly().

    lastError() can be used to retrieve verbose information if there
    was an error setting the query.

    \note Calling setQuery() will remove any inserted columns.

    \sa query(), QSqlQuery::isActive(), QSqlQuery::setForwardOnly(), lastError()
*/
void QSqlQueryModel::setQuery(const QSqlQuery &query)
{
   Q_D(QSqlQueryModel);
   QSqlRecord newRec = query.record();
   bool columnsChanged = (newRec != d->rec);
   bool hasQuerySize = query.driver()->hasFeature(QSqlDriver::QuerySize);
   bool hasNewData = (newRec != QSqlRecord()) || !query.lastError().isValid();

   if (d->colOffsets.size() != newRec.count() || columnsChanged) {
      d->initColOffsets(newRec.count());
   }

   bool mustClearModel = d->bottom.isValid();
   if (mustClearModel) {
      d->atEnd = true;
      beginRemoveRows(QModelIndex(), 0, qMax(d->bottom.row(), 0));
      d->bottom = QModelIndex();
   }

   d->error = QSqlError();
   d->query = query;
   d->rec = newRec;

   if (mustClearModel) {
      endRemoveRows();
   }

   d->atEnd = false;

   if (columnsChanged && hasNewData) {
      reset();
   }

   if (!query.isActive() || query.isForwardOnly()) {
      d->atEnd = true;
      d->bottom = QModelIndex();
      if (query.isForwardOnly())
         d->error = QSqlError(QLatin1String("Forward-only queries "
                                            "cannot be used in a data model"),
                              QString(), QSqlError::ConnectionError);
      else {
         d->error = query.lastError();
      }
      return;
   }
   QModelIndex newBottom;
   if (hasQuerySize && d->query.size() > 0) {
      newBottom = createIndex(d->query.size() - 1, d->rec.count() - 1);
      beginInsertRows(QModelIndex(), 0, qMax(0, newBottom.row()));
      d->bottom = createIndex(d->query.size() - 1, columnsChanged ? 0 : d->rec.count() - 1);
      d->atEnd = true;
      endInsertRows();
   } else {
      newBottom = createIndex(-1, d->rec.count() - 1);
   }
   d->bottom = newBottom;

   queryChange();

   // fetchMore does the rowsInserted stuff for incremental models
   fetchMore();
}

/*! \overload

    Executes the query \a query for the given database connection \a
    db. If no database (or an invalid database) is specified, the
    default connection is used.

    lastError() can be used to retrieve verbose information if there
    was an error setting the query.

    Example:
    \snippet doc/src/snippets/code/src_sql_models_qsqlquerymodel.cpp 1

    \sa query(), queryChange(), lastError()
*/
void QSqlQueryModel::setQuery(const QString &query, const QSqlDatabase &db)
{
   setQuery(QSqlQuery(query, db));
}

/*!
    Clears the model and releases any acquired resource.
*/
void QSqlQueryModel::clear()
{
   Q_D(QSqlQueryModel);
   d->error = QSqlError();
   d->atEnd = true;
   d->query.clear();
   d->rec.clear();
   d->colOffsets.clear();
   d->bottom = QModelIndex();
   d->headers.clear();
}

/*!
    Sets the caption for a horizontal header for the specified \a role to
    \a value. This is useful if the model is used to
    display data in a view (e.g., QTableView).

    Returns true if \a orientation is Qt::Horizontal and
    the \a section refers to a valid section; otherwise returns
    false.

    Note that this function cannot be used to modify values in the
    database since the model is read-only.

    \sa data()
 */
bool QSqlQueryModel::setHeaderData(int section, Qt::Orientation orientation,
                                   const QVariant &value, int role)
{
   Q_D(QSqlQueryModel);
   if (orientation != Qt::Horizontal || section < 0 || columnCount() <= section) {
      return false;
   }

   if (d->headers.size() <= section) {
      d->headers.resize(qMax(section + 1, 16));
   }
   d->headers[section][role] = value;
   emit headerDataChanged(orientation, section, section);
   return true;
}

/*!
    Returns the QSqlQuery associated with this model.

    \sa setQuery()
*/
QSqlQuery QSqlQueryModel::query() const
{
   Q_D(const QSqlQueryModel);
   return d->query;
}

/*!
    Returns information about the last error that occurred on the
    database.

    \sa query()
*/
QSqlError QSqlQueryModel::lastError() const
{
   Q_D(const QSqlQueryModel);
   return d->error;
}

/*!
   Protected function which allows derived classes to set the value of
   the last error that occurred on the database to \a error.

   \sa lastError()
*/
void QSqlQueryModel::setLastError(const QSqlError &error)
{
   Q_D(QSqlQueryModel);
   d->error = error;
}

/*!
    Returns the record containing information about the fields of the
    current query. If \a row is the index of a valid row, the record
    will be populated with values from that row.

    If the model is not initialized, an empty record will be
    returned.

    \sa QSqlRecord::isEmpty()
*/
QSqlRecord QSqlQueryModel::record(int row) const
{
   Q_D(const QSqlQueryModel);
   if (row < 0) {
      return d->rec;
   }

   QSqlRecord rec = d->rec;
   for (int i = 0; i < rec.count(); ++i) {
      rec.setValue(i, data(createIndex(row, i), Qt::EditRole));
   }
   return rec;
}

/*! \overload

    Returns an empty record containing information about the fields
    of the current query.

    If the model is not initialized, an empty record will be
    returned.

    \sa QSqlRecord::isEmpty()
 */
QSqlRecord QSqlQueryModel::record() const
{
   Q_D(const QSqlQueryModel);
   return d->rec;
}

/*!
    Inserts \a count columns into the model at position \a column. The
    \a parent parameter must always be an invalid QModelIndex, since
    the model does not support parent-child relationships.

    Returns true if \a column is within bounds; otherwise returns false.

    By default, inserted columns are empty. To fill them with data,
    reimplement data() and handle any inserted column separately:

    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 23

    \sa removeColumns()
*/
bool QSqlQueryModel::insertColumns(int column, int count, const QModelIndex &parent)
{
   Q_D(QSqlQueryModel);
   if (count <= 0 || parent.isValid() || column < 0 || column > d->rec.count()) {
      return false;
   }

   beginInsertColumns(parent, column, column + count - 1);
   for (int c = 0; c < count; ++c) {
      QSqlField field;
      field.setReadOnly(true);
      field.setGenerated(false);
      d->rec.insert(column, field);
      if (d->colOffsets.size() < d->rec.count()) {
         int nVal = d->colOffsets.isEmpty() ? 0 : d->colOffsets[d->colOffsets.size() - 1];
         d->colOffsets.append(nVal);
         Q_ASSERT(d->colOffsets.size() >= d->rec.count());
      }
      for (int i = column + 1; i < d->colOffsets.count(); ++i) {
         ++d->colOffsets[i];
      }
   }
   endInsertColumns();
   return true;
}

/*!
    Removes \a count columns from the model starting from position \a
    column. The \a parent parameter must always be an invalid
    QModelIndex, since the model does not support parent-child
    relationships.

    Removing columns effectively hides them. It does not affect the
    underlying QSqlQuery.

    Returns true if the columns were removed; otherwise returns false.
 */
bool QSqlQueryModel::removeColumns(int column, int count, const QModelIndex &parent)
{
   Q_D(QSqlQueryModel);
   if (count <= 0 || parent.isValid() || column < 0 || column >= d->rec.count()) {
      return false;
   }

   beginRemoveColumns(parent, column, column + count - 1);

   int i;
   for (i = 0; i < count; ++i) {
      d->rec.remove(column);
   }
   for (i = column; i < d->colOffsets.count(); ++i) {
      d->colOffsets[i] -= count;
   }

   endRemoveColumns();
   return true;
}

/*!
    Returns the index of the value in the database result set for the
    given \a item in the model.

    The return value is identical to \a item if no columns or rows
    have been inserted, removed, or moved around.

    Returns an invalid model index if \a item is out of bounds or if
    \a item does not point to a value in the result set.

    \sa QSqlTableModel::indexInQuery(), insertColumns(), removeColumns()
*/
QModelIndex QSqlQueryModel::indexInQuery(const QModelIndex &item) const
{
   Q_D(const QSqlQueryModel);
   if (item.column() < 0 || item.column() >= d->rec.count()
         || !d->rec.isGenerated(item.column())
         || item.column() >= d->colOffsets.size()) {
      return QModelIndex();
   }
   return createIndex(item.row(), item.column() - d->colOffsets[item.column()],
                      item.internalPointer());
}

QT_END_NAMESPACE
