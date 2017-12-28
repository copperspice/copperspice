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

#ifndef QSTRINGVIEW8_H
#define QSTRINGVIEW8_H

#include <cs_string_view.h>

class QString8;
class QString16;

/*
namespace QUnicodeTables {
   struct CasefoldTraits;
}
*/

template <typename S>
class QStringView;

using QStringView8  = QStringView<QString8>;
using QStringView16 = QStringView<QString16>;

template <typename S>
class Q_CORE_EXPORT QStringView : public CsString::CsBasicStringView<S>
{
   public:
      using difference_type = std::ptrdiff_t;
      using size_type       = std::ptrdiff_t;
      using value_type      = const QChar32;

      using const_iterator  = typename S::const_iterator;
      using iterator        = typename S::const_iterator;

      using const_reverse_iterator = typename S::const_reverse_iterator;
      using reverse_iterator       = typename S::const_reverse_iterator;

      QStringView() = default;

      QStringView(const_iterator begin, const_iterator end)
         : CsString::CsBasicStringView<S>(begin, end)
      { }

      QStringView(const QStringView &other) = default;
      QStringView(QStringView &&other) = default;

      QStringView(const S &str)
         : QStringView(str.cbegin(), str.cend())
      { }

      // methods
      QChar32 at(size_type n) const {
         return CsString::CsBasicStringView<S>::at(n);
      }

      QChar32 back() const {
         return CsString::CsBasicStringView<S>::back();
      }

      QStringView<S> chopped(size_type numOfChars) const;

      bool empty() const {
         return CsString::CsBasicStringView<S>::empty();
      }

      bool endsWith(QChar32 ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool endsWith(QStringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      QChar32 first() const {
         return CsString::CsBasicStringView<S>::front();
      }

      QChar32 front() const{
         return CsString::CsBasicStringView<S>::front();
      }

      bool isEmpty() const {
         return CsString::CsBasicStringView<S>::empty();
      }

      QChar32 last() const {
         return CsString::CsBasicStringView<S>::back();
      }

      QStringView<S> left(size_type numOfChars) const;

      size_type length() const {
         return CsString::CsBasicStringView<S>::size();
      }

      QStringView<S> mid(size_type index, size_type numOfChars = -1) const;
      QStringView<S> right(size_type numOfChars) const;

      size_type size() const{
         return CsString::CsBasicStringView<S>::size();
      }

      bool startsWith(QChar32 ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool startsWith(QStringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      S toCaseFolded() const Q_REQUIRED_RESULT;

      QByteArray toLatin1() const;
      QByteArray toUtf8() const;

      // hold
      // QString16 toUtf16() const;

      QStringView<S> trimmed() const;
      QChar32 operator[](size_type n) const;

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
};

template <typename S>
QStringView<S> QStringView<S>::chopped(size_type numOfChars) const
{
   if (numOfChars < 0) {
      return QStringView8(cbegin(), cend());
   }

   const_iterator iter = cend();

   for (size_type i = 0; i < numOfChars && iter != cbegin(); ++i)  {
      --iter;
   }

   return QStringView8(cbegin(), iter);
}

template <typename S>
bool QStringView<S>::endsWith(QChar32 c, Qt::CaseSensitivity cs) const
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
      return QStringView8(cbegin(), cend());
   }

   const_iterator iter = cbegin();

   for (size_type i = 0; i < numOfChars && iter != cend(); ++i)  {
      ++iter;
   }

   return QStringView8(cbegin(), iter);
}

template <typename S>
QStringView<S> QStringView<S>::mid(size_type index, size_type numOfChars) const
{
   const_iterator iter_begin = cbegin();
   const_iterator iter_end;

   for (size_type i = 0; i < index && iter_begin != cend(); ++i)  {
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

template <typename S>
QStringView<S> QStringView<S>::right(size_type numOfChars) const
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

template <typename S>
bool QStringView<S>::startsWith(QChar32 c, Qt::CaseSensitivity cs) const
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

template <typename S>
bool QStringView<S>::startsWith(QStringView str, Qt::CaseSensitivity cs) const
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

/*

template <typename S>
S QStringView<S>::toCaseFolded() const
{
   return convertCase<QUnicodeTables::CasefoldTraits, S>(*this);
}

*/

template <typename S>
QByteArray QStringView<S>::toLatin1() const
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

template <typename S>
QByteArray QStringView<S>::toUtf8() const
{
   QByteArray retval;

   for (QChar32 c : *this) {
      CsString::utf8::insert(retval, retval.end(), c);
   }

   return retval;
}


/*  broom - hold

template <>
QByteArray QStringView<QString8>::toUtf8() const
{
   QByteArray retval = QByteArray(cbegin().codePointBegin(), cend().codePointBegin());
   return retval;
}

*/

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
}

template <typename S>
QChar32 QStringView<S>::operator[](size_type n) const
{
   return CsString::CsBasicStringView<S>::at(n);
}

#endif