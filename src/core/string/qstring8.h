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

#ifndef QSTRING8_H
#define QSTRING8_H

#define CS_STRING_ALLOW_UNSAFE

#include <qglobal.h>
#include <cs_string.h>
#include <qchar32.h>
#include <qbytearray.h>

class QRegExp;

class Q_CORE_EXPORT QChar32Arrow : public CsString::CsCharArrow
{
   public:
      QChar32Arrow (CsString::CsCharArrow c)
         : CsString::CsCharArrow(c)
      { }

      const QChar32 *operator->() const {
         static_assert(std::is_standard_layout<CsString::CsChar>::value, "Invalid reinterpret_cast for QChar32Arrow");
         static_assert(sizeof(QChar32) == sizeof(CsString::CsChar), "Invalid reinterpret_cast for QChar32Arrow");

         return reinterpret_cast<const QChar32 *>(CsString::CsCharArrow::operator->());
      }
};

class Q_CORE_EXPORT QString8 : public CsString::CsString
{
   public:
      class iterator : public CsString::CsString::iterator
      {
       public:
         using pointer     = QChar32 *;
         using reference   = QChar32 &;
         using value_type  = QChar32;

         iterator() = default;

         iterator(CsString::CsString::iterator iter)
            : CsString::CsString::iterator(std::move(iter)) {
         }

         // operators
         QChar32 operator*() const {
            return CsString::CsString::iterator::operator*();
         }

         QChar32Arrow operator->() const {
            return CsString::CsString::iterator::operator->();
         }

         bool operator==(iterator other) const {
            return CsString::CsString::iterator::operator==(other);
         }

         bool operator!=(iterator other) const {
            return CsString::CsString::iterator::operator!=(other);
         }

         iterator &operator+=(size_type n) {
            CsString::CsString::iterator::operator+=(n);
            return *this;
         }

         iterator &operator-=(size_type n) {
            CsString::CsString::iterator::operator-=(n);
            return *this;
         }

         iterator operator+(size_type n) const {
            return CsString::CsString::iterator::operator+(n);
         }

         iterator operator-(size_type n) const {
            return CsString::CsString::iterator::operator-(n);
         }

         iterator &operator++() {
            CsString::CsString::iterator::operator++();
            return *this;
         }

         iterator operator++(int n) {
            return CsString::CsString::iterator::operator++(n);
         }

         iterator &operator--() {
            CsString::CsString::iterator::operator--();
            return *this;
         }

         iterator operator--(int n) {
            return CsString::CsString::iterator::operator--(n);
         }
      };

      class const_iterator : public CsString::CsString::const_iterator
      {
       public:
         using pointer           = const QChar32 *;
         using reference         = const QChar32 &;
         using value_type        = const QChar32;

         const_iterator() = default;

         const_iterator(CsString::CsString::const_iterator iter)
            : CsString::CsString::const_iterator(std::move(iter)) {
         }

         const_iterator(iterator iter)
            : CsString::CsString::const_iterator(std::move(iter)) {
         }

         // operators
         const QChar32 operator*() const {
            return CsString::CsString::const_iterator::operator*();
         }

         QChar32Arrow operator->() const {
            return CsString::CsString::const_iterator::operator->();
         }

         bool operator==(const_iterator other) const {
            return CsString::CsString::const_iterator::operator==(other);
         }

         bool operator!=(const_iterator other) const {
            return CsString::CsString::const_iterator::operator!=(other);
         }

         const_iterator &operator+=(size_type n) {
            CsString::CsString::const_iterator::operator+=(n);
            return *this;
         }

         const_iterator &operator-=(size_type n) {
            CsString::CsString::const_iterator::operator-=(n);
            return *this;
         }

         const_iterator operator+(size_type n) const {
            return CsString::CsString::const_iterator::operator+(n);
         }

         const_iterator operator-(size_type n) const {
            return CsString::CsString::const_iterator::operator-(n);
         }

         const_iterator &operator++() {
            CsString::CsString::const_iterator::operator++();
            return *this;
         }

         const_iterator operator++(int n) {
            return CsString::CsString::const_iterator::operator++(n);
         }

