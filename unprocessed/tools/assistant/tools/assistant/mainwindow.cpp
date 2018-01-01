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

#include "mainwindow.h"

#include "aboutdialog.h"
#include "bookmarkmanager.h"
#include "centralwidget.h"
#include "cmdlineparser.h"
#include "contentwindow.h"
#include "globalactions.h"
#include "helpenginewrapper.h"
#include "indexwindow.h"
#include "openpagesmanager.h"
#include "preferencesdialog.h"
#include "qtdocinstaller.h"
#include "remotecontrol.h"
#include "searchwidget.h"
#include "topicchooser.h"
#include "tracer.h"

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QPair>
#include <QtCore/QResource>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>

#include <QtGui/QAction>
#include <QtGui/QComboBox>
#include <QtGui/QDesktopServices>
#include <QtGui/QDesktopWidget>
#include <QtGui/QDockWidget>
#include <QtGui/QFontDatabase>
#include <QtGui/QFileDialog>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressBar>
#include <QtGui/QShortcut>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>

#include <QtHelp/QHelpContentModel>
#include <QtHelp/QHelpEngineCore>
#include <QtHelp/QHelpIndexModel>
#include <QtHelp/QHelpSearchEngine>

#include <cstdlib>

QT_BEGIN_NAMESPACE

