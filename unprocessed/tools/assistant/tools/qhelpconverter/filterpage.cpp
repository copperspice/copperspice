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

#include <QtGui/QMessageBox>
#include "filterpage.h"

QT_BEGIN_NAMESPACE

FilterPage::FilterPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Filter Settings"));
    setSubTitle(tr("Specify the filter attributes for the "
        "documentation. If filter attributes are used, "
        "also define a custom filter for it. Both the "
        "filter attributes and the custom filters are "
        "optional."));

    m_ui.setupUi(this);
    m_ui.customFilterWidget->headerItem()->setText(0, tr("Filter Name"));
    m_ui.customFilterWidget->headerItem()->setText(1, tr("Filter Attributes"));
    m_ui.customFilterWidget->setRootIsDecorated(false);
    m_ui.removeButton->setDisabled(true);
    connect(m_ui.addButton, SIGNAL(clicked()),
        this, SLOT(addFilter()));
    connect(m_ui.removeButton, SIGNAL(clicked()),
        this, SLOT(removeFilter()));
}

bool FilterPage::validatePage()
{
    m_filterAttributes.clear();
    foreach (const QString &f, m_ui.filterLineEdit->text().split(QLatin1Char(','))) {
        if (!f.trimmed().isEmpty())
            m_filterAttributes.append(f.trimmed());
    }

    m_customFilters.clear();
    QSet<QString> names;
    QSet<QString> atts;
    QString str;
    CustomFilter customFilter;
    QTreeWidgetItem *item = 0;
    for (int i=0; i<m_ui.customFilterWidget->topLevelItemCount(); ++i) {
        item = m_ui.customFilterWidget->topLevelItem(i);
        str = item->text(0);
        if (str.isEmpty() || names.contains(str)) {
            QMessageBox::critical(this, tr("Custom Filters"),
                tr("The custom filter \'%1\' is defined multiple times.")
                .arg(str));
            return false;
        }
        names.insert(str);
        customFilter.name = str;

        str.clear();
        QStringList lst;
        foreach (const QString &s, item->text(1).split(QLatin1Char(','))) {
            const QString st = s.trimmed();
            if (!st.isEmpty()) {
                str += QLatin1Char(',') + st;
                lst.append(st);
            }
        }
        if (atts.contains(str)) {
            QMessageBox::critical(this, tr("Custom Filters"),
                tr("The attributes for custom filter \'%1\' are defined multiple times.")
                .arg(customFilter.name));
            return false;
        }
        atts.insert(str);
        customFilter.filterAttributes = lst;
        m_customFilters.append(customFilter);
    }
    return true;
}

QStringList FilterPage::filterAttributes() const
{
    return m_filterAttributes;
}

QList<CustomFilter> FilterPage::customFilters() const
{
    return m_customFilters;
}

void FilterPage::addFilter()
{
    QTreeWidgetItem *item = new QTreeWidgetItem(m_ui.customFilterWidget);
    item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsEditable|Qt::ItemIsSelectable);
    item->setText(0, tr("unfiltered", "list of available documentation"));
    item->setText(1, QLatin1String(""));
    m_ui.customFilterWidget->editItem(item, 0);
    m_ui.removeButton->setDisabled(false);
}

void FilterPage::removeFilter()
{
    QModelIndex idx = m_ui.customFilterWidget->currentIndex();
    if (!idx.isValid())
        return;
    QTreeWidgetItem *item = m_ui.customFilterWidget->takeTopLevelItem(idx.row());
    delete item;
    if (!m_ui.customFilterWidget->topLevelItemCount())
        m_ui.removeButton->setDisabled(true);
}

QT_END_NAMESPACE
