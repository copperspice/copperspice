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

#include <qundoview.h>
#include <qlistview_p.h>

#ifndef QT_NO_UNDOVIEW

#include <qabstractitemmodel.h>
#include <qicon.h>
#include <qpointer.h>
#include <qundogroup.h>
#include <qundostack.h>

class QUndoModel : public QAbstractItemModel
{
   GUI_CS_OBJECT(QUndoModel)

 public:
   QUndoModel(QObject *parent = nullptr);

   QUndoStack *stack() const;

   virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
   virtual QModelIndex parent(const QModelIndex &child) const override;
   virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
   virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
   virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

   QModelIndex selectedIndex() const;
   QItemSelectionModel *selectionModel() const;

   QString emptyLabel() const;
   void setEmptyLabel(const QString &label);

   void setCleanIcon(const QIcon &icon);
   QIcon cleanIcon() const;

   GUI_CS_SLOT_1(Public, void setStack(QUndoStack *stack))
   GUI_CS_SLOT_2(setStack)

 private:
   GUI_CS_SLOT_1(Private, void stackChanged())
   GUI_CS_SLOT_2(stackChanged)

   // slots
   void stackDestroyed(QObject *obj);
   void setStackCurrentIndex(const QModelIndex &index);

   QUndoStack *m_stack;
   QItemSelectionModel *m_sel_model;
   QString m_emty_label;
   QIcon m_clean_icon;
};

QUndoModel::QUndoModel(QObject *parent)
   : QAbstractItemModel(parent)
{
   m_stack = nullptr;
   m_sel_model = new QItemSelectionModel(this, this);

   connect(m_sel_model, &QItemSelectionModel::currentChanged, this, &QUndoModel::setStackCurrentIndex);

   m_emty_label = tr("<empty>");
}

QItemSelectionModel *QUndoModel::selectionModel() const
{
   return m_sel_model;
}

QUndoStack *QUndoModel::stack() const
{
   return m_stack;
}

void QUndoModel::setStack(QUndoStack *stack)
{
   if (m_stack == stack) {
      return;
   }

   if (m_stack != nullptr) {
      disconnect(m_stack, &QUndoStack::cleanChanged, this, &QUndoModel::stackChanged);
      disconnect(m_stack, &QUndoStack::indexChanged, this, &QUndoModel::stackChanged);
      disconnect(m_stack, &QUndoStack::destroyed,    this, &QUndoModel::stackDestroyed);
   }

   m_stack = stack;

   if (m_stack != nullptr) {
      connect(m_stack, &QUndoStack::cleanChanged, this, &QUndoModel::stackChanged);
      connect(m_stack, &QUndoStack::indexChanged, this, &QUndoModel::stackChanged);
      connect(m_stack, &QUndoStack::destroyed,    this, &QUndoModel::stackDestroyed);
   }

   stackChanged();
}

void QUndoModel::stackDestroyed(QObject *obj)
{
   if (obj != m_stack) {
      return;
   }

   m_stack = nullptr;
   stackChanged();
}

void QUndoModel::stackChanged()
{
   beginResetModel();
   endResetModel();
   m_sel_model->setCurrentIndex(selectedIndex(), QItemSelectionModel::ClearAndSelect);
}

void QUndoModel::setStackCurrentIndex(const QModelIndex &index)
{
   if (m_stack == nullptr) {
      return;
   }

   if (index == selectedIndex()) {
      return;
   }

   if (index.column() != 0) {
      return;
   }

   m_stack->setIndex(index.row());
}

QModelIndex QUndoModel::selectedIndex() const
{
   return m_stack == nullptr ? QModelIndex() : createIndex(m_stack->index(), 0);
}

QModelIndex QUndoModel::index(int row, int column, const QModelIndex &parent) const
{
   if (m_stack == nullptr) {
      return QModelIndex();
   }

   if (parent.isValid()) {
      return QModelIndex();
   }

   if (column != 0) {
      return QModelIndex();
   }

   if (row < 0 || row > m_stack->count()) {
      return QModelIndex();
   }

   return createIndex(row, column);
}

QModelIndex QUndoModel::parent(const QModelIndex &) const
{
   return QModelIndex();
}

int QUndoModel::rowCount(const QModelIndex &parent) const
{
   if (m_stack == nullptr) {
      return 0;
   }

   if (parent.isValid()) {
      return 0;
   }

   return m_stack->count() + 1;
}

