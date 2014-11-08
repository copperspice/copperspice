/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#include <qchar.h>
#include <qdatastream.h>
#include <qtextcodec.h>
#include <qunicodetables_p.h>
#include "qunicodetables.cpp"         // do not change to < > 

QT_BEGIN_NAMESPACE

#ifndef QT_NO_CODEC_FOR_C_STRINGS
#  ifdef QT_NO_TEXTCODEC
#    define QT_NO_CODEC_FOR_C_STRINGS
#  endif
#endif

#define FLAG(x) (1 << (x))

bool QChar::isPrint() const
{
   const int test = FLAG(Other_Control) |
                    FLAG(Other_NotAssigned);
   return !(FLAG(qGetProp(ucs)->category) & test);
}

/*!
    Returns true if the character is a separator character
    (Separator_* categories); otherwise returns false.
*/
bool QChar::isSpace() const
{
   if (ucs >= 9 && ucs <= 13) {
      return true;
   }
   const int test = FLAG(Separator_Space) |
                    FLAG(Separator_Line) |
                    FLAG(Separator_Paragraph);
   return FLAG(qGetProp(ucs)->category) & test;
}

/*!
    Returns true if the character is a mark (Mark_* categories);
    otherwise returns false.

    See QChar::Category for more information regarding marks.
*/
bool QChar::isMark() const
{
   const int test = FLAG(Mark_NonSpacing) |
                    FLAG(Mark_SpacingCombining) |
                    FLAG(Mark_Enclosing);
   return FLAG(qGetProp(ucs)->category) & test;
}

/*!
    Returns true if the character is a punctuation mark (Punctuation_*
    categories); otherwise returns false.
*/
bool QChar::isPunct() const
{
   const int test = FLAG(Punctuation_Connector) |
                    FLAG(Punctuation_Dash) |
                    FLAG(Punctuation_Open) |
                    FLAG(Punctuation_Close) |
                    FLAG(Punctuation_InitialQuote) |
                    FLAG(Punctuation_FinalQuote) |
                    FLAG(Punctuation_Other);
   return FLAG(qGetProp(ucs)->category) & test;
}

/*!
    Returns true if the character is a letter (Letter_* categories);
    otherwise returns false.
*/
bool QChar::isLetter() const
{
   const int test = FLAG(Letter_Uppercase) |
                    FLAG(Letter_Lowercase) |
                    FLAG(Letter_Titlecase) |
                    FLAG(Letter_Modifier) |
                    FLAG(Letter_Other);
   return FLAG(qGetProp(ucs)->category) & test;
}

/*!
    Returns true if the character is a number (Number_* categories,
    not just 0-9); otherwise returns false.

    \sa isDigit()
*/
bool QChar::isNumber() const
{
   const int test = FLAG(Number_DecimalDigit) |
                    FLAG(Number_Letter) |
                    FLAG(Number_Other);
   return FLAG(qGetProp(ucs)->category) & test;
}

/*!
    Returns true if the character is a letter or number (Letter_* or
    Number_* categories); otherwise returns false.
*/
bool QChar::isLetterOrNumber() const
{
   const int test = FLAG(Letter_Uppercase) |
                    FLAG(Letter_Lowercase) |
                    FLAG(Letter_Titlecase) |
                    FLAG(Letter_Modifier) |
                    FLAG(Letter_Other) |
                    FLAG(Number_DecimalDigit) |
                    FLAG(Number_Letter) |
                    FLAG(Number_Other);
   return FLAG(qGetProp(ucs)->category) & test;
}


/*!
    Returns true if the character is a decimal digit
    (Number_DecimalDigit); otherwise returns false.
*/
bool QChar::isDigit() const
{
   return (qGetProp(ucs)->category == Number_DecimalDigit);
}


/*!
    Returns true if the character is a symbol (Symbol_* categories);
    otherwise returns false.
*/
bool QChar::isSymbol() const
{
   const int test = FLAG(Symbol_Math) |
                    FLAG(Symbol_Currency) |
                    FLAG(Symbol_Modifier) |
                    FLAG(Symbol_Other);
   return FLAG(qGetProp(ucs)->category) & test;
}

int QChar::digitValue() const
{
   return qGetProp(ucs)->digitValue;
}

