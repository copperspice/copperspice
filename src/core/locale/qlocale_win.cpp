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

#include <qdatetime.h>
#include <qdebug.h>
#include <qstringlist.h>
#include <qvariant.h>

#include <qlocale_p.h>
#include <qlocale_tools_p.h>
#include <qsystemlibrary_p.h>

#if defined(Q_OS_WIN)
#include <qt_windows.h>
#include <time.h>
#endif

static QByteArray getWinLocaleName(LCID id = LOCALE_USER_DEFAULT);
static const char *winLangCodeToIsoName(int code);
static QString winIso639LangName(LCID id = LOCALE_USER_DEFAULT);
static QString winIso3116CtryName(LCID id = LOCALE_USER_DEFAULT);

#ifndef QT_NO_SYSTEMLOCALE

#ifndef MUI_LANGUAGE_NAME
#define MUI_LANGUAGE_NAME 0x8
#endif

#ifndef LOCALE_SSHORTESTDAYNAME1
#  define LOCALE_SSHORTESTDAYNAME1 0x0060
#  define LOCALE_SSHORTESTDAYNAME2 0x0061
#  define LOCALE_SSHORTESTDAYNAME3 0x0062
#  define LOCALE_SSHORTESTDAYNAME4 0x0063
#  define LOCALE_SSHORTESTDAYNAME5 0x0064
#  define LOCALE_SSHORTESTDAYNAME6 0x0065
#  define LOCALE_SSHORTESTDAYNAME7 0x0066
#endif

#ifndef LOCALE_SNATIVELANGUAGENAME
#  define LOCALE_SNATIVELANGUAGENAME 0x00000004
#endif

#ifndef LOCALE_SNATIVECOUNTRYNAME
#  define LOCALE_SNATIVECOUNTRYNAME 0x00000008
#endif

struct QSystemLocalePrivate {
   QSystemLocalePrivate();

   QChar zeroDigit();
   QChar decimalPoint();
   QChar groupSeparator();
   QChar negativeSign();
   QChar positiveSign();

   QString dateFormat(QLocale::FormatType);
   QString timeFormat(QLocale::FormatType);
   QString dateTimeFormat(QLocale::FormatType);

   QString dayName(int, QLocale::FormatType);
   QString monthName(int, QLocale::FormatType);

   QString toString(const QDate &, QLocale::FormatType);
   QString toString(const QTime &, QLocale::FormatType);
   QString toString(const QDateTime &, QLocale::FormatType);

   QVariant measurementSystem();
   QVariant amText();
   QVariant pmText();
   QVariant firstDayOfWeek();
   QVariant currencySymbol(QLocale::CurrencySymbolFormat);
   QVariant toCurrencyString(const QSystemLocale::CurrencyToStringArgument &);

   QStringList uiLanguages();

   QVariant nativeLanguageName();
   QVariant nativeCountryName();

   void update();

 private:
   QByteArray langEnvVar;

   enum SubstitutionType {
      SUnknown,
      SContext,
      SAlways,
      SNever
   };

   // cached values
   LCID lcid;
   SubstitutionType substitutionType;
   QChar zero;

   QString getLocaleInfo(LCTYPE type, int maxlen = 0);
   int getLocaleInfo_int(LCTYPE type, int maxlen = 0);
   QChar getLocaleInfo_qchar(LCTYPE type);

   SubstitutionType substitution();
   QString &substituteDigits(QString &string);

   static QString fromWinFormat(const QString &sys_fmt);
};

static QSystemLocalePrivate *systemLocalePrivate()
{
   static QSystemLocalePrivate retval;
   return &retval;
}

QSystemLocalePrivate::QSystemLocalePrivate()
   : substitutionType(SUnknown)
{
   langEnvVar = qgetenv("LANG");
   lcid = GetUserDefaultLCID();
}

QString QSystemLocalePrivate::getLocaleInfo(LCTYPE type, int maxlen)
{
   std::wstring buffer(maxlen ? maxlen : 64, L'\0');

   if (! GetLocaleInfo(lcid, type, &buffer[0], buffer.size())) {
      return QString();
   }

   if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
      int cnt = GetLocaleInfo(lcid, type, nullptr, 0);

      if (cnt == 0) {
         return QString();
      }

      buffer.resize(cnt);

      if (! GetLocaleInfo(lcid, type, &buffer[0], buffer.size())) {
         return QString();
      }
   }

   return QString::fromStdWString(buffer);
}

