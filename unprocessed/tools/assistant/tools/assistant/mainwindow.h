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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtGui/QMainWindow>

QT_BEGIN_NAMESPACE

class QAction;
class QComboBox;
class QFileSystemWatcher;
class QLineEdit;
class QMenu;

class CentralWidget;
class CmdLineParser;
class ContentWindow;
class IndexWindow;
class OpenPagesWindow;
class QtDocInstaller;
class QHelpEngineCore;
class QHelpEngine;
class SearchWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(CmdLineParser *cmdLine, QWidget *parent = 0);
    ~MainWindow();

    static void activateCurrentBrowser();
    static QString collectionFileDirectory(bool createDir = false,
        const QString &cacheDir = QString());
    static QString defaultHelpCollectionFileName();

public:
    void setIndexString(const QString &str);
    void expandTOC(int depth);
    bool usesDefaultCollection() const;

signals:
    void initDone();

public slots:
    void setContentsVisible(bool visible);
    void setIndexVisible(bool visible);
    void setBookmarksVisible(bool visible);
    void setSearchVisible(bool visible);
    void syncContents();
    void activateCurrentCentralWidgetTab();
    void currentFilterChanged(const QString &filter);

private slots:
    void showContents();
    void showIndex();
    void showSearch();
    void showOpenPages();
    void insertLastPages();
    void gotoAddress();
    void showPreferences();
    void showNewAddress();
    void showAboutDialog();
    void showNewAddress(const QUrl &url);
    void showTopicChooser(const QMap<QString, QUrl> &links, const QString &keyword);
    void updateApplicationFont();
    void filterDocumentation(const QString &customFilter);
    void setupFilterCombo();
    void lookForNewQtDocumentation();
    void indexingStarted();
    void indexingFinished();
    void qtDocumentationInstalled();
    void registerDocumentation(const QString &component,
        const QString &absFileName);
    void resetQtDocInfo(const QString &component);
    void checkInitState();
    void documentationRemoved(const QString &namespaceName);
    void documentationUpdated(const QString &namespaceName);

private:
    bool initHelpDB(bool registerInternalDoc);
    void setupActions();
    void closeEvent(QCloseEvent *e);
    void activateDockWidget(QWidget *w);
    void updateAboutMenuText();
    void setupFilterToolbar();
    void setupAddressToolbar();
    QMenu *toolBarMenu();
    void hideContents();
    void hideIndex();
    void hideSearch();

private slots:
    void showBookmarksDockWidget();
    void hideBookmarksDockWidget();

private:
    QWidget *m_bookmarkWidget;

private:
    CentralWidget *m_centralWidget;
    IndexWindow *m_indexWindow;
    ContentWindow *m_contentWindow;
    SearchWidget *m_searchWindow;
    QLineEdit *m_addressLineEdit;
    QComboBox *m_filterCombo;

    QAction *m_syncAction;
    QAction *m_printPreviewAction;
    QAction *m_pageSetupAction;
    QAction *m_resetZoomAction;
    QAction *m_aboutAction;
    QAction *m_closeTabAction;
    QAction *m_newTabAction;

    QMenu *m_viewMenu;
    QMenu *m_toolBarMenu;

    CmdLineParser *m_cmdLine;

    QWidget *m_progressWidget;
    QtDocInstaller *m_qtDocInstaller;

    bool m_connectedInitSignals;
};

QT_END_NAMESPACE

#endif // MAINWINDOW_H
