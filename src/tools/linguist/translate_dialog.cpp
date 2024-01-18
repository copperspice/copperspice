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

#include <translate_dialog.h>

TranslateDialog::TranslateDialog(QWidget *parent)
   : QDialog(parent), m_ui(new Ui::TranslateDialog)
{
   m_ui->setupUi(this);

   connect(m_ui->findNxt,      &QPushButton::clicked,   this, &TranslateDialog::emitFindNext);
   connect(m_ui->translate,    &QPushButton::clicked,   this, &TranslateDialog::emitTranslateAndFindNext);
   connect(m_ui->translateAll, &QPushButton::clicked,   this, &TranslateDialog::emitTranslateAll);
   connect(m_ui->ledFindWhat,  &QLineEdit::textChanged, this, &TranslateDialog::verifyText);
   connect(m_ui->ckMatchCase,  &QCheckBox::toggled,     this, &TranslateDialog::verifyText);
}

TranslateDialog::~TranslateDialog()
{
   delete m_ui;
}

void TranslateDialog::showEvent(QShowEvent *)
{
   verifyText();
   m_ui->ledFindWhat->setFocus();
}

void TranslateDialog::verifyText()
{
   QString str = m_ui->ledFindWhat->text();

   bool canFind = ! str.isEmpty();
   bool hit     = false;

   if (canFind) {
      emit requestMatchUpdate(hit);
   }

   m_ui->findNxt->setEnabled(canFind);
   m_ui->translate->setEnabled(canFind && hit);
   m_ui->translateAll->setEnabled(canFind);
}

void TranslateDialog::emitFindNext()
{
   emit activated(Skip);
}

void TranslateDialog::emitTranslateAndFindNext()
{
   emit activated(Translate);
}

void TranslateDialog::emitTranslateAll()
{
   emit activated(TranslateAll);
}

