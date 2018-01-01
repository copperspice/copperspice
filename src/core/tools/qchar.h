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

#ifndef QCHAR_H
#define QCHAR_H

#include <qglobal.h>
#include <qexport.h>

class QString;
class QDataStream;

struct QLatin1Char {
 public:
   inline explicit QLatin1Char(char c) : ch(c) {}

   inline char toLatin1() const {
      return ch;
   }

   inline ushort unicode() const {
      return ushort(uchar(ch));
   }

 private:
   char ch;
};


class Q_CORE_EXPORT QChar
{
 public:
   enum SpecialCharacter {
        Null                       = 0x0000,
        Tabulation                 = 0x0009,
        LineFeed                   = 0x000a,
        CarriageReturn             = 0x000d,
        Space                      = 0x0020,
        Nbsp                       = 0x00a0,
        SoftHyphen                 = 0x00ad,
        ReplacementCharacter       = 0xfffd,
        ObjectReplacementCharacter = 0xfffc,
        ByteOrderMark              = 0xfeff,
        ByteOrderSwapped           = 0xfffe,
        ParagraphSeparator         = 0x2029,
        LineSeparator              = 0x2028,
        LastValidCodePoint         = 0x10ffff
   };

   enum Category {
        Mark_NonSpacing,          //   Mn
        Mark_SpacingCombining,    //   Mc
        Mark_Enclosing,           //   Me

        Number_DecimalDigit,      //   Nd
        Number_Letter,            //   Nl
        Number_Other,             //   No

        Separator_Space,          //   Zs
        Separator_Line,           //   Zl
        Separator_Paragraph,      //   Zp

        Other_Control,            //   Cc
        Other_Format,             //   Cf
        Other_Surrogate,          //   Cs
        Other_PrivateUse,         //   Co
        Other_NotAssigned,        //   Cn

        Letter_Uppercase,         //   Lu
        Letter_Lowercase,         //   Ll
        Letter_Titlecase,         //   Lt
        Letter_Modifier,          //   Lm
        Letter_Other,             //   Lo

        Punctuation_Connector,    //   Pc
        Punctuation_Dash,         //   Pd
        Punctuation_Open,         //   Ps
        Punctuation_Close,        //   Pe
        Punctuation_InitialQuote, //   Pi
        Punctuation_FinalQuote,   //   Pf
        Punctuation_Other,        //   Po

        Symbol_Math,              //   Sm
        Symbol_Currency,          //   Sc
        Symbol_Modifier,          //   Sk
        Symbol_Other              //   So
   };

   enum Script
   {
        Script_Unknown,
        Script_Inherited,
        Script_Common,

        Script_Latin,
        Script_Greek,
        Script_Cyrillic,
        Script_Armenian,
        Script_Hebrew,
        Script_Arabic,
        Script_Syriac,
        Script_Thaana,
        Script_Devanagari,
        Script_Bengali,
        Script_Gurmukhi,
        Script_Gujarati,
        Script_Oriya,
        Script_Tamil,
        Script_Telugu,
        Script_Kannada,
        Script_Malayalam,
        Script_Sinhala,
        Script_Thai,
        Script_Lao,
        Script_Tibetan,
        Script_Myanmar,
        Script_Georgian,
        Script_Hangul,
        Script_Ethiopic,
        Script_Cherokee,
        Script_CanadianAboriginal,
        Script_Ogham,
        Script_Runic,
        Script_Khmer,
        Script_Mongolian,
        Script_Hiragana,
        Script_Katakana,
        Script_Bopomofo,
        Script_Han,
        Script_Yi,
        Script_OldItalic,
        Script_Gothic,
        Script_Deseret,
        Script_Tagalog,
        Script_Hanunoo,
        Script_Buhid,
        Script_Tagbanwa,
        Script_Coptic,

        // Unicode 4.0 additions
        Script_Limbu,
        Script_TaiLe,
        Script_LinearB,
        Script_Ugaritic,
        Script_Shavian,
        Script_Osmanya,
        Script_Cypriot,
        Script_Braille,

