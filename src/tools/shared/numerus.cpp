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

#include <translator.h>

#include <qbytearray.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmap.h>

static const std::variant<CountGuide, int> englishStyleRules[] = {
   CountGuide::Equal,
   1
};

static const std::variant<CountGuide, int> frenchStyleRules[]  = { CountGuide::LessThanEqual, 1 };

static const std::variant<CountGuide, int> latvianRules[]      = {
   CountGuide::Remainder_10  | CountGuide::Equal,
   1,
   CountGuide::And,
   CountGuide::Remainder_100 | CountGuide::NotEqual,
   11,
   CountGuide::LastEntry, CountGuide::NotEqual,
   0
};

static const std::variant<CountGuide, int> icelandicRules[]    = {
   CountGuide::Remainder_10 | CountGuide::Equal,
   1,
   CountGuide::And,
   CountGuide::Remainder_100 | CountGuide::NotEqual,
   11
};

static const std::variant<CountGuide, int> irishStyleRules[]   = {
   CountGuide::Equal,
   1,
   CountGuide::LastEntry,
   CountGuide::Equal,
   2
};

static const std::variant<CountGuide, int> gaelicStyleRules[] = {
   CountGuide::Equal,
   1,
   CountGuide::Or,
   CountGuide::Equal,
   11,
   CountGuide::LastEntry,
   CountGuide::Equal,
   2,
   CountGuide::Or,
   CountGuide::Equal,
   12,
   CountGuide::LastEntry,
   CountGuide::Between,
   3,
   19
};

static const std::variant<CountGuide, int> slovakStyleRules[] = {
   CountGuide::Equal,
   1,
   CountGuide::LastEntry,
   CountGuide::Between,
   2,
   4
};

static const std::variant<CountGuide, int> macedonianRules[] = {
   CountGuide::Remainder_10 | CountGuide::Equal,
   1,
   CountGuide::LastEntry,
   CountGuide::Remainder_10 | CountGuide::Equal,
   2
};

static const std::variant<CountGuide, int> lithuanianRules[] = {
   CountGuide::Remainder_10 | CountGuide::Equal,
   1,
   CountGuide::And,
   CountGuide::Remainder_100 | CountGuide::NotEqual,
   11,
   CountGuide::LastEntry,
   CountGuide::Remainder_10 | CountGuide::NotEqual,
   0,
   CountGuide::And,
   CountGuide::Remainder_100 | CountGuide::NotBetween,
   10,
   19
};

static const std::variant<CountGuide, int> russianStyleRules[] = {
   CountGuide::Remainder_10 | CountGuide::Equal,
   1,
   CountGuide::And,
   CountGuide::Remainder_100 | CountGuide::NotEqual,
   11,
   CountGuide::LastEntry,
   CountGuide::Remainder_10 | CountGuide::Between,
   2,
   4,
   CountGuide::And,
   CountGuide::Remainder_100 | CountGuide::NotBetween,
   10,
   19
};

static const std::variant<CountGuide, int> polishRules[] = {
   CountGuide::Equal,
   1,
   CountGuide::LastEntry,
   CountGuide::Remainder_10 | CountGuide::Between,
   2,
   4,
   CountGuide::And, CountGuide::Remainder_100 | CountGuide::NotBetween,
   10,
   19
};

static const std::variant<CountGuide, int> romanianRules[] = {
   CountGuide::Equal,
   1,
   CountGuide::LastEntry,
   CountGuide::Equal,
   0,
   CountGuide::Or,
   CountGuide::Remainder_100 | CountGuide::Between,
   1,
   19
};

static const std::variant<CountGuide, int> slovenianRules[] = {
   CountGuide::Remainder_100 | CountGuide::Equal,
   1,
   CountGuide::LastEntry,
   CountGuide::Remainder_100 | CountGuide::Equal,
   2,
   CountGuide::LastEntry,
   CountGuide::Remainder_100 | CountGuide::Between,
   3,
   4
};

static const std::variant<CountGuide, int> malteseRules[] = {
   CountGuide::Equal,
   1,
   CountGuide::LastEntry,
   CountGuide::Equal,
   0,
   CountGuide::Or,
   CountGuide::Remainder_100 | CountGuide::Between,
   1,
   10,
   CountGuide::LastEntry,
   CountGuide::Remainder_100 | CountGuide::Between,
   11,
   19
};

