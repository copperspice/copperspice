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

#include "listwidgeteditor.h"
#include <designerpropertymanager.h>
#include <abstractformbuilder.h>

#include <QtDesigner/private/abstractsettings_p.h>
#include <QtDesigner/QDesignerFormEditorInterface>

#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QDialogButtonBox>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

ListWidgetEditor::ListWidgetEditor(QDesignerFormWindowInterface *form,
                                   QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

    m_itemsEditor = new ItemListEditor(form, 0);
    m_itemsEditor->layout()->setMargin(0);
    m_itemsEditor->setNewItemText(tr("New Item"));

    QFrame *sep = new QFrame;
    sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    QBoxLayout *box = new QVBoxLayout(this);
    box->addWidget(m_itemsEditor);
    box->addWidget(sep);
    box->addWidget(buttonBox);

    // Numbers copied from itemlisteditor.ui
    // (Automatic resizing doesn't work because ui has parent).
    resize(550, 360);
}

static AbstractItemEditor::PropertyDefinition listBoxPropList[] = {
    { Qt::DisplayPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "text" },
    { Qt::DecorationPropertyRole, 0, DesignerPropertyManager::designerIconTypeId, "icon" },
    { Qt::ToolTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "toolTip" },
    { Qt::StatusTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "statusTip" },
    { Qt::WhatsThisPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "whatsThis" },
    { Qt::FontRole, QVariant::Font, 0, "font" },
    { Qt::TextAlignmentRole, 0, DesignerPropertyManager::designerAlignmentTypeId, "textAlignment" },
    { Qt::BackgroundRole, QVariant::Brush, 0, "background" },
    { Qt::ForegroundRole, QVariant::Brush, 0, "foreground" },
    { ItemFlagsShadowRole, 0, QtVariantPropertyManager::flagTypeId, "flags" },
    { Qt::CheckStateRole, 0, QtVariantPropertyManager::enumTypeId, "checkState" },
    { 0, 0, 0, 0 }
};

ListContents ListWidgetEditor::fillContentsFromListWidget(QListWidget *listWidget)
{
    setWindowTitle(tr("Edit List Widget"));

    ListContents retVal;
    retVal.createFromListWidget(listWidget, false);
    retVal.applyToListWidget(m_itemsEditor->listWidget(), m_itemsEditor->iconCache(), true);

    m_itemsEditor->setupEditor(listWidget, listBoxPropList);

    return retVal;
}

static AbstractItemEditor::PropertyDefinition comboBoxPropList[] = {
    { Qt::DisplayPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "text" },
    { Qt::DecorationPropertyRole, 0, DesignerPropertyManager::designerIconTypeId, "icon" },
    { 0, 0, 0, 0 }
};

ListContents ListWidgetEditor::fillContentsFromComboBox(QComboBox *comboBox)
{
    setWindowTitle(tr("Edit Combobox"));

    ListContents retVal;
    retVal.createFromComboBox(comboBox);
    retVal.applyToListWidget(m_itemsEditor->listWidget(), m_itemsEditor->iconCache(), true);

    m_itemsEditor->setupEditor(comboBox, comboBoxPropList);

    return retVal;
}

ListContents ListWidgetEditor::contents() const
{
    ListContents retVal;
    retVal.createFromListWidget(m_itemsEditor->listWidget(), true);
    return retVal;
}

QT_END_NAMESPACE
