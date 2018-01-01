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

#ifndef BOOKMARKITEM_H
#define BOOKMARKITEM_H

#include <QtCore/QVariant>
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

enum {
    UserRoleUrl = Qt::UserRole + 50,
    UserRoleFolder = Qt::UserRole + 100,
    UserRoleExpanded = Qt::UserRole + 150
};

typedef QVector<QVariant> DataVector;

class BookmarkItem
{
public:
    explicit BookmarkItem(const DataVector &data, BookmarkItem *parent = 0);
    ~BookmarkItem();

    BookmarkItem *parent() const;
    void setParent(BookmarkItem *parent);

    void addChild(BookmarkItem *child);
    BookmarkItem *child(int number) const;

    int childCount() const;
    int childNumber() const;

    QVariant data(int column) const;
    void setData(const DataVector &data);
    bool setData(int column, const QVariant &value);

    bool insertChildren(bool isFolder, int position, int count);
    bool removeChildren(int position, int count);

    void dumpTree(int indent) const;

private:
    DataVector m_data;

    BookmarkItem *m_parent;
    QList<BookmarkItem*> m_children;
};

QT_END_NAMESPACE

#endif // BOOKMARKITEM_H
