/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef LOCALEMODEL_H
#define LOCALEMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QVariant>

class LocaleModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    LocaleModel(QObject *parent = 0);

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column,
                                const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value,
                            int role = Qt::EditRole);
private:
    QList<QVariant> m_data_list;
};

#endif // LOCALEMODEL_H