int QSystemLocalePrivate::getLocaleInfo_int(LCTYPE type, int maxlen)
{
   QString str = getLocaleInfo(type, maxlen);
   bool ok     = false;
   int v       = str.toInteger<int>(&ok);

   return ok ? v : 0;
}

QChar QSystemLocalePrivate::getLocaleInfo_qchar(LCTYPE type)
{
   QString str = getLocaleInfo(type);
   return str.isEmpty() ? QChar() : str.at(0);
}

QSystemLocalePrivate::SubstitutionType QSystemLocalePrivate::substitution()
{
   if (substitutionType == SUnknown) {
      wchar_t buf[8];

      if (!GetLocaleInfo(lcid, LOCALE_IDIGITSUBSTITUTION, buf, 8)) {
         substitutionType = QSystemLocalePrivate::SNever;
         return substitutionType;
      }

      if (buf[0] == '1') {
         substitutionType = QSystemLocalePrivate::SNever;

      } else if (buf[0] == '0') {
         substitutionType = QSystemLocalePrivate::SContext;

      } else if (buf[0] == '2') {
         substitutionType = QSystemLocalePrivate::SAlways;

      } else {
         wchar_t digits[11];

         if (!GetLocaleInfo(lcid, LOCALE_SNATIVEDIGITS, digits, 11)) {
            substitutionType = QSystemLocalePrivate::SNever;
            return substitutionType;
         }

         const wchar_t zero = digits[0];

         if (buf[0] == zero + 2) {
            substitutionType = QSystemLocalePrivate::SAlways;
         } else {
            substitutionType = QSystemLocalePrivate::SNever;
         }
      }
   }

   return substitutionType;
}

QString &QSystemLocalePrivate::substituteDigits(QString &string)
{
   ushort zero = zeroDigit().unicode();
   ushort *qch = (ushort *)string.data();

   for (ushort *end = qch + string.size(); qch != end; ++qch) {
      if (*qch >= '0' && *qch <= '9') {
         *qch = zero + (*qch - '0');
      }
   }

   return string;
}

QChar QSystemLocalePrivate::zeroDigit()
{
   if (zero.isNull()) {
      zero = getLocaleInfo_qchar(LOCALE_SNATIVEDIGITS);
   }

   return zero;
}

QChar QSystemLocalePrivate::decimalPoint()
{
   return getLocaleInfo_qchar(LOCALE_SDECIMAL);
}

QChar QSystemLocalePrivate::groupSeparator()
{
   return getLocaleInfo_qchar(LOCALE_STHOUSAND);
}

QChar QSystemLocalePrivate::negativeSign()
{
   return getLocaleInfo_qchar(LOCALE_SNEGATIVESIGN);
}

QChar QSystemLocalePrivate::positiveSign()
{
   return getLocaleInfo_qchar(LOCALE_SPOSITIVESIGN);
}

QString QSystemLocalePrivate::dateFormat(QLocale::FormatType type)
{
   switch (type) {
      case QLocale::ShortFormat:
         return fromWinFormat(getLocaleInfo(LOCALE_SSHORTDATE));

      case QLocale::LongFormat:
         return fromWinFormat(getLocaleInfo(LOCALE_SLONGDATE));

      case QLocale::NarrowFormat:
         break;
   }

   return QString();
}

QString QSystemLocalePrivate::timeFormat(QLocale::FormatType type)
{
   switch (type) {
      case QLocale::ShortFormat:
         return fromWinFormat(getLocaleInfo(LOCALE_STIMEFORMAT));

      case QLocale::LongFormat:
         return fromWinFormat(getLocaleInfo(LOCALE_STIMEFORMAT));

      case QLocale::NarrowFormat:
         break;
   }

   return QString();
}

QString QSystemLocalePrivate::dateTimeFormat(QLocale::FormatType type)
{
   return dateFormat(type) + ' ' + timeFormat(type);
}

