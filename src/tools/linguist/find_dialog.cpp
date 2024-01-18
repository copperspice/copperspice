/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <find_dialog.h>

FindDialog::FindDialog(QWidget *parent)
   : QDialog(parent), m_ui(new Ui::FindDialog)
{
   m_ui->setupUi(this);

   m_ui->findNxt->setEnabled(false);

   connect(m_ui->findNxt, &QPushButton::clicked,   this, &FindDialog::emitFindNext);
   connect(m_ui->led,     &QLineEdit::textChanged, this, &FindDialog::verifyText);

   m_ui->led->setFocus();
}

FindDialog::~FindDialog()
{
   delete m_ui;
}

void FindDialog::verifyText(const QString &text)
{
   m_ui->findNxt->setEnabled(!text.isEmpty());
}

void FindDialog::emitFindNext()
{
   DataModel::FindLocation where;

   if (m_ui->sourceText == nullptr) {
      where = DataModel::Translations;

   } else {
      where = DataModel::FindLocation(
               (m_ui->sourceText->isChecked()   ? DataModel::SourceText   : 0) |
               (m_ui->translations->isChecked() ? DataModel::Translations : 0) |
               (m_ui->comments->isChecked()     ? DataModel::Comments     : 0));

   }

   emit findNext(m_ui->led->text(), where, m_ui->matchCase->isChecked(), m_ui->ignoreAccelerators->isChecked(), m_ui->skipObsolete->isChecked());
   m_ui->led->selectAll();
}

void FindDialog::find()
{
   m_ui->led->setFocus();

   show();
   activateWindow();
   raise();
}
