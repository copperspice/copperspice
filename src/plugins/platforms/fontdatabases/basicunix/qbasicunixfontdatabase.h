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

#ifndef QBASICUNIXFONTDATABASE_H
#define QBASICUNIXFONTDATABASE_H

#include <QPlatformFontDatabase>
#include <QtCore/QByteArray>
#include <QtCore/QString>

struct FontFile
{
    QString fileName;
    int indexValue;
};

class QBasicUnixFontDatabase : public QPlatformFontDatabase
{
public:
    void populateFontDatabase();
    QFontEngine *fontEngine(const QFontDef &fontDef, QUnicodeTables::Script script, void *handle);
    QStringList fallbacksForFamily(const QString family, const QFont::Style &style, const QFont::StyleHint &styleHint, const QUnicodeTables::Script &script) const;
    QStringList addApplicationFont(const QByteArray &fontData, const QString &fileName);
    void releaseHandle(void *handle);

    static QStringList addTTFile(const QByteArray &fontData, const QByteArray &file);
};

#endif // QBASICUNIXFONTDATABASE_H
