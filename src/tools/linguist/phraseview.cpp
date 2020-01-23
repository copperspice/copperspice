/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "globals.h"
#include "mainwindow.h"
#include "messagemodel.h"
#include "phrase.h"
#include "phraseview.h"
#include "phrasemodel.h"
#include "simtexth.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QSettings>
#include <QTreeView>
#include <QWidget>
#include <QDebug>

// Maximum number of guesses to display
static const int MaxCandidates = 5;

static QString phraseViewHeaderKey()
{
   return settingPath("PhraseViewHeader");
}

PhraseView::PhraseView(MultiDataModel *model, QList<QHash<QString, QList<Phrase *> > > *phraseDict, QWidget *parent)
   : QTreeView(parent),
     m_dataModel(model),
     m_phraseDict(phraseDict),
     m_modelIndex(-1),
     m_doGuesses(true)
{
   setObjectName(QLatin1String("phrase list view"));

   m_phraseModel = new PhraseModel(this);

   setModel(m_phraseModel);
   setAlternatingRowColors(true);
   setSelectionBehavior(QAbstractItemView::SelectRows);
   setSelectionMode(QAbstractItemView::SingleSelection);
   setRootIsDecorated(false);
   setItemsExpandable(false);

   for (int i = 0; i < 10; i++) {
      (void) new GuessShortcut(i, this, SLOT(guessShortcut(int)));
   }

   header()->setResizeMode(QHeaderView::Interactive);
   header()->setClickable(true);
   header()->restoreState(QSettings().value(phraseViewHeaderKey()).toByteArray());

   connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(selectPhrase(QModelIndex)));
}

PhraseView::~PhraseView()
{
   QSettings().setValue(phraseViewHeaderKey(), header()->saveState());
   deleteGuesses();
}

void PhraseView::toggleGuessing()
{
   m_doGuesses = !m_doGuesses;
   update();
}

void PhraseView::update()
{
   setSourceText(m_modelIndex, m_sourceText);
}


void PhraseView::contextMenuEvent(QContextMenuEvent *event)
{
   QModelIndex index = indexAt(event->pos());
   if (!index.isValid()) {
      return;
   }

   QMenu *contextMenu = new QMenu(this);

   QAction *insertAction = new QAction(tr("Insert"), contextMenu);
   connect(insertAction, SIGNAL(triggered()), this, SLOT(selectPhrase()));

   QAction *editAction = new QAction(tr("Edit"), contextMenu);
   connect(editAction, SIGNAL(triggered()), this, SLOT(editPhrase()));
   editAction->setEnabled(model()->flags(index) & Qt::ItemIsEditable);

   contextMenu->addAction(insertAction);
   contextMenu->addAction(editAction);

   contextMenu->exec(event->globalPos());
   event->accept();
}

void PhraseView::mouseDoubleClickEvent(QMouseEvent *event)
{
   QModelIndex index = indexAt(event->pos());
   if (!index.isValid()) {
      return;
   }

   emit phraseSelected(m_modelIndex, m_phraseModel->phrase(index)->target());
   event->accept();
}

void PhraseView::guessShortcut(int key)
{
   for (const Phrase * phrase : m_phraseModel->phraseList())
   if (phrase->shortcut() == key) {
      emit phraseSelected(m_modelIndex, phrase->target());
      return;
   }
}

void PhraseView::selectPhrase(const QModelIndex &index)
{
   emit phraseSelected(m_modelIndex, m_phraseModel->phrase(index)->target());
}

void PhraseView::selectPhrase()
{
   emit phraseSelected(m_modelIndex, m_phraseModel->phrase(currentIndex())->target());
}

void PhraseView::editPhrase()
{
   edit(currentIndex());
}

static CandidateList similarTextHeuristicCandidates(MultiDataModel *model, int mi,
      const char *text, int maxCandidates)
{
   QList<int> scores;
   CandidateList candidates;

   StringSimilarityMatcher stringmatcher(QString::fromLatin1(text));

   for (MultiDataModelIterator it(model, mi); it.isValid(); ++it) {
      MessageItem *m = it.current();
      if (!m) {
         continue;
      }

      TranslatorMessage mtm = m->message();
      if (mtm.type() == TranslatorMessage::Unfinished
            || mtm.translation().isEmpty()) {
         continue;
      }

      QString s = m->text();

      int score = stringmatcher.getSimilarityScore(s);

      if (candidates.count() == maxCandidates && score > scores[maxCandidates - 1]) {
         candidates.removeLast();
      }
      if (candidates.count() < maxCandidates && score >= textSimilarityThreshold ) {
         Candidate cand(s, mtm.translation());

         int i;
         for (i = 0; i < candidates.size(); ++i) {
            if (score >= scores.at(i)) {
               if (score == scores.at(i)) {
                  if (candidates.at(i) == cand) {
                     goto continue_outer_loop;
                  }
               } else {
                  break;
               }
            }
         }
         scores.insert(i, score);
         candidates.insert(i, cand);
      }
   continue_outer_loop:
      ;
   }
   return candidates;
}


void PhraseView::setSourceText(int model, const QString &sourceText)
{
   m_modelIndex = model;
   m_sourceText = sourceText;
   m_phraseModel->removePhrases();
   deleteGuesses();

   if (model < 0) {
      return;
   }

   for (Phrase * p : getPhrases(model, sourceText))
   m_phraseModel->addPhrase(p);

   if (!sourceText.isEmpty() && m_doGuesses) {
      CandidateList cl = similarTextHeuristicCandidates(m_dataModel, model,
                         sourceText.toLatin1(), MaxCandidates);
      int n = 0;
      for (const Candidate & candidate : cl) {
         QString def;
         if (n < 9) {
            def = tr("Guess (%1)").arg(QString(QKeySequence(Qt::CTRL | (Qt::Key_0 + (n + 1)))));
         } else {
            def = tr("Guess");
         }
         Phrase *guess = new Phrase(candidate.source, candidate.target, def, n);
         m_guesses.append(guess);
         m_phraseModel->addPhrase(guess);
         ++n;
      }
   }
}

QList<Phrase *> PhraseView::getPhrases(int model, const QString &source)
{
   QList<Phrase *> phrases;
   QString f = MainWindow::friendlyString(source);
   QStringList lookupWords = f.split(QLatin1Char(' '));

   for (const QString & s : lookupWords) {
      if (m_phraseDict->at(model).contains(s)) {
         for (Phrase * p : m_phraseDict->at(model).value(s)) {
            if (f.contains(MainWindow::friendlyString(p->source()))) {
               phrases.append(p);
            }
         }
      }
   }
   return phrases;
}

void PhraseView::deleteGuesses()
{
   qDeleteAll(m_guesses);
   m_guesses.clear();
}
