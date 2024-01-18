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

/*
 *  Copyright (C) 2006 George Staikos <staikos@kde.org>
 *  Copyright (C) 2006 Alexey Proskuryakov <ap@nypop.com>
 */

#ifndef WTF_UNICODE_QT4_H
#define WTF_UNICODE_QT4_H

#include "UnicodeMacrosFromICU.h"

#include <qchar.h>
#include <qstring8.h>
#include <qstring16.h>

#include <config.h>

#include <stdint.h>
#if USE(QT_ICU_TEXT_BREAKING)
#include <unicode/ubrk.h>
#endif

namespace QUnicodeTables {
    struct Properties {
        ushort category : 8;
        ushort line_break_class : 8;
        ushort direction : 8;
        ushort combiningClass :8;
        ushort joining : 2;
        signed short digitValue : 6; /* 5 needed */
        ushort unicodeVersion : 4;
        ushort lowerCaseSpecial : 1;
        ushort upperCaseSpecial : 1;
        ushort titleCaseSpecial : 1;
        ushort caseFoldSpecial : 1; /* currently unused */
        signed short mirrorDiff : 16;
        signed short lowerCaseDiff : 16;
        signed short upperCaseDiff : 16;
        signed short titleCaseDiff : 16;
        signed short caseFoldDiff : 16;
    };
    Q_CORE_EXPORT const Properties * QT_FASTCALL properties(uint ucs4);
    Q_CORE_EXPORT const Properties * QT_FASTCALL properties(ushort ucs2);
}

// ugly hack to make UChar compatible with JSChar in API/JSStringRef.h
#if defined(Q_OS_WIN) || (COMPILER(RVCT) && ! OS(LINUX))
typedef wchar_t UChar;
#else
typedef uint16_t UChar;
#endif

#if !USE(QT_ICU_TEXT_BREAKING)
typedef uint32_t UChar32;
#endif

