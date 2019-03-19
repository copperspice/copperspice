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

#include <qlocale_p.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <qvariant.h>

static QByteArray getSystemLocale()
{
   return qgetenv("LC_ALL");
}

#ifndef QT_NO_SYSTEMLOCALE
struct QSystemLocaleData {
   QSystemLocaleData()
      : lc_numeric(QLocale::C)
      , lc_time(QLocale::C)
      , lc_monetary(QLocale::C)
      , lc_messages(QLocale::C) {
      updateLocale();
   }

   void updateLocale() {
      QByteArray all = getSystemLocale();
      QByteArray numeric  = all.isEmpty() ? qgetenv("LC_NUMERIC") : all;
      QByteArray time     = all.isEmpty() ? qgetenv("LC_TIME") : all;
      QByteArray monetary = all.isEmpty() ? qgetenv("LC_MONETARY") : all;
      lc_messages_var     = all.isEmpty() ? qgetenv("LC_MESSAGES") : all;
      lc_measurement_var  = all.isEmpty() ? qgetenv("LC_MEASUREMENT") : all;

      QByteArray lang = qgetenv("LANG");
      if (lang.isEmpty()) {
         lang = QByteArray("C");
      }
      if (numeric.isEmpty()) {
         numeric = lang;
      }
      if (time.isEmpty()) {
         time = lang;
      }
      if (monetary.isEmpty()) {
         monetary = lang;
      }
      if (lc_messages_var.isEmpty()) {
         lc_messages_var = lang;
      }
      if (lc_measurement_var.isEmpty()) {
         lc_measurement_var = lang;
      }
      lc_numeric = QLocale(QString::fromLatin1(numeric));
      lc_time = QLocale(QString::fromLatin1(time));
      lc_monetary = QLocale(QString::fromLatin1(monetary));
      lc_messages = QLocale(QString::fromLatin1(lc_messages_var));
   }

   QLocale lc_numeric;
   QLocale lc_time;
   QLocale lc_monetary;
   QLocale lc_messages;
   QByteArray lc_messages_var;
   QByteArray lc_measurement_var;
};
Q_GLOBAL_STATIC(QSystemLocaleData, qSystemLocaleData)
#endif

#ifndef QT_NO_SYSTEMLOCALE
QLocale QSystemLocale::fallbackUiLocale() const
{

   QByteArray lang = getSystemLocale();

   if (lang.isEmpty()) {
      lang = qgetenv("LC_MESSAGES");
   }
   if (lang.isEmpty()) {
      lang = qgetenv("LANG");
   }
   return QLocale(QString::fromLatin1(lang));
}

