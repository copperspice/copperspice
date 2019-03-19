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

#include <qsqlrelationaltablemodel.h>

#include <qhash.h>
#include <qstringlist.h>
#include <qsqldatabase.h>
#include <qsqldriver.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlquery.h>
#include <qsqlrecord.h>

#include <qsqltablemodel_p.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

class QRelatedTableModel;

struct QRelation {
 public:
   QRelation(): model(0), m_parent(0), m_dictInitialized(false) {}
   void init(QSqlRelationalTableModel *parent, const QSqlRelation &relation);

   void populateModel();

   bool isDictionaryInitialized();
   void populateDictionary();
   void clearDictionary();

   void clear();
   bool isValid();

   QSqlRelation rel;
   QRelatedTableModel *model;
   QHash<QString, QVariant> dictionary;//maps keys to display values

 private:
   QSqlRelationalTableModel *m_parent;
   bool m_dictInitialized;
};

class QRelatedTableModel : public QSqlTableModel
{
 public:
   QRelatedTableModel(QRelation *rel, QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());
   bool select() override;

 private:
   bool firstSelect;
   QRelation *relation;
};
/*
    A QRelation must be initialized before it is considered valid.
    Note: population of the model and dictionary are kept separate
          from initialization, and are populated on an as needed basis.
*/
void QRelation::init(QSqlRelationalTableModel *parent, const QSqlRelation &relation)
{
   Q_ASSERT(parent != NULL);
   m_parent = parent;
   rel = relation;
}

void QRelation::populateModel()
{
   if (!isValid()) {
      return;
   }
   Q_ASSERT(m_parent != NULL);

   if (!model) {
      model = new QRelatedTableModel(this, m_parent, m_parent->database());
      model->setTable(rel.tableName());
      model->select();
   }
}

bool QRelation::isDictionaryInitialized()
{
   return m_dictInitialized;
}

void QRelation::populateDictionary()
{
   if (!isValid()) {
      return;
   }

   if (model ==  NULL) {
      populateModel();
   }

   QSqlRecord record;
   QString indexColumn;
   QString displayColumn;
   for (int i = 0; i < model->rowCount(); ++i) {
      record = model->record(i);

      indexColumn = rel.indexColumn();
      if (m_parent->database().driver()->isIdentifierEscaped(indexColumn, QSqlDriver::FieldName)) {
         indexColumn = m_parent->database().driver()->stripDelimiters(indexColumn, QSqlDriver::FieldName);
      }

      displayColumn = rel.displayColumn();
      if (m_parent->database().driver()->isIdentifierEscaped(displayColumn, QSqlDriver::FieldName)) {
         displayColumn = m_parent->database().driver()->stripDelimiters(displayColumn, QSqlDriver::FieldName);
      }

      dictionary[record.field(indexColumn).value().toString()] =
         record.field(displayColumn).value();
   }
   m_dictInitialized = true;
}

void QRelation::clearDictionary()
{
   dictionary.clear();
   m_dictInitialized = false;
}

void QRelation::clear()
{
   delete model;
   model = 0;
   clearDictionary();
}

bool QRelation::isValid()
{
   return (rel.isValid() && m_parent != NULL);
}



QRelatedTableModel::QRelatedTableModel(QRelation *rel, QObject *parent, QSqlDatabase db) :
   QSqlTableModel(parent, db), firstSelect(true), relation(rel)
{
}

bool QRelatedTableModel::select()
{
   if (firstSelect) {
      firstSelect = false;
      return QSqlTableModel::select();
   }

   relation->clearDictionary();
   bool res = QSqlTableModel::select();
   if (res) {
      relation->populateDictionary();
   }

   return res;
}


class QSqlRelationalTableModelPrivate: public QSqlTableModelPrivate
{
   Q_DECLARE_PUBLIC(QSqlRelationalTableModel)

