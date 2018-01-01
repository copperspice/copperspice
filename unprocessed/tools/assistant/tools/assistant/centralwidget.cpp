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

#include "centralwidget.h"

#include "findwidget.h"
#include "helpenginewrapper.h"
#include "helpviewer.h"
#include "openpagesmanager.h"
#include "tracer.h"
#include "../shared/collectionconfiguration.h"

#include <QtCore/QTimer>

#include <QtGui/QKeyEvent>
#include <QtGui/QMenu>
#include <QtGui/QPageSetupDialog>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrintPreviewDialog>
#include <QtGui/QPrinter>
#include <QtGui/QStackedWidget>
#include <QtGui/QTextBrowser>
#include <QtGui/QVBoxLayout>

#include <QtHelp/QHelpSearchEngine>

QT_BEGIN_NAMESPACE

namespace {
    CentralWidget *staticCentralWidget = 0;
}

// -- TabBar

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent)
{
    TRACE_OBJ
#ifdef Q_OS_MAC
    setDocumentMode(true);
#endif
    setMovable(true);
    setShape(QTabBar::RoundedNorth);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred,
        QSizePolicy::TabWidget));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(slotTabCloseRequested(int)));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this,
        SLOT(slotCustomContextMenuRequested(QPoint)));
}

TabBar::~TabBar()
{
    TRACE_OBJ
}

int TabBar::addNewTab(const QString &title)
{
    TRACE_OBJ
    const int index = addTab(title);
    setTabsClosable(count() > 1);
    return index;
}

void TabBar::setCurrent(HelpViewer *viewer)
{
    TRACE_OBJ
    for (int i = 0; i < count(); ++i) {
        HelpViewer *data = tabData(i).value<HelpViewer*>();
        if (data == viewer) {
            setCurrentIndex(i);
            break;
        }
    }
}

void TabBar::removeTabAt(HelpViewer *viewer)
{
    TRACE_OBJ
    for (int i = 0; i < count(); ++i) {
        HelpViewer *data = tabData(i).value<HelpViewer*>();
        if (data == viewer) {
            removeTab(i);
            break;
        }
    }
    setTabsClosable(count() > 1);
}

void TabBar::titleChanged()
{
    TRACE_OBJ
    for (int i = 0; i < count(); ++i) {
        HelpViewer *data = tabData(i).value<HelpViewer*>();
        QString title = data->title();
        title.replace(QLatin1Char('&'), QLatin1String("&&"));
        setTabText(i, title.isEmpty() ? tr("(Untitled)") : title);
    }
}

void TabBar::slotCurrentChanged(int index)
{
    TRACE_OBJ
    emit currentTabChanged(tabData(index).value<HelpViewer*>());
}

void TabBar::slotTabCloseRequested(int index)
{
    TRACE_OBJ
    OpenPagesManager::instance()->closePage(tabData(index).value<HelpViewer*>());
}

void TabBar::slotCustomContextMenuRequested(const QPoint &pos)
{
    TRACE_OBJ
    const int tab = tabAt(pos);
    if (tab < 0)
        return;

    QMenu menu(QLatin1String(""), this);
    menu.addAction(tr("New &Tab"), OpenPagesManager::instance(), SLOT(createPage()));

    const bool enableAction = count() > 1;
    QAction *closePage = menu.addAction(tr("&Close Tab"));
    closePage->setEnabled(enableAction);

    QAction *closePages = menu.addAction(tr("Close Other Tabs"));
    closePages->setEnabled(enableAction);

    menu.addSeparator();

    HelpViewer *viewer = tabData(tab).value<HelpViewer*>();
    QAction *newBookmark = menu.addAction(tr("Add Bookmark for this Page..."));
    const QString &url = viewer->source().toString();
    if (url.isEmpty() || url == QLatin1String("about:blank"))
        newBookmark->setEnabled(false);

    QAction *pickedAction = menu.exec(mapToGlobal(pos));
    if (pickedAction == closePage)
        slotTabCloseRequested(tab);
    else if (pickedAction == closePages) {
        for (int i = count() - 1; i >= 0; --i) {
            if (i != tab)
                slotTabCloseRequested(i);
        }
    } else if (pickedAction == newBookmark)
        emit addBookmark(viewer->title(), url);
}

// -- CentralWidget

CentralWidget::CentralWidget(QWidget *parent)
    : QWidget(parent)
#ifndef QT_NO_PRINTER
    , m_printer(0)
