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

#include <qsqlrelationaltablemodel.h>

#include <qdebug.h>
#include <qhash.h>
#include <qsqldatabase.h>
#include <qsqldriver.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlquery.h>
#include <qsqlrecord.h>
#include <qstringlist.h>

#include <qsqltablemodel_p.h>

class QSqlRelationalTableModelSql: public QSqlTableModelSql
{
 public:
   inline const static QString relTablePrefix(int i) {
      return QString::number(i).prepend("relTblAl_");
   }
};

typedef QSqlRelationalTableModelSql Sql;
class QRelatedTableModel;

struct QRelation {
 public:
   QRelation(): model(nullptr), m_parent(nullptr), m_dictInitialized(false)
   {
   }

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

void QRelation::init(QSqlRelationalTableModel *parent, const QSqlRelation &relation)
{
   Q_ASSERT(parent != nullptr);
   m_parent = parent;
   rel = relation;
}

void QRelation::populateModel()
{
   if (!isValid()) {
      return;
   }
   Q_ASSERT(m_parent != nullptr);

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

   if (model == nullptr) {
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
   model = nullptr;
   clearDictionary();
}

bool QRelation::isValid()
{
   return (rel.isValid() && m_parent != nullptr);
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

   QString fullyQualifiedFieldName(const QString &tableName, const QString &fieldName) const;

   int nameToIndex(const QString &name) const override;
   mutable QVector<QRelation> relations;
   QSqlRecord baseRec;                   // the record without relations
   void clearChanges();

   void clearCache() override;
   void revertCachedRow(int row) override;

   void translateFieldNames(QSqlRecord &values) const;
   QSqlRelationalTableModel::JoinMode joinMode;
};

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

   const QString fieldname = strippedFieldName(name);
   int idx = baseRec.indexOf(fieldname);
   if (idx == -1) {
      // If the name is an alias we can find it here.
      idx = QSqlTableModelPrivate::nameToIndex(name);
   }
   return idx;
}

void QSqlRelationalTableModelPrivate::clearCache()
{
   for (int i = 0; i < relations.count(); ++i) {
      relations[i].clearDictionary();
   }

   QSqlTableModelPrivate::clearCache();
}


QSqlRelationalTableModel::QSqlRelationalTableModel(QObject *parent, QSqlDatabase db)
   : QSqlTableModel(*new QSqlRelationalTableModelPrivate, parent, db)
{
}

QSqlRelationalTableModel::~QSqlRelationalTableModel()
{
}

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

      if (d->strategy != OnFieldChange) {
         const QSqlTableModelPrivate::ModifiedRow row = d->cache.value(index.row());

         if (row.op() != QSqlTableModelPrivate::None && row.rec().isGenerated(index.column())) {
            if (d->strategy == OnManualSubmit || row.op() != QSqlTableModelPrivate::Delete) {
               QVariant v = row.rec().value(index.column());
               if (v.isValid()) {
                  return relation.dictionary[v.toString()];
               }
            }
         }
      }
   }

   return QSqlTableModel::data(index, role);
}


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

QString QSqlRelationalTableModelPrivate::fullyQualifiedFieldName(const QString &tableName,
   const QString &fieldName) const
{
   QString retval;
   retval.append(tableName).append(QLatin1Char('.')).append(fieldName);

   return retval;
}


