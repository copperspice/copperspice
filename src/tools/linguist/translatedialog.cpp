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

#include "translatedialog.h"

QT_BEGIN_NAMESPACE

TranslateDialog::TranslateDialog(QWidget *parent)
   : QDialog(parent)
{
   m_ui.setupUi(this);
   connect(m_ui.findNxt, SIGNAL(clicked()), this, SLOT(emitFindNext()));
   connect(m_ui.translate, SIGNAL(clicked()), this, SLOT(emitTranslateAndFindNext()));
   connect(m_ui.translateAll, SIGNAL(clicked()), this, SLOT(emitTranslateAll()));
   connect(m_ui.ledFindWhat, SIGNAL(textChanged(QString)), SLOT(verifyText()));
   connect(m_ui.ckMatchCase, SIGNAL(toggled(bool)), SLOT(verifyText()));
}

void TranslateDialog::showEvent(QShowEvent *)
{
   verifyText();
   m_ui.ledFindWhat->setFocus();
}

void TranslateDialog::verifyText()
{
   QString text = m_ui.ledFindWhat->text();
   bool canFind = !text.isEmpty();
   bool hit = false;
   if (canFind) {
      emit requestMatchUpdate(hit);
   }
   m_ui.findNxt->setEnabled(canFind);
   m_ui.translate->setEnabled(canFind && hit);
   m_ui.translateAll->setEnabled(canFind);
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

QT_END_NAMESPACE