/*!
    \overload
    Returns the numeric value of the digit, specified by the UCS-2-encoded
    character, \a ucs2, or -1 if the character is not a digit.
*/
int QChar::digitValue(ushort ucs2)
{
   return qGetProp(ucs2)->digitValue;
}

/*!
    \overload
    Returns the numeric value of the digit specified by the UCS-4-encoded
    character, \a ucs4, or -1 if the character is not a digit.
*/
int QChar::digitValue(uint ucs4)
{
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return 0;
   }
   return qGetProp(ucs4)->digitValue;
}

/*!
    Returns the character's category.
*/
QChar::Category QChar::category() const
{
   return (QChar::Category) qGetProp(ucs)->category;
}

/*!
    \overload
    \since 4.3
    Returns the category of the UCS-4-encoded character specified by \a ucs4.
*/
QChar::Category QChar::category(uint ucs4)
{
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return QChar::NoCategory;
   }
   return (QChar::Category) qGetProp(ucs4)->category;
}

/*!
    \overload
    Returns the category of the UCS-2-encoded character specified by \a ucs2.
*/
QChar::Category QChar::category(ushort ucs2)
{
   return (QChar::Category) qGetProp(ucs2)->category;
}


/*!
    Returns the character's direction.
*/
QChar::Direction QChar::direction() const
{
   return (QChar::Direction) qGetProp(ucs)->direction;
}

/*!
    \overload
    Returns the direction of the UCS-4-encoded character specified by \a ucs4.
*/
QChar::Direction QChar::direction(uint ucs4)
{
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return QChar::DirL;
   }
   return (QChar::Direction) qGetProp(ucs4)->direction;
}

/*!
    \overload
    Returns the direction of the UCS-2-encoded character specified by \a ucs2.
*/
QChar::Direction QChar::direction(ushort ucs2)
{
   return (QChar::Direction) qGetProp(ucs2)->direction;
}

/*!
    Returns information about the joining properties of the character
    (needed for certain languages such as Arabic).
*/
QChar::Joining QChar::joining() const
{
   return (QChar::Joining) qGetProp(ucs)->joining;
}

/*!
    \overload
    Returns information about the joining properties of the UCS-4-encoded
    character specified by \a ucs4 (needed for certain languages such as
    Arabic).
*/
QChar::Joining QChar::joining(uint ucs4)
{
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return QChar::OtherJoining;
   }
   return (QChar::Joining) qGetProp(ucs4)->joining;
}

/*!
    \overload
    Returns information about the joining properties of the UCS-2-encoded
    character specified by \a ucs2 (needed for certain languages such as
    Arabic).
*/
QChar::Joining QChar::joining(ushort ucs2)
{
   return (QChar::Joining) qGetProp(ucs2)->joining;
}


/*!
    Returns true if the character should be reversed if the text
    direction is reversed; otherwise returns false.

    Same as (ch.mirroredChar() != ch).

    \sa mirroredChar()
*/
bool QChar::hasMirrored() const
{
   return qGetProp(ucs)->mirrorDiff != 0;
}

/*!
    \fn bool QChar::isLower() const

    Returns true if the character is a lowercase letter, i.e.
    category() is Letter_Lowercase.

    \sa isUpper(), toLower(), toUpper()
*/

/*!
    \fn bool QChar::isUpper() const

    Returns true if the character is an uppercase letter, i.e.
    category() is Letter_Uppercase.

    \sa isLower(), toUpper(), toLower()
*/

/*!
    \fn bool QChar::isTitleCase() const
    \since 4.3

    Returns true if the character is a titlecase letter, i.e.
    category() is Letter_Titlecase.

    \sa isLower(), toUpper(), toLower(), toTitleCase()
*/

/*!
    Returns the mirrored character if this character is a mirrored
    character; otherwise returns the character itself.

    \sa hasMirrored()
*/
QChar QChar::mirroredChar() const
{
   return ucs + qGetProp(ucs)->mirrorDiff;
}

/*!
    \overload
    Returns the mirrored character if the UCS-4-encoded character specified
    by \a ucs4 is a mirrored character; otherwise returns the character itself.

    \sa hasMirrored()
*/
uint QChar::mirroredChar(uint ucs4)
{
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return ucs4;
   }
   return ucs4 + qGetProp(ucs4)->mirrorDiff;
}

