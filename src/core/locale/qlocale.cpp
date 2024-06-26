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

#include <qglobal.h>
#include <qlocale.h>

#include <qplatformdefs.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qhashfunc.h>
#include <qnamespace.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qstringparser.h>
#include <qvariant.h>

#include <qdatetime_p.h>
#include <qdatetimeparser_p.h>
#include <qlocale_p.h>
#include <qlocale_data_p.h>
#include <qlocale_tools_p.h>
#include <qnumeric_p.h>
#include <qsystemlibrary_p.h>

#if defined(Q_OS_DARWIN)
#include <qcore_mac_p.h>
#endif

#if defined(Q_OS_WIN)
#include <qt_windows.h>
#include <time.h>
#endif

#include <stdlib.h>

#ifndef QT_NO_SYSTEMLOCALE

static QSystemLocale *s_system_locale = nullptr;
static QLocaleData *s_system_data     = nullptr;

static QSystemLocale *global_SystemLocale()
{
   static QSystemLocale retval = QSystemLocale::cs_internal_tag();
   return &retval;
}

static QLocaleData *global_SystemData()
{
   static QLocaleData retval;
   return &retval;
}

#endif

static const QLocaleData *defaultData();
static uint default_number_options = 0;

static constexpr const int locale_data_size = sizeof(locale_data) / sizeof(QLocaleData) - 1;

static const QLocaleData *default_data = nullptr;
static const QLocaleData *const c_data = locale_data;

static QSharedDataPointer<QLocalePrivate> *defaultLocalePrivate()
{
   static QSharedDataPointer<QLocalePrivate> retval(QLocalePrivate::create(defaultData(), default_number_options));
   return &retval;
}

QLocale::Language QLocalePrivate::codeToLanguage(const QString &code)
{
   int len = code.length();

   if (len != 2 && len != 3) {
      return QLocale::C;
   }

   ushort uc1 = code[0].toLower()[0].unicode();
   ushort uc2 = code[1].toLower()[0].unicode();
   ushort uc3 = len > 2 ? code[2].toLower()[0].unicode() : 0;

   const unsigned char *c = language_code_list;

   for (; *c != 0; c += 3) {
      if (uc1 == c[0] && uc2 == c[1] && uc3 == c[2]) {
         return QLocale::Language((c - language_code_list) / 3);
      }
   }

   // Android uses the following deprecated codes
   if (uc1 == 'i' && uc2 == 'w' && uc3 == 0) {
      // iw -> he
      return QLocale::Hebrew;
   }

   if (uc1 == 'i' && uc2 == 'n' && uc3 == 0) {
      // in -> id
      return QLocale::Indonesian;
   }

   if (uc1 == 'j' && uc2 == 'i' && uc3 == 0) {
      // ji -> yi
      return QLocale::Yiddish;
   }

   return QLocale::C;
}

QLocale::Script QLocalePrivate::codeToScript(const QString &code)
{
   int len = code.length();

   if (len != 4) {
      return QLocale::AnyScript;
   }

   // script is titlecased in our data
   unsigned char c0 = code.at(0).toUpper()[0].toLatin1();
   unsigned char c1 = code.at(1).toLower()[0].toLatin1();
   unsigned char c2 = code.at(2).toLower()[0].toLatin1();
   unsigned char c3 = code.at(3).toLower()[0].toLatin1();

   const unsigned char *c = script_code_list;

   for (int i = 0; i < QLocale::LastScript; ++i, c += 4) {
      if (c0 == c[0] && c1 == c[1] && c2 == c[2] && c3 == c[3]) {
         return QLocale::Script(i);
      }
   }

   return QLocale::AnyScript;
}

QLocale::Country QLocalePrivate::codeToCountry(const QString &code)
{
   int len = code.length();

   if (len != 2 && len != 3) {
      return QLocale::AnyCountry;
   }

   ushort uc1 = code[0].toUpper()[0].unicode();
   ushort uc2 = code[1].toUpper()[0].unicode();
   ushort uc3 = len > 2 ? code[2].toUpper()[0].unicode() : 0;

   const unsigned char *c = country_code_list;

   for (; *c != 0; c += 3) {
      if (uc1 == c[0] && uc2 == c[1] && uc3 == c[2]) {
         return QLocale::Country((c - country_code_list) / 3);
      }
   }

   return QLocale::AnyCountry;
}

QString QLocalePrivate::languageCode(QLocale::Language language)
{
   if (language == QLocale::AnyLanguage) {
      return QString();
   }

   if (language == QLocale::C) {
      return QString("C");
   }

   const unsigned char *c = language_code_list + 3 * (uint(language));

   QString code;
   code.append(c[0]);
   code.append(c[1]);

   if (c[2] != 0) {
      code.append(c[2]);
   }

   return code;
}

QString QLocalePrivate::scriptCode(QLocale::Script script)
{
   if (script == QLocale::AnyScript || script > QLocale::LastScript) {
      return QString();
   }

   const unsigned char *c = script_code_list + 4 * (uint(script));
   return QString::fromLatin1((const char *)c, 4);
}

QString QLocalePrivate::countryCode(QLocale::Country country)
{
   if (country == QLocale::AnyCountry) {
      return QString();
   }

   const unsigned char *c = country_code_list + 3 * (uint(country));

   QString code;
   code.append(c[0]);
   code.append(c[1]);

   if (c[2] != 0) {
      code.append(c[2]);
   }

   return code;
}

static bool addLikelySubtags(QLocaleId &localeId)
{
   // ### optimize with bsearch
   const int likely_subtags_count = sizeof(likely_subtags) / sizeof(likely_subtags[0]);
   const QLocaleId *p = likely_subtags;
   const QLocaleId *const e = p + likely_subtags_count;

   for ( ; p < e; p += 2) {
      if (localeId == p[0]) {
         localeId = p[1];
         return true;
      }
   }

   return false;
}

QLocaleId QLocaleId::withLikelySubtagsAdded() const
{
   // language_script_region
   if (language_id || script_id || country_id) {
      QLocaleId id = QLocaleId::fromIds(language_id, script_id, country_id);

      if (addLikelySubtags(id)) {
         return id;
      }
   }

   // language_script
   if (country_id) {
      QLocaleId id = QLocaleId::fromIds(language_id, script_id, 0);

      if (addLikelySubtags(id)) {
         id.country_id = country_id;
         return id;
      }
   }

   // language_region
   if (script_id) {
      QLocaleId id = QLocaleId::fromIds(language_id, 0, country_id);

      if (addLikelySubtags(id)) {
         id.script_id = script_id;
         return id;
      }
   }

   // language
   if (script_id && country_id) {
      QLocaleId id = QLocaleId::fromIds(language_id, 0, 0);

      if (addLikelySubtags(id)) {
         id.script_id = script_id;
         id.country_id = country_id;
         return id;
      }
   }

   return *this;
}

QLocaleId QLocaleId::withLikelySubtagsRemoved() const
{
   QLocaleId max = withLikelySubtagsAdded();

   // language
   {
      QLocaleId id = QLocaleId::fromIds(language_id, 0, 0);

      if (id.withLikelySubtagsAdded() == max) {
         return id;
      }
   }

   // language_region
   if (country_id) {
      QLocaleId id = QLocaleId::fromIds(language_id, 0, country_id);

      if (id.withLikelySubtagsAdded() == max) {
         return id;
      }
   }

   // language_script
   if (script_id) {
      QLocaleId id = QLocaleId::fromIds(language_id, script_id, 0);

      if (id.withLikelySubtagsAdded() == max) {
         return id;
      }
   }

   return max;
}

QString QLocaleId::name(char separator) const
{
   if (language_id == QLocale::AnyLanguage) {
      return QString();
   }

   if (language_id == QLocale::C) {
      return QString("C");
   }

   const unsigned char *lang    = language_code_list + 3 * language_id;
   const unsigned char *script  = (script_id  != QLocale::AnyScript  ? script_code_list + 4  * script_id : nullptr);
   const unsigned char *country = (country_id != QLocale::AnyCountry ? country_code_list + 3 * country_id : nullptr);

   QString name;
   name.append(lang[0]);
   name.append(lang[1]);

   if (lang[2] != 0) {
      name.append(lang[2]);
   }

   if (script) {
      name.append(separator);
      name.append(script[0]);
      name.append(script[1]);
      name.append(script[2]);
      name.append(script[3]);
   }

   if (country) {
      name.append(separator);
      name.append(country[0]);
      name.append(country[1]);

      if (country[2] != 0) {
         name.append(country[2]);
      }
   }

   return name;
}

QString QLocalePrivate::bcp47Name(char separator) const
{
   if (m_data->m_language_id == QLocale::AnyLanguage) {
      return QString();
   }

   if (m_data->m_language_id == QLocale::C) {
      return QString("C");
   }

   QLocaleId localeId = QLocaleId::fromIds(m_data->m_language_id, m_data->m_script_id, m_data->m_country_id);

   return localeId.withLikelySubtagsRemoved().name(separator);
}

