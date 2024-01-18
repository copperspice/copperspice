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

#ifndef QITEMSELECTIONMODEL_H
#define QITEMSELECTIONMODEL_H

#include <qset.h>
#include <qvector.h>
#include <qlist.h>
#include <qabstractitemmodel.h>
#include <qscopedpointer.h>

#ifndef QT_NO_ITEMVIEWS

class QItemSelection;
class QItemSelectionModel;
class QItemSelectionModelPrivate;

CS_DECLARE_METATYPE(QItemSelection)

class Q_GUI_EXPORT QItemSelectionRange
{
 public:
   inline QItemSelectionRange()
      : tl(), br()
   { }

   QItemSelectionRange(const QItemSelectionRange &other) = default;
   QItemSelectionRange(QItemSelectionRange &&other) = default;

   QItemSelectionRange &operator=(const QItemSelectionRange &other) = default;
   QItemSelectionRange &operator=(QItemSelectionRange &&other) = default;

   inline QItemSelectionRange(const QModelIndex &topLeft, const QModelIndex &bottomRight)
      : tl(topLeft), br(bottomRight)
   { }

   explicit inline QItemSelectionRange(const QModelIndex &index)
      : tl(index), br(index)
   { }

   void swap(QItemSelectionRange &other) {
      qSwap(tl, other.tl);
      qSwap(br, other.br);
   }

   int top() const {
      return tl.row();
   }

   int left() const {
      return tl.column();
   }

   int bottom() const {
      return br.row();
   }

   int right() const {
      return br.column();
   }

   int width() const {
      return br.column() - tl.column() + 1;
   }

   int height() const {
      return br.row() - tl.row() + 1;
   }

   const QPersistentModelIndex &topLeft() const {
      return tl;
   }

   const QPersistentModelIndex &bottomRight() const {
      return br;
   }

   QModelIndex parent() const {
      return tl.parent();
   }

   const QAbstractItemModel *model() const {
      return tl.model();
   }

   bool contains(const QModelIndex &index) const {
      return (parent() == index.parent()
            && tl.row() <= index.row() && tl.column() <= index.column()
            && br.row() >= index.row() && br.column() >= index.column());
   }

   bool contains(int row, int column, const QModelIndex &parentIndex) const {
      return (parent() == parentIndex
            && tl.row() <= row && tl.column() <= column
            && br.row() >= row && br.column() >= column);
   }

   bool intersects(const QItemSelectionRange &other) const;
   QItemSelectionRange intersected(const QItemSelectionRange &other) const;

   bool operator==(const QItemSelectionRange &other) const {
      return (tl == other.tl && br == other.br);
   }

   bool operator!=(const QItemSelectionRange &other) const {
      return !operator==(other);
   }

   bool operator<(const QItemSelectionRange &other) const {
      // Comparing parents will compare the models, but if two equivalent ranges
      // in two different models have invalid parents, they would appear the same

      if (other.tl.model() == tl.model()) {
         // parent has to be calculated, so we only do so once.
         const QModelIndex topLeftParent = tl.parent();
         const QModelIndex otherTopLeftParent = other.tl.parent();

         if (topLeftParent == otherTopLeftParent) {
            if (other.tl.row() == tl.row()) {
               if (other.tl.column() == tl.column()) {
                  if (other.br.row() == br.row()) {
                     return br.column() < other.br.column();
                  }
                  return br.row() < other.br.row();
               }
               return tl.column() < other.tl.column();
            }
            return tl.row() < other.tl.row();
         }

         return topLeftParent < otherTopLeftParent;
      }

      return tl.model() < other.tl.model();
   }

   inline bool isValid() const {
      return (tl.isValid() && br.isValid() && tl.parent() == br.parent()
            && top() <= bottom() && left() <= right());
   }

   bool isEmpty() const;

   QModelIndexList indexes() const;

 private:
   QPersistentModelIndex tl, br;
};

