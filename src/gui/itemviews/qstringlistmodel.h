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

#ifndef QSTRINGLISTMODEL_H
#define QSTRINGLISTMODEL_H

#include <qstringlist.h>
#include <qabstractitemview.h>

#ifndef QT_NO_STRINGLISTMODEL

class Q_GUI_EXPORT QStringListModel : public QAbstractListModel
{
   GUI_CS_OBJECT(QStringListModel)

 public:
   explicit QStringListModel(QObject *parent = nullptr);
   explicit QStringListModel(const QStringList &strings, QObject *parent = nullptr);

   QStringListModel(const QStringListModel &) = delete;
   QStringListModel &operator=(const QStringListModel &) = delete;

   int rowCount(const QModelIndex &parent = QModelIndex()) const override;
   QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;

   QVariant data(const QModelIndex &index, int role) const override;
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

   Qt::ItemFlags flags(const QModelIndex &index) const override;

   bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
   bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

   void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

   QStringList stringList() const;
   void setStringList(const QStringList &strings);

   Qt::DropActions supportedDropActions() const override;

 private:
   QStringList lst;
};

#endif // QT_NO_STRINGLISTMODEL

#endif
