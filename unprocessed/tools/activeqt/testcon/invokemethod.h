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

#ifndef INVOKEMETHOD_H
#define INVOKEMETHOD_H

#include <QtCore/qglobal.h>
#include "ui_invokemethod.h"

QT_BEGIN_NAMESPACE

class QAxBase;

class InvokeMethod : public QDialog, Ui::InvokeMethod
{
    Q_OBJECT
public:
    InvokeMethod(QWidget *parent);

    void setControl(QAxBase *ax);

protected slots:
    void on_buttonInvoke_clicked();
    void on_buttonSet_clicked();

    void on_comboMethods_activated(const QString &method);
    void on_listParameters_currentItemChanged(QTreeWidgetItem *item);

private:
    QAxBase *activex;
};

QT_END_NAMESPACE

#endif // INVOKEMETHOD_H
