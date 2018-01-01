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

#include "bookmarkmodel.h"
#include "bookmarkitem.h"

#include <QtCore/QMimeData>
#include <QtCore/QStack>

#include <QtGui/QApplication>
#include <QtGui/QStyle>
#include <QtGui/QTreeView>

const qint32 VERSION = 0xe53798;
const QLatin1String MIMETYPE("application/bookmarks.assistant");

BookmarkModel::BookmarkModel()
    : QAbstractItemModel()
    , m_folder(false)
    , m_editable(false)
    , rootItem(0)
{
}

BookmarkModel::~BookmarkModel()
{
    delete rootItem;
}

QByteArray
BookmarkModel::bookmarks() const
{
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << qint32(VERSION);

    const QModelIndex &root = index(0,0, QModelIndex()).parent();
    for (int i = 0; i < rowCount(root); ++i)
        collectItems(index(i, 0, root), 0, &stream);

    return ba;
}

void
BookmarkModel::setBookmarks(const QByteArray &bookmarks)
{
    beginResetModel();

    delete rootItem;
    folderIcon = QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon);
    bookmarkIcon = QIcon(QLatin1String(":/trolltech/assistant/images/bookmark.png"));

    rootItem = new BookmarkItem(DataVector() << tr("Name") << tr("Address")
        << true);

    QStack<BookmarkItem*> parents;
    QDataStream stream(bookmarks);

    qint32 version;
    stream >> version;
    if (version < VERSION) {
        stream.device()->seek(0);
        BookmarkItem* toolbar = new BookmarkItem(DataVector() << tr("Toolbar Menu")
            << QLatin1String("Folder") << true);
        rootItem->addChild(toolbar);

        BookmarkItem* menu = new BookmarkItem(DataVector() << tr("Bookmarks Menu")
            << QLatin1String("Folder") << true);
        rootItem->addChild(menu);
        parents.push(menu);
    } else {
        parents.push(rootItem);
    }

    qint32 depth;
    bool expanded;
    QString name, url;
    while (!stream.atEnd()) {
        stream >> depth >> name >> url >> expanded;
        while ((parents.count() - 1) != depth)
            parents.pop();

        BookmarkItem *item = new BookmarkItem(DataVector() << name << url << expanded);
        if (url == QLatin1String("Folder")) {
            parents.top()->addChild(item);
            parents.push(item);
        } else {
            parents.top()->addChild(item);
        }
    }

    cache.clear();
    setupCache(index(0,0, QModelIndex().parent()));
    endResetModel();
}

void
BookmarkModel::setItemsEditable(bool editable)
{
    m_editable = editable;
}

void
BookmarkModel::expandFoldersIfNeeeded(QTreeView *treeView)
{
    foreach (const QModelIndex &index, cache)
        treeView->setExpanded(index, index.data(UserRoleExpanded).toBool());
}

QModelIndex
BookmarkModel::addItem(const QModelIndex &parent, bool isFolder)
{
    m_folder = isFolder;
    QModelIndex next;
    if (insertRow(rowCount(parent), parent))
        next = index(rowCount(parent) - 1, 0, parent);
    m_folder = false;

    return next;
}

bool
BookmarkModel::removeItem(const QModelIndex &index)
{
    if (!index.isValid())
        return false;

    QModelIndexList indexes;
    if (rowCount(index) > 0)
        indexes = collectItems(index);
    indexes.append(index);

    foreach (const QModelIndex &itemToRemove, indexes) {
        if (!removeRow(itemToRemove.row(), itemToRemove.parent()))
            return false;
        cache.remove(itemFromIndex(itemToRemove));
    }
    return true;
}

int
BookmarkModel::rowCount(const QModelIndex &index) const
{
    if (BookmarkItem *item = itemFromIndex(index))
        return item->childCount();
    return 0;
}

int
BookmarkModel::columnCount(const QModelIndex &/*index*/) const
{
    return 2;
}

QModelIndex
BookmarkModel::parent(const QModelIndex &index) const
{
     if (!index.isValid())
         return QModelIndex();

     if (BookmarkItem *childItem = itemFromIndex(index)) {
         if (BookmarkItem *parent = childItem->parent()) {
             if (parent != rootItem)
                 return createIndex(parent->childNumber(), 0, parent);
         }
     }
     return QModelIndex();
}

QModelIndex
BookmarkModel::index(int row, int column, const QModelIndex &index) const
{
    if (index.isValid() && (index.column() != 0 && index.column() != 1))
         return QModelIndex();

    if (BookmarkItem *parent = itemFromIndex(index)) {
        if (BookmarkItem *childItem = parent->child(row))
            return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

Qt::DropActions
BookmarkModel::supportedDropActions () const
{
    return /* Qt::CopyAction | */Qt::MoveAction;
}

Qt::ItemFlags
BookmarkModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags defaultFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (m_editable)
        defaultFlags |= Qt::ItemIsEditable;

    if (itemFromIndex(index) && index.data(UserRoleFolder).toBool()) {
        if (index.column() > 0)
            return defaultFlags &~ Qt::ItemIsEditable;
        return defaultFlags | Qt::ItemIsDropEnabled;
    }

    return defaultFlags | Qt::ItemIsDragEnabled;
}

QVariant
BookmarkModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (BookmarkItem *item = itemFromIndex(index)) {
            switch (role) {
                case Qt::EditRole: {
                case Qt::DisplayRole:
                    if (index.data(UserRoleFolder).toBool() && index.column() == 1)
                        return QLatin1String("");
                    return item->data(index.column());
                }   break;

                case Qt::DecorationRole: {
                    if (index.column() == 0)
                        return index.data(UserRoleFolder).toBool()
                            ? folderIcon : bookmarkIcon;
                }   break;

                default:;
                    return item->data(role);
            }
        }
    }
    return QVariant();
}

