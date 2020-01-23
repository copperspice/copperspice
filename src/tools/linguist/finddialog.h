/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include "ui_finddialog.h"
#include "messagemodel.h"

#include <QDialog>

QT_BEGIN_NAMESPACE

class FindDialog : public QDialog, public Ui::FindDialog
{
   Q_OBJECT
 public:
   FindDialog(QWidget *parent = nullptr);

 signals:
   void findNext(const QString &text, DataModel::FindLocation where, bool matchCase, bool ignoreAccelerators);

 private slots:
   void emitFindNext();
   void verifyText(const QString &);
   void find();
};

QT_END_NAMESPACE

#endif