namespace WTF {
namespace Unicode {

enum Direction {
    LeftToRight = QChar::DirL,
    RightToLeft = QChar::DirR,
    EuropeanNumber = QChar::DirEN,
    EuropeanNumberSeparator = QChar::DirES,
    EuropeanNumberTerminator = QChar::DirET,
    ArabicNumber = QChar::DirAN,
    CommonNumberSeparator = QChar::DirCS,
    BlockSeparator = QChar::DirB,
    SegmentSeparator = QChar::DirS,
    WhiteSpaceNeutral = QChar::DirWS,
    OtherNeutral = QChar::DirON,
    LeftToRightEmbedding = QChar::DirLRE,
    LeftToRightOverride = QChar::DirLRO,
    RightToLeftArabic = QChar::DirAL,
    RightToLeftEmbedding = QChar::DirRLE,
    RightToLeftOverride = QChar::DirRLO,
    PopDirectionalFormat = QChar::DirPDF,
    NonSpacingMark = QChar::DirNSM,
    BoundaryNeutral = QChar::DirBN
};

enum DecompositionType {
    DecompositionNone = QChar::NoDecomposition,
    DecompositionCanonical = QChar::Canonical,
    DecompositionCompat = QChar::Compat,
    DecompositionCircle = QChar::Circle,
    DecompositionFinal = QChar::Final,
    DecompositionFont = QChar::Font,
    DecompositionFraction = QChar::Fraction,
    DecompositionInitial = QChar::Initial,
    DecompositionIsolated = QChar::Isolated,
    DecompositionMedial = QChar::Medial,
    DecompositionNarrow = QChar::Narrow,
    DecompositionNoBreak = QChar::NoBreak,
    DecompositionSmall = QChar::Small,
    DecompositionSquare = QChar::Square,
    DecompositionSub = QChar::Sub,
    DecompositionSuper = QChar::Super,
    DecompositionVertical = QChar::Vertical,
    DecompositionWide = QChar::Wide
};

enum CharCategory {
    NoCategory = 0,
    Mark_NonSpacing = U_MASK(QChar::Mark_NonSpacing),
    Mark_SpacingCombining = U_MASK(QChar::Mark_SpacingCombining),
    Mark_Enclosing = U_MASK(QChar::Mark_Enclosing),
    Number_DecimalDigit = U_MASK(QChar::Number_DecimalDigit),
    Number_Letter = U_MASK(QChar::Number_Letter),
    Number_Other = U_MASK(QChar::Number_Other),
    Separator_Space = U_MASK(QChar::Separator_Space),
    Separator_Line = U_MASK(QChar::Separator_Line),
    Separator_Paragraph = U_MASK(QChar::Separator_Paragraph),
    Other_Control = U_MASK(QChar::Other_Control),
    Other_Format = U_MASK(QChar::Other_Format),
    Other_Surrogate = U_MASK(QChar::Other_Surrogate),
    Other_PrivateUse = U_MASK(QChar::Other_PrivateUse),
    Other_NotAssigned = U_MASK(QChar::Other_NotAssigned),
    Letter_Uppercase = U_MASK(QChar::Letter_Uppercase),
    Letter_Lowercase = U_MASK(QChar::Letter_Lowercase),
    Letter_Titlecase = U_MASK(QChar::Letter_Titlecase),
    Letter_Modifier = U_MASK(QChar::Letter_Modifier),
    Letter_Other = U_MASK(QChar::Letter_Other),
    Punctuation_Connector = U_MASK(QChar::Punctuation_Connector),
    Punctuation_Dash = U_MASK(QChar::Punctuation_Dash),
    Punctuation_Open = U_MASK(QChar::Punctuation_Open),
    Punctuation_Close = U_MASK(QChar::Punctuation_Close),
    Punctuation_InitialQuote = U_MASK(QChar::Punctuation_InitialQuote),
    Punctuation_FinalQuote = U_MASK(QChar::Punctuation_FinalQuote),
    Punctuation_Other = U_MASK(QChar::Punctuation_Other),
    Symbol_Math = U_MASK(QChar::Symbol_Math),
    Symbol_Currency = U_MASK(QChar::Symbol_Currency),
    Symbol_Modifier = U_MASK(QChar::Symbol_Modifier),
    Symbol_Other = U_MASK(QChar::Symbol_Other)
};

inline UChar32 toLower(UChar32 c)
{
   return QChar(char32_t(c)).toLower()[0].unicode();
}

inline int toLower(UChar* result, int resultLength, const UChar* src, int srcLength,  bool* error)
{
   const UChar *e = src + srcLength;
   const UChar *s = src;
   UChar *r    = result;
   int rindex  = 0;

   // this avoids one out of bounds check in the loop
   if (s < e && (*s >= 0xDC00 && *s <= 0xDFFF)) {

      if (r) {
         r[rindex] = *s++;
      }

      ++rindex;
   }

   int needed = 0;

   while (s < e && (rindex < uint(resultLength) || ! r)) {
      uint c = *s;

      if (c >= 0xDC00 && c <= 0xDFFF) {
         // current char is a lowSurrogate
         uint prevC = *(s-1);

         if (prevC >= 0xD800 && prevC <= 0xDBFF) {
            // previous char is a highSurrogate
            c = (prevC & 0x03FF) << 10 | (c & 0x03FF) | 0x010000;
         }
      }

      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(c);

      if (prop->lowerCaseSpecial) {

         QString16 tmp = QChar(char32_t(c));
         tmp = tmp.toLower();

         for (int i = 0; i < tmp.size_storage(); ++i) {

            if (rindex >= resultLength) {
               needed += tmp.size_storage() - i;
               break;
            }

            if (r) {
               r[rindex] = tmp.constData()[i];
               ++rindex;
            }
         }

      } else if (r) {
         r[rindex] = *s + prop->lowerCaseDiff;
         ++rindex;
      }

      ++s;
   }

   if (s < e) {
      needed += e - s;
   }

   *error = (needed != 0);

   if (rindex < uint(resultLength)) {
      r[rindex] = 0;
   }

   return rindex + needed;
}

inline UChar32 toUpper(UChar32 c)
{
   return QChar(char32_t(c)).toUpper()[0].unicode();
}

inline int toUpper(UChar* result, int resultLength, const UChar* src, int srcLength,  bool* error)
{
   const UChar *e = src + srcLength;
   const UChar *s = src;
   UChar *r   = result;
   int rindex = 0;

   // this avoids one out of bounds check in the loop
   if (s < e && (*s >= 0xDC00 && *s <= 0xDFFF)) {

      if (r) {
         r[rindex] = *s++;
      }

      ++rindex;
   }

   int needed = 0;

   while (s < e && (rindex < resultLength || !r)) {
      uint c = *s;

      if (c >= 0xDC00 && c <= 0xDFFF) {
         // current char is a lowSurrogate
         uint prevC = *(s-1);

         if (prevC >= 0xD800 && prevC <= 0xDBFF) {
            // previous char is a highSurrogate
            c = (prevC & 0x03FF) << 10 | (c & 0x03FF) | 0x010000;
         }
      }

      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(c);

      if (prop->upperCaseSpecial) {
         QString16 tmp = QChar(char32_t(c));
         tmp = tmp.toUpper();

         for (int i = 0; i < tmp.size_storage(); ++i) {
            if (rindex >= resultLength) {
               needed += tmp.size_storage() - i;
               break;
            }

            if (r) {
               r[rindex] = tmp.constData()[i];
               ++rindex;
            }
         }

      } else if (r) {
         r[rindex] = *s + prop->upperCaseDiff;
         ++rindex;
      }

      ++s;
   }

   if (s < e) {
      needed += e - s;
   }

   *error = (needed != 0);
   if (rindex < resultLength) {
      r[rindex] = 0;
   }

   return rindex + needed;
}

inline int toTitleCase(UChar32 c)
{
   return QChar(char32_t(c)).toTitleCase()[0].unicode();
}

inline UChar32 foldCase(UChar32 c)
{
   return QChar(char32_t(c)).toCaseFolded()[0].unicode();
}

inline int foldCase(UChar *result, int resultLength, const UChar *src, int srcLength, bool *error)
{
   *error = false;

   if (resultLength < srcLength) {
      *error = true;
      return srcLength;
   }

   for (int i = 0; i < srcLength; ++i) {
      QChar tmp = char32_t(src[i]);
      result[i] = tmp.toCaseFolded()[0].unicode();
   }

   return srcLength;
}

inline bool isArabicChar(UChar32 c)
{
   return c >= 0x0600 && c <= 0x06FF;
}

inline bool isPrintableChar(UChar32 c)
{
   return QChar(char32_t(c)).isPrint();
}

inline bool isSeparatorSpace(UChar32 c)
{
   return QChar(char32_t(c)).category() == QChar::Separator_Space;
}

inline bool isPunct(UChar32 c)
{
   return QChar(char32_t(c)).isPunct();
}

inline bool isLower(UChar32 c)
{
   return QChar(char32_t(c)).isLower();
}

inline bool hasLineBreakingPropertyComplexContext(UChar32)
{
   // Implement this to return whether the character has line breaking property SA (Complex Context).
   return false;
}

inline UChar32 mirroredChar(UChar32 c)
{
   return QChar(char32_t(c)).mirroredChar().unicode();
}

inline uint8_t combiningClass(UChar32 c)
{
   return QChar(char32_t(c)).combiningClass();
}

inline DecompositionType decompositionType(UChar32 c)
{
   return (DecompositionType)QChar(char32_t(c)).decompositionTag();
}

inline int umemcasecmp(const UChar *a, const UChar *b, int len)
{
   for (int i = 0; i < len; ++i) {
       uint c1 = QChar(char32_t(a[i])).toCaseFolded()[0].unicode();
       uint c2 = QChar(char32_t(b[i])).toCaseFolded()[0].unicode();

      if (c1 != c2) {
         return c1 - c2;
      }
   }

   return 0;
}

inline Direction direction(UChar32 c)
{
   return (Direction)QChar(char32_t(c)).direction();
}

inline CharCategory category(UChar32 c)
{
   return (CharCategory)  U_MASK(QChar(char32_t(c)).category());
}

} }

#endif
