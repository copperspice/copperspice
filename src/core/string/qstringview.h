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

#ifndef QSTRINGVIEW_H
#define QSTRINGVIEW_H

#define CS_STRING_ALLOW_UNSAFE

#include <cs_string_view.h>

#include <qglobal.h>
#include <qbytearray.h>
#include <qchar32.h>
#include <qstringfwd.h>

class QStringParser;

Q_CORE_EXPORT std::pair<char32_t, const char32_t *> cs_internal_convertCaseTrait(int trait, char32_t value);

#if ! defined (CS_DOXYPRESS)
namespace Cs {
#endif

template <typename S>
class QStringView : public CsString::CsBasicStringView<S>
{
   public:
      using difference_type = typename S::difference_type;
      using size_type       = typename S::difference_type;
      using value_type      = typename S::value_type;

      using const_iterator  = typename S::const_iterator;
      using iterator        = typename S::const_iterator;

      using reverse_iterator       = typename S::const_reverse_iterator;
      using const_reverse_iterator = typename S::const_reverse_iterator;

      QStringView() = default;

      QStringView(const_iterator begin, const_iterator end)
         : CsString::CsBasicStringView<S>(begin, end)
      {
      }

      QStringView(const QStringView &other) = default;
      QStringView(QStringView &&other) = default;

      QStringView(const S &str)
         : QStringView(str.cbegin(), str.cend())
      {
      }

      QStringView(S &&str) = delete;

      // methods
      value_type at(size_type n) const {
         return CsString::CsBasicStringView<S>::at(n);
      }

      value_type back() const {
         return CsString::CsBasicStringView<S>::back();
      }

      const typename S::storage_type * charData() const {
         return reinterpret_cast<const typename S::storage_type *>(&(begin().codePointBegin())[0]);
      }

      void chop(size_type numOfChars);

      int compare(QStringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
         return S::compare(*this, str, cs);
      }

      size_type count() const {
         return CsString::CsBasicStringView<S>::size();
      }

