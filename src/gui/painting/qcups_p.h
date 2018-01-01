/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QCUPS_P_H
#define QCUPS_P_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qprinter.h>

#ifndef QT_NO_CUPS
#include <QtCore/qlibrary.h>
#include <cups/cups.h>
#include <cups/ppd.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_TYPEINFO(cups_option_t, Q_MOVABLE_TYPE | Q_PRIMITIVE_TYPE);

class QCUPSSupport
{
 public:
   QCUPSSupport();
   ~QCUPSSupport();

   static bool isAvailable();
   static int cupsVersion() {
      return isAvailable() ? CUPS_VERSION_MAJOR * 10000 + CUPS_VERSION_MINOR * 100 + CUPS_VERSION_PATCH : 0;
   }

   int availablePrintersCount() const;
   const cups_dest_t *availablePrinters() const;
   int currentPrinterIndex() const;
   const ppd_file_t *setCurrentPrinter(int index);

   const ppd_file_t *currentPPD() const;
   const ppd_option_t *ppdOption(const char *key) const;

   const cups_option_t *printerOption(const QString &key) const;
   const ppd_option_t *pageSizes() const;

   int markOption(const char *name, const char *value);
   void saveOptions(QList<const ppd_option_t *> options, QList<const char *> markedOptions);

   QRect paperRect(const char *choice) const;
   QRect pageRect(const char *choice) const;

   QStringList options() const;

   static bool printerHasPPD(const char *printerName);

   QString unicodeString(const char *s);

   QPair<int, QString> tempFd();
   int printFile(const char *printerName, const char *filename, const char *title,
                 int num_options, cups_option_t *options);

 private:
   void collectMarkedOptions(QStringList &list, const ppd_group_t *group = 0) const;
   void collectMarkedOptionsHelper(QStringList &list, const ppd_group_t *group) const;

   int prnCount;
   cups_dest_t *printers;
   const ppd_option_t *page_sizes;
   int currPrinterIndex;
   ppd_file_t *currPPD;

#ifndef QT_NO_TEXTCODEC
   QTextCodec *codec;
#endif

};

QT_END_NAMESPACE

#endif // QT_NO_CUPS

#endif
