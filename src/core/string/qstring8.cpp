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

#include <array>

#include <qstring8.h>
#include <qstring16.h>
#include <qdatastream.h>
#include <qregularexpression.h>
#include <qunicodetables_p.h>

static bool cs_internal_quickCheck(QString8::const_iterator &first_iter, QString8::const_iterator last_iter,
                  QString8::NormalizationForm mode);

static QString8 cs_internal_decompose(QString8::const_iterator first_iter, QString8::const_iterator last_iter,
                  bool canonical, QChar32::UnicodeVersion version);

static QString8 cs_internal_canonicalOrder(const QString8 &str, QChar32::UnicodeVersion version);
static QString8 cs_internal_compose(const QString8 &str, QChar32::UnicodeVersion version);

struct UCS2Pair {
   ushort u1;
   ushort u2;
};

struct UCS2SurrogatePair {
   UCS2Pair p1;
   UCS2Pair p2;
};

inline bool operator<(ushort u1, const UCS2Pair &ligature)
{
   return u1 < ligature.u1;
}

inline bool operator<(const UCS2Pair &ligature, ushort u1)
{
   return ligature.u1 < u1;
}

QString8::QString8(QChar32 c)
   : CsString::CsString(1, c)
{
}

QString8::QString8(size_type numOfChars, QChar32 c)
   : CsString::CsString(numOfChars, c)
{
}

QString8::const_iterator QString8::cs_internal_find_fast(QChar32 c, const_iterator iter_begin) const
{
   const_iterator iter_end = cend();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   auto iter = iter_begin;
   QString8 strFolded = c.toCaseFolded();

   if (strFolded.size() == 1) {
      char32_t value = strFolded.first().unicode();

      while (iter != iter_end)   {
         if (iter->toCaseFolded().first().unicode() == value)  {
            // found a match
            return iter;
         }

         ++iter;
      }

   } else {
      return cs_internal_find_fast(strFolded, iter_begin);

   }

   return iter_end;
}

QString8::const_iterator QString8::cs_internal_find_fast(const QString8 &str, const_iterator iter_begin) const
{
   const_iterator iter_end = cend();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   if (str.empty()) {
      return iter_begin;
   }

   auto iter = iter_begin;
   QString8 strFolded = str.toCaseFolded();

   while (iter != iter_end)   {
      // review: account for code points which expand when folded

      if (iter->toCaseFolded() == QString8(strFolded[0]))  {
         auto text_iter    = iter + 1;
         auto pattern_iter = strFolded.begin() + 1;

         while (text_iter != iter_end && pattern_iter != str.cend())  {

            if (text_iter->toCaseFolded() == QString8(*pattern_iter))  {
               ++text_iter;
               ++pattern_iter;

            } else {
               break;

            }
         }

         if (pattern_iter == strFolded.end()) {
            // found a match
            return iter;
         }
      }

      ++iter;
   }

   return iter_end;
}

QString8::const_iterator QString8::cs_internal_rfind_fast(QChar32 c, const_iterator iter_begin) const
{
   const_iterator iter_end = cend();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   auto iter = iter_begin;
   QString8 strFolded = c.toCaseFolded();

   if (strFolded.size() == 1) {
      char32_t value = strFolded.first().unicode();

      while (iter != iter_end)   {
         if (iter->toCaseFolded().first().unicode() == value)  {
            // found a match
            return iter;
         }

         ++iter;
      }

   } else {
      return cs_internal_rfind_fast(strFolded, iter_begin);

   }

   return iter_end;
}

QString8::const_iterator QString8::cs_internal_rfind_fast(const QString8 &str, const_iterator iter_begin) const
{
   const_iterator iter_end = cend();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   if (str.empty()) {
      return iter_begin;
   }

   auto iter = iter_begin;
   QString8 strFolded = str.toCaseFolded();

   while (iter != iter_end)   {
      // review: account for code points which expand when folded

      if (iter->toCaseFolded() == QString8(strFolded[0]))  {
         auto text_iter    = iter + 1;
         auto pattern_iter = strFolded.begin() + 1;

         while (text_iter != iter_end && pattern_iter != str.cend())  {

            if (text_iter->toCaseFolded() == QString8(*pattern_iter))  {
               ++text_iter;
               ++pattern_iter;

            } else {
               break;

            }
         }

         if (pattern_iter == strFolded.end()) {
            // found a match
            return iter;
         }
      }

      ++iter;
   }

   return iter_end;
}



// methods
void QString8::chop(size_type n)
{
   if (n > 0) {
      auto iter = end() - n;
      erase(iter, end());
   }
}

