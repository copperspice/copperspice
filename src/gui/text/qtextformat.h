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

#ifndef QTEXTFORMAT_H
#define QTEXTFORMAT_H

#include <qcolor.h>
#include <qfont.h>
#include <qshareddata.h>
#include <qpen.h>
#include <qbrush.h>
#include <qtextoption.h>
#include <qstringfwd.h>
#include <qvector.h>
#include <qvariant.h>

class QTextCursor;
class QTextFormat;
class QTextFormatCollection;
class QTextFrameFormat;
class QTextFormatPrivate;
class QTextBlockFormat;
class QTextCharFormat;
class QTextListFormat;
class QTextTableFormat;

class QTextImageFormat;
class QTextTableCellFormat;
class QTextObject;
class QTextDocument;
class QTextLength;

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QTextLength &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QTextLength &);

Q_GUI_EXPORT QDebug operator<<(QDebug, const QTextLength &);

class Q_GUI_EXPORT QTextLength
{
 public:
   enum Type { VariableLength = 0, FixedLength, PercentageLength };

   QTextLength()
      : lengthType(VariableLength), fixedValueOrPercentage(0)
   {
   }

   explicit QTextLength(Type type, qreal value);

   Type type() const {
      return lengthType;
   }

   qreal value(qreal maximumLength) const {
      switch (lengthType) {
         case FixedLength:
            return fixedValueOrPercentage;

         case VariableLength:
            return maximumLength;

         case PercentageLength:
            return fixedValueOrPercentage * maximumLength / qreal(100);
      }

      return -1;
   }

   qreal rawValue() const {
      return fixedValueOrPercentage;
   }

   bool operator==(const QTextLength &other) const {
      return lengthType == other.lengthType && qFuzzyCompare(fixedValueOrPercentage, other.fixedValueOrPercentage);
   }

   bool operator!=(const QTextLength &other) const {
      return lengthType != other.lengthType
         || !qFuzzyCompare(fixedValueOrPercentage, other.fixedValueOrPercentage);
   }

   operator QVariant() const;

 private:
   Type lengthType;
   qreal fixedValueOrPercentage;

   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QTextLength &);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QTextLength &);
};

inline QTextLength::QTextLength(Type type, qreal value)
   : lengthType(type), fixedValueOrPercentage(value)
{
}

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QTextFormat &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QTextFormat &);

Q_GUI_EXPORT QDebug operator<<(QDebug, const QTextFormat &);

class Q_GUI_EXPORT QTextFormat
{
   GUI_CS_GADGET(QTextFormat)

   GUI_CS_ENUM(FormatType)
   GUI_CS_ENUM(Property)
   GUI_CS_ENUM(ObjectTypes)

 public:
   enum FormatType {
      InvalidFormat = -1,
      BlockFormat   = 1,
      CharFormat    = 2,
      ListFormat    = 3,
      TableFormat   = 4,
      FrameFormat   = 5,

      UserFormat    = 100
   };

   enum Property {
      ObjectIndex            = 0x0,

      // paragraph and char
      CssFloat               = 0x0800,
      LayoutDirection        = 0x0801,

      OutlinePen             = 0x810,
      BackgroundBrush        = 0x820,
      ForegroundBrush        = 0x821,

      // used in qtextlayout.cpp
      // ObjectSelectionBrush = 0x822,

      BackgroundImageUrl     = 0x823,

      // paragraph
      BlockAlignment         = 0x1010,
      BlockTopMargin         = 0x1030,
      BlockBottomMargin      = 0x1031,
      BlockLeftMargin        = 0x1032,
      BlockRightMargin       = 0x1033,
      TextIndent             = 0x1034,
      TabPositions           = 0x1035,
      BlockIndent            = 0x1040,
      LineHeight             = 0x1048,
      LineHeightType         = 0x1049,
      BlockNonBreakableLines = 0x1050,
      BlockTrailingHorizontalRulerWidth = 0x1060,