const QLocaleData *QLocaleData::findLocaleData(QLocale::Language language, QLocale::Script script, QLocale::Country country)
{
   QLocaleId localeId = QLocaleId::fromIds(language, script, country);
   localeId = localeId.withLikelySubtagsAdded();

   uint idx = locale_index[localeId.language_id];

   const QLocaleData *data = locale_data + idx;

   if (idx == 0) {
      // default language has no associated country
      return data;
   }

   Q_ASSERT(data->m_language_id == localeId.language_id);

   if (localeId.script_id != QLocale::AnyScript && localeId.country_id != QLocale::AnyCountry) {
      // both script and country are explicitly specified
      do {
         if (data->m_script_id == localeId.script_id && data->m_country_id == localeId.country_id) {
            return data;
         }

         ++data;

      } while (data->m_language_id == localeId.language_id);

      // no match; try again with default script
      localeId.script_id = QLocale::AnyScript;
      data = locale_data + idx;
   }

   if (localeId.script_id == QLocale::AnyScript && localeId.country_id == QLocale::AnyCountry) {
      return data;
   }

   if (localeId.script_id == QLocale::AnyScript) {
      do {
         if (data->m_country_id == localeId.country_id) {
            return data;
         }

         ++data;

      } while (data->m_language_id == localeId.language_id);

   } else if (localeId.country_id == QLocale::AnyCountry) {
      do {
         if (data->m_script_id == localeId.script_id) {
            return data;
         }

         ++data;
      } while (data->m_language_id == localeId.language_id);
   }

   return locale_data + idx;
}

static bool parse_locale_tag(const QString &input, int &i, QString *result, const QString &separators)
{
   result->clear();

   QString::const_iterator iter = input.begin() + i;

   const int len = input.length();
   int size = 0;

   for (; i < len && size < 8; ++i, ++size) {

      if (separators.contains(*iter)) {
         break;
      }

      char32_t uc = iter->unicode();

      if (! ((uc >= 'a' && uc <= 'z') || (uc >= 'A' && uc <= 'Z') || (uc >= '0' && uc <= '9')) ) {
         // latin only
         return false;
      }

      result->append(*iter);
      ++iter;
   }

   return true;
}

bool qt_splitLocaleName(const QString &name, QString &lang, QString &script, QString &cntry)
{
   const int length         = name.length();
   const QString separators = "_-.@";

   lang.clear();
   script.clear();
   cntry.clear();

   enum ParserState { NoState, LangState, ScriptState, CountryState };
   ParserState state = LangState;

   for (int i = 0; i < length && state != NoState; ) {
      QString value;

      if (! parse_locale_tag(name, i, &value, separators) || value.isEmpty()) {
         break;
      }

      QChar sep = i < length ? name.at(i) : QChar();

      switch (state) {
         case LangState:

            if (! sep.isNull() && ! separators.contains(sep)) {
               state = NoState;
               break;
            }

            lang = value;

            if (i == length) {
               // just language was specified
               state = NoState;
               break;
            }

            state = ScriptState;
            break;

         case ScriptState: {
            QString scripts = QString::fromLatin1((const char *)script_code_list, sizeof(script_code_list) - 1);

            if (value.length() == 4 && scripts.indexOf(value) % 4 == 0) {
               // script name is always 4 characters
               script = value;
               state = CountryState;
            } else {
               // it wasn't a script, maybe it is a country then?
               cntry = value;
               state = NoState;
            }

            break;
         }

         case CountryState:
            cntry = value;
            state = NoState;
            break;

         case NoState:
            qWarning("QLocale() Invalid state");
            break;
      }

      ++i;
   }

   return lang.length() == 2 || lang.length() == 3;
}

void QLocalePrivate::getLangAndCountry(const QString &name, QLocale::Language &lang,
      QLocale::Script &script, QLocale::Country &cntry)
{
   lang = QLocale::C;
   script = QLocale::AnyScript;
   cntry = QLocale::AnyCountry;

   QString lang_code;
   QString script_code;
   QString cntry_code;

   if (!qt_splitLocaleName(name, lang_code, script_code, cntry_code)) {
      return;
   }

   lang = QLocalePrivate::codeToLanguage(lang_code);

   if (lang == QLocale::C) {
      return;
   }

   script = QLocalePrivate::codeToScript(script_code);
   cntry = QLocalePrivate::codeToCountry(cntry_code);
}

static const QLocaleData *findLocaleData(const QString &name)
{
   QLocale::Language lang;
   QLocale::Script script;
   QLocale::Country cntry;
   QLocalePrivate::getLangAndCountry(name, lang, script, cntry);

   return QLocaleData::findLocaleData(lang, script, cntry);
}

QString qt_readEscapedFormatString(const QString &format, int *idx)
{
   int &i = *idx;

   Q_ASSERT(format.at(i) == '\'');
   ++i;

   if (i == format.size()) {
      return QString();
   }

   if (format.at(i).unicode() == '\'') {
      // "''" outside of a quoted stirng

      ++i;
      return QString("'");
   }

   QString result;

   while (i < format.size()) {
      if (format.at(i).unicode() == '\'') {

         if (i + 1 < format.size() && format.at(i + 1).unicode() == '\'') {
            // "''" inside of a quoted string
            result.append(QLatin1Char('\''));
            i += 2;

         } else {
            break;
         }

      } else {
         result.append(format.at(i++));
      }
   }

   if (i < format.size()) {
      ++i;
   }

   return result;
}

int qt_repeatCount(const QString &s, int i)
{
   QChar c = s.at(i);
   int j = i + 1;

   while (j < s.size() && s.at(j) == c) {
      ++j;
   }

   return j - i;
}

static QLocalePrivate *c_private()
{
   static QLocalePrivate c_locale = { c_data, 1, QLocale::OmitGroupSeparator };
   return &c_locale;
}

#ifndef QT_NO_SYSTEMLOCALE

QSystemLocale::QSystemLocale()
{
   delete s_system_locale;
   s_system_locale = this;

   if (s_system_data) {
      s_system_data->m_language_id = 0;
   }
}

QSystemLocale::QSystemLocale(QSystemLocale::cs_internal_tag)
{
}

QSystemLocale::~QSystemLocale()
{
   if (s_system_locale == this) {
      s_system_locale = nullptr;

      if (s_system_data != nullptr) {
         s_system_data->m_language_id = 0;
      }
   }
}

static const QSystemLocale *systemLocale()
{
   if (s_system_locale != nullptr) {
      return s_system_locale;
   }

   return global_SystemLocale();
}

void QLocalePrivate::updateSystemPrivate()
{
   const QSystemLocale *current_locale = systemLocale();

   if (s_system_data == nullptr) {
      s_system_data = global_SystemData();
   }

   // system locale has changed
   current_locale->query(QSystemLocale::LocaleChanged, QVariant());

   *s_system_data = *current_locale->fallbackUiLocale().d->m_data;

   QVariant res   = current_locale->query(QSystemLocale::LanguageId, QVariant());

   if (res.isValid()) {
      s_system_data->m_language_id = res.toInt();
      s_system_data->m_script_id   = QLocale::AnyScript;       // default for compatibility
   }

   res = current_locale->query(QSystemLocale::CountryId, QVariant());

   if (res.isValid()) {
      s_system_data->m_country_id = res.toInt();
      s_system_data->m_script_id  = QLocale::AnyScript;        // default for compatibility
   }

   res = current_locale->query(QSystemLocale::ScriptId, QVariant());

   if (res.isValid()) {
      s_system_data->m_script_id = res.toInt();
   }

   res = current_locale->query(QSystemLocale::DecimalPoint, QVariant());

   if (res.isValid()) {
      s_system_data->m_decimal = res.toString().at(0).unicode();
   }

   res = current_locale->query(QSystemLocale::GroupSeparator, QVariant());

   if (res.isValid()) {
      s_system_data->m_group = res.toString().at(0).unicode();
   }

   res = current_locale->query(QSystemLocale::ZeroDigit, QVariant());

   if (res.isValid()) {
      s_system_data->m_zero = res.toString().at(0).unicode();
   }

   res = current_locale->query(QSystemLocale::NegativeSign, QVariant());

   if (res.isValid()) {
      s_system_data->m_minus = res.toString().at(0).unicode();
   }

   res = current_locale->query(QSystemLocale::PositiveSign, QVariant());

   if (res.isValid()) {
      s_system_data->m_plus = res.toString().at(0).unicode();
   }
}
#endif

static const QLocaleData *systemData()
{
#ifndef QT_NO_SYSTEMLOCALE

   // copy the information from the fallback locale and modify
   if (! s_system_data || s_system_data->m_language_id == 0) {
      QLocalePrivate::updateSystemPrivate();
   }

   return s_system_data;

#else
   return locale_data;

#endif
}

const QLocaleData *defaultData()
{
   if (! default_data) {
      default_data = systemData();
   }

   return default_data;
}

const QLocaleData *QLocaleData::c()
{
   Q_ASSERT(locale_index[QLocale::C] == 0);
   return c_data;
}

static inline QString getLocaleData(const ushort *data, int size)
{
   if (size > 0) {
      return QString::fromUtf16(reinterpret_cast<const char16_t *>(data), size);

   } else {
      return QString();
   }
}

