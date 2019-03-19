/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "translator.h"

#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <qtranslator_p.h>

static const uchar englishStyleRules[] = { Q_EQ, 1 };

static const uchar frenchStyleRules[]  = { Q_LEQ, 1 };

static const uchar latvianRules[]      = { Q_MOD_10 | Q_EQ, 1, Q_AND, Q_MOD_100 | Q_NEQ, 11, Q_NEWRULE, Q_NEQ, 0 };

static const uchar icelandicRules[]    = { Q_MOD_10 | Q_EQ, 1, Q_AND, Q_MOD_100 | Q_NEQ, 11 };

static const uchar irishStyleRules[]   = { Q_EQ, 1, Q_NEWRULE, Q_EQ, 2 };


static const uchar gaelicStyleRules[] =
    { Q_EQ, 1, Q_OR, Q_EQ, 11, Q_NEWRULE,
      Q_EQ, 2, Q_OR, Q_EQ, 12, Q_NEWRULE,
      Q_BETWEEN, 3, 19 };

static const uchar slovakStyleRules[] =
    { Q_EQ, 1, Q_NEWRULE,
      Q_BETWEEN, 2, 4 };

static const uchar macedonianRules[] = {
   Q_MOD_10 | Q_EQ, 1, Q_NEWRULE,
   Q_MOD_10 | Q_EQ, 2 };

static const uchar lithuanianRules[] = {
   Q_MOD_10 | Q_EQ, 1, Q_AND, Q_MOD_100 | Q_NEQ, 11, Q_NEWRULE,
   Q_MOD_10 | Q_NEQ, 0, Q_AND, Q_MOD_100 | Q_NOT_BETWEEN, 10, 19 };

static const uchar russianStyleRules[] = {
   Q_MOD_10 | Q_EQ, 1, Q_AND, Q_MOD_100 | Q_NEQ, 11, Q_NEWRULE,
   Q_MOD_10 | Q_BETWEEN, 2, 4, Q_AND, Q_MOD_100 | Q_NOT_BETWEEN, 10, 19 };

static const uchar polishRules[] = {
   Q_EQ, 1, Q_NEWRULE,
   Q_MOD_10 | Q_BETWEEN, 2, 4, Q_AND, Q_MOD_100 | Q_NOT_BETWEEN, 10, 19 };
static const uchar romanianRules[] = {
   Q_EQ, 1, Q_NEWRULE,
   Q_EQ, 0, Q_OR, Q_MOD_100 | Q_BETWEEN, 1, 19 };

static const uchar slovenianRules[] = {
   Q_MOD_100 | Q_EQ, 1, Q_NEWRULE,
   Q_MOD_100 | Q_EQ, 2, Q_NEWRULE,
   Q_MOD_100 | Q_BETWEEN, 3, 4 };

static const uchar malteseRules[] = {
   Q_EQ, 1, Q_NEWRULE,
   Q_EQ, 0, Q_OR, Q_MOD_100 | Q_BETWEEN, 1, 10, Q_NEWRULE,
   Q_MOD_100 | Q_BETWEEN, 11, 19 };

static const uchar welshRules[] = {
   Q_EQ, 0, Q_NEWRULE,
   Q_EQ, 1, Q_NEWRULE,
   Q_BETWEEN, 2, 5, Q_NEWRULE,
   Q_EQ, 6 };

static const uchar arabicRules[] = {
   Q_EQ, 0, Q_NEWRULE,
   Q_EQ, 1, Q_NEWRULE,
   Q_EQ, 2, Q_NEWRULE,
   Q_MOD_100 | Q_BETWEEN, 3, 10, Q_NEWRULE,
   Q_MOD_100 | Q_GEQ, 11 };

static const uchar tagalogRules[] = {
   Q_LEQ, 1, Q_NEWRULE,
   Q_MOD_10 | Q_EQ, 4, Q_OR, Q_MOD_10 | Q_EQ, 6, Q_OR, Q_MOD_10 | Q_EQ, 9 };


