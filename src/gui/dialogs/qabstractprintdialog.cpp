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

#include <qabstractprintdialog_p.h>
#include <qcoreapplication.h>
#include <qprintdialog.h>
#include <qprinter.h>
#include <qprinter_p.h>

#ifndef QT_NO_PRINTDIALOG

QT_BEGIN_NAMESPACE

// hack - awful implementation from Qt
class QPrintDialogPrivate : public QAbstractPrintDialogPrivate
{
};


QAbstractPrintDialog::QAbstractPrintDialog(QPrinter *printer, QWidget *parent)
   : QDialog(*(new QAbstractPrintDialogPrivate), parent)
{
   Q_D(QAbstractPrintDialog);
   setWindowTitle(QCoreApplication::translate("QPrintDialog", "Print"));
   d->setPrinter(printer);
}

/*!
     \internal
*/
QAbstractPrintDialog::QAbstractPrintDialog(QAbstractPrintDialogPrivate &ptr, QPrinter *printer, QWidget *parent)
   : QDialog(ptr, parent)
{
   Q_D(QAbstractPrintDialog);
   setWindowTitle(QCoreApplication::translate("QPrintDialog", "Print"));
   d->setPrinter(printer);
}

/*!
    \internal
*/
QAbstractPrintDialog::~QAbstractPrintDialog()
{
   Q_D(QAbstractPrintDialog);
   if (d->ownsPrinter) {
      delete d->printer;
   }
}

/*!
    Sets the given \a option to be enabled if \a on is true;
    otherwise, clears the given \a option.

    \sa options, testOption()
*/
void QPrintDialog::setOption(PrintDialogOption option, bool on)
{
   Q_D(QPrintDialog);
   if (!(d->options & option) != !on) {
      setOptions(d->options ^ option);
   }
}

/*!
    Returns true if the given \a option is enabled; otherwise, returns
    false.

    \sa options, setOption()
*/
bool QPrintDialog::testOption(PrintDialogOption option) const
{
   Q_D(const QPrintDialog);
   return (d->options & option) != 0;
}

void QPrintDialog::setOptions(PrintDialogOptions options)
{
   Q_D(QPrintDialog);

   PrintDialogOptions changed = (options ^ d->options);
   if (!changed) {
      return;
   }

   d->options = options;
}

QPrintDialog::PrintDialogOptions QPrintDialog::options() const
{
   Q_D(const QPrintDialog);
   return d->options;
}

/*!
    \obsolete

    Use QPrintDialog::setOptions() instead.
*/
void QAbstractPrintDialog::setEnabledOptions(PrintDialogOptions options)
{
   Q_D(QAbstractPrintDialog);
   d->options = options;
}

/*!
    \obsolete

    Use QPrintDialog::setOption(\a option, true) instead.
*/
void QAbstractPrintDialog::addEnabledOption(PrintDialogOption option)
{
   Q_D(QAbstractPrintDialog);
   d->options |= option;
}

/*!
    \obsolete

    Use QPrintDialog::options() instead.
*/
QAbstractPrintDialog::PrintDialogOptions QAbstractPrintDialog::enabledOptions() const
{
   Q_D(const QAbstractPrintDialog);
   return d->options;
}

/*!
    \obsolete

    Use QPrintDialog::testOption(\a option) instead.
*/
bool QAbstractPrintDialog::isOptionEnabled(PrintDialogOption option) const
{
   Q_D(const QAbstractPrintDialog);
   return d->options & option;
}

/*!
    Sets the print range option in to be \a range.
 */
void QAbstractPrintDialog::setPrintRange(PrintRange range)
{
   Q_D(QAbstractPrintDialog);
   d->pd->printRange = QPrinter::PrintRange(range);
}

/*!
    Returns the print range.
*/
QAbstractPrintDialog::PrintRange QAbstractPrintDialog::printRange() const
{
   Q_D(const QAbstractPrintDialog);
   return QAbstractPrintDialog::PrintRange(d->pd->printRange);
}

/*!
    Sets the page range in this dialog to be from \a min to \a max. This also
    enables the PrintPageRange option.
*/
void QAbstractPrintDialog::setMinMax(int min, int max)
{
   Q_D(QAbstractPrintDialog);
   Q_ASSERT_X(min <= max, "QAbstractPrintDialog::setMinMax",
              "'min' must be less than or equal to 'max'");
   d->pd->minPage = min;
   d->pd->maxPage = max;
   d->options |= PrintPageRange;
}

