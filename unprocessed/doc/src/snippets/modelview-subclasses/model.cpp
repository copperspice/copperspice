/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

/*
  model.cpp

  A simple model that uses a QVector as its data source.
*/

#include "model.h"

/*!
    Returns the number of items in the string list as the number of rows
    in the model.
*/

int LinearModel::rowCount(const QModelIndex &parent) const
{
    Q_USING(parent);

    return values.count();
}

/*
    Returns an appropriate value for the requested data.
    If the view requests an invalid index, an invalid variant is returned.
    If a header is requested then we just return the column or row number,
    depending on the orientation of the header.
    Any valid index that corresponds to a string in the list causes that
    string to be returned.
*/

/*!
    Returns a model index for other component to use when referencing the
    item specified by the given row, column, and type. The parent index
    is ignored.
*/

QModelIndex LinearModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent == QModelIndex() && row >= 0 && row < rowCount()
        && column == 0)
        return createIndex(row, column, 0);
    else
        return QModelIndex();
}

QVariant LinearModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);

    if (!index.isValid())
        return QVariant();

    return values.at(index.row());
}

/*!
    Returns Qt::ItemIsEditable so that all items in the vector can be edited.
*/

Qt::ItemFlags LinearModel::flags(const QModelIndex &index) const
{
    // all items in the model are editable
    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

/*!
    Changes an item in the string list, but only if the following conditions
    are met:

    * The index supplied is valid.
    * The index corresponds to an item to be shown in a view.
    * The role associated with editing text is specified.

    The dataChanged() signal is emitted if the item is changed.
*/

bool LinearModel::setData(const QModelIndex &index,
                          const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;
    values.replace(index.row(), value.toInt());
    emit dataChanged(index, index);
    return true;
}

/*!
    Inserts a number of rows into the model at the specified position.
*/

bool LinearModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(parent, position, position + rows - 1);

    values.insert(position, rows, 0);

    endInsertRows();
    return true;
}

/*!
    Removes a number of rows from the model at the specified position.
*/

bool LinearModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    values.remove(position, rows);

    endRemoveRows();
    return true;
}
