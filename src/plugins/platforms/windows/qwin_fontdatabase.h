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

#ifndef QWINDOWSFONTDATABASE_H
#define QWINDOWSFONTDATABASE_H

#include <qplatform_fontdatabase.h>
#include <qsharedpointer.h>
#include <qwin_additional.h>

class QWindowsFontEngineData
{
 public:
   QWindowsFontEngineData();

   QWindowsFontEngineData(const QWindowsFontEngineData &) = delete;
   QWindowsFontEngineData &operator=(const QWindowsFontEngineData &) = delete;

   ~QWindowsFontEngineData();

   uint pow_gamma[256];

   bool clearTypeEnabled;
   qreal fontSmoothingGamma;
   HDC hdc;
};

class QWindowsFontDatabase : public QPlatformFontDatabase
{
 public:
   QWindowsFontDatabase();
   ~QWindowsFontDatabase();

   void populateFontDatabase() override;
   void populateFamily(const QString &familyName) override;

   // emerald (multi)
   QFontEngineMulti *fontEngineMulti(QFontEngine *fontEngine, QChar::Script script) override;

   QFontEngine *fontEngine(const QFontDef &fontDef, void *handle) override;
   QFontEngine *fontEngine(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference) override;
   QStringList fallbacksForFamily(const QString &family, QFont::Style style, QFont::StyleHint styleHint,
      QChar::Script script) const override;
   QStringList addApplicationFont(const QByteArray &fontData, const QString &fileName) override;
   void releaseHandle(void *handle) override;
   QString fontDir() const override;

   QFont defaultFont() const  override {
      return systemDefaultFont();
   }

   bool fontsAlwaysScalable() const override;
   void derefUniqueFont(const QString &uniqueFont);
   void refUniqueFont(const QString &uniqueFont);

   static QFont systemDefaultFont();

   static QFontEngine *createEngine(const QFontDef &request, int dpi, const QSharedPointer<QWindowsFontEngineData> &data);

   static HFONT systemFont();
   static QFont LOGFONT_to_QFont(const LOGFONT &lf, int verticalDPI = 0);

   static qreal fontSmoothingGamma();
   static LOGFONT fontDefToLOGFONT(const QFontDef &fontDef);

   static QStringList extraTryFontsForFamily(const QString &family);
   static QString familyForStyleHint(QFont::StyleHint styleHint);

 private:
   void populateFamily(const QString &familyName, bool registerAlias);
   void removeApplicationFonts();

   struct WinApplicationFont {
      HANDLE handle;
      QString fileName;
   };

   QList<WinApplicationFont> m_applicationFonts;

   struct UniqueFontData {
      HANDLE handle;
      QAtomicInt refCount;
   };

   QMap<QString, UniqueFontData> m_uniqueFontData;
};

QDebug operator<<(QDebug debug, const QFontDef &def);

#endif
