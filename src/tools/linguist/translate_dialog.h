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

#ifndef TRANSLATE_DIALOG_H
#define TRANSLATE_DIALOG_H

#include <ui_translate_dialog.h>

#include <qdialog.h>

class TranslateDialog : public QDialog
{
   CS_OBJECT(TranslateDialog)

 public:
   enum {
      Skip,
      Translate,
      TranslateAll
   };

   TranslateDialog(QWidget *parent = nullptr);
   ~TranslateDialog();

   bool markFinished() const {
      return m_ui->ckMarkFinished->isChecked();
   }

   Qt::CaseSensitivity caseSensitivity() const {
      return m_ui->ckMatchCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
   }

   QString findText() const {
      return m_ui->ledFindWhat->text();
   }

   QString replaceText() const {
      return m_ui->ledTranslateTo->text();
   }

   CS_SIGNAL_1(Public, void requestMatchUpdate(bool &hit))
   CS_SIGNAL_2(requestMatchUpdate,hit)

   CS_SIGNAL_1(Public, void activated(int mode))
   CS_SIGNAL_2(activated,mode)

 protected:
   void showEvent(QShowEvent *event) override;

 private:
   Ui::TranslateDialog *m_ui;

   CS_SLOT_1(Private, void emitFindNext())
   CS_SLOT_2(emitFindNext)

   CS_SLOT_1(Private, void emitTranslateAndFindNext())
   CS_SLOT_2(emitTranslateAndFindNext)

   CS_SLOT_1(Private, void emitTranslateAll())
   CS_SLOT_2(emitTranslateAll)

   CS_SLOT_1(Private, void verifyText())
   CS_SLOT_2(verifyText)

};

#endif

