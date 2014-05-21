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

/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
****************************************************************************/

#include <QtGui>

#include "dragdropmodel.h"

DragDropModel::DragDropModel(const QStringList &strings, QObject *parent)
    : TreeModel(strings, parent)
{
}

bool DragDropModel::dropMimeData(const QMimeData *data,
    Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("text/plain"))
        return false;

    int beginRow;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
        beginRow = 0;
    else
        beginRow = rowCount(QModelIndex());

    QByteArray encodedData = data->data("text/plain");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QHash<qint64, QMap<int,QHash<int,QString> > > newItems;

    while (!stream.atEnd()) {
        qint64 id;
        int row;
        int column;
        QString text;
        stream >> id >> row >> column >> text;
        newItems[id][row][column] = text;
    }
    int rows = newItems.count();

    insertRows(beginRow, rows, parent);
    QMap<int,QHash<int,QString> > childItems;
    foreach (childItems, newItems.values()) {
        QHash<int,QString> rowItems;
        foreach (rowItems, childItems.values()) {
            foreach (int column, rowItems.keys()) {
                QModelIndex idx = index(beginRow, column, parent);
                setData(idx, rowItems[column]);
            }
            ++beginRow;
        }
    }

    return true;
}

Qt::ItemFlags DragDropModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = TreeModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QMimeData *DragDropModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            QString text = data(index, Qt::DisplayRole).toString();
            stream << index.internalId() << index.row() << index.column() << text;
        }
    }

    mimeData->setData("text/plain", encodedData);
    return mimeData;
}

QStringList DragDropModel::mimeTypes() const
{
    QStringList types;
    types << "text/plain";
    return types;
}

Qt::DropActions DragDropModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
