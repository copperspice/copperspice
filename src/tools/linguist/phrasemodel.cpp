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

#include <phrasemodel.h>

void PhraseModel::removePhrases()
{
   int r = plist.count();

   if (r > 0) {
      beginResetModel();
      plist.clear();
      endResetModel();
   }
}

Phrase *PhraseModel::phrase(const QModelIndex &index) const
{
   return plist.at(index.row());
}

void PhraseModel::setPhrase(const QModelIndex &indx, Phrase *ph)
{
   int r = indx.row();

   plist[r] = ph;

   // update item in view
   const QModelIndex &si = index(r, 0);
   const QModelIndex &ei = index(r, 2);
   emit dataChanged(si, ei);
}

QModelIndex PhraseModel::addPhrase(Phrase *p)
{
   int r = plist.count();

   plist.append(p);

   // update phrases as we add them
   beginInsertRows(QModelIndex(), r, r);
   QModelIndex i = index(r, 0);
   endInsertRows();
   return i;
}

void PhraseModel::removePhrase(const QModelIndex &index)
{
   int r = index.row();
   beginRemoveRows(QModelIndex(), r, r);
   plist.removeAt(r);
   endRemoveRows();
}

QModelIndex PhraseModel::index(Phrase *const phr) const
{
   int row;
   if ((row = plist.indexOf(phr)) == -1) {
      return QModelIndex();
   }

   return index(row, 0);
}

int PhraseModel::rowCount(const QModelIndex &) const
{
   return plist.count();
}

int PhraseModel::columnCount(const QModelIndex &) const
{
   return 3;
}

QVariant PhraseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal)) {
      switch (section) {
         case 0:
            return tr("Source phrase");

         case 1:
            return tr("Translation");

         case 2:
            return tr("Definition");
      }
   }

   return QVariant();
}

Qt::ItemFlags PhraseModel::flags(const QModelIndex &index) const
{
   if (! index.isValid()) {
      return Qt::EmptyFlag;
   }

   Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

   // Edit is allowed for source & translation if item is from phrasebook
   if (plist.at(index.row())->phraseBook() && (index.column() != 2)) {
      flags |= Qt::ItemIsEditable;
   }

   return flags;
}

bool PhraseModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   int row = index.row();
   int column = index.column();

   if (!index.isValid() || row >= plist.count() || role != Qt::EditRole) {
      return false;
   }

   Phrase *phrase = plist.at(row);

   switch (column) {
      case 0:
         phrase->setSource(value.toString());
         break;

      case 1:
         phrase->setTarget(value.toString());
         break;

      case 2:
         phrase->setDefinition(value.toString());
         break;

      default:
         return false;
   }

   emit dataChanged(index, index);
   return true;
}

QVariant PhraseModel::data(const QModelIndex &index, int role) const
{
   int row = index.row();
   int column = index.column();

   if (row >= plist.count() || !index.isValid()) {
      return QVariant();
   }

   Phrase *phrase = plist.at(row);

   if (role == Qt::DisplayRole || (role == Qt::ToolTipRole && column != 2)) {
      switch (column) {
         case 0: // source phrase
            return phrase->source().simplified();

         case 1: // translation
            return phrase->target().simplified();

         case 2: // definition
            return phrase->definition();
      }
   } else if (role == Qt::EditRole && column != 2) {
      switch (column) {
         case 0: // source phrase
            return phrase->source();

         case 1: // translation
            return phrase->target();
      }
   }

   return QVariant();
}

