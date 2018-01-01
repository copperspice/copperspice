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

#ifndef QPAGESETUPDIALOG_H
#define QPAGESETUPDIALOG_H

#include <QtGui/qabstractpagesetupdialog.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PRINTDIALOG

class QPageSetupDialogPrivate;

class Q_GUI_EXPORT QPageSetupDialog : public QAbstractPageSetupDialog
{
   GUI_CS_OBJECT(QPageSetupDialog)
   Q_DECLARE_PRIVATE(QPageSetupDialog)

   GUI_CS_ENUM(PageSetupDialogOption)
   GUI_CS_PROPERTY_READ(options, options)
   GUI_CS_PROPERTY_WRITE(options, setOptions)

 public:
   enum PageSetupDialogOption {
      None                    = 0x00000000, // internal
      DontUseSheet            = 0x00000001,
      OwnsPrinter             = 0x80000000  // internal
   };

   using PageSetupDialogOptions = QFlags<PageSetupDialogOption>;

   explicit QPageSetupDialog(QPrinter *printer, QWidget *parent = nullptr);
   explicit QPageSetupDialog(QWidget *parent = nullptr);

   // obsolete
   void addEnabledOption(PageSetupDialogOption option);
   void setEnabledOptions(PageSetupDialogOptions options);
   PageSetupDialogOptions enabledOptions() const;
   bool isOptionEnabled(PageSetupDialogOption option) const;

   void setOption(PageSetupDialogOption option, bool on = true);
   bool testOption(PageSetupDialogOption option) const;
   void setOptions(PageSetupDialogOptions options);
   PageSetupDialogOptions options() const;

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
   void setVisible(bool visible) override;
#endif

   int exec() override;

   using QDialog::open;
   void open(QObject *receiver, const char *member);

   QPrinter *printer() {
      return QAbstractPageSetupDialog::printer();
   }
};

#endif

QT_END_NAMESPACE

#endif
