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

#include "preferencesdialog.h"

#include "centralwidget.h"
#include "filternamedialog.h"
#include "fontpanel.h"
#include "helpenginewrapper.h"
#include "installdialog.h"
#include "openpagesmanager.h"
#include "tracer.h"

#include <QtCore/QtAlgorithms>
#include <QtCore/QFileSystemWatcher>

#include <QtGui/QDesktopWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QFontDatabase>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

#include <QtHelp/QHelpEngineCore>

QT_BEGIN_NAMESPACE

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
    , m_appFontChanged(false)
    , m_browserFontChanged(false)
    , helpEngine(HelpEngineWrapper::instance())
{
    TRACE_OBJ
    m_ui.setupUi(this);

    connect(m_ui.buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
        this, SLOT(applyChanges()));
    connect(m_ui.buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
        this, SLOT(reject()));

    m_hideFiltersTab = !helpEngine.filterFunctionalityEnabled();
    m_hideDocsTab = !helpEngine.documentationManagerEnabled();

    if (!m_hideFiltersTab) {
        m_ui.attributeWidget->header()->hide();
        m_ui.attributeWidget->setRootIsDecorated(false);

        connect(m_ui.attributeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(updateFilterMap()));

        connect(m_ui.filterWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this,
            SLOT(updateAttributes(QListWidgetItem*)));

        connect(m_ui.filterAddButton, SIGNAL(clicked()), this,
            SLOT(addFilter()));
        connect(m_ui.filterRemoveButton, SIGNAL(clicked()), this,
            SLOT(removeFilter()));

        updateFilterPage();
    } else {
        m_ui.tabWidget->removeTab(m_ui.tabWidget->indexOf(m_ui.filtersTab));
    }

    if (!m_hideDocsTab) {
        connect(m_ui.docAddButton, SIGNAL(clicked()), this,
            SLOT(addDocumentationLocal()));
        connect(m_ui.docRemoveButton, SIGNAL(clicked()), this,
            SLOT(removeDocumentation()));

        m_docsBackup = helpEngine.registeredDocumentations();
        m_ui.registeredDocsListWidget->addItems(m_docsBackup);
    } else {
        m_ui.tabWidget->removeTab(m_ui.tabWidget->indexOf(m_ui.docsTab));
    }

    updateFontSettingsPage();
    updateOptionsPage();

    if (helpEngine.usesAppFont())
        setFont(helpEngine.appFont());
}

PreferencesDialog::~PreferencesDialog()
{
    TRACE_OBJ
    if (m_appFontChanged) {
        helpEngine.setAppFont(m_appFontPanel->selectedFont());
        helpEngine.setUseAppFont(m_appFontPanel->isChecked());
        helpEngine.setAppWritingSystem(m_appFontPanel->writingSystem());
        emit updateApplicationFont();
    }

    if (m_browserFontChanged) {
        helpEngine.setBrowserFont(m_browserFontPanel->selectedFont());
        helpEngine.setUseBrowserFont(m_browserFontPanel->isChecked());
        helpEngine.setBrowserWritingSystem(m_browserFontPanel->writingSystem());
        emit updateBrowserFont();
    }

    QString homePage = m_ui.homePageLineEdit->text();
    if (homePage.isEmpty())
        homePage = QLatin1String("help");
    helpEngine.setHomePage(homePage);

    int option = m_ui.helpStartComboBox->currentIndex();
    helpEngine.setStartOption(option);
}

void PreferencesDialog::showDialog()
{
    TRACE_OBJ
    if (exec() != Accepted)
        m_appFontChanged = m_browserFontChanged = false;
}