        // Unicode 4.1 additions
        Script_Buginese,
        Script_NewTaiLue,
        Script_Glagolitic,
        Script_Tifinagh,
        Script_SylotiNagri,
        Script_OldPersian,
        Script_Kharoshthi,

        // Unicode 5.0 additions
        Script_Balinese,
        Script_Cuneiform,
        Script_Phoenician,
        Script_PhagsPa,
        Script_Nko,

        // Unicode 5.1 additions
        Script_Sundanese,
        Script_Lepcha,
        Script_OlChiki,
        Script_Vai,
        Script_Saurashtra,
        Script_KayahLi,
        Script_Rejang,
        Script_Lycian,
        Script_Carian,
        Script_Lydian,
        Script_Cham,

        // Unicode 5.2 additions
        Script_TaiTham,
        Script_TaiViet,
        Script_Avestan,
        Script_EgyptianHieroglyphs,
        Script_Samaritan,
        Script_Lisu,
        Script_Bamum,
        Script_Javanese,
        Script_MeeteiMayek,
        Script_ImperialAramaic,
        Script_OldSouthArabian,
        Script_InscriptionalParthian,
        Script_InscriptionalPahlavi,
        Script_OldTurkic,
        Script_Kaithi,

        // Unicode 6.0 additions
        Script_Batak,
        Script_Brahmi,
        Script_Mandaic,

        // Unicode 6.1 additions
        Script_Chakma,
        Script_MeroiticCursive,
        Script_MeroiticHieroglyphs,
        Script_Miao,
        Script_Sharada,
        Script_SoraSompeng,
        Script_Takri,

        // Unicode 7.0 additions
        Script_CaucasianAlbanian,
        Script_BassaVah,
        Script_Duployan,
        Script_Elbasan,
        Script_Grantha,
        Script_PahawhHmong,
        Script_Khojki,
        Script_LinearA,
        Script_Mahajani,
        Script_Manichaean,
        Script_MendeKikakui,
        Script_Modi,
        Script_Mro,
        Script_OldNorthArabian,
        Script_Nabataean,
        Script_Palmyrene,
        Script_PauCinHau,
        Script_OldPermic,
        Script_PsalterPahlavi,
        Script_Siddham,
        Script_Khudawadi,
        Script_Tirhuta,
        Script_WarangCiti,

        // Unicode 8.0 additions
        Script_Ahom,
        Script_AnatolianHieroglyphs,
        Script_Hatran,
        Script_Multani,
        Script_OldHungarian,
        Script_SignWriting,

        ScriptCount
   };

   enum Direction {
      DirL, DirR, DirEN, DirES, DirET, DirAN, DirCS, DirB, DirS, DirWS,
      DirON, DirLRE, DirLRO, DirAL, DirRLE, DirRLO, DirPDF, DirNSM, DirBN
   };

   enum Decomposition {
      NoDecomposition,
      Canonical,
      Font,
      NoBreak,
      Initial,
      Medial,
      Final,
      Isolated,
      Circle,
      Super,
      Sub,
      Vertical,
      Wide,
      Narrow,
      Small,
      Square,
      Compat,
      Fraction
   };

   enum Joining {
      OtherJoining, Dual, Right, Center
   };

   enum CombiningClass {
        Combining_BelowLeftAttached       = 200,
        Combining_BelowAttached           = 202,
        Combining_BelowRightAttached      = 204,
        Combining_LeftAttached            = 208,
        Combining_RightAttached           = 210,
        Combining_AboveLeftAttached       = 212,
        Combining_AboveAttached           = 214,
        Combining_AboveRightAttached      = 216,

        Combining_BelowLeft               = 218,
        Combining_Below                   = 220,
        Combining_BelowRight              = 222,
        Combining_Left                    = 224,
        Combining_Right                   = 226,
        Combining_AboveLeft               = 228,
        Combining_Above                   = 230,
        Combining_AboveRight              = 232,