 public:
   QSqlRelationalTableModelPrivate()
      : QSqlTableModelPrivate(),
        joinMode( QSqlRelationalTableModel::InnerJoin ) {
   }
   QString relationField(const QString &tableName, const QString &fieldName) const;

   int nameToIndex(const QString &name) const override;
   mutable QVector<QRelation> relations;
   QSqlRecord baseRec;                   // the record without relations
   void clearChanges();
   void clearEditBuffer() override;
   void clearCache() override;
   void revertCachedRow(int row) override;

   void translateFieldNames(int row, QSqlRecord &values) const;
   QSqlRelationalTableModel::JoinMode joinMode;
};

static void qAppendWhereClause(QString &query, const QString &clause1, const QString &clause2)
{
   if (clause1.isEmpty() && clause2.isEmpty()) {
      return;
   }

   if (clause1.isEmpty() || clause2.isEmpty()) {
      query.append(QLatin1String(" WHERE (")).append(clause1).append(clause2);

   } else
      query.append(QLatin1String(" WHERE (")).append(clause1).append(
         QLatin1String(") AND (")).append(clause2);

      query.append(QLatin1String(") "));
}

void QSqlRelationalTableModelPrivate::clearChanges()
{
   for (int i = 0; i < relations.count(); ++i) {
      QRelation &rel = relations[i];
      rel.clear();
   }
}

void QSqlRelationalTableModelPrivate::revertCachedRow(int row)
{
   QSqlTableModelPrivate::revertCachedRow(row);
}

int QSqlRelationalTableModelPrivate::nameToIndex(const QString &name) const
{
   QString fieldname = name;
   if (db.driver()->isIdentifierEscaped(fieldname, QSqlDriver::FieldName)) {
      fieldname = db.driver()->stripDelimiters(fieldname, QSqlDriver::FieldName);
   }
   return baseRec.indexOf(fieldname);
}

void QSqlRelationalTableModelPrivate::clearEditBuffer()
{
   editBuffer = baseRec;
   clearGenerated(editBuffer);
}

/*!
    \reimp
*/
void QSqlRelationalTableModelPrivate::clearCache()
{
   for (int i = 0; i < relations.count(); ++i) {
      relations[i].clearDictionary();
   }

   QSqlTableModelPrivate::clearCache();
}

/*!
    \class QSqlRelationalTableModel
    \brief The QSqlRelationalTableModel class provides an editable
    data model for a single database table, with foreign key support.

    \ingroup database
    \inmodule QtSql

    QSqlRelationalTableModel acts like QSqlTableModel, but allows
    columns to be set as foreign keys into other database tables.

    \table
    \row \o \inlineimage noforeignkeys.png
         \o \inlineimage foreignkeys.png
    \endtable

    The screenshot on the left shows a plain QSqlTableModel in a
    QTableView. Foreign keys (\c city and \c country) aren't resolved
    to human-readable values. The screenshot on the right shows a
    QSqlRelationalTableModel, with foreign keys resolved into
    human-readable text strings.

    The following code snippet shows how the QSqlRelationalTableModel
    was set up:

    \snippet examples/sql/relationaltablemodel/relationaltablemodel.cpp 0
    \codeline
    \snippet examples/sql/relationaltablemodel/relationaltablemodel.cpp 1
    \snippet examples/sql/relationaltablemodel/relationaltablemodel.cpp 2

    The setRelation() function calls establish a relationship between
    two tables. The first call specifies that column 2 in table \c
    employee is a foreign key that maps with field \c id of table \c
    city, and that the view should present the \c{city}'s \c name
    field to the user. The second call does something similar with
    column 3.

    If you use a read-write QSqlRelationalTableModel, you probably
    want to use QSqlRelationalDelegate on the view. Unlike the default
    delegate, QSqlRelationalDelegate provides a combobox for fields
    that are foreign keys into other tables. To use the class, simply
    call QAbstractItemView::setItemDelegate() on the view with an
    instance of QSqlRelationalDelegate:

    \snippet examples/sql/relationaltablemodel/relationaltablemodel.cpp 4

    The \l{sql/relationaltablemodel} example illustrates how to use
    QSqlRelationalTableModel in conjunction with
    QSqlRelationalDelegate to provide tables with foreign key
    support.

    \image relationaltable.png

    Notes:

    \list
    \o The table must have a primary key declared.
    \o The table's primary key may not contain a relation to
       another table.
    \o If a relational table contains keys that refer to non-existent
       rows in the referenced table, the rows containing the invalid
       keys will not be exposed through the model. The user or the
       database is responsible for keeping referential integrity.
    \o If a relation's display column name is also used as a column
       name in the main table, or if it is used as display column
       name in more than one relation it will be aliased. The alias is
       is the relation's table name and display column name joined
       by an underscore (e.g. tablename_columnname). All occurrences
       of the duplicate display column name are aliased when
       duplication is detected, but no aliasing is done to the column
       names in the main table. The aliasing doesn't affect
       QSqlRelation, so QSqlRelation::displayColumn() will return the
       original display column name, but QSqlRecord::fieldName() will
       return aliases.
    \o When using setData() the role should always be Qt::EditRole,
       and when using data() the role should always be Qt::DisplayRole.
    \endlist

    \sa QSqlRelation, QSqlRelationalDelegate,
        {Relational Table Model Example}
*/