void PreferencesDialog::updateFilterPage()
{
    TRACE_OBJ
    m_ui.filterWidget->clear();
    m_ui.attributeWidget->clear();

    m_filterMapBackup.clear();
    const QStringList &filters = helpEngine.customFilters();
    foreach (const QString &filter, filters) {
        if (filter == HelpEngineWrapper::TrUnfiltered)
            continue;
        QStringList atts = helpEngine.filterAttributes(filter);
        m_filterMapBackup.insert(filter, atts);
        if (!m_filterMap.contains(filter))
            m_filterMap.insert(filter, atts);
    }

    m_ui.filterWidget->addItems(m_filterMap.keys());

    foreach (const QString &a, helpEngine.filterAttributes())
        new QTreeWidgetItem(m_ui.attributeWidget, QStringList() << a);

    if (!m_filterMap.keys().isEmpty())
        m_ui.filterWidget->setCurrentRow(0);
}

void PreferencesDialog::updateAttributes(QListWidgetItem *item)
{
    TRACE_OBJ
    QStringList checkedList;
    if (item)
        checkedList = m_filterMap.value(item->text());
    QTreeWidgetItem *itm;
    for (int i = 0; i < m_ui.attributeWidget->topLevelItemCount(); ++i) {
        itm = m_ui.attributeWidget->topLevelItem(i);
        if (checkedList.contains(itm->text(0)))
            itm->setCheckState(0, Qt::Checked);
        else
            itm->setCheckState(0, Qt::Unchecked);
    }
}

void PreferencesDialog::updateFilterMap()
{
    TRACE_OBJ
    if (!m_ui.filterWidget->currentItem())
        return;
    QString filter = m_ui.filterWidget->currentItem()->text();
    if (!m_filterMap.contains(filter))
        return;

    QStringList newAtts;
    QTreeWidgetItem *itm = 0;
    for (int i = 0; i < m_ui.attributeWidget->topLevelItemCount(); ++i) {
        itm = m_ui.attributeWidget->topLevelItem(i);
        if (itm->checkState(0) == Qt::Checked)
            newAtts.append(itm->text(0));
    }
    m_filterMap[filter] = newAtts;
}

void PreferencesDialog::addFilter()
{
    TRACE_OBJ
    FilterNameDialog dia(this);
    if (dia.exec() == QDialog::Rejected)
        return;

    QString filterName = dia.filterName();
    if (!m_filterMap.contains(filterName)) {
        m_filterMap.insert(filterName, QStringList());
        m_ui.filterWidget->addItem(filterName);
    }

    QList<QListWidgetItem*> lst = m_ui.filterWidget
        ->findItems(filterName, Qt::MatchCaseSensitive);
    m_ui.filterWidget->setCurrentItem(lst.first());
}

void PreferencesDialog::removeFilter()
{
    TRACE_OBJ
    QListWidgetItem *item =
        m_ui.filterWidget ->takeItem(m_ui.filterWidget->currentRow());
    if (!item)
        return;

    m_filterMap.remove(item->text());
    m_removedFilters.append(item->text());
    delete item;
    if (m_ui.filterWidget->count())
        m_ui.filterWidget->setCurrentRow(0);
}

void PreferencesDialog::addDocumentationLocal()
{
    TRACE_OBJ
    const QStringList fileNames = QFileDialog::getOpenFileNames(this,
        tr("Add Documentation"), QString(), tr("Qt Compressed Help Files (*.qch)"));
    if (fileNames.isEmpty())
        return;

    QStringList invalidFiles;
    QStringList alreadyRegistered;
    foreach (const QString &fileName, fileNames) {
        const QString nameSpace = QHelpEngineCore::namespaceName(fileName);
        if (nameSpace.isEmpty()) {
            invalidFiles.append(fileName);
            continue;
        }

        if (m_ui.registeredDocsListWidget->findItems(nameSpace,
            Qt::MatchFixedString).count()) {
                alreadyRegistered.append(nameSpace);
                continue;
        }

        if (helpEngine.registerDocumentation(fileName)) {
            m_ui.registeredDocsListWidget->addItem(nameSpace);
            m_regDocs.append(nameSpace);
            m_unregDocs.removeAll(nameSpace);
        }
    }

    if (!invalidFiles.isEmpty() || !alreadyRegistered.isEmpty()) {
        QString message;
        if (!alreadyRegistered.isEmpty()) {
            foreach (const QString &ns, alreadyRegistered) {
                message += tr("The namespace %1 is already registered!")
                    .arg(QString("<b>%1</b>").arg(ns)) + QLatin1String("<br>");
            }
            if (!invalidFiles.isEmpty())
                message.append(QLatin1String("<br>"));
        }

        if (!invalidFiles.isEmpty()) {
            message += tr("The specified file is not a valid Qt Help File!");
            message.append(QLatin1String("<ul>"));
            foreach (const QString &file, invalidFiles)
                message += QLatin1String("<li>") + file + QLatin1String("</li>");
            message.append(QLatin1String("</ul>"));
        }
        QMessageBox::warning(this, tr("Add Documentation"), message);
    }

    updateFilterPage();
}