QString QSystemLocalePrivate::dayName(int day, QLocale::FormatType type)
{
   static const LCTYPE short_day_map[] =
         { LOCALE_SABBREVDAYNAME1, LOCALE_SABBREVDAYNAME2,
           LOCALE_SABBREVDAYNAME3, LOCALE_SABBREVDAYNAME4, LOCALE_SABBREVDAYNAME5,
           LOCALE_SABBREVDAYNAME6, LOCALE_SABBREVDAYNAME7
         };

   static const LCTYPE long_day_map[] =
         { LOCALE_SDAYNAME1, LOCALE_SDAYNAME2,
           LOCALE_SDAYNAME3, LOCALE_SDAYNAME4, LOCALE_SDAYNAME5,
           LOCALE_SDAYNAME6, LOCALE_SDAYNAME7
         };

   static const LCTYPE narrow_day_map[] =
         { LOCALE_SSHORTESTDAYNAME1, LOCALE_SSHORTESTDAYNAME2,
           LOCALE_SSHORTESTDAYNAME3, LOCALE_SSHORTESTDAYNAME4,
           LOCALE_SSHORTESTDAYNAME5, LOCALE_SSHORTESTDAYNAME6,
           LOCALE_SSHORTESTDAYNAME7
         };

   day -= 1;

   if (type == QLocale::LongFormat) {
      return getLocaleInfo(long_day_map[day]);

   } else if (type == QLocale::NarrowFormat && QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA) {
      return getLocaleInfo(narrow_day_map[day]);
   }

   return getLocaleInfo(short_day_map[day]);
}

QString QSystemLocalePrivate::monthName(int month, QLocale::FormatType type)
{
   static const LCTYPE short_month_map[] =
         { LOCALE_SABBREVMONTHNAME1, LOCALE_SABBREVMONTHNAME2, LOCALE_SABBREVMONTHNAME3,
           LOCALE_SABBREVMONTHNAME4, LOCALE_SABBREVMONTHNAME5, LOCALE_SABBREVMONTHNAME6,
           LOCALE_SABBREVMONTHNAME7, LOCALE_SABBREVMONTHNAME8, LOCALE_SABBREVMONTHNAME9,
           LOCALE_SABBREVMONTHNAME10, LOCALE_SABBREVMONTHNAME11, LOCALE_SABBREVMONTHNAME12
         };

   static const LCTYPE long_month_map[] =
         { LOCALE_SMONTHNAME1, LOCALE_SMONTHNAME2, LOCALE_SMONTHNAME3,
           LOCALE_SMONTHNAME4, LOCALE_SMONTHNAME5, LOCALE_SMONTHNAME6,
           LOCALE_SMONTHNAME7, LOCALE_SMONTHNAME8, LOCALE_SMONTHNAME9,
           LOCALE_SMONTHNAME10, LOCALE_SMONTHNAME11, LOCALE_SMONTHNAME12
         };

   month -= 1;

   if (month < 0 || month > 11) {
      return QString();
   }

   LCTYPE lctype = (type == QLocale::ShortFormat || type == QLocale::NarrowFormat)
         ? short_month_map[month] : long_month_map[month];

   return getLocaleInfo(lctype);
}

QString QSystemLocalePrivate::toString(const QDate &date, QLocale::FormatType type)
{
   SYSTEMTIME st;

   memset(&st, 0, sizeof(SYSTEMTIME));

   st.wYear  = date.year();
   st.wMonth = date.month();
   st.wDay   = date.day();

   DWORD flags = (type == QLocale::LongFormat ? DATE_LONGDATE : DATE_SHORTDATE);
   std::wstring buffer(255, L'\0');

   if (GetDateFormat(lcid, flags, &st, nullptr, &buffer[0], 255)) {
      QString format = QString::fromStdWString(buffer);

      if (substitution() == SAlways) {
         substituteDigits(format);
      }

      return format;
   }

   return QString();
}

QString QSystemLocalePrivate::toString(const QTime &time, QLocale::FormatType)
{
   SYSTEMTIME st;
   memset(&st, 0, sizeof(SYSTEMTIME));

   st.wHour   = time.hour();
   st.wMinute = time.minute();
   st.wSecond = time.second();
   st.wMilliseconds = 0;

   DWORD flags = 0;
   std::wstring buffer(255, L'\0');

   if (GetTimeFormat(lcid, flags, &st, nullptr, &buffer[0], buffer.size())) {
      QString format = QString::fromStdWString(buffer);

      if (substitution() == SAlways) {
         substituteDigits(format);
      }

      return format;
   }

   return QString();
}

QString QSystemLocalePrivate::toString(const QDateTime &dt, QLocale::FormatType type)
{
   return toString(dt.date(), type) + ' ' + toString(dt.time(), type);
}

