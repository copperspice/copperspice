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

#ifndef QTABLEWIDGET_P_H
#define QTABLEWIDGET_P_H

#include <qheaderview.h>
#include <qtablewidget.h>
#include <qabstractitemmodel.h>

#include <qabstractitemmodel_p.h>
#include <qtableview_p.h>
#include <qwidgetitemdata_p.h>

#ifndef QT_NO_TABLEWIDGET

// workaround for VC++ 6.0 linker bug
typedef bool(*LessThan)(const QPair<QTableWidgetItem *, int> &, const QPair<QTableWidgetItem *, int> &);

class QTableWidgetMimeData : public QMimeData
{
   GUI_CS_OBJECT(QTableWidgetMimeData)

 public:
   QList<QTableWidgetItem *> items;
};

class QTableModelLessThan
{
 public:
   inline bool operator()(QTableWidgetItem *i1, QTableWidgetItem *i2) const {
      return (*i1 < *i2);
   }
};

class QTableModelGreaterThan
{
 public:
   inline bool operator()(QTableWidgetItem *i1, QTableWidgetItem *i2) const {
      return (*i2 < *i1);
   }
};

class QTableModel : public QAbstractTableModel
{
   GUI_CS_OBJECT(QTableModel)

 public:
   // need this to separate header items from other items
   enum ItemFlagsExtension {
      ItemIsHeaderItem = 128
   };

   QTableModel(int rows, int columns, QTableWidget *parent);
   ~QTableModel();

   bool insertRows(int row, int count = 1, const QModelIndex &parent = QModelIndex()) override;
   bool insertColumns(int column, int count = 1, const QModelIndex &parent = QModelIndex()) override;

   bool removeRows(int row, int count = 1, const QModelIndex &parent = QModelIndex()) override;
   bool removeColumns(int column, int count = 1, const QModelIndex &parent = QModelIndex()) override;

   void setItem(int row, int column, QTableWidgetItem *item);
   QTableWidgetItem *takeItem(int row, int column);
   QTableWidgetItem *item(int row, int column) const;
   QTableWidgetItem *item(const QModelIndex &index) const;
   void removeItem(QTableWidgetItem *item);

   void setHorizontalHeaderItem(int section, QTableWidgetItem *item);
   void setVerticalHeaderItem(int section, QTableWidgetItem *item);
   QTableWidgetItem *takeHorizontalHeaderItem(int section);
   QTableWidgetItem *takeVerticalHeaderItem(int section);
   QTableWidgetItem *horizontalHeaderItem(int section);
   QTableWidgetItem *verticalHeaderItem(int section);

   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override {
      return QAbstractTableModel::index(row, column, parent);
   }

   QModelIndex index(const QTableWidgetItem *item) const;

   void setRowCount(int rows);
   void setColumnCount(int columns);

   int rowCount(const QModelIndex &parent = QModelIndex()) const override;
   int columnCount(const QModelIndex &parent = QModelIndex()) const override;

   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
   bool setData(const QModelIndex &index, const QVariant &value, int role) override;
   bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) override;

   QMap<int, QVariant> itemData(const QModelIndex &index) const override;

   QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
   bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) override;

   Qt::ItemFlags flags(const QModelIndex &index) const override;

   void sort(int column, Qt::SortOrder order) override;
   static bool itemLessThan(const QPair<QTableWidgetItem *, int> &left, const QPair<QTableWidgetItem *, int> &right);
   static bool itemGreaterThan(const QPair<QTableWidgetItem *, int> &left, const QPair<QTableWidgetItem *, int> &right);

   void ensureSorted(int column, Qt::SortOrder order, int start, int end);
   QVector<QTableWidgetItem *> columnItems(int column) const;
   void updateRowIndexes(QModelIndexList &indexes, int movedFromRow, int movedToRow);

   static QVector<QTableWidgetItem *>::iterator sortedInsertionIterator(const QVector<QTableWidgetItem *>::iterator &begin,
      const QVector<QTableWidgetItem *>::iterator &end, Qt::SortOrder order, QTableWidgetItem *item);

   bool isValid(const QModelIndex &index) const;
   inline long tableIndex(int row, int column) const {
      return (row * horizontalHeaderItems.count()) + column;
   }

   void clear();
   void clearContents();
   void itemChanged(QTableWidgetItem *item);

   QTableWidgetItem *createItem() const;
   const QTableWidgetItem *itemPrototype() const;
   void setItemPrototype(const QTableWidgetItem *item);

   // dnd
   QStringList mimeTypes() const override;
   QMimeData *mimeData(const QModelIndexList &indexes) const override;
   bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
   Qt::DropActions supportedDropActions() const override;

   QMimeData *internalMimeData()  const;

 private:
   const QTableWidgetItem *prototype;
   QVector<QTableWidgetItem *> tableItems;
   QVector<QTableWidgetItem *> verticalHeaderItems;
   QVector<QTableWidgetItem *> horizontalHeaderItems;

   // A cache must be mutable if get-functions should have const modifiers
   mutable QModelIndexList cachedIndexes;

   friend class QTableWidget;
};

class QTableWidgetPrivate : public QTableViewPrivate
{
   Q_DECLARE_PUBLIC(QTableWidget)

 public:
   QTableWidgetPrivate() : QTableViewPrivate() {}

   inline QTableModel *tableModel() const {
      return qobject_cast<QTableModel *>(model);
   }
   void setup();

   // view signals
   void _q_emitItemPressed(const QModelIndex &index);
   void _q_emitItemClicked(const QModelIndex &index);
   void _q_emitItemDoubleClicked(const QModelIndex &index);
   void _q_emitItemActivated(const QModelIndex &index);
   void _q_emitItemEntered(const QModelIndex &index);

   // model signals
   void _q_emitItemChanged(const QModelIndex &index);

   // selection signals
   void _q_emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &current);

   // sorting
   void _q_sort();
   void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
};

class QTableWidgetItemPrivate
{
 public:
   QTableWidgetItemPrivate(QTableWidgetItem *item) : q(item), id(-1) {}
   QTableWidgetItem *q;
   int id;
};

#endif // QT_NO_TABLEWIDGET

#endif // QTABLEWIDGET_P_H
