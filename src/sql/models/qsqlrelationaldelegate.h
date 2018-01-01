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

#ifndef QSQLRELATIONALDELEGATE_H
#define QSQLRELATIONALDELEGATE_H

#include <QtGui/qitemdelegate.h>
#include <QtGui/qlistview.h>
#include <QtGui/qcombobox.h>
#include <QtSql/qsqlrelationaltablemodel.h>

QT_BEGIN_NAMESPACE

class QSqlRelationalDelegate: public QItemDelegate
{
 public:

   explicit QSqlRelationalDelegate(QObject *aParent = 0)
      : QItemDelegate(aParent) {
   }

   ~QSqlRelationalDelegate() {
   }

   QWidget *createEditor(QWidget *aParent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
      const QSqlRelationalTableModel *sqlModel = qobject_cast<const QSqlRelationalTableModel *>(index.model());
      QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;

      if (!childModel) {
         return QItemDelegate::createEditor(aParent, option, index);
      }

      QComboBox *combo = new QComboBox(aParent);
      combo->setModel(childModel);
      combo->setModelColumn(childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn()));
      combo->installEventFilter(const_cast<QSqlRelationalDelegate *>(this));

      return combo;
   }

   void setEditorData(QWidget *editor, const QModelIndex &index) const {
      const QSqlRelationalTableModel *sqlModel = qobject_cast<const QSqlRelationalTableModel *>(index.model());
      QComboBox *combo = qobject_cast<QComboBox *>(editor);
      if (!sqlModel || !combo) {
         QItemDelegate::setEditorData(editor, index);
         return;
      }
      combo->setCurrentIndex(combo->findText(sqlModel->data(index).toString()));
   }

   void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
      if (!index.isValid()) {
         return;
      }

      QSqlRelationalTableModel *sqlModel = qobject_cast<QSqlRelationalTableModel *>(model);
      QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;
      QComboBox *combo = qobject_cast<QComboBox *>(editor);
      if (!sqlModel || !childModel || !combo) {
         QItemDelegate::setModelData(editor, model, index);
         return;
      }

      int currentItem = combo->currentIndex();
      int childColIndex = childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn());
      int childEditIndex = childModel->fieldIndex(sqlModel->relation(index.column()).indexColumn());
      sqlModel->setData(index,
                        childModel->data(childModel->index(currentItem, childColIndex), Qt::DisplayRole),
                        Qt::DisplayRole);
      sqlModel->setData(index,
                        childModel->data(childModel->index(currentItem, childEditIndex), Qt::EditRole),
                        Qt::EditRole);
   }

};

QT_END_NAMESPACE

#endif // QSQLRELATIONALDELEGATE_H