QVariant QSystemLocalePrivate::measurementSystem()
{
   std::wstring buffer(2, L'\0');

   if (GetLocaleInfo(lcid, LOCALE_IMEASURE, &buffer[0], 2)) {
      QString iMeasure = QString::fromStdWString(buffer);

      if (iMeasure == "1") {
         return QLocale::ImperialSystem;
      }
   }

   return QLocale::MetricSystem;
}

QVariant QSystemLocalePrivate::amText()
{
   // maximum length including  terminating zero character for Win2003+
   std::wstring buffer(15, L'\0');

   if (GetLocaleInfo(lcid, LOCALE_S1159, &buffer[0], 15)) {
      return QString::fromStdWString(buffer);
   }

   return QVariant();
}

QVariant QSystemLocalePrivate::pmText()
{
   // maximum length including  terminating zero character for Win2003+
   std::wstring buffer(15, L'\0');

   if (GetLocaleInfo(lcid, LOCALE_S2359, &buffer[0], 15)) {
      return QString::fromStdWString(buffer);
   }

   return QVariant();
}

QVariant QSystemLocalePrivate::firstDayOfWeek()
{
   // maximum length including  terminating zero character for Win2003+
   std::wstring buffer(4, L'\0');

   if (GetLocaleInfo(lcid, LOCALE_IFIRSTDAYOFWEEK, &buffer[0], 4)) {
      return QString::fromStdWString(buffer).toInteger<uint>() + 1;
   }

   return 1;
}

QVariant QSystemLocalePrivate::currencySymbol(QLocale::CurrencySymbolFormat format)
{
   std::wstring buffer(64, L'\0');

   switch (format) {
      case QLocale::CurrencySymbol:
         if (GetLocaleInfo(lcid, LOCALE_SCURRENCY, &buffer[0], buffer.size())) {
            return QString::fromStdWString(buffer);
         }

         break;

      case QLocale::CurrencyIsoCode:
         if (GetLocaleInfo(lcid, LOCALE_SINTLSYMBOL, &buffer[0], buffer.size())) {
            return QString::fromStdWString(buffer);
         }

         break;

      case QLocale::CurrencyDisplayName: {

         if (! GetLocaleInfo(lcid, LOCALE_SNATIVECURRNAME, &buffer[0], buffer.size())) {
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
               break;
            }

            buffer.resize(255);

            if (! GetLocaleInfo(lcid, LOCALE_SNATIVECURRNAME, &buffer[0], buffer.size())) {
               break;
            }
         }

         return QString::fromStdWString(buffer);
      }

      default:
         break;
   }

   return QVariant();
}

