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

#ifndef QFONTDATABASE_H
#define QFONTDATABASE_H

#include <QtGui/qwindowdefs.h>
#include <QtCore/qstring.h>
#include <QtGui/qfont.h>

QT_BEGIN_NAMESPACE

class QStringList;
class QFontEngine;
class QFontDatabasePrivate;

template <class T> class QList;
struct QFontDef;

class Q_GUI_EXPORT QFontDatabase
{
   GUI_CS_GADGET(QFontDatabase)

   GUI_CS_ENUM(WritingSystem)

 public:
   // do not re-order or delete entries from this enum without updating the QPF2 format and makeqpf!!
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

   static QList<int> standardSizes();

   QFontDatabase();

   QList<WritingSystem> writingSystems() const;
   QList<WritingSystem> writingSystems(const QString &family) const;

   QStringList families(WritingSystem writingSystem = Any) const;
   QStringList styles(const QString &family) const;
   QList<int> pointSizes(const QString &family, const QString &style = QString());
   QList<int> smoothSizes(const QString &family, const QString &style);
   QString styleString(const QFont &font);
   QString styleString(const QFontInfo &fontInfo);

   QFont font(const QString &family, const QString &style, int pointSize) const;

   bool isBitmapScalable(const QString &family, const QString &style = QString()) const;
   bool isSmoothlyScalable(const QString &family, const QString &style = QString()) const;
   bool isScalable(const QString &family, const QString &style = QString()) const;
   bool isFixedPitch(const QString &family, const QString &style = QString()) const;

   bool italic(const QString &family, const QString &style) const;
   bool bold(const QString &family, const QString &style) const;
   int weight(const QString &family, const QString &style) const;

   bool hasFamily(const QString &family) const;

   static QString writingSystemName(WritingSystem writingSystem);
   static QString writingSystemSample(WritingSystem writingSystem);

   static int addApplicationFont(const QString &fileName);
   static int addApplicationFontFromData(const QByteArray &fontData);
   static QStringList applicationFontFamilies(int id);
   static bool removeApplicationFont(int id);
   static bool removeAllApplicationFonts();

   static bool supportsThreadedFontRendering();

 private:
   static void createDatabase();
   static void parseFontName(const QString &name, QString &foundry, QString &family);
   static QString resolveFontFamilyAlias(const QString &family);

#if defined(Q_WS_QWS) || defined(Q_WS_QPA)
   static QFontEngine *findFont(int script, const QFontPrivate *fp, const QFontDef &request);
#endif
   static void load(const QFontPrivate *d, int script);
#ifdef Q_WS_X11
   static QFontEngine *loadXlfd(int screen, int script, const QFontDef &request, int force_encoding_id = -1);
#endif

   friend struct QFontDef;
   friend class QFontPrivate;
   friend class QFontDialog;
   friend class QFontDialogPrivate;
   friend class QFontEngineMultiXLFD;
   friend class QFontEngineMultiQWS;
   friend class QFontEngineMultiS60;
   friend class QFontEngineMultiQPA;

   QFontDatabasePrivate *d;
};

QT_END_NAMESPACE

#endif // QFONTDATABASE_H
