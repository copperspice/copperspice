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

#include <QtCore/QFile>
#include <QtCore/QVariant>

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>

#include "inputpage.h"
#include "adpreader.h"

QT_BEGIN_NAMESPACE

InputPage::InputPage(AdpReader *reader, QWidget *parent)
    : QWizardPage(parent)
{
    m_adpReader = reader;
    setTitle(tr("Input File"));
    setSubTitle(tr("Specify the .adp or .dcf file you want "
        "to convert to the new Qt help project format and/or "
        "collection format."));

    m_ui.setupUi(this);
    connect(m_ui.browseButton, SIGNAL(clicked()),
        this, SLOT(getFileName()));

    registerField(QLatin1String("adpFileName"), m_ui.fileLineEdit);
}

void InputPage::getFileName()
{
    QString f = QFileDialog::getOpenFileName(this, tr("Open file"), QString(),
        tr("Qt Help Files (*.adp *.dcf)"));
    if (!f.isEmpty())
        m_ui.fileLineEdit->setText(f);
}

bool InputPage::validatePage()
{
    QFile f(m_ui.fileLineEdit->text().trimmed());
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("File Open Error"),
            tr("The specified file could not be opened!"));
        return false;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    m_adpReader->readData(f.readAll());
    QApplication::restoreOverrideCursor();
    if (m_adpReader->hasError()) {
        QMessageBox::critical(this, tr("File Parsing Error"),
            tr("Parsing error in line %1!").arg(m_adpReader->lineNumber()));
        return false;
    }

    return true;
}

QT_END_NAMESPACE