QVariant QSystemLocalePrivate::toCurrencyString(const QSystemLocale::CurrencyToStringArgument &arg)
{
   QString value;

   switch (arg.value.type()) {
      case QVariant::Int:
         value = QLocaleData::longLongToString('0', ',', '+', '-',
               arg.value.toInt(), -1, 10, -1, QLocale::OmitGroupSeparator);
         break;

      case QVariant::UInt:
         value = QLocaleData::unsLongLongToString('0', ',', '+',
               arg.value.toInt(), -1, 10, -1, QLocale::OmitGroupSeparator);
         break;

      case QVariant::Double:
         value = QLocaleData::doubleToString('0', '+', '-', ' ', ',', '.',
               arg.value.toDouble(), -1, QLocaleData::DFDecimal, -1, QLocale::OmitGroupSeparator);

         break;

      case QVariant::LongLong:
         value = QLocaleData::longLongToString('0', ',', '+', '-',
               arg.value.toLongLong(), -1, 10, -1, QLocale::OmitGroupSeparator);
         break;

      case QVariant::ULongLong:
         value = QLocaleData::unsLongLongToString('0', ',', '+',
               arg.value.toULongLong(), -1, 10, -1, QLocale::OmitGroupSeparator);
         break;

      default:
         return QVariant();
   }

   std::wstring out(64, L'\0');

   QString decimalSep;
   QString thousandSep;

   CURRENCYFMT format;
   CURRENCYFMT *pformat = nullptr;

   if (! arg.symbol.isEmpty()) {
      format.NumDigits   = getLocaleInfo_int(lcid, LOCALE_ICURRDIGITS);
      format.LeadingZero = getLocaleInfo_int(lcid, LOCALE_ILZERO);
      decimalSep         = getLocaleInfo(lcid, LOCALE_SMONDECIMALSEP);

      std::wstring tmp_1      = decimalSep.toStdWString();
      format.lpDecimalSep     = &tmp_1[0];

      thousandSep = getLocaleInfo(lcid, LOCALE_SMONTHOUSANDSEP);

      std::wstring tmp_2      = thousandSep.toStdWString();
      format.lpThousandSep    = &tmp_2[0];

      format.NegativeOrder    = getLocaleInfo_int(lcid, LOCALE_INEGCURR);
      format.PositiveOrder    = getLocaleInfo_int(lcid, LOCALE_ICURRENCY);

      std::wstring tmp_3      = arg.symbol.toStdWString();
      format.lpCurrencySymbol = &tmp_3[0];

      // grouping is complicated and ugly:
      // int(0)  == "123456789.00"    == string("0")
      // int(3)  == "123,456,789.00"  == string("3;0")
      // int(30) == "123456,789.00"   == string("3;0;0")
      // int(32) == "12,34,56,789.00" == string("3;2;0")
      // int(320)== "1234,56,789.00"  == string("3;2")

      QString groupingStr = getLocaleInfo(lcid, LOCALE_SMONGROUPING);
      format.Grouping     = groupingStr.remove(';').toInteger<int>();

      if (format.Grouping % 10 == 0) {
         format.Grouping /= 10;
      } else {
         format.Grouping *= 10;
      }

      pformat = &format;
   }

   int ret = ::GetCurrencyFormat(lcid, 0, &value.toStdWString()[0], pformat, &out[0], out.size());

   if (ret == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
      ret = ::GetCurrencyFormat(lcid, 0, &value.toStdWString()[0], pformat, &out[0], 0);
      out.resize(ret);

      ::GetCurrencyFormat(lcid, 0, &value.toStdWString()[0], pformat, &out[0], out.size());
   }

   value = QString::fromStdWString(out);

   if (substitution() == SAlways) {
      substituteDigits( value);
   }

   return value;
}

QStringList QSystemLocalePrivate::uiLanguages()
{
   if (QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA) {
      using GetUserPreferredUILanguagesFunc =
            BOOL (WINAPI *) (DWORD dwFlags, PULONG pulNumLanguages, PWSTR pwszLanguagesBuffer, PULONG pcchLanguagesBuffer);

      static GetUserPreferredUILanguagesFunc GetUserPreferredUILanguages_ptr = nullptr;

      if (! GetUserPreferredUILanguages_ptr) {
         QSystemLibrary lib("kernel32");

         if (lib.load()) {
            GetUserPreferredUILanguages_ptr = (GetUserPreferredUILanguagesFunc)lib.resolve("GetUserPreferredUILanguages");
         }
      }

      if (GetUserPreferredUILanguages_ptr) {
         unsigned long cnt = 0;

         std::wstring buffer(64, L'\0');
         ULONG size = buffer.size();

         if (! GetUserPreferredUILanguages_ptr(MUI_LANGUAGE_NAME, &cnt, &buffer[0], &size)) {
            size = 0;

            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && GetUserPreferredUILanguages_ptr(MUI_LANGUAGE_NAME, &cnt, nullptr, &size)) {
               buffer.resize(size);

               if (! GetUserPreferredUILanguages_ptr(MUI_LANGUAGE_NAME, &cnt, &buffer[0], &size)) {
                  return QStringList();
               }
            }
         }

         QString tmp = QString::fromStdWString(buffer, size - 2);

         return tmp.split(QChar32('\0'));
      }
   }

   // older Windows before Vista
   return QStringList(QString::fromLatin1(winLangCodeToIsoName(GetUserDefaultUILanguage())));
}

QVariant QSystemLocalePrivate::nativeLanguageName()
{
   if (QSysInfo::windowsVersion() < QSysInfo::WV_WINDOWS7) {
      return getLocaleInfo(LOCALE_SNATIVELANGNAME);
   }

   return getLocaleInfo(LOCALE_SNATIVELANGUAGENAME);
}

QVariant QSystemLocalePrivate::nativeCountryName()
{
   if (QSysInfo::windowsVersion() < QSysInfo::WV_WINDOWS7) {
      return getLocaleInfo(LOCALE_SNATIVECTRYNAME);
   }

   return getLocaleInfo(LOCALE_SNATIVECOUNTRYNAME);
}