        Combining_DoubleBelow             = 233,
        Combining_DoubleAbove             = 234,
        Combining_IotaSubscript           = 240
   };

   enum UnicodeVersion {
        Unicode_Unassigned,
        Unicode_1_1,
        Unicode_2_0,
        Unicode_2_1_2,
        Unicode_3_0,
        Unicode_3_1,
        Unicode_3_2,
        Unicode_4_0,
        Unicode_4_1,
        Unicode_5_0,
        Unicode_5_1,
        Unicode_5_2,
        Unicode_6_0,
        Unicode_6_1,
        Unicode_6_2,
        Unicode_6_3,
        Unicode_7_0,
        Unicode_8_0
   };

   QChar() : ucs(0) {}
   QChar(ushort rc) : ucs(rc) {} // implicit
   QChar(uchar c, uchar r) : ucs(ushort((r << 8) | c)) {}
   QChar(short rc) : ucs(ushort(rc)) {} // implicit
   QChar(uint rc) : ucs(ushort(rc & 0xffff)) {}
   QChar(int rc) : ucs(ushort(rc & 0xffff)) {}
   QChar(SpecialCharacter s) : ucs(ushort(s)) {} // implicit
   QChar(QLatin1Char ch) : ucs(ch.unicode()) {} // implicit

   explicit QChar(char c) : ucs(uchar(c)) { }
   explicit QChar(uchar c) : ucs(c) { }


   Category category() const;
   Direction direction() const;
   Joining joining() const;
   bool hasMirrored() const;
   unsigned char combiningClass() const;

   QChar mirroredChar() const;
   QString decomposition() const;
   Decomposition decompositionTag() const;

   int digitValue() const;
   QChar toLower() const;
   QChar toUpper() const;
   QChar toTitleCase() const;
   QChar toCaseFolded() const;

   inline Script script() const  {
      return QChar::script(ucs);
   }

   UnicodeVersion unicodeVersion() const;

   inline char toAscii() const;
   inline char toLatin1() const;
   inline ushort unicode() const {
      return ucs;
   }

#ifdef Q_NO_PACKED_REFERENCE
   inline ushort &unicode() {
      return const_cast<ushort &>(ucs);
   }
#else
   inline ushort &unicode() {
      return ucs;
   }
#endif

   static inline QChar fromAscii(char c);
   static inline QChar fromLatin1(char c);

   inline bool isNull() const {
      return ucs == 0;
   }
   bool isPrint() const;
   bool isPunct() const;
   bool isSpace() const;
   bool isMark() const;
   bool isLetter() const;
   bool isNumber() const;
   bool isLetterOrNumber() const;
   bool isDigit() const;
   bool isSymbol() const;

   inline bool isLower() const {
      return category() == Letter_Lowercase;
   }
   inline bool isUpper() const {
      return category() == Letter_Uppercase;
   }
   inline bool isTitleCase() const {
      return category() == Letter_Titlecase;
   }

   inline bool isHighSurrogate() const {
      return ((ucs & 0xfc00) == 0xd800);
   }
   inline bool isLowSurrogate() const {
      return ((ucs & 0xfc00) == 0xdc00);
   }

   inline uchar cell() const {
      return uchar(ucs & 0xff);
   }
   inline uchar row() const {
      return uchar((ucs >> 8) & 0xff);
   }
   inline void setCell(uchar cell);
   inline void setRow(uchar row);

   static inline bool isHighSurrogate(uint ucs4) {
      return ((ucs4 & 0xfffffc00) == 0xd800);
   }
   static inline bool isLowSurrogate(uint ucs4) {
      return ((ucs4 & 0xfffffc00) == 0xdc00);
   }
   static inline bool requiresSurrogates(uint ucs4) {
      return (ucs4 >= 0x10000);
   }
   static inline uint surrogateToUcs4(ushort high, ushort low) {
      return (uint(high) << 10) + low - 0x35fdc00;
   }
   static inline uint surrogateToUcs4(QChar high, QChar low) {
      return (uint(high.ucs) << 10) + low.ucs - 0x35fdc00;
   }
   static inline ushort highSurrogate(uint ucs4) {
      return ushort((ucs4 >> 10) + 0xd7c0);
   }
   static inline ushort lowSurrogate(uint ucs4) {
      return ushort(ucs4 % 0x400 + 0xdc00);
   }

