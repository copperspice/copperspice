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

#ifndef PHRASEBOOKBOX_H
#define PHRASEBOOKBOX_H

#include "ui_phrasebookbox.h"
#include "phrase.h"
#include "phrasemodel.h"
#include <QDialog>

QT_BEGIN_NAMESPACE

class TranslationSettingsDialog;

class QSortFilterProxyModel;

class PhraseBookBox : public QDialog, public Ui::PhraseBookBox
{
   Q_OBJECT
 public:
   PhraseBookBox(PhraseBook *phraseBook, QWidget *parent = nullptr);

 protected:
   bool eventFilter(QObject *obj, QEvent *event);

 private slots:
   void newPhrase();
   void removePhrase();
   void settings();
   void save();
   void sourceChanged(const QString &source);
   void targetChanged(const QString &target);
   void definitionChanged(const QString &definition);
   void selectionChanged();

 private:
   void selectItem(const QModelIndex &index);
   void enableDisable();
   QModelIndex currentPhraseIndex() const;

   QString fn;
   PhraseBook *m_phraseBook;
   PhraseModel *phrMdl;
   QSortFilterProxyModel *m_sortedPhraseModel;
   TranslationSettingsDialog *m_translationSettingsDialog;
};

QT_END_NAMESPACE

#endif