static QString getLocaleListData(const ushort *data, int size, int index)
{
   static const ushort separator = ';';

   while (index && size > 0) {
      while (*data != separator) {
         ++data, --size;
      }

      --index;
      ++data;
      --size;
   }

   const ushort *end = data;

   while (size > 0 && *end != separator) {
      ++end;
      --size;
   }

   return getLocaleData(data, end - data);
}

QDataStream &operator<<(QDataStream &stream, const QLocale &locale)
{
   stream << locale.name();
   return stream;
}

QDataStream &operator>>(QDataStream &stream, QLocale &locale)
{
   QString str;
   stream >> str;

   locale = QLocale(str);

   return stream;
}

static QLocalePrivate *localePrivateByName(const QString &name)
{
   if (name == "C") {
      return c_private();
   }

   const QLocaleData *data = findLocaleData(name);
   return QLocalePrivate::create(data, data->m_language_id == QLocale::C ? QLocale::OmitGroupSeparator : 0);
}

static QLocalePrivate *findLocalePrivate(QLocale::Language language, QLocale::Script script, QLocale::Country country)
{
   if (language == QLocale::C) {
      return c_private();
   }

   const QLocaleData *data = QLocaleData::findLocaleData(language, script, country);

   int numberOptions = 0;

   // If not found, should default to system
   if (data->m_language_id == QLocale::C && language != QLocale::C) {
      numberOptions = default_number_options;
      data = defaultData();
   }

   return QLocalePrivate::create(data, numberOptions);
}

QLocale::QLocale(QLocalePrivate &dd)
   : d(&dd)
{
}

QLocale::QLocale(const QString &name)
   : d(localePrivateByName(name))
{
}

QLocale::QLocale()
   : d(*defaultLocalePrivate())
{
}

QLocale::QLocale(Language language, Country country)
   : d(findLocalePrivate(language, QLocale::AnyScript, country))
{
}

QLocale::QLocale(Language language, Script script, Country country)
   : d(findLocalePrivate(language, script, country))
{
}

QLocale::~QLocale()
{
}

QLocale::QLocale(const QLocale &other) = default;
QLocale::QLocale(QLocale &&other) = default;

QLocale &QLocale::operator=(const QLocale &other) = default;
QLocale &QLocale::operator=(QLocale &&other) = default;

bool QLocale::operator==(const QLocale &other) const
{
   return d->m_data == other.d->m_data && d->m_numberOptions == other.d->m_numberOptions;
}

bool QLocale::operator!=(const QLocale &other) const
{
   return d->m_data != other.d->m_data || d->m_numberOptions != other.d->m_numberOptions;
}

uint qHash(const QLocale &key, uint seed)
{
   seed = qHash(key.d->m_data, seed);
   seed = qHash(key.d->m_numberOptions, seed);

   return seed;
}

void QLocale::setNumberOptions(NumberOptions options)
{
   d->m_numberOptions = options;
}

QLocale::NumberOptions QLocale::numberOptions() const
{
   return static_cast<NumberOption>(d->m_numberOptions);
}

QString QLocale::quoteString(const QString &str, QuotationStyle style) const
{
   return quoteString(QStringView(str), style);
}

QString QLocale::quoteString(QStringView str, QuotationStyle style) const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant retval;

      if (style == QLocale::AlternateQuotation) {
         retval = systemLocale()->query(QSystemLocale::StringToAlternateQuotation, QVariant::fromValue(str));
      }

      if (! retval.isValid() || style == QLocale::StandardQuotation) {
         retval = systemLocale()->query(QSystemLocale::StringToStandardQuotation, QVariant::fromValue(str));
      }

      if (retval.isValid()) {
         return retval.toString();
      }
   }

#endif

   if (style == QLocale::StandardQuotation) {
      return QChar(d->m_data->m_quotation_start) + str + QChar(d->m_data->m_quotation_end);

   } else {
      return QChar(d->m_data->m_alternate_quotation_start) + str + QChar(d->m_data->m_alternate_quotation_end);
   }
}

QString QLocale::createSeparatedList(const QStringList &list) const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(QSystemLocale::ListToSeparatedString, QVariant::fromValue(list));

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   const int size = list.size();

   if (size == 1) {
      return list.at(0);

   } else if (size == 2) {
      QString format = getLocaleData(list_pattern_part_data + d->m_data->m_list_pattern_part_two_idx, d->m_data->m_list_pattern_part_two_size);
      return format.formatArgs(list.at(0), list.at(1));

   } else if (size > 2) {
      QString formatStart = getLocaleData(list_pattern_part_data + d->m_data->m_list_pattern_part_start_idx,
            d->m_data->m_list_pattern_part_start_size);

      QString formatMid = getLocaleData(list_pattern_part_data + d->m_data->m_list_pattern_part_mid_idx,
            d->m_data->m_list_pattern_part_mid_size);

      QString formatEnd = getLocaleData(list_pattern_part_data + d->m_data->m_list_pattern_part_end_idx,
            d->m_data->m_list_pattern_part_end_size);

      QString result = formatStart.formatArgs(list.at(0), list.at(1));

      for (int i = 2; i < size - 1; ++i) {
         result = formatMid.formatArgs(result, list.at(i));
      }

      result = formatEnd.formatArgs(result, list.at(size - 1));
      return result;
   }

   return QString();
}

void QLocale::setDefault(const QLocale &locale)
{
   default_data = locale.d->m_data;
   default_number_options = locale.numberOptions();

   if (defaultLocalePrivate()->data() != nullptr) {
      // update the cached private
      *defaultLocalePrivate() = locale.d;
   }
}

QLocale::Language QLocale::language() const
{
   return Language(d->languageId());
}

QLocale::Script QLocale::script() const
{
   return Script(d->m_data->m_script_id);
}

QLocale::Country QLocale::country() const
{
   return Country(d->countryId());
}

QString QLocale::name() const
{
   Language l = language();
   QString result = d->languageCode();

   if (l == C) {
      return result;
   }

   Country c = country();

   if (c == AnyCountry) {
      return result;
   }

   result.append('_');
   result.append(d->countryCode());

   return result;
}

template <typename T>
static T cs_internal_integral(const QLocalePrivate *d, const QString &data, int base, bool *ok)
{
   using Int64 = typename std::conditional<std::is_unsigned<T>::value, quint64, qint64>::type;
   Int64 val = cs_internal_integral<Int64>(d, data, base, ok);

   if (val < std::numeric_limits<T>::min() || val > std::numeric_limits<T>::max()) {
      if (ok) {
         *ok = false;
      }

      val = 0;
   }

   return T(val);
}

template <>
qint64 cs_internal_integral<qint64>(const QLocalePrivate *d, const QString &data, int base, bool *ok)
{
   QLocaleData::GroupSeparatorMode mode = d->m_numberOptions & QLocale::RejectGroupSeparator
         ? QLocaleData::FailOnGroupSeparators : QLocaleData::ParseGroupSeparators;

   return d->m_data->stringToLongLong(data, base, ok, mode);
}

template <>
quint64 cs_internal_integral<quint64>(const QLocalePrivate *d, const QString &data, int base, bool *ok)
{
   QLocaleData::GroupSeparatorMode mode = d->m_numberOptions & QLocale::RejectGroupSeparator
         ? QLocaleData::FailOnGroupSeparators : QLocaleData::ParseGroupSeparators;

   return d->m_data->stringToUnsLongLong(data, base, ok, mode);
}

QString QLocale::bcp47Name() const
{
   return d->bcp47Name();
}

QString QLocale::languageToString(Language language)
{
   if (uint(language) > uint(QLocale::LastLanguage)) {
      return QString("Unknown");
   }

   return QString::fromUtf8(language_name_list + language_name_index[language]);
}

QString QLocale::countryToString(Country country)
{
   if (uint(country) > uint(QLocale::LastCountry)) {
      return QString("Unknown");
   }

   return QString::fromUtf8(country_name_list + country_name_index[country]);
}

QString QLocale::scriptToString(QLocale::Script script)
{
   if (uint(script) > uint(QLocale::LastScript)) {
      return QString("Unknown");
   }

   return QString::fromUtf8(script_name_list + script_name_index[script]);
}

short QLocale::toShort(const QString &s, bool *ok, int base) const
{
   return cs_internal_integral<short>(d, s, base, ok);
}

ushort QLocale::toUShort(const QString &s, bool *ok, int base) const
{
   return cs_internal_integral<ushort>(d, s, base, ok);
}

int QLocale::toInt(const QString &s, bool *ok, int base) const
{
   return cs_internal_integral<int>(d, s, base, ok);
}

uint QLocale::toUInt(const QString &s, bool *ok, int base) const
{
   return cs_internal_integral<uint>(d, s, base, ok);
}

qint64 QLocale::toLongLong(const QString &s, bool *ok, int base) const
{
   return cs_internal_integral<qint64>(d, s, base, ok);
}

quint64 QLocale::toULongLong(const QString &s, bool *ok, int base) const
{
   return cs_internal_integral<quint64>(d, s, base, ok);
}

