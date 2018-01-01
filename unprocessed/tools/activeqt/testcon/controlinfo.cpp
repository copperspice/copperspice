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

#include "controlinfo.h"

#include <QtGui>

QT_BEGIN_NAMESPACE

ControlInfo::ControlInfo(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    listInfo->setColumnCount(2);
    listInfo->headerItem()->setText(0, tr("Item"));
    listInfo->headerItem()->setText(1, tr("Details"));
}

void ControlInfo::setControl(QWidget *activex)
{
    listInfo->clear();

    const QMetaObject *mo = activex->metaObject();
    QTreeWidgetItem *group = new QTreeWidgetItem(listInfo);
    group->setText(0, tr("Class Info"));
    group->setText(1, QString::number(mo->classInfoCount()));

    QTreeWidgetItem *item = 0;
    int i;
    int count;
    for (i = mo->classInfoOffset(); i < mo->classInfoCount(); ++i) {
	const QMetaClassInfo info = mo->classInfo(i);
	item = new QTreeWidgetItem(group);
        item->setText(0, QString::fromLatin1(info.name()));
        item->setText(1, QString::fromLatin1(info.value()));
    }
    group = new QTreeWidgetItem(listInfo);
    group->setText(0, tr("Signals"));

    count = 0;
    for (i = mo->methodOffset(); i < mo->methodCount(); ++i) {
	const QMetaMethod method = mo->method(i);
        if (method.methodType() == QMetaMethod::Signal) {
            ++count;
	    item = new QTreeWidgetItem(group);
            item->setText(0, QString::fromLatin1(method.signature()));
        }
    }
    group->setText(1, QString::number(count));

    group = new QTreeWidgetItem(listInfo);
    group->setText(0, tr("Slots"));

    count = 0;
    for (i = mo->methodOffset(); i < mo->methodCount(); ++i) {
	const QMetaMethod method = mo->method(i);
        if (method.methodType() == QMetaMethod::Slot) {
            ++count;
	    item = new QTreeWidgetItem(group);
            item->setText(0, QString::fromLatin1(method.signature()));
        }
    }
    group->setText(1, QString::number(count));

    group = new QTreeWidgetItem(listInfo);
    group->setText(0, tr("Properties"));

    count = 0;
    for (i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
        ++count;
	const QMetaProperty property = mo->property(i);
	item = new QTreeWidgetItem(group);
        item->setText(0, QString::fromLatin1(property.name()));
        item->setText(1, QString::fromLatin1(property.typeName()));
        if (!property.isDesignable()) {
            item->setTextColor(0, Qt::gray);
            item->setTextColor(1, Qt::gray);
        }
    }
    group->setText(1, QString::number(count));
}

QT_END_NAMESPACE