int QString8::compare(const QString8 &str, Qt::CaseSensitivity cs) const
{
   auto iter_a = cbegin();
   auto iter_b = str.cbegin();

   while (iter_a != cend() && iter_b != str.cend()) {

      auto value_a = *iter_a;
      auto value_b = *iter_b;

      if (cs == Qt::CaseSensitive) {
         if (value_a < value_b) {
            return -1;

         } else if (value_a > value_b) {
            return 1;

         }

      } else {
         auto folded_a = value_a.toCaseFolded();
         auto folded_b = value_b.toCaseFolded();

         if (folded_a < folded_b) {
            return -1;

         } else if (folded_a > folded_b) {
            return 1;

         }
      }

      ++iter_a;
      ++iter_b;
   }

   if (iter_b != str.cend())  {
      return -1;

   } else if (iter_a != cend()) {
      return 1;

   }

   return 0;
}

int QString8::compare(QStringView8 str, Qt::CaseSensitivity cs) const
{
   auto iter_a = cbegin();
   auto iter_b = str.cbegin();

   while (iter_a != cend() && iter_b != str.cend()) {

      auto value_a = *iter_a;
      auto value_b = *iter_b;

      if (cs == Qt::CaseSensitive) {
         if (value_a < value_b) {
            return -1;

         } else if (value_a > value_b) {
            return 1;

         }

      } else {
         auto folded_a = value_a.toCaseFolded();
         auto folded_b = value_b.toCaseFolded();

         if (folded_a < folded_b) {
            return -1;

         } else if (folded_a > folded_b) {
            return 1;

         }
      }

      ++iter_a;
      ++iter_b;
   }

   if (iter_b != str.cend())  {
      return -1;

   } else if (iter_a != cend()) {
      return 1;

   }

   return 0;
}

QString8::size_type QString8::count(QChar32 c, Qt::CaseSensitivity cs) const
{
   size_type retval = 0;

   if (cs == Qt::CaseSensitive) {
      for (auto uc : *this) {
         if (uc == c) {
            ++retval;
         }
      }

   } else {
      QString8 tmp = c.toCaseFolded();

      for (auto uc : *this) {
         if (uc.toCaseFolded() == tmp) {
            ++retval;
         }
      }
   }

   return retval;
}

QString8::size_type QString8::count(const QString8 &str, Qt::CaseSensitivity cs) const
{
   size_type retval = 0;

   const_iterator iter      = this->cbegin();
   const_iterator iter_end  = this->cend();

   iter = indexOfFast(str, iter, cs);

   while (iter != iter_end) {
      ++retval;
      iter = indexOfFast(str, iter+1, cs);
   }

   return retval;
}

QString8::size_type QString8::count(QStringView8 str, Qt::CaseSensitivity cs) const
{
   size_type retval = 0;

   const_iterator iter      = cbegin();
   const_iterator iter_end  = cend();

   iter = indexOfFast(str, iter, cs);

   while (iter != iter_end) {
      ++retval;
      iter = indexOfFast(str, iter+1, cs);
   }

   return retval;
}

QString8::size_type QString8::count(const QRegularExpression<QString8> &regExp) const
{
   size_type retval = 0;

   const_iterator iter     = cbegin();
   const_iterator iter_end = cend();

   QRegularExpressionMatch<QString8> match;

   while (iter != iter_end) {                      // count overlapping matches
      match = regExp.match(*this, iter);

      if (! match.hasMatch()) {
         break;
      }

      retval++;

      iter = match.capturedStart(0);
      ++iter;
   }

   return retval;
}

bool QString8::contains(QChar32 c, Qt::CaseSensitivity cs) const
{
   const_iterator iter      = this->cbegin();
   const_iterator iter_end  = this->cend();

   QString8 other = QString8(c);
   iter = indexOfFast(other, iter, cs);

   if (iter != iter_end) {
      return true;
   }

   return false;
}

bool QString8::contains(const QString8 &other, Qt::CaseSensitivity cs) const
{

   const_iterator iter      = this->cbegin();
   const_iterator iter_end  = this->cend();

   iter = indexOfFast(other, iter, cs);

   if (iter != iter_end) {
      return true;
   }

   return false;
}

bool QString8::contains(QStringView8 str, Qt::CaseSensitivity cs) const
{

   const_iterator iter      = this->cbegin();
   const_iterator iter_end  = this->cend();

   iter = indexOfFast(str, iter, cs);      // PERTH str was other

   if (iter != iter_end) {
      return true;
   }

   return false;
}

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

