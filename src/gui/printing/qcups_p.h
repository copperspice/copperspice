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

#ifndef QCUPS_P_H
#define QCUPS_P_H

#include <qstring.h>
#include <qstringlist.h>
#include <qprinter.h>
#include <qdatetime.h>

#ifndef QT_NO_CUPS

// emerald - Define these temporarily so they can be used in the dialogs without a
// circular reference to QCupsPrintEngine in the plugin.
// Move back to qcupsprintengine_p.h in the plugin once all usage removed from the dialogs.

#define PPK_CupsOptions QPrintEngine::PrintEnginePropertyKey(0xfe00)

class Q_GUI_EXPORT QCUPSSupport
{
 public:
   // Enum for values of job-hold-until option
   enum JobHoldUntil {
      NoHold = 0,  //CUPS Default
      Indefinite,
      DayTime,
      Night,
      SecondShift,
      ThirdShift,
      Weekend,
      SpecificTime
   };

   // Enum for valid banner pages
   enum BannerPage {
      NoBanner = 0,  //CUPS Default 'none'
      Standard,
      Unclassified,
      Confidential,
      Classified,
      Secret,
      TopSecret
   };

   // Enum for valid page set
   enum PageSet {
      AllPages = 0,  //CUPS Default
      OddPages,
      EvenPages
   };

   // Enum for valid number of pages per sheet
   enum PagesPerSheet {
      OnePagePerSheet = 0,
      TwoPagesPerSheet,
      FourPagesPerSheet,
      SixPagesPerSheet,
      NinePagesPerSheet,
      SixteenPagesPerSheet
   };

   // Enum for valid layouts of pages per sheet
   enum PagesPerSheetLayout {
      LeftToRightTopToBottom = 0,
      LeftToRightBottomToTop,
      RightToLeftTopToBottom,
      RightToLeftBottomToTop,
      BottomToTopLeftToRight,
      BottomToTopRightToLeft,
      TopToBottomLeftToRight,
      TopToBottomRightToLeft
   };

   static QStringList cupsOptionsList(QPrinter *printer);
   static void setCupsOptions(QPrinter *printer, const QStringList &cupsOptions);
   static void setCupsOption(QStringList &cupsOptions, const QString &option, const QString &value);
   static void clearCupsOption(QStringList &cupsOptions, const QString &option);

   static void setJobHold(QPrinter *printer, const JobHoldUntil jobHold = NoHold, const QTime &holdUntilTime = QTime());
   static void setJobBilling(QPrinter *printer, const QString &jobBilling = QString());
   static void setJobPriority(QPrinter *printer, int priority = 50);
   static void setBannerPages(QPrinter *printer, const BannerPage startBannerPage, const BannerPage endBannerPage);
   static void setPageSet(QPrinter *printer, const PageSet pageSet);
   static void setPagesPerSheetLayout(QPrinter *printer, const PagesPerSheet pagesPerSheet,
      const PagesPerSheetLayout pagesPerSheetLayout);

   static void setPageRange(QPrinter *printer, int pageFrom, int pageTo);
};

#endif // QT_NO_CUPS

#endif