void BookmarkModel::setData(const QModelIndex &index, const DataVector &data)
{
    if (BookmarkItem *item = itemFromIndex(index)) {
        item->setData(data);
        emit dataChanged(index, index);
    }
}

bool
BookmarkModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool result = false;
    if (role != Qt::EditRole && role != UserRoleExpanded)
        return result;

    if (BookmarkItem *item = itemFromIndex(index)) {
        if (role == Qt::EditRole) {
            const bool isFolder = index.data(UserRoleFolder).toBool();
            if (!isFolder || (isFolder && index.column() == 0))
                result = item->setData(index.column(), value);
        } else if (role == UserRoleExpanded) {
            result = item->setData(UserRoleExpanded, value);
        }
    }

    if (result)
        emit dataChanged(index, index);
    return result;
}

QVariant
BookmarkModel::headerData(int section, Qt::Orientation orientation,
    int role) const
{
     if (rootItem && orientation == Qt::Horizontal && role == Qt::DisplayRole)
         return rootItem->data(section);
     return QVariant();
}

QModelIndex
BookmarkModel::indexFromItem(BookmarkItem *item) const
{
    return cache.value(item, QModelIndex());
}

BookmarkItem*
BookmarkModel::itemFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
         return static_cast<BookmarkItem*>(index.internalPointer());
     return rootItem;
}

QList<QPersistentModelIndex>
BookmarkModel::indexListFor(const QString &label) const
{
    QList<QPersistentModelIndex> hits;
    const QModelIndexList &list = collectItems(QModelIndex());
    foreach(const QModelIndex &index, list) {
        if (index.data().toString().contains(label, Qt::CaseInsensitive))
            hits.prepend(index);    // list is reverse sorted
    }
    return hits;
}

bool
BookmarkModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    if (!parent.data(UserRoleFolder).toBool())
        return false;

    bool success = false;
    if (BookmarkItem *parentItem = itemFromIndex(parent)) {
        beginInsertRows(parent, position, position + rows - 1);
        success = parentItem->insertChildren(m_folder, position, rows);
        if (success) {
            const QModelIndex &current = index(position, 0, parent);
            cache.insert(itemFromIndex(current), current);
        }
        endInsertRows();
    }
    return success;
}

bool
BookmarkModel::removeRows(int position, int rows, const QModelIndex &index)
{
    bool success = false;
    if (BookmarkItem *parent = itemFromIndex(index)) {
        beginRemoveRows(index, position, position + rows - 1);
        success = parent->removeChildren(position, rows);
        endRemoveRows();
    }
    return success;
}

QStringList
BookmarkModel::mimeTypes() const
{
    return QStringList() << MIMETYPE;
}

QMimeData*
BookmarkModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty())
        return 0;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        if (index.column() == 0)
            collectItems(index, 0, &stream);
    }

    QMimeData *mimeData = new QMimeData();
    mimeData->setData(MIMETYPE, data);
    return mimeData;
}

bool
BookmarkModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
    int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat(MIMETYPE) || column > 0)
        return false;

    QByteArray ba = data->data(MIMETYPE);
    QDataStream stream(&ba, QIODevice::ReadOnly);
    while (stream.atEnd())
        return false;

    qint32 depth;
    bool expanded;
    QString name, url;
    while (!stream.atEnd()) {
        stream >> depth >> name >> url >> expanded;
        if (insertRow(qMax(0, row), parent)) {
            const QModelIndex &current = index(qMax(0, row), 0, parent);
            if (current.isValid()) {
                BookmarkItem* item = itemFromIndex(current);
                item->setData(DataVector() << name << url << expanded);
            }
        }
    }
    return true;
}

void
BookmarkModel::setupCache(const QModelIndex &parent)
{
    const QModelIndexList &list = collectItems(parent);
    foreach (const QModelIndex &index, list)
        cache.insert(itemFromIndex(index), index);
}

QModelIndexList
BookmarkModel::collectItems(const QModelIndex &parent) const
{
    QModelIndexList list;
    for (int i = rowCount(parent) - 1; i >= 0 ; --i) {
        const QModelIndex &next = index(i, 0, parent);
        if (data(next, UserRoleFolder).toBool())
            list += collectItems(next);
        list.append(next);
    }
    return list;
}

void
BookmarkModel::collectItems(const QModelIndex &parent, qint32 depth,
    QDataStream *stream) const
{
    if (parent.isValid()) {
        *stream << depth;
        *stream << parent.data().toString();
        *stream << parent.data(UserRoleUrl).toString();
        *stream << parent.data(UserRoleExpanded).toBool();

        for (int i = 0; i < rowCount(parent); ++i) {
            if (parent.data(UserRoleFolder).toBool())
                collectItems(index(i, 0 , parent), depth + 1, stream);
        }
    }
}
