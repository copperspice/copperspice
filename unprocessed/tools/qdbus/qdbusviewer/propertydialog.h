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

#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include <QLabel>
#include <QDialog>
#include <QTableWidget>
#include <QDialogButtonBox>

class PropertyDialog: public QDialog
{
    Q_OBJECT
public:
    explicit PropertyDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

    void addProperty(const QString &name, QVariant::Type type);
    void setInfo(const QString &caption);

    QList<QVariant> values() const;

    int exec();

private:
    QLabel *label;
    QTableWidget *propertyTable;
    QDialogButtonBox *buttonBox;
};

#endif

