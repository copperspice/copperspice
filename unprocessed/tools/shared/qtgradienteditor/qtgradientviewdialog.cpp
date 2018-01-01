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

#include "qtgradientviewdialog.h"
#include "qtgradientmanager.h"
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

QtGradientViewDialog::QtGradientViewDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(m_ui.gradientView, SIGNAL(currentGradientChanged(QString)),
            this, SLOT(slotGradientSelected(QString)));
    connect(m_ui.gradientView, SIGNAL(gradientActivated(QString)),
            this, SLOT(slotGradientActivated(QString)));
}

void QtGradientViewDialog::setGradientManager(QtGradientManager *manager)
{
    m_ui.gradientView->setGradientManager(manager);
}

QGradient QtGradientViewDialog::getGradient(bool *ok, QtGradientManager *manager, QWidget *parent, const QString &caption)
{
    QtGradientViewDialog dlg(parent);
    dlg.setGradientManager(manager);
    dlg.setWindowTitle(caption);
    QGradient grad = QLinearGradient();
    const int res = dlg.exec();
    if (res == QDialog::Accepted)
        grad = dlg.m_ui.gradientView->gradientManager()->gradients().value(dlg.m_ui.gradientView->currentGradient());
    if (ok)
        *ok = res == QDialog::Accepted;
    return grad;
}

void QtGradientViewDialog::slotGradientSelected(const QString &id)
{
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!id.isEmpty());
}

void QtGradientViewDialog::slotGradientActivated(const QString &id)
{
    Q_UNUSED(id)
    accept();
}

QT_END_NAMESPACE
