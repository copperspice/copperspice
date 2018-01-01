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

#ifndef OPENPAGESMANAGER_H
#define OPENPAGESMANAGER_H

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QAbstractItemView;
class QModelIndex;
class QUrl;

class HelpViewer;
class OpenPagesModel;
class OpenPagesSwitcher;
class OpenPagesWidget;

class OpenPagesManager : public QObject
{
    Q_OBJECT
public:
    static OpenPagesManager *createInstance(QObject *parent,
        bool defaultCollection, const QUrl &cmdLineUrl);
    static OpenPagesManager *instance();

    bool pagesOpenForNamespace(const QString &nameSpace) const;
    void closePages(const QString &nameSpace);
    void reloadPages(const QString &nameSpace);

    QAbstractItemView* openPagesWidget() const;

    int pageCount() const;
    void setCurrentPage(int index);

public slots:
    HelpViewer *createPage(const QUrl &url, bool fromSearch = false);
    HelpViewer *createNewPageFromSearch(const QUrl &url);
    HelpViewer *createPage();
    void closeCurrentPage();

    void nextPage();
    void nextPageWithSwitcher();
    void previousPage();
    void previousPageWithSwitcher();

    void closePage(HelpViewer *page);
    void setCurrentPage(HelpViewer *page);

private slots:
    void setCurrentPage(const QModelIndex &index);
    void closePage(const QModelIndex &index);
    void closePagesExcept(const QModelIndex &index);

private:
    OpenPagesManager(QObject *parent, bool defaultCollection,
        const QUrl &cmdLineUrl);
    ~OpenPagesManager();

    void setupInitialPages(bool defaultCollection, const QUrl &cmdLineUrl);
    void closeOrReloadPages(const QString &nameSpace, bool tryReload);
    void removePage(int index);

    void nextOrPreviousPage(int offset);
    void showSwitcherOrSelectPage() const;

    OpenPagesModel *m_model;
    OpenPagesWidget *m_openPagesWidget;
    OpenPagesSwitcher *m_openPagesSwitcher;

    static OpenPagesManager *m_instance;
};

QT_END_NAMESPACE

#endif // OPENPAGESMANAGER_H
