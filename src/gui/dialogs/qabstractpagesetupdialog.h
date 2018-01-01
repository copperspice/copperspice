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

#ifndef QABSTRACTPAGESETUPDIALOG_H
#define QABSTRACTPAGESETUPDIALOG_H

#include <QtGui/qdialog.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PRINTDIALOG

class QAbstractPageSetupDialogPrivate;
class QPrinter;

// ### Qt5 - Remove this class
class Q_GUI_EXPORT QAbstractPageSetupDialog : public QDialog
{
   GUI_CS_OBJECT(QAbstractPageSetupDialog)
   Q_DECLARE_PRIVATE(QAbstractPageSetupDialog)

 public:
   explicit QAbstractPageSetupDialog(QPrinter *printer, QWidget *parent = nullptr);
   QAbstractPageSetupDialog(QAbstractPageSetupDialogPrivate &ptr,
                            QPrinter *printer, QWidget *parent = nullptr);
   ~QAbstractPageSetupDialog();

   virtual int exec() = 0;
   void done(int result) override;

   QPrinter *printer();
};

#endif // QT_NO_PRINTDIALOG

QT_END_NAMESPACE

#endif // QABSTRACTPAGESETUPDIALOG_H
