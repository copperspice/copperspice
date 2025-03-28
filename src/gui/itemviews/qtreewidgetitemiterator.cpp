/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qtreewidgetitemiterator_p.h>

#include <qtreewidget.h>

#include <qtreewidget_p.h>
#include <qwidgetitemdata_p.h>

#ifndef QT_NO_TREEWIDGET

QTreeWidgetItemIterator::QTreeWidgetItemIterator(const QTreeWidgetItemIterator &iter)
   :  d_ptr(new QTreeWidgetItemIteratorPrivate(*(iter.d_ptr))), current(iter.current), m_treeItemFlags(iter.m_treeItemFlags)
{
   Q_D(QTreeWidgetItemIterator);
   Q_ASSERT(d->m_model);
   d->m_model->iterators.append(this);
}

QTreeWidgetItemIterator::QTreeWidgetItemIterator(QTreeWidget *widget, IteratorFlags flags)
   : current(nullptr), m_treeItemFlags(flags)
{
   Q_ASSERT(widget);

   QTreeModel *model = qobject_cast<QTreeModel *>(widget->model());
   Q_ASSERT(model);
   d_ptr.reset(new QTreeWidgetItemIteratorPrivate(this, model));
   model->iterators.append(this);

   if (! model->rootItem->m_children.isEmpty()) {
      current = model->rootItem->child(0);
   }

   if (current && ! matchesFlags(current)) {
      ++(*this);
   }
}

QTreeWidgetItemIterator::QTreeWidgetItemIterator(QTreeWidgetItem *item, IteratorFlags flags)
   : d_ptr(new QTreeWidgetItemIteratorPrivate(this, qobject_cast<QTreeModel *>(item->m_view->model()))),
     current(item), m_treeItemFlags(flags)
{
   Q_D(QTreeWidgetItemIterator);
   Q_ASSERT(item);

   QTreeModel *model = qobject_cast<QTreeModel *>(item->m_view->model());
   Q_ASSERT(model);

   model->iterators.append(this);

   // Initialize m_currentIndex and m_parentIndex as it would be if we had traversed from the beginning
   QTreeWidgetItem *parent = item;
   parent = parent->parent();
   QTreeWidgetItem *root = d->m_model->rootItem;
   d->m_currentIndex = (parent ? parent : root)->indexOfChild(item);

   while (parent) {
      QTreeWidgetItem *itm = parent;
      parent = parent->parent();
      const int index = (parent ? parent : root)->indexOfChild(itm);
      d->m_parentIndex.prepend(index);
   }

   if (current && !matchesFlags(current)) {
      ++(*this);
   }
}

QTreeWidgetItemIterator::~QTreeWidgetItemIterator()
{
   d_func()->m_model->iterators.removeAll(this);
}

QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator=(const QTreeWidgetItemIterator &it)
{
   Q_D(QTreeWidgetItemIterator);

   if (d_func()->m_model != it.d_func()->m_model) {
      d_func()->m_model->iterators.removeAll(this);
      it.d_func()->m_model->iterators.append(this);
   }

   current = it.current;
   m_treeItemFlags = it.m_treeItemFlags;

   d->operator=(*it.d_func());

   return *this;
}

QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator++()
{
   if (current) {
      do {
         current = d_func()->next(current);
      } while (current && !matchesFlags(current));
   }

   return *this;
}

QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator--()
{
   if (current) {
      do {
         current = d_func()->previous(current);
      } while (current && !matchesFlags(current));
   }

   return *this;
}

bool QTreeWidgetItemIterator::matchesFlags(const QTreeWidgetItem *item) const
{
   if (! item) {
      return false;
   }

   if (m_treeItemFlags == All) {
      return true;
   }

   {
      Qt::ItemFlags itemFlags = item->flags();
      if ((m_treeItemFlags & Selectable) && ! (itemFlags & Qt::ItemIsSelectable)) {
         return false;
      }

      if ((m_treeItemFlags & NotSelectable) && (itemFlags & Qt::ItemIsSelectable)) {
         return false;
      }

      if ((m_treeItemFlags & DragEnabled) && !(itemFlags & Qt::ItemIsDragEnabled)) {
         return false;
      }

      if ((m_treeItemFlags & DragDisabled) && (itemFlags & Qt::ItemIsDragEnabled)) {
         return false;
      }

      if ((m_treeItemFlags & DropEnabled) && ! (itemFlags & Qt::ItemIsDropEnabled)) {
         return false;
      }

      if ((m_treeItemFlags & DropDisabled) && (itemFlags & Qt::ItemIsDropEnabled)) {
         return false;
      }

      if ((m_treeItemFlags & Enabled) && !(itemFlags & Qt::ItemIsEnabled)) {
         return false;
      }

      if ((m_treeItemFlags & Disabled) && (itemFlags & Qt::ItemIsEnabled)) {
         return false;
      }

      if ((m_treeItemFlags & Editable) && !(itemFlags & Qt::ItemIsEditable)) {
         return false;
      }

      if ((m_treeItemFlags & NotEditable) && (itemFlags & Qt::ItemIsEditable)) {
         return false;
      }
   }

   if (m_treeItemFlags & (Checked | NotChecked)) {
      // ### We only test the check state for column 0
      Qt::CheckState check = item->checkState(0);

      // PartiallyChecked matches as Checked.
      if ((m_treeItemFlags & Checked) && (check == Qt::Unchecked)) {
         return false;
      }

      if ((m_treeItemFlags & NotChecked) && (check != Qt::Unchecked)) {
         return false;
      }
   }

   if ((m_treeItemFlags & HasChildren) && !item->childCount()) {
      return false;
   }

   if ((m_treeItemFlags & NoChildren) && item->childCount()) {
      return false;
   }

   if ((m_treeItemFlags & Hidden) && !item->isHidden()) {
      return false;
   }

   if ((m_treeItemFlags & NotHidden) && item->isHidden()) {
      return false;
   }

   if ((m_treeItemFlags & Selected) && !item->isSelected()) {
      return false;
   }

   if ((m_treeItemFlags & Unselected) && item->isSelected()) {
      return false;
   }

   return true;
}

