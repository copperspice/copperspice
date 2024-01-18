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

#ifndef QITEMSELECTIONMODEL_P_H
#define QITEMSELECTIONMODEL_P_H

#ifndef QT_NO_ITEMVIEWS

class QItemSelectionModelPrivate
{
   Q_DECLARE_PUBLIC(QItemSelectionModel)

 public:
   QItemSelectionModelPrivate()
      : model(nullptr), currentCommand(QItemSelectionModel::NoUpdate),
        tableSelected(false), tableColCount(0), tableRowCount(0)
   {
   }

   virtual ~QItemSelectionModelPrivate() {}

   QItemSelection expandSelection(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) const;

   void initModel(QAbstractItemModel *model);

   void _q_rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
   void _q_columnsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
   void _q_rowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
   void _q_columnsAboutToBeInserted(const QModelIndex &parent, int start, int end);
   void _q_layoutAboutToBeChanged(const QList<QPersistentModelIndex> &parents = QList<QPersistentModelIndex>(),
      QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint);
   void _q_layoutChanged(const QList<QPersistentModelIndex> &parents = QList<QPersistentModelIndex>(),
      QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint);

   inline void remove(QList<QItemSelectionRange> &r) {
      QList<QItemSelectionRange>::const_iterator it = r.constBegin();
      for (; it != r.constEnd(); ++it) {
         ranges.removeAll(*it);
      }
   }

   inline void finalize() {
      ranges.merge(currentSelection, currentCommand);
      if (!currentSelection.isEmpty()) { // ### perhaps this should be in QList
         currentSelection.clear();
      }
   }

   QPointer<QAbstractItemModel> model;
   QItemSelection ranges;
   QItemSelection currentSelection;
   QPersistentModelIndex currentIndex;
   QItemSelectionModel::SelectionFlags currentCommand;
   QVector<QPersistentModelIndex> savedPersistentIndexes;
   QVector<QPersistentModelIndex> savedPersistentCurrentIndexes;
   QVector<QPair<QPersistentModelIndex, uint>> savedPersistentRowLengths;
   QVector<QPair<QPersistentModelIndex, uint>> savedPersistentCurrentRowLengths;

   // optimization when all indexes are selected
   bool tableSelected;
   QPersistentModelIndex tableParent;
   int tableColCount, tableRowCount;

 protected:
   QItemSelectionModel *q_ptr;

};

#endif // QT_NO_ITEMVIEWS


#endif // QITEMSELECTIONMODEL_P_H
