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

#include "formeditor_optionspage.h"

// shared
#include "formwindowbase_p.h"
#include "gridpanel_p.h"
#include "grid_p.h"
#include "previewconfigurationwidget_p.h"
#include "shared_settings_p.h"
#include "zoomwidget_p.h"

// SDK
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowManagerInterface>

#include <QtCore/QString>
#include <QtCore/QCoreApplication>
#include <QtGui/QGroupBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QFormLayout>
#include <QtGui/QComboBox>

QT_BEGIN_NAMESPACE

typedef QList<int> IntList;

namespace qdesigner_internal {

// Zoom, currently for preview only
class ZoomSettingsWidget : public QGroupBox {
    Q_DISABLE_COPY(ZoomSettingsWidget)
public:
    explicit ZoomSettingsWidget(QWidget *parent = 0);

    void fromSettings(const QDesignerSharedSettings &s);
    void toSettings(QDesignerSharedSettings &s) const;

private:
    QComboBox *m_zoomCombo;
};

ZoomSettingsWidget::ZoomSettingsWidget(QWidget *parent) :
    QGroupBox(parent),
    m_zoomCombo(new QComboBox)
{
    m_zoomCombo->setEditable(false);
    const IntList zoomValues = ZoomMenu::zoomValues();
    const IntList::const_iterator cend = zoomValues.constEnd();

    for (IntList::const_iterator it = zoomValues.constBegin(); it != cend; ++it) {
        //: Zoom percentage
        m_zoomCombo->addItem(QCoreApplication::translate("FormEditorOptionsPage", "%1 %").arg(*it), QVariant(*it));
    }

    // Layout
    setCheckable(true);
    setTitle(QCoreApplication::translate("FormEditorOptionsPage", "Preview Zoom"));
    QFormLayout *lt = new QFormLayout;
    lt->addRow(QCoreApplication::translate("FormEditorOptionsPage", "Default Zoom"), m_zoomCombo);
    setLayout(lt);
}

void ZoomSettingsWidget::fromSettings(const QDesignerSharedSettings &s)
{
    setChecked(s.zoomEnabled());
    const int idx = m_zoomCombo->findData(QVariant(s.zoom()));
    m_zoomCombo->setCurrentIndex(qMax(0, idx));
}

void ZoomSettingsWidget::toSettings(QDesignerSharedSettings &s) const
{
    s.setZoomEnabled(isChecked());
    const int zoom = m_zoomCombo->itemData(m_zoomCombo->currentIndex()).toInt();
    s.setZoom(zoom);
}



// FormEditorOptionsPage:
FormEditorOptionsPage::FormEditorOptionsPage(QDesignerFormEditorInterface *core)
        : m_core(core)
{
}

QString FormEditorOptionsPage::name() const
{
    //: Tab in preferences dialog
    return QCoreApplication::translate("FormEditorOptionsPage", "Forms");
}

QWidget *FormEditorOptionsPage::createPage(QWidget *parent)
{
    QWidget *optionsWidget = new QWidget(parent);

    const QDesignerSharedSettings settings(m_core);
    m_previewConf = new PreviewConfigurationWidget(m_core);
    m_zoomSettingsWidget = new ZoomSettingsWidget;
    m_zoomSettingsWidget->fromSettings(settings);

    m_defaultGridConf = new GridPanel();
    m_defaultGridConf->setTitle(QCoreApplication::translate("FormEditorOptionsPage", "Default Grid"));
    m_defaultGridConf->setGrid(settings.defaultGrid());

    QVBoxLayout *optionsVLayout = new QVBoxLayout();
    optionsVLayout->addWidget(m_defaultGridConf);
    optionsVLayout->addWidget(m_previewConf);
    optionsVLayout->addWidget(m_zoomSettingsWidget);
    optionsVLayout->addStretch(1);

    // Outer layout to give it horizontal stretch
    QHBoxLayout *optionsHLayout = new QHBoxLayout();
    optionsHLayout->addLayout(optionsVLayout);
    optionsHLayout->addStretch(1);
    optionsWidget->setLayout(optionsHLayout);

    return optionsWidget;
}

void FormEditorOptionsPage::apply()
{
    QDesignerSharedSettings settings(m_core);
    if (m_defaultGridConf) {
        const Grid defaultGrid = m_defaultGridConf->grid();
        settings.setDefaultGrid(defaultGrid);

        FormWindowBase::setDefaultDesignerGrid(defaultGrid);
        // Update grid settings in all existing form windows
        QDesignerFormWindowManagerInterface *fwm = m_core->formWindowManager();
        if (const int numWindows = fwm->formWindowCount()) {
            for (int i = 0; i < numWindows; i++)
                if (qdesigner_internal::FormWindowBase *fwb
                        = qobject_cast<qdesigner_internal::FormWindowBase *>( fwm->formWindow(i)))
                    if (!fwb->hasFormGrid())
                        fwb->setDesignerGrid(defaultGrid);
        }
    }
    if (m_previewConf) {
        m_previewConf->saveState();
    }

    if (m_zoomSettingsWidget)
        m_zoomSettingsWidget->toSettings(settings);
}

void FormEditorOptionsPage::finish()
{
}

}

QT_END_NAMESPACE
