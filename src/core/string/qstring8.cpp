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

#include <qstring8.h>
#include <qstring16.h>

#include <qdatastream.h>
#include <qregularexpression.h>

#include <qunicodetables_p.h>

#if defined(Q_OS_WIN)
#include <qt_windows.h>
#endif

#ifdef Q_OS_DARWIN
#include <CoreFoundation/CFString.h>
#endif

#include <array>

static bool cs_internal_quickCheck(QString8::const_iterator &first_iter, QString8::const_iterator last_iter,
                  QString8::NormalizationForm mode);

static QString8 cs_internal_decompose(QString8::const_iterator first_iter, QString8::const_iterator last_iter,
                  bool canonical, QChar32::UnicodeVersion version);

static QString8 cs_internal_canonicalOrder(const QString8 &str, QChar32::UnicodeVersion version);
static QString8 cs_internal_compose(const QString8 &str, QChar32::UnicodeVersion version);

#if ! defined(CSTR_LESS_THAN)
#define CSTR_LESS_THAN       1
#define CSTR_EQUAL           2
#define CSTR_GREATER_THAN    3
#endif

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
         auto pattern_iter = strFolded.cbegin() + 1;

         while (text_iter != iter_end && pattern_iter != strFolded.cend())  {

            if (text_iter->toCaseFolded() == QString8(*pattern_iter))  {
               ++text_iter;
               ++pattern_iter;

            } else {
               break;

            }
         }

         if (pattern_iter == strFolded.cend()) {
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
         auto pattern_iter = strFolded.cbegin() + 1;

         while (text_iter != iter_end && pattern_iter != strFolded.cend())  {

            if (text_iter->toCaseFolded() == QString8(*pattern_iter))  {
               ++text_iter;
               ++pattern_iter;

            } else {
               break;

            }
         }

         if (pattern_iter == strFolded.cend()) {
            // found a match
            return iter;
         }
      }

      ++iter;
   }

   return iter_end;
}

// methods
void QString8::chop(size_type numOfChars)
{
   if (numOfChars > 0) {
      auto iter = cend();

      for (size_type cnt = 0; cnt < numOfChars; ++cnt) {

         if (iter == cbegin()) {
            clear();
            return;
         }

         --iter;
      }

      erase(iter, cend());
   }
}

