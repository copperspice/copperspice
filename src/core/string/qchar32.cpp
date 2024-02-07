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

#include <qchar32.h>

#include <qdatastream.h>
#include <qstring16.h>
#include <qstring8.h>

#include <qunicodetables_p.h>

#include "../locale/qunicodetables.cpp"      // do not change to < >

#define FLAG(x) (1 << (x))

// methods
QChar32::Category QChar32::category() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return QChar32::Other_NotAssigned;
   }

   return static_cast<QChar32::Category>(QUnicodeTables::properties(value)->category);
}

unsigned char QChar32::combiningClass() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return 0;
   }

   return static_cast<unsigned char>(QUnicodeTables::properties(value)->combiningClass);
}

// buffer has to have a length of 3, required for Hangul decomposition
static const char32_t * cs_internal_decomposition(uint value, int *length, int *tag, char32_t *buffer)
{
   if (value >= Hangul_Constants::Hangul_SBase && value < Hangul_Constants::Hangul_SBase + Hangul_Constants::Hangul_SCount) {
      // compute Hangul syllable decomposition as per UAX #15

      const uint SIndex = value - Hangul_Constants::Hangul_SBase;
      buffer[0] = Hangul_Constants::Hangul_LBase + SIndex / Hangul_Constants::Hangul_NCount;                                     // L
      buffer[1] = Hangul_Constants::Hangul_VBase + (SIndex % Hangul_Constants::Hangul_NCount) / Hangul_Constants::Hangul_TCount; // V
      buffer[2] = Hangul_Constants::Hangul_TBase + SIndex % Hangul_Constants::Hangul_TCount;                                     // T

      *length = buffer[2] == Hangul_Constants::Hangul_TBase ? 2 : 3;
      *tag = QChar32::Canonical;

      return buffer;
   }

   const unsigned short index = GET_DECOMPOSITION_INDEX(value);
   if (index == 0xffff) {
      *length = 0;
      *tag    = QChar32::NoDecomposition;

      return nullptr;
   }

   const char32_t *decomposition = QUnicodeTables::uc_decomposition_map + index;
   *tag    = (*decomposition) & 0xff;
   *length = (*decomposition) >> 8;

   return decomposition + 1;
}

QString8 QChar32::decomposition() const
{
   char32_t buffer[3];
   int length;
   int tag;

   uint32_t value = unicode();
   const char32_t *d = cs_internal_decomposition(value, &length, &tag, buffer);

   return QString8(d, d + length);
}

QChar32::Decomposition QChar32::decompositionTag() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return QChar32::NoDecomposition;
   }

   const unsigned short index = GET_DECOMPOSITION_INDEX(value);

   if (index == 0xffff) {
      return QChar32::NoDecomposition;
   }

   return static_cast<QChar32::Decomposition>(QUnicodeTables::uc_decomposition_map[index] & 0xff);
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

QChar32::JoiningType QChar32::joiningType() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return QChar32::Joining_None;
   }

   return static_cast<QChar32::JoiningType>(QUnicodeTables::properties(value)->joining);
}

QChar32 QChar32::mirroredChar() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return *this;
   }

   return static_cast<char32_t>(value + QUnicodeTables::properties(value)->mirrorDiff);
}

QChar32::Script QChar32::script() const
{
   uint32_t value = unicode();

   if (value > LastValidCodePoint) {
      return QChar32::Script_Unknown;
   }

   return static_cast<QChar32::Script>(QUnicodeTables::properties(value)->script);
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
   char32_t caseValue = QUnicodeTables::TitleCaseTraits::caseValue(value);

   if (caseValue == 0 && value != 0) {
      // special char

      const char32_t *data = QUnicodeTables::TitleCaseTraits::caseSpecial(value);
      retval += data;

   } else {
      retval += QChar32(caseValue);
   }


   return retval;
}

QString8 QChar32::toCaseFolded() const
{
   QString8 retval(*this);
   return retval.toCaseFolded();
}

QString16 QChar32::toCaseFolded16() const
{
   QString16 retval(*this);
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
   return UNICODE_DATA_VERSION;
}

QDataStream &operator>>(QDataStream &stream, QChar32 &ch)
{
   quint32 tmp;
   stream >> tmp;

   ch = static_cast<char32_t>(tmp);

   return stream;
}

QDataStream &operator<<(QDataStream &stream, const QChar32 &ch)
{
   stream << static_cast<quint32>(ch.unicode());
   return stream;
}

