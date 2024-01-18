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

#include <batchtranslation_dialog.h>
#include <phrase.h>
#include <messagemodel.h>

#include <qmessagebox.h>
#include <qprogressdialog.h>

CheckableListModel::CheckableListModel(QObject *parent)
   : QStandardItemModel(parent)
{
}

Qt::ItemFlags CheckableListModel::flags(const QModelIndex &index) const
{
   (void) index;
   return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

BatchTranslationDialog::BatchTranslationDialog(MultiDataModel *dataModel, QWidget *w)
   : QDialog(w), m_ui(new Ui::BatchTranslationDialog), m_model(this), m_dataModel(dataModel)
{
   m_ui->setupUi(this);

   m_ui->phrasebookList->setModel(&m_model);
   m_ui->phrasebookList->setSelectionBehavior(QAbstractItemView::SelectItems);
   m_ui->phrasebookList->setSelectionMode(QAbstractItemView::SingleSelection);

   connect(m_ui->runButton,      &QPushButton::clicked, this, &BatchTranslationDialog::startTranslation);
   connect(m_ui->moveUpButton,   &QPushButton::clicked, this, &BatchTranslationDialog::movePhraseBookUp);
   connect(m_ui->moveDownButton, &QPushButton::clicked, this, &BatchTranslationDialog::movePhraseBookDown);
}

BatchTranslationDialog::~BatchTranslationDialog()
{
   delete m_ui;
}

void BatchTranslationDialog::setPhraseBooks(const QList<PhraseBook *> &phrasebooks, int modelIndex)
{
   QString fn = QFileInfo(m_dataModel->srcFileName(modelIndex)).baseName();
   setWindowTitle(tr("Batch Translation for %1").formatArg(fn));

   m_model.clear();
   m_model.insertColumn(0);
   m_phrasebooks = phrasebooks;
   m_modelIndex = modelIndex;

   int count = phrasebooks.count();
   m_model.insertRows(0, count);

   for (int i = 0; i < count; ++i) {
      QModelIndex idx(m_model.index(i, 0));
      m_model.setData(idx, phrasebooks[i]->friendlyPhraseBookName());
      int sortOrder;

      if (phrasebooks[i]->language() != QLocale::C && m_dataModel->language(m_modelIndex) != QLocale::C) {

         if (phrasebooks[i]->language() != m_dataModel->language(m_modelIndex)) {
            sortOrder = 3;

         } else {
            sortOrder = (phrasebooks[i]->country() == m_dataModel->model(m_modelIndex)->country()) ? 0 : 1;
         }

      } else {
         sortOrder = 2;
      }

      m_model.setData(idx, sortOrder == 3 ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
      m_model.setData(idx, sortOrder, Qt::UserRole + 1);
      m_model.setData(idx, i, Qt::UserRole);
   }

   m_model.setSortRole(Qt::UserRole + 1);
   m_model.sort(0);
}

void BatchTranslationDialog::startTranslation()
{
   int translatedcount = 0;
   QCursor oldCursor = cursor();
   setCursor(Qt::BusyCursor);
   int messageCount = m_dataModel->messageCount();

   QProgressDialog *dlgProgress;
   dlgProgress = new QProgressDialog(tr("Searching, please wait..."), tr("&Cancel"), 0, messageCount, this);
   dlgProgress->show();

   int msgidx = 0;
   const bool translateTranslated = m_ui->ckTranslateTranslated->isChecked();
   const bool translateFinished = m_ui->ckTranslateFinished->isChecked();

   for (MultiDataModelIterator it(m_dataModel, m_modelIndex); it.isValid(); ++it) {
      if (MessageItem *m = it.current()) {
         if (!m->isObsolete()
               && (translateTranslated || m->translation().isEmpty())
               && (translateFinished || !m->isFinished())) {

            // Go through them in the order the user specified in the phrasebookList
            for (int b = 0; b < m_model.rowCount(); ++b) {
               QModelIndex idx(m_model.index(b, 0));
               QVariant checkState = m_model.data(idx, Qt::CheckStateRole);

               if (checkState == Qt::Checked) {
                  PhraseBook *pb = m_phrasebooks[m_model.data(idx, Qt::UserRole).toInt()];

                  for (const Phrase * ph : pb->phrases()) {
                     if (ph->source() == m->text()) {
                        m_dataModel->setTranslation(it, ph->target());
                        m_dataModel->setFinished(it, m_ui->ckMarkFinished->isChecked());
                        ++translatedcount;

                        goto done; // break 2;
                     }
                  }
               }
            }
         }
      }

   done:
      ++msgidx;

      if (!(msgidx & 15)) {
         dlgProgress->setValue(msgidx);
      }

      qApp->processEvents();

      if (dlgProgress->wasCanceled()) {
         break;
      }
   }

   dlgProgress->hide();

   setCursor(oldCursor);
   emit finished();

   QMessageBox::information(this, tr("Linguist Batch Translator"),
            tr("Batch translated %n entries", "", translatedcount), QMessageBox::Ok);
}

void BatchTranslationDialog::movePhraseBookUp()
{
   QModelIndexList indexes = m_ui->phrasebookList->selectionModel()->selectedIndexes();

   if (indexes.count() <= 0) {
      return;
   }

   QModelIndex sel = indexes[0];
   int row = sel.row();

   if (row > 0) {
      QModelIndex other = m_model.index(row - 1, 0);
      QMap<int, QVariant> seldata = m_model.itemData(sel);
      m_model.setItemData(sel, m_model.itemData(other));
      m_model.setItemData(other, seldata);

      m_ui->phrasebookList->selectionModel()->setCurrentIndex(other, QItemSelectionModel::ClearAndSelect);
   }
}

void BatchTranslationDialog::movePhraseBookDown()
{
   QModelIndexList indexes = m_ui->phrasebookList->selectionModel()->selectedIndexes();

   if (indexes.count() <= 0) {
      return;
   }

   QModelIndex sel = indexes[0];
   int row = sel.row();

   if (row < m_model.rowCount() - 1) {
      QModelIndex other = m_model.index(row + 1, 0);
      QMap<int, QVariant> seldata = m_model.itemData(sel);
      m_model.setItemData(sel, m_model.itemData(other));
      m_model.setItemData(other, seldata);
      m_ui->phrasebookList->selectionModel()->setCurrentIndex(other, QItemSelectionModel::ClearAndSelect);
   }
}