static const std::variant<CountGuide, int> welshRules[] = {
   CountGuide::Equal,
   0,
   CountGuide::LastEntry,
   CountGuide::Equal,
   1,
   CountGuide::LastEntry,
   CountGuide::Between,
   2,
   5,
   CountGuide::LastEntry,
   CountGuide::Equal,
   6
};

static const std::variant<CountGuide, int> arabicRules[] = {
   CountGuide::Equal,
   0,
   CountGuide::LastEntry,
   CountGuide::Equal,
   1,
   CountGuide::LastEntry,
   CountGuide::Equal,
   2,
   CountGuide::LastEntry,
   CountGuide::Remainder_100 | CountGuide::Between,
   3,
   10,
   CountGuide::LastEntry,
   CountGuide::Remainder_100 | CountGuide::GreaterThanEqual,
   11
};

static const std::variant<CountGuide, int> filipinoRules[] = {
   CountGuide::LessThanEqual,
   1,
   CountGuide::LastEntry,
   CountGuide::Remainder_10 | CountGuide::Equal,
   4,
   CountGuide::Or,
   CountGuide::Remainder_10 | CountGuide::Equal,
   6,
   CountGuide::Or,
   CountGuide::Remainder_10 | CountGuide::Equal,
   9
};

static const char *const japaneseStyleForms[] = { "Universal Form",               nullptr };
static const char *const englishStyleForms[]  = { "Singular", "Plural",           nullptr };
static const char *const frenchStyleForms[]   = { "Singular", "Plural",           nullptr };
static const char *const icelandicForms[]     = { "Singular", "Plural",           nullptr };
static const char *const latvianForms[]       = { "Singular", "Plural", "Nullar", nullptr };
static const char *const irishStyleForms[]    = { "Singular", "Dual", "Plural",   nullptr };
static const char *const gaelicStyleForms[]   = { "1/11", "2/12", "Few", "Many",  nullptr };
static const char *const slovakStyleForms[]   = { "Singular", "Paucal", "Plural", nullptr };
static const char *const macedonianForms[]    = { "Singular", "Dual", "Plural",   nullptr };
static const char *const lithuanianForms[]    = { "Singular", "Paucal", "Plural", nullptr };
static const char *const russianStyleForms[]  = { "Singular", "Dual", "Plural",   nullptr };
static const char *const polishForms[]        = { "Singular", "Paucal", "Plural", nullptr };
static const char *const romanianForms[]      = { "Singular", "Paucal", "Plural", nullptr };

static const char *const slovenianForms[]     = { "Singular", "Dual", "Trial", "Plural",            nullptr };
static const char *const malteseForms[]       = { "Singular", "Paucal", "Greater Paucal", "Plural", nullptr };
static const char *const welshForms[]         = { "Nullar", "Singular", "Dual", "Sexal", "Plural",  nullptr };

static const char *const arabicForms[]        =
      { "Nullar", "Singular", "Dual", "Minority Plural", "Plural", "Plural (100-102, ...)", nullptr };

static const char *const filipinoForms[] =
      { "Singular", "Plural (consonant-ended)", "Plural (vowel-ended)", nullptr };

static const QLocale::Language japaneseStyleLanguages[] = {
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
   QLocale::C
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
   QLocale::Kyrgyz,
   QLocale::Kurdish,
   QLocale::Lao,
   QLocale::Latin,
   QLocale::Lingala,
   QLocale::Luxembourgish,
   QLocale::Malagasy,
   QLocale::Malayalam,
   QLocale::Marathi,
   QLocale::Mongolian,
   QLocale::Nepali,
   QLocale::NorthernSotho,
   QLocale::NorwegianBokmal,
   QLocale::NorwegianNynorsk,
   QLocale::Occitan,
   QLocale::Odia,
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
   QLocale::Uyghur,
   QLocale::Urdu,
   QLocale::Uzbek,
   QLocale::Volapuk,
   QLocale::WesternFrisian,
   QLocale::Wolof,
   QLocale::Xhosa,
   QLocale::Yiddish,
   QLocale::Zulu,
   QLocale::C
};

