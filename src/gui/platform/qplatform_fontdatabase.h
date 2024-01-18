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

#ifndef QPLATFORM_FONTDATABASE_H
#define QPLATFORM_FONTDATABASE_H

#include <qfontdatabase.h>

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include <qfontengine_p.h>
#include <qfont_p.h>

class QWritingSystemsPrivate;
class QFontRequestPrivate;
class QFontEngineMulti;

class Q_GUI_EXPORT QSupportedWritingSystems
{
 public:
   QSupportedWritingSystems();
   QSupportedWritingSystems(const QSupportedWritingSystems &other);
   QSupportedWritingSystems &operator=(const QSupportedWritingSystems &other);
   ~QSupportedWritingSystems();

   void setSupported(QFontDatabase::WritingSystem writingSystem, bool supported = true);
   bool supported(QFontDatabase::WritingSystem writingSystem) const;

 private:
   void detach();

   QWritingSystemsPrivate *d;

   friend Q_GUI_EXPORT bool operator==(const QSupportedWritingSystems &, const QSupportedWritingSystems &);
   friend Q_GUI_EXPORT bool operator!=(const QSupportedWritingSystems &, const QSupportedWritingSystems &);
};

Q_GUI_EXPORT bool operator==(const QSupportedWritingSystems &, const QSupportedWritingSystems &);
Q_GUI_EXPORT bool operator!=(const QSupportedWritingSystems &, const QSupportedWritingSystems &);

class Q_GUI_EXPORT QPlatformFontDatabase
{
 public:
   virtual ~QPlatformFontDatabase();
   virtual void populateFontDatabase();
   virtual void populateFamily(const QString &familyName);
   virtual void invalidate();

   virtual QFontEngineMulti *fontEngineMulti(QFontEngine *fontEngine, QChar::Script script);

   virtual QFontEngine *fontEngine(const QFontDef &fontDef, void *handle);

   virtual QStringList fallbacksForFamily(const QString &family, QFont::Style style, QFont::StyleHint styleHint,
      QChar::Script script) const;
   virtual QStringList addApplicationFont(const QByteArray &fontData, const QString &fileName);
   virtual void releaseHandle(void *handle);

   virtual QFontEngine *fontEngine(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference);
   virtual QString fontDir() const;
   virtual QFont defaultFont() const;
   virtual bool isPrivateFontFamily(const QString &family) const;

   virtual QString resolveFontFamilyAlias(const QString &family) const;
   virtual bool fontsAlwaysScalable() const;
   virtual QList<int> standardSizes() const;
   QFontEngine::SubpixelAntialiasingType subpixelAntialiasingTypeHint() const;

   // helper
   static QSupportedWritingSystems writingSystemsFromTrueTypeBits(quint32 unicodeRange[4], quint32 codePageRange[2]);
   static QFont::Weight weightFromInteger(int weight);

   //callback
   static void registerQPF2Font(const QByteArray &dataArray, void *handle);
   static void registerFont(const QString &familyName, const QString &styleName, const QString &foundryName,
            QFont::Weight weight, QFont::Style style, QFont::Stretch stretch, bool antialiased, bool scalable,
            int pixelSize, bool fixedPitch, const QSupportedWritingSystems &writingSystems, void *handle);

   static void registerFontFamily(const QString &familyName);
   static void registerAliasToFontFamily(const QString &familyName, const QString &alias);
};

#endif