      // character properties
      FirstFontProperty      = 0x1FE0,
      FontCapitalization     = FirstFontProperty,
      FontLetterSpacingType  = 0x2033,
      FontLetterSpacing      = 0x1FE1,
      FontWordSpacing        = 0x1FE2,
      FontStretch            = 0x2034,
      FontStyleHint          = 0x1FE3,
      FontStyleStrategy      = 0x1FE4,
      FontKerning            = 0x1FE5,
      FontHintingPreference  = 0x1FE6,
      FontFamily             = 0x2000,
      FontPointSize          = 0x2001,
      FontSizeAdjustment     = 0x2002,
      FontSizeIncrement      = FontSizeAdjustment,    // old name, compat
      FontWeight             = 0x2003,
      FontItalic             = 0x2004,
      FontUnderline          = 0x2005,                // deprecated, use TextUnderlineStyle instead
      FontOverline           = 0x2006,
      FontStrikeOut          = 0x2007,
      FontFixedPitch         = 0x2008,
      FontPixelSize          = 0x2009,
      LastFontProperty       = FontPixelSize,

      TextUnderlineColor     = 0x2010,
      TextVerticalAlignment  = 0x2021,
      TextOutline            = 0x2022,
      TextUnderlineStyle     = 0x2023,
      TextToolTip            = 0x2024,

      IsAnchor               = 0x2030,
      AnchorHref             = 0x2031,
      AnchorName             = 0x2032,
      ObjectType             = 0x2f00,

      // list properties
      ListStyle              = 0x3000,
      ListIndent             = 0x3001,
      ListNumberPrefix       = 0x3002,
      ListNumberSuffix       = 0x3003,

      // table and frame properties
      FrameBorder            = 0x4000,
      FrameMargin            = 0x4001,
      FramePadding           = 0x4002,
      FrameWidth             = 0x4003,
      FrameHeight            = 0x4004,
      FrameTopMargin         = 0x4005,
      FrameBottomMargin      = 0x4006,
      FrameLeftMargin        = 0x4007,
      FrameRightMargin       = 0x4008,
      FrameBorderBrush       = 0x4009,
      FrameBorderStyle       = 0x4010,

      TableColumns           = 0x4100,
      TableColumnWidthConstraints = 0x4101,
      TableCellSpacing       = 0x4102,
      TableCellPadding       = 0x4103,
      TableHeaderRowCount    = 0x4104,

      // table cell properties
      TableCellRowSpan       = 0x4810,
      TableCellColumnSpan    = 0x4811,

      TableCellTopPadding    = 0x4812,
      TableCellBottomPadding = 0x4813,
      TableCellLeftPadding   = 0x4814,
      TableCellRightPadding  = 0x4815,

      // image properties
      ImageName              = 0x5000,
      ImageWidth             = 0x5010,
      ImageHeight            = 0x5011,

      // unused
      /*
         SuppressText        = 0x5012,
         SuppressBackground  = 0x513,
      */

      // selection properties
      FullWidthSelection     = 0x06000,

      // page break properties
      PageBreakPolicy        = 0x7000,

      //
      UserProperty           = 0x100000
   };

   enum ObjectTypes {
      NoObject,
      ImageObject,
      TableObject,
      TableCellObject,

      UserObject = 0x1000
   };

   enum PageBreakFlag {
      PageBreak_Auto            = 0,
      PageBreak_AlwaysBefore    = 0x001,
      PageBreak_AlwaysAfter     = 0x010
      // PageBreak_AlwaysInside = 0x100
   };
   using PageBreakFlags = QFlags<PageBreakFlag>;

   QTextFormat();

   explicit QTextFormat(int type);

   QTextFormat(const QTextFormat &other);
   QTextFormat &operator=(const QTextFormat &other);
   ~QTextFormat();

   void merge(const QTextFormat &other);

   bool isValid() const {
      return type() != InvalidFormat;
   }

   bool isEmpty() const {
      return propertyCount() == 0;
   }

   int type() const;

   int objectIndex() const;
   void setObjectIndex(int index);

   void clearProperty(int propertyId);
   bool hasProperty(int propertyId) const;

   QVariant property(int propertyId) const;
   void setProperty(int propertyId, const QVariant &value);
   void setProperty(int propertyId, const QVector<QTextLength> &value);

