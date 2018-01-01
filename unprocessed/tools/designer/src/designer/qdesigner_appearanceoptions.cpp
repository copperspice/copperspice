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

#include "qdesigner_appearanceoptions.h"
#include "ui_qdesigner_appearanceoptions.h"

#include "qdesigner_settings.h"
#include "qdesigner_toolwindow.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

// ---------------- AppearanceOptions
AppearanceOptions::AppearanceOptions() :
     uiMode(DockedMode)
{
}

bool AppearanceOptions::equals(const AppearanceOptions &rhs) const
{
    return uiMode == rhs.uiMode && toolWindowFontSettings == rhs.toolWindowFontSettings;
}

void AppearanceOptions::toSettings(QDesignerSettings &settings) const
{
    settings.setUiMode(uiMode);
    settings.setToolWindowFont(toolWindowFontSettings);
}

void AppearanceOptions::fromSettings(const QDesignerSettings &settings)
{
    uiMode = settings.uiMode();
    toolWindowFontSettings = settings.toolWindowFont();
}

// ---------------- QDesignerAppearanceOptionsWidget
QDesignerAppearanceOptionsWidget::QDesignerAppearanceOptionsWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::AppearanceOptionsWidget),
    m_initialUIMode(NeutralMode)
{
    m_ui->setupUi(this);

    m_ui->m_uiModeCombo->addItem(tr("Docked Window"), QVariant(DockedMode));
    m_ui->m_uiModeCombo->addItem(tr("Multiple Top-Level Windows"), QVariant(TopLevelMode));
    connect(m_ui->m_uiModeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUiModeComboChanged()));

    m_ui->m_fontPanel->setCheckable(true);
    m_ui->m_fontPanel->setTitle(tr("Toolwindow Font"));

}

QDesignerAppearanceOptionsWidget::~QDesignerAppearanceOptionsWidget()
{
    delete m_ui;
}

UIMode QDesignerAppearanceOptionsWidget::uiMode() const
{
    return static_cast<UIMode>(m_ui->m_uiModeCombo->itemData(m_ui->m_uiModeCombo->currentIndex()).toInt());
}

AppearanceOptions QDesignerAppearanceOptionsWidget::appearanceOptions() const
{
    AppearanceOptions rc;
    rc.uiMode = uiMode();
    rc.toolWindowFontSettings.m_font = m_ui->m_fontPanel->selectedFont();
    rc.toolWindowFontSettings.m_useFont = m_ui->m_fontPanel->isChecked();
    rc.toolWindowFontSettings.m_writingSystem = m_ui->m_fontPanel->writingSystem();
    return rc;
}

void QDesignerAppearanceOptionsWidget::setAppearanceOptions(const AppearanceOptions &ao)
{
    m_initialUIMode = ao.uiMode;
    m_ui->m_uiModeCombo->setCurrentIndex(m_ui->m_uiModeCombo->findData(QVariant(ao.uiMode)));
    m_ui->m_fontPanel->setWritingSystem(ao.toolWindowFontSettings.m_writingSystem);
    m_ui->m_fontPanel->setSelectedFont(ao.toolWindowFontSettings.m_font);
    m_ui->m_fontPanel->setChecked(ao.toolWindowFontSettings.m_useFont);
}

void QDesignerAppearanceOptionsWidget::slotUiModeComboChanged()
{
    emit uiModeChanged(m_initialUIMode != uiMode());
}

// ----------- QDesignerAppearanceOptionsPage
QDesignerAppearanceOptionsPage::QDesignerAppearanceOptionsPage(QDesignerFormEditorInterface *core) :
    m_core(core)
{
}

QString QDesignerAppearanceOptionsPage::name() const
{
    //: Tab in preferences dialog
    return QCoreApplication::translate("QDesignerAppearanceOptionsPage", "Appearance");
}

QWidget *QDesignerAppearanceOptionsPage::createPage(QWidget *parent)
{
    m_widget = new QDesignerAppearanceOptionsWidget(parent);
    m_initialOptions.fromSettings(QDesignerSettings(m_core));
    m_widget->setAppearanceOptions(m_initialOptions);
    return m_widget;
}

void QDesignerAppearanceOptionsPage::apply()
{
    if (m_widget) {
        const AppearanceOptions newOptions = m_widget->appearanceOptions();
        if (newOptions != m_initialOptions) {
            QDesignerSettings settings(m_core);
            newOptions.toSettings(settings);
            QTimer::singleShot(0, this, SIGNAL(settingsChangedDelayed()));
            m_initialOptions = newOptions;
        }
    }
}

void QDesignerAppearanceOptionsPage::finish()
{
}

QT_END_NAMESPACE

