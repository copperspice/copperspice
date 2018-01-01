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

#ifndef QABSTRACTPRINTDIALOG_H
#define QABSTRACTPRINTDIALOG_H

#include <QtGui/qdialog.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PRINTER

class QAbstractPrintDialogPrivate;
class QPrinter;

// ### Qt5/remove this class
class Q_GUI_EXPORT QAbstractPrintDialog : public QDialog
{
   GUI_CS_OBJECT(QAbstractPrintDialog)
   Q_DECLARE_PRIVATE(QAbstractPrintDialog)

 public:
   // Keep in sync with QPrinter::PrintRange
   enum PrintRange {
      AllPages,
      Selection,
      PageRange,
      CurrentPage
   };

   enum PrintDialogOption {
      None                    = 0x0000, // obsolete
      PrintToFile             = 0x0001,
      PrintSelection          = 0x0002,
      PrintPageRange          = 0x0004,
      PrintShowPageSize       = 0x0008,
      PrintCollateCopies      = 0x0010,
      DontUseSheet            = 0x0020,
      PrintCurrentPage        = 0x0040
   };

   using PrintDialogOptions = QFlags<PrintDialogOption>;

#ifndef QT_NO_PRINTDIALOG
   explicit QAbstractPrintDialog(QPrinter *printer, QWidget *parent = nullptr);
   ~QAbstractPrintDialog();

   virtual int exec() = 0;

   // obsolete
   void addEnabledOption(PrintDialogOption option);
   void setEnabledOptions(PrintDialogOptions options);
   PrintDialogOptions enabledOptions() const;
   bool isOptionEnabled(PrintDialogOption option) const;

   void setOptionTabs(const QList<QWidget *> &tabs);

   void setPrintRange(PrintRange range);
   PrintRange printRange() const;

   void setMinMax(int min, int max);
   int minPage() const;
   int maxPage() const;

   void setFromTo(int fromPage, int toPage);
   int fromPage() const;
   int toPage() const;

   QPrinter *printer() const;

 protected:
   QAbstractPrintDialog(QAbstractPrintDialogPrivate &ptr, QPrinter *printer, QWidget *parent = nullptr);

 private:
   Q_DISABLE_COPY(QAbstractPrintDialog)

#endif // QT_NO_PRINTDIALOG
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractPrintDialog::PrintDialogOptions)

#endif // QT_NO_PRINTER

QT_END_NAMESPACE

#endif // QABSTRACTPRINTDIALOG_H