/*!
    Creates an empty QSqlRelationalTableModel and sets the parent to \a parent
    and the database connection to \a db. If \a db is not valid, the
    default database connection will be used.
*/
QSqlRelationalTableModel::QSqlRelationalTableModel(QObject *parent, QSqlDatabase db)
   : QSqlTableModel(*new QSqlRelationalTableModelPrivate, parent, db)
{
}

/*!
    Destroys the object and frees any allocated resources.
*/
QSqlRelationalTableModel::~QSqlRelationalTableModel()
{
}

/*!
    \reimp
*/
QVariant QSqlRelationalTableModel::data(const QModelIndex &index, int role) const
{
   Q_D(const QSqlRelationalTableModel);

   if (role == Qt::DisplayRole && index.column() >= 0 && index.column() < d->relations.count() &&
         d->relations.value(index.column()).isValid()) {
      QRelation &relation = d->relations[index.column()];
      if (!relation.isDictionaryInitialized()) {
         relation.populateDictionary();
      }

      //only perform a dictionary lookup for the display value
      //when the value at index has been changed or added.
      //At an unmodified index, the underlying model will
      //already have the correct display value.
      QVariant v;
      switch (d->strategy) {
         case OnFieldChange:
            break;
         case OnRowChange:
            if ((index.row() == d->editIndex || index.row() == d->insertIndex)
                  && d->editBuffer.isGenerated(index.column())) {
               v = d->editBuffer.value(index.column());
            }
            break;
         case OnManualSubmit:
            const QSqlTableModelPrivate::ModifiedRow row = d->cache.value(index.row());
            if (row.op != QSqlTableModelPrivate::None && row.rec.isGenerated(index.column())) {
               v = row.rec.value(index.column());
            }
            break;
      }
      if (v.isValid()) {
         return relation.dictionary[v.toString()];
      }
   }
   return QSqlTableModel::data(index, role);
}

/*!
    Sets the data for the \a role in the item with the specified \a
    index to the \a value given. Depending on the edit strategy, the
    value might be applied to the database at once, or it may be
    cached in the model.

    Returns true if the value could be set, or false on error (for
    example, if \a index is out of bounds).

    For relational columns, \a value must be the index, not the
    display value. The index must also exist in the referenced
    table, otherwise the function returns false.

    \sa editStrategy(), data(), submit(), revertRow()
*/
bool QSqlRelationalTableModel::setData(const QModelIndex &index, const QVariant &value,
                                       int role)
{
   Q_D(QSqlRelationalTableModel);
   if ( role == Qt::EditRole && index.column() > 0 && index.column() < d->relations.count()
         && d->relations.value(index.column()).isValid()) {
      QRelation &relation = d->relations[index.column()];
      if (!relation.isDictionaryInitialized()) {
         relation.populateDictionary();
      }
      if (!relation.dictionary.contains(value.toString())) {
         return false;
      }
   }
   return QSqlTableModel::setData(index, value, role);
}