MainWindow::MainWindow(CmdLineParser *cmdLine, QWidget *parent)
    : QMainWindow(parent)
    , m_bookmarkWidget(0)
    , m_filterCombo(0)
    , m_toolBarMenu(0)
    , m_cmdLine(cmdLine)
    , m_progressWidget(0)
    , m_qtDocInstaller(0)
    , m_connectedInitSignals(false)
{
    TRACE_OBJ

    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setDockOptions(dockOptions() | AllowNestedDocks);

    QString collectionFile;
    if (usesDefaultCollection()) {
        MainWindow::collectionFileDirectory(true);
        collectionFile = MainWindow::defaultHelpCollectionFileName();
    } else {
        collectionFile = cmdLine->collectionFile();
    }
    HelpEngineWrapper &helpEngineWrapper =
        HelpEngineWrapper::instance(collectionFile);
    BookmarkManager *bookMarkManager = BookmarkManager::instance();

    if (!initHelpDB(!cmdLine->collectionFileGiven())) {
        qDebug("Fatal error: Help engine initialization failed. "
            "Error message was: %s\nAssistant will now exit.",
            qPrintable(HelpEngineWrapper::instance().error()));
        std::exit(1);
    }

    m_centralWidget = new CentralWidget(this);
    setCentralWidget(m_centralWidget);

    m_indexWindow = new IndexWindow(this);
    QDockWidget *indexDock = new QDockWidget(tr("Index"), this);
    indexDock->setObjectName(QLatin1String("IndexWindow"));
    indexDock->setWidget(m_indexWindow);
    addDockWidget(Qt::LeftDockWidgetArea, indexDock);

    m_contentWindow = new ContentWindow;
    QDockWidget *contentDock = new QDockWidget(tr("Contents"), this);
    contentDock->setObjectName(QLatin1String("ContentWindow"));
    contentDock->setWidget(m_contentWindow);
    addDockWidget(Qt::LeftDockWidgetArea, contentDock);

    m_searchWindow = new SearchWidget(helpEngineWrapper.searchEngine());
    m_searchWindow->setFont(!helpEngineWrapper.usesBrowserFont() ? qApp->font()
        : helpEngineWrapper.browserFont());
    QDockWidget *searchDock = new QDockWidget(tr("Search"), this);
    searchDock->setObjectName(QLatin1String("SearchWindow"));
    searchDock->setWidget(m_searchWindow);
    addDockWidget(Qt::LeftDockWidgetArea, searchDock);

    QDockWidget *bookmarkDock = new QDockWidget(tr("Bookmarks"), this);
    bookmarkDock->setObjectName(QLatin1String("BookmarkWindow"));
    bookmarkDock->setWidget(m_bookmarkWidget
        = bookMarkManager->bookmarkDockWidget());
    addDockWidget(Qt::LeftDockWidgetArea, bookmarkDock);

    QDockWidget *openPagesDock = new QDockWidget(tr("Open Pages"), this);
    openPagesDock->setObjectName(QLatin1String("Open Pages"));
    OpenPagesManager *openPagesManager
        = OpenPagesManager::createInstance(this, usesDefaultCollection(), m_cmdLine->url());
    openPagesDock->setWidget(openPagesManager->openPagesWidget());
    addDockWidget(Qt::LeftDockWidgetArea, openPagesDock);

    connect(m_centralWidget, SIGNAL(addBookmark(QString, QString)),
        bookMarkManager, SLOT(addBookmark(QString, QString)));
    connect(bookMarkManager, SIGNAL(escapePressed()), this,
            SLOT(activateCurrentCentralWidgetTab()));
    connect(bookMarkManager, SIGNAL(setSource(QUrl)), m_centralWidget,
            SLOT(setSource(QUrl)));
    connect(bookMarkManager, SIGNAL(setSourceInNewTab(QUrl)),
        openPagesManager, SLOT(createPage(QUrl)));

    QHelpSearchEngine *searchEngine = helpEngineWrapper.searchEngine();
    connect(searchEngine, SIGNAL(indexingStarted()), this, SLOT(indexingStarted()));
    connect(searchEngine, SIGNAL(indexingFinished()), this, SLOT(indexingFinished()));

    QString defWindowTitle = tr("Qt Assistant");
    setWindowTitle(defWindowTitle);

    setupActions();
    statusBar()->show();
    m_centralWidget->connectTabBar();

    setupFilterToolbar();
    setupAddressToolbar();

    const QString windowTitle = helpEngineWrapper.windowTitle();
    setWindowTitle(windowTitle.isEmpty() ? defWindowTitle : windowTitle);
    QByteArray iconArray = helpEngineWrapper.applicationIcon();
    if (iconArray.size() > 0) {
        QPixmap pix;
        pix.loadFromData(iconArray);
        QIcon appIcon(pix);
        qApp->setWindowIcon(appIcon);
    } else {
        QIcon appIcon(QLatin1String(":/trolltech/assistant/images/assistant-128.png"));
        qApp->setWindowIcon(appIcon);
    }

    QToolBar *toolBar = addToolBar(tr("Bookmark Toolbar"));
    toolBar->setObjectName(QLatin1String("Bookmark Toolbar"));
    bookMarkManager->setBookmarksToolbar(toolBar);

    // Show the widget here, otherwise the restore geometry and state won't work
    // on x11.
    show();

    toolBar->hide();
    toolBarMenu()->addAction(toolBar->toggleViewAction());

    QByteArray ba(helpEngineWrapper.mainWindow());
    if (!ba.isEmpty())
        restoreState(ba);

    ba = helpEngineWrapper.mainWindowGeometry();
    if (!ba.isEmpty()) {
        restoreGeometry(ba);
    } else {
        tabifyDockWidget(contentDock, indexDock);
        tabifyDockWidget(indexDock, bookmarkDock);
        tabifyDockWidget(bookmarkDock, searchDock);
        contentDock->raise();
        const QRect screen = QApplication::desktop()->screenGeometry();
        resize(4*screen.width()/5, 4*screen.height()/5);
    }

    if (!helpEngineWrapper.hasFontSettings()) {
        helpEngineWrapper.setUseAppFont(false);
        helpEngineWrapper.setUseBrowserFont(false);
        helpEngineWrapper.setAppFont(qApp->font());
        helpEngineWrapper.setAppWritingSystem(QFontDatabase::Latin);
        helpEngineWrapper.setBrowserFont(qApp->font());
        helpEngineWrapper.setBrowserWritingSystem(QFontDatabase::Latin);
    } else {
        updateApplicationFont();
    }

    updateAboutMenuText();

    QTimer::singleShot(0, this, SLOT(insertLastPages()));
    if (m_cmdLine->enableRemoteControl())
        (void)new RemoteControl(this);

    if (m_cmdLine->contents() == CmdLineParser::Show)
        showContents();
    else if (m_cmdLine->contents() == CmdLineParser::Hide)
        hideContents();

    if (m_cmdLine->index() == CmdLineParser::Show)
        showIndex();
    else if (m_cmdLine->index() == CmdLineParser::Hide)
        hideIndex();

    if (m_cmdLine->bookmarks() == CmdLineParser::Show)
        showBookmarksDockWidget();
    else if (m_cmdLine->bookmarks() == CmdLineParser::Hide)
        hideBookmarksDockWidget();

    if (m_cmdLine->search() == CmdLineParser::Show)
        showSearch();
    else if (m_cmdLine->search() == CmdLineParser::Hide)
        hideSearch();

    if (m_cmdLine->contents() == CmdLineParser::Activate)
        showContents();
    else if (m_cmdLine->index() == CmdLineParser::Activate)
        showIndex();
    else if (m_cmdLine->bookmarks() == CmdLineParser::Activate)
        showBookmarksDockWidget();

    if (!m_cmdLine->currentFilter().isEmpty()) {
        const QString &curFilter = m_cmdLine->currentFilter();
        if (helpEngineWrapper.customFilters().contains(curFilter))
            helpEngineWrapper.setCurrentFilter(curFilter);
    }

    if (usesDefaultCollection())
        QTimer::singleShot(0, this, SLOT(lookForNewQtDocumentation()));
    else
        checkInitState();

    connect(&helpEngineWrapper, SIGNAL(documentationRemoved(QString)),
            this, SLOT(documentationRemoved(QString)));
    connect(&helpEngineWrapper, SIGNAL(documentationUpdated(QString)),
            this, SLOT(documentationUpdated(QString)));

    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    GlobalActions::instance()->updateActions();
    if (helpEngineWrapper.addressBarEnabled())
        showNewAddress();
}

