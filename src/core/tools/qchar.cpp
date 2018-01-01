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
   const int test = FLAG(Other_Control) | FLAG(Other_NotAssigned);
   return ! (FLAG(QUnicodeTables::qGetProp(ucs)->category) & test);
}

bool QChar::isSpace() const
{
   if (ucs >= 9 && ucs <= 13) {
      return true;
   }

   const int test = FLAG(Separator_Space) | FLAG(Separator_Line) | FLAG(Separator_Paragraph);
   return FLAG(QUnicodeTables::qGetProp(ucs)->category) & test;
}

bool QChar::isMark() const
{
   const int test = FLAG(Mark_NonSpacing) | FLAG(Mark_SpacingCombining) | FLAG(Mark_Enclosing);
   return FLAG(QUnicodeTables::qGetProp(ucs)->category) & test;
}

bool QChar::isPunct() const
{
   const int test = FLAG(Punctuation_Connector) |
                    FLAG(Punctuation_Dash) |
                    FLAG(Punctuation_Open) |
                    FLAG(Punctuation_Close) |
                    FLAG(Punctuation_InitialQuote) |
                    FLAG(Punctuation_FinalQuote) |
                    FLAG(Punctuation_Other);

   return FLAG(QUnicodeTables::qGetProp(ucs)->category) & test;
}

bool QChar::isLetter() const
{
   const int test = FLAG(Letter_Uppercase) |
                    FLAG(Letter_Lowercase) |
                    FLAG(Letter_Titlecase) |
                    FLAG(Letter_Modifier) |
                    FLAG(Letter_Other);

   return FLAG(QUnicodeTables::qGetProp(ucs)->category) & test;
}

bool QChar::isNumber() const
{
   const int test = FLAG(Number_DecimalDigit) |
                    FLAG(Number_Letter) |
                    FLAG(Number_Other);

   return FLAG(QUnicodeTables::qGetProp(ucs)->category) & test;
}

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

   return FLAG(QUnicodeTables::qGetProp(ucs)->category) & test;
}


bool QChar::isDigit() const
{
   return (QUnicodeTables::qGetProp(ucs)->category == Number_DecimalDigit);
}


bool QChar::isSymbol() const
{
   const int test = FLAG(Symbol_Math) |
                    FLAG(Symbol_Currency) |
                    FLAG(Symbol_Modifier) |
                    FLAG(Symbol_Other);

   return FLAG(QUnicodeTables::qGetProp(ucs)->category) & test;
}

int QChar::digitValue() const
{
   return QUnicodeTables::qGetProp(ucs)->digitValue;
}

int QChar::digitValue(ushort ucs2)
{
   return QUnicodeTables::qGetProp(ucs2)->digitValue;
}

int QChar::digitValue(uint ucs4)
{
   if (ucs4 > LastValidCodePoint) {
      return 0;
   }

   return QUnicodeTables::qGetProp(ucs4)->digitValue;
}

QChar::Category QChar::category() const
{
   return (QChar::Category) QUnicodeTables::qGetProp(ucs)->category;
}

QChar::Category QChar::category(uint ucs4)
{
   if (ucs4 > LastValidCodePoint) {
      return QChar::Other_NotAssigned;
   }

   return (QChar::Category) QUnicodeTables::qGetProp(ucs4)->category;
}

/*!
    \overload
    Returns the category of the UCS-2-encoded character specified by \a ucs2.
*/
QChar::Category QChar::category(ushort ucs2)
{
   return (QChar::Category) QUnicodeTables::qGetProp(ucs2)->category;
}


/*!
    Returns the character's direction.
*/
QChar::Direction QChar::direction() const
{
   return (QChar::Direction) QUnicodeTables::qGetProp(ucs)->direction;
}

/*!
    \overload
    Returns the direction of the UCS-4-encoded character specified by \a ucs4.
*/
QChar::Direction QChar::direction(uint ucs4)
{
   if (ucs4 > LastValidCodePoint) {
      return QChar::DirL;
   }
   return (QChar::Direction) QUnicodeTables::qGetProp(ucs4)->direction;
}

/*!
    \overload
    Returns the direction of the UCS-2-encoded character specified by \a ucs2.
*/
QChar::Direction QChar::direction(ushort ucs2)
{
   return (QChar::Direction) QUnicodeTables::qGetProp(ucs2)->direction;
}

