/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QCHAR_H
#define QCHAR_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QString;

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
      Null = 0x0000,
      Tabulation = 0x0009,
      LineFeed = 0x000a,
      CarriageReturn = 0x000d,
      Space = 0x0020,
      Nbsp = 0x00a0,
      ReplacementCharacter = 0xfffd,
      ObjectReplacementCharacter = 0xfffc,
      ByteOrderMark = 0xfeff,
      ByteOrderSwapped = 0xfffe,
      ParagraphSeparator = 0x2029,
      LineSeparator = 0x2028
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

   // Unicode information
   enum Category {
      NoCategory,    // ### Qt5/replace with Other_NotAssigned

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

      Punctuation_Dask = Punctuation_Dash // ### Qt5/remove
   };

   enum Direction {
      DirL, DirR, DirEN, DirES, DirET, DirAN, DirCS, DirB, DirS, DirWS, DirON,
      DirLRE, DirLRO, DirAL, DirRLE, DirRLO, DirPDF, DirNSM, DirBN
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
      Unicode_Unassigned,    // ### Qt5/assign with some constantly big value
      Unicode_1_1,
      Unicode_2_0,
      Unicode_2_1_2,
      Unicode_3_0,
      Unicode_3_1,
      Unicode_3_2,
      Unicode_4_0,
      Unicode_4_1,
      Unicode_5_0
   };
   // ****** WHEN ADDING FUNCTIONS, CONSIDER ADDING TO QCharRef TOO

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

QT_END_NAMESPACE

#endif // QCHAR_H