MainWindow::~MainWindow()
{
    TRACE_OBJ
    if (m_qtDocInstaller)
        delete m_qtDocInstaller;
}

bool MainWindow::usesDefaultCollection() const
{
    TRACE_OBJ
    return m_cmdLine->collectionFile().isEmpty();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    TRACE_OBJ
    BookmarkManager::destroy();
    HelpEngineWrapper::instance().setMainWindow(saveState());
    HelpEngineWrapper::instance().setMainWindowGeometry(saveGeometry());
    QMainWindow::closeEvent(e);
}

bool MainWindow::initHelpDB(bool registerInternalDoc)
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngineWrapper = HelpEngineWrapper::instance();
    if (!helpEngineWrapper.setupData())
        return false;

    if (!registerInternalDoc) {
        if (helpEngineWrapper.defaultHomePage() == QLatin1String("help"))
            helpEngineWrapper.setDefaultHomePage(QLatin1String("about:blank"));
        return true;
    }
    bool assistantInternalDocRegistered = false;
    QString intern(QLatin1String("com.trolltech.com.assistantinternal-"));
    foreach (const QString &ns, helpEngineWrapper.registeredDocumentations()) {
        if (ns.startsWith(intern)) {
            intern = ns;
            assistantInternalDocRegistered = true;
            break;
        }
    }

    const QString &collectionFile = helpEngineWrapper.collectionFile();
    QFileInfo fi(collectionFile);
    QString helpFile;
    QTextStream(&helpFile) << fi.absolutePath() << QDir::separator()
        << QLatin1String("assistant.qch.") << (QT_VERSION >> 16)
        << QLatin1Char('.') << ((QT_VERSION >> 8) & 0xFF);

    bool needsSetup = false;
    if (!assistantInternalDocRegistered || !QFile::exists(helpFile)) {
        QFile file(helpFile);
        if (file.open(QIODevice::WriteOnly)) {
            QResource res(QLatin1String(":/trolltech/assistant/assistant.qch"));
            if (file.write((const char*)res.data(), res.size()) != res.size())
                qDebug() << QLatin1String("could not write assistant.qch...");

            file.close();
        }
        helpEngineWrapper.unregisterDocumentation(intern);
        helpEngineWrapper.registerDocumentation(helpFile);
        needsSetup = true;
    }

    if (needsSetup)
        helpEngineWrapper.setupData();
    return true;
}

void MainWindow::lookForNewQtDocumentation()
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    QStringList docs;
    docs << QLatin1String("assistant")
        << QLatin1String("designer")
        << QLatin1String("linguist")
        << QLatin1String("qmake")
        << QLatin1String("qt");
    QList<QtDocInstaller::DocInfo> qtDocInfos;
    foreach (const QString &doc, docs)
        qtDocInfos.append(QtDocInstaller::DocInfo(doc, helpEngine.qtDocInfo(doc)));

    m_qtDocInstaller = new QtDocInstaller(qtDocInfos);
    connect(m_qtDocInstaller, SIGNAL(docsInstalled(bool)), this,
        SLOT(qtDocumentationInstalled()));
    connect(m_qtDocInstaller, SIGNAL(qchFileNotFound(QString)), this,
            SLOT(resetQtDocInfo(QString)));
    connect(m_qtDocInstaller, SIGNAL(registerDocumentation(QString, QString)),
            this, SLOT(registerDocumentation(QString, QString)));
    if (helpEngine.qtDocInfo(QLatin1String("qt")).count() != 2)
        statusBar()->showMessage(tr("Looking for Qt Documentation..."));
    m_qtDocInstaller->installDocs();
}

void MainWindow::qtDocumentationInstalled()
{
    TRACE_OBJ
    statusBar()->clearMessage();
    checkInitState();
}

void MainWindow::checkInitState()
{
    TRACE_OBJ
    if (!m_cmdLine->enableRemoteControl()) {
        HelpEngineWrapper::instance().initialDocSetupDone();
        return;
    }

    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    if (helpEngine.contentModel()->isCreatingContents()
        || helpEngine.indexModel()->isCreatingIndex()) {
        if (!m_connectedInitSignals) {
            connect(helpEngine.contentModel(), SIGNAL(contentsCreated()),
                this, SLOT(checkInitState()));
            connect(helpEngine.indexModel(), SIGNAL(indexCreated()), this,
                SLOT(checkInitState()));
            m_connectedInitSignals = true;
        }
    } else {
        if (m_connectedInitSignals) {
            disconnect(helpEngine.contentModel(), 0, this, 0);
            disconnect(helpEngine.indexModel(), 0, this, 0);
        }
        HelpEngineWrapper::instance().initialDocSetupDone();
        emit initDone();
    }
}