float QLocale::toFloat(const QString &s, bool *ok) const
{
   bool isOk;
   double d = toDouble(s, &isOk);

   if (! isOk || d < std::numeric_limits<float>::lowest() || d > std::numeric_limits<float>::max() )  {

      if (ok != nullptr) {
         *ok = false;
      }

      return 0.0;
   }

   if (ok != nullptr) {
      *ok = true;
   }

   return float(d);
}

double QLocale::toDouble(const QString &s, bool *ok) const
{
   QLocaleData::GroupSeparatorMode mode = d->m_numberOptions & RejectGroupSeparator
         ? QLocaleData::FailOnGroupSeparators : QLocaleData::ParseGroupSeparators;

   return d->m_data->stringToDouble(s, ok, mode);
}

QString QLocale::toString(qint64 value) const
{
   int flags = d->m_numberOptions & OmitGroupSeparator ? 0 : QLocaleData::ThousandsGroup;

   return d->m_data->longLongToString(value, -1, 10, -1, flags);
}

QString QLocale::toString(quint64 value) const
{
   int flags = d->m_numberOptions & OmitGroupSeparator ? 0 : QLocaleData::ThousandsGroup;

   return d->m_data->unsLongLongToString(value, -1, 10, -1, flags);
}

QString QLocale::toString(const QDate &date, const QString &format) const
{
   return d->dateTimeToString(format, QDateTime(), date, QTime(), this);
}

QString QLocale::toString(const QDate &date, FormatType formatType) const
{
   if (! date.isValid()) {
      return QString();
   }

#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(formatType == LongFormat
            ? QSystemLocale::DateToStringLong : QSystemLocale::DateToStringShort, date);

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   QString format_str = dateFormat(formatType);
   return toString(date, format_str);
}

static bool timeFormatContainsAP(const QString &format)
{
   int i = 0;

   while (i < format.size()) {
      if (format.at(i).unicode() == '\'') {
         qt_readEscapedFormatString(format, &i);
         continue;
      }

      if (format.at(i).toLower() == "a") {
         return true;
      }

      ++i;
   }

   return false;
}

QString QLocale::toString(const QTime &time, const QString &format) const
{
   return d->dateTimeToString(format, QDateTime(), QDate(), time, this);
}

QString QLocale::toString(const QDateTime &dateTime, const QString &format) const
{
   return d->dateTimeToString(format, dateTime, QDate(), QTime(), this);
}

QString QLocale::toString(const QDateTime &dateTime, FormatType formatType) const
{
   if (! dateTime.isValid()) {
      return QString();
   }

#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(formatType == LongFormat
            ? QSystemLocale::DateTimeToStringLong : QSystemLocale::DateTimeToStringShort, dateTime);

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   const QString format_str = dateTimeFormat(formatType);
   return toString(dateTime, format_str);
}

QString QLocale::toString(const QTime &time, FormatType formatType) const
{
   if (! time.isValid()) {
      return QString();
   }

#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(formatType == LongFormat
            ? QSystemLocale::TimeToStringLong : QSystemLocale::TimeToStringShort, time);

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   QString format_str = timeFormat(formatType);

   return toString(time, format_str);
}

QString QLocale::dateFormat(FormatType formatType) const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(formatType == LongFormat
            ? QSystemLocale::DateFormatLong : QSystemLocale::DateFormatShort, QVariant());

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   quint32 idx, size;

   switch (formatType) {
      case LongFormat:
         idx  = d->m_data->m_long_date_format_idx;
         size = d->m_data->m_long_date_format_size;
         break;

      default:
         idx  = d->m_data->m_short_date_format_idx;
         size = d->m_data->m_short_date_format_size;
         break;
   }

   return getLocaleData(date_format_data + idx, size);
}

QString QLocale::timeFormat(FormatType formatType) const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(formatType == LongFormat
            ? QSystemLocale::TimeFormatLong : QSystemLocale::TimeFormatShort, QVariant());

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   quint32 idx;
   quint32 size;

   switch (formatType) {
      case LongFormat:
         idx  = d->m_data->m_long_time_format_idx;
         size = d->m_data->m_long_time_format_size;
         break;

      default:
         idx  = d->m_data->m_short_time_format_idx;
         size = d->m_data->m_short_time_format_size;
         break;
   }

   return getLocaleData(time_format_data + idx, size);
}

QString QLocale::dateTimeFormat(FormatType formatType) const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(formatType == LongFormat
            ? QSystemLocale::DateTimeFormatLong : QSystemLocale::DateTimeFormatShort, QVariant());

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   return dateFormat(formatType) + ' ' + timeFormat(formatType);
}

QTime QLocale::toTime(const QString &string, FormatType formatType) const
{
   return toTime(string, timeFormat(formatType));
}

QDate QLocale::toDate(const QString &string, FormatType formatType) const
{
   return toDate(string, dateFormat(formatType));
}

QDateTime QLocale::toDateTime(const QString &string, FormatType formatType) const
{
   return toDateTime(string, dateTimeFormat(formatType));
}

QTime QLocale::toTime(const QString &string, const QString &format) const
{
   QTime time;

   QDateTimeParser dt(QVariant::Time, QDateTimeParser::FromString);

   dt.setDefaultLocale(*this);

   if (dt.parseFormat(format)) {
      dt.fromString(string, nullptr, &time);
   }

   return time;
}

QDate QLocale::toDate(const QString &string, const QString &format) const
{
   QDate date;

   QDateTimeParser dt(QVariant::Date, QDateTimeParser::FromString);

   dt.setDefaultLocale(*this);

   if (dt.parseFormat(format)) {
      dt.fromString(string, &date, nullptr);
   }

   return date;
}

QDateTime QLocale::toDateTime(const QString &string, const QString &format) const
{
   QTime time;
   QDate date;

   QDateTimeParser dt(QVariant::DateTime, QDateTimeParser::FromString);

   dt.setDefaultLocale(*this);

   if (dt.parseFormat(format) && dt.fromString(string, &date, &time)) {
      return QDateTime(date, time);
   }

   return QDateTime(QDate(), QTime(-1, -1, -1));
}

QChar QLocale::decimalPoint() const
{
   return d->decimal();
}

QChar QLocale::groupSeparator() const
{
   return d->group();
}

QChar QLocale::percent() const
{
   return d->percent();
}

QChar QLocale::zeroDigit() const
{
   return d->zero();
}

QChar QLocale::negativeSign() const
{
   return d->minus();
}

QChar QLocale::positiveSign() const
{
   return d->plus();
}

QChar QLocale::exponential() const
{
   return d->exponential();
}

static bool qIsUpper(char c)
{
   return c >= 'A' && c <= 'Z';
}

static char qToLower(char c)
{
   if (c >= 'A' && c <= 'Z') {
      return c - 'A' + 'a';
   } else {
      return c;
   }
}

QString QLocale::toString(double value, char f, int prec) const
{
   QLocaleData::DoubleForm form = QLocaleData::DFDecimal;
   uint flags = 0;

   if (qIsUpper(f)) {
      flags = QLocaleData::CapitalEorX;
   }

   f = qToLower(f);

   switch (f) {
      case 'f':
         form = QLocaleData::DFDecimal;
         break;

      case 'e':
         form = QLocaleData::DFExponent;
         break;

      case 'g':
         form = QLocaleData::DFSignificantDigits;
         break;

      default:
         break;
   }

   if (! (d->m_numberOptions & OmitGroupSeparator)) {
      flags |= QLocaleData::ThousandsGroup;
   }

   return d->m_data->doubleToString(value, prec, form, -1, flags);
}

QLocale QLocale::system()
{
   return QLocale(*QLocalePrivate::create(systemData()));
}

QList<QLocale> QLocale::matchingLocales(QLocale::Language language, QLocale::Script script, QLocale::Country country)
{
   if (uint(language) > QLocale::LastLanguage || uint(script) > QLocale::LastScript || uint(country) > QLocale::LastCountry) {
      return QList<QLocale>();
   }

   if (language == QLocale::C) {
      return QList<QLocale>() << QLocale(QLocale::C);
   }

   QList<QLocale> result;

   const QLocaleData *data = locale_data + locale_index[language];

   while ( (data != locale_data + locale_data_size)
         && (language == QLocale::AnyLanguage || data->m_language_id == uint(language))) {

      if ((script == QLocale::AnyScript || data->m_script_id == uint(script))
            && (country == QLocale::AnyCountry || data->m_country_id == uint(country))) {

         QLocale locale(*QLocalePrivate::create(data));
         result.append(locale);
      }

      ++data;
   }

   return result;
}

QString QLocale::monthName(int month, FormatType type) const
{
   if (month < 1 || month > 12) {
      return QString();
   }

#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {

      QVariant res = systemLocale()->query(type == LongFormat
            ? QSystemLocale::MonthNameLong : QSystemLocale::MonthNameShort, month);

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   quint32 idx, size;

   switch (type) {
      case QLocale::LongFormat:
         idx = d->m_data->m_long_month_names_idx;
         size = d->m_data->m_long_month_names_size;
         break;

      case QLocale::ShortFormat:
         idx = d->m_data->m_short_month_names_idx;
         size = d->m_data->m_short_month_names_size;
         break;

      case QLocale::NarrowFormat:
         idx = d->m_data->m_narrow_month_names_idx;
         size = d->m_data->m_narrow_month_names_size;
         break;

      default:
         return QString();
   }

   return getLocaleListData(months_data + idx, size, month - 1);
}