      size_type count(value_type ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      size_type count(const S &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      size_type count(QStringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      size_type count(const QRegularExpression<S> &regExp) const;

      bool contains(value_type ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool contains(const S &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool contains(QStringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      bool contains(const QRegularExpression<S> &regExp) const {
         return indexOfFast(regExp) != end();
      }

      bool empty() const {
         return CsString::CsBasicStringView<S>::empty();
      }

      template <typename C = value_type>
      bool endsWith(value_type ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      bool endsWith(S str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
         return endsWith(QStringView<S>(str), cs);
      }

      bool endsWith(QStringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      value_type first() const {
         return CsString::CsBasicStringView<S>::front();
      }

      value_type front() const{
         return CsString::CsBasicStringView<S>::front();
      }

      bool isEmpty() const {
         return CsString::CsBasicStringView<S>::empty();
      }

      // using iterators
      const_iterator indexOfFast(value_type ch) const {
         return indexOfFast(ch, cbegin(), Qt::CaseSensitive);
      }

      const_iterator indexOfFast(value_type ch, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsBasicStringView<S>::find_fast(ch, from);

         } else {
            return cs_internal_find_fast(ch, from);

         }
      }

      const_iterator indexOfFast(const S &str) const {
         return indexOfFast(str, cbegin(), Qt::CaseSensitive);
      }

      const_iterator indexOfFast(const S &str, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsBasicStringView<S>::find_fast(str, from);

         } else {
            return cs_internal_find_fast(str, from);
         }
      }

      const_iterator indexOfFast(QStringView<S> str, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsBasicStringView<S>::find_fast(str, from);

         } else {
            return cs_internal_find_fast(str, from);
         }
      }

      const_iterator lastIndexOfFast(value_type ch) const {
         return lastIndexOfFast(ch, cend(), Qt::CaseSensitive);
      }

      const_iterator lastIndexOfFast(value_type ch, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
      {
         if (cs == Qt::CaseSensitive) {
            return CsString::CsBasicStringView<S>::rfind_fast(ch, from);

         } else {
            return cs_internal_rfind_fast(ch, from);

         }
      }

      const_iterator lastIndexOfFast(const S &str) const {
         return lastIndexOfFast(str, cend(), Qt::CaseSensitive);
      }

      const_iterator lastIndexOfFast(const S &str, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsBasicStringView<S>::rfind_fast(str, from);

         } else {
            return cs_internal_rfind_fast(str, from);
         }
      }

      const_iterator lastIndexOfFast(QStringView<S> str, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsBasicStringView<S>::rfind_fast(str, from);

         } else {
            return cs_internal_rfind_fast(str, from);
         }
      }

      value_type last() const {
         return CsString::CsBasicStringView<S>::back();
      }

      QStringView<S> left(size_type numOfChars) const;

      size_type length() const {
         return CsString::CsBasicStringView<S>::size();
      }

      QStringView<S> mid(size_type indexStart, size_type numOfChars = -1) const;
      QStringView<S> right(size_type numOfChars) const;

      size_type size() const{
         return CsString::CsBasicStringView<S>::size();
      }

      size_type size_storage() const{
         return end().codePointBegin() - begin().codePointBegin();
      }

      template <typename C = value_type>
      bool startsWith(value_type ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      bool startsWith(S str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
         return startsWith(QStringView<S>(str), cs);
      }

      bool startsWith(QStringView<S> str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      template <typename R, typename SP = QStringParser>
      R toInteger(bool *ok = nullptr, int base = 10) const
      {
         return SP::template toInteger<R>(*this, ok, base);
      }

      template <typename SP = QStringParser>
      double toDouble(bool *ok = nullptr) const
      {
         return SP::toDouble(*this, ok);
      }

      template <typename SP = QStringParser>
      float toFloat(bool *ok = nullptr) const
      {
         return SP::toFloat(*this, ok);
      }

      [[nodiscard]] S toCaseFolded() const;
      [[nodiscard]] S toLower() const;
      [[nodiscard]] S toUpper() const;

      QByteArray toLatin1() const;
      QByteArray toUtf8() const;

      S toString() const {
         return S(cbegin(), cend());
      }

      // on hold
      // QString16 toUtf16() const;

      QStringView<S> trimmed() const;
      value_type operator[](size_type n) const;

      QStringView &operator=(const QStringView &other) = default;
      QStringView &operator=(QStringView &&other) = default;

      // iterators
      iterator begin() {
         return CsString::CsBasicStringView<S>::begin();
      }

      const_iterator begin() const {
         return CsString::CsBasicStringView<S>::cbegin();
      }

      const_iterator cbegin() const {
         return CsString::CsBasicStringView<S>::cbegin();
      }

      const_iterator constBegin() const {
         return CsString::CsBasicStringView<S>::cbegin();
      }

      iterator end() {
         return CsString::CsBasicStringView<S>::end();
      }

      const_iterator end() const {
         return CsString::CsBasicStringView<S>::cend();
      }

      const_iterator cend() const {
         return CsString::CsBasicStringView<S>::cend();
      }

      const_iterator constEnd() const {
         return CsString::CsBasicStringView<S>::cend();
      }

      reverse_iterator rbegin()  {
         return CsString::CsBasicStringView<S>::rbegin();
      }

      const_reverse_iterator rbegin() const {
         return CsString::CsBasicStringView<S>::rbegin();
      }

      reverse_iterator rend()  {
         return CsString::CsBasicStringView<S>::rend();
      }

      const_reverse_iterator rend() const {
         return CsString::CsBasicStringView<S>::rend();
      }

      const_reverse_iterator crbegin() const {
         return CsString::CsBasicStringView<S>::crbegin();
      }

      const_reverse_iterator crend() const {
         return CsString::CsBasicStringView<S>::crend();
      }

   private:
      template <typename C = value_type>
      const_iterator cs_internal_find_fast(value_type ch,  const_iterator iter_begin) const;

      template <typename C = value_type>
      const_iterator cs_internal_rfind_fast(value_type ch, const_iterator iter_begin) const;

      const_iterator cs_internal_find_fast(const S &str,  const_iterator iter_begin) const;
      const_iterator cs_internal_rfind_fast(const S &str, const_iterator iter_begin) const;

      S convertCase(int trait) const;
};

template <typename S>
template <typename C>
typename QStringView<S>::const_iterator QStringView<S>::cs_internal_find_fast(value_type ch, const_iterator iter_begin) const
{
   const_iterator iter_end = cend();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   auto iter = iter_begin;
   S strFolded = C(ch).toCaseFolded();

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

template <typename S>
typename QStringView<S>::const_iterator QStringView<S>::cs_internal_find_fast(const S &str, const_iterator iter_begin) const
{
   const_iterator iter_end = cend();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   if (str.empty()) {
      return iter_begin;
   }

   auto iter = iter_begin;
   S strFolded = str.toCaseFolded();

   while (iter != iter_end)   {
      // review: account for code points which expand when folded

      if (iter->toCaseFolded() == S(strFolded[0]))  {
         auto text_iter    = iter + 1;
         auto pattern_iter = strFolded.cbegin() + 1;

         while (text_iter != iter_end && pattern_iter != strFolded.cend())  {

            if (text_iter->toCaseFolded() == S(*pattern_iter))  {
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

template <typename S>
template <typename C>
typename QStringView<S>::const_iterator QStringView<S>::cs_internal_rfind_fast(value_type ch, const_iterator iter_begin) const
{
   const_iterator iter_end = cend();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   auto iter = iter_begin;
   S strFolded = C(ch).toCaseFolded();

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

template <typename S>
typename QStringView<S>::const_iterator QStringView<S>::cs_internal_rfind_fast(const S &str, const_iterator iter_begin) const
{
   const_iterator iter_end = cend();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   if (str.empty()) {
      return iter_begin;
   }

   auto iter = iter_begin;
   S strFolded = str.toCaseFolded();

   while (iter != iter_end)   {
      // review: account for code points which expand when folded

      if (iter->toCaseFolded() == S(strFolded[0]))  {
         auto text_iter    = iter + 1;
         auto pattern_iter = strFolded.cbegin() + 1;

         while (text_iter != iter_end && pattern_iter != strFolded.cend())  {

            if (text_iter->toCaseFolded() == S(*pattern_iter))  {
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

template <typename S>
void QStringView<S>::chop(size_type numOfChars)
{
   if (numOfChars > 0) {
      auto iter = end();

      for (size_type cnt = 0; cnt < numOfChars; ++cnt) {
         if (iter == begin()) {
            break;
         }

         --iter;
      }

      *this = QStringView<S>(begin(), iter);
   }
}

template <typename S>
typename QStringView<S>::size_type QStringView<S>::count(value_type ch, Qt::CaseSensitivity cs) const
{
   size_type retval = 0;

   if (cs == Qt::CaseSensitive) {
      for (auto uc : *this) {
         if (uc == ch) {
            ++retval;
         }
      }

   } else {
      S tmp = ch.toCaseFolded();

      for (auto uc : *this) {
         if (uc.toCaseFolded() == tmp) {
            ++retval;
         }
      }
   }

   return retval;
}

template <typename S>
typename QStringView<S>::size_type QStringView<S>::count(const S &str, Qt::CaseSensitivity cs) const
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

template <typename S>
typename QStringView<S>::size_type QStringView<S>::count(QStringView<S> str, Qt::CaseSensitivity cs) const
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

template <typename S>
typename QStringView<S>::size_type QStringView<S>::count(const QRegularExpression<S> &regExp) const
{
   size_type retval = 0;

   const_iterator iter     = cbegin();
   const_iterator iter_end = cend();

   QRegularExpressionMatch<S> match;

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

template <typename S>
bool QStringView<S>::contains(value_type ch, Qt::CaseSensitivity cs) const
{
   const_iterator iter      = this->cbegin();
   const_iterator iter_end  = this->cend();

   S other = S(ch);
   iter = indexOfFast(other, iter, cs);

   if (iter != iter_end) {
      return true;
   }

   return false;
}

template <typename S>
bool QStringView<S>::contains(const S &str, Qt::CaseSensitivity cs) const
{
   const_iterator iter      = this->cbegin();
   const_iterator iter_end  = this->cend();

   iter = indexOfFast(str, iter, cs);

   if (iter != iter_end) {
      return true;
   }

   return false;
}

template <typename S>
bool QStringView<S>::contains(QStringView<S> str, Qt::CaseSensitivity cs) const
{
   const_iterator iter      = this->cbegin();
   const_iterator iter_end  = this->cend();

   iter = indexOfFast(str, iter, cs);

   if (iter != iter_end) {
      return true;
   }

   return false;
}

template <typename S>
template <typename C>
bool QStringView<S>::endsWith(value_type ch, Qt::CaseSensitivity cs) const
{
   if (empty()) {
      return false;
   }

   auto iter = end() - 1;

   if (cs == Qt::CaseSensitive) {
      return *iter == ch;

   } else {
      return iter->toCaseFolded() == C(ch).toCaseFolded();

   }
}

template <typename S>
bool QStringView<S>::endsWith(QStringView str, Qt::CaseSensitivity cs) const
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

template <typename S>
QStringView<S> QStringView<S>::left(size_type numOfChars) const
{
   if (numOfChars < 0) {
      return QStringView<S>(cbegin(), cend());
   }

   const_iterator iter = cbegin();

   for (size_type i = 0; i < numOfChars && iter != cend(); ++i)  {
      ++iter;
   }

   return QStringView<S>(cbegin(), iter);
}

template <typename S>
QStringView<S> QStringView<S>::mid(size_type indexStart, size_type numOfChars) const
{
   const_iterator iter_begin = cbegin();
   const_iterator iter_end;

   for (size_type i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (iter_begin == cend()) {
      // index > size()
      return QStringView<S>();
   }

   if (numOfChars >= 0) {
      iter_end = iter_begin;

      for (size_type i = 0; i < numOfChars && iter_end != cend(); ++i)  {
         ++iter_end;
      }

   } else {
      iter_end = cend();

   }

   return QStringView<S>(iter_begin, iter_end);
}

template <typename S>
QStringView<S> QStringView<S>::right(size_type numOfChars) const
{
   if (numOfChars < 0) {
      return QStringView<S>(cbegin(), cend());
   }

   const_iterator iter = cend();

   for (size_type i = 0; i < numOfChars && iter != cbegin(); ++i)  {
      --iter;
   }

   return QStringView<S>(iter, cend());
}

template <typename S>
template <typename C>
bool QStringView<S>::startsWith(value_type ch, Qt::CaseSensitivity cs) const
{
   if (empty()) {
      return false;
   }

   if (cs == Qt::CaseSensitive) {
      return at(0) == ch;

   } else {
      return at(0).toCaseFolded() == C(ch).toCaseFolded();

   }
}

template <typename S>
bool QStringView<S>::startsWith(QStringView<S> str, Qt::CaseSensitivity cs) const
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

         if (iter->toCaseFolded() != uc.toCaseFolded()) {
            return false;
         }

         ++iter;
      }

      return true;
   }
}

template <typename S>
S QStringView<S>::convertCase(int trait) const
{
   S retval;

   for (auto ch : *this)  {
      char32_t value = ch.unicode();
      std::pair<char32_t, const char32_t *> unicodeLookUp = cs_internal_convertCaseTrait(trait, value);

      char32_t caseValue = unicodeLookUp.first;

      if (caseValue == 0 && value != 0) {
         retval += unicodeLookUp.second;

      } else {
         retval += value_type(caseValue);

      }
   }

   return retval;
}

template <typename S>
S QStringView<S>::toCaseFolded() const
{
   return convertCase(1);
}

template <typename S>
S QStringView<S>::toLower() const
{
   return convertCase(2);
}

template <typename S>
S QStringView<S>::toUpper() const
{
   return convertCase(3);
}

template <typename S>
QByteArray QStringView<S>::toLatin1() const
{
   QByteArray retval;

   for (value_type ch : *this) {
      const char32_t value = ch.unicode();

      if (value > 255) {
         retval.append('?' );

      } else {
         retval.append(value);

      }
   }

   return retval;
}

template <typename S>
QByteArray QStringView<S>::toUtf8() const
{
   QByteArray retval;

   for (value_type ch : *this) {
      CsString::utf8::insert(retval, retval.cend(), ch);
   }

   return retval;
}

template <typename S>
QStringView<S> QStringView<S>::trimmed() const
{
   QStringView<S> retval;

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
   retval = QStringView<S>(first_iter, last_iter);

   return retval;
}

template <typename S>
typename QStringView<S>::value_type QStringView<S>::operator[](size_type n) const
{
   return CsString::CsBasicStringView<S>::at(n);
}

template <typename S>
inline bool operator==(QStringView<S> str1, QStringView<S> str2)
{
   return str1.compare(str2) == 0;
}

// remaining operator==() functions are located in qstring8.h and qstring16.h

template <typename S>
inline bool operator!=(QStringView<S> str1, QStringView<S> str2)
{
   return str1.compare(str2) != 0;
}

// remaining operator!=() functions are located in qstring8.h and qstring16.h


#if ! defined (CS_DOXYPRESS)
}  // cs namespace
#endif

Q_CORE_EXPORT QStringView8  make_view(QString8 &&str);
Q_CORE_EXPORT QStringView16 make_view(QString16 &&str);

#endif