/*!
    Returns the minimum page in the page range.
    By default, this value is set to 1.
*/
int QAbstractPrintDialog::minPage() const
{
   Q_D(const QAbstractPrintDialog);
   return d->pd->minPage;
}

/*!
    Returns the maximum page in the page range. As of Qt 4.4, this
    function returns INT_MAX by default. Previous versions returned 1
    by default.
*/
int QAbstractPrintDialog::maxPage() const
{
   Q_D(const QAbstractPrintDialog);
   return d->pd->maxPage;
}

/*!
    Sets the range in the print dialog to be from \a from to \a to.
*/
void QAbstractPrintDialog::setFromTo(int from, int to)
{
   Q_D(QAbstractPrintDialog);
   Q_ASSERT_X(from <= to, "QAbstractPrintDialog::setFromTo",
              "'from' must be less than or equal to 'to'");
   d->pd->fromPage = from;
   d->pd->toPage = to;

   if (d->pd->minPage == 0 && d->pd->maxPage == 0) {
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
   return d->pd->fromPage;
}

/*!
    Returns the last page to be printed.
    By default, this value is set to 0.
*/
int QAbstractPrintDialog::toPage() const
{
   Q_D(const QAbstractPrintDialog);
   return d->pd->toPage;
}


/*!
    Returns the printer that this printer dialog operates
    on.
*/
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

/*!
    \fn int QAbstractPrintDialog::exec()

    This virtual function is called to pop up the dialog. It must be
    reimplemented in subclasses.
*/

/*!
    \class QPrintDialog

    \brief The QPrintDialog class provides a dialog for specifying
    the printer's configuration.

    \ingroup standard-dialogs
    \ingroup printing

    The dialog allows users to change document-related settings, such
    as the paper size and orientation, type of print (color or
    grayscale), range of pages, and number of copies to print.

    Controls are also provided to enable users to choose from the
    printers available, including any configured network printers.

    Typically, QPrintDialog objects are constructed with a QPrinter
    object, and executed using the exec() function.

    \snippet doc/src/snippets/code/src_gui_dialogs_qabstractprintdialog.cpp 0

    If the dialog is accepted by the user, the QPrinter object is
    correctly configured for printing.

    \table
    \row
    \o \inlineimage plastique-printdialog.png
    \o \inlineimage plastique-printdialog-properties.png
    \endtable

    The printer dialog (shown above in Plastique style) enables access to common
    printing properties. On X11 platforms that use the CUPS printing system, the
    settings for each available printer can be modified via the dialog's
    \gui{Properties} push button.

    On Windows and Mac OS X, the native print dialog is used, which means that
    some QWidget and QDialog properties set on the dialog won't be respected.
    The native print dialog on Mac OS X does not support setting printer options,
    i.e. setOptions() and setOption() have no effect.

    In Qt 4.4, it was possible to use the static functions to show a sheet on
    Mac OS X. This is no longer supported in Qt 4.5. If you want this
    functionality, use QPrintDialog::open().

    \sa QPageSetupDialog, QPrinter, {Pixelator Example}, {Order Form Example},
        {Image Viewer Example}, {Scribble Example}
*/

/*!
    \fn QPrintDialog::QPrintDialog(QPrinter *printer, QWidget *parent)

    Constructs a new modal printer dialog for the given \a printer
    with the given \a parent.
*/

/*!
    \fn QPrintDialog::~QPrintDialog()

    Destroys the print dialog.
*/

/*!
    \fn int QPrintDialog::exec()
    \reimp
*/

/*!
    \since 4.4

    Set a list of widgets as \a tabs to be shown on the print dialog, if supported.

    Currently this option is only supported on X11.

    Setting the option tabs will transfer their ownership to the print dialog.
*/
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
      disconnect(this, SIGNAL(accepted(QPrinter *)),
                 d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);
      d->receiverToDisconnectOnClose = 0;
   }
   d->memberToDisconnectOnClose.clear();
}

/*!
    \since 4.5
    \overload

    Opens the dialog and connects its accepted() signal to the slot specified
    by \a receiver and \a member.

    The signal will be disconnected from the slot when the dialog is closed.
*/
void QPrintDialog::open(QObject *receiver, const char *member)
{
   Q_D(QPrintDialog);
   connect(this, SIGNAL(accepted(QPrinter *)), receiver, member);
   d->receiverToDisconnectOnClose = receiver;
   d->memberToDisconnectOnClose = member;
   QDialog::open();
}

QT_END_NAMESPACE

#endif // QT_NO_PRINTDIALOG