QString QLocale::standaloneMonthName(int month, FormatType type) const
{
   if (month < 1 || month > 12) {
      return QString();
   }

#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(type == LongFormat
            ? QSystemLocale::StandaloneMonthNameLong : QSystemLocale::StandaloneMonthNameShort, month);

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   quint32 idx, size;

   switch (type) {
      case QLocale::LongFormat:
         idx = d->m_data->m_standalone_long_month_names_idx;
         size = d->m_data->m_standalone_long_month_names_size;
         break;

      case QLocale::ShortFormat:
         idx = d->m_data->m_standalone_short_month_names_idx;
         size = d->m_data->m_standalone_short_month_names_size;
         break;

      case QLocale::NarrowFormat:
         idx = d->m_data->m_standalone_narrow_month_names_idx;
         size = d->m_data->m_standalone_narrow_month_names_size;
         break;

      default:
         return QString();
   }

   QString name = getLocaleListData(months_data + idx, size, month - 1);

   if (name.isEmpty()) {
      return monthName(month, type);
   }

   return name;
}

QString QLocale::dayName(int day, FormatType type) const
{
   if (day < 1 || day > 7) {
      return QString();
   }

#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(type == LongFormat
            ? QSystemLocale::DayNameLong : QSystemLocale::DayNameShort, day);

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   if (day == 7) {
      day = 0;
   }

   quint32 idx, size;

   switch (type) {
      case QLocale::LongFormat:
         idx = d->m_data->m_long_day_names_idx;
         size = d->m_data->m_long_day_names_size;
         break;

      case QLocale::ShortFormat:
         idx = d->m_data->m_short_day_names_idx;
         size = d->m_data->m_short_day_names_size;
         break;

      case QLocale::NarrowFormat:
         idx = d->m_data->m_narrow_day_names_idx;
         size = d->m_data->m_narrow_day_names_size;
         break;

      default:
         return QString();
   }

   return getLocaleListData(days_data + idx, size, day);
}

QString QLocale::standaloneDayName(int day, FormatType type) const
{
   if (day < 1 || day > 7) {
      return QString();
   }

#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(type == LongFormat
            ? QSystemLocale::DayNameLong : QSystemLocale::DayNameShort, day);

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   if (day == 7) {
      day = 0;
   }

   quint32 idx, size;

   switch (type) {
      case QLocale::LongFormat:
         idx = d->m_data->m_standalone_long_day_names_idx;
         size = d->m_data->m_standalone_long_day_names_size;
         break;

      case QLocale::ShortFormat:
         idx = d->m_data->m_standalone_short_day_names_idx;
         size = d->m_data->m_standalone_short_day_names_size;
         break;

      case QLocale::NarrowFormat:
         idx = d->m_data->m_standalone_narrow_day_names_idx;
         size = d->m_data->m_standalone_narrow_day_names_size;
         break;

      default:
         return QString();
   }

   QString name = getLocaleListData(days_data + idx, size, day);

   if (name.isEmpty()) {
      return dayName(day == 0 ? 7 : day, type);
   }

   return name;
}

Qt::DayOfWeek QLocale::firstDayOfWeek() const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(QSystemLocale::FirstDayOfWeek, QVariant());

      if (res.isValid()) {
         return static_cast<Qt::DayOfWeek>(res.toUInt());
      }
   }

#endif

   return static_cast<Qt::DayOfWeek>(d->m_data->m_first_day_of_week);
}

QLocale::MeasurementSystem QLocalePrivate::measurementSystem() const
{
   for (int i = 0; i < ImperialMeasurementSystemsCount; ++i) {
      if (ImperialMeasurementSystems[i].languageId == m_data->m_language_id
            && ImperialMeasurementSystems[i].countryId == m_data->m_country_id) {
         return ImperialMeasurementSystems[i].system;
      }
   }

   return QLocale::MetricSystem;
}

QList<Qt::DayOfWeek> QLocale::weekdays() const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(QSystemLocale::Weekdays, QVariant());

      if (res.isValid()) {
         return static_cast<QList<Qt::DayOfWeek>>(res.value<QList<Qt::DayOfWeek>>());
      }
   }

#endif

   QList<Qt::DayOfWeek> weekdays;
   quint16 weekendStart = d->m_data->m_weekend_start;
   quint16 weekendEnd = d->m_data->m_weekend_end;

   for (int day = Qt::Monday; day <= Qt::Sunday; day++) {
      if ((weekendEnd >= weekendStart && (day < weekendStart || day > weekendEnd)) ||
            (weekendEnd < weekendStart && (day > weekendEnd && day < weekendStart))) {
         weekdays << static_cast<Qt::DayOfWeek>(day);
      }
   }

   return weekdays;
}

QLocale::MeasurementSystem QLocale::measurementSystem() const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(QSystemLocale::MeasurementSystem, QVariant());

      if (res.isValid()) {
         return MeasurementSystem(res.toInt());
      }
   }

#endif

   return d->measurementSystem();
}

Qt::LayoutDirection QLocale::textDirection() const
{
   switch (script()) {
      case QLocale::ArabicScript:
      case QLocale::AvestanScript:
      case QLocale::CypriotScript:
      case QLocale::HebrewScript:
      case QLocale::ImperialAramaicScript:
      case QLocale::InscriptionalPahlaviScript:
      case QLocale::InscriptionalParthianScript:
      case QLocale::KharoshthiScript:
      case QLocale::LydianScript:
      case QLocale::MandaeanScript:
      case QLocale::ManichaeanScript:
      case QLocale::MendeKikakuiScript:
      case QLocale::MeroiticCursiveScript:
      case QLocale::MeroiticScript:
      case QLocale::NabataeanScript:
      case QLocale::NkoScript:
      case QLocale::OldNorthArabianScript:
      case QLocale::OldSouthArabianScript:
      case QLocale::OrkhonScript:
      case QLocale::PalmyreneScript:
      case QLocale::PhoenicianScript:
      case QLocale::PsalterPahlaviScript:
      case QLocale::SamaritanScript:
      case QLocale::SyriacScript:
      case QLocale::ThaanaScript:
         return Qt::RightToLeft;

      default:
         break;
   }

   return Qt::LeftToRight;
}

QString QLocale::toUpper(const QString &str) const
{

   return str.toUpper();
}

QString QLocale::toLower(const QString &str) const
{

   return str.toLower();
}

QString QLocale::amText() const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(QSystemLocale::AMText, QVariant());

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   return getLocaleData(am_data + d->m_data->m_am_idx, d->m_data->m_am_size);
}

QString QLocale::pmText() const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(QSystemLocale::PMText, QVariant());

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif
   return getLocaleData(pm_data + d->m_data->m_pm_idx, d->m_data->m_pm_size);
}

