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

#ifndef BATCHTRANSLATION_DIALOG_H
#define BATCHTRANSLATION_DIALOG_H

#include <phrase.h>
#include <ui_batchtranslation.h>

#include <qdialog.h>
#include <qstandarditemmodel.h>

class MultiDataModel;

class CheckableListModel : public QStandardItemModel
{
 public:
   CheckableListModel(QObject *parent = nullptr);
   Qt::ItemFlags flags(const QModelIndex &index) const override;
};

class BatchTranslationDialog : public QDialog
{
   CS_OBJECT(BatchTranslationDialog)

 public:
   BatchTranslationDialog(MultiDataModel *model, QWidget *w = nullptr);
   ~BatchTranslationDialog();

   void setPhraseBooks(const QList<PhraseBook *> &phrasebooks, int modelIndex);

   CS_SIGNAL_1(Public, void finished())
   CS_SIGNAL_2(finished)

 private:
   Ui::BatchTranslationDialog *m_ui;

   CheckableListModel m_model;
   MultiDataModel *m_dataModel;
   QList<PhraseBook *> m_phrasebooks;
   int m_modelIndex;

   // slots
   void startTranslation();
   void movePhraseBookUp();
   void movePhraseBookDown();
};

#endif
