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

#ifndef SAVEFORMASTEMPLATE_H
#define SAVEFORMASTEMPLATE_H

#include "ui_saveformastemplate.h"

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class SaveFormAsTemplate: public QDialog
{
    Q_OBJECT
public:
    explicit SaveFormAsTemplate(QDesignerFormEditorInterface *m_core,
                                QDesignerFormWindowInterface *formWindow,
                                QWidget *parent = 0);
    virtual ~SaveFormAsTemplate();

private slots:
    void accept();
    void updateOKButton(const QString &str);
    void checkToAddPath(int itemIndex);

private:
    static QString chooseTemplatePath(QWidget *parent);

    Ui::SaveFormAsTemplate ui;
    QDesignerFormEditorInterface *m_core;
    QDesignerFormWindowInterface *m_formWindow;
    int m_addPathIndex;
};

QT_END_NAMESPACE

#endif // SAVEFORMASTEMPLATE_H