QString QLocalePrivate::dateTimeToString(const QString &format, const QDateTime &datetime,
      const QDate &dateOnly, const QTime &timeOnly, const QLocale *q) const
{
   QDate date;
   QTime time;

   bool formatDate = false;
   bool formatTime = false;

   if (datetime.isValid()) {
      date = datetime.date();
      time = datetime.time();
      formatDate = true;
      formatTime = true;

   } else if (dateOnly.isValid()) {
      date = dateOnly;
      formatDate = true;

   } else if (timeOnly.isValid()) {
      time = timeOnly;
      formatTime = true;

   } else {
      return QString();
   }

   QString result;

   int i = 0;

   while (i < format.size()) {
      if (format.at(i).unicode() == '\'') {
         result.append(qt_readEscapedFormatString(format, &i));
         continue;
      }

      const QChar c = format.at(i);
      int repeat = qt_repeatCount(format, i);
      bool used = false;

      if (formatDate) {
         switch (c.unicode()) {
            case 'y':
               used = true;

               if (repeat >= 4) {
                  repeat = 4;
               } else if (repeat >= 2) {
                  repeat = 2;
               }

               switch (repeat) {
                  case 4: {
                     const int yr = date.year();
                     const int len = (yr < 0) ? 5 : 4;
                     result.append(m_data->longLongToString(yr, -1, 10, len, QLocaleData::ZeroPadded));
                     break;
                  }

                  case 2:
                     result.append(m_data->longLongToString(date.year() % 100, -1, 10, 2, QLocaleData::ZeroPadded));
                     break;

                  default:
                     repeat = 1;
                     result.append(c);
                     break;
               }

               break;

            case 'M':
               used = true;
               repeat = qMin(repeat, 4);

               switch (repeat) {
                  case 1:
                     result.append(m_data->longLongToString(date.month()));
                     break;

                  case 2:
                     result.append(m_data->longLongToString(date.month(), -1, 10, 2, QLocaleData::ZeroPadded));
                     break;

                  case 3:
                     result.append(q->monthName(date.month(), QLocale::ShortFormat));
                     break;

                  case 4:
                     result.append(q->monthName(date.month(), QLocale::LongFormat));
                     break;
               }

               break;

            case 'd':
               used = true;
               repeat = qMin(repeat, 4);

               switch (repeat) {
                  case 1:
                     result.append(m_data->longLongToString(date.day()));
                     break;

                  case 2:
                     result.append(m_data->longLongToString(date.day(), -1, 10, 2, QLocaleData::ZeroPadded));
                     break;

                  case 3:
                     result.append(q->dayName(date.dayOfWeek(), QLocale::ShortFormat));
                     break;

                  case 4:
                     result.append(q->dayName(date.dayOfWeek(), QLocale::LongFormat));
                     break;
               }

               break;

            default:
               break;
         }
      }

      if (! used && formatTime) {
         switch (c.unicode()) {
            case 'h': {
               used = true;
               repeat = qMin(repeat, 2);
               int hour = time.hour();

               if (timeFormatContainsAP(format)) {
                  if (hour > 12) {
                     hour -= 12;
                  } else if (hour == 0) {
                     hour = 12;
                  }
               }

               switch (repeat) {
                  case 1:
                     result.append(m_data->longLongToString(hour));
                     break;

                  case 2:
                     result.append(m_data->longLongToString(hour, -1, 10, 2, QLocaleData::ZeroPadded));
                     break;
               }

               break;
            }

            case 'H':
               used = true;
               repeat = qMin(repeat, 2);

               switch (repeat) {
                  case 1:
                     result.append(m_data->longLongToString(time.hour()));
                     break;

                  case 2:
                     result.append(m_data->longLongToString(time.hour(), -1, 10, 2, QLocaleData::ZeroPadded));
                     break;
               }

               break;

            case 'm':
               used = true;
               repeat = qMin(repeat, 2);

               switch (repeat) {
                  case 1:
                     result.append(m_data->longLongToString(time.minute()));
                     break;

                  case 2:
                     result.append(m_data->longLongToString(time.minute(), -1, 10, 2, QLocaleData::ZeroPadded));
                     break;
               }

               break;

            case 's':
               used = true;
               repeat = qMin(repeat, 2);

               switch (repeat) {
                  case 1:
                     result.append(m_data->longLongToString(time.second()));
                     break;

                  case 2:
                     result.append(m_data->longLongToString(time.second(), -1, 10, 2, QLocaleData::ZeroPadded));
                     break;
               }

               break;

            case 'a':
               used = true;

               if (i + 1 < format.length() && format.at(i + 1).unicode() == 'p') {
                  repeat = 2;
               } else {
                  repeat = 1;
               }

               result.append(time.hour() < 12 ? q->amText().toLower() : q->pmText().toLower());
               break;

            case 'A':
               used = true;

               if (i + 1 < format.length() && format.at(i + 1).unicode() == 'P') {
                  repeat = 2;
               } else {
                  repeat = 1;
               }

               result.append(time.hour() < 12 ? q->amText().toUpper() : q->pmText().toUpper());
               break;

            case 'z':
               used = true;

               if (repeat >= 3) {
                  repeat = 3;
               } else {
                  repeat = 1;
               }

               switch (repeat) {
                  case 1:
                     result.append(m_data->longLongToString(time.msec()));
                     break;

                  case 3:
                     result.append(m_data->longLongToString(time.msec(), -1, 10, 3, QLocaleData::ZeroPadded));
                     break;
               }

               break;

            case 't':
               used = true;
               repeat = 1;

               if (formatDate) {
                  result.append(datetime.timeZoneAbbreviation());
               } else {
                  result.append(QDateTime::currentDateTime().timeZoneAbbreviation());
               }

               break;

            default:
               break;
         }
      }

      if (! used) {
         result.append(QString(repeat, c));
      }

      i += repeat;
   }

   return result;
}

QString QLocaleData::doubleToString(double d, int precision, DoubleForm form, int width, unsigned flags) const
{
   return doubleToString(m_zero, m_plus, m_minus, m_exponential, m_group, m_decimal, d, precision, form, width, flags);
}

QString QLocaleData::doubleToString(const QChar _zero, const QChar plus, const QChar minus,
      const QChar exponential, const QChar group, const QChar decimal,
      double d, int precision, DoubleForm form, int width, unsigned flags)
{
   if (precision < 0) {
      precision = 6;
   }

   if (width < 0) {
      width = 0;
   }

   bool negative = false;
   bool special_number = false;             // nan, +/-inf
   QString num_str;

   // Detect special numbers (nan, +/-inf)
   if (qt_is_inf(d)) {
      num_str  = QString::fromLatin1("inf");
      special_number = true;
      negative = d < 0;

   } else if (qt_is_nan(d)) {
      num_str = QString::fromLatin1("nan");
      special_number = true;
   }

   // Handle normal numbers
   if (! special_number) {
      int decpt, sign;
      QString digits;

      int mode;

      if (form == DFDecimal) {
         mode = 3;
      } else {
         mode = 2;
      }

      /* This next bit is a bit quirky. In DFExponent form, the precision
         is the number of digits after decpt. So that would suggest using
         mode=3 for qdtoa. But qdtoa behaves strangely when mode=3 and
         precision=0. So we get around this by using mode=2 and reasoning
         that we want precision+1 significant digits, since the decimal
         point in this mode is always after the first digit. */

      int pr = precision;

      if (form == DFExponent) {
         ++pr;
      }

      char *rve  = nullptr;
      char *buff = nullptr;

      try {
         digits = qdtoa(d, mode, pr, &decpt, &sign, &rve, &buff);

      } catch (...) {
         if (buff != nullptr) {
            free(buff);
         }

         throw;
      }

      if (buff != nullptr) {
         free(buff);
      }

      if (_zero.unicode() != '0') {
         ushort z = _zero.unicode() - '0';

         QString tmp;

         for (QChar ch : digits) {
            tmp.append( char32_t(ch.unicode() + z) );
         }

         digits = tmp;
      }

      bool always_show_decpt = (flags & Alternate || flags & ForcePoint);

      switch (form) {
         case DFExponent: {
            num_str = exponentForm(_zero, decimal, exponential, group, plus, minus,
                  digits, decpt, precision, PMDecimalDigits, always_show_decpt);
            break;
         }

         case DFDecimal: {
            num_str = decimalForm(_zero, decimal, group,
                  digits, decpt, precision, PMDecimalDigits, always_show_decpt, flags & ThousandsGroup);
            break;
         }

         case DFSignificantDigits: {
            PrecisionMode mode = (flags & Alternate) ? PMSignificantDigits : PMChopTrailingZeros;

            if (decpt != digits.length() && (decpt <= -4 || decpt > precision)) {

               num_str = exponentForm(_zero, decimal, exponential, group, plus, minus,
                     digits, decpt, precision, mode, always_show_decpt);
            } else {
               num_str = decimalForm(_zero, decimal, group, digits, decpt, precision, mode,
                     always_show_decpt, flags & ThousandsGroup);
            }

            break;
         }
      }

      negative = sign != 0 && ! isZero(d);
   }

   // pad with zeros. LeftAdjusted overrides this flag). Also, we don't pad special numbers
   if (flags & QLocaleData::ZeroPadded && !(flags & QLocaleData::LeftAdjusted) && !special_number) {
      int num_pad_chars = width - num_str.length();

      // leave space for the sign
      if (negative || flags & QLocaleData::AlwaysShowSign || flags & QLocaleData::BlankBeforePositive) {
         --num_pad_chars;
      }

      for (int i = 0; i < num_pad_chars; ++i) {
         num_str.prepend(_zero);
      }
   }

   // add sign
   if (negative) {
      num_str.prepend(minus);

   } else if (flags & QLocaleData::AlwaysShowSign) {
      num_str.prepend(plus);

   } else if (flags & QLocaleData::BlankBeforePositive) {
      num_str.prepend(QLatin1Char(' '));
   }

   if (flags & QLocaleData::CapitalEorX) {
      num_str = num_str.toUpper();
   }

   return num_str;
}

QString QLocaleData::longLongToString(qint64 l, int precision, int base, int width, unsigned flags) const
{
   return longLongToString(m_zero, m_group, m_plus, m_minus, l, precision, base, width, flags);
}