   bool boolProperty(int propertyId) const;
   int intProperty(int propertyId) const;
   qreal doubleProperty(int propertyId) const;
   QString stringProperty(int propertyId) const;
   QColor colorProperty(int propertyId) const;
   QPen penProperty(int propertyId) const;
   QBrush brushProperty(int propertyId) const;

   QTextLength lengthProperty(int propertyId) const;
   QVector<QTextLength> lengthVectorProperty(int propertyId) const;

   QMap<int, QVariant> properties() const;
   int propertyCount() const;

   void swap(QTextFormat &other) {
      qSwap(d, other.d);
      qSwap(format_type, other.format_type);
   }

   inline void setObjectType(int type);

   int objectType() const {
      return intProperty(ObjectType);
   }

   bool isCharFormat() const {
      return type() == CharFormat;
   }

   bool isBlockFormat() const {
      return type() == BlockFormat;
   }

   bool isListFormat() const {
      return type() == ListFormat;
   }

   bool isFrameFormat() const {
      return type() == FrameFormat;
   }

   bool isImageFormat() const {
      return type() == CharFormat && objectType() == ImageObject;
   }

   bool isTableFormat() const {
      return type() == FrameFormat && objectType() == TableObject;
   }

   bool isTableCellFormat() const {
      return type() == CharFormat && objectType() == TableCellObject;
   }

   QTextBlockFormat toBlockFormat() const;
   QTextCharFormat toCharFormat() const;
   QTextListFormat toListFormat() const;
   QTextTableFormat toTableFormat() const;
   QTextFrameFormat toFrameFormat() const;
   QTextImageFormat toImageFormat() const;
   QTextTableCellFormat toTableCellFormat() const;

   bool operator==(const QTextFormat &other) const;

   bool operator!=(const QTextFormat &other) const {
      return !operator==(other);
   }
   operator QVariant() const;

   void setLayoutDirection(Qt::LayoutDirection direction) {
      setProperty(QTextFormat::LayoutDirection, direction);
   }

   Qt::LayoutDirection layoutDirection() const {
      return Qt::LayoutDirection(intProperty(QTextFormat::LayoutDirection));
   }

   void setBackground(const QBrush &brush) {
      setProperty(BackgroundBrush, brush);
   }

   QBrush background() const {
      return brushProperty(BackgroundBrush);
   }

   void clearBackground() {
      clearProperty(BackgroundBrush);
   }

   void setForeground(const QBrush &brush) {
      setProperty(ForegroundBrush, brush);
   }

   QBrush foreground() const {
      return brushProperty(ForegroundBrush);
   }

   void clearForeground() {
      clearProperty(ForegroundBrush);
   }

 private:
   QSharedDataPointer<QTextFormatPrivate> d;
   qint32 format_type;

   friend class QTextFormatCollection;
   friend class QTextCharFormat;
   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QTextFormat &);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QTextFormat &);
};

inline void QTextFormat::setObjectType(int type)
{
   setProperty(ObjectType, type);
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextFormat::PageBreakFlags)

class Q_GUI_EXPORT QTextCharFormat : public QTextFormat
{
 public:
   enum VerticalAlignment {
      AlignNormal = 0,
      AlignSuperScript,
      AlignSubScript,
      AlignMiddle,
      AlignTop,
      AlignBottom,
      AlignBaseline
   };

   // keep in sync with enum PenStyle in core/global/qnamespace.h
   enum UnderlineStyle {
      NoUnderline,
      SingleUnderline,
      DashUnderline,
      DotLine,
      DashDotLine,
      DashDotDotLine,
      WaveUnderline,
      SpellCheckUnderline
   };

   enum FontPropertiesInheritanceBehavior {
      FontPropertiesSpecifiedOnly,
      FontPropertiesAll
   };

   void setFont(const QFont &font, FontPropertiesInheritanceBehavior behavior);

   QTextCharFormat();

   bool isValid() const {
      return isCharFormat();
   }

   void setFont(const QFont &font);
   QFont font() const;

   void setFontFamily(const QString &family) {
      setProperty(FontFamily, family);
   }

