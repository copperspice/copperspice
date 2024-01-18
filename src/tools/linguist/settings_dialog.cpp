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

#include <settings_dialog.h>
#include <messagemodel.h>
#include <phrase.h>

#include <qlocale.h>

SettingsDialog::SettingsDialog(QWidget *parent)
   : QDialog(parent), m_ui(new Ui::SettingsDialog)
{
   m_ui->setupUi(this);

   for (int i = QLocale::C + 1; i < QLocale::LastLanguage; ++i) {
      QString lang = QLocale::languageToString(QLocale::Language(i));
      m_ui->srcCbLanguageList->addItem(lang, QVariant(i));
   }

   m_ui->srcCbLanguageList->model()->sort(0, Qt::AscendingOrder);
   m_ui->srcCbLanguageList->insertItem(0, "POSIX", QVariant(QLocale::C));

   m_ui->tgtCbLanguageList->setModel(m_ui->srcCbLanguageList->model());
}

SettingsDialog::~SettingsDialog()
{
   delete m_ui;
}

void SettingsDialog::setDataModel(DataModel *dataModel)
{
   m_dataModel  = dataModel;
   m_phraseBook = nullptr;

   QString fname = QFileInfo(dataModel->srcFileName()).baseName();
   setWindowTitle(tr("Settings for %1").formatArg(fname));
}

void SettingsDialog::setPhraseBook(PhraseBook *phraseBook)
{
   m_phraseBook  = phraseBook;
   m_dataModel   = nullptr;
   QString fname = QFileInfo(phraseBook->fileName()).baseName();

   setWindowTitle(tr("Settings for %1").formatArg(fname));
}

static void fillCountryCombo(const QVariant &data, QComboBox *combo)
{
   combo->clear();
   QLocale::Language lang = QLocale::Language(data.toInt());

   if (lang != QLocale::C) {
      QList<QLocale> list_locale = QLocale::matchingLocales(lang, QLocale::AnyScript, QLocale::AnyCountry);

      for (QLocale item : list_locale) {
         auto countryId = item.country();

         QString countryName = QLocale::countryToString(countryId );
         combo->addItem(countryName, QVariant(countryId ));
      }

      combo->model()->sort(0, Qt::AscendingOrder);
   }

   combo->insertItem(0, SettingsDialog::tr("Any Country"), QVariant(QLocale::AnyCountry));
   combo->setCurrentIndex(0);
}

void SettingsDialog::on_srcCbLanguageList_currentIndexChanged(int idx)
{
   fillCountryCombo(m_ui->srcCbLanguageList->itemData(idx), m_ui->srcCbCountryList);
}

void SettingsDialog::on_tgtCbLanguageList_currentIndexChanged(int idx)
{
   fillCountryCombo(m_ui->tgtCbLanguageList->itemData(idx), m_ui->tgtCbCountryList);
}

void SettingsDialog::on_buttonBox_accepted()
{
   int itemindex = m_ui->tgtCbLanguageList->currentIndex();
   QVariant var  = m_ui->tgtCbLanguageList->itemData(itemindex);
   QLocale::Language lang = QLocale::Language(var.toInt());

   itemindex = m_ui->tgtCbCountryList->currentIndex();
   var = m_ui->tgtCbCountryList->itemData(itemindex);
   QLocale::Country country = QLocale::Country(var.toInt());

   itemindex = m_ui->srcCbLanguageList->currentIndex();
   var = m_ui->srcCbLanguageList->itemData(itemindex);
   QLocale::Language lang2 = QLocale::Language(var.toInt());

   itemindex = m_ui->srcCbCountryList->currentIndex();
   var = m_ui->srcCbCountryList->itemData(itemindex);
   QLocale::Country country2 = QLocale::Country(var.toInt());

   if (m_phraseBook) {
      m_phraseBook->setLanguageAndCountry(lang, country);
      m_phraseBook->setSourceLanguageAndCountry(lang2, country2);
   } else {
      m_dataModel->setLanguageAndCountry(lang, country);
      m_dataModel->setSourceLanguageAndCountry(lang2, country2);
   }

   accept();
}

void SettingsDialog::showEvent(QShowEvent *)
{
   QLocale::Language lang1;
   QLocale::Language lang2;

   QLocale::Country country1;
   QLocale::Country country2;

   if (m_phraseBook) {
      lang1    = m_phraseBook->language();
      country1 = m_phraseBook->country();
      lang2    = m_phraseBook->sourceLanguage();
      country2 = m_phraseBook->sourceCountry();

   } else {
      lang1    = m_dataModel->language();
      country1 = m_dataModel->country();
      lang2    = m_dataModel->sourceLanguage();
      country2 = m_dataModel->sourceCountry();
   }

   int itemindex;

   itemindex = m_ui->tgtCbLanguageList->findData(QVariant(int(lang1)));
   m_ui->tgtCbLanguageList->setCurrentIndex(itemindex == -1 ? 0 : itemindex);

   itemindex = m_ui->tgtCbCountryList->findData(QVariant(int(country1)));
   m_ui->tgtCbCountryList->setCurrentIndex(itemindex == -1 ? 0 : itemindex);

   itemindex = m_ui->srcCbLanguageList->findData(QVariant(int(lang2)));
   m_ui->srcCbLanguageList->setCurrentIndex(itemindex == -1 ? 0 : itemindex);

   itemindex = m_ui->srcCbCountryList->findData(QVariant(int(country2)));
   m_ui->srcCbCountryList->setCurrentIndex(itemindex == -1 ? 0 : itemindex);
}