         const_iterator &operator--() {
            CsString::CsString::const_iterator::operator--();
            return *this;
         }

         const_iterator operator--(int n) {
            return CsString::CsString::const_iterator::operator--(n);
         }
      };

      enum NormalizationForm {
         NormalizationForm_D,
         NormalizationForm_C,
         NormalizationForm_KD,
         NormalizationForm_KC
      };

      enum SplitBehavior { KeepEmptyParts, SkipEmptyParts };

      using Iterator        = iterator;
      using ConstIterator   = const_iterator;

      using difference_type = ssize_t;
      using size_type       = ssize_t;
      using value_type      = QChar32;

      using const_reverse_iterator = std::reverse_iterator<const_iterator>;
      using reverse_iterator       = std::reverse_iterator<iterator>;

      QString8() = default;
      QString8(const QString8 &other) = default;
      QString8(QString8 &&other) = default;

      QString8(QChar32 c);
      QString8(size_type size, QChar32 c);

      QString8(const QChar32 *data, size_type size = -1)  {

         if (size == -1) {
            const QChar32 *p = data;

            while (p->unicode() != 0) {
               ++p;
            }

            CsString::CsString::append(data, p);

         } else {
            CsString::CsString::append(data, data + size);

         }
      }

/*
      // for a const char * and char *
      template <typename T, typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      CsBasicString(const T &str, const A &a = A());
*/

      // for an array of chars
      template <int N>
      QString8(const char (&str)[N])
         : CsString::CsString(str)
      { }

      template <typename Iterator>
      QString8(Iterator begin, Iterator end)
         : CsString::CsString(begin, end)
      { }

      // internal
      QString8(const CsString::CsString &other)
         : CsString::CsString(other)
      {
      }

      // internal
      QString8(CsString::CsString &&other)
         : CsString::CsString(std::move(other))
      {
      }

      ~QString8() = default;

      // methods
      QString8 &append(QChar32 c)  {
         CsString::CsString::append(c);
         return *this;
      }

      QString8 &append(const QChar32 *data, size_type size)  {
         CsString::CsString::append(data, data + size);
         return *this;
      }

      QString8 &append(const QString8 &other)  {
         CsString::CsString::append(other);
         return *this;
      }

      // internal
      using CsString::CsString::append;

      QChar32 at(size_type index) const;

      void chop(size_type n);

      void clear() {
         CsString::CsString::clear();
      }

      size_type count() const {
         return CsString::CsString::size();
      }

      const char *constData() const {
         return reinterpret_cast<const char *>(CsString::CsString::constData());
      }

      const char *data() const {
         return constData();
      }

      QString8 &fill(QChar32 c, size_type size = -1);

      QString8 &insert (size_type index, const QString8 & other)  {
         CsString::CsString::insert(index, other);
         return *this;
      }

      QString8 &insert(size_type index, QChar32 c) {
         CsString::CsString::insert(index, 1, c);
         return *this;
      }

      QString8 &insert(size_type index, const QChar32 *data, size_type size) {
         CsString::CsString::insert(begin() + index, data, data + size);
         return *this;
      }

      bool isEmpty() const;

      // internal
      bool isSimpleText() const;

      QString8 left(size_type numOfChars) const Q_REQUIRED_RESULT;
      QString8 leftJustified(size_type width, QChar32 fill = UCHAR(' '), bool trunc = false) const Q_REQUIRED_RESULT;

      size_type length() const {
         return CsString::CsString::size();
      }

      QString8 mid(size_type index, size_type numOfChars = -1) const Q_REQUIRED_RESULT;

      QString8 normalized(QString8::NormalizationForm mode, QChar32::UnicodeVersion version = QChar32::Unicode_Unassigned)
                  const Q_REQUIRED_RESULT;

      QString8 &prepend(const QString8 &other) {
         CsString::CsString::insert(begin(), other);
         return *this;
      }

      QString8 &prepend(QChar32 c) {
         CsString::CsString::insert(begin(), c);
         return *this;
      }

      QString8 &prepend(const QChar32 *data, size_type size)  {
         CsString::CsString::insert(begin(), data, data + size);
         return *this;
      }

