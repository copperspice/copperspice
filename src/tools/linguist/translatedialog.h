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

#ifndef TRANSLATEDIALOG_H
#define TRANSLATEDIALOG_H

#include "ui_translatedialog.h"
#include <QDialog>

QT_BEGIN_NAMESPACE

class TranslateDialog : public QDialog
{
   Q_OBJECT

 public:
   enum {
      Skip,
      Translate,
      TranslateAll
   };

   TranslateDialog(QWidget *parent = nullptr);

   bool markFinished() const {
      return m_ui.ckMarkFinished->isChecked();
   }
   Qt::CaseSensitivity caseSensitivity() const {
      return m_ui.ckMatchCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
   }
   QString findText() const {
      return m_ui.ledFindWhat->text();
   }
   QString replaceText() const {
      return m_ui.ledTranslateTo->text();
   }

 signals:
   void requestMatchUpdate(bool &hit);
   void activated(int mode);

 protected:
   virtual void showEvent(QShowEvent *event);

 private slots:
   void emitFindNext();
   void emitTranslateAndFindNext();
   void emitTranslateAll();
   void verifyText();

 private:
   Ui::TranslateDialog m_ui;
};


QT_END_NAMESPACE
#endif  //TRANSLATEDIALOG_H

