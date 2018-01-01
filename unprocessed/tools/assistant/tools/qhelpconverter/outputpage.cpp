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

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QMessageBox>

#include "outputpage.h"

QT_BEGIN_NAMESPACE

OutputPage::OutputPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Output File Names"));
    setSubTitle(tr("Specify the file names for the output files."));
    setButtonText(QWizard::NextButton, tr("Convert..."));

    m_ui.setupUi(this);
    connect(m_ui.projectLineEdit, SIGNAL(textChanged(QString)),
        this, SIGNAL(completeChanged()));
    connect(m_ui.collectionLineEdit, SIGNAL(textChanged(QString)),
        this, SIGNAL(completeChanged()));

    registerField(QLatin1String("ProjectFileName"),
        m_ui.projectLineEdit);
    registerField(QLatin1String("CollectionFileName"),
        m_ui.collectionLineEdit);
}

void OutputPage::setPath(const QString &path)
{
    m_path = path;
}

void OutputPage::setCollectionComponentEnabled(bool enabled)
{
    m_ui.collectionLineEdit->setEnabled(enabled);
    m_ui.label_2->setEnabled(enabled);
}

bool OutputPage::isComplete() const
{
    if (m_ui.projectLineEdit->text().isEmpty()
        || m_ui.collectionLineEdit->text().isEmpty())
        return false;
    return true;
}

bool OutputPage::validatePage()
{
    return checkFile(m_ui.projectLineEdit->text(),
        tr("Qt Help Project File"))
        && checkFile(m_ui.collectionLineEdit->text(),
        tr("Qt Help Collection Project File"));
}

bool OutputPage::checkFile(const QString &fileName, const QString &title)
{
    QFile fi(m_path + QDir::separator() + fileName);
    if (!fi.exists())
        return true;
    
    if (QMessageBox::warning(this, title,
        tr("The specified file %1 already exist.\n\nDo you want to remove it?")
        .arg(fileName), tr("Remove"), tr("Cancel")) == 0) {
        return fi.remove();
    }
    return false;
}

QT_END_NAMESPACE
