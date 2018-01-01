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

#include "plaintexteditor_p.h"
#include "abstractsettings_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>

#include <QtGui/QPlainTextEdit>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

static const char *PlainTextDialogC = "PlainTextDialog";
static const char *Geometry = "Geometry";


namespace qdesigner_internal {

PlainTextEditorDialog::PlainTextEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent)  :
    QDialog(parent),
    m_editor(new QPlainTextEdit),
    m_core(core)
{
    setWindowTitle(tr("Edit text"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->addWidget(m_editor);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    QPushButton *ok_button = buttonBox->button(QDialogButtonBox::Ok);
    ok_button->setDefault(true);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    vlayout->addWidget(buttonBox);

    QDesignerSettingsInterface *settings = core->settingsManager();
    settings->beginGroup(QLatin1String(PlainTextDialogC));

    if (settings->contains(QLatin1String(Geometry)))
        restoreGeometry(settings->value(QLatin1String(Geometry)).toByteArray());

    settings->endGroup();
}

PlainTextEditorDialog::~PlainTextEditorDialog()
{
    QDesignerSettingsInterface *settings = m_core->settingsManager();
    settings->beginGroup(QLatin1String(PlainTextDialogC));

    settings->setValue(QLatin1String(Geometry), saveGeometry());
    settings->endGroup();
}

int PlainTextEditorDialog::showDialog()
{
    m_editor->setFocus();
    return exec();
}

void PlainTextEditorDialog::setDefaultFont(const QFont &font)
{
    m_editor->setFont(font);
}

void PlainTextEditorDialog::setText(const QString &text)
{
    m_editor->setPlainText(text);
}

QString PlainTextEditorDialog::text() const
{
    return m_editor->toPlainText();
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