#endif
    , m_findWidget(new FindWidget(this))
    , m_stackedWidget(new QStackedWidget(this))
    , m_tabBar(new TabBar(this))
{
    TRACE_OBJ
    staticCentralWidget = this;
    QVBoxLayout *vboxLayout = new QVBoxLayout(this);

    vboxLayout->setMargin(0);
    vboxLayout->setSpacing(0);
    vboxLayout->addWidget(m_tabBar);
    m_tabBar->setVisible(HelpEngineWrapper::instance().showTabs());
    vboxLayout->addWidget(m_stackedWidget);
    vboxLayout->addWidget(m_findWidget);
    m_findWidget->hide();

    connect(m_findWidget, SIGNAL(findNext()), this, SLOT(findNext()));
    connect(m_findWidget, SIGNAL(findPrevious()), this, SLOT(findPrevious()));
    connect(m_findWidget, SIGNAL(find(QString, bool, bool)), this,
        SLOT(find(QString, bool, bool)));
    connect(m_findWidget, SIGNAL(escapePressed()), this, SLOT(activateTab()));
    connect(m_tabBar, SIGNAL(addBookmark(QString, QString)), this,
        SIGNAL(addBookmark(QString, QString)));
}

CentralWidget::~CentralWidget()
{
    TRACE_OBJ
    QStringList zoomFactors;
    QStringList currentPages;
    for (int i = 0; i < m_stackedWidget->count(); ++i) {
        const HelpViewer * const viewer = viewerAt(i);
        const QUrl &source = viewer->source();
        if (source.isValid()) {
            currentPages << source.toString();
            zoomFactors << QString::number(viewer->scale());
        }
    }

    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    helpEngine.setLastShownPages(currentPages);
    helpEngine.setLastZoomFactors(zoomFactors);
    helpEngine.setLastTabPage(m_stackedWidget->currentIndex());

#ifndef QT_NO_PRINTER
    delete m_printer;
#endif
}

CentralWidget *CentralWidget::instance()
{
    TRACE_OBJ
    return staticCentralWidget;
}

QUrl CentralWidget::currentSource() const
{
    TRACE_OBJ
    return currentHelpViewer()->source();
}

QString CentralWidget::currentTitle() const
{
    TRACE_OBJ
    return currentHelpViewer()->title();
}

bool CentralWidget::hasSelection() const
{
    TRACE_OBJ
    return !currentHelpViewer()->selectedText().isEmpty();
}

bool CentralWidget::isForwardAvailable() const
{
    TRACE_OBJ
    return currentHelpViewer()->isForwardAvailable();
}

bool CentralWidget::isBackwardAvailable() const
{
    TRACE_OBJ
    return currentHelpViewer()->isBackwardAvailable();
}

HelpViewer* CentralWidget::viewerAt(int index) const
{
    TRACE_OBJ
    return static_cast<HelpViewer*>(m_stackedWidget->widget(index));
}

HelpViewer* CentralWidget::currentHelpViewer() const
{
    TRACE_OBJ
    return static_cast<HelpViewer *>(m_stackedWidget->currentWidget());
}

void CentralWidget::addPage(HelpViewer *page, bool fromSearch)
{
    TRACE_OBJ
    page->installEventFilter(this);
    page->setFocus(Qt::OtherFocusReason);
    connectSignals(page);
    const int index = m_stackedWidget->addWidget(page);
    m_tabBar->setTabData(m_tabBar->addNewTab(page->title()),
        QVariant::fromValue(viewerAt(index)));
    connect (page, SIGNAL(titleChanged()), m_tabBar, SLOT(titleChanged()));

    if (fromSearch) {
        connect(currentHelpViewer(), SIGNAL(loadFinished(bool)), this,
            SLOT(highlightSearchTerms()));
    }
}

void CentralWidget::removePage(int index)
{
    TRACE_OBJ
    const bool currentChanged = index == currentIndex();
    m_tabBar->removeTabAt(viewerAt(index));
    m_stackedWidget->removeWidget(m_stackedWidget->widget(index));
    if (currentChanged)
        emit currentViewerChanged();
}

int CentralWidget::currentIndex() const
{
    TRACE_OBJ
    return  m_stackedWidget->currentIndex();
}

void CentralWidget::setCurrentPage(HelpViewer *page)
{
    TRACE_OBJ
    m_tabBar->setCurrent(page);
    m_stackedWidget->setCurrentWidget(page);
    emit currentViewerChanged();
}