void PreferencesDialog::removeDocumentation()
{
    TRACE_OBJ

    bool foundBefore = false;
    QList<QListWidgetItem*> l = m_ui.registeredDocsListWidget->selectedItems();
    foreach (QListWidgetItem* item, l) {
        const QString& ns = item->text();
        if (!foundBefore && OpenPagesManager::instance()->pagesOpenForNamespace(ns)) {
            if (0 == QMessageBox::information(this, tr("Remove Documentation"),
                tr("Some documents currently opened in Assistant reference the "
                   "documentation you are attempting to remove. Removing the "
                   "documentation will close those documents."), tr("Cancel"),
                tr("OK"))) return;
            foundBefore = true;
        }

        m_unregDocs.append(ns);
        delete m_ui.registeredDocsListWidget->takeItem(
            m_ui.registeredDocsListWidget->row(item));
    }

    if (m_ui.registeredDocsListWidget->count()) {
        m_ui.registeredDocsListWidget->setCurrentRow(0,
            QItemSelectionModel::ClearAndSelect);
    }
}

void PreferencesDialog::applyChanges()
{
    TRACE_OBJ
    bool filtersWereChanged = false;
    if (!m_hideFiltersTab) {
        if (m_filterMap.count() != m_filterMapBackup.count()) {
            filtersWereChanged = true;
        } else {
            QMapIterator<QString, QStringList> it(m_filterMapBackup);
            while (it.hasNext() && !filtersWereChanged) {
                it.next();
                if (!m_filterMap.contains(it.key())) {
                    filtersWereChanged = true;
                } else {
                    QStringList a = it.value();
                    QStringList b = m_filterMap.value(it.key());
                    if (a.count() != b.count()) {
                        filtersWereChanged = true;
                    } else {
                        QStringList::const_iterator i(a.constBegin());
                        while (i != a.constEnd()) {
                            if (!b.contains(*i)) {
                                filtersWereChanged = true;
                                break;
                            }
                            ++i;
                        }
                    }
                }
            }
        }
    }

    if (filtersWereChanged) {
        foreach (const QString &filter, m_removedFilters)
            helpEngine.removeCustomFilter(filter);
        QMapIterator<QString, QStringList> it(m_filterMap);
        while (it.hasNext()) {
            it.next();
            helpEngine.addCustomFilter(it.key(), it.value());
        }
    }

    foreach (const QString &doc, m_unregDocs) {
        OpenPagesManager::instance()->closePages(doc);
        helpEngine.unregisterDocumentation(doc);
    }

    if (filtersWereChanged || !m_regDocs.isEmpty() || !m_unregDocs.isEmpty())
        helpEngine.setupData();

    helpEngine.setShowTabs(m_ui.showTabs->isChecked());
    if (m_showTabs != m_ui.showTabs->isChecked())
        emit updateUserInterface();

    accept();
}

