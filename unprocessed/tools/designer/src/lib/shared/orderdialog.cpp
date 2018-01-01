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

#include "orderdialog_p.h"
#include "iconloader_p.h"
#include "ui_orderdialog.h"

#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QModelIndex>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

// OrderDialog: Used to reorder the pages of QStackedWidget and QToolBox.
// Provides up and down buttons as well as  DnD via QAbstractItemView::InternalMove mode
namespace qdesigner_internal {

OrderDialog::OrderDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::OrderDialog),
    m_format(PageOrderFormat)
{
    m_ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    m_ui->upButton->setIcon(createIconSet(QString::fromUtf8("up.png")));
    m_ui->downButton->setIcon(createIconSet(QString::fromUtf8("down.png")));
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(slotReset()));
    // Catch the remove operation of a DnD operation in QAbstractItemView::InternalMove mode to enable buttons
    // Selection mode is 'contiguous' to enable DnD of groups
    connect(m_ui->pageList->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(slotEnableButtonsAfterDnD()));

    m_ui->upButton->setEnabled(false);
    m_ui->downButton->setEnabled(false);
}

OrderDialog::~OrderDialog()
{
    delete  m_ui;
}

void OrderDialog::setDescription(const QString &d)
{
     m_ui->groupBox->setTitle(d);
}

void OrderDialog::setPageList(const QWidgetList &pages)
{
    // The QWidget* are stored in a map indexed by the old index.
    // The old index is set as user data on the item instead of the QWidget*
    // because DnD is enabled which requires the user data to serializable
    m_orderMap.clear();
    const int count = pages.count();
    for (int i=0; i < count; ++i)
        m_orderMap.insert(i, pages.at(i));
    buildList();
}

void OrderDialog::buildList()
{
    m_ui->pageList->clear();
    const OrderMap::const_iterator cend = m_orderMap.constEnd();
    for (OrderMap::const_iterator it = m_orderMap.constBegin(); it != cend; ++it) {
        QListWidgetItem *item = new QListWidgetItem();
        const int index = it.key();
        switch (m_format) {
        case PageOrderFormat:
            item->setText(tr("Index %1 (%2)").arg(index).arg(it.value()->objectName()));
            break;
        case TabOrderFormat:
            item->setText(tr("%1 %2").arg(index+1).arg(it.value()->objectName()));
            break;
        }
        item->setData(Qt::UserRole, QVariant(index));
        m_ui->pageList->addItem(item);
    }

    if (m_ui->pageList->count() > 0)
        m_ui->pageList->setCurrentRow(0);
}

void OrderDialog::slotReset()
{
    buildList();
}

QWidgetList OrderDialog::pageList() const
{
    QWidgetList rc;
    const int count = m_ui->pageList->count();
    for (int i=0; i < count; ++i) {
        const int oldIndex = m_ui->pageList->item(i)->data(Qt::UserRole).toInt();
        rc.append(m_orderMap.value(oldIndex));
    }
    return rc;
}

void OrderDialog::on_upButton_clicked()
{
    const int row = m_ui->pageList->currentRow();
    if (row <= 0)
        return;

    m_ui->pageList->insertItem(row - 1, m_ui->pageList->takeItem(row));
    m_ui->pageList->setCurrentRow(row - 1);
}

void OrderDialog::on_downButton_clicked()
{
    const int row = m_ui->pageList->currentRow();
    if (row == -1 || row == m_ui->pageList->count() - 1)
        return;

    m_ui->pageList->insertItem(row + 1, m_ui->pageList->takeItem(row));
    m_ui->pageList->setCurrentRow(row + 1);
}

void OrderDialog::slotEnableButtonsAfterDnD()
{
    enableButtons(m_ui->pageList->currentRow());
}

void OrderDialog::on_pageList_currentRowChanged(int r)
{
    enableButtons(r);
}

void OrderDialog::enableButtons(int r)
{
    m_ui->upButton->setEnabled(r > 0);
    m_ui->downButton->setEnabled(r >= 0 && r < m_ui->pageList->count() - 1);
}

QWidgetList OrderDialog::pagesOfContainer(const QDesignerFormEditorInterface *core, QWidget *container)
{
    QWidgetList rc;
    if (QDesignerContainerExtension* ce = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), container)) {
        const int count = ce->count();
        for (int i = 0; i < count ;i ++)
            rc.push_back(ce->widget(i));
    }
    return rc;
}

}

QT_END_NAMESPACE