/*!
    Returns information about the joining properties of the character
    (needed for certain languages such as Arabic).
*/
QChar::Joining QChar::joining() const
{
   return (QChar::Joining) QUnicodeTables::qGetProp(ucs)->joining;
}

/*!
    \overload
    Returns information about the joining properties of the UCS-4-encoded
    character specified by \a ucs4 (needed for certain languages such as
    Arabic).
*/
QChar::Joining QChar::joining(uint ucs4)
{
   if (ucs4 > LastValidCodePoint) {
      return QChar::OtherJoining;
   }

   return (QChar::Joining) QUnicodeTables::qGetProp(ucs4)->joining;
}

/*!
    \overload
    Returns information about the joining properties of the UCS-2-encoded
    character specified by \a ucs2 (needed for certain languages such as
    Arabic).
*/
QChar::Joining QChar::joining(ushort ucs2)
{
   return (QChar::Joining) QUnicodeTables::qGetProp(ucs2)->joining;
}


/*!
    Returns true if the character should be reversed if the text
    direction is reversed; otherwise returns false.

    Same as (ch.mirroredChar() != ch).

    \sa mirroredChar()
*/
bool QChar::hasMirrored() const
{
   return QUnicodeTables::qGetProp(ucs)->mirrorDiff != 0;
}

QChar QChar::mirroredChar() const
{
   return ucs + QUnicodeTables::qGetProp(ucs)->mirrorDiff;
}

/*!
    \overload
    Returns the mirrored character if the UCS-4-encoded character specified
    by \a ucs4 is a mirrored character; otherwise returns the character itself.

    \sa hasMirrored()
*/
uint QChar::mirroredChar(uint ucs4)
{
   if (ucs4 > LastValidCodePoint) {
      return ucs4;
   }
   return ucs4 + QUnicodeTables::qGetProp(ucs4)->mirrorDiff;
}

/*!
    \overload
    Returns the mirrored character if the UCS-2-encoded character specified
    by \a ucs2 is a mirrored character; otherwise returns the character itself.

    \sa hasMirrored()
*/
ushort QChar::mirroredChar(ushort ucs2)
{
   return ucs2 + QUnicodeTables::qGetProp(ucs2)->mirrorDiff;
}