void QSystemLocalePrivate::update()
{
   lcid = GetUserDefaultLCID();

   substitutionType = SUnknown;
   zero = QChar();
}

QString QSystemLocalePrivate::fromWinFormat(const QString &sys_fmt)
{
   QString result;
   int i = 0;

   while (i < sys_fmt.size()) {

      if (sys_fmt.at(i) == '\'') {
         QString text = qt_readEscapedFormatString(sys_fmt, &i);

         if (text == "'") {
            result += "''";

         } else {
            result += '\'' + text + '\'';
         }

         continue;
      }

      QChar c = sys_fmt.at(i);
      int repeat = qt_repeatCount(sys_fmt, i);

      switch (c.unicode()) {
         // Date
         case 'y':
            if (repeat > 5) {
               repeat = 5;

            } else if (repeat == 3) {
               repeat = 2;
            }

            switch (repeat) {
               case 1:
                  result += "yy";                     // "y" unsupported, use "yy"
                  break;

               case 5:
                  result += "yyyy";                   // "yyyyy" same as "yyyy" on Windows
                  break;

               default:
                  result += QString(repeat, 'y');
                  break;
            }

            break;

         case 'g':
            if (repeat > 2) {
               repeat = 2;
            }

            switch (repeat) {
               case 2:
                  break;                              // no equivalent of "gg"

               default:
                  result += 'g';
                  break;
            }

            break;

         case 't':
            if (repeat > 2) {
               repeat = 2;
            }

            result += "AP";                           // "t" unsupported, use "AP"
            break;

         default:
            result += QString(repeat, c);
            break;
      }

      i += repeat;
   }

   return result;
}

QLocale QSystemLocale::fallbackUiLocale() const
{
   return QLocale(QString::fromLatin1(getWinLocaleName()));
}

QVariant QSystemLocale::query(QueryType type, QVariant in = QVariant()) const
{
   QSystemLocalePrivate *d = systemLocalePrivate();

   switch (type) {
      case DecimalPoint:
         return d->decimalPoint();

      case GroupSeparator:
         return d->groupSeparator();

      case NegativeSign:
         return d->negativeSign();

      case PositiveSign:
         return d->positiveSign();

      case DateFormatLong:
         return d->dateFormat(QLocale::LongFormat);

      case DateFormatShort:
         return d->dateFormat(QLocale::ShortFormat);

      case TimeFormatLong:
         return d->timeFormat(QLocale::LongFormat);

      case TimeFormatShort:
         return d->timeFormat(QLocale::ShortFormat);

      case DateTimeFormatLong:
         return d->dateTimeFormat(QLocale::LongFormat);

      case DateTimeFormatShort:
         return d->dateTimeFormat(QLocale::ShortFormat);

      case DayNameLong:
         return d->dayName(in.toInt(), QLocale::LongFormat);

      case DayNameShort:
         return d->dayName(in.toInt(), QLocale::ShortFormat);

      case MonthNameLong:
      case StandaloneMonthNameLong:
         return d->monthName(in.toInt(), QLocale::LongFormat);

      case MonthNameShort:
      case StandaloneMonthNameShort:
         return d->monthName(in.toInt(), QLocale::ShortFormat);

      case DateToStringShort:
         return d->toString(in.toDate(), QLocale::ShortFormat);

      case DateToStringLong:
         return d->toString(in.toDate(), QLocale::LongFormat);

      case TimeToStringShort:
         return d->toString(in.toTime(), QLocale::ShortFormat);

      case TimeToStringLong:
         return d->toString(in.toTime(), QLocale::LongFormat);

      case DateTimeToStringShort:
         return d->toString(in.toDateTime(), QLocale::ShortFormat);

      case DateTimeToStringLong:
         return d->toString(in.toDateTime(), QLocale::LongFormat);

      case ZeroDigit:
         return d->zeroDigit();

      case LanguageId:
      case CountryId: {
         QString locale = QString::fromLatin1(getWinLocaleName());

         QLocale::Language lang;
         QLocale::Script script;
         QLocale::Country cntry;
         QLocalePrivate::getLangAndCountry(locale, lang, script, cntry);

         if (type == LanguageId) {
            return lang;
         }

         if (cntry == QLocale::AnyCountry) {
            return fallbackUiLocale().country();
         }

         return cntry;
      }

      case ScriptId:
         return QVariant(QLocale::AnyScript);

      case MeasurementSystem:
         return d->measurementSystem();

      case AMText:
         return d->amText();

      case PMText:
         return d->pmText();

      case FirstDayOfWeek:
         return d->firstDayOfWeek();

      case CurrencySymbol:
         return d->currencySymbol(QLocale::CurrencySymbolFormat(in.toUInt()));

      case CurrencyToString:
         return d->toCurrencyString(in.value<QSystemLocale::CurrencyToStringArgument>());

      case UILanguages:
         return d->uiLanguages();

      case LocaleChanged:
         d->update();
         break;

      case NativeLanguageName:
         return d->nativeLanguageName();

      case NativeCountryName:
         return d->nativeCountryName();

      default:
         break;
   }

   return QVariant();
}
#endif // QT_NO_SYSTEMLOCALE

