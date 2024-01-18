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

#ifndef QPAGESETUPDIALOG_H
#define QPAGESETUPDIALOG_H

#include <qdialog.h>

#ifndef QT_NO_PRINTDIALOG

class QPrinter;
class QPageSetupDialogPrivate;

class Q_GUI_EXPORT QPageSetupDialog : public QDialog
{
   GUI_CS_OBJECT(QPageSetupDialog)
   Q_DECLARE_PRIVATE(QPageSetupDialog)

 public:
   explicit QPageSetupDialog(QPrinter *printer, QWidget *parent = nullptr);
   explicit QPageSetupDialog(QWidget *parent = nullptr);

   ~QPageSetupDialog();

#if defined(Q_OS_DARWIN) || defined(Q_OS_WIN)
   void setVisible(bool visible) override;
#endif

   int exec() override;

   using QDialog::open;
   void open(QObject *receiver, const QString &member);

   void done(int result) override;
   QPrinter *printer();
};

#endif // QT_NO_PRINTDIALOG


#endif
