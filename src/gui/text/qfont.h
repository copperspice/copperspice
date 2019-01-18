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

#ifndef QFONT_H
#define QFONT_H

#include <qwindowdefs.h>
#include <qstring.h>
#include <qsharedpointer.h>

class QFontPrivate;
class QStringList;
class QVariant;

class Q_GUI_EXPORT QFont
{
   GUI_CS_GADGET(QFont)

   GUI_CS_ENUM(StyleStrategy)

 public:
   enum StyleHint {
      Helvetica,  SansSerif = Helvetica,
      Times,      Serif = Times,
      Courier,    TypeWriter = Courier,
      OldEnglish, Decorative = OldEnglish,
      System,
      AnyStyle,
      Cursive,
      Monospace,
      Fantasy
   };

   enum StyleStrategy {
      PreferDefault       = 0x0001,
      PreferBitmap        = 0x0002,
      PreferDevice        = 0x0004,
      PreferOutline       = 0x0008,
      ForceOutline        = 0x0010,
      PreferMatch         = 0x0020,
      PreferQuality       = 0x0040,
      PreferAntialias     = 0x0080,
      NoAntialias         = 0x0100,
      OpenGLCompatible    = 0x0200,
      ForceIntegerMetrics = 0x0400,
      NoSubpixelAntialias = 0x0800,
      NoFontMerging       = 0x8000
   };

   enum HintingPreference {
      PreferDefaultHinting        = 0,
      PreferNoHinting             = 1,
      PreferVerticalHinting       = 2,
      PreferFullHinting           = 3
   };

   enum Weight {
      Thin       = 0,    // 100
      ExtraLight = 12,   // 200
      Light      = 25,   // 300
      Normal     = 50,   // 400
      Medium     = 57,   // 500
      DemiBold   = 63,   // 600
      Bold       = 75,   // 700
      ExtraBold  = 81,   // 800
      Black      = 87    // 900
   };

   enum Style {
      StyleNormal,
      StyleItalic,
      StyleOblique
   };

   enum Stretch {
      UltraCondensed =  50,
      ExtraCondensed =  62,
      Condensed      =  75,
      SemiCondensed  =  87,
      Unstretched    = 100,
      SemiExpanded   = 112,
      Expanded       = 125,
      ExtraExpanded  = 150,
      UltraExpanded  = 200
   };

   enum Capitalization {
      MixedCase,
      AllUppercase,
      AllLowercase,
      SmallCaps,
      Capitalize
   };

   enum SpacingType {
      PercentageSpacing,
      AbsoluteSpacing
   };

   enum ResolveProperties {
      FamilyResolved              = 0x0001,
      SizeResolved                = 0x0002,
      StyleHintResolved           = 0x0004,
      StyleStrategyResolved       = 0x0008,
      WeightResolved              = 0x0010,
      StyleResolved               = 0x0020,
      UnderlineResolved           = 0x0040,
      OverlineResolved            = 0x0080,
      StrikeOutResolved           = 0x0100,
      FixedPitchResolved          = 0x0200,
      StretchResolved             = 0x0400,
      KerningResolved             = 0x0800,
      CapitalizationResolved      = 0x1000,
      LetterSpacingResolved       = 0x2000,
      WordSpacingResolved         = 0x4000,
      HintingPreferenceResolved   = 0x8000,
      StyleNameResolved           = 0x10000,
      AllPropertiesResolved       = 0x1ffff
   };

   QFont();
   QFont(const QString &family, int pointSize = -1, int weight = -1, bool italic = false);
   QFont(const QFont &, QPaintDevice *pd);
   QFont(const QFont &);
   ~QFont();

   void swap(QFont &other) {
      qSwap(d, other.d);
      qSwap(resolve_mask, other.resolve_mask);
   }

   QString family() const;
   void setFamily(const QString &);

   QString styleName() const;
   void setStyleName(const QString &);

   int pointSize() const;
   void setPointSize(int);
   qreal pointSizeF() const;
   void setPointSizeF(qreal);

