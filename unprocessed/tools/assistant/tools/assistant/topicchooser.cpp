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

#include "tracer.h"

#include "topicchooser.h"

QT_BEGIN_NAMESPACE

TopicChooser::TopicChooser(QWidget *parent, const QString &keyword,
                         const QMap<QString, QUrl> &links)
    : QDialog(parent)
{
    TRACE_OBJ
    ui.setupUi(this);
    ui.label->setText(tr("Choose a topic for <b>%1</b>:").arg(keyword));

    QMap<QString, QUrl>::const_iterator it = links.constBegin();
    for (; it != links.constEnd(); ++it) {
        ui.listWidget->addItem(it.key());
        m_links.append(it.value());
    }
    
    if (ui.listWidget->count() != 0)
        ui.listWidget->setCurrentRow(0);
    ui.listWidget->setFocus();

    connect(ui.buttonDisplay, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui.buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui.listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this,
        SLOT(accept()));
}

QUrl TopicChooser::link() const
{
    TRACE_OBJ
    QListWidgetItem *item = ui.listWidget->currentItem();
    if (!item)
        return QUrl();

    QString title = item->text();
    if (title.isEmpty())
        return QUrl();

    const int row = ui.listWidget->row(item);
    Q_ASSERT(row < m_links.count());
    return m_links.at(row);
}

QT_END_NAMESPACE