void CentralWidget::connectTabBar()
{
    TRACE_OBJ
    connect(m_tabBar, SIGNAL(currentTabChanged(HelpViewer*)),
        OpenPagesManager::instance(), SLOT(setCurrentPage(HelpViewer*)));
}

// -- public slots

void CentralWidget::copy()
{
    TRACE_OBJ
    currentHelpViewer()->copy();
}

void CentralWidget::home()
{
    TRACE_OBJ
    currentHelpViewer()->home();
}

void CentralWidget::zoomIn()
{
    TRACE_OBJ
    currentHelpViewer()->scaleUp();
}

void CentralWidget::zoomOut()
{
    TRACE_OBJ
    currentHelpViewer()->scaleDown();
}

void CentralWidget::resetZoom()
{
    TRACE_OBJ
    currentHelpViewer()->resetScale();
}

void CentralWidget::forward()
{
    TRACE_OBJ
    currentHelpViewer()->forward();
}

void CentralWidget::nextPage()
{
    TRACE_OBJ
    m_stackedWidget->setCurrentIndex((m_stackedWidget->currentIndex() + 1)
        % m_stackedWidget->count());
}

void CentralWidget::backward()
{
    TRACE_OBJ
    currentHelpViewer()->backward();
}

void CentralWidget::previousPage()
{
    TRACE_OBJ
    m_stackedWidget->setCurrentIndex((m_stackedWidget->currentIndex() - 1)
        % m_stackedWidget->count());
}

void CentralWidget::print()
{
    TRACE_OBJ
#ifndef QT_NO_PRINTER
    initPrinter();
    QPrintDialog dlg(m_printer, this);

    if (!currentHelpViewer()->selectedText().isEmpty())
        dlg.addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg.addEnabledOption(QAbstractPrintDialog::PrintPageRange);
    dlg.addEnabledOption(QAbstractPrintDialog::PrintCollateCopies);
    dlg.setWindowTitle(tr("Print Document"));
    if (dlg.exec() == QDialog::Accepted)
        currentHelpViewer()->print(m_printer);
#endif
}

void CentralWidget::pageSetup()
{
    TRACE_OBJ
#ifndef QT_NO_PRINTER
    initPrinter();
    QPageSetupDialog dlg(m_printer);
    dlg.exec();
#endif
}

void CentralWidget::printPreview()
{
    TRACE_OBJ
#ifndef QT_NO_PRINTER
    initPrinter();
    QPrintPreviewDialog preview(m_printer, this);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)),
        SLOT(printPreview(QPrinter*)));
    preview.exec();
#endif
}

void CentralWidget::setSource(const QUrl &url)
{
    TRACE_OBJ
    HelpViewer *viewer = currentHelpViewer();
    viewer->setSource(url);
    viewer->setFocus(Qt::OtherFocusReason);
}

void CentralWidget::setSourceFromSearch(const QUrl &url)
{
    TRACE_OBJ
    connect(currentHelpViewer(), SIGNAL(loadFinished(bool)), this,
        SLOT(highlightSearchTerms()));
    currentHelpViewer()->setSource(url);
    currentHelpViewer()->setFocus(Qt::OtherFocusReason);
}

void CentralWidget::findNext()
{
    TRACE_OBJ
    find(m_findWidget->text(), true, false);
}

void CentralWidget::findPrevious()
{
    TRACE_OBJ
    find(m_findWidget->text(), false, false);
}

void CentralWidget::find(const QString &ttf, bool forward, bool incremental)
{
    TRACE_OBJ
    bool found = false;
    if (HelpViewer *viewer = currentHelpViewer()) {
        HelpViewer::FindFlags flags = 0;
        if (!forward)
            flags |= HelpViewer::FindBackward;
        if (m_findWidget->caseSensitive())
            flags |= HelpViewer::FindCaseSensitively;
        found = viewer->findText(ttf, flags, incremental, false);
    }

    if (!found && ttf.isEmpty())
        found = true;   // the line edit is empty, no need to mark it red...

    if (!m_findWidget->isVisible())
        m_findWidget->show();
    m_findWidget->setPalette(found);
}

void CentralWidget::activateTab()
{
    TRACE_OBJ
    currentHelpViewer()->setFocus();
}

void CentralWidget::showTextSearch()
{
    TRACE_OBJ
    m_findWidget->show();
}