void MainWindow::insertLastPages()
{
    TRACE_OBJ
    if (m_cmdLine->search() == CmdLineParser::Activate)
        showSearch();
}

void MainWindow::setupActions()
{
    TRACE_OBJ
    QString resourcePath = QLatin1String(":/trolltech/assistant/images/");
#ifdef Q_OS_MAC
    setUnifiedTitleAndToolBarOnMac(true);
    resourcePath.append(QLatin1String("mac"));
#else
    resourcePath.append(QLatin1String("win"));
#endif

    QMenu *menu = menuBar()->addMenu(tr("&File"));

    OpenPagesManager * const openPages = OpenPagesManager::instance();
    m_newTabAction
        = menu->addAction(tr("New &Tab"), openPages, SLOT(createPage()));
    m_newTabAction->setShortcut(QKeySequence::AddTab);
    m_closeTabAction = menu->addAction(tr("&Close Tab"),
                                          openPages, SLOT(closeCurrentPage()));
    m_closeTabAction->setShortcuts(QKeySequence::Close);

    menu->addSeparator();

    m_pageSetupAction = menu->addAction(tr("Page Set&up..."), m_centralWidget,
        SLOT(pageSetup()));
    m_printPreviewAction = menu->addAction(tr("Print Preview..."), m_centralWidget,
        SLOT(printPreview()));

    GlobalActions *globalActions = GlobalActions::instance(this);
    menu->addAction(globalActions->printAction());
    menu->addSeparator();

    QIcon appExitIcon = QIcon::fromTheme("application-exit");
    QAction *tmp;
#ifdef Q_OS_WIN
    tmp = menu->addAction(appExitIcon, tr("E&xit"), this, SLOT(close()));
    tmp->setShortcut(QKeySequence(tr("CTRL+Q")));
#else
    tmp = menu->addAction(appExitIcon, tr("&Quit"), this, SLOT(close()));
    tmp->setShortcut(QKeySequence::Quit);
#endif
    tmp->setMenuRole(QAction::QuitRole);

    menu = menuBar()->addMenu(tr("&Edit"));
    menu->addAction(globalActions->copyAction());
    menu->addAction(globalActions->findAction());

    QAction *findNextAction = menu->addAction(tr("Find &Next"), m_centralWidget,
        SLOT(findNext()));
    findNextAction->setShortcuts(QKeySequence::FindNext);

    QAction *findPreviousAction = menu->addAction(tr("Find &Previous"),
        m_centralWidget, SLOT(findPrevious()));
    findPreviousAction->setShortcuts(QKeySequence::FindPrevious);

    menu->addSeparator();
    tmp = menu->addAction(tr("Preferences..."), this, SLOT(showPreferences()));
    tmp->setMenuRole(QAction::PreferencesRole);

    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addAction(globalActions->zoomInAction());
    m_viewMenu->addAction(globalActions->zoomOutAction());

    m_resetZoomAction = m_viewMenu->addAction(tr("Normal &Size"), m_centralWidget,
        SLOT(resetZoom()));
    m_resetZoomAction->setPriority(QAction::LowPriority);
    m_resetZoomAction->setIcon(QIcon(resourcePath + QLatin1String("/resetzoom.png")));
    m_resetZoomAction->setShortcut(tr("Ctrl+0"));

    m_viewMenu->addSeparator();

    m_viewMenu->addAction(tr("Contents"), this, SLOT(showContents()),
        QKeySequence(tr("ALT+C")));
    m_viewMenu->addAction(tr("Index"), this, SLOT(showIndex()),
        QKeySequence(tr("ALT+I")));
    m_viewMenu->addAction(tr("Bookmarks"), this, SLOT(showBookmarksDockWidget()),
        QKeySequence(tr("ALT+O")));
    m_viewMenu->addAction(tr("Search"), this, SLOT(showSearch()),
        QKeySequence(tr("ALT+S")));
    m_viewMenu->addAction(tr("Open Pages"), this, SLOT(showOpenPages()),
        QKeySequence(tr("ALT+P")));

    menu = menuBar()->addMenu(tr("&Go"));
    menu->addAction(globalActions->homeAction());
    menu->addAction(globalActions->backAction());
    menu->addAction(globalActions->nextAction());

    m_syncAction = menu->addAction(tr("Sync with Table of Contents"), this,
        SLOT(syncContents()));
    m_syncAction->setIconText(tr("Sync"));
    m_syncAction->setIcon(QIcon(resourcePath + QLatin1String("/synctoc.png")));

    menu->addSeparator();

    tmp = menu->addAction(tr("Next Page"), openPages, SLOT(nextPage()));
    tmp->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Ctrl+Alt+Right"))
        << QKeySequence(Qt::CTRL + Qt::Key_PageDown));

    tmp = menu->addAction(tr("Previous Page"), openPages, SLOT(previousPage()));
    tmp->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Ctrl+Alt+Left"))
        << QKeySequence(Qt::CTRL + Qt::Key_PageUp));

