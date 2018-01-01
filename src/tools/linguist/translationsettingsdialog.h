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

#ifndef TRANSLATIONSETTINGSDIALOG_H
#define TRANSLATIONSETTINGSDIALOG_H

#include "ui_translationsettings.h"

#include <QtCore/QLocale>
#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE

class DataModel;
class PhraseBook;

class TranslationSettingsDialog : public QDialog
{
   Q_OBJECT

 public:
   TranslationSettingsDialog(QWidget *parent = nullptr);
   void setDataModel(DataModel *model);
   void setPhraseBook(PhraseBook *phraseBook);

 private:
   virtual void showEvent(QShowEvent *e);

 private slots:
   void on_buttonBox_accepted();
   void on_srcCbLanguageList_currentIndexChanged(int idx);
   void on_tgtCbLanguageList_currentIndexChanged(int idx);

 private:
   Ui::TranslationSettingsDialog m_ui;
   DataModel *m_dataModel;
   PhraseBook *m_phraseBook;

};

QT_END_NAMESPACE

#endif // TRANSLATIONSETTINGSDIALOG_H
