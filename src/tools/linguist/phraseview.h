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

#ifndef PHRASEVIEW_H
#define PHRASEVIEW_H

#include <QList>
#include <QShortcut>
#include <QTreeView>
#include "phrase.h"

QT_BEGIN_NAMESPACE

class MultiDataModel;
class PhraseModel;

class GuessShortcut : public QShortcut
{
   Q_OBJECT
 public:
   GuessShortcut(int nkey, QWidget *parent, const char *member)
      : QShortcut(parent), nrkey(nkey) {
      setKey(Qt::CTRL + (Qt::Key_1 + nrkey));
      connect(this, SIGNAL(activated()), this, SLOT(keyActivated()));
      connect(this, SIGNAL(activated(int)), parent, member);
   }

 private slots:
   void keyActivated() {
      emit activated(nrkey);
   }

 signals:
   void activated(int nkey);

 private:
   int nrkey;
};

class PhraseView : public QTreeView
{
   Q_OBJECT

 public:
   PhraseView(MultiDataModel *model, QList<QHash<QString, QList<Phrase *> > > *phraseDict, QWidget *parent = nullptr);
   ~PhraseView();
   void setSourceText(int model, const QString &sourceText);

 public slots:
   void toggleGuessing();
   void update();

 signals:
   void phraseSelected(int latestModel, const QString &phrase);

 protected:
   // QObject
   virtual void contextMenuEvent(QContextMenuEvent *event);
   // QAbstractItemView
   virtual void mouseDoubleClickEvent(QMouseEvent *event);

 private slots:
   void guessShortcut(int nkey);
   void selectPhrase(const QModelIndex &index);
   void selectPhrase();
   void editPhrase();

 private:
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

QT_END_NAMESPACE

#endif // PHRASEVIEW_H