#ifdef Q_WS_MAC
    QShortcut *sct = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Tab), this);
    connect(sct, SIGNAL(activated()), openPages, SLOT(nextPageWithSwitcher()));
    sct = new QShortcut(QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Tab), this);
    connect(sct, SIGNAL(activated()), openPages, SLOT(previousPageWithSwitcher()));
#else
    QShortcut *sct = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Tab), this);
    connect(sct, SIGNAL(activated()), openPages, SLOT(nextPageWithSwitcher()));
    sct = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab), this);
    connect(sct, SIGNAL(activated()), openPages, SLOT(previousPageWithSwitcher()));
#endif

    BookmarkManager::instance()->setBookmarksMenu(menuBar()->addMenu(tr("&Bookmarks")));

    menu = menuBar()->addMenu(tr("&Help"));
    m_aboutAction = menu->addAction(tr("About..."), this, SLOT(showAboutDialog()));
    m_aboutAction->setMenuRole(QAction::AboutRole);

#ifdef Q_WS_X11
    m_resetZoomAction->setIcon(QIcon::fromTheme("zoom-original" , m_resetZoomAction->icon()));
    m_syncAction->setIcon(QIcon::fromTheme("view-refresh" , m_syncAction->icon()));
#endif

    QToolBar *navigationBar = addToolBar(tr("Navigation Toolbar"));
    navigationBar->setObjectName(QLatin1String("NavigationToolBar"));
    navigationBar->addAction(globalActions->backAction());
    navigationBar->addAction(globalActions->nextAction());
    navigationBar->addAction(globalActions->homeAction());
    navigationBar->addAction(m_syncAction);
    navigationBar->addSeparator();
    navigationBar->addAction(globalActions->copyAction());
    navigationBar->addAction(globalActions->printAction());
    navigationBar->addAction(globalActions->findAction());
    navigationBar->addSeparator();
    navigationBar->addAction(globalActions->zoomInAction());
    navigationBar->addAction(globalActions->zoomOutAction());
    navigationBar->addAction(m_resetZoomAction);

#if defined(Q_WS_MAC)
    QMenu *windowMenu = new QMenu(tr("&Window"), this);
    menuBar()->insertMenu(menu->menuAction(), windowMenu);
    windowMenu->addAction(tr("Zoom"), this, SLOT(showMaximized()));
    windowMenu->addAction(tr("Minimize"), this, SLOT(showMinimized()),
        QKeySequence(tr("Ctrl+M")));
#endif

    // content viewer connections
    connect(m_centralWidget, SIGNAL(copyAvailable(bool)), globalActions,
        SLOT(setCopyAvailable(bool)));
    connect(m_centralWidget, SIGNAL(currentViewerChanged()), globalActions,
        SLOT(updateActions()));
    connect(m_centralWidget, SIGNAL(forwardAvailable(bool)), globalActions,
        SLOT(updateActions()));
    connect(m_centralWidget, SIGNAL(backwardAvailable(bool)), globalActions,
        SLOT(updateActions()));
    connect(m_centralWidget, SIGNAL(highlighted(QString)), statusBar(),
        SLOT(showMessage(QString)));

    // index window
    connect(m_indexWindow, SIGNAL(linkActivated(QUrl)), m_centralWidget,
        SLOT(setSource(QUrl)));
    connect(m_indexWindow, SIGNAL(linksActivated(QMap<QString,QUrl>,QString)),
        this, SLOT(showTopicChooser(QMap<QString,QUrl>,QString)));
    connect(m_indexWindow, SIGNAL(escapePressed()), this,
        SLOT(activateCurrentCentralWidgetTab()));

    // content window
    connect(m_contentWindow, SIGNAL(linkActivated(QUrl)), m_centralWidget,
        SLOT(setSource(QUrl)));
    connect(m_contentWindow, SIGNAL(escapePressed()), this,
        SLOT(activateCurrentCentralWidgetTab()));

    // search window
    connect(m_searchWindow, SIGNAL(requestShowLink(QUrl)),
        CentralWidget::instance(), SLOT(setSourceFromSearch(QUrl)));
    connect(m_searchWindow, SIGNAL(requestShowLinkInNewTab(QUrl)),
        OpenPagesManager::instance(), SLOT(createNewPageFromSearch(QUrl)));

