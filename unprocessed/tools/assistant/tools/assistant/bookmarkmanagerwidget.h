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

#ifndef BOOKMARKMANAGERWIDGET_H
#define BOOKMARKMANAGERWIDGET_H

#include "ui_bookmarkmanagerwidget.h"

#include <QtCore/QPersistentModelIndex>

#include <QtGui/QMenu>

QT_BEGIN_NAMESPACE

class BookmarkModel;
class QCloseEvent;
class QString;

class BookmarkManagerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BookmarkManagerWidget(BookmarkModel *bookmarkModel,
        QWidget *parent = 0);
    ~BookmarkManagerWidget();

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void setSource(const QUrl &url);
    void setSourceInNewTab(const QUrl &url);

    void managerWidgetAboutToClose();

private:
    void renameItem(const QModelIndex &index);
    void selectNextIndex(bool direction) const;
    bool eventFilter(QObject *object, QEvent *event);

private slots:
    void findNext();
    void findPrevious();

    void importBookmarks();
    void exportBookmarks();

    void refeshBookmarkCache();
    void textChanged(const QString &text);

    void removeItem(const QModelIndex &index = QModelIndex());

    void customContextMenuRequested(const QPoint &point);
    void setSourceFromIndex(const QModelIndex &index, bool newTab = false);

private:
    QMenu importExportMenu;
    Ui::BookmarkManagerWidget ui;
    QList<QPersistentModelIndex> cache;

    BookmarkModel *bookmarkModel;
};

QT_END_NAMESPACE

#endif  // BOOKMARKMANAGERWIDGET_H
