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

#ifndef QTREEWIDGETITEMITERATOR_H
#define QTREEWIDGETITEMITERATOR_H

#include <qglobal.h>
#include <qscopedpointer.h>

#include <cstddef>

#ifndef QT_NO_TREEWIDGET

class QTreeWidget;
class QTreeWidgetItem;
class QTreeModel;
class QTreeWidgetItemIteratorPrivate;

class Q_GUI_EXPORT QTreeWidgetItemIterator
{

   Q_DECLARE_PRIVATE(QTreeWidgetItemIterator)

 public:
   enum IteratorFlag {
      All           = 0x00000000,
      Hidden        = 0x00000001,
      NotHidden     = 0x00000002,
      Selected      = 0x00000004,
      Unselected    = 0x00000008,
      Selectable    = 0x00000010,
      NotSelectable = 0x00000020,
      DragEnabled   = 0x00000040,
      DragDisabled  = 0x00000080,
      DropEnabled   = 0x00000100,
      DropDisabled  = 0x00000200,
      HasChildren   = 0x00000400,
      NoChildren    = 0x00000800,
      Checked       = 0x00001000,
      NotChecked    = 0x00002000,
      Enabled       = 0x00004000,
      Disabled      = 0x00008000,
      Editable      = 0x00010000,
      NotEditable   = 0x00020000,
      UserFlag      = 0x01000000 // The first flag that can be used by the user.
   };

   using IteratorFlags = QFlags<IteratorFlag>;
   using size_type     = std::ptrdiff_t;

   QTreeWidgetItemIterator(const QTreeWidgetItemIterator &iter);
   explicit QTreeWidgetItemIterator(QTreeWidget *widget, IteratorFlags flags = All);
   explicit QTreeWidgetItemIterator(QTreeWidgetItem *item, IteratorFlags flags = All);

   ~QTreeWidgetItemIterator();

   QTreeWidgetItemIterator &operator=(const QTreeWidgetItemIterator &it);

   QTreeWidgetItemIterator &operator++();
   inline const QTreeWidgetItemIterator operator++(int);
   inline QTreeWidgetItemIterator &operator+=(size_type n);

   QTreeWidgetItemIterator &operator--();
   inline const QTreeWidgetItemIterator operator--(int);
   inline QTreeWidgetItemIterator &operator-=(size_type n);

   inline QTreeWidgetItem *operator*() const;

 private:
   bool matchesFlags(const QTreeWidgetItem *item) const;
   QScopedPointer<QTreeWidgetItemIteratorPrivate> d_ptr;
   QTreeWidgetItem *current;
   IteratorFlags flags;

   friend class QTreeModel;
};

inline const QTreeWidgetItemIterator QTreeWidgetItemIterator::operator++(int)
{
   QTreeWidgetItemIterator it = *this;
   ++(*this);
   return it;
}

inline const QTreeWidgetItemIterator QTreeWidgetItemIterator::operator--(int)
{
   QTreeWidgetItemIterator it = *this;
   --(*this);
   return it;
}

inline QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator+=(size_type n)
{
   if (n < 0) {
      return (*this) -= (-n);
   }
   while (current && n--) {
      ++(*this);
   }
   return *this;
}

inline QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator-=(size_type n)
{
   if (n < 0) {
      return (*this) += (-n);
   }
   while (current && n--) {
      --(*this);
   }
   return *this;
}

inline QTreeWidgetItem *QTreeWidgetItemIterator::operator*() const
{
   return current;
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QTreeWidgetItemIterator::IteratorFlags)

#endif // QT_NO_TREEWIDGET

#endif // QTREEWIDGETITEMITERATOR_H
