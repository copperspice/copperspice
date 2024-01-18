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

#ifndef QDECLARATIVEFOLDERLISTMODEL_H
#define QDECLARATIVEFOLDERLISTMODEL_H

#include <qdeclarative.h>
#include <QStringList>
#include <QUrl>
#include <QAbstractListModel>

#ifndef QT_NO_DIRMODEL

QT_BEGIN_NAMESPACE

class QDeclarativeContext;
class QModelIndex;
class QDeclarativeFolderListModelPrivate;

]
class QDeclarativeFolderListModel : public QAbstractListModel, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)

    Q_PROPERTY(QUrl folder READ folder WRITE setFolder NOTIFY folderChanged)
    Q_PROPERTY(QUrl parentFolder READ parentFolder NOTIFY folderChanged)
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters)
    Q_PROPERTY(SortField sortField READ sortField WRITE setSortField)
    Q_PROPERTY(bool sortReversed READ sortReversed WRITE setSortReversed)
    Q_PROPERTY(bool showDirs READ showDirs WRITE setShowDirs)
    Q_PROPERTY(bool showDotAndDotDot READ showDotAndDotDot WRITE setShowDotAndDotDot)
    Q_PROPERTY(bool showOnlyReadable READ showOnlyReadable WRITE setShowOnlyReadable)
    Q_PROPERTY(int count READ count)

public:
    QDeclarativeFolderListModel(QObject *parent = nullptr);
    ~QDeclarativeFolderListModel();

    enum Roles { FileNameRole = Qt::UserRole+1, FilePathRole = Qt::UserRole+2 };

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
//![abslistmodel]

//![count]
    int count() const { return rowCount(QModelIndex()); }
//![count]

//![prop funcs]
    QUrl folder() const;
    void setFolder(const QUrl &folder);

    QUrl parentFolder() const;

    QStringList nameFilters() const;
    void setNameFilters(const QStringList &filters);

    enum SortField { Unsorted, Name, Time, Size, Type };
    SortField sortField() const;
    void setSortField(SortField field);
    Q_ENUMS(SortField)

    bool sortReversed() const;
    void setSortReversed(bool rev);

    bool showDirs() const;
    void  setShowDirs(bool);
    bool showDotAndDotDot() const;
    void  setShowDotAndDotDot(bool);
    bool showOnlyReadable() const;
    void  setShowOnlyReadable(bool);
//![prop funcs]

//![isfolder]
    Q_INVOKABLE bool isFolder(int index) const;
//![isfolder]

//![parserstatus]
    virtual void classBegin();
    virtual void componentComplete();
//![parserstatus]

//![notifier]
Q_SIGNALS:
    void folderChanged();
//![notifier]

//![class end]
private Q_SLOTS:
    void refresh();
    void inserted(const QModelIndex &index, int start, int end);
    void removed(const QModelIndex &index, int start, int end);
    void handleDataChanged(const QModelIndex &start, const QModelIndex &end);

private:
    Q_DISABLE_COPY(QDeclarativeFolderListModel)
    QDeclarativeFolderListModelPrivate *d;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeFolderListModel)

#endif // QT_NO_DIRMODEL

#endif // QDECLARATIVEFOLDERLISTMODEL_H
