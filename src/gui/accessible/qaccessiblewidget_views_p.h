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

#ifndef ACCESSIBLE_ITEMVIEWS_H
#define ACCESSIBLE_ITEMVIEWS_H

#include <qpointer.h>
#include <qaccessible.h>
#include <qaccessiblewidget.h>
#include <qabstractitemview.h>
#include <qheaderview.h>

#ifndef QT_NO_ACCESSIBILITY

#ifndef QT_NO_ITEMVIEWS

class QAccessibleTableCell;
class QAccessibleTableHeaderCell;

class QAccessibleTable : public QAccessibleTableInterface, public QAccessibleObject
{
 public:
   explicit QAccessibleTable(QWidget *w);
   bool isValid() const override;

   QAccessible::Role role() const override;
   QAccessible::State state() const override;
   QString text(QAccessible::Text t) const override;
   QRect rect() const override;

   QAccessibleInterface *childAt(int x, int y) const override;
   int childCount() const override;
   int indexOfChild(const QAccessibleInterface *) const override;

   QAccessibleInterface *parent() const override;
   QAccessibleInterface *child(int index) const override;

   void *interface_cast(QAccessible::InterfaceType t) override;

   // table interface
   QAccessibleInterface *cellAt(int row, int column) const override;
   QAccessibleInterface *caption() const override;
   QAccessibleInterface *summary() const override;
   QString columnDescription(int column) const override;
   QString rowDescription(int row) const override;
   int columnCount() const override;
   int rowCount() const override;

   // selection
   int selectedCellCount() const override;
   int selectedColumnCount() const override;
   int selectedRowCount() const override;
   QList<QAccessibleInterface *> selectedCells() const override;
   QList<int> selectedColumns() const override;
   QList<int> selectedRows() const override;
   bool isColumnSelected(int column) const override;
   bool isRowSelected(int row) const override;
   bool selectRow(int row) override;
   bool selectColumn(int column) override;
   bool unselectRow(int row) override;
   bool unselectColumn(int column) override;

   QAbstractItemView *view() const;

   void modelChange(QAccessibleTableModelChangeEvent *event) override;

 protected:
   QAccessible::Role cellRole() const {
      switch (m_role) {
         case QAccessible::List:
            return QAccessible::ListItem;
         case QAccessible::Table:
            return QAccessible::Cell;
         case QAccessible::Tree:
            return QAccessible::TreeItem;
         default:
            Q_ASSERT(0);
      }
      return QAccessible::NoRole;
   }

   QHeaderView *horizontalHeader() const;
   QHeaderView *verticalHeader() const;

   // maybe vector
   typedef QHash<int, QAccessible::Id> ChildCache;
   mutable ChildCache childToId;

   virtual ~QAccessibleTable();

 private:
   // the child index for a model index
   inline int logicalIndex(const QModelIndex &index) const;
   QAccessible::Role m_role;
};

class QAccessibleTree : public QAccessibleTable
{
 public:
   explicit QAccessibleTree(QWidget *w)
      : QAccessibleTable(w)
   {}

   QAccessibleInterface *childAt(int x, int y) const override;
   int childCount() const override;
   QAccessibleInterface *child(int index) const override;

   int indexOfChild(const QAccessibleInterface *) const override;

   int rowCount() const override;

   // table interface
   QAccessibleInterface *cellAt(int row, int column) const override;
   QString rowDescription(int row) const override;
   bool isRowSelected(int row) const override;
   bool selectRow(int row) override;

 private:
   QModelIndex indexFromLogical(int row, int column = 0) const;

   inline int logicalIndex(const QModelIndex &index) const;
};

class QAccessibleTableCell: public QAccessibleInterface, public QAccessibleTableCellInterface, public QAccessibleActionInterface
{
 public:
   QAccessibleTableCell(QAbstractItemView *view, const QModelIndex &m_index, QAccessible::Role role);

   void *interface_cast(QAccessible::InterfaceType t) override;
   QObject *object() const override {
      return nullptr;
   }
   QAccessible::Role role() const override;
   QAccessible::State state() const override;
   QRect rect() const override;
   bool isValid() const override;

   QAccessibleInterface *childAt(int, int) const override {
      return nullptr;
   }
   int childCount() const override {
      return 0;
   }
   int indexOfChild(const QAccessibleInterface *) const override {
      return -1;
   }

   QString text(QAccessible::Text t) const override;
   void setText(QAccessible::Text t, const QString &text) override;

   QAccessibleInterface *parent() const override;
   QAccessibleInterface *child(int) const override;

   // cell interface
   int columnExtent() const override;
   QList<QAccessibleInterface *> columnHeaderCells() const override;
   int columnIndex() const override;
   int rowExtent() const override;
   QList<QAccessibleInterface *> rowHeaderCells() const override;
   int rowIndex() const override;
   bool isSelected() const override;
   QAccessibleInterface *table() const override;

   //action interface
   QStringList actionNames() const override;
   void doAction(const QString &actionName) override;
   QStringList keyBindingsForAction(const QString &actionName) const override;

 private:
   QHeaderView *verticalHeader() const;
   QHeaderView *horizontalHeader() const;
   QPointer<QAbstractItemView > view;
   QPersistentModelIndex m_index;
   QAccessible::Role m_role;

   void selectCell();
   void unselectCell();

   friend class QAccessibleTable;
   friend class QAccessibleTree;
};

class QAccessibleTableHeaderCell: public QAccessibleInterface
{
 public:
   // For header cells, pass the header view in addition
   QAccessibleTableHeaderCell(QAbstractItemView *view, int index, Qt::Orientation orientation);

   QObject *object() const override {
      return nullptr;
   }
   QAccessible::Role role() const override;
   QAccessible::State state() const override;
   QRect rect() const override;
   bool isValid() const override;

   QAccessibleInterface *childAt(int, int) const override {
      return nullptr;
   }
   int childCount() const override {
      return 0;
   }
   int indexOfChild(const QAccessibleInterface *) const override {
      return -1;
   }

   QString text(QAccessible::Text t) const override;
   void setText(QAccessible::Text t, const QString &text) override;

   QAccessibleInterface *parent() const override;
   QAccessibleInterface *child(int index) const override;

 private:
   QHeaderView *headerView() const;

   QPointer<QAbstractItemView> view;
   int index;
   Qt::Orientation orientation;

   friend class QAccessibleTable;
   friend class QAccessibleTree;
};

// This is the corner button on the top left of a table.
// It can be used to select all cells or it is not active at all.
// For now it is ignored.
class QAccessibleTableCornerButton: public QAccessibleInterface
{
 public:
   QAccessibleTableCornerButton(QAbstractItemView *view_)
      : view(view_)
   {}

   QObject *object() const override {
      return nullptr;
   }
   QAccessible::Role role() const override {
      return QAccessible::Pane;
   }
   QAccessible::State state() const override {
      return QAccessible::State();
   }
   QRect rect() const override {
      return QRect();
   }
   bool isValid() const override {
      return true;
   }

   QAccessibleInterface *childAt(int, int) const override {
      return nullptr;
   }
   int childCount() const override {
      return 0;
   }
   int indexOfChild(const QAccessibleInterface *) const override {
      return -1;
   }

   QString text(QAccessible::Text) const override {
      return QString();
   }
   void setText(QAccessible::Text, const QString &) override {}

   QAccessibleInterface *parent() const override {
      return QAccessible::queryAccessibleInterface(view);
   }
   QAccessibleInterface *child(int) const override {
      return nullptr;
   }

 private:
   QPointer<QAbstractItemView> view;
};

#endif

#endif // QT_NO_ACCESSIBILITY

#endif
