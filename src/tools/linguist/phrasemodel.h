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

#ifndef PHRASEMODEL_H
#define PHRASEMODEL_H

#include <phrase.h>

#include <qabstractitemmodel.h>
#include <qlist.h>

class PhraseModel : public QAbstractTableModel
{
   CS_OBJECT(PhraseModel)

 public:
   PhraseModel(QObject *parent = nullptr)
      : QAbstractTableModel(parent) {
   }

   void removePhrases();
   QList<Phrase *> phraseList() const {
      return plist;
   }

   QModelIndex addPhrase(Phrase *p);
   void removePhrase(const QModelIndex &index);

   Phrase *phrase(const QModelIndex &index) const;
   void setPhrase(const QModelIndex &indx, Phrase *ph);
   QModelIndex index(Phrase *const phr) const;

   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override {
      return QAbstractTableModel::index(row, column, parent);
   }

   // from qabstracttablemodel
   int rowCount(const QModelIndex &) const override;
   int columnCount(const QModelIndex &) const override;
   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
   Qt::ItemFlags flags(const QModelIndex &index) const override;
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

   // This model will be displayed in a _TreeView_  which has a tendency to expand 'children' on double click
   bool hasChildren(const QModelIndex &parent) const override {
      return ! parent.isValid();
   }

 private:
   QList<Phrase *> plist;
};

#endif
