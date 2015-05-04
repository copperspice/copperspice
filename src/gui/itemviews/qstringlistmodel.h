/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSTRINGLISTMODEL_H
#define QSTRINGLISTMODEL_H

#include <QtCore/qstringlist.h>
#include <QtGui/qabstractitemview.h>

QT_BEGIN_NAMESPACE


#ifndef QT_NO_STRINGLISTMODEL

class Q_GUI_EXPORT QStringListModel : public QAbstractListModel
{
   CS_OBJECT(QStringListModel)

 public:
   explicit QStringListModel(QObject *parent = 0);
   QStringListModel(const QStringList &strings, QObject *parent = 0);

   int rowCount(const QModelIndex &parent = QModelIndex()) const;

   QVariant data(const QModelIndex &index, int role) const;
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

   Qt::ItemFlags flags(const QModelIndex &index) const;

   bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
   bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

   void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

   QStringList stringList() const;
   void setStringList(const QStringList &strings);

   Qt::DropActions supportedDropActions() const;

 private:
   Q_DISABLE_COPY(QStringListModel)
   QStringList lst;
};

#endif // QT_NO_STRINGLISTMODEL

QT_END_NAMESPACE

#endif // QSTRINGLISTMODEL_H
