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
#include "ui_preferencesdialog.h"
#include "qdesigner_appearanceoptions.h"

#include <QtDesigner/private/abstractoptionspage_p.h>

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtGui/QFileDialog>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

PreferencesDialog::PreferencesDialog(QDesignerFormEditorInterface *core, QWidget *parentWidget) :
    QDialog(parentWidget),
    m_ui(new Ui::PreferencesDialog()),
    m_core(core)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    m_ui->setupUi(this);

    m_optionsPages = core->optionsPages();

    m_ui->m_optionTabWidget->clear();
    foreach (QDesignerOptionsPageInterface *optionsPage, m_optionsPages) {
        QWidget *page = optionsPage->createPage(this);
        m_ui->m_optionTabWidget->addTab(page, optionsPage->name());
        if (QDesignerAppearanceOptionsWidget *appearanceWidget = qobject_cast<QDesignerAppearanceOptionsWidget *>(page))
            connect(appearanceWidget, SIGNAL(uiModeChanged(bool)), this, SLOT(slotUiModeChanged(bool)));
    }

    connect(m_ui->m_dialogButtonBox, SIGNAL(rejected()), this, SLOT(slotRejected()));
    connect(m_ui->m_dialogButtonBox, SIGNAL(accepted()), this, SLOT(slotAccepted()));
    connect(applyButton(), SIGNAL(clicked()), this, SLOT(slotApply()));
}

PreferencesDialog::~PreferencesDialog()
{
    delete m_ui;
}

QPushButton *PreferencesDialog::applyButton() const
{
    return m_ui->m_dialogButtonBox->button(QDialogButtonBox::Apply);
}

void PreferencesDialog::slotApply()
{
    foreach (QDesignerOptionsPageInterface *optionsPage, m_optionsPages)
        optionsPage->apply();
}

void PreferencesDialog::slotAccepted()
{
    slotApply();
    closeOptionPages();
    accept();
}

void PreferencesDialog::slotRejected()
{
    closeOptionPages();
    reject();
}

void PreferencesDialog::slotUiModeChanged(bool modified)
{
    // Cannot "apply" a ui mode change (destroy the dialogs parent)
    applyButton()->setEnabled(!modified);
}

void PreferencesDialog::closeOptionPages()
{
    foreach (QDesignerOptionsPageInterface *optionsPage, m_optionsPages)
        optionsPage->finish();
}

QT_END_NAMESPACE
