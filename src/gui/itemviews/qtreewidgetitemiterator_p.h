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

#ifndef QTREEWIDGETITEMITERATOR_P_H
#define QTREEWIDGETITEMITERATOR_P_H

#include <qstack.h>

#ifndef QT_NO_TREEWIDGET
#include <qtreewidgetitemiterator.h>



class QTreeModel;
class QTreeWidgetItem;

class QTreeWidgetItemIteratorPrivate
{
   Q_DECLARE_PUBLIC(QTreeWidgetItemIterator)

 public:
   QTreeWidgetItemIteratorPrivate(QTreeWidgetItemIterator *q, QTreeModel *model)
      : m_currentIndex(0), m_model(model), q_ptr(q) {

   }

   QTreeWidgetItemIteratorPrivate(const QTreeWidgetItemIteratorPrivate &other)
      : m_currentIndex(other.m_currentIndex), m_model(other.m_model),
        m_parentIndex(other.m_parentIndex), q_ptr(other.q_ptr) {

   }

   QTreeWidgetItemIteratorPrivate &operator=(const QTreeWidgetItemIteratorPrivate &other) {
      m_currentIndex = other.m_currentIndex;
      m_parentIndex = other.m_parentIndex;
      m_model = other.m_model;
      return (*this);
   }

   ~QTreeWidgetItemIteratorPrivate() {
   }

   QTreeWidgetItem *nextSibling(const QTreeWidgetItem *item) const;
   void ensureValidIterator(const QTreeWidgetItem *itemToBeRemoved);

   QTreeWidgetItem *next(const QTreeWidgetItem *current);
   QTreeWidgetItem *previous(const QTreeWidgetItem *current);

 private:
   int m_currentIndex;
   QTreeModel *m_model;        // This iterator class should not have ownership of the model.
   QStack<int>  m_parentIndex;
   QTreeWidgetItemIterator *q_ptr;
};



#endif // QT_NO_TREEWIDGET

#endif //QTREEWIDGETITEMITERATOR_P_H