static const char *const japaneseStyleForms[] = { "Universal Form", 0 };
static const char *const englishStyleForms[]  = { "Singular", "Plural", 0 };
static const char *const frenchStyleForms[]   = { "Singular", "Plural", 0 };
static const char *const icelandicForms[]     = { "Singular", "Plural", 0 };
static const char *const latvianForms[]       = { "Singular", "Plural", "Nullar", 0 };
static const char *const irishStyleForms[]    = { "Singular", "Dual", "Plural", 0 };
static const char * const gaelicStyleForms[]  = { "1/11", "2/12", "Few", "Many", 0 };
static const char *const slovakStyleForms[]   = { "Singular", "Paucal", "Plural", 0 };
static const char *const macedonianForms[]    = { "Singular", "Dual", "Plural", 0 };
static const char *const lithuanianForms[]    = { "Singular", "Paucal", "Plural", 0 };
static const char *const russianStyleForms[]  = { "Singular", "Dual", "Plural", 0 };
static const char *const polishForms[] = { "Singular", "Paucal", "Plural", 0 };
static const char *const romanianForms[] = { "Singular", "Paucal", "Plural", 0 };
static const char *const slovenianForms[] = { "Singular", "Dual", "Trial", "Plural", 0 };

static const char *const malteseForms[] =
{ "Singular", "Paucal", "Greater Paucal", "Plural", 0 };

static const char *const welshForms[] =
{ "Nullar", "Singular", "Dual", "Sexal", "Plural", 0 };

static const char *const arabicForms[] =
{ "Nullar", "Singular", "Dual", "Minority Plural", "Plural", "Plural (100-102, ...)", 0 };

static const char *const tagalogForms[] =
{ "Singular", "Plural (consonant-ended)", "Plural (vowel-ended)", 0 };

#define EOL QLocale::C

static const QLocale::Language japaneseStyleLanguages[] = {
   QLocale::Armenian,
   QLocale::Bislama,
   QLocale::Burmese,
   QLocale::Chinese,
   QLocale::Dzongkha,
   QLocale::Fijian,
   QLocale::Guarani,
   QLocale::Hungarian,
   QLocale::Indonesian,
   QLocale::Japanese,
   QLocale::Javanese,
   QLocale::Korean,
   QLocale::Malay,
   QLocale::NauruLanguage,
   QLocale::Oromo,
   QLocale::Persian,
   QLocale::Sundanese,
   QLocale::Thai,
   QLocale::Tibetan,
   QLocale::Turkish,
   QLocale::Vietnamese,
   QLocale::Yoruba,
   QLocale::Zhuang,
   EOL
};

static const QLocale::Language englishStyleLanguages[] = {
    QLocale::Abkhazian,
    QLocale::Afar,
    QLocale::Afrikaans,
    QLocale::Albanian,
    QLocale::Amharic,
    QLocale::Assamese,
    QLocale::Aymara,
    QLocale::Azerbaijani,
    QLocale::Bashkir,
    QLocale::Basque,
    QLocale::Bengali,
    QLocale::Bihari,
    QLocale::Bulgarian,
    QLocale::Catalan,
    QLocale::Cornish,
    QLocale::Corsican,
    QLocale::Danish,
    QLocale::Dutch,
    QLocale::English,
    QLocale::Esperanto,
    QLocale::Estonian,
    QLocale::Faroese,
    QLocale::Finnish,
    QLocale::Friulian,
    QLocale::WesternFrisian,
    QLocale::Galician,
    QLocale::Georgian,
    QLocale::German,
    QLocale::Greek,
    QLocale::Greenlandic,
    QLocale::Gujarati,
    QLocale::Hausa,
    QLocale::Hebrew,
    QLocale::Hindi,
    QLocale::Interlingua,
    QLocale::Interlingue,
    QLocale::Italian,
    QLocale::Kannada,
    QLocale::Kashmiri,
    QLocale::Kazakh,
    QLocale::Khmer,
    QLocale::Kinyarwanda,
    QLocale::Kirghiz,
    QLocale::Kurdish,
    QLocale::Lao,
    QLocale::Latin,
    QLocale::Lingala,
    QLocale::Luxembourgish,
    QLocale::Malagasy,
    QLocale::Malayalam,
    QLocale::Marathi,
    QLocale::Mongolian,
    // Missing: Nahuatl,
    QLocale::Nepali,
    QLocale::NorthernSotho,
    QLocale::NorwegianBokmal,       // same as Norwegian
    QLocale::NorwegianNynorsk,
    QLocale::Occitan,
    QLocale::Oriya,
    QLocale::Pashto,
    QLocale::Portuguese,
    QLocale::Punjabi,
    QLocale::Quechua,
    QLocale::Romansh,
    QLocale::Rundi,
    QLocale::Shona,
    QLocale::Sindhi,
    QLocale::Sinhala,
    QLocale::Somali,
    QLocale::SouthernSotho,
    QLocale::Spanish,
    QLocale::Swahili,
    QLocale::Swati,
    QLocale::Swedish,
    QLocale::Tajik,
    QLocale::Tamil,
    QLocale::Tatar,
    QLocale::Telugu,
    QLocale::Tongan,
    QLocale::Tsonga,
    QLocale::Tswana,
    QLocale::Turkmen,
    // QLocale::Twi,          // mapped to Akan
    QLocale::Uigur,
    QLocale::Urdu,
    QLocale::Uzbek,
    QLocale::Volapuk,
    QLocale::Wolof,
    QLocale::Xhosa,
    QLocale::Yiddish,
    QLocale::Zulu,
   EOL
};