/*!
    \overload
    Returns the mirrored character if the UCS-2-encoded character specified
    by \a ucs2 is a mirrored character; otherwise returns the character itself.

    \sa hasMirrored()
*/
ushort QChar::mirroredChar(ushort ucs2)
{
   return ucs2 + qGetProp(ucs2)->mirrorDiff;
}


enum {
   Hangul_SBase = 0xac00,
   Hangul_LBase = 0x1100,
   Hangul_VBase = 0x1161,
   Hangul_TBase = 0x11a7,
   Hangul_SCount = 11172,
   Hangul_LCount = 19,
   Hangul_VCount = 21,
   Hangul_TCount = 28,
   Hangul_NCount = 21 * 28
};

// buffer has to have a length of 3. It's needed for Hangul decomposition
static const unsigned short *QT_FASTCALL decompositionHelper
(uint ucs4, int *length, int *tag, unsigned short *buffer)
{
   *length = 0;
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return 0;
   }
   if (ucs4 >= Hangul_SBase && ucs4 < Hangul_SBase + Hangul_SCount) {
      int SIndex = ucs4 - Hangul_SBase;
      buffer[0] = Hangul_LBase + SIndex / Hangul_NCount; // L
      buffer[1] = Hangul_VBase + (SIndex % Hangul_NCount) / Hangul_TCount; // V
      buffer[2] = Hangul_TBase + SIndex % Hangul_TCount; // T
      *length = buffer[2] == Hangul_TBase ? 2 : 3;
      *tag = QChar::Canonical;
      return buffer;
   }

   const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
   if (index == 0xffff) {
      return 0;
   }
   const unsigned short *decomposition = uc_decomposition_map + index;
   *tag = (*decomposition) & 0xff;
   *length = (*decomposition) >> 8;
   return decomposition + 1;
}

/*!
    Decomposes a character into its parts. Returns an empty string if
    no decomposition exists.
*/
QString QChar::decomposition() const
{
   return decomposition(ucs);
}

/*!
    \overload
    Decomposes the UCS-4-encoded character specified by \a ucs4 into its
    constituent parts. Returns an empty string if no decomposition exists.
*/
QString QChar::decomposition(uint ucs4)
{
   unsigned short buffer[3];
   int length;
   int tag;
   const unsigned short *d = decompositionHelper(ucs4, &length, &tag, buffer);
   return QString::fromUtf16(d, length);
}

/*!
    Returns the tag defining the composition of the character. Returns
    QChar::Single if no decomposition exists.
*/
QChar::Decomposition QChar::decompositionTag() const
{
   return decompositionTag(ucs);
}

/*!
    \overload
    Returns the tag defining the composition of the UCS-4-encoded character
    specified by \a ucs4. Returns QChar::Single if no decomposition exists.
*/
QChar::Decomposition QChar::decompositionTag(uint ucs4)
{
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return QChar::NoDecomposition;
   }
   const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
   if (index == 0xffff) {
      return QChar::NoDecomposition;
   }
   return (QChar::Decomposition)(uc_decomposition_map[index] & 0xff);
}

/*!
    Returns the combining class for the character as defined in the
    Unicode standard. This is mainly useful as a positioning hint for
    marks attached to a base character.

    The Qt text rendering engine uses this information to correctly
    position non-spacing marks around a base character.
*/
unsigned char QChar::combiningClass() const
{
   return (unsigned char) qGetProp(ucs)->combiningClass;
}

/*!
    \overload
    Returns the combining class for the UCS-4-encoded character specified by
    \a ucs4, as defined in the Unicode standard.
*/
unsigned char QChar::combiningClass(uint ucs4)
{
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return 0;
   }
   return (unsigned char) qGetProp(ucs4)->combiningClass;
}

/*!
    \overload
    Returns the combining class for the UCS-2-encoded character specified by
    \a ucs2, as defined in the Unicode standard.
*/
unsigned char QChar::combiningClass(ushort ucs2)
{
   return (unsigned char) qGetProp(ucs2)->combiningClass;
}

/*!
    Returns the Unicode version that introduced this character.
*/
QChar::UnicodeVersion QChar::unicodeVersion() const
{
   return (QChar::UnicodeVersion) qGetProp(ucs)->unicodeVersion;
}