   QString fontFamily() const {
      return stringProperty(FontFamily);
   }

   void setFontPointSize(qreal size) {
      setProperty(FontPointSize, size);
   }

   qreal fontPointSize() const {
      return doubleProperty(FontPointSize);
   }

   void setFontWeight(int weight) {
      setProperty(FontWeight, weight);
   }

   int fontWeight() const {
      return hasProperty(FontWeight) ? intProperty(FontWeight) : QFont::Normal;
   }

   void setFontItalic(bool italic) {
      setProperty(FontItalic, italic);
   }

   bool fontItalic() const {
      return boolProperty(FontItalic);
   }

   void setFontCapitalization(QFont::Capitalization capitalization) {
      setProperty(FontCapitalization, capitalization);
   }

   QFont::Capitalization fontCapitalization() const {
      return static_cast<QFont::Capitalization>(intProperty(FontCapitalization));
   }

   void setFontLetterSpacingType(QFont::SpacingType letterSpacingType) {
      setProperty(FontLetterSpacingType, letterSpacingType);
   }

   QFont::SpacingType fontLetterSpacingType() const {
      return static_cast<QFont::SpacingType>(intProperty(FontLetterSpacingType));
   }

   void setFontLetterSpacing(qreal spacing) {
      setProperty(FontLetterSpacing, spacing);
   }

   qreal fontLetterSpacing() const {
      return doubleProperty(FontLetterSpacing);
   }

   void setFontWordSpacing(qreal spacing) {
      setProperty(FontWordSpacing, spacing);
   }

   qreal fontWordSpacing() const {
      return doubleProperty(FontWordSpacing);
   }

   void setFontUnderline(bool underline) {
      setProperty(TextUnderlineStyle, underline ? SingleUnderline : NoUnderline);
   }

   bool fontUnderline() const;

   void setFontOverline(bool overline) {
      setProperty(FontOverline, overline);
   }

   bool fontOverline() const {
      return boolProperty(FontOverline);
   }

   void setFontStrikeOut(bool strikeOut) {
      setProperty(FontStrikeOut, strikeOut);
   }

   bool fontStrikeOut() const {
      return boolProperty(FontStrikeOut);
   }

   void setUnderlineColor(const QColor &color) {
      setProperty(TextUnderlineColor, color);
   }

   QColor underlineColor() const {
      return colorProperty(TextUnderlineColor);
   }

   void setFontFixedPitch(bool fixedPitch) {
      setProperty(FontFixedPitch, fixedPitch);
   }

   bool fontFixedPitch() const {
      return boolProperty(FontFixedPitch);
   }

   void setFontStretch(int factor) {
      setProperty(FontStretch, factor);
   }

   int fontStretch() const  {
      return intProperty(FontStretch);
   }

   void setFontStyleHint(QFont::StyleHint hint, QFont::StyleStrategy strategy = QFont::PreferDefault) {
      setProperty(FontStyleHint, hint);
      setProperty(FontStyleStrategy, strategy);
   }

   void setFontStyleStrategy(QFont::StyleStrategy strategy) {
      setProperty(FontStyleStrategy, strategy);
   }

   QFont::StyleHint fontStyleHint() const {
      return static_cast<QFont::StyleHint>(intProperty(FontStyleHint));
   }

   QFont::StyleStrategy fontStyleStrategy() const {
      return static_cast<QFont::StyleStrategy>(intProperty(FontStyleStrategy));
   }

   void setFontHintingPreference(QFont::HintingPreference hintingPreference) {
      setProperty(FontHintingPreference, hintingPreference);
   }

   QFont::HintingPreference fontHintingPreference() const {
      return static_cast<QFont::HintingPreference>(intProperty(FontHintingPreference));
   }

   void setFontKerning(bool enable) {
      setProperty(FontKerning, enable);
   }

   bool fontKerning() const {
      return boolProperty(FontKerning);
   }

   void setUnderlineStyle(UnderlineStyle style);

   UnderlineStyle underlineStyle() const {
      return static_cast<UnderlineStyle>(intProperty(TextUnderlineStyle));
   }