   int pixelSize() const;
   void setPixelSize(int);

   int weight() const;
   void setWeight(int);

   inline bool bold() const;
   inline void setBold(bool);

   void setStyle(Style style);
   Style style() const;

   inline bool italic() const;
   inline void setItalic(bool b);

   bool underline() const;
   void setUnderline(bool);

   bool overline() const;
   void setOverline(bool);

   bool strikeOut() const;
   void setStrikeOut(bool);

   bool fixedPitch() const;
   void setFixedPitch(bool);

   bool kerning() const;
   void setKerning(bool);

   StyleHint styleHint() const;
   StyleStrategy styleStrategy() const;
   void setStyleHint(StyleHint, StyleStrategy = PreferDefault);
   void setStyleStrategy(StyleStrategy s);

   int stretch() const;
   void setStretch(int);

   qreal letterSpacing() const;
   SpacingType letterSpacingType() const;
   void setLetterSpacing(SpacingType type, qreal spacing);

   qreal wordSpacing() const;
   void setWordSpacing(qreal spacing);

   void setCapitalization(Capitalization);
   Capitalization capitalization() const;

   void setHintingPreference(HintingPreference hintingPreference);
   HintingPreference hintingPreference() const;


   // dupicated from QFontInfo
   bool exactMatch() const;

   QFont &operator=(const QFont &);
   bool operator==(const QFont &) const;
   bool operator!=(const QFont &) const;
   bool operator<(const QFont &) const;
   operator QVariant() const;
   bool isCopyOf(const QFont &font) const;

   inline QFont &operator=(QFont &&other) {
      qSwap(d, other.d);
      qSwap(resolve_mask, other.resolve_mask);
      return *this;
   }


   QString key() const;

   QString toString() const;
   bool fromString(const QString &);

   static QString substitute(const QString &);
   static QStringList substitutes(const QString &);
   static QStringList substitutions();
   static void insertSubstitution(const QString &, const QString &);
   static void insertSubstitutions(const QString &, const QStringList &);
   static void removeSubstitutions(const QString &);
   static void initialize();
   static void cleanup();

   static void cacheStatistics();

   QString defaultFamily() const;
   QString lastResortFamily() const;
   QString lastResortFont() const;

   QFont resolve(const QFont &other) const;
   uint resolve() const {
      return resolve_mask;
   }

   void resolve(uint mask) {
      resolve_mask = mask;
   }

 private:
   explicit QFont(QFontPrivate *);

   void detach();


   friend class QFontPrivate;
   friend class QFontDialogPrivate;
   friend class QFontMetrics;
   friend class QFontMetricsF;
   friend class QFontInfo;
   friend class QPainter;
   friend class QPainterPrivate;

   friend class QApplication;
   friend class QWidget;
   friend class QWidgetPrivate;
   friend class QTextLayout;
   friend class QTextEngine;
   friend class QStackTextEngine;
   friend class QTextLine;
   friend struct QScriptLine;
   friend class QOpenGLContext;
   friend class QWin32PaintEngine;
   friend class QAlphaPaintEngine;
   friend class QPainterPath;
   friend class QTextItemInt;
   friend class QPicturePaintEngine;
   friend class QPainterReplayer;
   friend class QPaintBufferEngine;
   friend class QCommandLinkButtonPrivate;
   friend class QFontEngine;

   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QFont &);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QFont &);

   QExplicitlySharedDataPointer<QFontPrivate> d;
   uint resolve_mask;
};

Q_GUI_EXPORT uint qHash(const QFont &font, uint seed = 0);
inline bool QFont::bold() const
{
   return weight() > Normal;
}

inline void QFont::setBold(bool enable)
{
   setWeight(enable ? Bold : Normal);
}

inline bool QFont::italic() const
{
   return (style() != StyleNormal);
}

inline void QFont::setItalic(bool b)
{
   setStyle(b ? StyleItalic : StyleNormal);
}

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QFont &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QFont &);

Q_GUI_EXPORT QDebug operator<<(QDebug, const QFont &);

#endif