/*!
    Lets the specified \a column be a foreign index specified by \a relation.

    Example:

    \snippet examples/sql/relationaltablemodel/relationaltablemodel.cpp 0
    \codeline
    \snippet examples/sql/relationaltablemodel/relationaltablemodel.cpp 1

    The setRelation() call specifies that column 2 in table \c
    employee is a foreign key that maps with field \c id of table \c
    city, and that the view should present the \c{city}'s \c name
    field to the user.

    Note: The table's primary key may not contain a relation to another table.

    \sa relation()
*/
void QSqlRelationalTableModel::setRelation(int column, const QSqlRelation &relation)
{
   Q_D(QSqlRelationalTableModel);
   if (column < 0) {
      return;
   }
   if (d->relations.size() <= column) {
      d->relations.resize(column + 1);
   }
   d->relations[column].init(this, relation);
}
QSqlRelation QSqlRelationalTableModel::relation(int column) const
{
   Q_D(const QSqlRelationalTableModel);
   return d->relations.value(column).rel;
}

QString QSqlRelationalTableModelPrivate::relationField(const QString &tableName,
      const QString &fieldName) const
{
   QString retval;
   retval.append(tableName).append('.').append(fieldName);

   return retval;
}

/*!
    \reimp
*/
QString QSqlRelationalTableModel::selectStatement() const
{
   Q_D(const QSqlRelationalTableModel);
   QString query;

   if (tableName().isEmpty()) {
      return query;
   }
   if (d->relations.isEmpty()) {
      return QSqlTableModel::selectStatement();
   }

   QString tList;
   QString fList;
   QString where;

   QSqlRecord rec = d->baseRec;
   QStringList tables;
   const QRelation nullRelation;

   // Count how many times each field name occurs in the record
   QHash<QString, int> fieldNames;
   QStringList fieldList;
   for (int i = 0; i < rec.count(); ++i) {
      QSqlRelation relation = d->relations.value(i, nullRelation).rel;
      QString name;
      if (relation.isValid()) {
         // Count the display column name, not the original foreign key
         name = relation.displayColumn();
         if (d->db.driver()->isIdentifierEscaped(name, QSqlDriver::FieldName)) {
            name = d->db.driver()->stripDelimiters(name, QSqlDriver::FieldName);
         }

         QSqlRecord rec = database().record(relation.tableName());
         for (int i = 0; i < rec.count(); ++i) {
            if (name.compare(rec.fieldName(i), Qt::CaseInsensitive) == 0) {
               name = rec.fieldName(i);
               break;
            }
         }
      } else {
         name = rec.fieldName(i);
      }
      fieldNames.insert(name, fieldNames.value(name, 0) + 1);
      fieldList.append(name);
   }

   for (int i = 0; i < rec.count(); ++i) {
      QSqlRelation relation = d->relations.value(i, nullRelation).rel;

      if (relation.isValid()) {
         QString relTableAlias = QString::fromLatin1("relTblAl_%1").formatArg(i);

         if (!fList.isEmpty()) {
            fList.append(QLatin1String(", "));
         }

         fList.append(d->relationField(relTableAlias, relation.displayColumn()));

         // If there are duplicate field names they must be aliased
         if (fieldNames.value(fieldList[i]) > 1) {
            QString relTableName = relation.tableName().section(QChar::fromLatin1('.'), -1, -1);

            if (d->db.driver()->isIdentifierEscaped(relTableName, QSqlDriver::TableName)) {
               relTableName = d->db.driver()->stripDelimiters(relTableName, QSqlDriver::TableName);
            }

            QString displayColumn = relation.displayColumn();

            if (d->db.driver()->isIdentifierEscaped(displayColumn, QSqlDriver::FieldName)) {
               displayColumn = d->db.driver()->stripDelimiters(displayColumn, QSqlDriver::FieldName);
            }

            fList.append(QString::fromLatin1(" AS %1_%2_%3").formatArg(relTableName).formatArg(displayColumn).formatArg(fieldNames.value(fieldList[i])));
            fieldNames.insert(fieldList[i], fieldNames.value(fieldList[i]) - 1);
         }

         if (d->joinMode == QSqlRelationalTableModel::InnerJoin) {
            // this needs fixing!! the below if is borken.
            // Use LeftJoin mode if you want correct behavior
            tables.append(relation.tableName().append(QLatin1Char(' ')).append(relTableAlias));
            if (!where.isEmpty()) {
               where.append(QLatin1String(" AND "));
            }
            where.append(d->relationField(tableName(), d->db.driver()->escapeIdentifier(rec.fieldName(i), QSqlDriver::FieldName)));
            where.append(QLatin1String(" = "));
            where.append(d->relationField(relTableAlias, relation.indexColumn()));
         } else {
            tables.append(QLatin1String(" LEFT JOIN"));
            tables.append(relation.tableName().append(QLatin1Char(' ')).append(relTableAlias));
            tables.append(QLatin1String("ON"));

            QString clause;
            clause.append(d->relationField(tableName(), d->db.driver()->escapeIdentifier(rec.fieldName(i), QSqlDriver::FieldName)));
            clause.append(QLatin1String(" = "));
            clause.append(d->relationField(relTableAlias, relation.indexColumn()));

            tables.append(clause);
         }
      } else {
         if (!fList.isEmpty()) {
            fList.append(QLatin1String(", "));
         }
         fList.append(d->relationField(tableName(), d->db.driver()->escapeIdentifier(rec.fieldName(i), QSqlDriver::FieldName)));
      }
   }

   if (d->joinMode == QSqlRelationalTableModel::InnerJoin && !tables.isEmpty()) {
      tList.append(tables.join(QLatin1String(", ")));
      if (!tList.isEmpty()) {
         tList.prepend(QLatin1String(", "));
      }
   } else {
      tList.append(tables.join(QLatin1String(" ")));
   }

   if (fList.isEmpty()) {
      return query;
   }

   tList.prepend(tableName());
   query.append(QLatin1String("SELECT "));
   query.append(fList).append(QLatin1String(" FROM ")).append(tList);

   if (d->joinMode == QSqlRelationalTableModel::InnerJoin) {
      qAppendWhereClause(query, where, filter());
   } else if (!filter().isEmpty()) {
      query.append(QLatin1String(" WHERE ("));
      query.append(filter());
      query.append(QLatin1String(")"));
   }

   QString orderBy = orderByClause();
   if (!orderBy.isEmpty()) {
      query.append(QLatin1Char(' ')).append(orderBy);
   }

   return query;
}

