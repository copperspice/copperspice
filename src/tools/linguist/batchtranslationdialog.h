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

#ifndef BATCHTRANSLATIONDIALOG_H
#define BATCHTRANSLATIONDIALOG_H

#include "ui_batchtranslation.h"
#include "phrase.h"

#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>

QT_BEGIN_NAMESPACE

class MultiDataModel;

class CheckableListModel : public QStandardItemModel
{
 public:
   CheckableListModel(QObject *parent = nullptr);
   virtual Qt::ItemFlags flags(const QModelIndex &index) const;
};

class BatchTranslationDialog : public QDialog
{
   Q_OBJECT
 public:
   BatchTranslationDialog(MultiDataModel *model, QWidget *w = 0);
   void setPhraseBooks(const QList<PhraseBook *> &phrasebooks, int modelIndex);

 signals:
   void finished();

 private slots:
   void startTranslation();
   void movePhraseBookUp();
   void movePhraseBookDown();

 private:
   Ui::BatchTranslationDialog m_ui;
   CheckableListModel m_model;
   MultiDataModel *m_dataModel;
   QList<PhraseBook *> m_phrasebooks;
   int m_modelIndex;
};

QT_END_NAMESPACE

#endif // BATCHTRANSLATIONDIALOG_H
