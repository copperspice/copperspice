/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#include <qchar32.h>
#include <qstring8.h>
#include <qdatastream.h>
#include <qunicodetables_p.h>

// #include <qtextcodec.h>

#define FLAG(x) (1 << (x))

enum {
   Hangul_SBase  = 0xac00,
   Hangul_LBase  = 0x1100,
   Hangul_VBase  = 0x1161,
   Hangul_TBase  = 0x11a7,
   Hangul_SCount = 11172,
   Hangul_LCount = 19,
   Hangul_VCount = 21,
   Hangul_TCount = 28,
   Hangul_NCount = 21 * 28
};

// methods
QChar32::Category QChar32::category() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return QChar32::Other_NotAssigned;
   }

   return static_cast<QChar32::Category>(QUnicodeTables::properties(value)->category);
}

int QChar32::digitValue() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint)  {
      return -1;
   }

   return QUnicodeTables::properties(value)->digitValue;
}


QChar32::Direction QChar32::direction() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return QChar32::DirL;
   }

   return static_cast<QChar32::Direction>(QUnicodeTables::properties(value)->direction);
}

QChar32::JoiningType QChar32::joiningType() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return QChar32::Joining_None;
   }

   return static_cast<QChar32::JoiningType>(QUnicodeTables::properties(value)->joining);
}

bool QChar32::hasMirrored() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return false;
   }

   return QUnicodeTables::properties(value)->mirrorDiff != 0;
}

bool QChar32::isLetter() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return false;
   }

   const int test = FLAG(Letter_Uppercase) | FLAG(Letter_Lowercase) | FLAG(Letter_Titlecase) |
                    FLAG(Letter_Modifier)  | FLAG(Letter_Other);

   return FLAG(QUnicodeTables::properties(value)->category) & test;
}

bool QChar32::isLetterOrNumber() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return false;
   }

   const int test = FLAG(Letter_Uppercase) | FLAG(Letter_Lowercase) | FLAG(Letter_Titlecase) |
                    FLAG(Letter_Modifier)  | FLAG(Letter_Other)     | FLAG(Number_DecimalDigit) |
                    FLAG(Number_Letter)    | FLAG(Number_Other);

   return FLAG(QUnicodeTables::properties(value)->category) & test;
}

bool QChar32::isMark() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return false;
   }

   const int test = FLAG(Mark_NonSpacing) | FLAG(Mark_SpacingCombining) | FLAG(Mark_Enclosing);

   return FLAG(QUnicodeTables::properties(value)->category) & test;
}

bool QChar32::isNonCharacter() const
{
   uint32_t value = unicode();

   return value >= 0xfdd0 && (value <= 0xfdef || (value & 0xfffe) == 0xfffe);
}

bool QChar32::isNumber() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return false;
   }

   const int test = FLAG(Number_DecimalDigit) | FLAG(Number_Letter) | FLAG(Number_Other);

   return FLAG(QUnicodeTables::properties(value)->category) & test;
}

bool QChar32::isPrint() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return false;
   }

   const int test = FLAG(Other_Control)    | FLAG(Other_Format) | FLAG(Other_Surrogate) |
                    FLAG(Other_PrivateUse) | FLAG(Other_NotAssigned);

   return ! (FLAG(QUnicodeTables::properties(value)->category) & test);
}

bool QChar32::isPunct() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return false;
   }

   const int test = FLAG(Punctuation_Connector) | FLAG(Punctuation_Dash) | FLAG(Punctuation_Open) |
                    FLAG(Punctuation_Close) | FLAG(Punctuation_InitialQuote) | FLAG(Punctuation_FinalQuote) |
                    FLAG(Punctuation_Other);

   return FLAG(QUnicodeTables::properties(value)->category) & test;
}

bool QChar32::isSpace() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return false;
   }

   // note that [0x09..0x0d] + 0x85 are exceptional Cc-s and must be handled explicitly

   const int test = FLAG(Separator_Space) | FLAG(Separator_Line) | FLAG(Separator_Paragraph);

   return value == 0x20 || (value <= 0x0d && value >= 0x09) || (value > 127 && (value == 0x85 ||
                  value == 0xa0 ||  FLAG(QUnicodeTables::properties(value)->category) & test));
}

bool QChar32::isSymbol() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return false;
   }

   const int test = FLAG(Symbol_Math) | FLAG(Symbol_Currency) | FLAG(Symbol_Modifier) | FLAG(Symbol_Other);

   return FLAG(QUnicodeTables::properties(value)->category) & test;
}

QChar32 QChar32::mirroredChar() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return *this;
   }

   return static_cast<char32_t>(value + QUnicodeTables::properties(value)->mirrorDiff);
}

QString8 QChar32::toLower() const
{
   QString8 retval(*this);
   return retval.toLower();
}

QString8 QChar32::toUpper() const
{
   QString8 retval(*this);
   return retval.toUpper();
}

QString8 QChar32::toTitleCase() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint)  {
      return *this;
   }

   QString8 retval;

   const QUnicodeTables::Properties *prop = QUnicodeTables::properties(value);
   int32_t caseDiff = QUnicodeTables::TitlecaseTraits::caseDiff(prop);

   if (QUnicodeTables::TitlecaseTraits::caseSpecial(prop)) {
      const ushort *specialCase = QUnicodeTables::specialCaseMap + caseDiff;

      ushort length = *specialCase;
      ++specialCase;

      for (ushort cnt; cnt < length; ++cnt)  {
         retval += QChar32(specialCase[cnt]);
      }

   } else {
      retval += QChar32( static_cast<char32_t>(value + caseDiff) );

   }

   return retval;
}

QString8 QChar32::toCaseFolded() const
{
   QString8 retval(*this);
   return retval.toCaseFolded();
}

QChar32::UnicodeVersion QChar32::unicodeVersion() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return QChar32::Unicode_Unassigned;
   }

   return static_cast<QChar32::UnicodeVersion>(QUnicodeTables::properties(value)->unicodeVersion);
}

QChar32::UnicodeVersion QChar32::currentUnicodeVersion()
{
   return UNICODE_DATA_VERSION_32;
}


// operators

#if ! defined(QT_NO_DATASTREAM)
   QDataStream &operator>>(QDataStream &out, QChar32 &str)
   {
      // broom - not implemented
      return out;
   }

   QDataStream &operator<<(QDataStream &out, const QChar32 &str)
   {
      // broom - not implemented
      return out;
   }
#endif

