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

#include <qcups_p.h>

#include <qprintengine.h>

#ifndef QT_NO_CUPS

QStringList QCUPSSupport::cupsOptionsList(QPrinter *printer)
{
   return printer->printEngine()->property(PPK_CupsOptions).toStringList();
}

void QCUPSSupport::setCupsOptions(QPrinter *printer, const QStringList &cupsOptions)
{
   printer->printEngine()->setProperty(PPK_CupsOptions, QVariant(cupsOptions));
}

void QCUPSSupport::setCupsOption(QStringList &cupsOptions, const QString &option, const QString &value)
{
   if (cupsOptions.contains(option)) {
      cupsOptions.replace(cupsOptions.indexOf(option) + 1, value);

   } else {
      cupsOptions.append(option);
      cupsOptions.append(value);
   }
}

void QCUPSSupport::clearCupsOption(QStringList &cupsOptions, const QString &option)
{
   const QStringList::const_iterator iter = std::find(cupsOptions.begin(), cupsOptions.end(), option);

   if (iter != cupsOptions.end()) {
      Q_ASSERT(iter + 1 < cupsOptions.end());
      cupsOptions.erase(iter);
   }
}

static inline QString jobHoldToString(const QCUPSSupport::JobHoldUntil jobHold, const QTime holdUntilTime)
{
   switch (jobHold) {
      case QCUPSSupport::Indefinite:
         return QString("indefinite");

      case QCUPSSupport::DayTime:
         return QString("day-time");

      case QCUPSSupport::Night:
         return QString("night");

      case QCUPSSupport::SecondShift:
         return QString("second-shift");

      case QCUPSSupport::ThirdShift:
         return QString("third-shift");

      case QCUPSSupport::Weekend:
         return QString("weekend");

      case QCUPSSupport::SpecificTime:
         if (! holdUntilTime.isNull()) {
            // CUPS expects the time in UTC, user has entered in local time, so get the UTS equivalent
            QDateTime localDateTime = QDateTime::currentDateTime();

            // Check if time is for tomorrow in case of DST change overnight
            if (holdUntilTime < localDateTime.time()) {
               localDateTime = localDateTime.addDays(1);
            }

            localDateTime.setTime(holdUntilTime);
            return localDateTime.toUTC().time().toString("HH:mm");
         }
         [[fallthrough]];

      case QCUPSSupport::NoHold:
         return QString();
   }

   // error, may want to throw

   return QString();
}

void QCUPSSupport::setJobHold(QPrinter *printer, const JobHoldUntil jobHold, const QTime &holdUntilTime)
{
   QStringList cupsOptions = cupsOptionsList(printer);
   const QString jobHoldUntilArgument = jobHoldToString(jobHold, holdUntilTime);

   if (!jobHoldUntilArgument.isEmpty()) {
      setCupsOption(cupsOptions, "job-hold-until", jobHoldUntilArgument);

   } else {
      clearCupsOption(cupsOptions, "job-hold-until");
   }
   setCupsOptions(printer, cupsOptions);
}

void QCUPSSupport::setJobBilling(QPrinter *printer, const QString &jobBilling)
{
   QStringList cupsOptions = cupsOptionsList(printer);
   setCupsOption(cupsOptions, "job-billing", jobBilling);
   setCupsOptions(printer, cupsOptions);
}

void QCUPSSupport::setJobPriority(QPrinter *printer, int priority)
{
   QStringList cupsOptions = cupsOptionsList(printer);
   setCupsOption(cupsOptions, "job-priority", QString::number(priority));
   setCupsOptions(printer, cupsOptions);
}

static inline QString bannerPageToString(const QCUPSSupport::BannerPage bannerPage)
{
   switch (bannerPage) {
      case QCUPSSupport::NoBanner:
         return QString("none");

      case QCUPSSupport::Standard:
         return QString("standard");

      case QCUPSSupport::Unclassified:
         return QString("unclassified");

      case QCUPSSupport::Confidential:
         return QString("confidential");

      case QCUPSSupport::Classified:
         return QString("classified");

      case QCUPSSupport::Secret:
         return QString("secret");

      case QCUPSSupport::TopSecret:
         return QString("topsecret");
   }

   // error, may want to throw

   return QString();
}

void QCUPSSupport::setBannerPages(QPrinter *printer, const BannerPage startBannerPage, const BannerPage endBannerPage)
{
   QStringList cupsOptions    = cupsOptionsList(printer);
   const QString startBanner = bannerPageToString(startBannerPage);
   const QString endBanner   = bannerPageToString(endBannerPage);

   setCupsOption(cupsOptions, "job-sheets", startBanner + QLatin1Char(',') + endBanner);
   setCupsOptions(printer, cupsOptions);
}

void QCUPSSupport::setPageSet(QPrinter *printer, const PageSet pageSet)
{
   QStringList cupsOptions = cupsOptionsList(printer);
   QString pageSetString;

   switch (pageSet) {
      case OddPages:
         pageSetString = "odd";
         break;

      case EvenPages:
         pageSetString = "even";
         break;

      case AllPages:
         pageSetString = "all";
         break;
   }

   setCupsOption(cupsOptions, "page-set", pageSetString);
   setCupsOptions(printer, cupsOptions);
}

void QCUPSSupport::setPagesPerSheetLayout(QPrinter *printer,  const PagesPerSheet pagesPerSheet,
   const PagesPerSheetLayout pagesPerSheetLayout)
{
   QStringList cupsOptions = cupsOptionsList(printer);

   // WARNING: the following trick (with a [2]-extent) only works as
   // WARNING: long as there's only one two-digit number in the list
   // WARNING: and it is the last one (before the "\0")!

   static const char pagesPerSheetData[][2] = { "1", "2", "4", "6", "9", {'1', '6'}, "\0" };
   static const char pageLayoutData[][5] = {"lrtb", "lrbt", "rlbt", "rltb", "btlr", "btrl", "tblr", "tbrl"};

   setCupsOption(cupsOptions, "number-up", pagesPerSheetData[pagesPerSheet]);
   setCupsOption(cupsOptions, "number-up-layout", pageLayoutData[pagesPerSheetLayout]);
   setCupsOptions(printer, cupsOptions);
}

void QCUPSSupport::setPageRange(QPrinter *printer, int pageFrom, int pageTo)
{
   QStringList cupsOptions = cupsOptionsList(printer);
   setCupsOption(cupsOptions, "page-ranges", QString("%1-%2").formatArg(pageFrom).formatArg(pageTo));
   setCupsOptions(printer, cupsOptions);
}

#endif // QT_NO_CUPS