int QString8::compare(QStringView8 str1, QStringView8 str2, Qt::CaseSensitivity cs)
{
   auto iter_a = str1.cbegin();
   auto iter_b = str2.cbegin();

   while (iter_a != str1.cend() && iter_b != str2.cend()) {

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

   if (iter_b != str2.cend())  {
      return -1;

   } else if (iter_a != str1.cend()) {
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

QString8::size_type QString8::count(const QRegularExpression8 &regExp) const
{
   size_type retval = 0;

   const_iterator iter     = cbegin();
   const_iterator iter_end = cend();

   QRegularExpressionMatch8 match;

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

   iter = indexOfFast(str, iter, cs);

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
      uint32_t value     = c.unicode();
      char32_t caseValue = TRAITS::caseValue(value);

      if (caseValue == 0 && value != 0) {
         // special char

         const char32_t *data = TRAITS::caseSpecial(value);
         retval += data;

      } else {
         retval += QChar32(caseValue);

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
         const char32_t value = static_cast<uint8_t>(str[i]);
         retval.append(value);
      }

   } else {

      for (size_type i = 0; i < numOfChars; ++i) {
         const char32_t value = static_cast<uint8_t>(str[i]);
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
   return CsString::CsString::fromUtf16(str, numOfChars);
}

QString8 QString8::fromUtf16(const QString16 &str)
{
   return fromUtf16((const char16_t *)str.constData(), str.size_storage());
}

QString8 QString8::fromStdWString(const std::wstring &str, size_type numOfChars)
{
   QString8 retval;

   if (sizeof(wchar_t) == sizeof(char16_t)) {
      retval = fromUtf16((const char16_t *)&str[0], numOfChars);

   } else {
      for (wchar_t ch : str) {
         if (numOfChars == 0) {
            break;
         }

         retval.push_back(static_cast<char32_t>(ch));
         --numOfChars;
      }
   }

   return retval;
}

QString8 QString8::fromStdString(const std::string &str, size_type numOfChars)
{
   QString8 retval;

   for (char ch : str) {
      if (numOfChars == 0) {
         break;
      }

      retval.push_back(ch);
      --numOfChars;
   }

   return retval;
}

QString8::const_iterator QString8::indexOfFast(const QRegularExpression8 &regExp, const_iterator from) const
{
   QRegularExpressionMatch8 match = regExp.match(*this, from);

   if (match.hasMatch())  {
      return match.capturedStart(0);
   }

   return end();
}

QString8 QString8::left(size_type numOfChars) const
{
   if (numOfChars < 0) {
      return *this;
   }

   const_iterator iter = cbegin();

   for (size_type i = 0; i < numOfChars && iter != cend(); ++i)  {
      ++iter;
   }

   return QString8(cbegin(), iter);
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

int QString8::localeAwareCompare(QStringView8 str1, QStringView8 str2)
{
   int len1 = str1.size();
   int len2 = str2.size();

   if (len1 == 0 && len2 == 0) {
      return 0;

   } else if (len1 == 0) {
      return -1;

   } else if (len2 == 0) {
      return 1;
   }

#if defined(Q_OS_WIN)
   QString16 tmp1(str1.cbegin(), str1.cend());
   QString16 tmp2(str2.cbegin(), str2.cend());

   int retval = CompareString(GetUserDefaultLCID(), 0, (wchar_t *)tmp1.constData(), tmp1.size_storage(),
        (wchar_t *)tmp2.constData(), tmp2.size_storage());

   switch (retval) {
      case CSTR_LESS_THAN:
         return -1;

      case CSTR_GREATER_THAN:
         return 1;

      default:
         return 0;
   }

#elif defined (Q_OS_DARWIN)

   const CFStringRef mac_str1 = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
                  reinterpret_cast<const UInt8 *>(str1.charData()), str1.size_storage(), kCFStringEncodingUTF8, false, kCFAllocatorNull);

   const CFStringRef mac_str2 = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
                  reinterpret_cast<const UInt8 *>(str2.charData()), str2.size_storage(), kCFStringEncodingUTF8, false, kCFAllocatorNull);

   const int result = CFStringCompare(mac_str1, mac_str2, kCFCompareLocalized);

   CFRelease(mac_str1);
   CFRelease(mac_str2);

   return result;

#elif defined(Q_OS_UNIX)
   // may be a bit slow however it is correct

   std::wstring tmp1(str1.toString().toStdWString());
   std::wstring tmp2(str2.toString().toStdWString());

   int delta = wcscoll(tmp1.c_str(), tmp2.c_str());

   if (delta == 0) {
      delta = compare(str1, str2);
   }

   return delta;

#else
   return compare(str1, str2);

#endif
}

QString8 QString8::mid(size_type indexStart, size_type numOfChars) const
{
   return substr(indexStart, numOfChars);
}

QString8 QString8::mid(const_iterator iter, size_type numOfChars) const
{
   return midView(iter, numOfChars);
}

QStringView8 QString8::midView(size_type indexStart, size_type numOfChars) const
{
   const_iterator iter_begin = cbegin();
   const_iterator iter_end;

   iter_begin = advance(iter_begin, indexStart);

   if (iter_begin == cend()) {
      return QStringView8();
   }

   if (numOfChars >= 0) {
      iter_end = iter_begin;
      iter_end = advance(iter_begin, numOfChars);

   } else {
      iter_end = cend();

   }

   return QStringView8(iter_begin, iter_end);
}

QStringView8 QString8::midView(const_iterator iter, size_type numOfChars) const
{
   const_iterator iter_end;

   if (iter == cend()) {
      return QStringView8();
   }

   if (numOfChars >= 0) {
      iter_end = iter;

      for (size_type i = 0; i < numOfChars && iter_end != cend(); ++i)  {
         ++iter_end;
      }

   } else {
      iter_end = cend();

   }

   return QStringView8(iter, iter_end);
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

   auto iter     = indexOfFast(before, cbegin(), cs);
   size_type len = before.size();

   while (iter != cend()) {
      auto last = iter + len;

      iter = erase(iter, last);
      iter = CsString::CsString::insert(iter, after);
      iter = iter.advance_storage(after.size_storage());

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

QString8 &QString8::replace(const QRegularExpression8 &regExp, const QString8 &after)
{
   QRegularExpressionMatch8 match = regExp.match(*this);
   QRegularExpressionMatch8 splitMatch;

   static QRegularExpression8 regSplit("(.*?)(\\\\[0-9])");
   bool noCapture = true;

   auto iter_start = after.indexOfFast('\\');

   if (iter_start != after.end() && iter_start != after.end() - 1) {
      splitMatch = regSplit.match(after);

      if (splitMatch.hasMatch()) {
         noCapture = false;
      }
   }

   if (noCapture) {

      while (match.hasMatch())  {
         auto first = match.capturedStart(0);
         auto last  = match.capturedEnd(0);

         auto iter = this->erase(first, last);
         iter  = CsString::CsString::insert(iter, after);
         iter  = iter.advance_storage(after.size_storage());

         match = regExp.match(*this, iter);
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

         auto iter = this->erase(first, last);

         for (const auto &item : list) {
            if (item == "\\0") {
               iter = CsString::CsString::insert(iter, saveCapture[0]);
               iter = iter.advance_storage(saveCapture[0].size_storage());

            } else if (item == "\\1") {
               iter = CsString::CsString::insert(iter, saveCapture[1]);
               iter = iter.advance_storage(saveCapture[1].size_storage());

            } else if (item == "\\2") {
               iter = CsString::CsString::insert(iter, saveCapture[2]);
               iter = iter.advance_storage(saveCapture[2].size_storage());

            } else if (item == "\\3") {
               iter = CsString::CsString::insert(iter, saveCapture[3]);
               iter = iter.advance_storage(saveCapture[3].size_storage());

            } else if (item == "\\4") {
               iter = CsString::CsString::insert(iter, saveCapture[4]);
               iter = iter.advance_storage(saveCapture[4].size_storage());

            } else if (item == "\\5") {
               iter = CsString::CsString::insert(iter, saveCapture[5]);
               iter = iter.advance_storage(saveCapture[5].size_storage());

            } else if (item == "\\6") {
               iter = CsString::CsString::insert(iter, saveCapture[6]);
               iter = iter.advance_storage(saveCapture[6].size_storage());

            } else if (item == "\\7") {
               iter = CsString::CsString::insert(iter, saveCapture[7]);
               iter = iter.advance_storage(saveCapture[7].size_storage());

            } else if (item == "\\8") {
               iter = CsString::CsString::insert(iter, saveCapture[8]);
               iter = iter.advance_storage(saveCapture[8].size_storage());

            } else if (item == "\\9") {
               iter = CsString::CsString::insert(iter, saveCapture[9]);
               iter = iter.advance_storage(saveCapture[9].size_storage());

            } else {
               iter = CsString::CsString::insert(iter, item);
               iter = iter.advance_storage(item.size_storage());

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

   const_iterator iter = cend();

   for (size_type i = 0; i < numOfChars; ++i) {
      if (iter == cbegin()) {
         break;
      }

      --iter;
   }

   return QString8(iter, cend());
}

QStringView8 QString8::rightView(size_type numOfChars) const
{
   if (numOfChars < 0) {
      return QStringView8(cbegin(), cend());
   }

   const_iterator iter = cend();

   for (size_type i = 0; i < numOfChars; ++i) {
      if (iter == cbegin()) {
         break;
      }

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

   if (first_iter == last_iter)  {
      // string was all whitespace
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

   if (first_iter == last_iter)  {
      // string was all whitespace
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
   return convertCase<QUnicodeTables::CaseFoldTraits>(*this);
}

QString8 QString8::toCaseFolded() &&
{
   return convertCase<QUnicodeTables::CaseFoldTraits>(*this);
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
    return convertCase<QUnicodeTables::LowerCaseTraits>(*this);
}

QString8 QString8::toLower() &&
{
    return convertCase<QUnicodeTables::LowerCaseTraits>(*this);
}

QString8 QString8::toUpper() const &
{
    return convertCase<QUnicodeTables::UpperCaseTraits>(*this);
}

QString8 QString8::toUpper() &&
{
    return convertCase<QUnicodeTables::UpperCaseTraits>(*this);
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
   return QString16(this->begin(), this->end());
}

std::wstring QString8::toStdWString() const
{
   std::wstring retval;

   if (sizeof(wchar_t) == sizeof(char16_t)) {
      QString16 tmp = this->toUtf16();
      retval = std::wstring(reinterpret_cast<const wchar_t *>(tmp.constData()), tmp.size_storage());

   } else {
      for (QChar32 c : *this) {
         const wchar_t value = c.unicode();
         retval.push_back(value);
      }
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
QDataStream &operator>>(QDataStream &stream, QString8 &str)
{
   char *tmp;
   uint len;

   stream.readBytes(tmp, len);
   str = QString8::fromUtf8(tmp, len);
   delete [] tmp;

   return stream;
}

QDataStream &operator<<(QDataStream &stream, const QString8 &str)
{
   stream.writeBytes(str.constData(), str.size_storage());
   return stream;
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

   static constexpr const int NFQC_YES      = 0;
   // static constexpr const int NFQC_NO    = 1;
   // static constexpr const int NFQC_MAYBE = 3;

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
static const char32_t * cs_internal_decompose_2(char32_t ucs4, int *length, int *tag, char32_t *buffer)
{
    if (ucs4 >= Hangul_Constants::Hangul_SBase && ucs4 < Hangul_Constants::Hangul_SBase + Hangul_Constants::Hangul_SCount) {
        // compute Hangul syllable decomposition as per UAX #15

        const uint SIndex = ucs4 - Hangul_Constants::Hangul_SBase;
        buffer[0] = Hangul_Constants::Hangul_LBase + SIndex / Hangul_Constants::Hangul_NCount;                                     // L
        buffer[1] = Hangul_Constants::Hangul_VBase + (SIndex % Hangul_Constants::Hangul_NCount) / Hangul_Constants::Hangul_TCount; // V
        buffer[2] = Hangul_Constants::Hangul_TBase + SIndex % Hangul_Constants::Hangul_TCount;                                     // T

        *length = buffer[2] == Hangul_Constants::Hangul_TBase ? 2 : 3;
        *tag = QChar32::Canonical;

        return buffer;
    }

    const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
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

QString8 cs_internal_decompose(QString8::const_iterator first_iter, QString8::const_iterator last_iter,
                  bool canonical, QChar32::UnicodeVersion version)
{
   // method two
   QString8 retval;

   int length;
   int tag;

   char32_t buffer[3];

   if (first_iter == last_iter) {
      return retval;
   }

   for (auto iter = first_iter; iter != last_iter; ++iter) {
      QChar32 uc = *iter;

      if (uc.unicodeVersion() > version) {
        retval.append(uc);
        continue;
      }

      const char32_t *strDecomp = cs_internal_decompose_2(uc.unicode(), &length, &tag, buffer);

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

static char32_t inline cs_internal_ligature(char32_t u1, char32_t u2)
{
   char32_t retval = U'\0';

   if (u1 >= Hangul_Constants::Hangul_LBase && u1 <= Hangul_Constants::Hangul_SBase + Hangul_Constants::Hangul_SCount) {
     // compute Hangul syllable composition as per UAX #15
     // hangul L-V pair
     const uint LIndex = u1 - Hangul_Constants::Hangul_LBase;

     if (LIndex < Hangul_Constants::Hangul_LCount) {
         const uint VIndex = u2 - Hangul_Constants::Hangul_VBase;

         if (VIndex < Hangul_Constants::Hangul_VCount) {
            return Hangul_Constants::Hangul_SBase + (LIndex * Hangul_Constants::Hangul_VCount + VIndex) * Hangul_Constants::Hangul_TCount;
         }
     }

     // hangul LV-T pair
     const uint SIndex = u1 - Hangul_Constants::Hangul_SBase;
     if (SIndex < Hangul_Constants::Hangul_SCount && (SIndex % Hangul_Constants::Hangul_TCount) == 0) {
         const uint TIndex = u2 - Hangul_Constants::Hangul_TBase;

         if (TIndex <= Hangul_Constants::Hangul_TCount) {
            return u1 + TIndex;
         }
     }
   }

   const unsigned short index = GET_LIGATURE_INDEX(u2);
   if (index == 0xffff) {
      return retval;
   }

   const char32_t *ligatures = QUnicodeTables::uc_ligature_map + index;

   uint32_t length = *ligatures;
   ++ligatures;

   for (uint32_t i = 0; i < length *2; i += 2)  {
      if (ligatures[i] == u1) {
         retval = ligatures[i + 1];
         break;
      }
   }

   return retval;
}

static QString8 cs_internal_compose(const QString8 &str, QChar32::UnicodeVersion version)
{
   // method four
   QString8 retval;

   auto first_iter = str.cbegin();
   auto last_iter  = str.cend();

   QString8::const_iterator iterBeg = last_iter;
   QString8::const_iterator iterEnd = last_iter;

   char32_t codePointBeg = 0;       // starting code point value
   int lastCombining = 255;         // to prevent combining > lastCombining

   for (auto iter = first_iter; iter != last_iter; ++iter) {
      QChar32 uc = *iter;
      char32_t ucValue = uc.unicode();

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

         if (ligature != U'\0') {
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

