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

#ifndef CHANGEPROPERTIES_H
#define CHANGEPROPERTIES_H

#include <QtCore/qglobal.h>
#include "ui_changeproperties.h"

QT_BEGIN_NAMESPACE

class QAxWidget;

class ChangeProperties : public QDialog, Ui::ChangeProperties
{
    Q_OBJECT
public:
    ChangeProperties(QWidget *parent);

    void setControl(QAxWidget *control);

public slots:
    void updateProperties();

protected slots:
    void on_listProperties_currentItemChanged(QTreeWidgetItem *current);
    void on_listEditRequests_itemChanged(QTreeWidgetItem *item);
    void on_buttonSet_clicked();

private:
    QAxWidget *activex;
};

QT_END_NAMESPACE

#endif // CHANGEPROPERTIES_H