static const QLocale::Language frenchStyleLanguages[] = {
   // keep synchronized with frenchStyleCountries
   QLocale::Armenian,
   QLocale::Breton,
   QLocale::French,
   QLocale::Portuguese,
   QLocale::Filipino,
   QLocale::Tigrinya,
   QLocale::Walloon,
   QLocale::C
};

static const QLocale::Language latvianLanguage[]     = { QLocale::Latvian,   QLocale::C};
static const QLocale::Language icelandicLanguage[]   = { QLocale::Icelandic, QLocale::C };

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
   QLocale::C
};

static const QLocale::Language gaelicStyleLanguages[] = {
   QLocale::Gaelic,
   QLocale::C
};

static const QLocale::Language slovakStyleLanguages[] = {
   QLocale::Slovak,
   QLocale::Czech,
   QLocale::C
};

static const QLocale::Language macedonianLanguage[] = {
   QLocale::Macedonian,
   QLocale::C
};

static const QLocale::Language lithuanianLanguage[] = {
   QLocale::Lithuanian,
   QLocale::C
};

static const QLocale::Language russianStyleLanguages[] = {
   QLocale::Bosnian,
   QLocale::Belarusian,
   QLocale::Croatian,
   QLocale::Russian,
   QLocale::Serbian,
   QLocale::Ukrainian,
   QLocale::C
};

static const QLocale::Language polishLanguage[] = {
   QLocale::Polish, QLocale::C
};

static const QLocale::Language romanianLanguages[] = {
   QLocale::Romanian,
   QLocale::C
};

static const QLocale::Language slovenianLanguage[] = { QLocale::Slovenian, QLocale::C };
static const QLocale::Language malteseLanguage[]   = { QLocale::Maltese,   QLocale::C };
static const QLocale::Language welshLanguage[]     = { QLocale::Welsh,     QLocale::C };
static const QLocale::Language arabicLanguage[]    = { QLocale::Arabic,    QLocale::C };
static const QLocale::Language filipinoLanguage[]  = { QLocale::Filipino,  QLocale::C };

static const QLocale::Country frenchStyleCountries[] = {
   // keep synchronized with frenchStyleLanguages
   QLocale::AnyCountry,
   QLocale::AnyCountry,
   QLocale::AnyCountry,
   QLocale::Brazil,
   QLocale::AnyCountry,
   QLocale::AnyCountry,
   QLocale::AnyCountry
};

struct CountTableEntry {
   const std::variant<CountGuide, int> *rules;
   int rulesSize;

   const char *const *forms;
   const QLocale::Language *languages;
   const QLocale::Country *countries;
   const char *const gettextRules;
};