// buffer has to have a length of 3. It's needed for Hangul decomposition
static const unsigned short *QT_FASTCALL decompositionHelper(uint ucs4, int *length, int *tag, unsigned short *buffer)
{
   *length = 0;

   if (ucs4 > QChar::LastValidCodePoint) {
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

   const unsigned short *decomposition = QUnicodeTables::uc_decomposition_map + index;
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

QChar::Decomposition QChar::decompositionTag(uint ucs4)
{
   if (ucs4 > LastValidCodePoint) {
      return QChar::NoDecomposition;
   }

   const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
   if (index == 0xffff) {
      return QChar::NoDecomposition;
   }

   return (QChar::Decomposition)(QUnicodeTables::uc_decomposition_map[index] & 0xff);
}

unsigned char QChar::combiningClass() const
{
   return (unsigned char) QUnicodeTables::qGetProp(ucs)->combiningClass;
}

unsigned char QChar::combiningClass(uint ucs4)
{
   if (ucs4 > LastValidCodePoint) {
      return 0;
   }
   return (unsigned char) QUnicodeTables::qGetProp(ucs4)->combiningClass;
}

unsigned char QChar::combiningClass(ushort ucs2)
{
   return (unsigned char) QUnicodeTables::qGetProp(ucs2)->combiningClass;
}

QChar::Script QChar::script(uint ucs4)
{
   if (ucs4 > LastValidCodePoint) {
      return QChar::Script_Unknown;
   }

   return (QChar::Script) QUnicodeTables::qGetProp(ucs4)->script;
}

QChar::UnicodeVersion QChar::unicodeVersion() const
{
   return (QChar::UnicodeVersion) QUnicodeTables::qGetProp(ucs)->unicodeVersion;
}

QChar::UnicodeVersion QChar::unicodeVersion(uint ucs4)
{
   if (ucs4 > LastValidCodePoint) {
      return QChar::Unicode_Unassigned;
   }
   return (QChar::UnicodeVersion) QUnicodeTables::qGetProp(ucs4)->unicodeVersion;
}

QChar::UnicodeVersion QChar::unicodeVersion(ushort ucs2)
{
   return (QChar::UnicodeVersion) QUnicodeTables::qGetProp(ucs2)->unicodeVersion;
}

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
   const QUnicodeTables::Properties *p = QUnicodeTables::qGetProp(ucs);

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
   if (ucs4 > LastValidCodePoint) {
      return ucs4;
   }
   const QUnicodeTables::Properties *p = QUnicodeTables::qGetProp(ucs4);
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
   const QUnicodeTables::Properties *p = QUnicodeTables::qGetProp(ucs2);
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
   const QUnicodeTables::Properties *p = QUnicodeTables::qGetProp(ucs);
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
   if (ucs4 > LastValidCodePoint) {
      return ucs4;
   }
   const QUnicodeTables::Properties *p = QUnicodeTables::qGetProp(ucs4);
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
   const QUnicodeTables::Properties *p = QUnicodeTables::qGetProp(ucs2);
   if (!p->upperCaseSpecial) {
      return ucs2 + p->upperCaseDiff;
   }
   return ucs2;
}

QChar QChar::toTitleCase() const
{
   const QUnicodeTables::Properties *p = QUnicodeTables::qGetProp(ucs);
   if (!p->titleCaseSpecial) {
      return ucs + p->titleCaseDiff;
   }
   return ucs;
}

uint QChar::toTitleCase(uint ucs4)
{
   if (ucs4 > LastValidCodePoint) {
      return ucs4;
   }

   const QUnicodeTables::Properties *p = QUnicodeTables::qGetProp(ucs4);
   if (!p->titleCaseSpecial) {
      return ucs4 + p->titleCaseDiff;
   }

   return ucs4;
}

ushort QChar::toTitleCase(ushort ucs2)
{
   const QUnicodeTables::Properties *p = QUnicodeTables::qGetProp(ucs2);

   if (! p->titleCaseSpecial) {
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
   return *ch + QUnicodeTables::qGetProp(c)->caseFoldDiff;
}

static inline uint foldCase(uint ch, uint &last)
{
   uint c = ch;
   if (QChar(c).isLowSurrogate() && QChar(last).isHighSurrogate()) {
      c = QChar::surrogateToUcs4(last, c);
   }
   last = ch;
   return ch + QUnicodeTables::qGetProp(c)->caseFoldDiff;
}

static inline ushort foldCase(ushort ch)
{
   return ch + QUnicodeTables::qGetProp(ch)->caseFoldDiff;
}

/*!
    Returns the case folded equivalent of the character. For most Unicode characters this
    is the same as toLowerCase().
*/
QChar QChar::toCaseFolded() const
{
   return ucs + QUnicodeTables::qGetProp(ucs)->caseFoldDiff;
}

/*!
    \overload
    Returns the case folded equivalent of the UCS-4-encoded character specified
    by \a ucs4. For most Unicode characters this is the same as toLowerCase().
*/
uint QChar::toCaseFolded(uint ucs4)
{
   if (ucs4 > LastValidCodePoint) {
      return ucs4;
   }
   return ucs4 + QUnicodeTables::qGetProp(ucs4)->caseFoldDiff;
}

/*!
    \overload
    Returns the case folded equivalent of the UCS-2-encoded character specified
    by \a ucs2. For most Unicode characters this is the same as toLowerCase().
*/
ushort QChar::toCaseFolded(ushort ucs2)
{
   return ucs2 + QUnicodeTables::qGetProp(ucs2)->caseFoldDiff;
}


#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &out, QChar chr)
{
   out << quint16(chr.unicode());
   return out;
}


QDataStream &operator>>(QDataStream &in, QChar &chr)
{
   quint16 u;
   in >> u;
   chr.unicode() = ushort(u);
   return in;
}
#endif // QT_NO_DATASTREAM


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

   const unsigned short *ligatures = QUnicodeTables::uc_ligature_map + index;
   ushort length = *ligatures++;
   {
      const UCS2Pair *data = reinterpret_cast<const UCS2Pair *>(ligatures);
      const UCS2Pair *r = std::lower_bound(data, data + length, ushort(u1));

      if (r != data + length && r->u1 == ushort(u1)) {
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
      const QUnicodeTables::Properties *p = QUnicodeTables::qGetProp(uc);
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
         const QUnicodeTables::Properties *p = QUnicodeTables::qGetProp(u2);
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
         const QUnicodeTables::Properties *p = QUnicodeTables::qGetProp(u1);
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
