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

#include "helpgenerator.h"

#include <private/qhelpgenerator_p.h>
#include <stdio.h>

QT_BEGIN_NAMESPACE

HelpGenerator::HelpGenerator()
{
    generator = new QHelpGenerator(this);
    connect(generator, SIGNAL(statusChanged(QString)),
        this, SLOT(printStatus(QString)));
    connect(generator, SIGNAL(warning(QString)),
        this, SLOT(printWarning(QString)));
}

bool HelpGenerator::generate(QHelpDataInterface *helpData,
                             const QString &outputFileName)
{
    return generator->generate(helpData, outputFileName);
}

bool HelpGenerator::checkLinks(const QHelpDataInterface &helpData)
{
    return generator->checkLinks(helpData);
}

QString HelpGenerator::error() const
{
    return generator->error();
}

void HelpGenerator::printStatus(const QString &msg)
{
    puts(qPrintable(msg));
}

void HelpGenerator::printWarning(const QString &msg)
{
    puts(qPrintable(tr("Warning: %1").arg(msg)));
}

QT_END_NAMESPACE