static const CountTableEntry countTable[] = {
   {  nullptr, 0, japaneseStyleForms, japaneseStyleLanguages, nullptr, "nplurals=1; plural=0;" },

   {
      englishStyleRules, std::size(englishStyleRules), englishStyleForms, englishStyleLanguages, nullptr,
      "nplurals=2; plural=(n != 1);"
   },

   {
      frenchStyleRules, std::size(frenchStyleRules), frenchStyleForms, frenchStyleLanguages,
      frenchStyleCountries, "nplurals=2; plural=(n > 1);"
   },

   {
      latvianRules, std::size(latvianRules), latvianForms, latvianLanguage, nullptr,
      "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n != 0 ? 1 : 2);"
   },

   {
      icelandicRules, std::size(icelandicRules), icelandicForms, icelandicLanguage, nullptr,
      "nplurals=2; plural=(n%10==1 && n%100!=11 ? 0 : 1);"
   },

   {
      irishStyleRules, std::size(irishStyleRules), irishStyleForms, irishStyleLanguages, nullptr,
      "nplurals=3; plural=(n==1 ? 0 : n==2 ? 1 : 2);"
   },

   {
      gaelicStyleRules, std::size(gaelicStyleRules), gaelicStyleForms, gaelicStyleLanguages, nullptr,
      "nplurals=4; plural=(n==1 || n==11) ? 0 : (n==2 || n==12) ? 1 : (n > 2 && n < 20) ? 2 : 3;"
   },

   {
      slovakStyleRules, std::size(slovakStyleRules), slovakStyleForms, slovakStyleLanguages, nullptr,
      "nplurals=3; plural=((n==1) ? 0 : (n>=2 && n<=4) ? 1 : 2);"
   },

   {
      macedonianRules, std::size(macedonianRules), macedonianForms, macedonianLanguage, nullptr,
      "nplurals=3; plural=(n%100==1 ? 0 : n%100==2 ? 1 : 2);"
   },

   {
      lithuanianRules, std::size(lithuanianRules), lithuanianForms, lithuanianLanguage, nullptr,
      "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && (n%100<10 || n%100>=20) ? 1 : 2);"
   },

   {
      russianStyleRules, std::size(russianStyleRules), russianStyleForms, russianStyleLanguages, nullptr,
      "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);"
   },

   {
      polishRules, std::size(polishRules), polishForms, polishLanguage, nullptr,
      "nplurals=3; plural=(n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);"
   },

   {
      romanianRules, std::size(romanianRules), romanianForms, romanianLanguages, nullptr,
      "nplurals=3; plural=(n==1 ? 0 : (n==0 || (n%100 > 0 && n%100 < 20)) ? 1 : 2);"
   },

   {
      slovenianRules, std::size(slovenianRules), slovenianForms, slovenianLanguage, nullptr,
      "nplurals=4; plural=(n%100==1 ? 0 : n%100==2 ? 1 : n%100==3 || n%100==4 ? 2 : 3);"
   },

   {
      malteseRules, std::size(malteseRules), malteseForms, malteseLanguage, nullptr,
      "nplurals=4; plural=(n==1 ? 0 : (n==0 || (n%100>=1 && n%100<=10)) ? 1 : (n%100>=11 && n%100<=19) ? 2 : 3);"
   },

   {
      welshRules, std::size(welshRules), welshForms, welshLanguage, nullptr,
      "nplurals=5; plural=(n==0 ? 0 : n==1 ? 1 : (n>=2 && n<=5) ? 2 : n==6 ? 3 : 4);"
   },

   {
      arabicRules, std::size(arabicRules), arabicForms, arabicLanguage, nullptr,
      "nplurals=6; plural=(n==0 ? 0 : n==1 ? 1 : n==2 ? 2 : (n%100>=3 && n%100<=10) ? 3 : n%100>=11 ? 4 : 5);"
   },

   {
      filipinoRules, std::size(filipinoRules), filipinoForms, filipinoLanguage, nullptr,
      "nplurals=3; plural=(n==1 ? 0 : (n%10==4 || n%10==6 || n%10== 9) ? 1 : 2);"
   },
};

static const int CountTableSize = std::size(countTable);

bool getCountInfo(QLocale::Language language, QLocale::Country country,
               QVector<std::variant<CountGuide, int>> *data, QStringList *forms, const char **gettextRules)
{
   while (true) {
      for (int i = 0; i < CountTableSize; ++i) {
         const CountTableEntry &entry = countTable[i];

         for (int j = 0; entry.languages[j] != QLocale::C; ++j) {

            if (entry.languages[j] == language && ((entry.countries == nullptr && country == QLocale::AnyCountry)
                    || (entry.countries != nullptr && entry.countries[j] == country))) {

               if (data != nullptr) {
                  QVector<std::variant<CountGuide, int>> tmp;
                  tmp.append(entry.rules, entry.rules + entry.rulesSize);

                  *data = std::move(tmp);
               }

               if (gettextRules != nullptr) {
                  *gettextRules = entry.gettextRules;
               }

               if (forms != nullptr) {
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

QString getCountInfoString()
{
   QStringList langs;

   for (int i = 0; i < CountTableSize; ++i) {
      const CountTableEntry &entry = countTable[i];

      for (int j = 0; entry.languages[j] != QLocale::C; ++j) {
         QLocale loc(entry.languages[j], entry.countries ? entry.countries[j] : QLocale::AnyCountry);
         QString lang = QLocale::languageToString(entry.languages[j]);

         if (loc.language() == QLocale::C) {
            lang += " (!!!)";

         } else if (entry.countries && entry.countries[j] != QLocale::AnyCountry) {
            lang += " (" + QLocale::countryToString(loc.country()) + ')';

         } else {
            lang += " [" + QLocale::countryToString(loc.country()) + ']';
         }

         langs << QString("%1 %2 %3\n").formatArg(lang, -40).
               formatArg(loc.name(), -8).formatArg(QString::fromLatin1(entry.gettextRules));
      }
   }

   langs.sort();

   return langs.join("");
}