#if defined(QT_NO_PRINTER)
        m_pageSetupAction->setVisible(false);
        m_printPreviewAction->setVisible(false);
        m_printAction->setVisible(false);
#endif
}

QMenu *MainWindow::toolBarMenu()
{
    TRACE_OBJ
    if (!m_toolBarMenu) {
        m_viewMenu->addSeparator();
        m_toolBarMenu = m_viewMenu->addMenu(tr("Toolbars"));
    }
    return m_toolBarMenu;
}

void MainWindow::setupFilterToolbar()
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    if (!helpEngine.filterFunctionalityEnabled())
        return;

    m_filterCombo = new QComboBox(this);
    m_filterCombo->setMinimumWidth(QFontMetrics(QFont()).
        width(QLatin1String("MakeTheComboBoxWidthEnough")));

    QToolBar *filterToolBar = addToolBar(tr("Filter Toolbar"));
    filterToolBar->setObjectName(QLatin1String("FilterToolBar"));
    filterToolBar->addWidget(new QLabel(tr("Filtered by:").append(QLatin1Char(' ')),
        this));
    filterToolBar->addWidget(m_filterCombo);

    if (!helpEngine.filterToolbarVisible())
        filterToolBar->hide();
    toolBarMenu()->addAction(filterToolBar->toggleViewAction());

    connect(&helpEngine, SIGNAL(setupFinished()), this,
        SLOT(setupFilterCombo()), Qt::QueuedConnection);
    connect(m_filterCombo, SIGNAL(activated(QString)), this,
        SLOT(filterDocumentation(QString)));
    connect(&helpEngine, SIGNAL(currentFilterChanged(QString)), this,
        SLOT(currentFilterChanged(QString)));

    setupFilterCombo();
}

void MainWindow::setupAddressToolbar()
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    if (!helpEngine.addressBarEnabled())
        return;

    m_addressLineEdit = new QLineEdit(this);
    QToolBar *addressToolBar = addToolBar(tr("Address Toolbar"));
    addressToolBar->setObjectName(QLatin1String("AddressToolBar"));
    insertToolBarBreak(addressToolBar);

    addressToolBar->addWidget(new QLabel(tr("Address:").append(QLatin1String(" ")),
        this));
    addressToolBar->addWidget(m_addressLineEdit);

    if (!helpEngine.addressBarVisible())
        addressToolBar->hide();
    toolBarMenu()->addAction(addressToolBar->toggleViewAction());

    // address lineedit
    connect(m_addressLineEdit, SIGNAL(returnPressed()), this,
        SLOT(gotoAddress()));
    connect(m_centralWidget, SIGNAL(currentViewerChanged()), this,
        SLOT(showNewAddress()));
    connect(m_centralWidget, SIGNAL(sourceChanged(QUrl)), this,
        SLOT(showNewAddress(QUrl)));
}

void MainWindow::updateAboutMenuText()
{
    TRACE_OBJ
    QByteArray ba = HelpEngineWrapper::instance().aboutMenuTexts();
    if (ba.size() > 0) {
        QString lang;
        QString str;
        QString trStr;
        QString currentLang = QLocale::system().name();
        int i = currentLang.indexOf(QLatin1Char('_'));
        if (i > -1)
            currentLang = currentLang.left(i);
        QDataStream s(&ba, QIODevice::ReadOnly);
        while (!s.atEnd()) {
            s >> lang;
            s >> str;
            if (lang == QLatin1String("default") && trStr.isEmpty()) {
                trStr = str;
            } else if (lang == currentLang) {
                trStr = str;
                break;
            }
        }
        if (!trStr.isEmpty())
            m_aboutAction->setText(trStr);
    }
}

void MainWindow::showNewAddress()
{
    TRACE_OBJ
    showNewAddress(m_centralWidget->currentSource());
}

void MainWindow::showNewAddress(const QUrl &url)
{
    TRACE_OBJ
    m_addressLineEdit->setText(url.toString());
}

void MainWindow::gotoAddress()
{
    TRACE_OBJ
    m_centralWidget->setSource(m_addressLineEdit->text());
}

void MainWindow::showTopicChooser(const QMap<QString, QUrl> &links,
                                  const QString &keyword)
{
    TRACE_OBJ
    TopicChooser tc(this, keyword, links);
    if (tc.exec() == QDialog::Accepted) {
        m_centralWidget->setSource(tc.link());
    }
}

void MainWindow::showPreferences()
{
    TRACE_OBJ
    PreferencesDialog dia(this);
    connect(&dia, SIGNAL(updateApplicationFont()), this,
        SLOT(updateApplicationFont()));
    connect(&dia, SIGNAL(updateBrowserFont()), m_centralWidget,
        SLOT(updateBrowserFont()));
    connect(&dia, SIGNAL(updateUserInterface()), m_centralWidget,
        SLOT(updateUserInterface()));
    dia.showDialog();
}

