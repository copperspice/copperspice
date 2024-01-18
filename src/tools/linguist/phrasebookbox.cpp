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

#include <phrasebookbox.h>
#include <settings_dialog.h>

#include <qevent.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qheaderview.h>
#include <qsortfilterproxymodel.h>

PhraseBookBox::PhraseBookBox(PhraseBook *phraseBook, QWidget *parent)
   : QDialog(parent), m_phraseBook(phraseBook), m_settingsDialog(nullptr)
{

// definition needs to be within class context for lupdate to find it
#define NewPhrase tr("(New Entry)")

   setupUi(this);
   setWindowTitle(tr("%1[*]").formatArg(m_phraseBook->friendlyPhraseBookName()));
   setWindowModified(m_phraseBook->isModified());

   phrMdl = new PhraseModel(this);

   m_sortedPhraseModel = new QSortFilterProxyModel(this);
   m_sortedPhraseModel->setSortCaseSensitivity(Qt::CaseInsensitive);
   m_sortedPhraseModel->setSortLocaleAware(true);
   m_sortedPhraseModel->setDynamicSortFilter(true);
   m_sortedPhraseModel->setSourceModel(phrMdl);

   phraseList->setModel(m_sortedPhraseModel);
   phraseList->header()->setDefaultSectionSize(150);
   phraseList->header()->setSectionResizeMode(QHeaderView::Interactive);

   connect(sourceLed,     &QLineEdit::textChanged,      this, &PhraseBookBox::sourceChanged);
   connect(targetLed,     &QLineEdit::textChanged,      this, &PhraseBookBox::targetChanged);
   connect(definitionLed, &QLineEdit::textChanged,      this, &PhraseBookBox::definitionChanged);

   connect(phraseList->selectionModel(), &QItemSelectionModel::currentChanged,
                  this, &PhraseBookBox::selectionChanged);

   connect(newBut,        &QAbstractButton::clicked,    this, &PhraseBookBox::newPhrase);
   connect(removeBut,     &QAbstractButton::clicked,    this, &PhraseBookBox::removePhrase);
   connect(settingsBut,   &QAbstractButton::clicked,    this, &PhraseBookBox::settings);
   connect(saveBut,       &QAbstractButton::clicked,    this, &PhraseBookBox::save);
   connect(m_phraseBook,  &PhraseBook::modifiedChanged, this, &PhraseBookBox::setWindowModified);

   sourceLed->installEventFilter(this);
   targetLed->installEventFilter(this);
   definitionLed->installEventFilter(this);

   for (Phrase *p : phraseBook->phrases()) {
      phrMdl->addPhrase(p);
   }

   phraseList->sortByColumn(0, Qt::AscendingOrder);

   enableDisable();
}

bool PhraseBookBox::eventFilter(QObject *obj, QEvent *event)
{
   if (event->type() == QEvent::KeyPress && (obj == sourceLed || obj == targetLed || obj == definitionLed)) {
      const QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      const int key = keyEvent->key();

      switch (key) {
         case Qt::Key_Down:
         case Qt::Key_Up:
         case Qt::Key_PageDown:
         case Qt::Key_PageUp:
            return QApplication::sendEvent(phraseList, event);
      }
   }

   return QDialog::eventFilter(obj, event);
}

void PhraseBookBox::newPhrase()
{
   Phrase *p = new Phrase();
   p->setSource(NewPhrase);
   m_phraseBook->append(p);
   selectItem(phrMdl->addPhrase(p));
}

void PhraseBookBox::removePhrase()
{
   QModelIndex index = currentPhraseIndex();
   Phrase *phrase = phrMdl->phrase(index);
   m_phraseBook->remove(phrase);
   phrMdl->removePhrase(index);
   delete phrase;
}

void PhraseBookBox::settings()
{
   if (! m_settingsDialog) {
      m_settingsDialog = new SettingsDialog(this);
   }

   m_settingsDialog->setPhraseBook(m_phraseBook);
   m_settingsDialog->exec();
}

void PhraseBookBox::save()
{
   const QString &fileName = m_phraseBook->fileName();
   if (! m_phraseBook->save(fileName))
      QMessageBox::warning(this, tr("Linguist"), tr("Unable to savephrase book '%1'.").formatArg(fileName));
}

void PhraseBookBox::sourceChanged(const QString &source)
{
   QModelIndex index = currentPhraseIndex();
   if (index.isValid()) {
      phrMdl->setData(phrMdl->index(index.row(), 0), source);
   }
}

void PhraseBookBox::targetChanged(const QString &target)
{
   QModelIndex index = currentPhraseIndex();

   if (index.isValid()) {
      phrMdl->setData(phrMdl->index(index.row(), 1), target);
   }
}

void PhraseBookBox::definitionChanged(const QString &definition)
{
   QModelIndex index = currentPhraseIndex();

   if (index.isValid()) {
      phrMdl->setData(phrMdl->index(index.row(), 2), definition);
   }
}

void PhraseBookBox::selectionChanged()
{
   enableDisable();
}

void PhraseBookBox::selectItem(const QModelIndex &index)
{
   const QModelIndex &sortedIndex = m_sortedPhraseModel->mapFromSource(index);
   phraseList->scrollTo(sortedIndex);
   phraseList->setCurrentIndex(sortedIndex);
}

void PhraseBookBox::enableDisable()
{
   QModelIndex index = currentPhraseIndex();

   sourceLed->blockSignals(true);
   targetLed->blockSignals(true);
   definitionLed->blockSignals(true);

   bool indexValid = index.isValid();

   if (indexValid) {
      Phrase *p = phrMdl->phrase(index);
      sourceLed->setText(p->source().simplified());
      targetLed->setText(p->target().simplified());
      definitionLed->setText(p->definition());
   } else {
      sourceLed->setText(QString());
      targetLed->setText(QString());
      definitionLed->setText(QString());
   }

   sourceLed->setEnabled(indexValid);
   targetLed->setEnabled(indexValid);
   definitionLed->setEnabled(indexValid);
   removeBut->setEnabled(indexValid);

   sourceLed->blockSignals(false);
   targetLed->blockSignals(false);
   definitionLed->blockSignals(false);

   QWidget *f = QApplication::focusWidget();
   if (f != sourceLed && f != targetLed && f != definitionLed) {
      QLineEdit *led = (sourceLed->text() == NewPhrase ? sourceLed : targetLed);
      led->setFocus();
      led->selectAll();
   } else {
      static_cast<QLineEdit *>(f)->selectAll();
   }
}

QModelIndex PhraseBookBox::currentPhraseIndex() const
{
   return m_sortedPhraseModel->mapToSource(phraseList->currentIndex());
}