int QUndoModel::columnCount(const QModelIndex &) const
{
   return 1;
}

QVariant QUndoModel::data(const QModelIndex &index, int role) const
{
   if (m_stack == nullptr) {
      return QVariant();
   }

   if (index.column() != 0) {
      return QVariant();
   }

   if (index.row() < 0 || index.row() > m_stack->count()) {
      return QVariant();
   }

   if (role == Qt::DisplayRole) {
      if (index.row() == 0) {
         return m_emty_label;
      }

      return m_stack->text(index.row() - 1);

   } else if (role == Qt::DecorationRole) {
      if (index.row() == m_stack->cleanIndex() && !m_clean_icon.isNull()) {
         return m_clean_icon;
      }

      return QVariant();
   }

   return QVariant();
}

QString QUndoModel::emptyLabel() const
{
   return m_emty_label;
}

void QUndoModel::setEmptyLabel(const QString &label)
{
   m_emty_label = label;
   stackChanged();
}

void QUndoModel::setCleanIcon(const QIcon &icon)
{
   m_clean_icon = icon;
   stackChanged();
}

QIcon QUndoModel::cleanIcon() const
{
   return m_clean_icon;
}

class QUndoViewPrivate : public QListViewPrivate
{
   Q_DECLARE_PUBLIC(QUndoView)

 public:

#ifdef QT_NO_UNDOGROUP
   QUndoViewPrivate()
      : model(nullptr)
   {
   }

#else
   QUndoViewPrivate()
      : group(nullptr), model(nullptr)
   {
   }

   QPointer<QUndoGroup> group;

#endif

   QUndoModel *model;

   void init();
};

void QUndoViewPrivate::init()
{
   Q_Q(QUndoView);

   model = new QUndoModel(q);
   q->setModel(model);
   q->setSelectionModel(model->selectionModel());
}

QUndoView::QUndoView(QWidget *parent)
   : QListView(*new QUndoViewPrivate(), parent)
{
   Q_D(QUndoView);
   d->init();
}

QUndoView::QUndoView(QUndoStack *stack, QWidget *parent)
   : QListView(*new QUndoViewPrivate(), parent)
{
   Q_D(QUndoView);
   d->init();
   setStack(stack);
}

#ifndef QT_NO_UNDOGROUP
QUndoView::QUndoView(QUndoGroup *group, QWidget *parent)
   : QListView(*new QUndoViewPrivate(), parent)
{
   Q_D(QUndoView);
   d->init();
   setGroup(group);
}
#endif

QUndoView::~QUndoView()
{
}

QUndoStack *QUndoView::stack() const
{
   Q_D(const QUndoView);
   return d->model->stack();
}

void QUndoView::setStack(QUndoStack *stack)
{
   Q_D(QUndoView);

#ifndef QT_NO_UNDOGROUP
   setGroup(nullptr);
#endif

   d->model->setStack(stack);
}

#ifndef QT_NO_UNDOGROUP

void QUndoView::setGroup(QUndoGroup *group)
{
   Q_D(QUndoView);

   if (d->group == group) {
      return;
   }

   if (d->group != nullptr) {
      disconnect(d->group.data(), &QUndoGroup::activeStackChanged, d->model, &QUndoModel::setStack);
   }

   d->group = group;

   if (d->group != nullptr) {
      connect(d->group.data(), &QUndoGroup::activeStackChanged, d->model, &QUndoModel::setStack);
      d->model->setStack(d->group->activeStack());

   } else {
      d->model->setStack(nullptr);

   }
}

QUndoGroup *QUndoView::group() const
{
   Q_D(const QUndoView);
   return d->group;
}

#endif

void QUndoView::setEmptyLabel(const QString &label)
{
   Q_D(QUndoView);
   d->model->setEmptyLabel(label);
}

QString QUndoView::emptyLabel() const
{
   Q_D(const QUndoView);
   return d->model->emptyLabel();
}

void QUndoView::setCleanIcon(const QIcon &icon)
{
   Q_D(const QUndoView);
   d->model->setCleanIcon(icon);
}

QIcon QUndoView::cleanIcon() const
{
   Q_D(const QUndoView);
   return d->model->cleanIcon();
}

#endif // QT_NO_UNDOVIEW