QTreeWidgetItem *QTreeWidgetItemIteratorPrivate::nextSibling(const QTreeWidgetItem *item) const
{
   Q_ASSERT(item);

   QTreeWidgetItem *next = nullptr;

   if (QTreeWidgetItem *par = item->parent()) {
      int i = par->indexOfChild(const_cast<QTreeWidgetItem *>(item));
      next = par->child(i + 1);
   } else {
      QTreeWidget *tw = item->treeWidget();
      int i = tw->indexOfTopLevelItem(const_cast<QTreeWidgetItem *>(item));
      next = tw->topLevelItem(i + 1);
   }

   return next;
}

QTreeWidgetItem *QTreeWidgetItemIteratorPrivate::next(const QTreeWidgetItem *current)
{
   if (!current) {
      return nullptr;
   }

   QTreeWidgetItem *next = nullptr;
   if (current->childCount()) {
      // walk the child
      m_parentIndex.push(m_currentIndex);
      m_currentIndex = 0;
      next = current->child(0);

   } else {
      // walk the sibling
      QTreeWidgetItem *parent = current->parent();

      next = parent ? parent->child(m_currentIndex + 1)
         : m_model->rootItem->child(m_currentIndex + 1);

      while (!next && parent) {
         // if we had no sibling walk up the parent and try the sibling of that
         parent = parent->parent();
         m_currentIndex = m_parentIndex.pop();
         next = parent ? parent->child(m_currentIndex + 1)
            : m_model->rootItem->child(m_currentIndex + 1);
      }

      if (next) {
         ++(m_currentIndex);
      }
   }

   return next;
}

QTreeWidgetItem *QTreeWidgetItemIteratorPrivate::previous(const QTreeWidgetItem *current)
{
   if (!current) {
      return nullptr;
   }

   QTreeWidgetItem *prev = nullptr;
   // walk the previous sibling
   QTreeWidgetItem *parent = current->parent();
   prev = parent ? parent->child(m_currentIndex - 1)
      : m_model->rootItem->child(m_currentIndex - 1);

   if (prev) {
      // Yes, we had a previous sibling but we need go down to the last leafnode.
      --m_currentIndex;

      while (prev && prev->childCount()) {
         m_parentIndex.push(m_currentIndex);
         m_currentIndex = prev->childCount() - 1;
         prev = prev->child(m_currentIndex);
      }

   } else if (parent) {
      m_currentIndex = m_parentIndex.pop();
      prev = parent;
   }

   return prev;
}

void QTreeWidgetItemIteratorPrivate::ensureValidIterator(const QTreeWidgetItem *itemToBeRemoved)
{
   Q_Q(QTreeWidgetItemIterator);
   Q_ASSERT(itemToBeRemoved);

   if (!q->current) {
      return;
   }
   QTreeWidgetItem *nextItem = q->current;

   // Do not walk to the ancestor to find the other item if they have the same parent.
   if (nextItem->parent() != itemToBeRemoved->parent()) {
      while (nextItem->parent() && nextItem != itemToBeRemoved) {
         nextItem = nextItem->parent();
      }
   }

   // If the item to be removed is an ancestor of the current iterator item,
   // we need to adjust the iterator.
   if (nextItem == itemToBeRemoved) {
      QTreeWidgetItem *parent = nextItem;
      nextItem = nullptr;
      while (parent && !nextItem) {
         nextItem = nextSibling(parent);
         parent = parent->parent();
      }

      if (nextItem) {
         // Set the iterator to the next valid item
         *q = QTreeWidgetItemIterator(nextItem, q->m_treeItemFlags);

         if (!(q->matchesFlags(nextItem))) {
            ++(*q);
         }

      } else {
         // set it to null.
         q->current = nullptr;
         m_parentIndex.clear();
         return;
      }
   }

   if (nextItem->parent() == itemToBeRemoved->parent()) {
      // They have the same parent, i.e. we have to adjust the m_currentIndex member of the iterator
      // if the deleted item is to the left of the nextItem.

      QTreeWidgetItem *par = itemToBeRemoved->parent();   // We know they both have the same parent.
      QTreeWidget *tw = itemToBeRemoved->treeWidget();    // ..and widget
      int indexOfItemToBeRemoved = par ? par->indexOfChild(const_cast<QTreeWidgetItem *>(itemToBeRemoved))
         : tw->indexOfTopLevelItem(const_cast<QTreeWidgetItem *>(itemToBeRemoved));
      int indexOfNextItem = par ? par->indexOfChild(nextItem) : tw->indexOfTopLevelItem(nextItem);

      if (indexOfItemToBeRemoved <= indexOfNextItem) {
         // A sibling to the left of us was deleted, adjust the m_currentIndex member of the iterator.
         // Note that the m_currentIndex will be wrong until the item is actually removed!
         m_currentIndex--;
      }
   }
}

#endif