/*!
    \overload
    Returns the Unicode version that introduced the character specified in
    its UCS-4-encoded form as \a ucs4.
*/
QChar::UnicodeVersion QChar::unicodeVersion(uint ucs4)
{
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return QChar::Unicode_Unassigned;
   }
   return (QChar::UnicodeVersion) qGetProp(ucs4)->unicodeVersion;
}

/*!
    \overload
    Returns the Unicode version that introduced the character specified in
    its UCS-2-encoded form as \a ucs2.
*/
QChar::UnicodeVersion QChar::unicodeVersion(ushort ucs2)
{
   return (QChar::UnicodeVersion) qGetProp(ucs2)->unicodeVersion;
}

/*!
    \since 4.8

    Returns the most recent supported Unicode version.
*/
QChar::UnicodeVersion QChar::currentUnicodeVersion()
{
   return UNICODE_DATA_VERSION;
}

/*!
    Returns the lowercase equivalent if the character is uppercase or titlecase;
    otherwise returns the character itself.
*/
QChar QChar::toLower() const
{
   const QUnicodeTables::Properties *p = qGetProp(ucs);
   if (!p->lowerCaseSpecial) {
      return ucs + p->lowerCaseDiff;
   }
   return ucs;
}

/*!
    \overload
    Returns the lowercase equivalent of the UCS-4-encoded character specified
    by \a ucs4 if the character is uppercase or titlecase; otherwise returns
    the character itself.
*/
uint QChar::toLower(uint ucs4)
{
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return ucs4;
   }
   const QUnicodeTables::Properties *p = qGetProp(ucs4);
   if (!p->lowerCaseSpecial) {
      return ucs4 + p->lowerCaseDiff;
   }
   return ucs4;
}

/*!
    \overload
    Returns the lowercase equivalent of the UCS-2-encoded character specified
    by \a ucs2 if the character is uppercase or titlecase; otherwise returns
    the character itself.
*/
ushort QChar::toLower(ushort ucs2)
{
   const QUnicodeTables::Properties *p = qGetProp(ucs2);
   if (!p->lowerCaseSpecial) {
      return ucs2 + p->lowerCaseDiff;
   }
   return ucs2;
}

/*!
    Returns the uppercase equivalent if the character is lowercase or titlecase;
    otherwise returns the character itself.
*/
QChar QChar::toUpper() const
{
   const QUnicodeTables::Properties *p = qGetProp(ucs);
   if (!p->upperCaseSpecial) {
      return ucs + p->upperCaseDiff;
   }
   return ucs;
}

/*!
    \overload
    Returns the uppercase equivalent of the UCS-4-encoded character specified
    by \a ucs4 if the character is lowercase or titlecase; otherwise returns
    the character itself.
*/
uint QChar::toUpper(uint ucs4)
{
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return ucs4;
   }
   const QUnicodeTables::Properties *p = qGetProp(ucs4);
   if (!p->upperCaseSpecial) {
      return ucs4 + p->upperCaseDiff;
   }
   return ucs4;
}

/*!
    \overload
    Returns the uppercase equivalent of the UCS-2-encoded character specified
    by \a ucs2 if the character is lowercase or titlecase; otherwise returns
    the character itself.
*/
ushort QChar::toUpper(ushort ucs2)
{
   const QUnicodeTables::Properties *p = qGetProp(ucs2);
   if (!p->upperCaseSpecial) {
      return ucs2 + p->upperCaseDiff;
   }
   return ucs2;
}

/*!
    Returns the title case equivalent if the character is lowercase or uppercase;
    otherwise returns the character itself.
*/
QChar QChar::toTitleCase() const
{
   const QUnicodeTables::Properties *p = qGetProp(ucs);
   if (!p->titleCaseSpecial) {
      return ucs + p->titleCaseDiff;
   }
   return ucs;
}

/*!
    \overload
    Returns the title case equivalent of the UCS-4-encoded character specified
    by \a ucs4 if the character is lowercase or uppercase; otherwise returns
    the character itself.
*/
uint QChar::toTitleCase(uint ucs4)
{
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return ucs4;
   }
   const QUnicodeTables::Properties *p = qGetProp(ucs4);
   if (!p->titleCaseSpecial) {
      return ucs4 + p->titleCaseDiff;
   }
   return ucs4;
}

