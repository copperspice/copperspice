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

#include <qstring8.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <qunicodetables_p.h>

QString8::QString8(QChar32 c)
   : CsString::CsString(1, c)
{
}

QString8::QString8(size_type size, QChar32 c)
   : CsString::CsString(size, c)
{
}

// methods
QChar32 QString8::at(size_type index) const
{
   return CsString::CsString::operator[](index);
}

void QString8::chop(size_type n)
{
   if (n > 0) {
      auto iter = end() - n;
      erase(iter, end());
   }
}

QString8 &QString8::fill(QChar32 c, size_type newSize)
{
   if (newSize > 0) {
      assign(newSize, c);
   } else {
      assign(size(), c);
   }

   return *this;
}

bool QString8::isEmpty() const
{
   return empty();
}

bool QString8::isSimpleText() const
{
   for (auto c : *this) {
      uint32_t value = c.unicode();

      if (value > 0x058f && (value < 0x1100 || value > 0xfb0f)) {
         return false;
      }
   }

   return true;
}

QString8 QString8::left(size_type numOfChars) const
{
   if (numOfChars < 0) {
      return *this;
   }

   return QString8(substr(0, numOfChars));
}

QString8 QString8::leftJustified(size_type width, QChar32 fill, bool truncate) const
{
   QString8 retval;

   size_type len    = length();
   size_type padlen = width - len;

   if (padlen > 0) {
      retval = *this;
      retval.resize(width, fill);

   } else if (truncate) {
      retval = this->left(width);

   } else {
      retval = *this;

   }

   return retval;
}

QString8 QString8::mid(size_type index, size_type numOfChars) const
{
   return substr(index, numOfChars);
}

QString8 QString8::normalized(QString8::NormalizationForm mode, QChar32::UnicodeVersion version) const
{
   QString8 retval = cs_internal_string_normalize(*this, mode, version, 0);
   return retval;
}

QString8 &QString8::remove(size_type indexStart, size_type size)
{
   erase(indexStart, size);
   return *this;
}

QString8 QString8::repeated(size_type count) const
{
   QString8 retval;

   if (count < 1 || empty()) {
      return retval;
   }

   if (count == 1) {
      return *this;
   }

   for (size_type i = 0; i < count; ++i )  {
      retval += *this;
   }

   return retval;
}

QString8 QString8::right(size_type numOfChars) const
{
   if (numOfChars < 0) {
      return *this;
   }

   auto iter = end() - numOfChars;

   return QString8(iter, end());
}

QString8 QString8::rightJustified(size_type width, QChar32 fill, bool truncate) const
{
   QString8 retval;

   size_type len    = length();
   size_type padlen = width - len;

   if (padlen > 0) {
      retval = QString8(padlen, fill);
      retval.append(*this);

   } else if (truncate) {
      retval = this->left(width);

   } else {
      retval = *this;

   }

   return retval;
}


QString8 QString8::simplified() const &
{
   QString8 retval;

   if (empty()) {
      return retval;
   }

   // BROOM

/*
   const Char *src = cbegin();
   const Char *end = cend();

   NakedStringType result = isConst || ! str.isDetached() ?
      StringType(str.size(), Qt::Uninitialized) : qMove(str);

   Char *dst = const_cast<Char *>(result.cbegin());
   Char *ptr = dst;

   bool unmodified = true;

   forever {
      while (src != end && isSpace(*src))
         ++src;

      while (src != end && !isSpace(*src))
         *ptr++ = *src++;

      if (src == end)
         break;

      if (*src != QChar::Space)
         unmodified = false;

      *ptr++ = QChar::Space;
   }

   if (ptr != dst && ptr[-1] == QChar::Space) {
      --ptr;
   }

   int newlen = ptr - dst;

   if (isConst && newlen == str.size() && unmodified) {
      // nothing happened, return the original
      return str;
   }

   result.resize(newlen);
*/

   return retval;
}

QString8 QString8::simplified() &&
{
   QString8 retval;

   if (empty()) {
      return retval;
   }


   // broom


   return retval;
}

QString8 QString8::trimmed() const &
{
   QString8 retval;

   if (empty()) {
      return retval;
   }

   auto first_iter = begin();
   auto last_iter  = end();

   while (first_iter != last_iter) {

      if (! first_iter->isSpace() ) {
         break;
      }

      ++first_iter;
   }

   --last_iter;

   while (first_iter != last_iter) {

      if (! last_iter->isSpace() ) {
         break;
      }

      --last_iter;
   }

   ++last_iter;
   retval.append(first_iter, last_iter);

   return retval;
}

QString8 QString8::trimmed() &&
{
   if (empty()) {
      return *this;
   }

   auto first_iter = begin();
   auto last_iter  = end();

   while (first_iter != last_iter) {

      if (! first_iter->isSpace() ) {
         break;
      }

      ++first_iter;
   }

   erase(begin(), first_iter);

   //
   first_iter = begin();
   last_iter  = end();

   --last_iter;

   while (first_iter != last_iter) {

      if (! last_iter->isSpace() ) {
         break;
      }

      --last_iter;
   }

   ++last_iter;

   erase(last_iter, end());

   return *this;
}

