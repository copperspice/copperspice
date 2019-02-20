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

#ifndef QBASICFONTDATABASE_H
#define QBASICFONTDATABASE_H

#include <qplatform_fontdatabase.h>
#include <QByteArray>
#include <QString>

struct FontFile
{
    QString fileName;
    int indexValue;
};

class QBasicFontDatabase : public QPlatformFontDatabase
{
public:
    void populateFontDatabase() override;
    QFontEngine *fontEngine(const QFontDef &fontDef, void *handle) override;
    QFontEngine *fontEngine(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference) override;
    QStringList addApplicationFont(const QByteArray &fontData, const QString &fileName) override;
    void releaseHandle(void *handle) override;

    static QStringList addTTFile(const QByteArray &fontData, const QByteArray &file);
};

#endif
