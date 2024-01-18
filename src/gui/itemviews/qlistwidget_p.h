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

#ifndef QLISTWIDGET_P_H
#define QLISTWIDGET_P_H

#include <qabstractitemmodel.h>
#include <qabstractitemview.h>
#include <qlistwidget.h>
#include <qitemdelegate.h>
#include <qlistview_p.h>
#include <qwidgetitemdata_p.h>

#ifndef QT_NO_LISTWIDGET

class QListModelLessThan
{
 public:
   inline bool operator()(QListWidgetItem *i1, QListWidgetItem *i2) const {
      return *i1 < *i2;
   }
};

class QListModelGreaterThan
{
 public:
   inline bool operator()(QListWidgetItem *i1, QListWidgetItem *i2) const {
      return *i2 < *i1;
   }
};

class QListModel : public QAbstractListModel
{
   GUI_CS_OBJECT(QListModel)

 public:
   QListModel(QListWidget *parent);
   ~QListModel();

   void clear();
   QListWidgetItem *at(int row) const;
   void insert(int row, QListWidgetItem *item);
   void insert(int row, const QStringList &items);
   void remove(QListWidgetItem *item);
   QListWidgetItem *take(int row);
   void move(int srcRow, int dstRow);

   int rowCount(const QModelIndex &parent = QModelIndex()) const override;

   QModelIndex index(QListWidgetItem *item) const;
   QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;

   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
   bool setData(const QModelIndex &index, const QVariant &value, int role) override;

   QMap<int, QVariant> itemData(const QModelIndex &index) const override;

   bool insertRows(int row, int count = 1, const QModelIndex &parent = QModelIndex()) override;
   bool removeRows(int row, int count = 1, const QModelIndex &parent = QModelIndex()) override;

   Qt::ItemFlags flags(const QModelIndex &index) const override;

   void sort(int column, Qt::SortOrder order) override;
   void ensureSorted(int column, Qt::SortOrder order, int start, int end);
   static bool itemLessThan(const QPair<QListWidgetItem *, int> &left, const QPair<QListWidgetItem *, int> &right);
   static bool itemGreaterThan(const QPair<QListWidgetItem *, int> &left, const QPair<QListWidgetItem *, int> &right);

   static QList<QListWidgetItem *>::iterator sortedInsertionIterator(const QList<QListWidgetItem *>::iterator &begin,
      const QList<QListWidgetItem *>::iterator &end, Qt::SortOrder order, QListWidgetItem *item);

   void itemChanged(QListWidgetItem *item);

   // dnd
   QStringList mimeTypes() const override;
   QMimeData *mimeData(const QModelIndexList &indexes) const override;

#ifndef QT_NO_DRAGANDDROP
   bool dropMimeData(const QMimeData *data, Qt::DropAction action,
      int row, int column, const QModelIndex &parent) override;
   Qt::DropActions supportedDropActions() const override;
#endif

   QMimeData *internalMimeData()  const;

 private:
   QList<QListWidgetItem *> items;

   // A cache must be mutable if get-functions should have const modifiers
   mutable QModelIndexList cachedIndexes;

   friend class QListWidget;
};



class QListWidgetPrivate : public QListViewPrivate
{
   Q_DECLARE_PUBLIC(QListWidget)

 public:
   QListWidgetPrivate() : QListViewPrivate(), sortOrder(Qt::AscendingOrder), sortingEnabled(false) {}

   inline QListModel *listModel() const {
      return qobject_cast<QListModel *>(model);
   }

   void setup();
   void _q_emitItemPressed(const QModelIndex &index);
   void _q_emitItemClicked(const QModelIndex &index);
   void _q_emitItemDoubleClicked(const QModelIndex &index);
   void _q_emitItemActivated(const QModelIndex &index);
   void _q_emitItemEntered(const QModelIndex &index);
   void _q_emitItemChanged(const QModelIndex &index);
   void _q_emitCurrentItemChanged(const QModelIndex &current, const QModelIndex &previous);
   void _q_sort();
   void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
   Qt::SortOrder sortOrder;
   bool sortingEnabled;
};

class QListWidgetItemPrivate
{
 public:
   QListWidgetItemPrivate(QListWidgetItem *item) : q(item), theid(-1) {}
   QListWidgetItem *q;
   QVector<QWidgetItemData> values;
   int theid;
};


#endif // QT_NO_LISTWIDGET

#endif // QLISTWIDGET_P_H
