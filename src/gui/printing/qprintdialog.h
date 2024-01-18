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

#ifndef QPRINTDIALOG_H
#define QPRINTDIALOG_H

#include <qabstractprintdialog.h>

#ifndef QT_NO_PRINTDIALOG

class QPrintDialogPrivate;
class QPushButton;
class QPrinter;

class Q_GUI_EXPORT QPrintDialog : public QAbstractPrintDialog
{
   GUI_CS_OBJECT(QPrintDialog)
   Q_DECLARE_PRIVATE(QPrintDialog)

   GUI_CS_ENUM(PrintDialogOption)

   GUI_CS_PROPERTY_READ(options,  options)
   GUI_CS_PROPERTY_WRITE(options, setOptions)

 public:
   explicit QPrintDialog(QPrinter *printer, QWidget *parent = nullptr);
   explicit QPrintDialog(QWidget *parent = nullptr);
   ~QPrintDialog();

   int exec() override;

#if defined (Q_OS_UNIX) && ! defined(Q_OS_DARWIN)
   void accept() override;
#endif

   void done(int result) override;
   void setOption(PrintDialogOption option, bool on = true);
   bool testOption(PrintDialogOption option) const;
   void setOptions(PrintDialogOptions options);

   PrintDialogOptions options() const;

#if defined(Q_OS_UNIX) || defined(Q_OS_WIN)
   void setVisible(bool visible) override;
#endif

   using QDialog::open;
   void open(QObject *receiver, const QString &member);

   using QDialog::accepted;
   GUI_CS_SIGNAL_1(Public, void accepted(QPrinter *printer))
   GUI_CS_SIGNAL_OVERLOAD(accepted, (QPrinter *), printer)

 private:

#if defined (Q_OS_UNIX) && ! defined (Q_OS_DARWIN)

   GUI_CS_SLOT_1(Private, void _q_togglePageSetCombo(bool arg1))
   GUI_CS_SLOT_2(_q_togglePageSetCombo)

   GUI_CS_SLOT_1(Private, void _q_collapseOrExpandDialog())
   GUI_CS_SLOT_2(_q_collapseOrExpandDialog)

   GUI_CS_SLOT_1(Private, void _q_checkFields())
   GUI_CS_SLOT_2(_q_checkFields)

   friend class QUnixPrintWidget;
#endif

};

#endif // QT_NO_PRINTDIALOG



#endif
