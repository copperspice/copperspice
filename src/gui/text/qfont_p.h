/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QFONT_P_H
#define QFONT_P_H

#include <QtGui/qfont.h>
#include <QtCore/qmap.h>
#include <QtCore/qobject.h>
#include <qunicodetables_p.h>
#include <QtGui/qfontdatabase.h>
#include <qfixed_p.h>

QT_BEGIN_NAMESPACE

class QFontCache;
class QFontEngine;

struct QFontDef {
   inline QFontDef()
      : pointSize(-1.0), pixelSize(-1),
        styleStrategy(QFont::PreferDefault), styleHint(QFont::AnyStyle),
        weight(50), fixedPitch(false), style(QFont::StyleNormal), stretch(100),
        ignorePitch(true), hintingPreference(QFont::PreferDefaultHinting)
#ifdef Q_OS_MAC
        , fixedPitchComputed(false)
#endif
   {
   }

   QString family;
   QString styleName;

#ifdef Q_WS_X11
   QString addStyle;
#endif

   qreal pointSize;
   qreal pixelSize;

   uint styleStrategy : 16;
   uint styleHint     : 8;

   uint weight     :  7; // 0-99
   uint fixedPitch :  1;
   uint style      :  2;
   uint stretch    : 12; // 0-400

   uint ignorePitch : 1;
   uint hintingPreference : 2;
   uint fixedPitchComputed : 1; // for Mac OS X only
   int reserved   : 14; // for future extensions

   bool exactMatch(const QFontDef &other) const;
   bool operator==(const QFontDef &other) const {
      return pixelSize == other.pixelSize
             && weight == other.weight
             && style == other.style
             && stretch == other.stretch
             && styleHint == other.styleHint
             && styleStrategy == other.styleStrategy
             && ignorePitch == other.ignorePitch && fixedPitch == other.fixedPitch
             && family == other.family
             && (styleName.isEmpty() || other.styleName.isEmpty() || styleName == other.styleName)
             && hintingPreference == other.hintingPreference
#ifdef Q_WS_X11
             && addStyle == other.addStyle
#endif
             ;
   }
   inline bool operator<(const QFontDef &other) const {
      if (pixelSize != other.pixelSize) {
         return pixelSize < other.pixelSize;
      }
      if (weight != other.weight) {
         return weight < other.weight;
      }
      if (style != other.style) {
         return style < other.style;
      }
      if (stretch != other.stretch) {
         return stretch < other.stretch;
      }
      if (styleHint != other.styleHint) {
         return styleHint < other.styleHint;
      }
      if (styleStrategy != other.styleStrategy) {
         return styleStrategy < other.styleStrategy;
      }
      if (family != other.family) {
         return family < other.family;
      }
      if (!styleName.isEmpty() && !other.styleName.isEmpty() && styleName != other.styleName) {
         return styleName < other.styleName;
      }
      if (hintingPreference != other.hintingPreference) {
         return hintingPreference < other.hintingPreference;
      }

#ifdef Q_WS_X11
      if (addStyle != other.addStyle) {
         return addStyle < other.addStyle;
      }
#endif

      if (ignorePitch != other.ignorePitch) {
         return ignorePitch < other.ignorePitch;
      }
      if (fixedPitch != other.fixedPitch) {
         return fixedPitch < other.fixedPitch;
      }
      return false;
   }
};

class QFontEngineData
{
 public:
   QFontEngineData();
   ~QFontEngineData();

   QAtomicInt ref;
   QFontCache *fontCache;

#if ! defined(Q_OS_MAC)
   QFontEngine *engines[QChar::ScriptCount];
#else
   QFontEngine *engine;
#endif
};


class Q_GUI_EXPORT QFontPrivate
{
 public:
#ifdef Q_WS_X11
   static int defaultEncodingID;
#endif

   QFontPrivate();
   QFontPrivate(const QFontPrivate &other);
   ~QFontPrivate();

   QFontEngine *engineForScript(int script) const;
   void alterCharForCapitalization(QChar &c) const;

   QAtomicInt ref;
   QFontDef request;
   mutable QFontEngineData *engineData;
   int dpi;
   int screen;

#ifdef Q_OS_WIN
   HDC hdc;
#endif

   uint rawMode    :  1;
   uint underline  :  1;
   uint overline   :  1;
   uint strikeOut  :  1;
   uint kerning    :  1;
   uint capital    :  3;
   bool letterSpacingIsAbsolute : 1;

   QFixed letterSpacing;
   QFixed wordSpacing;

   mutable QFontPrivate *scFont;
   QFont smallCapsFont() const {
      return QFont(smallCapsFontPrivate());
   }
   QFontPrivate *smallCapsFontPrivate() const;

   static QFontPrivate *get(const QFont &font) {
      return font.d.data();
   }

   void resolve(uint mask, const QFontPrivate *other);

 private:
   QFontPrivate &operator=(const QFontPrivate &) {
      return *this;
   }
};


class QFontCache : public QObject
{
   GUI_CS_OBJECT(QFontCache)

 public:
   // note: these static functions work on a per-thread basis
   static QFontCache *instance();
   static void cleanup();

   QFontCache();
   ~QFontCache();

   void clear();

#if defined(Q_WS_QWS) && !defined(QT_NO_QWS_QPF2)
   void removeEngineForFont(const QByteArray &fontName);
#endif

   // universal key structure.  QFontEngineDatas and QFontEngines are cached using
   // the same keys
   struct Key {
      Key() : script(0), screen(0) { }
      Key(const QFontDef &d, int c, int s = 0)
         : def(d), script(c), screen(s) { }

      QFontDef def;
      int script;
      int screen;

      inline bool operator<(const Key &other) const {
         if (script != other.script) {
            return script < other.script;
         }
         if (screen != other.screen) {
            return screen < other.screen;
         }
         return def < other.def;
      }
      inline bool operator==(const Key &other) const {
         return def == other.def && script == other.script && screen == other.screen;
      }
   };

   // QFontEngineData cache
   typedef QMap<Key, QFontEngineData *> EngineDataCache;
   EngineDataCache engineDataCache;

   QFontEngineData *findEngineData(const Key &key) const;
   void insertEngineData(const Key &key, QFontEngineData *engineData);

   // QFontEngine cache
   struct Engine {
      Engine() : data(0), timestamp(0), hits(0) { }
      Engine(QFontEngine *d) : data(d), timestamp(0), hits(0) { }

      QFontEngine *data;
      uint timestamp;
      uint hits;
   };

   typedef QMap<Key, Engine> EngineCache;
   EngineCache engineCache;

   QFontEngine *findEngine(const Key &key);
   void insertEngine(const Key &key, QFontEngine *engine);

#if defined(Q_OS_WIN) || defined(Q_WS_QWS)
   void cleanupPrinterFonts();
#endif

 private:
   void increaseCost(uint cost);
   void decreaseCost(uint cost);
   void timerEvent(QTimerEvent *event) override;

   static const uint min_cost;
   uint total_cost, max_cost;
   uint current_timestamp;
   bool fast;
   int timer_id;
};

Q_GUI_EXPORT int qt_defaultDpiX();
Q_GUI_EXPORT int qt_defaultDpiY();
Q_GUI_EXPORT int qt_defaultDpi();

QT_END_NAMESPACE

#endif // QFONT_P_H
