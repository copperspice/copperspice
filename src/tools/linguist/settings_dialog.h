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

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <ui_settings_dialog.h>

#include <qdialog.h>
#include <qlocale.h>

class DataModel;
class PhraseBook;

class SettingsDialog : public QDialog
{
   CS_OBJECT(SettingsDialog)

 public:
   SettingsDialog(QWidget *parent = nullptr);
   ~SettingsDialog();

   void setDataModel(DataModel *model);
   void setPhraseBook(PhraseBook *phraseBook);

 private:
   void showEvent(QShowEvent *e) override;
   Ui::SettingsDialog *m_ui;

   DataModel *m_dataModel;
   PhraseBook *m_phraseBook;

   CS_SLOT_1(Private, void on_buttonBox_accepted())
   CS_SLOT_2(on_buttonBox_accepted)

   CS_SLOT_1(Private, void on_srcCbLanguageList_currentIndexChanged(int idx))
   CS_SLOT_2(on_srcCbLanguageList_currentIndexChanged)

   CS_SLOT_1(Private, void on_tgtCbLanguageList_currentIndexChanged(int idx))
   CS_SLOT_2(on_tgtCbLanguageList_currentIndexChanged)
};

#endif