QString8 QString8::toHtmlEscaped() const
{
   QString8 retval;

   for (auto c : *this) {

      if (c == UCHAR('<'))         {
         retval.append("&lt;");

      } else if (c == UCHAR('>'))  {
         retval.append("&gt;");

      } else if (c == UCHAR('&'))  {
         retval.append("&amp;");

      } else if (c == UCHAR('"'))  {
         retval.append("&quot;");

      } else {
          retval.append(c);
      }
   }

   return retval;
}

//
template <typename TRAITS, typename T>
static T convertCase(const T &str)
{
   T retval;

   for (auto c : str)  {
      uint32_t value = c.unicode();

      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(value);
      int32_t caseDiff = TRAITS::caseDiff(prop);

      if (TRAITS::caseSpecial(prop)) {

         const ushort *specialCase = QUnicodeTables::specialCaseMap + caseDiff;

         ushort length = *specialCase;
         ++specialCase;

         for (ushort cnt; cnt < length; ++cnt)  {
            retval += QChar32(specialCase[cnt]);
         }


      } else {
         retval += QChar32( static_cast<char32_t>(value + caseDiff) );

      }
   }

   return retval;
}

QString8 QString8::toCaseFolded() const &
{
    return convertCase<QUnicodeTables::CasefoldTraits>(*this);
}

QString8 QString8::toCaseFolded() &&
{
    return convertCase<QUnicodeTables::CasefoldTraits>(*this);
}

QString8 QString8::toLower() const &
{
    return convertCase<QUnicodeTables::LowercaseTraits>(*this);
}

QString8 QString8::toLower() &&
{
    return convertCase<QUnicodeTables::LowercaseTraits>(*this);
}

QString8 QString8::toUpper() const &
{
    return convertCase<QUnicodeTables::UppercaseTraits>(*this);
}

QString8 QString8::toUpper() &&
{
    return convertCase<QUnicodeTables::UppercaseTraits>(*this);
}

void QString8::truncate(size_type length)
{
   if (length < size()) {
      resize(length);
   }
}

// operators

#if ! defined(QT_NO_DATASTREAM)
   QDataStream &operator>>(QDataStream &out, QString8 &str)
   {
      // broom - not implemented
      return out;
   }

   QDataStream &operator<<(QDataStream &out, const QString8 &str)
   {
      // broom - not implemented
      return out;
   }
#endif


// functions

QString8 cs_internal_string_normalize(const QString8 &data, QString8::NormalizationForm mode,
                  QChar32::UnicodeVersion version, int from)
{
   QString8 retval;

   auto first_iter = data.begin() + from;
   auto last_iter  = data.end();

   while (first_iter != last_iter) {

      if (first_iter->unicode() >= 0x80) {
         break;
      }

      ++first_iter;
   }

   if (first_iter == last_iter) {
      // nothing to normalize
      return data;
   }

   if (version == QChar32::Unicode_Unassigned) {
     version = QChar32::currentUnicodeVersion();

   } else if (static_cast<int>(version) <= QUnicodeTables::NormalizationCorrectionsVersionMax) {


// start here
/*
      const QString8 &s = *data;
      QChar32 *d = 0;

      for (int i = 0; i < QUnicodeTables::NumNormalizationCorrections; ++i) {
         const QUnicodeTables::NormalizationCorrection &n = uc_normalization_corrections[i];

         if (n.version > version) {
            int pos = from;

                if (QChar::requiresSurrogates(n.ucs4)) {
                    ushort ucs4High = QChar32::highSurrogate(n.ucs4);
                    ushort ucs4Low  = QChar32::lowSurrogate(n.ucs4);
                    ushort oldHigh  = QChar32::highSurrogate(n.old_mapping);
                    ushort oldLow   = QChar32::lowSurrogate(n.old_mapping);

                    while (pos < s.length() - 1) {
                        if (s.at(pos).unicode() == ucs4High && s.at(pos + 1).unicode() == ucs4Low) {
                            if (! d) {
                                d = data->data();
                            }
                            d[pos] = QChar(oldHigh);
                            d[++pos] = QChar(oldLow);
                        }

                        ++pos;
                    }

                } else {
                    while (pos < s.length()) {
                        if (s.at(pos).unicode() == n.ucs4) {
                            if (! d) {
                                d = data->data();
                            }

                            d[pos] = QChar(n.old_mapping);
                        }
                        ++pos;
                    }
                }
            }
        }

*/

   }

/*  broom

   if (normalizationQuickCheckHelper(data, mode, from, &from)) {
      return;
   }

   decomposeHelper(data, mode < QString8::NormalizationForm_KD, version, from);
   canonicalOrderHelper(data, version, from);

   if (mode == QString::NormalizationForm_D || mode == QString8::NormalizationForm_KD) {
      return;
   }

   composeHelper(data, version, from);
*/

   return retval;
}