/*!
    \overload
    Returns the title case equivalent of the UCS-2-encoded character specified
    by \a ucs2 if the character is lowercase or uppercase; otherwise returns
    the character itself.
*/
ushort QChar::toTitleCase(ushort ucs2)
{
   const QUnicodeTables::Properties *p = qGetProp(ucs2);
   if (!p->titleCaseSpecial) {
      return ucs2 + p->titleCaseDiff;
   }
   return ucs2;
}


static inline uint foldCase(const ushort *ch, const ushort *start)
{
   uint c = *ch;
   if (QChar(c).isLowSurrogate() && ch > start && QChar(*(ch - 1)).isHighSurrogate()) {
      c = QChar::surrogateToUcs4(*(ch - 1), c);
   }
   return *ch + qGetProp(c)->caseFoldDiff;
}

static inline uint foldCase(uint ch, uint &last)
{
   uint c = ch;
   if (QChar(c).isLowSurrogate() && QChar(last).isHighSurrogate()) {
      c = QChar::surrogateToUcs4(last, c);
   }
   last = ch;
   return ch + qGetProp(c)->caseFoldDiff;
}

static inline ushort foldCase(ushort ch)
{
   return ch + qGetProp(ch)->caseFoldDiff;
}

/*!
    Returns the case folded equivalent of the character. For most Unicode characters this
    is the same as toLowerCase().
*/
QChar QChar::toCaseFolded() const
{
   return ucs + qGetProp(ucs)->caseFoldDiff;
}

/*!
    \overload
    Returns the case folded equivalent of the UCS-4-encoded character specified
    by \a ucs4. For most Unicode characters this is the same as toLowerCase().
*/
uint QChar::toCaseFolded(uint ucs4)
{
   if (ucs4 > UNICODE_LAST_CODEPOINT) {
      return ucs4;
   }
   return ucs4 + qGetProp(ucs4)->caseFoldDiff;
}

/*!
    \overload
    Returns the case folded equivalent of the UCS-2-encoded character specified
    by \a ucs2. For most Unicode characters this is the same as toLowerCase().
*/
ushort QChar::toCaseFolded(ushort ucs2)
{
   return ucs2 + qGetProp(ucs2)->caseFoldDiff;
}


/*!
    \fn char QChar::latin1() const

    Use toLatin1() instead.
*/

/*!
    \fn char QChar::ascii() const

    Use toAscii() instead.
*/

/*!
    \fn char QChar::toLatin1() const

    Returns the Latin-1 character equivalent to the QChar, or 0. This
    is mainly useful for non-internationalized software.

    \sa toAscii(), unicode()
*/

/*!
    \fn char QChar::toAscii() const

    Returns the Latin-1 character value of the QChar, or 0 if the character is not
    representable.

    The main purpose of this function is to preserve ASCII characters used
    in C strings. This is mainly useful for developers of non-internationalized
    software.

    \note It is not possible to distinguish a non-Latin 1 character from an ASCII 0
    (NUL) character. Prefer to use unicode(), which does not have this ambiguity.

    \sa toLatin1(), unicode()
*/

/*!
    \fn QChar QChar::fromAscii(char)

    Converts the ASCII character \a c to it's equivalent QChar. This
    is mainly useful for non-internationalized software.

    An alternative is to use QLatin1Char.

    \sa fromLatin1(), unicode()
*/

#ifndef QT_NO_DATASTREAM
/*!
    \relates QChar

    Writes the char \a chr to the stream \a out.

    \sa {Serializing Qt Data Types}
*/
QDataStream &operator<<(QDataStream &out, QChar chr)
{
   out << quint16(chr.unicode());
   return out;
}

/*!
    \relates QChar

    Reads a char from the stream \a in into char \a chr.

    \sa {Serializing Qt Data Types}
*/
QDataStream &operator>>(QDataStream &in, QChar &chr)
{
   quint16 u;
   in >> u;
   chr.unicode() = ushort(u);
   return in;
}
#endif // QT_NO_DATASTREAM

/*!
    \fn ushort & QChar::unicode()

    Returns a reference to the numeric Unicode value of the QChar.
*/

/*!
    \fn ushort QChar::unicode() const

    \overload
*/

/*****************************************************************************
  Documentation of QChar related functions
 *****************************************************************************/