static const QLocale::Language frenchStyleLanguages[] = {
   // keep synchronized with frenchStyleCountries
    QLocale::Breton,
    QLocale::French,
    QLocale::Portuguese,
    QLocale::Filipino,
    QLocale::Tigrinya,
    QLocale::Walloon,
   EOL
};

static const QLocale::Language latvianLanguage[] = { QLocale::Latvian, EOL };
static const QLocale::Language icelandicLanguage[] = { QLocale::Icelandic, EOL };
static const QLocale::Language irishStyleLanguages[] = {
    QLocale::Divehi,
    QLocale::Inuktitut,
    QLocale::Inupiak,
    QLocale::Irish,
    QLocale::Manx,
    QLocale::Maori,
    QLocale::NorthernSami,
    QLocale::Samoan,
    QLocale::Sanskrit,
   EOL
};

static const QLocale::Language gaelicStyleLanguages[] = { QLocale::Gaelic, EOL };
static const QLocale::Language slovakStyleLanguages[] = { QLocale::Slovak, QLocale::Czech, EOL };
static const QLocale::Language macedonianLanguage[] = { QLocale::Macedonian, EOL };
static const QLocale::Language lithuanianLanguage[] = { QLocale::Lithuanian, EOL };
static const QLocale::Language russianStyleLanguages[] = {
   QLocale::Bosnian,
   QLocale::Belarusian,
   QLocale::Croatian,
   QLocale::Russian,
   QLocale::Serbian,
   QLocale::Ukrainian,
   EOL
};

static const QLocale::Language polishLanguage[] = { QLocale::Polish, EOL };
static const QLocale::Language romanianLanguages[] = {
   QLocale::Romanian,
   EOL
};

static const QLocale::Language slovenianLanguage[] = { QLocale::Slovenian, EOL };
static const QLocale::Language malteseLanguage[] = { QLocale::Maltese, EOL };
static const QLocale::Language welshLanguage[] = { QLocale::Welsh, EOL };
static const QLocale::Language arabicLanguage[] = { QLocale::Arabic, EOL };
static const QLocale::Language tagalogLanguage[] = { QLocale::Tagalog, EOL };

static const QLocale::Country frenchStyleCountries[] = {
   // keep synchronized with frenchStyleLanguages
    QLocale::AnyCountry,
    QLocale::AnyCountry,
    QLocale::Brazil,
    QLocale::AnyCountry,
    QLocale::AnyCountry,
    QLocale::AnyCountry
};

struct NumerusTableEntry {
   const uchar *rules;
   int rulesSize;

   const char *const *forms;
   const QLocale::Language *languages;
   const QLocale::Country *countries;
   const char *const gettextRules;
};