   void setVerticalAlignment(VerticalAlignment alignment) {
      setProperty(TextVerticalAlignment, alignment);
   }

   VerticalAlignment verticalAlignment() const {
      return static_cast<VerticalAlignment>(intProperty(TextVerticalAlignment));
   }

   void setTextOutline(const QPen &pen) {
      setProperty(TextOutline, pen);
   }

   QPen textOutline() const {
      return penProperty(TextOutline);
   }

   void setToolTip(const QString &text) {
      setProperty(TextToolTip, text);
   }

   QString toolTip() const {
      return stringProperty(TextToolTip);
   }

   void setAnchor(bool anchor) {
      setProperty(IsAnchor, anchor);
   }

   bool isAnchor() const {
      return boolProperty(IsAnchor);
   }

   void setAnchorHref(const QString &value) {
      setProperty(AnchorHref, value);
   }

   QString anchorHref() const {
      return stringProperty(AnchorHref);
   }

   void setAnchorName(const QString &name) {
      setAnchorNames(QStringList(name));
   }

   QString anchorName() const;

   void setAnchorNames(const QStringList &names) {
      setProperty(AnchorName, names);
   }

   QStringList anchorNames() const;

   void setTableCellRowSpan(int cellRowSpan);
   int tableCellRowSpan() const {
      int s = intProperty(TableCellRowSpan);
      if (s == 0) {
         s = 1;
      }
      return s;
   }

   inline void setTableCellColumnSpan(int cellRowSpan);

   int tableCellColumnSpan() const {
      int s = intProperty(TableCellColumnSpan);
      if (s == 0) {
         s = 1;
      }
      return s;
   }

 protected:
   explicit QTextCharFormat(const QTextFormat &fmt);
   friend class QTextFormat;
};

inline void QTextCharFormat::setTableCellRowSpan(int cellRowSpan)
{
   if (cellRowSpan <= 1) {
      clearProperty(TableCellRowSpan);   // the getter will return 1 here.
   } else {
      setProperty(TableCellRowSpan, cellRowSpan);
   }
}

inline void QTextCharFormat::setTableCellColumnSpan(int cellRowSpan)
{
   if (cellRowSpan <= 1) {
      clearProperty(TableCellColumnSpan);   // the getter will return 1 here.
   } else {
      setProperty(TableCellColumnSpan, cellRowSpan);
   }
}

class Q_GUI_EXPORT QTextBlockFormat : public QTextFormat
{
 public:
   enum LineHeightTypes {
      SingleHeight = 0,
      ProportionalHeight = 1,
      FixedHeight = 2,
      MinimumHeight = 3,
      LineDistanceHeight = 4
   };

   QTextBlockFormat();

   bool isValid() const {
      return isBlockFormat();
   }

   inline void setAlignment(Qt::Alignment alignment);

   Qt::Alignment alignment() const {
      int a = intProperty(BlockAlignment);
      if (a == 0) {
         a = Qt::AlignLeft;
      }
      return QFlag(a);
   }

   void setTopMargin(qreal margin) {
      setProperty(BlockTopMargin, margin);
   }

   qreal topMargin() const {
      return doubleProperty(BlockTopMargin);
   }

   void setBottomMargin(qreal margin) {
      setProperty(BlockBottomMargin, margin);
   }

   qreal bottomMargin() const {
      return doubleProperty(BlockBottomMargin);
   }

   void setLeftMargin(qreal margin) {
      setProperty(BlockLeftMargin, margin);
   }

   qreal leftMargin() const {
      return doubleProperty(BlockLeftMargin);
   }

   void setRightMargin(qreal margin) {
      setProperty(BlockRightMargin, margin);
   }

   qreal rightMargin() const {
      return doubleProperty(BlockRightMargin);
   }

   void setTextIndent(qreal indent) {
      setProperty(TextIndent, indent);
   }

   qreal textIndent() const {
      return doubleProperty(TextIndent);
   }

   inline void setIndent(int indent);

   int indent() const {
      return intProperty(BlockIndent);
   }

