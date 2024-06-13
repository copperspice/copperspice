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

#ifndef FIND_DIALOG_H
#define FIND_DIALOG_H

#include <messagemodel.h>
#include <ui_find_dialog.h>

#include <qdialog.h>

class FindDialog : public QDialog
{
   CS_OBJECT(FindDialog)

 public:
   FindDialog(QWidget *parent = nullptr);
   ~FindDialog();

   CS_SIGNAL_1(Public, void findNext(const QString & text, DataModel::FindLocation where,
               bool matchCase, bool ignoreAccelerators, bool skipObsolete))
   CS_SIGNAL_2(findNext, text, where, matchCase, ignoreAccelerators, skipObsolete)

   // slot
   void find();

 private:
   Ui::FindDialog *m_ui;

   // slot
   void emitFindNext();
   void verifyText(const QString &text);
};

#endif