void MainWindow::syncContents()
{
    TRACE_OBJ
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    const QUrl url = m_centralWidget->currentSource();
    showContents();
    if (!m_contentWindow->syncToContent(url))
        statusBar()->showMessage(
            tr("Could not find the associated content item."), 3000);
    qApp->restoreOverrideCursor();
}

void MainWindow::showAboutDialog()
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    QByteArray contents;
    QByteArray ba = helpEngine.aboutTexts();
    if (!ba.isEmpty()) {
        QString lang;
        QByteArray cba;
        QString currentLang = QLocale::system().name();
        int i = currentLang.indexOf(QLatin1Char('_'));
        if (i > -1)
            currentLang = currentLang.left(i);
        QDataStream s(&ba, QIODevice::ReadOnly);
        while (!s.atEnd()) {
            s >> lang;
            s >> cba;
            if (lang == QLatin1String("default") && contents.isEmpty()) {
                contents = cba;
            } else if (lang == currentLang) {
                contents = cba;
                break;
            }
        }
    }

    AboutDialog aboutDia(this);

    QByteArray iconArray;
    if (!contents.isEmpty()) {
        iconArray = helpEngine.aboutIcon();
        QByteArray resources = helpEngine.aboutImages();
        QPixmap pix;
        pix.loadFromData(iconArray);
        aboutDia.setText(QString::fromUtf8(contents), resources);
        if (!pix.isNull())
            aboutDia.setPixmap(pix);
        aboutDia.setWindowTitle(aboutDia.documentTitle());
    } else {
        QByteArray resources;
        aboutDia.setText(tr("<center>"
            "<h3>%1</h3>"
            "<p>Version %2</p></center>"
            "<p>Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).</p>")
            .arg(tr("Qt Assistant")).arg(QLatin1String(QT_VERSION_STR)),
            resources);
        QLatin1String path(":/trolltech/assistant/images/assistant-128.png");
        aboutDia.setPixmap(QString(path));
    }
    if (aboutDia.windowTitle().isEmpty())
        aboutDia.setWindowTitle(tr("About %1").arg(windowTitle()));
    aboutDia.exec();
}

void MainWindow::setContentsVisible(bool visible)
{
    TRACE_OBJ
    if (visible)
        showContents();
    else
        hideContents();
}

void MainWindow::showContents()
{
    TRACE_OBJ
    activateDockWidget(m_contentWindow);
}

void MainWindow::hideContents()
{
    TRACE_OBJ
    m_contentWindow->parentWidget()->hide();
}

void MainWindow::setIndexVisible(bool visible)
{
    TRACE_OBJ
    if (visible)
        showIndex();
    else
        hideIndex();
}

void MainWindow::showIndex()
{
    TRACE_OBJ
    activateDockWidget(m_indexWindow);
}

void MainWindow::hideIndex()
{
    TRACE_OBJ
    m_indexWindow->parentWidget()->hide();
}

void MainWindow::setBookmarksVisible(bool visible)
{
    TRACE_OBJ
    if (visible)
        showBookmarksDockWidget();
    else
        hideBookmarksDockWidget();
}

void MainWindow::showBookmarksDockWidget()
{
    TRACE_OBJ
    activateDockWidget(m_bookmarkWidget);
}

void MainWindow::hideBookmarksDockWidget()
{
    TRACE_OBJ
    m_bookmarkWidget->parentWidget()->hide();
}

void MainWindow::setSearchVisible(bool visible)
{
    TRACE_OBJ
    if (visible)
        showSearch();
    else
        hideSearch();
}

void MainWindow::showSearch()
{
    TRACE_OBJ
    activateDockWidget(m_searchWindow);
}

void MainWindow::showOpenPages()
{
    TRACE_OBJ
    activateDockWidget(OpenPagesManager::instance()->openPagesWidget());
}

void MainWindow::hideSearch()
{
    TRACE_OBJ
    m_searchWindow->parentWidget()->hide();
}

void MainWindow::activateDockWidget(QWidget *w)
{
    TRACE_OBJ
    w->parentWidget()->show();
    w->parentWidget()->raise();
    w->setFocus();
}

void MainWindow::setIndexString(const QString &str)
{
    TRACE_OBJ
    m_indexWindow->setSearchLineEditText(str);
}

void MainWindow::activateCurrentBrowser()
{
    TRACE_OBJ
    CentralWidget::instance()->activateTab();
}

void MainWindow::activateCurrentCentralWidgetTab()
{
    TRACE_OBJ
    m_centralWidget->activateTab();
}