struct WindowsToISOListElt {
   ushort windows_code;
   char iso_name[6];
};

/* NOTE: This array should be sorted by the first column! */
static const WindowsToISOListElt windows_to_iso_list[] = {
   { 0x0401, "ar_SA" },
   { 0x0402, "bg\0  " },
   { 0x0403, "ca\0  " },
   { 0x0404, "zh_TW" },
   { 0x0405, "cs\0  " },
   { 0x0406, "da\0  " },
   { 0x0407, "de\0  " },
   { 0x0408, "el\0  " },
   { 0x0409, "en_US" },
   { 0x040a, "es\0  " },
   { 0x040b, "fi\0  " },
   { 0x040c, "fr\0  " },
   { 0x040d, "he\0  " },
   { 0x040e, "hu\0  " },
   { 0x040f, "is\0  " },
   { 0x0410, "it\0  " },
   { 0x0411, "ja\0  " },
   { 0x0412, "ko\0  " },
   { 0x0413, "nl\0  " },
   { 0x0414, "no\0  " },
   { 0x0415, "pl\0  " },
   { 0x0416, "pt_BR" },
   { 0x0418, "ro\0  " },
   { 0x0419, "ru\0  " },
   { 0x041a, "hr\0  " },
   { 0x041c, "sq\0  " },
   { 0x041d, "sv\0  " },
   { 0x041e, "th\0  " },
   { 0x041f, "tr\0  " },
   { 0x0420, "ur\0  " },
   { 0x0421, "in\0  " },
   { 0x0422, "uk\0  " },
   { 0x0423, "be\0  " },
   { 0x0425, "et\0  " },
   { 0x0426, "lv\0  " },
   { 0x0427, "lt\0  " },
   { 0x0429, "fa\0  " },
   { 0x042a, "vi\0  " },
   { 0x042d, "eu\0  " },
   { 0x042f, "mk\0  " },
   { 0x0436, "af\0  " },
   { 0x0438, "fo\0  " },
   { 0x0439, "hi\0  " },
   { 0x043e, "ms\0  " },
   { 0x0458, "mt\0  " },
   { 0x0801, "ar_IQ" },
   { 0x0804, "zh_CN" },
   { 0x0807, "de_CH" },
   { 0x0809, "en_GB" },
   { 0x080a, "es_MX" },
   { 0x080c, "fr_BE" },
   { 0x0810, "it_CH" },
   { 0x0812, "ko\0  " },
   { 0x0813, "nl_BE" },
   { 0x0814, "no\0  " },
   { 0x0816, "pt\0  " },
   { 0x081a, "sr\0  " },
   { 0x081d, "sv_FI" },
   { 0x0c01, "ar_EG" },
   { 0x0c04, "zh_HK" },
   { 0x0c07, "de_AT" },
   { 0x0c09, "en_AU" },
   { 0x0c0a, "es\0  " },
   { 0x0c0c, "fr_CA" },
   { 0x0c1a, "sr\0  " },
   { 0x1001, "ar_LY" },
   { 0x1004, "zh_SG" },
   { 0x1007, "de_LU" },
   { 0x1009, "en_CA" },
   { 0x100a, "es_GT" },
   { 0x100c, "fr_CH" },
   { 0x1401, "ar_DZ" },
   { 0x1407, "de_LI" },
   { 0x1409, "en_NZ" },
   { 0x140a, "es_CR" },
   { 0x140c, "fr_LU" },
   { 0x1801, "ar_MA" },
   { 0x1809, "en_IE" },
   { 0x180a, "es_PA" },
   { 0x1c01, "ar_TN" },
   { 0x1c09, "en_ZA" },
   { 0x1c0a, "es_DO" },
   { 0x2001, "ar_OM" },
   { 0x2009, "en_JM" },
   { 0x200a, "es_VE" },
   { 0x2401, "ar_YE" },
   { 0x2409, "en\0  " },
   { 0x240a, "es_CO" },
   { 0x2801, "ar_SY" },
   { 0x2809, "en_BZ" },
   { 0x280a, "es_PE" },
   { 0x2c01, "ar_JO" },
   { 0x2c09, "en_TT" },
   { 0x2c0a, "es_AR" },
   { 0x3001, "ar_LB" },
   { 0x300a, "es_EC" },
   { 0x3401, "ar_KW" },
   { 0x340a, "es_CL" },
   { 0x3801, "ar_AE" },
   { 0x380a, "es_UY" },
   { 0x3c01, "ar_BH" },
   { 0x3c0a, "es_PY" },
   { 0x4001, "ar_QA" },
   { 0x400a, "es_BO" },
   { 0x440a, "es_SV" },
   { 0x480a, "es_HN" },
   { 0x4c0a, "es_NI" },
   { 0x500a, "es_PR" }
};

