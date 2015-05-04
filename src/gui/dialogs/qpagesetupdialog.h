/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
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
   CS_OBJECT(QPageSetupDialog)
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

   explicit QPageSetupDialog(QPrinter *printer, QWidget *parent = 0);
   explicit QPageSetupDialog(QWidget *parent = 0);

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
   virtual void setVisible(bool visible);
#endif

   virtual int exec();
   using QDialog::open;

   void open(QObject *receiver, const char *member);
};

#endif // QT_NO_PRINTDIALOG

QT_END_NAMESPACE

#endif // QPAGESETUPDIALOG_H