void MainWindow::updateApplicationFont()
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    QFont font = qApp->font();
    if (helpEngine.usesAppFont())
        font = helpEngine.appFont();

    const QWidgetList &widgets = qApp->allWidgets();
    foreach (QWidget* widget, widgets)
        widget->setFont(font);
}

void MainWindow::setupFilterCombo()
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    QString curFilter = m_filterCombo->currentText();
    if (curFilter.isEmpty())
        curFilter = helpEngine.currentFilter();
    m_filterCombo->clear();
    m_filterCombo->addItems(helpEngine.customFilters());
    int idx = m_filterCombo->findText(curFilter);
    if (idx < 0)
        idx = 0;
    m_filterCombo->setCurrentIndex(idx);
}

void MainWindow::filterDocumentation(const QString &customFilter)
{
    TRACE_OBJ
    HelpEngineWrapper::instance().setCurrentFilter(customFilter);
}

void MainWindow::expandTOC(int depth)
{
    TRACE_OBJ
    Q_ASSERT(depth >= -1);
    m_contentWindow->expandToDepth(depth);
}

void MainWindow::indexingStarted()
{
    TRACE_OBJ
    if (!m_progressWidget) {
        m_progressWidget = new QWidget();
        QLayout* hlayout = new QHBoxLayout(m_progressWidget);

        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

        QLabel *label = new QLabel(tr("Updating search index"));
        label->setSizePolicy(sizePolicy);
        hlayout->addWidget(label);

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, 0);
        progressBar->setTextVisible(false);
        progressBar->setSizePolicy(sizePolicy);

        hlayout->setSpacing(6);
        hlayout->setMargin(0);
        hlayout->addWidget(progressBar);

        statusBar()->addPermanentWidget(m_progressWidget);
    }
}

void MainWindow::indexingFinished()
{
    TRACE_OBJ
    statusBar()->removeWidget(m_progressWidget);
    delete m_progressWidget;
    m_progressWidget = 0;
}

QString MainWindow::collectionFileDirectory(bool createDir, const QString &cacheDir)
{
    TRACE_OBJ
    QString collectionPath =
        QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    if (collectionPath.isEmpty()) {
        if (cacheDir.isEmpty())
            collectionPath = QDir::homePath() + QDir::separator()
                + QLatin1String(".assistant");
        else
            collectionPath = QDir::homePath() + QLatin1String("/.") + cacheDir;
    } else {
        if (cacheDir.isEmpty())
            collectionPath = collectionPath + QLatin1String("/Trolltech/Assistant");
        else
            collectionPath = collectionPath + QDir::separator() + cacheDir;
    }
    collectionPath = QDir::cleanPath(collectionPath);
    if (createDir) {
        QDir dir;
        if (!dir.exists(collectionPath))
            dir.mkpath(collectionPath);
    }
    return collectionPath;
}

QString MainWindow::defaultHelpCollectionFileName()
{
    TRACE_OBJ
    // forces creation of the default collection file path
    return collectionFileDirectory(true) + QDir::separator() +
        QString(QLatin1String("qthelpcollection_%1.qhc")).
        arg(QLatin1String(QT_VERSION_STR));
}

void MainWindow::currentFilterChanged(const QString &filter)
{
    TRACE_OBJ
    const int index = m_filterCombo->findText(filter);
    Q_ASSERT(index != -1);
    m_filterCombo->setCurrentIndex(index);
}

void MainWindow::documentationRemoved(const QString &namespaceName)
{
    TRACE_OBJ
    OpenPagesManager::instance()->closePages(namespaceName);
}

void MainWindow::documentationUpdated(const QString &namespaceName)
{
    TRACE_OBJ
    OpenPagesManager::instance()->reloadPages(namespaceName);
}

void MainWindow::resetQtDocInfo(const QString &component)
{
    TRACE_OBJ
    HelpEngineWrapper::instance().setQtDocInfo(component,
        QStringList(QDateTime().toString(Qt::ISODate)));
}

void MainWindow::registerDocumentation(const QString &component,
                                       const QString &absFileName)
{
    TRACE_OBJ
    QString ns = QHelpEngineCore::namespaceName(absFileName);
    if (ns.isEmpty())
        return;

    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    if (helpEngine.registeredDocumentations().contains(ns))
        helpEngine.unregisterDocumentation(ns);
    if (!helpEngine.registerDocumentation(absFileName)) {
        QMessageBox::warning(this, tr("Qt Assistant"),
            tr("Could not register file '%1': %2").
            arg(absFileName).arg(helpEngine.error()));
    } else {
        QStringList docInfo;
        docInfo << QFileInfo(absFileName).lastModified().toString(Qt::ISODate)
                << absFileName;
        helpEngine.setQtDocInfo(component, docInfo);
    }
}

QT_END_NAMESPACE
