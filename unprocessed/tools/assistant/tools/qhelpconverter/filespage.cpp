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

#include <QtGui/QKeyEvent>
#include "filespage.h"

QT_BEGIN_NAMESPACE

FilesPage::FilesPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Unreferenced Files"));
    setSubTitle(tr("Remove files which are neither referenced "
        "by a keyword nor by the TOC."));

    m_ui.setupUi(this);
    m_ui.fileListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_ui.fileListWidget->installEventFilter(this);
    connect(m_ui.removeButton, SIGNAL(clicked()),
        this, SLOT(removeFile()));
    connect(m_ui.removeAllButton, SIGNAL(clicked()),
        this, SLOT(removeAllFiles()));

    m_ui.fileLabel->setText(tr("<p><b>Warning:</b> "
        "When removing images or stylesheets, be aware that those files "
        "are not directly referenced by the .adp or .dcf "
        "file.</p>"));
}

void FilesPage::setFilesToRemove(const QStringList &files)
{
    m_files = files;
    m_ui.fileListWidget->clear();
    m_ui.fileListWidget->addItems(files);
}

QStringList FilesPage::filesToRemove() const
{
    return m_filesToRemove;
}

void FilesPage::removeFile()
{
    int row = m_ui.fileListWidget->currentRow()
        - m_ui.fileListWidget->selectedItems().count() + 1;
    foreach (const QListWidgetItem *item, m_ui.fileListWidget->selectedItems()) {
        m_filesToRemove.append(item->text());
        delete item;
    }
    if (m_ui.fileListWidget->count() > row && row >= 0)
        m_ui.fileListWidget->setCurrentRow(row);
    else
        m_ui.fileListWidget->setCurrentRow(m_ui.fileListWidget->count());
}

void FilesPage::removeAllFiles()
{
    m_ui.fileListWidget->clear();
    m_filesToRemove = m_files;
}

bool FilesPage::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_ui.fileListWidget && event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Delete) {
            removeFile();
            return true;
        }
    }
    return QWizardPage::eventFilter(obj, event);
}

QT_END_NAMESPACE
