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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE

class QPushButton;
class QDesignerFormEditorInterface;
class QDesignerOptionsPageInterface;

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog: public QDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QDesignerFormEditorInterface *core, QWidget *parentWidget = 0);
    ~PreferencesDialog();


private slots:
    void slotAccepted();
    void slotRejected();
    void slotApply();
    void slotUiModeChanged(bool modified);

private:
    QPushButton *applyButton() const;
    void closeOptionPages();

    Ui::PreferencesDialog *m_ui;
    QDesignerFormEditorInterface *m_core;
    QList<QDesignerOptionsPageInterface*> m_optionsPages;
};

QT_END_NAMESPACE

#endif // PREFERENCESDIALOG_H