/*!
    Returns a QSqlTableModel object for accessing the table for which
    \a column is a foreign key, or 0 if there is no relation for the
    given \a column.

    The returned object is owned by the QSqlRelationalTableModel.

    \sa setRelation(), relation()
*/
QSqlTableModel *QSqlRelationalTableModel::relationModel(int column) const
{
   Q_D(const QSqlRelationalTableModel);
   if ( column < 0 || column >= d->relations.count()) {
      return 0;
   }

   QRelation &relation = const_cast<QSqlRelationalTableModelPrivate *>(d)->relations[column];
   if (!relation.isValid()) {
      return 0;
   }

   if (!relation.model) {
      relation.populateModel();
   }
   return relation.model;
}

/*!
    \reimp
*/
void QSqlRelationalTableModel::revertRow(int row)
{
   QSqlTableModel::revertRow(row);
}

/*!
    \reimp
*/
void QSqlRelationalTableModel::clear()
{
   Q_D(QSqlRelationalTableModel);
   d->clearChanges();
   d->relations.clear();
   QSqlTableModel::clear();
}


/*!
    \enum QSqlRelationalTableModel::JoinMode
    \since 4.8

    This enum specifies the type of mode to use when joining two tables.

    \value InnerJoin Inner join mode, return rows when there is at least one
                     match in both tables.
    \value LeftJoin  Left join mode, returns all rows from the left table
                     (table_name1), even if there are no matches in the right
                     table (table_name2).

    \sa QSqlRelationalTableModel::setJoinMode()
*/