QVariant QSystemLocale::query(QueryType type, QVariant in) const
{
   QSystemLocaleData *d = qSystemLocaleData();
   const QLocale &lc_numeric = d->lc_numeric;
   const QLocale &lc_time = d->lc_time;
   const QLocale &lc_monetary = d->lc_monetary;
   const QLocale &lc_messages = d->lc_messages;

   switch (type) {
      case DecimalPoint:
         return lc_numeric.decimalPoint();
      case GroupSeparator:
         return lc_numeric.groupSeparator();
      case ZeroDigit:
         return lc_numeric.zeroDigit();
      case NegativeSign:
         return lc_numeric.negativeSign();
      case DateFormatLong:
         return lc_time.dateFormat(QLocale::LongFormat);
      case DateFormatShort:
         return lc_time.dateFormat(QLocale::ShortFormat);
      case TimeFormatLong:
         return lc_time.timeFormat(QLocale::LongFormat);
      case TimeFormatShort:
         return lc_time.timeFormat(QLocale::ShortFormat);
      case DayNameLong:
         return lc_time.dayName(in.toInt(), QLocale::LongFormat);
      case DayNameShort:
         return lc_time.dayName(in.toInt(), QLocale::ShortFormat);
      case MonthNameLong:
         return lc_time.monthName(in.toInt(), QLocale::LongFormat);
      case MonthNameShort:
         return lc_time.monthName(in.toInt(), QLocale::ShortFormat);
      case StandaloneMonthNameLong:
         return lc_time.standaloneMonthName(in.toInt(), QLocale::LongFormat);
      case StandaloneMonthNameShort:
         return lc_time.standaloneMonthName(in.toInt(), QLocale::ShortFormat);
      case DateToStringLong:
         return lc_time.toString(in.toDate(), QLocale::LongFormat);
      case DateToStringShort:
         return lc_time.toString(in.toDate(), QLocale::ShortFormat);
      case TimeToStringLong:
         return lc_time.toString(in.toTime(), QLocale::LongFormat);
      case TimeToStringShort:
         return lc_time.toString(in.toTime(), QLocale::ShortFormat);
      case DateTimeFormatLong:
         return lc_time.dateTimeFormat(QLocale::LongFormat);
      case DateTimeFormatShort:
         return lc_time.dateTimeFormat(QLocale::ShortFormat);
      case DateTimeToStringLong:
         return lc_time.toString(in.toDateTime(), QLocale::LongFormat);
      case DateTimeToStringShort:
         return lc_time.toString(in.toDateTime(), QLocale::ShortFormat);
      case PositiveSign:
         return lc_numeric.positiveSign();
      case AMText:
         return lc_time.amText();
      case PMText:
         return lc_time.pmText();
      case FirstDayOfWeek:
         return lc_time.firstDayOfWeek();
      case CurrencySymbol:
         return lc_monetary.currencySymbol(QLocale::CurrencySymbolFormat(in.toUInt()));
      case CurrencyToString: {
         switch (in.type()) {
            case QVariant::Int:
               return lc_monetary.toCurrencyString(in.toInt());
            case QVariant::UInt:
               return lc_monetary.toCurrencyString(in.toUInt());
            case QVariant::Double:
               return lc_monetary.toCurrencyString(in.toDouble());
            case QVariant::LongLong:
               return lc_monetary.toCurrencyString(in.toLongLong());
            case QVariant::ULongLong:
               return lc_monetary.toCurrencyString(in.toULongLong());
            default:
               break;
         }
         return QString();
      }
      case MeasurementSystem: {
         const QString meas_locale = QString::fromLatin1(d->lc_measurement_var.constData(), d->lc_measurement_var.size());
         if (meas_locale.compare(QLatin1String("Metric"), Qt::CaseInsensitive) == 0) {
            return QLocale::MetricSystem;
         }
         if (meas_locale.compare(QLatin1String("Other"), Qt::CaseInsensitive) == 0) {
            return QLocale::MetricSystem;
         }
         return QVariant((int)QLocale(meas_locale).measurementSystem());
      }
      case UILanguages: {
         static QString languages = QString::fromLatin1(qgetenv("LANGUAGE"));
         if (!languages.isEmpty()) {
            QStringList lst = languages.split(QLatin1Char(':'));

            for (int i = 0; i < lst.size();) {
               const QString &name = lst.at(i);
               QString lang, script, cntry;

               if (name.isEmpty() || !qt_splitLocaleName(name, lang, script, cntry)) {
                  lst.removeAt(i);
               } else {
                  ++i;
               }
            }
            return lst;
         }

         if (!d->lc_messages_var.isEmpty()) {
            QString lang, script, cntry;

            if (qt_splitLocaleName(QString::fromLatin1(d->lc_messages_var.constData(), d->lc_messages_var.size()),
                                   lang, script, cntry)) {
               if (!cntry.length() && lang.length()) {
                  return QStringList(lang);
               }

               return QStringList(lang + '-' + cntry);
            }
         }
         return QVariant();
      }

      case StringToStandardQuotation:
         return lc_messages.quoteString(in.value<QStringView>());

      case StringToAlternateQuotation:
         return lc_messages.quoteString(in.value<QStringView>(), QLocale::AlternateQuotation);

      case ListToSeparatedString:
         return lc_messages.createSeparatedList(in.value<QStringList>());

      case LocaleChanged:
         d->updateLocale();
         break;

      default:
         break;
   }

   return QVariant();
}
#endif // QT_NO_SYSTEMLOCALE

