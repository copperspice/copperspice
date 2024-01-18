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

#ifndef QCHAR32_H
#define QCHAR32_H

#define CS_STRING_ALLOW_UNSAFE

#include <qglobal.h>
#include <cs_char.h>
#include <cs_string.h>

class QChar32;
class QString8;
class QString16;

inline uint qHash(const QChar32 &key, uint seed = 0);

class Q_CORE_EXPORT QChar32 : public CsString::CsChar
{
  public:
   enum CombiningClass {
      Combining_BelowLeftAttached    = 200,
      Combining_BelowAttached        = 202,
      Combining_BelowRightAttached   = 204,
      Combining_LeftAttached         = 208,
      Combining_RightAttached        = 210,
      Combining_AboveLeftAttached    = 212,
      Combining_AboveAttached        = 214,
      Combining_AboveRightAttached   = 216,

      Combining_BelowLeft            = 218,
      Combining_Below                = 220,
      Combining_BelowRight           = 222,
      Combining_Left                 = 224,
      Combining_Right                = 226,
      Combining_AboveLeft            = 228,
      Combining_Above                = 230,
      Combining_AboveRight           = 232,

      Combining_DoubleBelow          = 233,
      Combining_DoubleAbove          = 234,
      Combining_IotaSubscript        = 240
   };

   enum SpecialCharacter {
      Null                           = 0x0000,
      Tabulation                     = 0x0009,
      LineFeed                       = 0x000a,
      FormFeed                       = 0x000c,
      CarriageReturn                 = 0x000d,
      Space                          = 0x0020,
      Nbsp                           = 0x00a0,
      SoftHyphen                     = 0x00ad,
      ReplacementCharacter           = 0xfffd,
      ObjectReplacementCharacter     = 0xfffc,
      ByteOrderMark                  = 0xfeff,
      ByteOrderSwapped               = 0xfffe,
      ParagraphSeparator             = 0x2029,
      LineSeparator                  = 0x2028,
      VisualTabCharacter             = 0x2192,
      LastValidCodePoint             = 0x10ffff
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
      Symbol_Other,             //   So

      CategoryCount
   };

   enum Script {
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

      // Unicode 4.0
      Script_Limbu,
      Script_TaiLe,
      Script_LinearB,
      Script_Ugaritic,
      Script_Shavian,
      Script_Osmanya,
      Script_Cypriot,
      Script_Braille,

      // Unicode 4.1
      Script_Buginese,
      Script_NewTaiLue,
      Script_Glagolitic,
      Script_Tifinagh,
      Script_SylotiNagri,
      Script_OldPersian,
      Script_Kharoshthi,

      // Unicode 5.0
      Script_Balinese,
      Script_Cuneiform,
      Script_Phoenician,
      Script_PhagsPa,
      Script_Nko,

      // Unicode 5.1
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

      // Unicode 5.2
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

      // Unicode 6.0
      Script_Batak,
      Script_Brahmi,
      Script_Mandaic,

      // Unicode 6.1
      Script_Chakma,
      Script_MeroiticCursive,
      Script_MeroiticHieroglyphs,
      Script_Miao,
      Script_Sharada,
      Script_SoraSompeng,
      Script_Takri,

      // Unicode 7.0
      Script_Caucasian_Albanian,
      Script_Bassa_Vah,
      Script_Duployan,
      Script_Elbasan,
      Script_Grantha,
      Script_Pahawh_Hmong,
      Script_Khojki,
      Script_Linear_A,
      Script_Mahajani,
      Script_Manichaean,
      Script_Mende_Kikakui,
      Script_Modi,
      Script_Mro,
      Script_Old_North_Arabian,
      Script_Nabataean,
      Script_Palmyrene,
      Script_PauCinHau,
      Script_OldPermic,
      Script_Psalter_Pahlavi,
      Script_Siddham,
      Script_Khudawadi,
      Script_Tirhuta,
      Script_Warang_Citi,

      // Unicode 8.0
      Script_Ahom,
      Script_Anatolian_Hieroglyphs,
      Script_Hatran,
      Script_Multani,
      Script_Old_Hungarian,
      Script_SignWriting,

      // Unicode 9.0
      Script_Adlam,
      Script_Bhaiksuki,
      Script_Marchen,
      Script_Newa,
      Script_Osage,
      Script_Tangut,

      // Unicode 10.0
      Script_Masaram_Gondi,
      Script_Nushu,
      Script_Soyombo,
      Script_Zanabazar_Square,

      // Unicode 11.0
      Script_Dogra,
      Script_Gunjala_Gondi,
      Script_Hanifi_Rohingya,
      Script_Makasar,
      Script_Medefaidrin,
      Script_Old_Sogdian,
      Script_Sogdian,

      // Unicode 12.0
      Script_Elymaic,
      Script_Nandinagari,
      Script_Nyiakeng_Puachue_Hmong,
      Script_Wancho,

      // Unicode 13.0
      Script_Chorasmian,
      Script_Dives_Akuru,
      Script_Khitan_Small_Script,
      Script_Yezidi,

      // Unicode 14.0
      Script_Cypro_Minoan,
      Script_Old_Uyghur,
      Script_Tangsa,
      Script_Toto,
      Script_Vithkuqi,