   void setLineHeight(qreal height, int heightType) {
      setProperty(LineHeight, height);
      setProperty(LineHeightType, heightType);
   }

   inline qreal lineHeight(qreal scriptLineHeight, qreal scaling) const;

   qreal lineHeight() const {
      return doubleProperty(LineHeight);
   }

   int lineHeightType() const {
      return intProperty(LineHeightType);
   }

   void setNonBreakableLines(bool b) {
      setProperty(BlockNonBreakableLines, b);
   }

   bool nonBreakableLines() const {
      return boolProperty(BlockNonBreakableLines);
   }

   void setPageBreakPolicy(PageBreakFlags policy) {
      setProperty(PageBreakPolicy, int(policy));
   }

   PageBreakFlags pageBreakPolicy() const {
      return PageBreakFlags(intProperty(PageBreakPolicy));
   }

   void setTabPositions(const QList<QTextOption::Tab> &tabs);
   QList<QTextOption::Tab> tabPositions() const;

 protected:
   explicit QTextBlockFormat(const QTextFormat &fmt);
   friend class QTextFormat;
};

inline void QTextBlockFormat::setAlignment(Qt::Alignment alignment)
{
   setProperty(BlockAlignment, int(alignment));
}

inline void QTextBlockFormat::setIndent(int indent)
{
   setProperty(BlockIndent, indent);
}

inline qreal QTextBlockFormat::lineHeight(qreal scriptLineHeight, qreal scaling = 1.0) const
{
   switch (intProperty(LineHeightType)) {
      case SingleHeight:
         return (scriptLineHeight);
      case ProportionalHeight:
         return (scriptLineHeight * doubleProperty(LineHeight) / 100.0);
      case FixedHeight:
         return (doubleProperty(LineHeight) * scaling);
      case MinimumHeight:
         return (qMax(scriptLineHeight, doubleProperty(LineHeight) * scaling));
      case LineDistanceHeight:
         return (scriptLineHeight + doubleProperty(LineHeight) * scaling);
   }
   return (0);
}

class Q_GUI_EXPORT QTextListFormat : public QTextFormat
{
 public:
   QTextListFormat();

   bool isValid() const {
      return isListFormat();
   }

   enum Style {
      ListDisc = -1,
      ListCircle = -2,
      ListSquare = -3,
      ListDecimal = -4,
      ListLowerAlpha = -5,
      ListUpperAlpha = -6,
      ListLowerRoman = -7,
      ListUpperRoman = -8,
      ListStyleUndefined = 0
   };

   inline void setStyle(Style style);
   Style style() const {
      return static_cast<Style>(intProperty(ListStyle));
   }

   inline void setIndent(int indent);
   int indent() const {
      return intProperty(ListIndent);
   }

   inline void setNumberPrefix(const QString &prefix);
   QString numberPrefix() const {
      return stringProperty(ListNumberPrefix);
   }

   inline void setNumberSuffix(const QString &suffix);
   QString numberSuffix() const {
      return stringProperty(ListNumberSuffix);
   }

 protected:
   explicit QTextListFormat(const QTextFormat &fmt);
   friend class QTextFormat;
};

inline void QTextListFormat::setStyle(Style style)
{
   setProperty(ListStyle, style);
}

inline void QTextListFormat::setIndent(int indent)
{
   setProperty(ListIndent, indent);
}

inline void QTextListFormat::setNumberPrefix(const QString &prefix)
{
   setProperty(ListNumberPrefix, prefix);
}

inline void QTextListFormat::setNumberSuffix(const QString &suffix)
{
   setProperty(ListNumberSuffix, suffix);
}

class Q_GUI_EXPORT QTextImageFormat : public QTextCharFormat
{
 public:
   QTextImageFormat();

   bool isValid() const {
      return isImageFormat();
   }

   inline void setName(const QString &name);
   QString name() const {
      return stringProperty(ImageName);
   }

   inline void setWidth(qreal width);
   qreal width() const {
      return doubleProperty(ImageWidth);
   }

   inline void setHeight(qreal height);
   qreal height() const {
      return doubleProperty(ImageHeight);
   }

