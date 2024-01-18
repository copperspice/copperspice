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

#ifndef QFONTDATABASE_H
#define QFONTDATABASE_H

#include <qcontainerfwd.h>
#include <qfont.h>
#include <qstring.h>
#include <qwindowdefs.h>

class QStringList;
class QFontEngine;
class QFontDatabasePrivate;

struct QFontDef;

class Q_GUI_EXPORT QFontDatabase
{
   GUI_CS_GADGET(QFontDatabase)

   GUI_CS_ENUM(WritingSystem)
   GUI_CS_ENUM(SystemFont)

 public:
   // do not re-order or delete entries from this enum without updating the QPF2 format and makeqpf

   GUI_CS_REGISTER_ENUM(
      enum WritingSystem {
         Any,
         Latin,
         Greek,
         Cyrillic,
         Armenian,
         Hebrew,
         Arabic,
         Syriac,
         Thaana,
         Devanagari,
         Bengali,
         Gurmukhi,
         Gujarati,
         Oriya,
         Tamil,
         Telugu,
         Kannada,
         Malayalam,
         Sinhala,
         Thai,
         Lao,
         Tibetan,
         Myanmar,
         Georgian,
         Khmer,
         SimplifiedChinese,
         TraditionalChinese,
         Japanese,
         Korean,
         Vietnamese,

         Symbol,
         Other = Symbol,

         Ogham,
         Runic,
         Nko,

         WritingSystemsCount
      };
   )

   enum SystemFont {
      GeneralFont,
      FixedFont,
      TitleFont,
      SmallestReadableFont
   };

   static QList<int> standardSizes();

   QFontDatabase();

   QList<WritingSystem> writingSystems() const;
   QList<WritingSystem> writingSystems(const QString &family) const;

   QStringList families(WritingSystem writingSystem = Any) const;
   QStringList styles(const QString &family) const;
   QList<int> pointSizes(const QString &family, const QString &style = QString());
   QList<int> smoothSizes(const QString &family, const QString &style);
   QString styleString(const QFont &font) const;
   QString styleString(const QFontInfo &fontInfo) const;

   QFont font(const QString &family, const QString &style, int pointSize) const;

   bool isBitmapScalable(const QString &family, const QString &style = QString()) const;
   bool isSmoothlyScalable(const QString &family, const QString &style = QString()) const;
   bool isScalable(const QString &family, const QString &style = QString()) const;
   bool isFixedPitch(const QString &family, const QString &style = QString()) const;

   bool italic(const QString &family, const QString &style) const;
   bool bold(const QString &family, const QString &style) const;
   int weight(const QString &family, const QString &style) const;

   bool hasFamily(const QString &family) const;
   bool isPrivateFamily(const QString &family) const;

   static QString writingSystemName(WritingSystem writingSystem);
   static QString writingSystemSample(WritingSystem writingSystem);

   static int addApplicationFont(const QString &fileName);
   static int addApplicationFontFromData(const QByteArray &fontData);
   static QStringList applicationFontFamilies(int id);
   static bool removeApplicationFont(int id);
   static bool removeAllApplicationFonts();

   static QFont systemFont(SystemFont type);

 private:
   QFontDatabasePrivate *m_fontdatabase;

   static void createDatabase();
   static void parseFontName(const QString &name, QString &foundry, QString &family);
   static QString resolveFontFamilyAlias(const QString &family);
   static QFontEngine *findFont(const QFontDef &request, int script);
   static void load(const QFontPrivate *d, int script);

   friend struct QFontDef;
   friend class QFontPrivate;
   friend class QFontDialog;
   friend class QFontDialogPrivate;
   friend class QFontEngineMulti;
};

#endif
