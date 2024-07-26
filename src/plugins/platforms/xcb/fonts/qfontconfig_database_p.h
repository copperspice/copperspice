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

#ifndef QFONTCONFIG_DATABASE_P_H
#define QFONTCONFIG_DATABASE_P_H

#include <qplatform_fontdatabase.h>

#include <qbasicfontdatabase_p.h>

class QFontEngineFT;

class QFontconfigDatabase : public QBasicFontDatabase
{
 public:
    void populateFontDatabase() override;
    QFontEngineMulti *fontEngineMulti(QFontEngine *fontEngine, QChar::Script script) override;
    QFontEngine *fontEngine(const QFontDef &fontDef, void *handle) override;
    QFontEngine *fontEngine(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference) override;

    QStringList fallbacksForFamily(const QString &family, QFont::Style style, QFont::StyleHint styleHint,
            QChar::Script script) const override;

    QStringList addApplicationFont(const QByteArray &fontData, const QString &fileName) override;
    QString resolveFontFamilyAlias(const QString &family) const override;
    QFont defaultFont() const override;

 private:
    void setupFontEngine(QFontEngineFT *engine, const QFontDef &fontDef) const;
};

#endif