/*!
    \since 4.8
    Sets the SQL join mode to the value given by \a joinMode to show or hide
    rows with NULL foreign keys.

    In InnerJoin mode (the default) these rows will not be shown; use the
    LeftJoin mode if you want to show them.

    \sa QSqlRelationalTableModel::JoinMode
*/
void QSqlRelationalTableModel::setJoinMode(QSqlRelationalTableModel::JoinMode joinMode)
{
   Q_D(QSqlRelationalTableModel);
   d->joinMode = joinMode;
}
/*!
    \reimp
*/
bool QSqlRelationalTableModel::select()
{
   return QSqlTableModel::select();
}

/*!
    \reimp
*/
void QSqlRelationalTableModel::setTable(const QString &table)
{
   Q_D(QSqlRelationalTableModel);

   // memorize the table before applying the relations
   d->baseRec = d->db.record(table);

   QSqlTableModel::setTable(table);
}

/*! \internal
 */
void QSqlRelationalTableModelPrivate::translateFieldNames(int row, QSqlRecord &values) const
{
   Q_Q(const QSqlRelationalTableModel);

   for (int i = 0; i < values.count(); ++i) {
      int realCol = q->indexInQuery(q->createIndex(row, i)).column();
      if (realCol != -1 && relations.value(realCol).isValid()) {
         QVariant v = values.value(i);
         bool gen = values.isGenerated(i);
         values.replace(i, baseRec.field(realCol));
         values.setValue(i, v);
         values.setGenerated(i, gen);
      }
   }
}

/*!
    \reimp
*/
bool QSqlRelationalTableModel::updateRowInTable(int row, const QSqlRecord &values)
{
   Q_D(QSqlRelationalTableModel);

   QSqlRecord rec = values;
   d->translateFieldNames(row, rec);

   return QSqlTableModel::updateRowInTable(row, rec);
}

/*!
    \reimp
*/
bool QSqlRelationalTableModel::insertRowIntoTable(const QSqlRecord &values)
{
   Q_D(QSqlRelationalTableModel);

   QSqlRecord rec = values;
   d->translateFieldNames(0, rec);

   return QSqlTableModel::insertRowIntoTable(rec);
}

/*!
    \reimp
*/
QString QSqlRelationalTableModel::orderByClause() const
{
   Q_D(const QSqlRelationalTableModel);

   const QSqlRelation rel = d->relations.value(d->sortColumn).rel;
   if (!rel.isValid()) {
      return QSqlTableModel::orderByClause();
   }

   QString s = QLatin1String("ORDER BY ");
   s.append(d->relationField(QLatin1String("relTblAl_") + QString::number(d->sortColumn),
                             rel.displayColumn()));
   s += d->sortOrder == Qt::AscendingOrder ? QLatin1String(" ASC") : QLatin1String(" DESC");
   return s;
}

/*!
    \reimp
*/
bool QSqlRelationalTableModel::removeColumns(int column, int count, const QModelIndex &parent)
{
   Q_D(QSqlRelationalTableModel);

   if (parent.isValid() || column < 0 || column + count > d->rec.count()) {
      return false;
   }

   for (int i = 0; i < count; ++i) {
      d->baseRec.remove(column);
      if (d->relations.count() > column) {
         d->relations.remove(column);
      }
   }
   return QSqlTableModel::removeColumns(column, count, parent);
}

QT_END_NAMESPACE