      // Unicode 15.0
      Script_Kawi,
      Script_Nag_Mundari,

      // Unicode 15.1
      // nothing was added

      ScriptCount
   };

   enum Direction {
      DirL, DirR, DirEN, DirES, DirET, DirAN, DirCS, DirB, DirS, DirWS, DirON,
      DirLRE, DirLRO, DirAL, DirRLE, DirRLO, DirPDF, DirNSM, DirBN,
      DirLRI, DirRLI, DirFSI, DirPDI
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

   enum JoiningType {
      Joining_None,
      Joining_Causing,
      Joining_Dual,
      Joining_Right,
      Joining_Left,
      Joining_Transparent
   };

   enum UnicodeVersion {
      Unicode_Unassigned,
      Unicode_1_1,
      Unicode_2_0,
      Unicode_2_1,
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
      Unicode_8_0,
      Unicode_9_0,
      Unicode_10_0,
      Unicode_11_0,
      Unicode_12_0,
      Unicode_12_1,
      Unicode_13_0,
      Unicode_14_0,
      Unicode_15_0,
      Unicode_15_1,
   };

   QChar32() = default;

   explicit QChar32(char c)
      : CsString::CsChar(c)
   { }

   QChar32(char32_t c)
      : CsString::CsChar(c)
   { }

   QChar32(char16_t c)
      : CsString::CsChar(c)
   { }

   QChar32(int c)
      : CsString::CsChar(c)
   { }

   QChar32(CsString::CsChar c)
      : CsString::CsChar(c)
   { }

   QChar32(SpecialCharacter c)
      : CsString::CsChar(static_cast<char32_t>(c))
   { }

   QChar32(const QChar32 &other)
      : CsString::CsChar(other)
   { }

   ~QChar32() = default;

   // methods
   Category category() const;
   unsigned char combiningClass() const;

   QString8 decomposition() const;
   Decomposition decompositionTag() const;

   int digitValue() const;
   Direction direction() const;

   static QChar32 fromLatin1(char c) {
      QChar32 retval(static_cast<char32_t>(c));
      return retval;
   }

   bool hasMirrored() const;

   bool isDigit() const {
      return category() == Number_DecimalDigit;
   }

   bool isLetter() const;
   bool isLetterOrNumber() const;

   bool isLower() const {
      return category() == Letter_Lowercase;
   }

   bool isMark() const;

   bool isNonCharacter() const;

   bool isNull() const {
      return unicode() == 0;
   }

   bool isNumber() const;

   bool isPrint() const;
   bool isPunct() const;
   bool isSpace() const;
   bool isSymbol() const;

   bool isTitleCase() const {
      return category() == Letter_Titlecase;
   }

   bool isUpper() const {
      return category() == Letter_Uppercase;
   }

   JoiningType joiningType() const;

   QChar32 mirroredChar() const;

   Script script() const;

   QString8 toCaseFolded() const;
   QString16 toCaseFolded16() const;

   char toLatin1() const {
      uint32_t tmp = unicode();

      if (tmp < 256) {
         return static_cast<char>(tmp);
      }

      return 0;
   }

   QString8 toLower() const;
   QString8 toUpper() const;
   QString8 toTitleCase() const;

   uint32_t unicode() const {
      return CsString::CsChar::unicode();
   }

   UnicodeVersion unicodeVersion() const;
   static UnicodeVersion currentUnicodeVersion();

   QChar32 &operator=(QChar32 c) & {
      CsString::CsChar::operator=(c);
      return *this;
   }
};

class Q_CORE_EXPORT QChar32Arrow : public CsString::CsCharArrow
{
  public:
   QChar32Arrow (CsString::CsCharArrow c)
      : CsString::CsCharArrow(c)
   {
   }

   const QChar32 *operator->() const {
      static_assert(std::is_standard_layout<CsString::CsChar>::value, "Invalid reinterpret_cast for QChar32Arrow");
      static_assert(sizeof(QChar32) == sizeof(CsString::CsChar), "Invalid reinterpret_cast for QChar32Arrow");

      return reinterpret_cast<const QChar32 *>(CsString::CsCharArrow::operator->());
   }
};

inline bool operator==(QChar32 c1, QChar32 c2)
{
   return c1.unicode() == c2.unicode();
}

inline bool operator!=(QChar32 c1, QChar32 c2)
{
   return c1.unicode() != c2.unicode();
}

inline bool operator<=(QChar32 c1, QChar32 c2)
{
   return c1.unicode() <= c2.unicode();
}

inline bool operator>=(QChar32 c1, QChar32 c2)
{
   return c1.unicode() >= c2.unicode();
}

inline bool operator<(QChar32 c1, QChar32 c2)
{
   return c1.unicode() < c2.unicode();
}

inline bool operator>(QChar32 c1, QChar32 c2)
{
   return c1.unicode() > c2.unicode();
}

inline uint qHash(const QChar32 &key, uint seed)
{
   return key.unicode() ^ seed;
}

namespace std {
   template <>
   struct hash<QChar32> {
      size_t operator()(const QChar32 &key) const {
         return key.unicode();
      }
   };
}

#endif
