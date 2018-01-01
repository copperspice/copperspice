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

#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QtCore/QUrl>

#include <QtGui/QTabBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class FindWidget;
class HelpViewer;
class QStackedWidget;

class TabBar : public QTabBar
{
    Q_OBJECT
public:
    TabBar(QWidget *parent = 0);
    ~TabBar();

    int addNewTab(const QString &title);
    void setCurrent(HelpViewer *viewer);
    void removeTabAt(HelpViewer *viewer);

public slots:
    void titleChanged();

signals:
    void currentTabChanged(HelpViewer *viewer);
    void addBookmark(const QString &title, const QString &url);

private slots:
    void slotCurrentChanged(int index);
    void slotTabCloseRequested(int index);
    void slotCustomContextMenuRequested(const QPoint &pos);
};

class CentralWidget : public QWidget
{
    Q_OBJECT

public:
    CentralWidget(QWidget *parent = 0);
    ~CentralWidget();

    static CentralWidget *instance();

    QUrl currentSource() const;
    QString currentTitle() const;

    bool hasSelection() const;
    bool isForwardAvailable() const;
    bool isBackwardAvailable() const;

    HelpViewer *viewerAt(int index) const;
    HelpViewer *currentHelpViewer() const;

    void addPage(HelpViewer *page, bool fromSearch = false);
    void removePage(int index);

    int currentIndex() const;
    void setCurrentPage(HelpViewer *page);

    void connectTabBar();

public slots:
    void copy();
    void home();

    void zoomIn();
    void zoomOut();
    void resetZoom();

    void forward();
    void nextPage();

    void backward();
    void previousPage();

    void print();
    void pageSetup();
    void printPreview();

    void setSource(const QUrl &url);
    void setSourceFromSearch(const QUrl &url);

    void findNext();
    void findPrevious();
    void find(const QString &text, bool forward, bool incremental);

    void activateTab();
    void showTextSearch();
    void updateBrowserFont();
    void updateUserInterface();

signals:
    void currentViewerChanged();
    void copyAvailable(bool yes);
    void sourceChanged(const QUrl &url);
    void highlighted(const QString &link);
    void forwardAvailable(bool available);
    void backwardAvailable(bool available);
    void addBookmark(const QString &title, const QString &url);

protected:
    void keyPressEvent(QKeyEvent *);
    void focusInEvent(QFocusEvent *event);

private slots:
    void highlightSearchTerms();
    void printPreview(QPrinter *printer);
    void handleSourceChanged(const QUrl &url);

private:
    void initPrinter();
    void connectSignals(HelpViewer *page);
    bool eventFilter(QObject *object, QEvent *e);

private:
#ifndef QT_NO_PRINTER
    QPrinter *m_printer;
#endif
    FindWidget *m_findWidget;
    QStackedWidget *m_stackedWidget;
    TabBar *m_tabBar;
};

QT_END_NAMESPACE

#endif  // CENTRALWIDGET_H
