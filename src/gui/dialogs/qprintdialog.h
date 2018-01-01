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

#ifndef QPRINTDIALOG_H
#define QPRINTDIALOG_H

#include <QtGui/qabstractprintdialog.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PRINTDIALOG

class QPrintDialogPrivate;
class QPushButton;
class QPrinter;

#if defined (Q_OS_UNIX) && ! defined(Q_OS_MAC)
class QUnixPrintWidgetPrivate;

class Q_GUI_EXPORT QUnixPrintWidget : public QWidget
{
   GUI_CS_OBJECT(QUnixPrintWidget)

 public:
   QUnixPrintWidget(QPrinter *printer, QWidget *parent = nullptr);
   ~QUnixPrintWidget();
   void updatePrinter();

 private:
   friend class QPrintDialogPrivate;
   friend class QUnixPrintWidgetPrivate;
   QUnixPrintWidgetPrivate *d;

   GUI_CS_SLOT_1(Private, void _q_printerChanged(int un_named_arg1))
   GUI_CS_SLOT_2(_q_printerChanged)

   GUI_CS_SLOT_1(Private, void _q_btnBrowseClicked())
   GUI_CS_SLOT_2(_q_btnBrowseClicked)

   GUI_CS_SLOT_1(Private, void _q_btnPropertiesClicked())
   GUI_CS_SLOT_2(_q_btnPropertiesClicked)

};
#endif

class Q_GUI_EXPORT QPrintDialog : public QAbstractPrintDialog
{
   GUI_CS_OBJECT(QPrintDialog)
   Q_DECLARE_PRIVATE(QPrintDialog)

   GUI_CS_ENUM(PrintDialogOption)

   GUI_CS_PROPERTY_READ(options, options)
   GUI_CS_PROPERTY_WRITE(options, setOptions)

 public:
   explicit QPrintDialog(QPrinter *printer, QWidget *parent = nullptr);
   explicit QPrintDialog(QWidget *parent = nullptr);
   ~QPrintDialog();

   int exec() override;

#if defined (Q_OS_UNIX) && !defined(Q_OS_MAC)
   void accept() override;
#endif

   void done(int result) override;
   void setOption(PrintDialogOption option, bool on = true);
   bool testOption(PrintDialogOption option) const;
   void setOptions(PrintDialogOptions options);

   PrintDialogOptions options() const;

#if defined(Q_OS_UNIX) || defined(Q_OS_MAC) || defined(Q_OS_WIN)
   void setVisible(bool visible) override;
#endif

   using QDialog::open;
   void open(QObject *receiver, const char *member);

   using QDialog::accepted;
   GUI_CS_SIGNAL_1(Public, void accepted(QPrinter *printer))
   GUI_CS_SIGNAL_OVERLOAD(accepted, (QPrinter *), printer)

 private:
   GUI_CS_SLOT_1(Private, void _q_chbPrintLastFirstToggled(bool un_named_arg1))
   GUI_CS_SLOT_2(_q_chbPrintLastFirstToggled)

#if defined (Q_OS_UNIX) && !defined (Q_OS_MAC)
   GUI_CS_SLOT_1(Private, void _q_collapseOrExpandDialog())
   GUI_CS_SLOT_2(_q_collapseOrExpandDialog)
#endif

# if defined(Q_OS_UNIX) && !defined (Q_OS_MAC) && !defined(QT_NO_MESSAGEBOX)
   GUI_CS_SLOT_1(Private, void _q_checkFields())
   GUI_CS_SLOT_2(_q_checkFields)
# endif

   friend class QUnixPrintWidget;
};

#endif // QT_NO_PRINTDIALOG

QT_END_NAMESPACE

#endif // QPRINTDIALOG_H
