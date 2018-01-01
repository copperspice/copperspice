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

#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include <QtCore/QMutex>
#include <QtGui/QTreeView>

#include "ui_bookmarkwidget.h"

QT_BEGIN_NAMESPACE

class BookmarkManagerWidget;
class BookmarkModel;
class BookmarkFilterModel;
class QKeyEvent;
class QSortFilterProxyModel;
class QToolBar;

class BookmarkManager : public QObject
{
    Q_OBJECT
    class BookmarkWidget;
    class BookmarkTreeView;
    class BookmarkListView;
    Q_DISABLE_COPY(BookmarkManager);

public:
    static BookmarkManager* instance();
    static void destroy();

    QWidget* bookmarkDockWidget() const;
    void setBookmarksMenu(QMenu* menu);
    void setBookmarksToolbar(QToolBar *toolBar);

public slots:
    void addBookmark(const QString &title, const QString &url);

signals:
    void escapePressed();
    void setSource(const QUrl &url);
    void setSourceInNewTab(const QUrl &url);

private:
    BookmarkManager();
    ~BookmarkManager();

    void removeItem(const QModelIndex &index);
    bool eventFilter(QObject *object, QEvent *event);
    void buildBookmarksMenu(const QModelIndex &index, QMenu *menu);
    void showBookmarkDialog(const QString &name, const QString &url);

private slots:
    void setupFinished();

    void addBookmark();
    void removeBookmark();
    void manageBookmarks();
    void refreshBookmarkMenu();
    void refreshBookmarkToolBar();
    void renameBookmark(const QModelIndex &index);

    void setSourceFromAction();
    void setSourceFromAction(QAction *action);
    void setSourceFromIndex(const QModelIndex &index, bool newTab = false);

    void focusInEvent();
    void managerWidgetAboutToClose();
    void textChanged(const QString &text);
    void customContextMenuRequested(const QPoint &point);

private:
    bool typeAndSearch;

    static QMutex mutex;
    static BookmarkManager *bookmarkManager;

    QMenu *bookmarkMenu;
    QToolBar *m_toolBar;

    BookmarkModel *bookmarkModel;
    BookmarkFilterModel *bookmarkFilterModel;
    QSortFilterProxyModel *typeAndSearchModel;

    BookmarkWidget *bookmarkWidget;
    BookmarkTreeView *bookmarkTreeView;
    BookmarkManagerWidget *bookmarkManagerWidget;
};

class BookmarkManager::BookmarkWidget : public QWidget
{
    Q_OBJECT
public:
    BookmarkWidget(QWidget *parent = 0)
        : QWidget(parent) { ui.setupUi(this); }
    virtual ~BookmarkWidget() {}

    Ui::BookmarkWidget ui;

signals:
    void focusInEvent();

private:
    void focusInEvent(QFocusEvent *event);
};

class BookmarkManager::BookmarkTreeView : public QTreeView
{
    Q_OBJECT
public:
    BookmarkTreeView(QWidget *parent = 0);
    ~BookmarkTreeView() {}

    void subclassKeyPressEvent(QKeyEvent *event);

private slots:
    void setExpandedData(const QModelIndex &index);
};

QT_END_NAMESPACE

#endif  // BOOKMARKMANAGER_H