class Q_GUI_EXPORT QItemSelection : public QList<QItemSelectionRange>
{
 public:
   QItemSelection()
      : QList<QItemSelectionRange>()
   {
   }

   QItemSelection(const QModelIndex &topLeft, const QModelIndex &bottomRight);

   void select(const QModelIndex &topLeft, const QModelIndex &bottomRight);
   bool contains(const QModelIndex &index) const;
   QModelIndexList indexes() const;

#if defined (CS_DOXYPRESS)
   void merge(const QItemSelection &other, QItemSelectionModel::SelectionFlags command);

#else
   template <typename T = QItemSelectionModel>
   void merge(const QItemSelection &other, typename T::SelectionFlags command);

#endif

   static void split(const QItemSelectionRange &range, const QItemSelectionRange &other, QItemSelection *result);

 private:
   void cs_internal_merge(const QItemSelection &other, uint command);
};

class Q_GUI_EXPORT QItemSelectionModel : public QObject
{
   GUI_CS_OBJECT(QItemSelectionModel)

   GUI_CS_PROPERTY_READ(model, model)
   GUI_CS_PROPERTY_WRITE(model, setModel)
   GUI_CS_PROPERTY_NOTIFY(model, modelChanged)

   GUI_CS_PROPERTY_READ(hasSelection, hasSelection)
   GUI_CS_PROPERTY_NOTIFY(hasSelection, selectionChanged)
   GUI_CS_PROPERTY_STORED(hasSelection, false)
   GUI_CS_PROPERTY_DESIGNABLE(hasSelection, false)

   GUI_CS_PROPERTY_READ(currentIndex, currentIndex)
   GUI_CS_PROPERTY_NOTIFY(currentIndex, currentChanged)
   GUI_CS_PROPERTY_STORED(currentIndex, false)
   GUI_CS_PROPERTY_DESIGNABLE(currentIndex, false)

   GUI_CS_PROPERTY_READ(selection, selection)
   GUI_CS_PROPERTY_NOTIFY(selection, selectionChanged)
   GUI_CS_PROPERTY_STORED(selection, false)
   GUI_CS_PROPERTY_DESIGNABLE(selection, false)

   GUI_CS_PROPERTY_READ(selectedIndexes, selectedIndexes)
   GUI_CS_PROPERTY_NOTIFY(selectedIndexes, selectionChanged)
   GUI_CS_PROPERTY_STORED(selectedIndexes, false)
   GUI_CS_PROPERTY_DESIGNABLE(selectedIndexes, false)

   GUI_CS_FLAG(SelectionFlag, SelectionFlags)

 public:
   enum SelectionFlag : uint {
      NoUpdate       = 0x0000,
      Clear          = 0x0001,
      Select         = 0x0002,
      Deselect       = 0x0004,
      Toggle         = 0x0008,
      Current        = 0x0010,
      Rows           = 0x0020,
      Columns        = 0x0040,
      SelectCurrent  = Select | Current,
      ToggleCurrent  = Toggle | Current,
      ClearAndSelect = Clear  | Select
   };

   using SelectionFlags = QFlags<SelectionFlag>;

   explicit QItemSelectionModel(QAbstractItemModel *model = nullptr);
   explicit QItemSelectionModel(QAbstractItemModel *model, QObject *parent);

   QItemSelectionModel(const QItemSelectionModel &) = delete;
   QItemSelectionModel &operator=(const QItemSelectionModel &) = delete;

   virtual ~QItemSelectionModel();

   QModelIndex currentIndex() const;

   bool isSelected(const QModelIndex &index) const;
   bool isRowSelected(int row, const QModelIndex &parent) const;
   bool isColumnSelected(int column, const QModelIndex &parent) const;

   bool rowIntersectsSelection(int row, const QModelIndex &parent) const;
   bool columnIntersectsSelection(int column, const QModelIndex &parent) const;

   bool hasSelection() const;

   QModelIndexList selectedIndexes() const;
   QModelIndexList selectedRows(int column = 0) const;
   QModelIndexList selectedColumns(int row = 0) const;

