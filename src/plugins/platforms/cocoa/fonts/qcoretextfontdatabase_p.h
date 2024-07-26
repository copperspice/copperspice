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

#ifndef QCORETEXTFONTDATABASE_H
#define QCORETEXTFONTDATABASE_H

#include <qglobal.h>
#include <qplatform_fontdatabase.h>
#include <qplatform_theme.h>

#include <qcore_mac_p.h>

#include <ApplicationServices/ApplicationServices.h>

class QCoreTextFontDatabase : public QPlatformFontDatabase
{
 public:
   QCoreTextFontDatabase(bool useFreeType = false);
   ~QCoreTextFontDatabase();

   void populateFontDatabase() override;
   void populateFamily(const QString &familyName) override;

   QFontEngine *fontEngine(const QFontDef &fontDef, void *handle) override;
   QFontEngine *fontEngine(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference) override;
   QStringList fallbacksForFamily(const QString &family, QFont::Style style, QFont::StyleHint styleHint,
      QChar::Script script) const override;
   QStringList addApplicationFont(const QByteArray &fontData, const QString &fileName) override;

   void releaseHandle(void *handle) override;
   bool isPrivateFontFamily(const QString &family) const override;
   QFont defaultFont() const override;
   bool fontsAlwaysScalable() const override;
   QList<int> standardSizes() const override;

   // For iOS and OS X platform themes
   QFont *themeFont(QPlatformTheme::Font) const;
   const QHash<QPlatformTheme::Font, QFont *> &themeFonts() const;

 private:
   void populateFromDescriptor(CTFontDescriptorRef font);

#if defined(QT_USE_FREETYPE)
   bool m_useFreeType;
   QFontEngine *freeTypeFontEngine(const QFontDef &fontDef, const QString &filename, const QByteArray &fontData = QByteArray());
#endif

   mutable QString defaultFontName;

   void removeApplicationFonts();

   QVector<QVariant> m_applicationFonts;
   mutable QSet<CTFontDescriptorRef> m_systemFontDescriptors;
   mutable QHash<QPlatformTheme::Font, QFont *> m_themeFonts;
};

#endif