void PreferencesDialog::updateFontSettingsPage()
{
    TRACE_OBJ
    m_browserFontPanel = new FontPanel(this);
    m_browserFontPanel->setCheckable(true);
    m_ui.stackedWidget_2->insertWidget(0, m_browserFontPanel);

    m_appFontPanel = new FontPanel(this);
    m_appFontPanel->setCheckable(true);
    m_ui.stackedWidget_2->insertWidget(1, m_appFontPanel);

    m_ui.stackedWidget_2->setCurrentIndex(0);

    const QString customSettings(tr("Use custom settings"));
    m_appFontPanel->setTitle(customSettings);

    QFont font = helpEngine.appFont();
    m_appFontPanel->setSelectedFont(font);

    QFontDatabase::WritingSystem system = helpEngine.appWritingSystem();
    m_appFontPanel->setWritingSystem(system);

    m_appFontPanel->setChecked(helpEngine.usesAppFont());

    m_browserFontPanel->setTitle(customSettings);

    font = helpEngine.browserFont();
    m_browserFontPanel->setSelectedFont(font);

    system = helpEngine.browserWritingSystem();
    m_browserFontPanel->setWritingSystem(system);

    m_browserFontPanel->setChecked(helpEngine.usesBrowserFont());

    connect(m_appFontPanel, SIGNAL(toggled(bool)), this,
        SLOT(appFontSettingToggled(bool)));
    connect(m_browserFontPanel, SIGNAL(toggled(bool)), this,
        SLOT(browserFontSettingToggled(bool)));

    QList<QComboBox*> allCombos = m_appFontPanel->findChildren<QComboBox*>();
    foreach (QComboBox* box, allCombos) {
        connect(box, SIGNAL(currentIndexChanged(int)), this,
            SLOT(appFontSettingChanged(int)));
    }

    allCombos = m_browserFontPanel->findChildren<QComboBox*>();
    foreach (QComboBox* box, allCombos) {
        connect(box, SIGNAL(currentIndexChanged(int)), this,
            SLOT(browserFontSettingChanged(int)));
    }
}

void PreferencesDialog::appFontSettingToggled(bool on)
{
    TRACE_OBJ
    Q_UNUSED(on)
    m_appFontChanged = true;
}

void PreferencesDialog::appFontSettingChanged(int index)
{
    TRACE_OBJ
    Q_UNUSED(index)
    m_appFontChanged = true;
}

void PreferencesDialog::browserFontSettingToggled(bool on)
{
    TRACE_OBJ
    Q_UNUSED(on)
    m_browserFontChanged = true;
}

void PreferencesDialog::browserFontSettingChanged(int index)
{
    TRACE_OBJ
    Q_UNUSED(index)
    m_browserFontChanged = true;
}

void PreferencesDialog::updateOptionsPage()
{
    TRACE_OBJ
    m_ui.homePageLineEdit->setText(helpEngine.homePage());

    int option = helpEngine.startOption();
    m_ui.helpStartComboBox->setCurrentIndex(option);

    m_showTabs = helpEngine.showTabs();
    m_ui.showTabs->setChecked(m_showTabs);

    connect(m_ui.blankPageButton, SIGNAL(clicked()), this, SLOT(setBlankPage()));
    connect(m_ui.currentPageButton, SIGNAL(clicked()), this, SLOT(setCurrentPage()));
    connect(m_ui.defaultPageButton, SIGNAL(clicked()), this, SLOT(setDefaultPage()));
}

void PreferencesDialog::setBlankPage()
{
    TRACE_OBJ
    m_ui.homePageLineEdit->setText(QLatin1String("about:blank"));
}

void PreferencesDialog::setCurrentPage()
{
    TRACE_OBJ
    QString homepage = CentralWidget::instance()->currentSource().toString();
    if (homepage.isEmpty())
        homepage = QLatin1String("help");

    m_ui.homePageLineEdit->setText(homepage);
}

void PreferencesDialog::setDefaultPage()
{
    TRACE_OBJ
    m_ui.homePageLineEdit->setText(helpEngine.defaultHomePage());
}

QT_END_NAMESPACE