void CentralWidget::updateBrowserFont()
{
    TRACE_OBJ
    const int count = m_stackedWidget->count();
    const QFont &font = viewerAt(count - 1)->viewerFont();
    for (int i = 0; i < count; ++i)
        viewerAt(i)->setViewerFont(font);
}

void CentralWidget::updateUserInterface()
{
    m_tabBar->setVisible(HelpEngineWrapper::instance().showTabs());
}

// -- protected

void CentralWidget::keyPressEvent(QKeyEvent *e)
{
    TRACE_OBJ
    const QString &text = e->text();
    if (text.startsWith(QLatin1Char('/'))) {
        if (!m_findWidget->isVisible()) {
            m_findWidget->showAndClear();
        } else {
            m_findWidget->show();
        }
    } else {
        QWidget::keyPressEvent(e);
    }
}

void CentralWidget::focusInEvent(QFocusEvent * /* event */)
{
    TRACE_OBJ
    // If we have a current help viewer then this is the 'focus proxy',
    // otherwise it's the central widget. This is needed, so an embedding
    // program can just set the focus to the central widget and it does
    // The Right Thing(TM)
    QObject *receiver = m_stackedWidget;
    if (HelpViewer *viewer = currentHelpViewer())
        receiver = viewer;
    QTimer::singleShot(1, receiver, SLOT(setFocus()));
}

// -- private slots

void CentralWidget::highlightSearchTerms()
{
    TRACE_OBJ
    QHelpSearchEngine *searchEngine =
        HelpEngineWrapper::instance().searchEngine();
    QList<QHelpSearchQuery> queryList = searchEngine->query();

    QStringList terms;
    foreach (const QHelpSearchQuery &query, queryList) {
        switch (query.fieldName) {
            default: break;
            case QHelpSearchQuery::ALL: {
            case QHelpSearchQuery::PHRASE:
            case QHelpSearchQuery::DEFAULT:
            case QHelpSearchQuery::ATLEAST:
                foreach (QString term, query.wordList)
                    terms.append(term.remove(QLatin1Char('"')));
            }
        }
    }

    HelpViewer *viewer = currentHelpViewer();
    foreach (const QString& term, terms)
        viewer->findText(term, 0, false, true);
    disconnect(viewer, SIGNAL(loadFinished(bool)), this,
        SLOT(highlightSearchTerms()));
}

void CentralWidget::printPreview(QPrinter *p)
{
    TRACE_OBJ
#ifndef QT_NO_PRINTER
    currentHelpViewer()->print(p);
#endif
}

void CentralWidget::handleSourceChanged(const QUrl &url)
{
    TRACE_OBJ
    if (sender() == currentHelpViewer())
        emit sourceChanged(url);
}

// -- private

void CentralWidget::initPrinter()
{
    TRACE_OBJ
#ifndef QT_NO_PRINTER
    if (!m_printer)
        m_printer = new QPrinter(QPrinter::HighResolution);
#endif
}

void CentralWidget::connectSignals(HelpViewer *page)
{
    TRACE_OBJ
    connect(page, SIGNAL(copyAvailable(bool)), this,
        SIGNAL(copyAvailable(bool)));
    connect(page, SIGNAL(forwardAvailable(bool)), this,
        SIGNAL(forwardAvailable(bool)));
    connect(page, SIGNAL(backwardAvailable(bool)), this,
        SIGNAL(backwardAvailable(bool)));
    connect(page, SIGNAL(sourceChanged(QUrl)), this,
        SLOT(handleSourceChanged(QUrl)));
    connect(page, SIGNAL(highlighted(QString)), this,
            SIGNAL(highlighted(QString)));
    connect(page, SIGNAL(printRequested()), this, SLOT(print()));
}

bool CentralWidget::eventFilter(QObject *object, QEvent *e)
{
    TRACE_OBJ
    if (e->type() != QEvent::KeyPress)
        return QWidget::eventFilter(object, e);

    HelpViewer *viewer = currentHelpViewer();
    QKeyEvent *keyEvent = static_cast<QKeyEvent*> (e);
    if (viewer == object && keyEvent->key() == Qt::Key_Backspace) {
        if (viewer->isBackwardAvailable()) {
#if !defined(QT_NO_WEBKIT)
            // this helps in case there is an html <input> field
            if (!viewer->hasFocus())
#endif
                viewer->backward();
        }
    }
    return QWidget::eventFilter(object, e);
}

QT_END_NAMESPACE
