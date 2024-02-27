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

#ifndef PHRASEVIEW_H
#define PHRASEVIEW_H

#include <phrase.h>

#include <qlist.h>
#include <qshortcut.h>
#include <qtreeview.h>

class MultiDataModel;
class PhraseModel;

class PhraseView : public QTreeView
{
   CS_OBJECT(PhraseView)

 public:
   PhraseView(MultiDataModel *model, QList<QHash<QString, QList<Phrase *>>> *phraseDict, QWidget *parent = nullptr);
   ~PhraseView();

   void setSourceText(int model, const QString &sourceText);

   CS_SLOT_1(Public, void toggleGuessing())
   CS_SLOT_2(toggleGuessing)

   CS_SLOT_1(Public, void update())
   CS_SLOT_2(update)

   CS_SIGNAL_1(Public, void phraseSelected(int latestModel,const QString & phrase))
   CS_SIGNAL_2(phraseSelected,latestModel,phrase)

 protected:
   // QObject
   void contextMenuEvent(QContextMenuEvent *event) override;

   // QAbstractItemView
   void mouseDoubleClickEvent(QMouseEvent *event) override;

 private:
   // slots
   void guessShortcut(int id);
   void selectPhrase(const QModelIndex & index);
   void selectCurrentPhrase();
   void editPhrase();

   QList<Phrase *> getPhrases(int model, const QString &sourceText);
   void deleteGuesses();

   MultiDataModel *m_dataModel;
   QList<QHash<QString, QList<Phrase *> > > *m_phraseDict;
   QList<Phrase *> m_guesses;
   PhraseModel *m_phraseModel;
   QString m_sourceText;
   int m_modelIndex;
   bool m_doGuesses;
};

#endif
