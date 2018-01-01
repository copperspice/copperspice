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

#ifndef BOOKMARKDIALOG_H
#define BOOKMARKDIALOG_H

#include "ui_bookmarkdialog.h"

QT_BEGIN_NAMESPACE

class BookmarkModel;
class BookmarkFilterModel;
class BookmarkTreeModel;

class BookmarkDialog : public QDialog
{
    Q_OBJECT
public:
    BookmarkDialog(BookmarkModel *bookmarkModel, const QString &title,
        const QString &url, QWidget *parent = 0);
    ~BookmarkDialog();

private:
    bool isRootItem(const QModelIndex &index) const;
    bool eventFilter(QObject *object, QEvent *event);

private slots:
    void currentIndexChanged(int index);
    void currentIndexChanged(const QModelIndex &index);

    void accepted();
    void rejected();

    void addFolder();
    void toolButtonClicked();
    void textChanged(const QString& text);
    void customContextMenuRequested(const QPoint &point);

private:
    QString m_url;
    QString m_title;
    Ui::BookmarkDialog ui;
    QList<QPersistentModelIndex> cache;

    BookmarkModel *bookmarkModel;
    BookmarkTreeModel *bookmarkTreeModel;
    BookmarkFilterModel *bookmarkProxyModel;
};

QT_END_NAMESPACE

#endif  // BOOKMARKDIALOG_H
