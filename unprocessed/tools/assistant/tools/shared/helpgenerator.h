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

#ifndef HELPGENERATOR_H
#define HELPGENERATOR_H

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QHelpDataInterface;
class QHelpGenerator;

class HelpGenerator : public QObject
{
    Q_OBJECT

public:
    HelpGenerator();
    bool generate(QHelpDataInterface *helpData,
        const QString &outputFileName);
    bool checkLinks(const QHelpDataInterface &helpData);
    QString error() const;

private slots:
    void printStatus(const QString &msg);
    void printWarning(const QString &msg);
    
private:
    QHelpGenerator *generator;
};

QT_END_NAMESPACE

#endif
