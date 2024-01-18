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

#include <statistics.h>

Statistics::Statistics(QWidget *parent, Qt::WindowFlags flags)
   : QDialog(parent, flags)
{
   setupUi(this);
}

void Statistics::languageChange()
{
   retranslateUi(this);
}

void Statistics::updateStats(int sW, int sC, int sCS, int trW, int trC, int trCS)
{
   untrWords->setText(QString::number(sW));
   untrChars->setText(QString::number(sC));
   untrCharsSpc->setText(QString::number(sCS));
   trWords->setText(QString::number(trW));
   trChars->setText(QString::number(trC));
   trCharsSpc->setText(QString::number(trCS));
}