   static Category QT_FASTCALL category(uint ucs4);
   static Category QT_FASTCALL category(ushort ucs2);
   static Direction QT_FASTCALL direction(uint ucs4);
   static Direction QT_FASTCALL direction(ushort ucs2);
   static Joining QT_FASTCALL joining(uint ucs4);
   static Joining QT_FASTCALL joining(ushort ucs2);
   static unsigned char QT_FASTCALL combiningClass(uint ucs4);
   static unsigned char QT_FASTCALL combiningClass(ushort ucs2);

   static uint QT_FASTCALL mirroredChar(uint ucs4);
   static ushort QT_FASTCALL mirroredChar(ushort ucs2);
   static Decomposition QT_FASTCALL decompositionTag(uint ucs4);

   static int QT_FASTCALL digitValue(uint ucs4);
   static int QT_FASTCALL digitValue(ushort ucs2);
   static uint QT_FASTCALL toLower(uint ucs4);
   static ushort QT_FASTCALL toLower(ushort ucs2);
   static uint QT_FASTCALL toUpper(uint ucs4);
   static ushort QT_FASTCALL toUpper(ushort ucs2);
   static uint QT_FASTCALL toTitleCase(uint ucs4);
   static ushort QT_FASTCALL toTitleCase(ushort ucs2);
   static uint QT_FASTCALL toCaseFolded(uint ucs4);
   static ushort QT_FASTCALL toCaseFolded(ushort ucs2);

   static Script QT_FASTCALL script(uint ucs4);

   static UnicodeVersion QT_FASTCALL unicodeVersion(uint ucs4);
   static UnicodeVersion QT_FASTCALL unicodeVersion(ushort ucs2);
   static UnicodeVersion QT_FASTCALL currentUnicodeVersion();

   static QString QT_FASTCALL decomposition(uint ucs4);

 private:
   ushort ucs;
}

#if (defined(__arm__) && defined(QT_NO_ARM_EABI))
Q_PACKED
#endif
;

Q_DECLARE_TYPEINFO(QChar, Q_MOVABLE_TYPE);

inline char QChar::toAscii() const
{
   return ucs > 0xff ? 0 : char(ucs);
}
inline char QChar::toLatin1() const
{
   return ucs > 0xff ? '\0' : char(ucs);
}
inline QChar QChar::fromLatin1(char c)
{
   return QChar(ushort(uchar(c)));
}
inline QChar QChar::fromAscii(char c)
{
   return QChar(ushort(uchar(c)));
}

inline void QChar::setCell(uchar acell)
{
   ucs = ushort((ucs & 0xff00) + acell);
}
inline void QChar::setRow(uchar arow)
{
   ucs = ushort((ushort(arow) << 8) + (ucs & 0xff));
}

inline bool operator==(QChar c1, QChar c2)
{
   return c1.unicode() == c2.unicode();
}
inline bool operator!=(QChar c1, QChar c2)
{
   return c1.unicode() != c2.unicode();
}
inline bool operator<=(QChar c1, QChar c2)
{
   return c1.unicode() <= c2.unicode();
}
inline bool operator>=(QChar c1, QChar c2)
{
   return c1.unicode() >= c2.unicode();
}
inline bool operator<(QChar c1, QChar c2)
{
   return c1.unicode() < c2.unicode();
}
inline bool operator>(QChar c1, QChar c2)
{
   return c1.unicode() > c2.unicode();
}

#ifndef QT_NO_DATASTREAM
   Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, QChar);
   Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QChar &);
#endif

#endif // QCHAR_H