   QItemSelection selection() const;

   const QAbstractItemModel *model() const;
   void setModel(QAbstractItemModel *model);

   GUI_CS_SLOT_1(Public, void setCurrentIndex(const QModelIndex &index, QItemSelectionModel::SelectionFlags command))
   GUI_CS_SLOT_2(setCurrentIndex)

   GUI_CS_SLOT_1(Public, virtual void select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command))
   GUI_CS_SLOT_OVERLOAD(select, (const QModelIndex &, QItemSelectionModel::SelectionFlags))

   GUI_CS_SLOT_1(Public, virtual void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command))
   GUI_CS_SLOT_OVERLOAD(select, (const QItemSelection &, QItemSelectionModel::SelectionFlags))

   GUI_CS_SLOT_1(Public, virtual void clear())
   GUI_CS_SLOT_2(clear)

   GUI_CS_SLOT_1(Public, virtual void reset())
   GUI_CS_SLOT_2(reset)

   GUI_CS_SLOT_1(Public, void clearSelection())
   GUI_CS_SLOT_2(clearSelection)

   GUI_CS_SLOT_1(Public, void clearCurrentIndex())
   GUI_CS_SLOT_2(clearCurrentIndex)

   GUI_CS_SIGNAL_1(Public, void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected))
   GUI_CS_SIGNAL_2(selectionChanged, selected, deselected)

   GUI_CS_SIGNAL_1(Public, void currentChanged(const QModelIndex &current, const QModelIndex &previous))
   GUI_CS_SIGNAL_2(currentChanged, current, previous)

   GUI_CS_SIGNAL_1(Public, void currentRowChanged(const QModelIndex &current, const QModelIndex &previous))
   GUI_CS_SIGNAL_2(currentRowChanged, current, previous)

   GUI_CS_SIGNAL_1(Public, void currentColumnChanged(const QModelIndex &current, const QModelIndex &previous))
   GUI_CS_SIGNAL_2(currentColumnChanged, current, previous)

   GUI_CS_SIGNAL_1(Public, void modelChanged(QAbstractItemModel *model))
   GUI_CS_SIGNAL_2(modelChanged, model)

 protected:
   QItemSelectionModel(QItemSelectionModelPrivate &dd, QAbstractItemModel *model);
   void emitSelectionChanged(const QItemSelection &newSelection, const QItemSelection &oldSelection);

   QScopedPointer<QItemSelectionModelPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QItemSelectionModel)

   GUI_CS_SLOT_1(Private, void _q_columnsAboutToBeRemoved(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_columnsAboutToBeRemoved)

   GUI_CS_SLOT_1(Private, void _q_rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_rowsAboutToBeRemoved)

   GUI_CS_SLOT_1(Private, void _q_columnsAboutToBeInserted(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_columnsAboutToBeInserted)

   GUI_CS_SLOT_1(Private, void _q_rowsAboutToBeInserted(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_rowsAboutToBeInserted)

   GUI_CS_SLOT_1(Private, void _q_layoutAboutToBeChanged())
   GUI_CS_SLOT_2(_q_layoutAboutToBeChanged)

   GUI_CS_SLOT_1(Private, void _q_layoutChanged())
   GUI_CS_SLOT_2(_q_layoutChanged)
};

// dummy implentation of qHash() necessary for instantiating QList<QItemSelectionRange>::toSet() with MSVC
inline uint qHash(const QItemSelectionRange &)
{
   return 0;
}

#if ! defined (CS_DOXYPRESS)

// must be at the end of the file
template <typename T>
void QItemSelection::merge(const QItemSelection &other, typename T::SelectionFlags command)
{
   cs_internal_merge(other, command);
}

#endif

Q_GUI_EXPORT QDebug operator<<(QDebug, const QItemSelectionRange &);

Q_DECLARE_OPERATORS_FOR_FLAGS(QItemSelectionModel::SelectionFlags)

#endif // QT_NO_ITEMVIEWS

#endif