static const NumerusTableEntry numerusTable[] = {
   { 0, 0, japaneseStyleForms, japaneseStyleLanguages, 0, "nplurals=1; plural=0;" },

   {  englishStyleRules, sizeof(englishStyleRules), englishStyleForms, englishStyleLanguages, 0,
      "nplurals=2; plural=(n != 1);" },

   {  frenchStyleRules, sizeof(frenchStyleRules), frenchStyleForms, frenchStyleLanguages,
      frenchStyleCountries, "nplurals=2; plural=(n > 1);" },

   {  latvianRules, sizeof(latvianRules), latvianForms, latvianLanguage, 0,
      "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n != 0 ? 1 : 2);" },

   {  icelandicRules, sizeof(icelandicRules), icelandicForms, icelandicLanguage, 0,
      "nplurals=2; plural=(n%10==1 && n%100!=11 ? 0 : 1);" },

   { irishStyleRules, sizeof(irishStyleRules), irishStyleForms, irishStyleLanguages, 0,
      "nplurals=3; plural=(n==1 ? 0 : n==2 ? 1 : 2);" },

   { gaelicStyleRules, sizeof(gaelicStyleRules), gaelicStyleForms, gaelicStyleLanguages, 0,
      "nplurals=4; plural=(n==1 || n==11) ? 0 : (n==2 || n==12) ? 1 : (n > 2 && n < 20) ? 2 : 3;" },

   { slovakStyleRules, sizeof(slovakStyleRules), slovakStyleForms, slovakStyleLanguages, 0,
      "nplurals=3; plural=((n==1) ? 0 : (n>=2 && n<=4) ? 1 : 2);" },

    { macedonianRules, sizeof(macedonianRules), macedonianForms, macedonianLanguage, 0,
      "nplurals=3; plural=(n%100==1 ? 0 : n%100==2 ? 1 : 2);" },

    { lithuanianRules, sizeof(lithuanianRules), lithuanianForms, lithuanianLanguage, 0,
      "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && (n%100<10 || n%100>=20) ? 1 : 2);" },

    { russianStyleRules, sizeof(russianStyleRules), russianStyleForms, russianStyleLanguages, 0,
      "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);" },

    { polishRules, sizeof(polishRules), polishForms, polishLanguage, 0,
      "nplurals=3; plural=(n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);" },

    { romanianRules, sizeof(romanianRules), romanianForms, romanianLanguages, 0,
      "nplurals=3; plural=(n==1 ? 0 : (n==0 || (n%100 > 0 && n%100 < 20)) ? 1 : 2);" },

    { slovenianRules, sizeof(slovenianRules), slovenianForms, slovenianLanguage, 0,
      "nplurals=4; plural=(n%100==1 ? 0 : n%100==2 ? 1 : n%100==3 || n%100==4 ? 2 : 3);" },

    { malteseRules, sizeof(malteseRules), malteseForms, malteseLanguage, 0,
      "nplurals=4; plural=(n==1 ? 0 : (n==0 || (n%100>=1 && n%100<=10)) ? 1 : (n%100>=11 && n%100<=19) ? 2 : 3);" },

    { welshRules, sizeof(welshRules), welshForms, welshLanguage, 0,
      "nplurals=5; plural=(n==0 ? 0 : n==1 ? 1 : (n>=2 && n<=5) ? 2 : n==6 ? 3 : 4);" },

    { arabicRules, sizeof(arabicRules), arabicForms, arabicLanguage, 0,
      "nplurals=6; plural=(n==0 ? 0 : n==1 ? 1 : n==2 ? 2 : (n%100>=3 && n%100<=10) ? 3 : n%100>=11 ? 4 : 5);" },

    { tagalogRules, sizeof(tagalogRules), tagalogForms, tagalogLanguage, 0,
      "nplurals=3; plural=(n==1 ? 0 : (n%10==4 || n%10==6 || n%10== 9) ? 1 : 2);" },
};

static const int NumerusTableSize = sizeof(numerusTable) / sizeof(numerusTable[0]);

bool getNumerusInfo(QLocale::Language language, QLocale::Country country,
                    QByteArray *rules, QStringList *forms, const char **gettextRules)
{
   while (true) {
      for (int i = 0; i < NumerusTableSize; ++i) {
         const NumerusTableEntry &entry = numerusTable[i];

         for (int j = 0; entry.languages[j] != EOL; ++j) {

            if (entry.languages[j] == language && ((!entry.countries && country == QLocale::AnyCountry)
                      || (entry.countries && entry.countries[j] == country))) {

               if (rules) {
                  *rules = QByteArray::fromRawData(reinterpret_cast<const char *>(entry.rules), entry.rulesSize);
               }

               if (gettextRules) {
                  *gettextRules = entry.gettextRules;
               }

               if (forms) {
                  forms->clear();
                  for (int k = 0; entry.forms[k]; ++k) {
                     forms->append(QString::fromLatin1(entry.forms[k]));
                  }
               }

               return true;
            }
         }
      }

      if (country == QLocale::AnyCountry) {
         break;
      }

      country = QLocale::AnyCountry;
   }

   return false;
}

QString getNumerusInfoString()
{
   QStringList langs;

   for (int i = 0; i < NumerusTableSize; ++i) {
      const NumerusTableEntry &entry = numerusTable[i];

      for (int j = 0; entry.languages[j] != EOL; ++j) {
         QLocale loc(entry.languages[j], entry.countries ? entry.countries[j] : QLocale::AnyCountry);
         QString lang = QLocale::languageToString(entry.languages[j]);

         if (loc.language() == QLocale::C) {
            lang += " (!!!)";

         } else if (entry.countries && entry.countries[j] != QLocale::AnyCountry) {
            lang += " (" + QLocale::countryToString(loc.country()) + ')';

         } else {
            lang += " [" + QLocale::countryToString(loc.country()) + ']';
         }

         langs << QString("%1 %2 %3\n").formatArg(lang, -40).formatArg(loc.name(), -8).formatArg(QString::fromLatin1(entry.gettextRules));
      }
   }

   langs.sort();

   return langs.join("");
}

