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

#include <qpagesetupdialog.h>
#include <qpagesetupdialog_p.h>

#include <qprinter.h>

#ifndef QT_NO_PRINTDIALOG

QPageSetupDialogPrivate::QPageSetupDialogPrivate(QPrinter *prntr)
   : printer(nullptr), ownsPrinter(false)
{
   setPrinter(prntr);
}

void QPageSetupDialogPrivate::setPrinter(QPrinter *newPrinter)
{
   if (printer && ownsPrinter) {
      delete printer;
   }
   if (newPrinter) {
      printer = newPrinter;
      ownsPrinter = false;
   } else {
      printer = new QPrinter;
      ownsPrinter = true;
   }

   if (printer->outputFormat() != QPrinter::NativeFormat) {
      qWarning("setPrinter() Unable to use on a non-native printer");
   }
}

void QPageSetupDialog::open(QObject *receiver, const QString &member)
{
   Q_D(QPageSetupDialog);
   connect(this, SIGNAL(accepted()), receiver, member);
   d->receiverToDisconnectOnClose = receiver;
   d->memberToDisconnectOnClose   = member;
   QDialog::open();
}

QPageSetupDialog::~QPageSetupDialog()
{
   Q_D(QPageSetupDialog);
   if (d->ownsPrinter) {
      delete d->printer;
   }
}

QPrinter *QPageSetupDialog::printer()
{
   Q_D(QPageSetupDialog);
   return d->printer;
}

void QPageSetupDialog::done(int result)
{
   Q_D(QPageSetupDialog);
   QDialog::done(result);

   if (d->receiverToDisconnectOnClose) {
      disconnect(this, SIGNAL(accepted()), d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);
      d->receiverToDisconnectOnClose = nullptr;
   }
   d->memberToDisconnectOnClose.clear();
}

#endif
