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

#include <qabstractprintdialog_p.h>
#include <qcoreapplication.h>
#include <qprintdialog.h>
#include <qprinter.h>

#include <qprinter_p.h>

#ifndef QT_NO_PRINTDIALOG

// awful implementation
class QPrintDialogPrivate : public QAbstractPrintDialogPrivate
{
};

QAbstractPrintDialog::QAbstractPrintDialog(QPrinter *printer, QWidget *parent)
   : QDialog(*(new QAbstractPrintDialogPrivate), parent)
{
   Q_D(QAbstractPrintDialog);
   setWindowTitle(QCoreApplication::translate("QPrintDialog", "Print"));

   d->setPrinter(printer);
   d->minPage = printer->fromPage();
   int to = printer->toPage();
   d->maxPage = to > 0 ? to : INT_MAX;
}

QAbstractPrintDialog::QAbstractPrintDialog(QAbstractPrintDialogPrivate &ptr, QPrinter *printer, QWidget *parent)
   : QDialog(ptr, parent)
{
   Q_D(QAbstractPrintDialog);
   setWindowTitle(QCoreApplication::translate("QPrintDialog", "Print"));
   d->setPrinter(printer);
}

QAbstractPrintDialog::~QAbstractPrintDialog()
{
   Q_D(QAbstractPrintDialog);
   if (d->ownsPrinter) {
      delete d->printer;
   }
}

void QPrintDialog::setOption(PrintDialogOption option, bool on)
{
   Q_D(QPrintDialog);
   if (!(d->options & option) != !on) {
      setOptions(d->options ^ option);
   }
}

bool QPrintDialog::testOption(PrintDialogOption option) const
{
   Q_D(const QPrintDialog);
   return (d->options & option) != 0;
}

void QPrintDialog::setOptions(PrintDialogOptions options)
{
   Q_D(QPrintDialog);

   PrintDialogOptions changed = (options ^ d->options);
   if (! changed) {
      return;
   }

   d->options = options;
}

QPrintDialog::PrintDialogOptions QPrintDialog::options() const
{
   Q_D(const QPrintDialog);
   return d->options;
}

void QAbstractPrintDialog::setEnabledOptions(PrintDialogOptions options)
{
   Q_D(QAbstractPrintDialog);
   d->options = options;
}

void QAbstractPrintDialog::addEnabledOption(PrintDialogOption option)
{
   Q_D(QAbstractPrintDialog);
   d->options |= option;
}

QAbstractPrintDialog::PrintDialogOptions QAbstractPrintDialog::enabledOptions() const
{
   Q_D(const QAbstractPrintDialog);
   return d->options;
}

bool QAbstractPrintDialog::isOptionEnabled(PrintDialogOption option) const
{
   Q_D(const QAbstractPrintDialog);
   return d->options & option;
}

void QAbstractPrintDialog::setPrintRange(PrintRange range)
{
   Q_D(QAbstractPrintDialog);
   d->printer->setPrintRange(QPrinter::PrintRange(range));
}

QAbstractPrintDialog::PrintRange QAbstractPrintDialog::printRange() const
{
   Q_D(const QAbstractPrintDialog);
   return QAbstractPrintDialog::PrintRange(d->pd->printRange);
}

void QAbstractPrintDialog::setMinMax(int min, int max)
{
   Q_D(QAbstractPrintDialog);
   Q_ASSERT_X(min <= max, "QAbstractPrintDialog::setMinMax",
      "'min' must be less than or equal to 'max'");

   d->minPage = min;
   d->maxPage = max;
   d->options |= PrintPageRange;
}

int QAbstractPrintDialog::minPage() const
{
   Q_D(const QAbstractPrintDialog);
   return d->minPage;
}

int QAbstractPrintDialog::maxPage() const
{
   Q_D(const QAbstractPrintDialog);
   return d->maxPage;
}

void QAbstractPrintDialog::setFromTo(int from, int to)
{
   Q_D(QAbstractPrintDialog);
   Q_ASSERT_X(from <= to, "QAbstractPrintDialog::setFromTo",
      "'from' must be less than or equal to 'to'");

   d->printer->setFromTo(from, to);

   if (d->minPage == 0 && d->maxPage == 0) {
      setMinMax(1, to);
   }
}

/*!
    Returns the first page to be printed
    By default, this value is set to 0.
*/
int QAbstractPrintDialog::fromPage() const
{
   Q_D(const QAbstractPrintDialog);
   return d->printer->fromPage();
}

int QAbstractPrintDialog::toPage() const
{
   Q_D(const QAbstractPrintDialog);
   return d->printer->toPage();
}

QPrinter *QAbstractPrintDialog::printer() const
{
   Q_D(const QAbstractPrintDialog);
   return d->printer;
}

void QAbstractPrintDialogPrivate::setPrinter(QPrinter *newPrinter)
{
   if (newPrinter) {
      printer = newPrinter;
      ownsPrinter = false;
      if (printer->fromPage() || printer->toPage()) {
         options |= QAbstractPrintDialog::PrintPageRange;
      }
   } else {
      printer = new QPrinter;
      ownsPrinter = true;
   }
   pd = printer->d_func();
}

void QAbstractPrintDialog::setOptionTabs(const QList<QWidget *> &tabs)
{
   Q_D(QAbstractPrintDialog);
   d->setTabs(tabs);
}

void QPrintDialog::done(int result)
{
   Q_D(QPrintDialog);

   QDialog::done(result);

   if (result == Accepted) {
      emit accepted(printer());
   }

   if (d->receiverToDisconnectOnClose) {
      disconnect(this, SIGNAL(accepted(QPrinter *)), d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);

      d->receiverToDisconnectOnClose = nullptr;
   }

   d->memberToDisconnectOnClose.clear();
}

void QPrintDialog::open(QObject *receiver, const QString &member)
{
   Q_D(QPrintDialog);

   connect(this, SIGNAL(accepted(QPrinter *)), receiver, member);
   d->receiverToDisconnectOnClose = receiver;
   d->memberToDisconnectOnClose   = member;

   QDialog::open();
}

#endif