/*!
    \fn bool operator==(QChar c1, QChar c2)

    \relates QChar

    Returns true if \a c1 and \a c2 are the same Unicode character;
    otherwise returns false.
*/

/*!
    \fn int operator!=(QChar c1, QChar c2)

    \relates QChar

    Returns true if \a c1 and \a c2 are not the same Unicode
    character; otherwise returns false.
*/

/*!
    \fn int operator<=(QChar c1, QChar c2)

    \relates QChar

    Returns true if the numeric Unicode value of \a c1 is less than
    or equal to that of \a c2; otherwise returns false.
*/

/*!
    \fn int operator>=(QChar c1, QChar c2)

    \relates QChar

    Returns true if the numeric Unicode value of \a c1 is greater than
    or equal to that of \a c2; otherwise returns false.
*/

/*!
    \fn int operator<(QChar c1, QChar c2)

    \relates QChar

    Returns true if the numeric Unicode value of \a c1 is less than
    that of \a c2; otherwise returns false.
*/

/*!
    \fn int operator>(QChar c1, QChar c2)

    \relates QChar

    Returns true if the numeric Unicode value of \a c1 is greater than
    that of \a c2; otherwise returns false.
*/

/*!
    \fn bool QChar::mirrored() const

    Use hasMirrored() instead.
*/

/*!
    \fn QChar QChar::lower() const

    Use toLower() instead.
*/

/*!
    \fn QChar QChar::upper() const

    Use toUpper() instead.
*/

/*!
    \fn bool QChar::networkOrdered()

    See if QSysInfo::ByteOrder == QSysInfo::BigEndian instead.
*/


// ---------------------------------------------------------------------------


static void decomposeHelper(QString *str, bool canonical, QChar::UnicodeVersion version, int from)
{
   unsigned short buffer[3];

   QString &s = *str;

   const unsigned short *utf16 = reinterpret_cast<unsigned short *>(s.data());
   const unsigned short *uc = utf16 + s.length();
   while (uc != utf16 + from) {
      uint ucs4 = *(--uc);
      if (QChar(ucs4).isLowSurrogate() && uc != utf16) {
         ushort high = *(uc - 1);
         if (QChar(high).isHighSurrogate()) {
            --uc;
            ucs4 = QChar::surrogateToUcs4(high, ucs4);
         }
      }
      QChar::UnicodeVersion v = QChar::unicodeVersion(ucs4);
      if (v == QChar::Unicode_Unassigned || v > version) {
         continue;
      }
      int length;
      int tag;
      const unsigned short *d = decompositionHelper(ucs4, &length, &tag, buffer);
      if (!d || (canonical && tag != QChar::Canonical)) {
         continue;
      }

      int pos = uc - utf16;
      s.replace(pos, QChar::requiresSurrogates(ucs4) ? 2 : 1, reinterpret_cast<const QChar *>(d), length);
      // since the insert invalidates the pointers and we do decomposition recursive
      utf16 = reinterpret_cast<unsigned short *>(s.data());
      uc = utf16 + pos + length;
   }
}


struct UCS2Pair {
   ushort u1;
   ushort u2;
};

inline bool operator<(ushort u1, const UCS2Pair &ligature)
{
   return u1 < ligature.u1;
}
inline bool operator<(const UCS2Pair &ligature, ushort u1)
{
   return ligature.u1 < u1;
}

static ushort ligatureHelper(ushort u1, ushort u2)
{
   // hangul L-V pair
   int LIndex = u1 - Hangul_LBase;
   if (0 <= LIndex && LIndex < Hangul_LCount) {
      int VIndex = u2 - Hangul_VBase;
      if (0 <= VIndex && VIndex < Hangul_VCount) {
         return Hangul_SBase + (LIndex * Hangul_VCount + VIndex) * Hangul_TCount;
      }
   }

   // hangul LV-T pair
   int SIndex = u1 - Hangul_SBase;
   if (0 <= SIndex && SIndex < Hangul_SCount && (SIndex % Hangul_TCount) == 0) {
      int TIndex = u2 - Hangul_TBase;
      if (0 <= TIndex && TIndex <= Hangul_TCount) {
         return u1 + TIndex;
      }
   }

   const unsigned short index = GET_LIGATURE_INDEX(u2);
   if (index == 0xffff) {
      return 0;
   }
   const unsigned short *ligatures = uc_ligature_map + index;
   ushort length = *ligatures++;
   {
      const UCS2Pair *data = reinterpret_cast<const UCS2Pair *>(ligatures);
      const UCS2Pair *r = qBinaryFind(data, data + length, u1);
      if (r != data + length) {
         return r->u2;
      }
   }

   return 0;
}

