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

#include <qabstractpagesetupdialog.h>
#include <qabstractpagesetupdialog_p.h>

#ifndef QT_NO_PRINTDIALOG

#include <QtCore/qcoreapplication.h>
#include <QtGui/qprinter.h>

QT_BEGIN_NAMESPACE

/*!
    \internal
    \class QAbstractPageSetupDialog

    \brief The QAbstractPageSetupDialog class provides a base for
    implementations of page setup dialogs.
*/

/*!
    Constructs the page setup dialog for the printer \a printer with
    \a parent as parent widget.
*/
QAbstractPageSetupDialog::QAbstractPageSetupDialog(QPrinter *printer, QWidget *parent)
   : QDialog(*(new QAbstractPageSetupDialogPrivate), parent)
{
   Q_D(QAbstractPageSetupDialog);
   setWindowTitle(QCoreApplication::translate("QPrintPreviewDialog", "Page Setup"));
   d->setPrinter(printer);
}

/*!
    \internal
*/
QAbstractPageSetupDialog::QAbstractPageSetupDialog(QAbstractPageSetupDialogPrivate &ptr,
      QPrinter *printer, QWidget *parent)
   : QDialog(ptr, parent)
{
   Q_D(QAbstractPageSetupDialog);
   setWindowTitle(QCoreApplication::translate("QPrintPreviewDialog", "Page Setup"));
   d->setPrinter(printer);
}

QAbstractPageSetupDialog::~QAbstractPageSetupDialog()
{
   Q_D(QAbstractPageSetupDialog);
   if (d->opts & QPageSetupDialog::OwnsPrinter) {
      delete d->printer;
   }
}

/*!
    Returns the printer that this page setup dialog is operating on.
*/
QPrinter *QAbstractPageSetupDialog::printer()
{
   Q_D(QAbstractPageSetupDialog);
   return d->printer;
}

void QAbstractPageSetupDialogPrivate::setPrinter(QPrinter *newPrinter)
{
   if (newPrinter) {
      printer = newPrinter;
   } else {
      printer = new QPrinter;
      opts |= QPageSetupDialog::OwnsPrinter;
   }

#ifndef Q_WS_X11
   if (printer->outputFormat() != QPrinter::NativeFormat) {
      qWarning("QPageSetupDialog: Can not be used on non-native printers");
   }
#endif
}

/*!
    \fn int QAbstractPageSetupDialog::exec()

    This virtual function is called to pop up the dialog. It must be
    reimplemented in subclasses.
*/

/*!
    \reimp
*/
void QAbstractPageSetupDialog::done(int result)
{
   Q_D(QAbstractPageSetupDialog);
   QDialog::done(result);

   if (d->receiverToDisconnectOnClose) {
      disconnect(this, SIGNAL(accepted()), d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose.constData());
      d->receiverToDisconnectOnClose = 0;
   }

   d->memberToDisconnectOnClose.clear();
}

QT_END_NAMESPACE

#endif // QT_NO_PRINTDIALOG