QString QSqlRelationalTableModel::selectStatement() const
{
   Q_D(const QSqlRelationalTableModel);

   if (tableName().isEmpty()) {
      return QString();
   }

   if (d->relations.isEmpty()) {
      return QSqlTableModel::selectStatement();
   }

   // Count how many times each field name occurs in the record
   QHash<QString, int> fieldNames;
   QStringList fieldList;

   for (int i = 0; i < d->baseRec.count(); ++i) {
      QSqlRelation relation = d->relations.value(i).rel;
      QString name;

      if (relation.isValid()) {
         // Count the display column name, not the original foreign key
         name = relation.displayColumn();
         if (d->db.driver()->isIdentifierEscaped(name, QSqlDriver::FieldName)) {
            name = d->db.driver()->stripDelimiters(name, QSqlDriver::FieldName);
         }

         const QSqlRecord rec = database().record(relation.tableName());
         for (int i = 0; i < rec.count(); ++i) {
            if (name.compare(rec.fieldName(i), Qt::CaseInsensitive) == 0) {
               name = rec.fieldName(i);
               break;
            }
         }
      } else {
         name = d->baseRec.fieldName(i);
      }
      fieldNames[name] = fieldNames.value(name, 0) + 1;
      fieldList.append(name);
   }

   QString fList;
   QString conditions;
   QString from = Sql::from(tableName());
   for (int i = 0; i < d->baseRec.count(); ++i) {
      QSqlRelation relation = d->relations.value(i).rel;
      const QString tableField = d->fullyQualifiedFieldName(tableName(), d->db.driver()->escapeIdentifier(d->baseRec.fieldName(i),
               QSqlDriver::FieldName));
      if (relation.isValid()) {
         const QString relTableAlias = Sql::relTablePrefix(i);
         QString displayTableField = d->fullyQualifiedFieldName(relTableAlias, relation.displayColumn());
         if (fieldNames.value(fieldList[i]) > 1) {
            QString relTableName = relation.tableName().section(QChar::fromLatin1('.'), -1, -1);
            if (d->db.driver()->isIdentifierEscaped(relTableName, QSqlDriver::TableName)) {
               relTableName = d->db.driver()->stripDelimiters(relTableName, QSqlDriver::TableName);
            }

            QString displayColumn = relation.displayColumn();

            if (d->db.driver()->isIdentifierEscaped(displayColumn, QSqlDriver::FieldName)) {
               displayColumn = d->db.driver()->stripDelimiters(displayColumn, QSqlDriver::FieldName);
            }

            const QString alias = QString("%1_%2_%3").formatArg(relTableName).formatArg(displayColumn).formatArg(fieldNames.value(fieldList[i]));

            displayTableField = Sql::as(displayTableField, alias);
            --fieldNames[fieldList[i]];
         }

         fList = Sql::comma(fList, displayTableField);

         const QString tblexpr = Sql::concat(relation.tableName(), relTableAlias);
         const QString relTableField = d->fullyQualifiedFieldName(relTableAlias, relation.indexColumn());
         const QString cond = Sql::eq(tableField, relTableField);
         if (d->joinMode == QSqlRelationalTableModel::InnerJoin) {
            // this needs fixing!! the below if is borken.
            // Use LeftJoin mode if you want correct behavior
            from = Sql::comma(from, tblexpr);
            conditions = Sql::et(conditions, cond);
         } else {
            from = Sql::concat(from, Sql::leftJoin(tblexpr));
            from = Sql::concat(from, Sql::on(cond));
         }
      } else {
         fList = Sql::comma(fList, tableField);
      }
   }

   if (fList.isEmpty())

   {
      return QString();
   }

   const QString stmt = Sql::concat(Sql::select(fList), from);
   const QString where = Sql::where(Sql::et(Sql::paren(conditions), Sql::paren(filter())));
   return Sql::concat(Sql::concat(stmt, where), orderByClause());
}

QSqlTableModel *QSqlRelationalTableModel::relationModel(int column) const
{
   Q_D(const QSqlRelationalTableModel);
   if ( column < 0 || column >= d->relations.count()) {
      return nullptr;
   }

   QRelation &relation = const_cast<QSqlRelationalTableModelPrivate *>(d)->relations[column];
   if (!relation.isValid()) {
      return nullptr;
   }

   if (!relation.model) {
      relation.populateModel();
   }

   return relation.model;
}

void QSqlRelationalTableModel::revertRow(int row)
{
   QSqlTableModel::revertRow(row);
}

void QSqlRelationalTableModel::clear()
{
   Q_D(QSqlRelationalTableModel);
   beginResetModel();
   d->clearChanges();
   d->relations.clear();
   QSqlTableModel::clear();
   endResetModel();
}

void QSqlRelationalTableModel::setJoinMode(QSqlRelationalTableModel::JoinMode joinMode)
{
   Q_D(QSqlRelationalTableModel);
   d->joinMode = joinMode;
}

bool QSqlRelationalTableModel::select()
{
   return QSqlTableModel::select();
}

void QSqlRelationalTableModel::setTable(const QString &table)
{
   Q_D(QSqlRelationalTableModel);

   // memorize the table before applying the relations
   d->baseRec = d->db.record(table);

   QSqlTableModel::setTable(table);
}

// internal
void QSqlRelationalTableModelPrivate::translateFieldNames(QSqlRecord &values) const
{
   for (int i = 0; i < values.count(); ++i) {
      if (relations.value(i).isValid()) {
         QVariant v = values.value(i);
         bool gen = values.isGenerated(i);
         values.replace(i, baseRec.field(i));
         values.setValue(i, v);
         values.setGenerated(i, gen);
      }
   }
}

bool QSqlRelationalTableModel::updateRowInTable(int row, const QSqlRecord &values)
{
   Q_D(QSqlRelationalTableModel);

   QSqlRecord rec = values;
   d->translateFieldNames(rec);

   return QSqlTableModel::updateRowInTable(row, rec);
}

bool QSqlRelationalTableModel::insertRowIntoTable(const QSqlRecord &values)
{
   Q_D(QSqlRelationalTableModel);

   QSqlRecord rec = values;
   d->translateFieldNames(rec);

   return QSqlTableModel::insertRowIntoTable(rec);
}

QString QSqlRelationalTableModel::orderByClause() const
{
   Q_D(const QSqlRelationalTableModel);

   const QSqlRelation rel = d->relations.value(d->sortColumn).rel;
   if (!rel.isValid()) {
      return QSqlTableModel::orderByClause();
   }

   QString f = d->fullyQualifiedFieldName(Sql::relTablePrefix(d->sortColumn), rel.displayColumn());
   f = d->sortOrder == Qt::AscendingOrder ? Sql::asc(f) : Sql::desc(f);
   return Sql::orderBy(f);
}

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