 protected:
   explicit QTextImageFormat(const QTextFormat &format);
   friend class QTextFormat;
};

inline void QTextImageFormat::setName(const QString &name)
{
   setProperty(ImageName, name);
}

inline void QTextImageFormat::setWidth(qreal width)
{
   setProperty(ImageWidth, width);
}

inline void QTextImageFormat::setHeight(qreal height)
{
   setProperty(ImageHeight, height);
}

class Q_GUI_EXPORT QTextFrameFormat : public QTextFormat
{
 public:
   QTextFrameFormat();

   bool isValid() const {
      return isFrameFormat();
   }

   enum Position {
      InFlow,
      FloatLeft,
      FloatRight
      // ######
      // Absolute
   };

   enum BorderStyle {
      BorderStyle_None,
      BorderStyle_Dotted,
      BorderStyle_Dashed,
      BorderStyle_Solid,
      BorderStyle_Double,
      BorderStyle_DotDash,
      BorderStyle_DotDotDash,
      BorderStyle_Groove,
      BorderStyle_Ridge,
      BorderStyle_Inset,
      BorderStyle_Outset
   };

   void setPosition(Position policy) {
      setProperty(CssFloat, policy);
   }

   Position position() const {
      return static_cast<Position>(intProperty(CssFloat));
   }

   inline void setBorder(qreal width);
   qreal border() const {
      return doubleProperty(FrameBorder);
   }

   void setBorderBrush(const QBrush &brush) {
      setProperty(FrameBorderBrush, brush);
   }

   QBrush borderBrush() const {
      return brushProperty(FrameBorderBrush);
   }

   void setBorderStyle(BorderStyle style) {
      setProperty(FrameBorderStyle, style);
   }

   BorderStyle borderStyle() const {
      return static_cast<BorderStyle>(intProperty(FrameBorderStyle));
   }

   void setMargin(qreal margin);
   qreal margin() const {
      return doubleProperty(FrameMargin);
   }

   inline void setTopMargin(qreal margin);
   qreal topMargin() const;

   inline void setBottomMargin(qreal margin);
   qreal bottomMargin() const;

   inline void setLeftMargin(qreal margin);
   qreal leftMargin() const;

   inline void setRightMargin(qreal margin);
   qreal rightMargin() const;

   inline void setPadding(qreal width);
   qreal padding() const {
      return doubleProperty(FramePadding);
   }

   inline void setWidth(qreal width);
   void setWidth(const QTextLength &width) {
      setProperty(FrameWidth, width);
   }
   QTextLength width() const {
      return lengthProperty(FrameWidth);
   }

   inline void setHeight(qreal height);
   inline void setHeight(const QTextLength &height);
   QTextLength height() const {
      return lengthProperty(FrameHeight);
   }

   void setPageBreakPolicy(PageBreakFlags policy) {
      setProperty(PageBreakPolicy, int(policy));
   }

   PageBreakFlags pageBreakPolicy() const {
      return PageBreakFlags(intProperty(PageBreakPolicy));
   }

 protected:
   explicit QTextFrameFormat(const QTextFormat &fmt);
   friend class QTextFormat;
};

inline void QTextFrameFormat::setBorder(qreal width)
{
   setProperty(FrameBorder, width);
}

inline void QTextFrameFormat::setPadding(qreal width)
{
   setProperty(FramePadding, width);
}

inline void QTextFrameFormat::setWidth(qreal width)
{
   setProperty(FrameWidth, QTextLength(QTextLength::FixedLength, width));
}

inline void QTextFrameFormat::setHeight(qreal height)
{
   setProperty(FrameHeight, QTextLength(QTextLength::FixedLength, height));
}

inline void QTextFrameFormat::setHeight(const QTextLength &height)
{
   setProperty(FrameHeight, height);
}

inline void QTextFrameFormat::setTopMargin(qreal margin)
{
   setProperty(FrameTopMargin, margin);
}

inline void QTextFrameFormat::setBottomMargin(qreal margin)
{
   setProperty(FrameBottomMargin, margin);
}

inline void QTextFrameFormat::setLeftMargin(qreal margin)
{
   setProperty(FrameLeftMargin, margin);
}

