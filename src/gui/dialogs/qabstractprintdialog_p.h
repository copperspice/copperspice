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

#ifndef QABSTRACTPRINTDIALOG_P_H
#define QABSTRACTPRINTDIALOG_P_H

#include <qdialog_p.h>

#ifndef QT_NO_PRINTDIALOG

#include <QtGui/qabstractprintdialog.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PRINTER

class QPrinter;
class QPrinterPrivate;

class QAbstractPrintDialogPrivate : public QDialogPrivate
{
   Q_DECLARE_PUBLIC(QAbstractPrintDialog)

 public:
   QAbstractPrintDialogPrivate()
      : printer(0), pd(0), ownsPrinter(false)
      , options(QAbstractPrintDialog::PrintToFile | QAbstractPrintDialog::PrintPageRange |
                QAbstractPrintDialog::PrintCollateCopies | QAbstractPrintDialog::PrintShowPageSize) {
   }

   QPrinter *printer;
   QPrinterPrivate *pd;
   bool ownsPrinter;
   QPointer<QObject> receiverToDisconnectOnClose;
   QByteArray memberToDisconnectOnClose;

   QAbstractPrintDialog::PrintDialogOptions options;

   virtual void setTabs(const QList<QWidget *> &) {}
   void setPrinter(QPrinter *newPrinter);
};

#endif //QT_NO_PRINTER

QT_END_NAMESPACE

#endif // QT_NO_PRINTDIALOG

#endif // QABSTRACTPRINTDIALOG_P_H