bool QString8::endsWith(QChar32 c, Qt::CaseSensitivity cs) const
{
   if (empty()) {
      return false;
   }

   auto iter = end() - 1;

   if (cs == Qt::CaseSensitive) {
      return *iter == c;

   } else {
      return iter->toCaseFolded() == c.toCaseFolded();

   }
}

bool QString8::endsWith(const QString8 &str, Qt::CaseSensitivity cs) const
{
   if (str.empty() ){
      return true;

   } else if (empty()) {
      return false;
   }

   if (cs == Qt::CaseSensitive) {
      auto iter = crbegin();

      for (auto iter_other = str.crbegin(); iter_other != str.crend(); ++iter_other) {

         if (iter == crend()) {
            return false;
         }

         if (*iter != *iter_other) {
            return false;
         }

         ++iter;
      }

      return true;

   } else {
      auto iter = crbegin();

      for (auto iter_other = str.crbegin(); iter_other != str.crend(); ++iter_other) {

         if (iter == rend()) {
            return false;
         }

         if (iter->toCaseFolded() != iter_other->toCaseFolded()) {
            return false;
         }

         ++iter;
      }

      return true;
   }
}

bool QString8::endsWith(QStringView8 str, Qt::CaseSensitivity cs) const
{
   if (str.empty() ){
      return true;

   } else if (empty()) {
      return false;
   }

   if (cs == Qt::CaseSensitive) {
      auto iter = crbegin();

      for (auto iter_other = str.crbegin(); iter_other != str.crend(); ++iter_other) {

         if (iter == crend()) {
            return false;
         }

         if (*iter != *iter_other) {
            return false;
         }

         ++iter;
      }

      return true;

   } else {
      auto iter = crbegin();

      for (auto iter_other = str.crbegin(); iter_other != str.crend(); ++iter_other) {

         if (iter == rend()) {
            return false;
         }

         if (iter->toCaseFolded() != iter_other->toCaseFolded()) {
            return false;
         }

         ++iter;
      }

      return true;
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

QString8 QString8::fromLatin1(const QByteArray &str)
{
   QString8 retval;

   for (char c : str) {
      const char32_t value = c;
      retval.append(value);
   }

   return retval;
}

QString8 QString8::fromLatin1(const char *str, size_type numOfChars)
{
   QString8 retval;

   if (str == nullptr) {
      // do nothing

   } else if (numOfChars == -1)  {

      for (size_type i = 0; str[i] != 0; ++i) {
         const char32_t value = str[i];
         retval.append(value);
      }

   } else {

      for (size_type i = 0; i < numOfChars; ++i) {
         const char32_t value = str[i];
         retval.append(value);
      }
   }

   return retval;
}

QString8 QString8::fromUtf8(const QByteArray &str)
{
   return fromUtf8(str.constData(), str.size());
}

QString8 QString8::fromUtf8(const char *str, size_type numOfChars)
{
   return CsString::CsString::fromUtf8(str, numOfChars);
}

QString8 QString8::fromUtf16(const char16_t *str, size_type numOfChars)
{
   if (str == nullptr) {
      return QString8();
   }

   if (numOfChars < 0) {
      numOfChars = 0;

      while (str[numOfChars] != 0) {
         ++numOfChars;
      }
   }

   // broom ( full implementation required )
   QString8 retval;

   for (int i = 0; i < numOfChars; ++i) {
      retval.append(static_cast<char32_t>(str[i]));
   }

   return retval;
}

QString8 QString8::fromUtf16(const QString16 &str)
{
   return fromUtf16((const char16_t *)str.constData(), str.size_storage());
}

QString8 QString8::fromStdWString(const std::wstring &str)
{
   // broom ( full implementation required )

   return QString8();
}

QString8 QString8::fromStdString(const std::string &str)
{

   // broom ( full implementation required )

   return QString8();
}

QString8::const_iterator QString8::indexOfFast(const QRegularExpression<QString8> &regExp, const_iterator from) const
{
   QRegularExpressionMatch<QString8> match = regExp.match(*this, from);

   if (match.hasMatch())  {
      return match.capturedStart(0);
   }

   return end();
}

QString8::const_iterator QString8::lastIndexOfFast(const QRegularExpression<QString8> &regExp, const_iterator from) const
{
   QRegularExpressionMatch<QString8> match = regExp.rmatch(*this, from);

   if (match.hasMatch())  {
      return match.capturedStart(0);
   }

   return end();
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

QStringView8 QString8::leftView(size_type numOfChars) const
{
   if (numOfChars < 0) {
      return QStringView8(cbegin(), cend());
   }

   const_iterator iter = cbegin();

   for (size_type i = 0; i < numOfChars && iter != cend(); ++i)  {
      ++iter;
   }

   return QStringView8(cbegin(), iter);
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

/*
int localeAwareCompare(const QString8 &str) const
{
   // not implemented
}

int localeAwareCompare(QStringView8 str) const
{
   // not imlemented
}
*/

QString8 QString8::mid(size_type indexStart, size_type numOfChars) const
{
   return substr(indexStart, numOfChars);
}

QStringView8 QString8::midView(size_type indexStart, size_type numOfChars) const
{
   const_iterator iter_begin = cbegin();
   const_iterator iter_end;

   for (size_type i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (iter_begin == cend()) {
      // index > size()
      return QStringView8();
   }

   if (numOfChars >= 0) {
      iter_end = iter_begin;

      for (size_type i = 0; i < numOfChars && iter_end != cend(); ++i)  {
         ++iter_end;
      }

   } else {
      iter_end = cend();

   }

   return QStringView8(iter_begin, iter_end);
}

QString8 QString8::normalized(QString8::NormalizationForm mode, QChar32::UnicodeVersion version) const
{
   QString8 retval = cs_internal_string_normalize(*this, mode, version, 0);
   return retval;
}

QString8 &QString8::remove(size_type indexStart, size_type numOfChars)
{
   erase(indexStart, numOfChars);
   return *this;
}

QString8 &QString8::remove(QChar32 c, Qt::CaseSensitivity cs)
{
   auto first_iter = cbegin();
   auto last_iter  = cend();

   if (cs == Qt::CaseSensitive) {
      auto iter = first_iter;

      while (iter != last_iter) {

         if (*iter == c) {
            iter = erase(iter);
            last_iter = cend();

         } else {
            ++iter;

         }
      }

   } else {
      QString8 str = c.toCaseFolded();

      if (str.size() == 1 ) {
         auto iter = first_iter;

         while (iter != last_iter) {

            if ( (*iter).toCaseFolded() == str) {
               iter = erase(iter);
               last_iter = cend();

            } else {
               ++iter;

            }
         }

      } else {
         remove(str, cs);

      }
   }

   return *this;
}

QString8 &QString8::remove(const QString8 &str, Qt::CaseSensitivity cs)
{
   if (! str.isEmpty()) {
      int i = 0;

      while ((i = indexOf(str, i, cs)) != -1) {
         remove(i, str.size());
      }
   }

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

QString8 &QString8::replace(const QString8 &before, const QString8 &after, Qt::CaseSensitivity cs)
{
   if (this->empty() || before.isEmpty()) {
      return *this;
   }

   auto iter     = indexOfFast(before, begin(), cs);
   size_type len = before.size();

   while (iter != end()) {
      auto last = iter + len;

      iter = erase(iter, last);
      iter = CsString::CsString::insert(iter, after);

      iter = indexOfFast(before, iter, cs);
   }

   return *this;
}

QString8 &QString8::replace(const QChar32 *before, size_type beforeSize, const QChar32 *after, size_type afterSize,
                  Qt::CaseSensitivity cs)
{
   if (this->empty() || beforeSize == 0) {
      return *this;
   }

   replace(QString8(before, beforeSize), QString8(after, afterSize), cs);

   return *this;
}

QString8 &QString8::replace(QChar32 before, QChar32 after, Qt::CaseSensitivity cs)
{
   auto first_iter = cbegin();
   auto last_iter  = cend();

   if (cs == Qt::CaseSensitive) {
      auto iter = first_iter;

      while (iter != last_iter) {

         if (*iter == before) {
           iter = replace(iter, after);
            last_iter = cend();

         } else {
            ++iter;

         }
      }

   } else {
      QString8 str = before.toCaseFolded();

      if (str.size() == 1 ) {
         auto iter = first_iter;

         while (iter != last_iter) {

            if ( (*iter).toCaseFolded() == str) {
               iter = replace(iter, after);
               last_iter = cend();

            } else {
               ++iter;

            }
         }

      } else {
         remove(str, cs);

      }
   }

   return *this;
}

QString8 &QString8::replace(QChar32 c, const QString8 &after, Qt::CaseSensitivity cs)
{
   auto first_iter = cbegin();
   auto last_iter  = cend();

   if (cs == Qt::CaseSensitive) {
      auto iter = first_iter;

      while (iter != last_iter) {

         if (*iter == c) {
            iter = replace(iter, after);
            last_iter = cend();

         } else {
            ++iter;

         }
      }

   } else {
      QString8 str = c.toCaseFolded();

      if (str.size() == 1 ) {
         auto iter = first_iter;

         while (iter != last_iter) {

            if ( (*iter).toCaseFolded() == str) {
               iter = replace(iter, after);
               last_iter = cend();

            } else {
               ++iter;

            }
         }

      } else {
         remove(str, cs);

      }
   }

   return *this;
}

QString8 &QString8::replace(const QRegularExpression<QString8> &regExp, const QString8 &after)
{
   QRegularExpressionMatch<QString8> match = regExp.match(*this);
   QRegularExpressionMatch<QString8> splitMatch;

   static QRegularExpression<QString8> regSplit("(.*?)(\\\\[0-9])");
   bool noCapture = true;

   auto iter = after.indexOfFast('\\');

   if (iter != after.end() && iter != end() - 1) {
      splitMatch = regSplit.match(after);

      if (splitMatch.hasMatch()) {
         noCapture = false;
      }
   }

   if (noCapture) {

      while (match.hasMatch())  {
         auto first = match.capturedStart(0);
         auto last  = match.capturedEnd(0);

         auto iter  = this->erase(first, last);
         iter       = CsString::CsString::insert(iter, after);

         match      = regExp.match(*this, iter);
      }

   } else {
      // look for a 0-9
      QVector<QStringView8> list;

      QString8::const_iterator hold;

      while (splitMatch.hasMatch())  {
         list.append(splitMatch.capturedView(1));
         list.append(splitMatch.capturedView(2));

         hold = splitMatch.capturedEnd(0);

         splitMatch = regSplit.match(after, splitMatch.capturedEnd(0));
      }

      if (hold != after.end()) {

         // grab the rest of 'after'
         list.append( QStringView8(hold, after.end()) );
      }

      std::array<QString8, 10> saveCapture;

      while (match.hasMatch())  {
         auto first = match.capturedStart(0);
         auto last  = match.capturedEnd(0);

         for (int x = 0; x < 10; ++x) {
            saveCapture[x] = match.captured(x);
         }

         auto iter  = this->erase(first, last);

         for (const auto &item : list) {

            if (item == "\\0") {
               iter = CsString::CsString::insert(iter, saveCapture[0]);

            } else if (item == "\\1") {
                  iter = CsString::CsString::insert(iter, saveCapture[1]);

            } else if (item == "\\2") {
                  iter = CsString::CsString::insert(iter, saveCapture[2]);

            } else if (item == "\\3") {
                  iter = CsString::CsString::insert(iter, saveCapture[3]);

            } else if (item == "\\4") {
                  iter = CsString::CsString::insert(iter, saveCapture[4]);

            } else if (item == "\\5") {
                  iter = CsString::CsString::insert(iter, saveCapture[5]);

            } else if (item == "\\6") {
                  iter = CsString::CsString::insert(iter, saveCapture[6]);

            } else if (item == "\\7") {
                  iter = CsString::CsString::insert(iter, saveCapture[7]);

            } else if (item == "\\8") {
                  iter = CsString::CsString::insert(iter, saveCapture[8]);

            } else if (item == "\\9") {
                  iter = CsString::CsString::insert(iter, saveCapture[9]);

            } else {
               iter = CsString::CsString::insert(iter, item);

            }
         }

         match = regExp.match(*this, iter);
      }
   }

   return *this;
}

QString8 QString8::right(size_type numOfChars) const
{
   if (numOfChars < 0) {
      return *this;
   }

   auto iter = cend() - numOfChars;

   return QString8(iter, cend());
}

QStringView8 QString8::rightView(size_type numOfChars) const
{
   if (numOfChars < 0) {
      return QStringView8(cbegin(), cend());
   }

   const_iterator iter = cend();

   for (size_type i = 0; i < numOfChars && iter != cbegin(); ++i)  {
      --iter;
   }

   return QStringView8(iter, cend());
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

   auto first_iter = cbegin();
   auto last_iter  = cend();

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

   // reduce white space in the middle
   bool isFirst = true;

   for (auto iter = first_iter; iter != last_iter; ++iter)  {
      QChar32 c = *iter;

      if (c.isSpace()) {

         if (isFirst) {
            isFirst = false;
            retval.append(' ');
         }

      } else {
         isFirst = true;
         retval.append(c);

      }
   }

   return retval;
}

QString8 QString8::simplified() &&
{
   QString8 retval;

   if (empty()) {
      return retval;
   }

   auto first_iter = cbegin();
   auto last_iter  = cend();

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

   // reduce white space in the middle
   bool isFirst = true;

   for (auto iter = first_iter; iter != last_iter; ++iter)  {

      QChar32 c = *iter;

      if (c.isSpace()) {

         if (isFirst) {
            isFirst = false;
            retval.append(' ');
         }

      } else {
         isFirst = true;
         retval.append(c);

      }
   }

   return retval;
}

QString8 QString8::trimmed() const &
{
   QString8 retval;

   if (empty()) {
      return retval;
   }

   auto first_iter = cbegin();
   auto last_iter  = cend();

   while (first_iter != last_iter) {

      if (! first_iter->isSpace() ) {
         break;
      }

      ++first_iter;
   }

   if (first_iter == last_iter) {
      // trimmed beginning, string is actually empty
      return retval;
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

   auto first_iter = cbegin();
   auto last_iter  = cend();

   while (first_iter != last_iter) {

      if (! first_iter->isSpace() ) {
         break;
      }

      ++first_iter;
   }

   erase(begin(), first_iter);

   if (empty()) {
      // trimmed beginning, string is actually empty
      return *this;
   }

   //
   first_iter = cbegin();
   last_iter  = cend();

   --last_iter;

   while (first_iter != last_iter) {

      if (! last_iter->isSpace() ) {
         break;
      }

      --last_iter;
   }

   ++last_iter;

   erase(last_iter, cend());

   return *this;
}

bool QString8::startsWith(QChar32 c, Qt::CaseSensitivity cs) const
{
   if (empty()) {
      return false;
   }

   if (cs == Qt::CaseSensitive) {
      return at(0) == c;

   } else {
      return at(0).toCaseFolded() == c.toCaseFolded();

   }
}

bool QString8::startsWith(const QString8 &str, Qt::CaseSensitivity cs) const
{
   if (str.empty()) {
      return true;

   } else if (empty()) {
      return false;

   }

   if (cs == Qt::CaseSensitive) {
      auto iter = cbegin();

      for (auto uc : str) {

         if (iter == cend()) {
            return false;
         }

         if (*iter != uc) {
            return false;
         }

         ++iter;
      }

      return true;

   } else {
      auto iter = cbegin();

      for (auto uc : str) {

         if (iter == cend()) {
            return false;
         }

         if ( iter->toCaseFolded() != uc.toCaseFolded()) {
            return false;
         }

         ++iter;
      }

      return true;
   }
}

bool QString8::startsWith(QStringView8 str, Qt::CaseSensitivity cs) const
{
   if (str.empty()) {
      return true;

   } else if (empty()) {
      return false;

   }

   if (cs == Qt::CaseSensitive) {
      auto iter = cbegin();

      for (auto uc : str) {

         if (iter == cend()) {
            return false;
         }

         if (*iter != uc) {
            return false;
         }

         ++iter;
      }

      return true;

   } else {
      auto iter = cbegin();

      for (auto uc : str) {

         if (iter == cend()) {
            return false;
         }

         if ( iter->toCaseFolded() != uc.toCaseFolded()) {
            return false;
         }

         ++iter;
      }

      return true;
   }
}

QString8 QString8::toCaseFolded() const &
{
   return convertCase<QUnicodeTables::CasefoldTraits>(*this);
}

QString8 QString8::toCaseFolded() &&
{
   return convertCase<QUnicodeTables::CasefoldTraits>(*this);
}

QString8 QString8::toHtmlEscaped() const
{
   QString8 retval;

   for (auto c : *this) {

      if (c == UCHAR('<'))         {
         retval.append("&lt;");

      } else if (c== UCHAR('>'))  {
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

QByteArray QString8::toLatin1() const
{
   QByteArray retval;

   for (QChar32 c : *this) {
      const char32_t value = c.unicode();

      if (value > 255) {
         retval.append('?' );

      } else {
         retval.append(value);

      }
   }

   return retval;
}

QByteArray QString8::toUtf8() const
{
   return QByteArray(constData(), CsString::CsString::size_storage());
}

QString16 QString8::toUtf16() const
{
   // broom ( full implementation required )


   return QString16();
}

std::wstring QString8::toStdWString() const
{
   // broom ( full implementation required )

   std::wstring retval;

   for (QChar32 c : *this) {
      const wchar_t value = c.unicode();

      retval.push_back(value);
   }

   return retval;
}

void QString8::truncate(size_type length)
{
   if (length < size()) {
      resize(length);
   }
}

// data stream
QDataStream &operator>>(QDataStream &in, QString8 &str)
{
   char *tmp;
   uint len;

   in.readBytes(tmp, len);
   str = QString8::fromUtf8(tmp, len);
   delete [] tmp;

   return in;
}

QDataStream &operator<<(QDataStream &out, const QString8 &str)
{
   out.writeBytes(str.constData(), str.size_storage());
   return out;
}

// normalization functions
QString8 cs_internal_string_normalize(const QString8 &data, QString8::NormalizationForm mode,
                  QChar32::UnicodeVersion version, int from)
{
   QString8 retval;

   auto first_iter = data.cbegin() + from;
   auto last_iter  = data.cend();

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
      // used passed version value

/*
      for (int i = 0; i < QUnicodeTables::NumNormalizationCorrections; ++i) {
         const QUnicodeTables::NormalizationCorrection &n = uc_normalization_corrections[i];

         if (n.version > version) {
           int pos = from;

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
*/

   }


   // ** 1
   if (cs_internal_quickCheck(first_iter, last_iter, mode)) {
      // nothing to normalize

      printf("QString 8  Quick Check \n");
      return data;
   }

   // ** 2
   retval.assign(data.begin(), first_iter);

   retval += cs_internal_decompose(first_iter, last_iter, mode < QString8::NormalizationForm_KD, version);

   // ** 3
   retval = cs_internal_canonicalOrder(retval, version);

   if (mode == QString8::NormalizationForm_D || mode == QString8::NormalizationForm_KD) {
      return retval;
   }

   // ** 4
   retval = cs_internal_compose(retval, version);

   return retval;
}

bool cs_internal_quickCheck(QString8::const_iterator &first_iter, QString8::const_iterator last_iter,
                  QString8::NormalizationForm mode)
{
   // method one

   static_assert(QString8::NormalizationForm_D  == 0, "Normalization form mismatch");
   static_assert(QString8::NormalizationForm_C  == 1, "Normalization form mismatch");
   static_assert(QString8::NormalizationForm_KD == 2, "Normalization form mismatch");
   static_assert(QString8::NormalizationForm_KC == 3, "Normalization form mismatch");

   enum { NFQC_YES = 0, NFQC_NO = 1, NFQC_MAYBE = 3 };

   uchar lastCombining = 0;

   for (auto iter = first_iter; iter != last_iter; ++iter) {
      QChar32 uc = *iter;

      if (uc < 0x80) {
         // ASCII characters are stable code points
         lastCombining = 0;
         first_iter = iter;
         continue;
      }

      const QUnicodeTables::Properties *p = QUnicodeTables::properties(uc.unicode());

      if (p->combiningClass < lastCombining && p->combiningClass > 0) {
         return false;
      }

      const uchar check = (p->nfQuickCheck >> (mode << 1)) & 0x03;
      if (check != NFQC_YES)  {
         return false;
      }

      lastCombining = p->combiningClass;
      if (lastCombining == 0) {
        first_iter = iter;
      }
   }

   return true;
}

// buffer has to have a length of 3, required for Hangul decomposition
static const uint16_t * cs_internal_decompose_2(uint ucs4, int *length, int *tag, unsigned short *buffer)
{
    if (ucs4 >= Hangul_SBase && ucs4 < Hangul_SBase + Hangul_SCount) {
        // compute Hangul syllable decomposition as per UAX #15

        const uint SIndex = ucs4 - Hangul_SBase;
        buffer[0] = Hangul_LBase + SIndex / Hangul_NCount;                   // L
        buffer[1] = Hangul_VBase + (SIndex % Hangul_NCount) / Hangul_TCount; // V
        buffer[2] = Hangul_TBase + SIndex % Hangul_TCount;                   // T

        *length = buffer[2] == Hangul_TBase ? 2 : 3;
        *tag = QChar32::Canonical;

        return buffer;
    }

    const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
    if (index == 0xffff) {
        *length = 0;
        *tag    = QChar32::NoDecomposition;
        return 0;
    }

    const unsigned short *decomposition = QUnicodeTables::uc_decomposition_map + index;
    *tag    = (*decomposition) & 0xff;
    *length = (*decomposition) >> 8;

    return decomposition + 1;
}

QString8 cs_internal_decompose(QString8::const_iterator first_iter, QString8::const_iterator last_iter,
                  bool canonical, QChar32::UnicodeVersion version)
{
   // method two
   QString8 retval;

   int length;
   int tag;

   unsigned short buffer[3];

   if (first_iter == last_iter) {
      return retval;
   }

   for (auto iter = first_iter; iter != last_iter; ++iter) {
      QChar32 uc = *iter;

      if (uc.unicodeVersion() > version) {
        retval.append(uc);
        continue;
      }

      const uint16_t *strDecomp = cs_internal_decompose_2(uc.unicode(), &length, &tag, buffer);

      if (! strDecomp || (canonical && tag != QChar32::Canonical)) {
         retval.append(uc);
         continue;
      }

      for (int index = 0; index < length; ++index)  {
         // assume code points are in the basic multi-lingual plane

         QChar32 c = QChar32(strDecomp[index]);

         if (c.unicode() < 0x80) {
            // ASCII characters are stable code points
            retval.append(c);

         } else {
            QString8 tmp = c;
            tmp = cs_internal_decompose(tmp.cbegin(), tmp.cend(), canonical, version);
            retval.append(tmp);

         }
      }
   }

   return retval;
}

QString8 cs_internal_canonicalOrder(const QString8 &str, QChar32::UnicodeVersion version)
{
   // method three
   QString8 retval;

   auto first_iter = str.cbegin();
   auto last_iter  = str.cend();

   QVector< std::pair<ushort, QChar32>> buffer;

   for (auto iter = first_iter; iter != last_iter; ++iter) {
      QChar32 uc = *iter;
      ushort combineType = 0;

      const QUnicodeTables::Properties *p = QUnicodeTables::properties(uc.unicode());

      if (p->unicodeVersion <= version) {
        combineType = p->combiningClass;
      }

      if (combineType == 0) {
         std::stable_sort(buffer.begin(), buffer.end(), [](auto a, auto b){ return (a.first < b.first); } );

         for (auto c : buffer) {
            retval.append(c.second);
         }
         buffer.clear();

         retval.append(uc);

      } else {
         // non combining character
         buffer.append(std::make_pair(combineType, uc));

      }
   }

   //
   std::stable_sort(buffer.begin(), buffer.end(), [](auto a, auto b){ return (a.first < b.first); } );

   for (auto c : buffer) {
      retval.append(c.second);
   }

   return retval;
}

static char32_t inline cs_internal_ligature(uint u1, uint u2)
{
   if (u1 >= Hangul_LBase && u1 <= Hangul_SBase + Hangul_SCount) {
     // compute Hangul syllable composition as per UAX #15
     // hangul L-V pair
     const uint LIndex = u1 - Hangul_LBase;

     if (LIndex < Hangul_LCount) {
         const uint VIndex = u2 - Hangul_VBase;

         if (VIndex < Hangul_VCount) {
            return Hangul_SBase + (LIndex * Hangul_VCount + VIndex) * Hangul_TCount;
         }
     }

     // hangul LV-T pair
     const uint SIndex = u1 - Hangul_SBase;
     if (SIndex < Hangul_SCount && (SIndex % Hangul_TCount) == 0) {
         const uint TIndex = u2 - Hangul_TBase;

         if (TIndex <= Hangul_TCount) {
            return u1 + TIndex;
         }
     }
   }

   const unsigned short index = GET_LIGATURE_INDEX(u2);
   if (index == 0xffff) {
      return 0;
   }

   const unsigned short *ligatures = QUnicodeTables::uc_ligature_map + index;
   ushort length = *ligatures++;

   const UCS2Pair *data = reinterpret_cast<const UCS2Pair *>(ligatures);
   const UCS2Pair *r    = std::lower_bound(data, data + length, ushort(u1));

   if (r != data + length && r->u1 == ushort(u1)) {
     return r->u2;
   }

   return 0;
}

static QString8 cs_internal_compose(const QString8 &str, QChar32::UnicodeVersion version)
{
   // method four
   QString8 retval;

   auto first_iter = str.cbegin();
   auto last_iter  = str.cend();

   QString8::const_iterator iterBeg = last_iter;
   QString8::const_iterator iterEnd = last_iter;

   uint codePointBeg = 0;           // starting code point value
   int lastCombining = 255;         // to prevent combining > lastCombining

   for (auto iter = first_iter; iter != last_iter; ++iter) {
      QChar32 uc   = *iter;
      uint ucValue = uc.unicode();

      const QUnicodeTables::Properties *p = QUnicodeTables::properties(ucValue);

      if (p->unicodeVersion > version) {
         iterBeg = last_iter;
         iterEnd = last_iter;

         lastCombining = 255;       // to prevent combining > lastCombining

         retval.append(uc);
         continue;
      }

      int combining = p->combiningClass;

      if (iterBeg >= first_iter && (iter == iterEnd || combining > lastCombining)) {
         // form ligature with prior code point
         char32_t ligature = cs_internal_ligature(codePointBeg, ucValue);

         if (ligature) {
            codePointBeg = ligature;

            retval.chop(1);
            retval.append(QChar32(ligature));
            continue;
         }
      }

      if (combining == 0) {
         retval.append(uc);

         codePointBeg = ucValue;

         iterBeg = iter;
         iterEnd = iter + 1;
      }

      lastCombining = combining;
   }

   return retval;
}