inline void QTextFrameFormat::setRightMargin(qreal margin)
{
   setProperty(FrameRightMargin, margin);
}

class Q_GUI_EXPORT QTextTableFormat : public QTextFrameFormat
{
 public:
   QTextTableFormat();

   bool isValid() const {
      return isTableFormat();
   }

   int columns() const {
      int cols = intProperty(TableColumns);
      if (cols == 0) {
         cols = 1;
      }
      return cols;
   }
   inline void setColumns(int columns);

   void setColumnWidthConstraints(const QVector<QTextLength> &constraints) {
      setProperty(TableColumnWidthConstraints, constraints);
   }

   QVector<QTextLength> columnWidthConstraints() const {
      return lengthVectorProperty(TableColumnWidthConstraints);
   }

   void clearColumnWidthConstraints() {
      clearProperty(TableColumnWidthConstraints);
   }

   qreal cellSpacing() const {
      return doubleProperty(TableCellSpacing);
   }
   void setCellSpacing(qreal spacing) {
      setProperty(TableCellSpacing, spacing);
   }

   qreal cellPadding() const {
      return doubleProperty(TableCellPadding);
   }
   inline void setCellPadding(qreal padding);

   inline void setAlignment(Qt::Alignment alignment);
   Qt::Alignment alignment() const {
      return QFlag(intProperty(BlockAlignment));
   }

   void setHeaderRowCount(int count) {
      setProperty(TableHeaderRowCount, count);
   }
   int headerRowCount() const {
      return intProperty(TableHeaderRowCount);
   }

 protected:
   explicit QTextTableFormat(const QTextFormat &fmt);
   friend class QTextFormat;
};

inline void QTextTableFormat::setColumns(int columns)
{
   if (columns == 1) {
      columns = 0;
   }

   setProperty(TableColumns, columns);
}

inline void QTextTableFormat::setCellPadding(qreal padding)
{
   setProperty(TableCellPadding, padding);
}

inline void QTextTableFormat::setAlignment(Qt::Alignment alignment)
{
   setProperty(BlockAlignment, int(alignment));
}

class Q_GUI_EXPORT QTextTableCellFormat : public QTextCharFormat
{

 public:
   QTextTableCellFormat();

   bool isValid() const {
      return isTableCellFormat();
   }

   inline void setTopPadding(qreal padding);
   inline qreal topPadding() const;

   inline void setBottomPadding(qreal padding);
   inline qreal bottomPadding() const;

   inline void setLeftPadding(qreal padding);
   inline qreal leftPadding() const;

   inline void setRightPadding(qreal padding);
   inline qreal rightPadding() const;

   inline void setPadding(qreal padding);

 protected:
   explicit QTextTableCellFormat(const QTextFormat &fmt);
   friend class QTextFormat;
};

inline void QTextTableCellFormat::setTopPadding(qreal padding)
{
   setProperty(TableCellTopPadding, padding);
}

inline qreal QTextTableCellFormat::topPadding() const
{
   return doubleProperty(TableCellTopPadding);
}

inline void QTextTableCellFormat::setBottomPadding(qreal padding)
{
   setProperty(TableCellBottomPadding, padding);
}

inline qreal QTextTableCellFormat::bottomPadding() const
{
   return doubleProperty(TableCellBottomPadding);
}

inline void QTextTableCellFormat::setLeftPadding(qreal padding)
{
   setProperty(TableCellLeftPadding, padding);
}

inline qreal QTextTableCellFormat::leftPadding() const
{
   return doubleProperty(TableCellLeftPadding);
}

inline void QTextTableCellFormat::setRightPadding(qreal padding)
{
   setProperty(TableCellRightPadding, padding);
}

inline qreal QTextTableCellFormat::rightPadding() const
{
   return doubleProperty(TableCellRightPadding);
}

inline void QTextTableCellFormat::setPadding(qreal padding)
{
   setTopPadding(padding);
   setBottomPadding(padding);
   setLeftPadding(padding);
   setRightPadding(padding);
}

#endif // QTEXTFORMAT_H
