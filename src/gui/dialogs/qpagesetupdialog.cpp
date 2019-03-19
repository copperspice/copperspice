/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qabstractpagesetupdialog_p.h>

#ifndef QT_NO_PRINTDIALOG

QT_BEGIN_NAMESPACE

// hack
class QPageSetupDialogPrivate : public QAbstractPageSetupDialogPrivate
{
};

void QPageSetupDialog::setOption(PageSetupDialogOption option, bool on)
{
   Q_D(QPageSetupDialog);
   if (!(d->opts & option) != !on) {
      setOptions(d->opts ^ option);
   }
}

bool QPageSetupDialog::testOption(PageSetupDialogOption option) const
{
   Q_D(const QPageSetupDialog);
   return (d->opts & option) != 0;
}

void QPageSetupDialog::setOptions(PageSetupDialogOptions options)
{
   Q_D(QPageSetupDialog);

   PageSetupDialogOptions changed = (options ^ d->opts);
   if (!changed) {
      return;
   }

   d->opts = options;
}

QPageSetupDialog::PageSetupDialogOptions QPageSetupDialog::options() const
{
   Q_D(const QPageSetupDialog);
   return d->opts;
}

/*!
    \obsolete

    Use setOption(\a option, true) instead.
*/
void QPageSetupDialog::addEnabledOption(PageSetupDialogOption option)
{
   setOption(option, true);
}

/*!
    \obsolete

    Use setOptions(\a options) instead.
*/
void QPageSetupDialog::setEnabledOptions(PageSetupDialogOptions options)
{
   setOptions(options);
}

/*!
    \obsolete

    Use options() instead.
*/
QPageSetupDialog::PageSetupDialogOptions QPageSetupDialog::enabledOptions() const
{
   return options();
}

/*!
    \obsolete

    Use testOption(\a option) instead.
*/
bool QPageSetupDialog::isOptionEnabled(PageSetupDialogOption option) const
{
   return testOption(option);
}

void QPageSetupDialog::open(QObject *receiver, const QString &member)
{
   Q_D(QPageSetupDialog);
   connect(this, SIGNAL(accepted()), receiver, member);
   d->receiverToDisconnectOnClose = receiver;
   d->memberToDisconnectOnClose = member;
   QDialog::open();
}

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
/*! \fn void QPageSetupDialog::setVisible(bool visible)
    \reimp
*/
#endif

QT_END_NAMESPACE

#endif