QString QLocaleData::longLongToString(const QChar zero, const QChar group, const QChar plus, const QChar minus,
      qint64 l, int precision, int base, int width, unsigned flags)
{
   bool precision_not_specified = false;

   if (precision == -1) {
      precision_not_specified = true;
      precision = 1;
   }

   bool negative = l < 0;

   if (base != 10) {
      // these are not supported by sprintf for octal and hex
      flags &= ~AlwaysShowSign;
      flags &= ~BlankBeforePositive;
      negative = false; // neither are negative numbers
   }

   QString num_str;

   if (base == 10) {
      num_str = qlltoa(l, base, zero);
   } else {
      num_str = qulltoa(l, base, zero);
   }

   if (flags & ThousandsGroup && base == 10) {
      for (int i = num_str.length() - 3; i > 0; i -= 3) {
         num_str.insert(i, group);
      }
   }

   for (int i = num_str.length(); i < precision; ++i) {
      num_str.prepend(base == 10 ? zero : QChar::fromLatin1('0'));
   }

   if ((flags & Alternate || flags & ShowBase) && base == 8 && (num_str.isEmpty() || num_str[0] != '0')) {
      num_str.prepend('0');
   }

   // LeftAdjusted overrides this flag ZeroPadded. sprintf only padds
   // when precision is not specified in the format string
   bool zero_padded = flags & ZeroPadded && !(flags & LeftAdjusted) && precision_not_specified;

   if (zero_padded) {
      int num_pad_chars = width - num_str.length();

      // leave space for the sign
      if (negative || flags & AlwaysShowSign || flags & BlankBeforePositive) {
         --num_pad_chars;
      }

      // leave space for optional '0x' in hex form
      if (base == 16 && (flags & Alternate || flags & ShowBase)) {
         num_pad_chars -= 2;

      } else if (base == 2 && (flags & Alternate || flags & ShowBase)) {
         // leave space for optional '0b' in binary form
         num_pad_chars -= 2;
      }

      for (int i = 0; i < num_pad_chars; ++i) {
         num_str.prepend(base == 10 ? zero : QChar::fromLatin1('0'));
      }
   }

   if (flags & CapitalEorX) {
      num_str = num_str.toUpper();
   }

   if (base == 16 && (flags & Alternate || flags & ShowBase)) {
      num_str.prepend(QLatin1String(flags & UppercaseBase ? "0X" : "0x"));
   }

   if (base == 2 && (flags & Alternate || flags & ShowBase)) {
      num_str.prepend(QLatin1String(flags & UppercaseBase ? "0B" : "0b"));
   }

   // add sign
   if (negative) {
      num_str.prepend(minus);
   } else if (flags & AlwaysShowSign) {
      num_str.prepend(plus);
   } else if (flags & BlankBeforePositive) {
      num_str.prepend(QLatin1Char(' '));
   }

   return num_str;
}

QString QLocaleData::unsLongLongToString(quint64 l, int precision, int base, int width, unsigned flags) const
{
   return unsLongLongToString(m_zero, m_group, m_plus, l, precision, base, width, flags);
}

QString QLocaleData::unsLongLongToString(const QChar zero, const QChar group, const QChar plus, quint64 l,
      int precision, int base, int width, unsigned flags)
{
   bool precision_not_specified = false;

   if (precision == -1) {
      precision_not_specified = true;
      precision = 1;
   }

   QString num_str = qulltoa(l, base, zero);

   if (flags & ThousandsGroup && base == 10) {
      for (int i = num_str.length() - 3; i > 0; i -= 3) {
         num_str.insert(i, group);
      }
   }

   for (int i = num_str.length(); i < precision; ++i) {
      num_str.prepend(base == 10 ? zero : QChar('0'));
   }

   if ((flags & Alternate || flags & ShowBase) && base == 8 && (num_str.isEmpty() || num_str[0] != '0')) {
      num_str.prepend('0');
   }

   // LeftAdjusted overrides this flag ZeroPadded. sprintf only padds
   // when precision is not specified in the format string
   bool zero_padded = flags & ZeroPadded && ! (flags & LeftAdjusted) && precision_not_specified;

   if (zero_padded) {
      int num_pad_chars = width - num_str.length();

      // leave space for optional '0x' in hex form
      if (base == 16 && flags & Alternate) {
         num_pad_chars -= 2;
      }
      // leave space for optional '0b' in binary form
      else if (base == 2 && flags & Alternate) {
         num_pad_chars -= 2;
      }

      for (int i = 0; i < num_pad_chars; ++i) {
         num_str.prepend(base == 10 ? zero : QChar('0'));
      }
   }

   if (flags & CapitalEorX) {
      num_str = num_str.toUpper();
   }

   if (base == 16 && (flags & Alternate || flags & ShowBase)) {
      num_str.prepend(QLatin1String(flags & UppercaseBase ? "0X" : "0x"));

   } else if (base == 2 && (flags & Alternate || flags & ShowBase)) {
      num_str.prepend(QLatin1String(flags & UppercaseBase ? "0B" : "0b"));
   }

   // add sign
   if (flags & AlwaysShowSign) {
      num_str.prepend(plus);
   } else if (flags & BlankBeforePositive) {
      num_str.prepend(QLatin1Char(' '));
   }

   return num_str;
}

bool QLocaleData::numberToCLocale(const QString &num, GroupSeparatorMode group_sep_mode, CharBuff *result) const
{
   QString::const_iterator iter = num.begin();
   QString::const_iterator end  = num.end();

   // Skip whitespace
   while (iter != end && iter->isSpace()) {
      ++iter;
   }

   if (iter == end) {
      return false;
   }

   // check trailing whitespace
   while (iter != end && (end - 1)->isSpace()) {
      --end;
   }

   int group_cnt = 0;

   QString::const_iterator decpt_iter           = end;
   QString::const_iterator last_separator_iter  = end;
   QString::const_iterator start_of_digits_iter = end;

   while (iter != end) {
      QChar in = *iter;
      char out = digitToCLocale(in);

      if (out == 0) {
         if (in == m_list) {
            out = ';';

         } else if (in == m_percent) {
            out = '%';

         } else if (in >= 'A' && in <= 'Z') {
            // for handling base-x numbers
            out = in.toLower()[0].toLatin1();

         } else if (in >= 'a' && in <= 'z') {
            out = in.toLatin1();

         } else {
            break;

         }

         if (group_sep_mode == ParseGroupSeparators) {

            if (start_of_digits_iter == end && out >= '0' && out <= '9') {
               start_of_digits_iter = iter;

            } else if (out == ',') {
               // Don't allow group chars after the decimal point
               if (decpt_iter != end) {
                  return false;
               }

               // check distance from the last separator or from the beginning of the digits

               if (last_separator_iter != end && iter - last_separator_iter != 4) {
                  return false;
               }

               if (last_separator_iter == end && (start_of_digits_iter == end || iter - start_of_digits_iter > 3)) {
                  return false;
               }

               last_separator_iter = iter;
               ++group_cnt;

               // don't add the group separator
               ++iter;
               continue;

            } else if (out == '.' || out == 'e' || out == 'E') {

               // Fail if more than one decimal point
               if (out == '.' && decpt_iter != end) {
                  return false;
               }

               if (decpt_iter == end) {
                  decpt_iter = iter;
               }

               // check distance from the last separator
               if (last_separator_iter != end && iter - last_separator_iter != 4) {
                  return false;
               }

               // stop processing separators
               last_separator_iter = end;
            }
         }
      }

      result->append(out);
      ++iter;
   }

   // check separators
   if (group_sep_mode == ParseGroupSeparators) {
      // group separator post-processing,  did we end in a separator?
      if (last_separator_iter == iter - 1) {
         return false;
      }

      // were there enough digits since the last separator?
      if (last_separator_iter != end && iter - last_separator_iter != 4)  {
         return false;
      }
   }

   result->append('\0');

   return iter == end;
}

bool QLocaleData::validateChars(const QString &str, NumberMode numMode, QByteArray *buff,
      int decDigits, bool rejectGroupSeparators) const
{
   buff->clear();
   buff->reserve(str.length());

   const bool scientific = numMode == DoubleScientificMode;

   bool lastWasE     = false;
   bool lastWasDigit = false;
   int eCnt          = 0;
   int decPointCnt   = 0;
   bool dec          = false;
   int decDigitCnt   = 0;

   for (int i = 0; i < str.length(); ++i) {
      char c = digitToCLocale(str.at(i));

      if (c >= '0' && c <= '9') {
         if (numMode != IntegerMode) {
            // If a double has too many digits after decpt, it shall be Invalid.
            if (dec && decDigits != -1 && decDigits < ++decDigitCnt) {
               return false;
            }
         }

         lastWasDigit = true;

      } else {
         switch (c) {
            case '.':
               if (numMode == IntegerMode) {
                  // If an integer has a decimal point, it shall be Invalid.
                  return false;
               } else {
                  // If a double has more than one decimal point, it shall be Invalid.
                  if (++decPointCnt > 1) {
                     return false;
                  }

                  dec = true;
               }

               break;

            case '+':
            case '-':
               if (scientific) {
                  // If a scientific has a sign that's not at the beginning or after
                  // an 'e', it shall be Invalid.
                  if (i != 0 && !lastWasE) {
                     return false;
                  }

               } else {
                  // If a non-scientific has a sign that's not at the beginning,
                  // it shall be Invalid.
                  if (i != 0) {
                     return false;
                  }
               }

               break;

            case ',':

               //it can only be placed after a digit which is before the decimal point
               if (rejectGroupSeparators || !lastWasDigit || decPointCnt > 0) {
                  return false;
               }

               break;

            case 'e':
               if (scientific) {
                  // If a scientific has more than one 'e', it shall be Invalid.
                  if (++eCnt > 1) {
                     return false;
                  }

                  dec = false;
               } else {
                  // If a non-scientific has an 'e', it shall be Invalid.
                  return false;
               }

               break;

            default:
               // If it's not a valid digit, it shall be Invalid.
               return false;
         }

         lastWasDigit = false;
      }

      lastWasE = c == 'e';

      if (c != ',') {
         buff->append(c);
      }
   }

   return true;
}

