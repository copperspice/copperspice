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

#include "configfile.h"

#include <QFile>

ConfigFile::SectionMap ConfigFile::parse(const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
        return ConfigFile::SectionMap();
    return parse(&f);
}

ConfigFile::SectionMap ConfigFile::parse(QIODevice *dev)
{
    SectionMap sections;
    SectionMap::Iterator currentSection = sections.end();

    ConfigFile::SectionMap result;
    int currentLineNumber = 0;
    while (!dev->atEnd()) {
        QString line = QString::fromUtf8(dev->readLine()).trimmed();
        ++currentLineNumber;

        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;

        if (line.startsWith(QLatin1Char('['))) {
            if (!line.endsWith(']')) {
                qWarning("Syntax error at line %d: Missing ']' at start of new section.", currentLineNumber);
                return SectionMap();
            }
            line.remove(0, 1);
            line.chop(1);
            const QString sectionName = line;
            currentSection = sections.insert(sectionName, Section());
            continue;
        }

        if (currentSection == sections.end()) {
            qWarning("Syntax error at line %d: Entry found outside of any section.", currentLineNumber);
            return SectionMap();
        }

        Entry e;
        e.lineNumber = currentLineNumber;

        int equalPos = line.indexOf(QLatin1Char('='));
        if (equalPos == -1) {
            e.key = line;
        } else {
            e.key = line;
            e.key.truncate(equalPos);
            e.key = e.key.trimmed();
            e.value = line.mid(equalPos + 1).trimmed();
        }
        currentSection->append(e);
    }
    return sections;
}