static void composeHelper(QString *str, QChar::UnicodeVersion version, int from)
{
   QString &s = *str;

   if (from < 0 || s.length() - from < 2) {
      return;
   }

   // the loop can partly ignore high Unicode as all ligatures are in the BMP
   int starter = -2; // to prevent starter == pos - 1
   int lastCombining = 255; // to prevent combining > lastCombining
   int pos = from;
   while (pos < s.length()) {
      uint uc = s.at(pos).unicode();
      if (QChar(uc).isHighSurrogate() && pos < s.length() - 1) {
         ushort low = s.at(pos + 1).unicode();
         if (QChar(low).isLowSurrogate()) {
            uc = QChar::surrogateToUcs4(uc, low);
            ++pos;
         }
      }
      const QUnicodeTables::Properties *p = qGetProp(uc);
      if (p->unicodeVersion == QChar::Unicode_Unassigned || p->unicodeVersion > version) {
         starter = -1; // to prevent starter == pos - 1
         lastCombining = 255; // to prevent combining > lastCombining
         ++pos;
         continue;
      }
      int combining = p->combiningClass;
      if ((starter == pos - 1 || combining > lastCombining) && starter >= from) {
         // allowed to form ligature with S
         QChar ligature = ligatureHelper(s.at(starter).unicode(), uc);
         if (ligature.unicode()) {
            s[starter] = ligature;
            s.remove(pos, 1);
            continue;
         }
      }
      if (!combining) {
         starter = pos;
      }
      lastCombining = combining;
      ++pos;
   }
}


static void canonicalOrderHelper(QString *str, QChar::UnicodeVersion version, int from)
{
   QString &s = *str;
   const int l = s.length() - 1;
   int pos = from;
   while (pos < l) {
      int p2 = pos + 1;
      uint u1 = s.at(pos).unicode();
      if (QChar(u1).isHighSurrogate()) {
         ushort low = s.at(p2).unicode();
         if (QChar(low).isLowSurrogate()) {
            u1 = QChar::surrogateToUcs4(u1, low);
            if (p2 >= l) {
               break;
            }
            ++p2;
         }
      }
      uint u2 = s.at(p2).unicode();
      if (QChar(u2).isHighSurrogate() && p2 < l) {
         ushort low = s.at(p2 + 1).unicode();
         if (QChar(low).isLowSurrogate()) {
            u2 = QChar::surrogateToUcs4(u2, low);
            ++p2;
         }
      }

      ushort c2 = 0;
      {
         const QUnicodeTables::Properties *p = qGetProp(u2);
         if (p->unicodeVersion != QChar::Unicode_Unassigned && p->unicodeVersion <= version) {
            c2 = p->combiningClass;
         }
      }
      if (c2 == 0) {
         pos = p2 + 1;
         continue;
      }

      ushort c1 = 0;
      {
         const QUnicodeTables::Properties *p = qGetProp(u1);
         if (p->unicodeVersion != QChar::Unicode_Unassigned && p->unicodeVersion <= version) {
            c1 = p->combiningClass;
         }
      }

      if (c1 > c2) {
         QChar *uc = s.data();
         int p = pos;
         // exchange characters
         if (!QChar::requiresSurrogates(u2)) {
            uc[p++] = u2;
         } else {
            uc[p++] = QChar::highSurrogate(u2);
            uc[p++] = QChar::lowSurrogate(u2);
         }
         if (!QChar::requiresSurrogates(u1)) {
            uc[p++] = u1;
         } else {
            uc[p++] = QChar::highSurrogate(u1);
            uc[p++] = QChar::lowSurrogate(u1);
         }
         if (pos > 0) {
            --pos;
         }
         if (pos > 0 && s.at(pos).isLowSurrogate()) {
            --pos;
         }
      } else {
         ++pos;
         if (QChar::requiresSurrogates(u1)) {
            ++pos;
         }
      }
   }
}

QT_END_NAMESPACE
