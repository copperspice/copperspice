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

#ifndef PHRASEBOOKBOX_H
#define PHRASEBOOKBOX_H

#include <phrase.h>
#include <phrasemodel.h>
#include <ui_phrasebookbox.h>

#include <qdialog.h>

class SettingsDialog;

class QSortFilterProxyModel;

class PhraseBookBox : public QDialog, public Ui::PhraseBookBox
{
   CS_OBJECT(PhraseBookBox)

 public:
   PhraseBookBox(PhraseBook *phraseBook, QWidget *parent = nullptr);

 protected:
   bool eventFilter(QObject *obj, QEvent *event) override;

 private:
   CS_SLOT_1(Private, void newPhrase())
   CS_SLOT_2(newPhrase)

   CS_SLOT_1(Private, void removePhrase())
   CS_SLOT_2(removePhrase)

   CS_SLOT_1(Private, void settings())
   CS_SLOT_2(settings)

   CS_SLOT_1(Private, void save())
   CS_SLOT_2(save)

   CS_SLOT_1(Private, void sourceChanged(const QString &source))
   CS_SLOT_2(sourceChanged)

   CS_SLOT_1(Private, void targetChanged(const QString &target))
   CS_SLOT_2(targetChanged)

   CS_SLOT_1(Private, void definitionChanged(const QString &definition))
   CS_SLOT_2(definitionChanged)

   CS_SLOT_1(Private, void selectionChanged())
   CS_SLOT_2(selectionChanged)

   void selectItem(const QModelIndex &index);
   void enableDisable();
   QModelIndex currentPhraseIndex() const;

   QString fn;
   PhraseBook *m_phraseBook;
   PhraseModel *phrMdl;
   QSortFilterProxyModel *m_sortedPhraseModel;
   SettingsDialog *m_settingsDialog;
};

#endif