      void push_back(QChar32 c) {
         append(c);
      }

      void push_back(const QString8 &other) {
         append(other);
      }

      void push_front(QChar32 c) {
         prepend(c);
      }

      void push_front(const QString8 &other) {
         prepend(other);
      }

      QString8 repeated(size_type count) const Q_REQUIRED_RESULT;

      QString8 &remove(size_type index, size_type size);

      QString8 &replace(size_type index, size_type numOfChars, QChar32 c) {
         CsString::CsString::replace(index, numOfChars, 1, c);
         return *this;
      }

      QString8 &replace(size_type index, size_type numOfChars, const QString8 &other) {
         CsString::CsString::replace(index, numOfChars, other);
         return *this;
      }

      void resize(size_type size) {
         return CsString::CsString::resize(size);
      }

      void resize(size_type size, QChar32 c) {
         return CsString::CsString::resize(size, c);
      }

      QString8 right(size_type numOfChars) const Q_REQUIRED_RESULT;
      QString8 rightJustified(size_type width, QChar32 fill = UCHAR(' '), bool trunc = false) const Q_REQUIRED_RESULT;

      QString8 simplified() const & Q_REQUIRED_RESULT;
      QString8 simplified() && Q_REQUIRED_RESULT;

      size_type size() const {
         return CsString::CsString::size();
      }

      void squeeze() {
         return CsString::CsString::shrink_to_fit();
      }

      void swap(QString8 &other) {
         CsString::CsString::swap(other);
      }

      QString8 toHtmlEscaped() const;

      QString8 toCaseFolded() const & Q_REQUIRED_RESULT;
      QString8 toCaseFolded() && Q_REQUIRED_RESULT;

      QString8 toLower() const & Q_REQUIRED_RESULT;
      QString8 toLower() && Q_REQUIRED_RESULT;

      QString8 toUpper() const & Q_REQUIRED_RESULT;
      QString8 toUpper() && Q_REQUIRED_RESULT;

      QString8 trimmed() const & Q_REQUIRED_RESULT;
      QString8 trimmed() && Q_REQUIRED_RESULT;

      void truncate(size_type length);

      const uint8_t *utf8() const {
         return CsString::CsString::constData();
      }

      // iterators
      iterator begin() {
         return CsString::CsString::begin();
      }

      const_iterator begin() const {
         return CsString::CsString::cbegin();
      }

      const_iterator cbegin() const {
         return CsString::CsString::cbegin();
      }

      const_iterator constBegin() const {
         return CsString::CsString::cbegin();
      }

      iterator end() {
         return CsString::CsString::end();
      }

      const_iterator end() const {
         return CsString::CsString::cend();
      }

      const_iterator cend() const {
         return CsString::CsString::cend();
      }

      const_iterator constEnd() const {
         return CsString::CsString::cend();
      }

      reverse_iterator rbegin()  {
         return CsString::CsString::rbegin();
      }

      const_reverse_iterator rbegin() const {
         return CsString::CsString::rbegin();
      }

      reverse_iterator rend()  {
         return CsString::CsString::rend();
      }

      const_reverse_iterator rend() const {
         return CsString::CsString::rend();
      }

      const_reverse_iterator crbegin() const {
         return CsString::CsString::crbegin();
      }

      const_reverse_iterator crend() const {
         return CsString::CsString::crend();
      }

      // operators
      using CsString::CsString::operator=;      // internal
      using CsString::CsString::operator+=;     // internal

      QString8 &operator=(QChar32 c)  {
         CsString::CsString::operator=(c);
         return *this;
      }

      QString8 &operator+=(QChar32 c)  {
         CsString::CsString::operator+=(c);
         return *this;
      }

      QString8 &operator+=(QChar32::SpecialCharacter c) {
         append(QChar32(c));
         return *this;
      }

      QString8 &operator=(const QString8 &other) = default;
      QString8 &operator=(QString8 && other) = default;

   private:

};

inline void swap(QString8 &a, QString8 &b) {
   a.swap(b);
}

QString8 cs_internal_string_normalize(const QString8 &data, QString8::NormalizationForm mode,
                  QChar32::UnicodeVersion version, int from);

#endif