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

/*  TRANSLATOR FindDialog

    Choose Edit|Find from the menu bar or press Ctrl+F to pop up the
    Find dialog
*/

#include "finddialog.h"

QT_BEGIN_NAMESPACE

FindDialog::FindDialog(QWidget *parent)
   : QDialog(parent)
{
   setupUi(this);

   findNxt->setEnabled(false);

   connect(findNxt, SIGNAL(clicked()), this, SLOT(emitFindNext()));
   connect(led, SIGNAL(textChanged(QString)), this, SLOT(verifyText(QString)));

   led->setFocus();
}

void FindDialog::verifyText(const QString &text)
{
   findNxt->setEnabled(!text.isEmpty());
}

void FindDialog::emitFindNext()
{
   DataModel::FindLocation where;
   if (sourceText != 0)
      where =
         DataModel::FindLocation(
            (sourceText->isChecked() ? DataModel::SourceText : 0) |
            (translations->isChecked() ? DataModel::Translations : 0) |
            (comments->isChecked() ? DataModel::Comments : 0));
   else {
      where = DataModel::Translations;
   }
   emit findNext(led->text(), where, matchCase->isChecked(), ignoreAccelerators->isChecked());
   led->selectAll();
}

void FindDialog::find()
{
   led->setFocus();

   show();
   activateWindow();
   raise();
}

QT_END_NAMESPACE