static constexpr const int windows_to_iso_count = sizeof(windows_to_iso_list) / sizeof(WindowsToISOListElt);

static const char *winLangCodeToIsoName(int code)
{
   int cmp = code - windows_to_iso_list[0].windows_code;

   if (cmp < 0) {
      return nullptr;
   }

   if (cmp == 0) {
      return windows_to_iso_list[0].iso_name;
   }

   int begin = 0;
   int end = windows_to_iso_count;

   while (end - begin > 1) {
      uint mid = (begin + end) / 2;

      const WindowsToISOListElt *elt = windows_to_iso_list + mid;
      int cmp = code - elt->windows_code;

      if (cmp < 0) {
         end = mid;
      } else if (cmp > 0) {
         begin = mid;
      } else {
         return elt->iso_name;
      }
   }

   return nullptr;

}

static QString winIso639LangName(LCID id)
{
   QString retval;

   // Windows returns the wrong ISO639 for some languages, we need to detect this using the language code
   QString lang_code;

   std::wstring buffer(255, L'\0');

   if (GetLocaleInfo(id, LOCALE_ILANGUAGE, &buffer[0], 255)) {
      lang_code = QString::fromStdWString(buffer);
   }

   if (! lang_code.isEmpty()) {
      bool ok = false;
      int i   = lang_code.toInteger<int>(&ok, 16);

      if (ok && (i == 0x814))  {
         return QString("nn");                         // Nynorsk (Norwegian)
      }
   }

   // not one of the problematic languages - do the usual lookup
   if (GetLocaleInfo(id, LOCALE_SISO639LANGNAME, &buffer[0], 255)) {
      retval = QString::fromStdWString(buffer);
   }

   return retval;
}

static QString winIso3116CtryName(LCID id)
{
   QString result;
   std::wstring buffer(256, L'\0');

   if (GetLocaleInfo(id, LOCALE_SISO3166CTRYNAME, &buffer[0], 255)) {
      result = QString::fromStdWString(buffer);
   }

   return result;
}

static QByteArray getWinLocaleName(LCID id)
{
   QByteArray result;

   if (id == LOCALE_USER_DEFAULT) {
      static QByteArray langEnvVar = qgetenv("LANG");
      result = langEnvVar;

      QString lang, script, cntry;

      if (result == "C" || (!result.isEmpty() && qt_splitLocaleName(QString::fromUtf8(result), lang, script, cntry)) ) {
         long id = 0;
         bool ok = false;
         id      = qstrtoll(result.data(), nullptr, 0, &ok);

         if ( !ok || id == 0 || id < INT_MIN || id > INT_MAX ) {
            return result;

         } else {
            return winLangCodeToIsoName( (int)id );
         }
      }
   }

   if (id == LOCALE_USER_DEFAULT) {
      id = GetUserDefaultLCID();
   }

   QString resultuage = winIso639LangName(id);
   QString country    = winIso3116CtryName(id);

   result = resultuage.toLatin1();

   if (! country.isEmpty()) {
      result += '_';
      result += country.toLatin1();
   }

   return result;
}

Q_CORE_EXPORT QLocale qt_localeFromLCID(LCID id)
{
   return QLocale(QString::fromLatin1(getWinLocaleName(id)));
}
