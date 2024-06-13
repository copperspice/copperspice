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

#include <phraseview.h>

#include <globals.h>
#include <mainwindow.h>
#include <messagemodel.h>
#include <phrase.h>
#include <phrasemodel.h>
#include <similartext.h>

#include <qalgorithms.h>
#include <qheaderview.h>
#include <qkeyevent.h>
#include <qsettings.h>
#include <qtreeview.h>
#include <qwidget.h>
#include <qdebug.h>

// Maximum number of guesses to display
static const int MaxCandidates = 5;

static QString phraseViewHeaderKey()
{
   return settingPath("PhraseViewHeader");
}

PhraseView::PhraseView(MultiDataModel *model, QList<QHash<QString, QList<Phrase *> > > *phraseDict, QWidget *parent)
   : QTreeView(parent), m_dataModel(model), m_phraseDict(phraseDict), m_modelIndex(-1), m_doGuesses(true)
{
   setObjectName("phrase list view");

   m_phraseModel = new PhraseModel(this);

   setModel(m_phraseModel);
   setAlternatingRowColors(true);
   setSelectionBehavior(QAbstractItemView::SelectRows);
   setSelectionMode(QAbstractItemView::SingleSelection);
   setRootIsDecorated(false);
   setItemsExpandable(false);

   for (int id = 0; id < 10; ++id) {
      // user pressed ctrl + n, apply that translation
      QShortcut *hotKey = new QShortcut( Qt::ControlModifier + (Qt::Key_1 + id), this);
      connect(hotKey, &QShortcut::activated, this, [this, id] () { guessShortcut(id); });
   }

   header()->setSectionResizeMode(QHeaderView::Interactive);
   header()->setSectionsClickable(true);
   header()->restoreState(QSettings().value(phraseViewHeaderKey()).toByteArray());

   connect(this, &PhraseView::activated, this, &PhraseView::selectPhrase);
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
   if (! index.isValid()) {
      return;
   }

   QMenu *contextMenu = new QMenu(this);

   QAction *insertAction = new QAction(tr("Insert"), contextMenu);
   connect(insertAction, &QAction::triggered, this, &PhraseView::selectCurrentPhrase);

   QAction *editAction = new QAction(tr("Edit"), contextMenu);
   connect(editAction,   &QAction::triggered, this, &PhraseView::editPhrase);
   editAction->setEnabled(model()->flags(index) & Qt::ItemIsEditable);

   contextMenu->addAction(insertAction);
   contextMenu->addAction(editAction);

   contextMenu->exec(event->globalPos());
   event->accept();
}

void PhraseView::mouseDoubleClickEvent(QMouseEvent *event)
{
   QModelIndex index = indexAt(event->pos());
   if (! index.isValid()) {
      return;
   }

   emit phraseSelected(m_modelIndex, m_phraseModel->phrase(index)->target());
   event->accept();
}

void PhraseView::guessShortcut(int key)
{
   for (const Phrase * phrase : m_phraseModel->phraseList()) {
      if (phrase->shortcut() == key) {
         emit phraseSelected(m_modelIndex, phrase->target());
         return;
      }
   }
}

void PhraseView::selectPhrase(const QModelIndex &index)
{
   emit phraseSelected(m_modelIndex, m_phraseModel->phrase(index)->target());
}

void PhraseView::selectCurrentPhrase()
{
   emit phraseSelected(m_modelIndex, m_phraseModel->phrase(currentIndex())->target());
}

void PhraseView::editPhrase()
{
   edit(currentIndex());
}

static QList<Candidate> similarTextHeuristicCandidates(MultiDataModel *model, int mi,
      const QString &text, int maxCandidates)
{
   QList<int> scores;
   QList<Candidate> candidates;

   StringSimilarityMatcher stringmatcher(text);

   for (MultiDataModelIterator it(model, mi); it.isValid(); ++it) {
      MessageItem *m = it.current();

      if (! m) {
         continue;
      }

      TranslatorMessage mtm = m->message();
      if (mtm.type() == TranslatorMessage::Type::Unfinished || mtm.translation().isEmpty()) {
         continue;
      }

      QString s = m->text();

      int score = stringmatcher.getSimilarityScore(s);

      if (candidates.count() == maxCandidates && score > scores[maxCandidates - 1]) {
         candidates.removeLast();
      }

      bool done = false;

      if (candidates.count() < maxCandidates && score >= textSimilarityThreshold ) {
         Candidate cand(s, mtm.translation());

         int i;
         for (i = 0; i < candidates.size(); ++i) {
            if (score >= scores.at(i)) {

               if (score == scores.at(i)) {
                  if (candidates.at(i) == cand) {
                     done = true;
                     break;
                  }

               } else {
                  break;
               }
            }
         }

         if (done) {
            continue;
         }

         scores.insert(i, score);
         candidates.insert(i, cand);
      }
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

   if (! sourceText.isEmpty() && m_doGuesses) {
      QList<Candidate> cl = similarTextHeuristicCandidates(m_dataModel, model, sourceText, MaxCandidates);

      int n = 0;
      for (const Candidate &item : cl) {
         QString def;

         if (n < 9) {
            def = tr("Guess (%1)").formatArg( QKeySequence(Qt::ControlModifier | (Qt::Key_0 + (n + 1))).toString() );
         } else {
            def = tr("Guess");
         }

         Phrase *guess = new Phrase(item.source, item.target, def, n);
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
   QStringList lookupWords = f.split(' ');

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