double QLocaleData::stringToDouble(const QString &number, bool *ok, GroupSeparatorMode group_sep_mode) const
{
   CharBuff buff;

   if (! numberToCLocale(number, group_sep_mode, &buff)) {
      if (ok != nullptr) {
         *ok = false;
      }

      return 0.0;
   }

   return bytearrayToDouble(buff.constData(), ok);
}

qint64 QLocaleData::stringToLongLong(const QString &number, int base, bool *ok, GroupSeparatorMode group_sep_mode) const
{
   CharBuff buff;

   if (! numberToCLocale(number, group_sep_mode, &buff)) {
      if (ok != nullptr) {
         *ok = false;
      }

      return 0;
   }

   return bytearrayToLongLong(buff.constData(), base, ok);
}

quint64 QLocaleData::stringToUnsLongLong(const QString &number, int base, bool *ok, GroupSeparatorMode group_sep_mode) const
{
   CharBuff buff;

   if (! numberToCLocale(number, group_sep_mode, &buff)) {
      if (ok != nullptr) {
         *ok = false;
      }

      return 0;
   }

   return bytearrayToUnsLongLong(buff.constData(), base, ok);
}

double QLocaleData::bytearrayToDouble(const char *num, bool *ok, bool *overflow)
{
   if (ok != nullptr) {
      *ok = true;
   }

   if (overflow != nullptr) {
      *overflow = false;
   }

   if (*num == '\0') {
      if (ok != nullptr) {
         *ok = false;
      }

      return 0.0;
   }

   if (qstrcmp(num, "nan") == 0) {
      return qt_snan();
   }

   if (qstrcmp(num, "+inf") == 0 || qstrcmp(num, "inf") == 0) {
      return qt_inf();
   }

   if (qstrcmp(num, "-inf") == 0) {
      return -qt_inf();
   }

   bool _ok;
   const char *endptr;
   double d = qstrtod(num, &endptr, &_ok);

   if (!_ok) {
      // the only way strtod can fail with *endptr != '\0' on a non-empty input string is overflow
      if (ok != nullptr) {
         *ok = false;
      }

      if (overflow != nullptr) {
         *overflow = *endptr != '\0';
      }

      return 0.0;
   }

   if (*endptr != '\0') {
      // we stopped at a non-digit character after converting some digits
      if (ok != nullptr) {
         *ok = false;
      }

      if (overflow != nullptr) {
         *overflow = false;
      }

      return 0.0;
   }

   if (ok != nullptr) {
      *ok = true;
   }

   if (overflow != nullptr) {
      *overflow = false;
   }

   return d;
}

qint64 QLocaleData::bytearrayToLongLong(const char *num, int base, bool *ok, bool *overflow)
{
   bool _ok;
   const char *endptr;

   if (*num == '\0') {
      if (ok != nullptr) {
         *ok = false;
      }

      if (overflow != nullptr) {
         *overflow = false;
      }

      return 0;
   }

   qint64 l = qstrtoll(num, &endptr, base, &_ok);

   if (!_ok) {
      if (ok != nullptr) {
         *ok = false;
      }

      if (overflow != nullptr) {
         // the only way qstrtoll can fail with *endptr != '\0' on a non-empty
         // input string is overflow
         *overflow = *endptr != '\0';
      }

      return 0;
   }

   if (*endptr != '\0') {
      // we stopped at a non-digit character after converting some digits
      if (ok != nullptr) {
         *ok = false;
      }

      if (overflow != nullptr) {
         *overflow = false;
      }

      return 0;
   }

   if (ok != nullptr) {
      *ok = true;
   }

   if (overflow != nullptr) {
      *overflow = false;
   }

   return l;
}

quint64 QLocaleData::bytearrayToUnsLongLong(const char *num, int base, bool *ok)
{
   bool _ok;
   const char *endptr;
   quint64 l = qstrtoull(num, &endptr, base, &_ok);

   if (!_ok || *endptr != '\0') {
      if (ok != nullptr) {
         *ok = false;
      }

      return 0;
   }

   if (ok != nullptr) {
      *ok = true;
   }

   return l;
}

QString QLocale::currencySymbol(QLocale::CurrencySymbolFormat symbolFormat) const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(QSystemLocale::CurrencySymbol, symbolFormat);

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   quint32 idx;
   quint32 size;

   switch (symbolFormat) {
      case CurrencySymbol:
         idx = d->m_data->m_currency_symbol_idx;
         size = d->m_data->m_currency_symbol_size;
         return getLocaleData(currency_symbol_data + idx, size);

      case CurrencyDisplayName:
         idx = d->m_data->m_currency_display_name_idx;
         size = d->m_data->m_currency_display_name_size;
         return getLocaleListData(currency_display_name_data + idx, size, 0);

      case CurrencyIsoCode: {
         int len = 0;
         const QLocaleData *data = this->d->m_data;

         for (; len < 3; ++len) {
            if (!data->m_currency_iso_code[len]) {
               break;
            }
         }

         return len ? QString::fromLatin1(data->m_currency_iso_code, len) : QString();
      }
   }

   return QString();
}

QString QLocale::toCurrencyString(qint64 value, const QString &symbol) const
{

#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QSystemLocale::CurrencyToStringArgument arg(value, symbol);
      QVariant res = systemLocale()->query(QSystemLocale::CurrencyToString, QVariant::fromValue(arg));

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   const QLocalePrivate *d = this->d;

   quint8 idx = d->m_data->m_currency_format_idx;
   quint8 size = d->m_data->m_currency_format_size;

   if (d->m_data->m_currency_negative_format_size && value < 0) {
      idx = d->m_data->m_currency_negative_format_idx;
      size = d->m_data->m_currency_negative_format_size;
      value = -value;
   }

   QString str = toString(value);
   QString sym = symbol.isEmpty() ? currencySymbol() : symbol;

   if (sym.isEmpty()) {
      sym = currencySymbol(QLocale::CurrencyIsoCode);
   }

   QString format = getLocaleData(currency_format_data + idx, size);

   return format.formatArgs(str, sym);
}

QString QLocale::toCurrencyString(quint64 value, const QString &symbol) const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QSystemLocale::CurrencyToStringArgument arg(value, symbol);
      QVariant res = systemLocale()->query(QSystemLocale::CurrencyToString, QVariant::fromValue(arg));

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   const QLocaleData *data = this->d->m_data;

   quint8 idx  = data->m_currency_format_idx;
   quint8 size = data->m_currency_format_size;
   QString str = toString(value);
   QString sym = symbol.isEmpty() ? currencySymbol() : symbol;

   if (sym.isEmpty()) {
      sym = currencySymbol(QLocale::CurrencyIsoCode);
   }

   QString format = getLocaleData(currency_format_data + idx, size);

   return format.formatArgs(str, sym);
}

QString QLocale::toCurrencyString(double value, const QString &symbol) const
{

#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QSystemLocale::CurrencyToStringArgument arg(value, symbol);
      QVariant res = systemLocale()->query(QSystemLocale::CurrencyToString, QVariant::fromValue(arg));

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   const QLocaleData *data = this->d->m_data;
   quint8 idx = data->m_currency_format_idx;
   quint8 size = data->m_currency_format_size;

   if (data->m_currency_negative_format_size && value < 0) {
      idx = data->m_currency_negative_format_idx;
      size = data->m_currency_negative_format_size;
      value = -value;
   }

   QString str = toString(value, 'f', d->m_data->m_currency_digits);
   QString sym = symbol.isEmpty() ? currencySymbol() : symbol;

   if (sym.isEmpty()) {
      sym = currencySymbol(QLocale::CurrencyIsoCode);
   }

   QString format = getLocaleData(currency_format_data + idx, size);

   return format.formatArgs(str, sym);
}

QStringList QLocale::uiLanguages() const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(QSystemLocale::UILanguages, QVariant());

      if (res.isValid()) {
         QStringList result = res.toStringList();

         if (!result.isEmpty()) {
            return result;
         }
      }
   }

#endif

   QLocaleId id = QLocaleId::fromIds(d->m_data->m_language_id, d->m_data->m_script_id, d->m_data->m_country_id);
   const QLocaleId max = id.withLikelySubtagsAdded();
   const QLocaleId min = max.withLikelySubtagsRemoved();

   QStringList uiLanguages;
   uiLanguages.append(min.name());

   if (id.script_id) {
      id.script_id = 0;

      if (id != min && id.withLikelySubtagsAdded() == max) {
         uiLanguages.append(id.name());
      }
   }

   if (max != min && max != id) {
      uiLanguages.append(max.name());
   }

   return uiLanguages;
}

QString QLocale::nativeLanguageName() const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(QSystemLocale::NativeLanguageName, QVariant());

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   return getLocaleData(endonyms_data + d->m_data->m_language_endonym_idx, d->m_data->m_language_endonym_size);
}

QString QLocale::nativeCountryName() const
{
#ifndef QT_NO_SYSTEMLOCALE

   if (d->m_data == systemData()) {
      QVariant res = systemLocale()->query(QSystemLocale::NativeCountryName, QVariant());

      if (res.isValid()) {
         return res.toString();
      }
   }

#endif

   return getLocaleData(endonyms_data + d->m_data->m_country_endonym_idx, d->m_data->m_country_endonym_size);
}
