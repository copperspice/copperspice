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

#ifndef QERRORMESSAGE_H
#define QERRORMESSAGE_H

#include <qdialog.h>

#ifndef QT_NO_ERRORMESSAGE

class QErrorMessagePrivate;

class Q_GUI_EXPORT QErrorMessage: public QDialog
{
   GUI_CS_OBJECT(QErrorMessage)

 public:
   explicit QErrorMessage(QWidget *parent = nullptr);

   QErrorMessage(const QErrorMessage &) = delete;
   QErrorMessage &operator=(const QErrorMessage &) = delete;

   ~QErrorMessage();

   GUI_CS_SLOT_1(Public, void showMessage(const QString &message))
   GUI_CS_SLOT_OVERLOAD(showMessage, (const QString &))

   GUI_CS_SLOT_1(Public, void showMessage(const QString &message, const QString &type))
   GUI_CS_SLOT_OVERLOAD(showMessage, (const QString &, const QString &))

 protected:
   void done(int status) override;
   void changeEvent(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QErrorMessage)
};

#endif // QT_NO_ERRORMESSAGE

#endif
