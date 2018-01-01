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

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#include <objectdescriptionmodel.h>
#include <objectdescriptionmodel_p.h>
#include <phonondefs_p.h>
#include <platform_p.h>
#include <QList>
#include <objectdescription.h>
#include <phononnamespace_p.h>
#include <QMimeData>
#include <QStringList>
#include <QIcon>
#include <factory_p.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_OBJECTDESCRIPTIONMODEL

namespace Phonon
{

int ObjectDescriptionModelData::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return d->data.size();
}

QVariant ObjectDescriptionModelData::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= d->data.size() || index.column() != 0)
        return QVariant();

    switch(role)
    {
       case Qt::EditRole:
       case Qt::DisplayRole:
           return d->data.at(index.row())->name();
           break;
   
       case Qt::ToolTipRole:
           return d->data.at(index.row())->description();
           break;
   
       case Qt::DecorationRole:
           {
               QVariant icon = d->data.at(index.row())->property("icon");
               if (icon.isValid()) {
                   if (icon.type() == QVariant::String) {
                       return Platform::icon(icon.toString());
                   } else if (icon.type() == QVariant::Icon) {
                       return icon;
                   }
               }
           }
           return QVariant();
       default:
           return QVariant();
   }
}

Qt::ItemFlags ObjectDescriptionModelData::flags(const QModelIndex &index) const
{
    if(!index.isValid() || index.row() >= d->data.size() || index.column() != 0) {
        return Qt::ItemIsDropEnabled;
    }

    QVariant available = d->data.at(index.row())->property("available");
    if (available.isValid() && available.type() == QVariant::Bool && !available.toBool()) {
        return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}

QList<int> ObjectDescriptionModelData::tupleIndexOrder() const
{
    QList<int> ret;
    for (int i = 0; i < d->data.size(); ++i) {
        ret.append(d->data.at(i)->index());
    }
    return ret;
}

int ObjectDescriptionModelData::tupleIndexAtPositionIndex(int positionIndex) const
{
    return d->data.at(positionIndex)->index();
}

QMimeData *ObjectDescriptionModelData::mimeData(ObjectDescriptionType type, const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QModelIndexList::const_iterator end = indexes.constEnd();
    QModelIndexList::const_iterator index = indexes.constBegin();

    for(; index!=end; ++index) {
        if ((*index).isValid()) {
            stream << d->data.at((*index).row())->index();
        }
    }
    //pDebug() << Q_FUNC_INFO << "setting mimeData to" << mimeTypes(type).first() << "=>" << encodedData.toHex();
    mimeData->setData(mimeTypes(type).first(), encodedData);
    return mimeData;
}

void ObjectDescriptionModelData::moveUp(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= d->data.size() || index.row() < 1 || index.column() != 0)
        return;

    emit d->model->layoutAboutToBeChanged();
    QModelIndex above = index.sibling(index.row() - 1, index.column());
    d->data.swap(index.row(), above.row());
    QModelIndexList from, to;
    from << index << above;
    to << above << index;
    d->model->changePersistentIndexList(from, to);
    emit d->model->layoutChanged();
}

void ObjectDescriptionModelData::moveDown(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= d->data.size() - 1 || index.column() != 0)
        return;

    emit d->model->layoutAboutToBeChanged();
    QModelIndex below = index.sibling(index.row() + 1, index.column());
    d->data.swap(index.row(), below.row());
    QModelIndexList from, to;
    from << index << below;
    to << below << index;
    d->model->changePersistentIndexList(from, to);
    emit d->model->layoutChanged();
}

ObjectDescriptionModelData::ObjectDescriptionModelData(QAbstractListModel *model)
    : d(new ObjectDescriptionModelDataPrivate(model))
{
}

ObjectDescriptionModelData::~ObjectDescriptionModelData()
{
    delete d;
}

void ObjectDescriptionModelData::setModelData(const QList<QExplicitlySharedDataPointer<ObjectDescriptionData> > &newData)
{
    d->data = newData;
    d->model->reset();
}

QList<QExplicitlySharedDataPointer<ObjectDescriptionData> > ObjectDescriptionModelData::modelData() const
{
    return d->data;
}

QExplicitlySharedDataPointer<ObjectDescriptionData> ObjectDescriptionModelData::modelData(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= d->data.size() || index.column() != 0) {
        return QExplicitlySharedDataPointer<ObjectDescriptionData>(new ObjectDescriptionData(0));
    }
    return d->data.at(index.row());
}

Qt::DropActions ObjectDescriptionModelData::supportedDropActions() const
{   
    return Qt::MoveAction;
}

bool ObjectDescriptionModelData::dropMimeData(ObjectDescriptionType type, const QMimeData *data, Qt::DropAction action,
        int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action);
    Q_UNUSED(column);
    Q_UNUSED(parent);
 
    QString format = mimeTypes(type).first();
    if (!data->hasFormat(format)) {
        return false;
    }

    if (row == -1) {
        row = d->data.size();
    }

    QByteArray encodedData = data->data(format);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<QExplicitlySharedDataPointer<ObjectDescriptionData> > toInsert;
    while (!stream.atEnd()) {
        int otherIndex;
        stream >> otherIndex;
        ObjectDescriptionData *obj = ObjectDescriptionData::fromIndex(type, otherIndex);

        if (obj->isValid()) {
            toInsert << QExplicitlySharedDataPointer<ObjectDescriptionData>(obj);
        } else {
            delete obj;
        }
    }
    d->model->beginInsertRows(QModelIndex(), row, row + toInsert.size() - 1);
    for (int i = 0 ; i < toInsert.count(); ++i) {
        d->data.insert(row, toInsert.at(i));
    }
    d->model->endInsertRows();
    return true;
}


bool ObjectDescriptionModelData::removeRows(int row, int count, const QModelIndex &parent)
{    
    if (parent.isValid() || row + count > d->data.size()) {
        return false;
    }
    d->model->beginRemoveRows(parent, row, row + count - 1);
    for (;count > 0; --count) {
        d->data.removeAt(row);
    }
    d->model->endRemoveRows();
    return true;
}

QStringList ObjectDescriptionModelData::mimeTypes(ObjectDescriptionType type) const
{
    return QStringList(QLatin1String("application/x-phonon-objectdescription") + QString::number(static_cast<int>(type)));
}

} // namespace Phonon

#endif //QT_NO_PHONON_OBJECTDESCRIPTIONMODEL

QT_END_NAMESPACE
