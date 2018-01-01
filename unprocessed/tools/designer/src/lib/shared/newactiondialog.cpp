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

#include "newactiondialog_p.h"
#include "ui_newactiondialog.h"
#include "richtexteditor_p.h"
#include "actioneditor_p.h"
#include "formwindowbase_p.h"
#include "qdesigner_utils_p.h"
#include "iconloader_p.h"

#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformeditor.h>

#include <QtGui/QPushButton>
#include <QtCore/QRegExp>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
// -------------------- ActionData

ActionData::ActionData() :
    checkable(false)
{
}

// Returns a combination of ChangeMask flags
unsigned ActionData::compare(const ActionData &rhs) const
{
    unsigned rc = 0;
    if (text != rhs.text)
        rc |= TextChanged;
    if (name != rhs.name)
        rc |= NameChanged;
    if (toolTip != rhs.toolTip)
        rc |= ToolTipChanged ;
    if (icon != rhs.icon)
        rc |= IconChanged ;
    if (checkable != rhs.checkable)
        rc |= CheckableChanged;
    if (keysequence != rhs.keysequence)
        rc |= KeysequenceChanged ;
    return rc;
}

// -------------------- NewActionDialog
NewActionDialog::NewActionDialog(ActionEditor *parent) :
    QDialog(parent, Qt::Sheet),
    m_ui(new Ui::NewActionDialog),
    m_actionEditor(parent)
{
    m_ui->setupUi(this);
    
    m_ui->tooltipEditor->setTextPropertyValidationMode(ValidationRichText);
    connect(m_ui->toolTipToolButton, SIGNAL(clicked()), this, SLOT(slotEditToolTip()));

    m_ui->keysequenceResetToolButton->setIcon(createIconSet(QLatin1String("resetproperty.png")));
    connect(m_ui->keysequenceResetToolButton, SIGNAL(clicked()), this, SLOT(slotResetKeySequence()));

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    m_ui->editActionText->setFocus();
    m_auto_update_object_name = true;
    updateButtons();

    QDesignerFormWindowInterface *form = parent->formWindow();
    m_ui->iconSelector->setFormEditor(form->core());
    FormWindowBase *formBase = qobject_cast<FormWindowBase *>(form);

    if (formBase) {
        m_ui->iconSelector->setPixmapCache(formBase->pixmapCache());
        m_ui->iconSelector->setIconCache(formBase->iconCache());
    }
}

NewActionDialog::~NewActionDialog()
{
    delete m_ui;
}

QString NewActionDialog::actionText() const
{
    return m_ui->editActionText->text();
}

QString NewActionDialog::actionName() const
{
    return m_ui->editObjectName->text();
}

ActionData NewActionDialog::actionData() const
{
    ActionData rc;
    rc.text = actionText();
    rc.name = actionName();
    rc.toolTip = m_ui->tooltipEditor->text();
    rc.icon = m_ui->iconSelector->icon();
    rc.icon.setTheme(m_ui->iconThemeEditor->theme());
    rc.checkable = m_ui->checkableCheckBox->checkState() == Qt::Checked;
    rc.keysequence = PropertySheetKeySequenceValue(m_ui->keySequenceEdit->keySequence());
    return rc;
}

void NewActionDialog::setActionData(const ActionData &d)
{
    m_ui->editActionText->setText(d.text);
    m_ui->editObjectName->setText(d.name);
    m_ui->iconSelector->setIcon(d.icon.unthemed());
    m_ui->iconThemeEditor->setTheme(d.icon.theme());
    m_ui->tooltipEditor->setText(d.toolTip);
    m_ui->keySequenceEdit->setKeySequence(d.keysequence.value());
    m_ui->checkableCheckBox->setCheckState(d.checkable ? Qt::Checked : Qt::Unchecked);

    m_auto_update_object_name = false;
    updateButtons();
}

void NewActionDialog::on_editActionText_textEdited(const QString &text)
{
    if (text.isEmpty())
        m_auto_update_object_name = true;

    if (m_auto_update_object_name)
        m_ui->editObjectName->setText(ActionEditor::actionTextToName(text));

    updateButtons();
}

void NewActionDialog::on_editObjectName_textEdited(const QString&)
{
    updateButtons();
    m_auto_update_object_name = false;
}

void NewActionDialog::slotEditToolTip()
{
    const QString oldToolTip = m_ui->tooltipEditor->text();
    RichTextEditorDialog richTextDialog(m_actionEditor->core(), this);
    richTextDialog.setText(oldToolTip);
    if (richTextDialog.showDialog() == QDialog::Rejected)
        return;
    const QString newToolTip = richTextDialog.text();
    if (newToolTip != oldToolTip)
        m_ui->tooltipEditor->setText(newToolTip);
}

void NewActionDialog::slotResetKeySequence()
{
    m_ui->keySequenceEdit->setKeySequence(QKeySequence());
    m_ui->keySequenceEdit->setFocus(Qt::MouseFocusReason);
}

void NewActionDialog::updateButtons()
{
    QPushButton *okButton = m_ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(!actionText().isEmpty() && !actionName().isEmpty());
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
