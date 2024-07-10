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

#ifndef QSTRING16_H
#define QSTRING16_H

#define CS_STRING_ALLOW_UNSAFE

#include <cs_string.h>

#include <qglobal.h>
#include <qbytearray.h>
#include <qchar32.h>
#include <qstringview.h>
#include <qstringfwd.h>

class QStringParser;

#include <cstddef>
#include <string>

#ifdef Q_OS_DARWIN
   using CFStringRef = const struct __CFString *;

#  ifdef __OBJC__
      @class NSString;
#  endif
#endif

class Q_CORE_EXPORT QString16 : public CsString::CsString_utf16
{
   public:
      class iterator : public CsString::CsString_utf16::iterator
      {
       public:
         using value_type  = QChar32;
         using pointer     = QChar32 *;
         using reference   = QChar32 &;

         iterator() = default;

         iterator(CsString::CsString_utf16::iterator iter)
            : CsString::CsString_utf16::iterator(std::move(iter)) {
         }

         // operators
         QChar32 operator*() const {
            return CsString::CsString_utf16::iterator::operator*();
         }

         QChar32Arrow operator->() const {
            return CsString::CsString_utf16::iterator::operator->();
         }

         QChar32 operator[](size_type n) const {
            return CsString::CsString_utf16::iterator::operator[](n);
         }

         bool operator==(iterator other) const {
            return CsString::CsString_utf16::iterator::operator==(other);
         }

         bool operator!=(iterator other) const {
            return CsString::CsString_utf16::iterator::operator!=(other);
         }

         iterator &operator+=(size_type n) {
            CsString::CsString_utf16::iterator::operator+=(n);
            return *this;
         }

         iterator &operator-=(size_type n) {
            CsString::CsString_utf16::iterator::operator-=(n);
            return *this;
         }

         iterator operator+(size_type n) const {
            return CsString::CsString_utf16::iterator::operator+(n);
         }

         iterator operator-(size_type n) const {
            return CsString::CsString_utf16::iterator::operator-(n);
         }

         size_type operator-(iterator other) const {
            return CsString::CsString_utf16::iterator::operator-(other);
         }

         iterator &operator++() {
            CsString::CsString_utf16::iterator::operator++();
            return *this;
         }

         iterator operator++(int n) {
            return CsString::CsString_utf16::iterator::operator++(n);
         }

         iterator &operator--() {
            CsString::CsString_utf16::iterator::operator--();
            return *this;
         }

         iterator operator--(int n) {
            return CsString::CsString_utf16::iterator::operator--(n);
         }
      };

      class const_iterator : public CsString::CsString_utf16::const_iterator
      {
       public:
         using value_type        = const QChar32;
         using pointer           = const QChar32 *;
         using reference         = const QChar32 &;

         const_iterator() = default;

         const_iterator(CsString::CsString_utf16::const_iterator iter)
            : CsString::CsString_utf16::const_iterator(std::move(iter)) {
         }

         const_iterator(iterator iter)
            : CsString::CsString_utf16::const_iterator(std::move(iter)) {
         }

         // operators
         const QChar32 operator*() const {
            return CsString::CsString_utf16::const_iterator::operator*();
         }

         QChar32Arrow operator->() const {
            return CsString::CsString_utf16::const_iterator::operator->();
         }

         QChar32 operator[](size_type n) const {
            return CsString::CsString_utf16::const_iterator::operator[](n);
         }

         bool operator==(const_iterator other) const {
            return CsString::CsString_utf16::const_iterator::operator==(other);
         }

         bool operator!=(const_iterator other) const {
            return CsString::CsString_utf16::const_iterator::operator!=(other);
         }

         const_iterator &operator+=(size_type n) {
            CsString::CsString_utf16::const_iterator::operator+=(n);
            return *this;
         }

         const_iterator &operator-=(size_type n) {
            CsString::CsString_utf16::const_iterator::operator-=(n);
            return *this;
         }

         const_iterator operator+(size_type n) const {
            return CsString::CsString_utf16::const_iterator::operator+(n);
         }

         const_iterator operator-(size_type n) const {
            return CsString::CsString_utf16::const_iterator::operator-(n);
         }

         size_type operator-(const_iterator other) const {
            return CsString::CsString_utf16::const_iterator::operator-(other);
         }

         const_iterator &operator++() {
            CsString::CsString_utf16::const_iterator::operator++();
            return *this;
         }

         const_iterator operator++(int n) {
            return CsString::CsString_utf16::const_iterator::operator++(n);
         }

         const_iterator &operator--() {
            CsString::CsString_utf16::const_iterator::operator--();
            return *this;
         }

         const_iterator operator--(int n) {
            return CsString::CsString_utf16::const_iterator::operator--(n);
         }
      };

      enum NormalizationForm {
         NormalizationForm_D,
         NormalizationForm_C,
         NormalizationForm_KD,
         NormalizationForm_KC
      };

      using difference_type = std::ptrdiff_t;
      using value_type      = QChar32;
      using size_type       = std::ptrdiff_t;
      using storage_type    = char16_t;

      using reverse_iterator       = CsString::CsStringReverseIterator<iterator>;
      using const_reverse_iterator = CsString::CsStringReverseIterator<const_iterator>;

      QString16() = default;

      QString16(const char16_t *data)
         : CsString::CsString_utf16(data)
      {
      }

      QString16(const QString16 &other) = default;
      QString16(QString16 &&other) = default;

      QString16(std::nullptr_t) = delete;

      QString16(QChar32 c);
      QString16(size_type numOfChars, QChar32 c);

      QString16(QChar32::SpecialCharacter c)
         : QString16(QChar32(c))
      {
      }

      QString16(const QChar32 *data, size_type numOfChars = -1)  {

         if (numOfChars == -1) {
            const QChar32 *p = data;

            while (p->unicode() != 0) {
               ++p;
            }

            CsString::CsString_utf16::append(data, p);

         } else {
            CsString::CsString_utf16::append(data, data + numOfChars);

         }
      }

      QString16(const_iterator begin, const_iterator end)
         : CsString::CsString_utf16(begin, end)
      {
      }

      // for an array of chars
      template <int N>
      QString16(const char (&cStr)[N])
         : CsString::CsString_utf16(cStr)
      { }

#ifdef CS_STRING_ALLOW_UNSAFE
      QString16(const QByteArray &str)
         : QString16(fromUtf8(str))
      { }
#endif

      template <typename Iterator>
      QString16(Iterator begin, Iterator end)
         : CsString::CsString_utf16(begin, end)
      { }

      // internal
      QString16(const CsString::CsString_utf16 &other)
         : CsString::CsString_utf16(other)
      { }

      // internal
      QString16(CsString::CsString_utf16 &&other)
         : CsString::CsString_utf16(std::move(other))
      { }

      QString16(QStringView16 str)
         : CsString::CsString_utf16( str )
      { }

      ~QString16() = default;

#if defined(__cpp_char8_t)
      // support new data type added in C++20

      inline QString16(const char8_t *str);
      inline QString16(const char8_t *str, size_type size);

      static inline QString16 fromUtf8(const char8_t *str, size_type numOfChars = -1);
#endif

      using CsString::CsString_utf16::append;         // internal
      using CsString::CsString_utf16::operator=;      // internal
      using CsString::CsString_utf16::operator+=;     // internal

      // methods
      QString16 &append(char32_t c)  {
         CsString::CsString_utf16::append(c);
         return *this;
      }

      QString16 &append(QChar32 c)  {
         CsString::CsString_utf16::append(c);
         return *this;
      }

      QString16 &append(const QString16 &other)  {
         CsString::CsString_utf16::append(other);
         return *this;
      }

      QString16 &append(const QChar32 *data, size_type numOfChars)  {
         CsString::CsString_utf16::append(data, data + numOfChars);
         return *this;
      }

      QString16 &append(const_iterator iter_begin, const_iterator iter_end)  {
         CsString::CsString_utf16::append(iter_begin, iter_end);
         return *this;
      }

      QString16 &append(QStringView16 str) {
         CsString::CsString_utf16::append(str.cbegin(), str.cend());
         return *this;
      }

      QString16 &append(QStringView16 str, size_type indexStart, size_type numOfChars) {
         CsString::CsString_utf16::append(str, indexStart, numOfChars);
         return *this;
      }

      QChar32 at(size_type index) const {
         return CsString::CsString_utf16::operator[](index);
      }

      QChar32 back() const {
         return CsString::CsString_utf16::back();
      }

      void chop(size_type numOfChars);

      void clear() {
         CsString::CsString_utf16::clear();
      }

      int compare(const QString16 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
         return compare(QStringView16(*this), QStringView16(str), cs);
      }

      static int compare(const QString16 &str1, const QString16 &str2, Qt::CaseSensitivity cs = Qt::CaseSensitive) {
         return compare(QStringView16(str1), QStringView16(str2), cs);
      }

      int compare(QStringView16 str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
         return compare(QStringView16(*this), str, cs);
      }

      static int compare(QStringView16 str1, QStringView16 str2, Qt::CaseSensitivity cs = Qt::CaseSensitive);

      const char16_t *constData() const {
         return reinterpret_cast<const char16_t *>(CsString::CsString_utf16::constData());
      }

      bool contains(char ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
         return contains(QChar32(ch), cs);
      }

      bool contains(QChar32 c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool contains(const QString16 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool contains(QStringView16 str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      bool contains(const QRegularExpression16 &regExp) const {
         return indexOfFast(regExp) != end();
      }

      size_type count() const {
         return CsString::CsString_utf16::size();
      }

      size_type count(QChar32 c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      size_type count(const QString16 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      size_type count(QStringView16 str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      size_type count(const QRegularExpression16 &regExp) const;

      const char16_t *data() const {
         return constData();
      }

      bool endsWith(QChar32 c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool endsWith(const QString16 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool endsWith(QStringView16 str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      QString16 &fill(QChar32 c, size_type numOfChars = -1);

      QChar32 first() const {
         return CsString::CsString_utf16::front();
      }

      QChar32 front() const {
         return CsString::CsString_utf16::front();
      }

      // using iterators
      const_iterator indexOfFast(QChar32 c) const {
         return indexOfFast(c, cbegin(), Qt::CaseSensitive);
      }

      const_iterator indexOfFast(QChar32 c, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString_utf16::find_fast(c, from);

         } else {
            return cs_internal_find_fast(c, from);

         }
      }

      const_iterator indexOfFast(const QString16 &str) const {
         return indexOfFast(str, cbegin(), Qt::CaseSensitive);
      }

      const_iterator indexOfFast(const QString16 &str, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString_utf16::find_fast(str, from);

         } else {
            return cs_internal_find_fast(str, from);
         }
      }

      const_iterator indexOfFast(QStringView16 str, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString_utf16::find_fast(str, from);

         } else {
            return cs_internal_find_fast(str, from);
         }
      }

      const_iterator indexOfFast(const QRegularExpression16 &regExp) const {
         return indexOfFast(regExp, begin());
      }

      const_iterator indexOfFast(const QRegularExpression16 &regExp, const_iterator from) const;

      // using iterators (last)
      const_iterator lastIndexOfFast(QChar32 c) const {
         return lastIndexOfFast(c, cend(), Qt::CaseSensitive);
      }

      const_iterator lastIndexOfFast(QChar32 c, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
      {
         if (cs == Qt::CaseSensitive) {
            return CsString::CsString_utf16::rfind_fast(c, from);

         } else {
            return cs_internal_rfind_fast(c, from);

         }
      }

      const_iterator lastIndexOfFast(const QString16 &str) const {
         return lastIndexOfFast(str, cend(), Qt::CaseSensitive);
      }

      const_iterator lastIndexOfFast(const QString16 &str, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString_utf16::rfind_fast(str, from);

         } else {
            return cs_internal_rfind_fast(str, from);
         }
      }

      const_iterator lastIndexOfFast(QStringView16 str, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString_utf16::rfind_fast(str, from);

         } else {
            return cs_internal_rfind_fast(str, from);
         }
      }

      // using indexes
      size_type indexOf(QChar32 c, size_type from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString_utf16::find(c, from);

         } else {
            QString16 tmp1 = this->toCaseFolded();
            return tmp1.CsString::CsString_utf16::find(c.toCaseFolded16(), from);
         }
      }

      size_type indexOf(const QString16 &str, size_type from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString_utf16::find(str, from);

         } else {
            QString16 tmp1 = this->toCaseFolded();
            return tmp1.CsString::CsString_utf16::find(str.toCaseFolded(), from);
         }
      }

      size_type indexOf(const QRegularExpression16 &regExp, size_type from = 0) const {
         if (from < 0) {
            from = 0;
         }

         const_iterator iter = indexOfFast(regExp, begin() + from);

         if (iter == end()) {
            return -1;
         }

         return iter - begin();
      }

      // using index (last)
      size_type lastIndexOf(QChar32 c, size_type from = -1, Qt::CaseSensitivity cs  = Qt::CaseSensitive) const  {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString_utf16::rfind(c, from);

         } else {
            QString16 tmp1 = this->toCaseFolded();
            return tmp1.CsString::CsString_utf16::rfind(c.toCaseFolded16(), from);
         }
      }

      size_type lastIndexOf(const QString16 &str, size_type from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString_utf16::rfind(str, from);

         } else {
            QString16 tmp1 = this->toCaseFolded();
            return tmp1.CsString::CsString_utf16::rfind(str.toCaseFolded(), from);
         }
      }

      //
      QString16 &insert (size_type indexStart, const QString16 &str)  {
         CsString::CsString_utf16::insert(indexStart, str);
         return *this;
      }

      QString16 &insert(size_type indexStart, QChar32 c) {
         CsString::CsString_utf16::insert(indexStart, 1, c);
         return *this;
      }

      QString16 &insert(size_type indexStart, const QChar32 *data, size_type numOfChars) {
         CsString::CsString_utf16::insert(begin() + indexStart, data, data + numOfChars);
         return *this;
      }

      QString16 &insert(const_iterator first, const QString16 &str) {
         CsString::CsString_utf16::insert(first, str);
         return *this;
      }

      template <typename Iterator>
      QString16 &insert(const_iterator first, Iterator begin, Iterator end) {
         CsString::CsString_utf16::insert(first, begin, end);
         return *this;
      }

      QString16 &insert(size_type indexStart, QStringView16 str) {
         CsString::CsString_utf16::insert(indexStart, str);
         return *this;
      }

      QString16 &insert(size_type indexStart, QStringView16 str, size_type srcStart, size_type numOfChars) {
         CsString::CsString_utf16::insert(indexStart, str, srcStart, numOfChars);
         return *this;
      }

      bool empty() const {
         return CsString::CsString_utf16::empty();
      }

      bool isEmpty() const {
         return empty();
      }

      QChar32 last()const {
         return CsString::CsString_utf16::back();
      }

      [[nodiscard]] QString16 left(size_type numOfChars) const;
      [[nodiscard]] QStringView16 leftView(size_type numOfChars) const;
      [[nodiscard]] QString16 leftJustified(size_type width, QChar32 fill = UCHAR(' '), bool truncate = false) const;

      size_type length() const {
         return CsString::CsString_utf16::size();
      }

      int localeAwareCompare(const QString16 &str) const {
         return localeAwareCompare(QStringView16(*this), QStringView16(str));
      }

      static int localeAwareCompare(const QString16 &str1, const QString16 &str2) {
         return localeAwareCompare(QStringView16(str1), QStringView16(str2));
      }

      int localeAwareCompare(QStringView16 str) const {
         return localeAwareCompare(QStringView16(*this), str);
      }

      static int localeAwareCompare(QStringView16 str1, QStringView16 str2);

      [[nodiscard]] QString16 mid(size_type indexStart, size_type numOfChars = -1) const;
      [[nodiscard]] QString16 mid(const_iterator iter, size_type numOfChars = -1) const;

      QStringView16 midView (size_type indexStart, size_type numOfChars = -1) const;
      QStringView16 midView (const_iterator iter, size_type numOfChars = -1) const;

      [[nodiscard]] QString16 normalized(QString16::NormalizationForm mode,
                  QChar32::UnicodeVersion version = QChar32::Unicode_Unassigned) const;

      QString16 &prepend(const QString16 &other) {
         CsString::CsString_utf16::insert(begin(), other);
         return *this;
      }

      QString16 &prepend(char32_t c) {
         CsString::CsString_utf16::insert(begin(), c);
         return *this;
      }

      QString16 &prepend(QChar32 c) {
         CsString::CsString_utf16::insert(begin(), c);
         return *this;
      }

      QString16 &prepend(const QChar32 *data, size_type numOfChars)  {
         CsString::CsString_utf16::insert(begin(), data, data + numOfChars);
         return *this;
      }

      QString16 &prepend(const_iterator iter_begin, const_iterator iter_end)  {
         CsString::CsString_utf16::insert(begin(), iter_begin, iter_end);
         return *this;
      }

      void push_back(QChar32 c) {
         append(c);
      }

      void push_back(const QString16 &other) {
         append(other);
      }

      void push_front(QChar32 c) {
         prepend(c);
      }

      void push_front(const QString16 &other) {
         prepend(other);
      }

      [[nodiscard]] QString16 repeated(size_type count) const;

      QString16 &remove(size_type indexStart, size_type numOfChars);
      QString16 &remove(QChar32 c, Qt::CaseSensitivity cs = Qt::CaseSensitive);
      QString16 &remove(const QString16 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive);


      QString16 &remove(const QRegularExpression16 &regExp) {
         replace(regExp, QString16());
         return *this;
      }

      QString16 &replace(QChar32 before, QChar32 after, Qt::CaseSensitivity cs = Qt::CaseSensitive);

      QString16 &replace(const QChar32 *before, size_type beforeSize, const QChar32 *after, size_type afterSize,
                  Qt::CaseSensitivity cs = Qt::CaseSensitive);

      QString16 &replace(const QString16 &before, const QString16 &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
      QString16 &replace(QChar32 c, const QString16 &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);

      QString16 &replace(size_type indexStart, size_type numOfChars, QChar32 c) {
         CsString::CsString_utf16::replace(indexStart, numOfChars, 1, c);
         return *this;
      }

      QString16 &replace(size_type indexStart, size_type numOfChars, const QChar32 *data, size_type sizeStr)
      {
         replace(indexStart, numOfChars, QString16(data, sizeStr));
         return *this;
      }

      QString16 &replace(size_type indexStart, size_type numOfChars, const QString16 &str) {
         CsString::CsString_utf16::replace(indexStart, numOfChars, str);
         return *this;
      }

      QString16 &replace(size_type indexStart, size_type numOfChars, const QString16 &str, size_type sizeStr)
      {
         CsString::CsString_utf16::replace(indexStart, numOfChars, str.left(sizeStr));
         return *this;
      }

      template <typename Iterator>
      QString16 &replace(const_iterator first1, const_iterator last1, Iterator first2, Iterator last2) {
         CsString::CsString_utf16::replace(first1, last1, first2, last2);
         return *this;
      }

      QString16 &replace(const_iterator first, const_iterator last, const QString16 &str) {
         CsString::CsString_utf16::replace(first, last, str);
         return *this;
      }

      QString16 &replace(const QRegularExpression16 &regExp, const QString16 &after);

      void resize(size_type numOfChars) {
         return CsString::CsString_utf16::resize(numOfChars);
      }

      void resize(size_type numOfChars, QChar32 c) {
         return CsString::CsString_utf16::resize(numOfChars, c);
      }

      [[nodiscard]] QString16 right(size_type count) const;
      [[nodiscard]] QStringView16 rightView(size_type count) const;
      [[nodiscard]] QString16 rightJustified(size_type width, QChar32 fill = UCHAR(' '), bool truncate = false) const;

      [[nodiscard]] QString16 simplified() const &;
      [[nodiscard]] QString16 simplified() &&;

      size_type size() const {
         return CsString::CsString_utf16::size();
      }

      size_type size_storage() const{
         return CsString::CsString_utf16::size_storage();
      }

      bool startsWith(QChar32 c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool startsWith(const QString16 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool startsWith(QStringView16 str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      void squeeze() {
         return CsString::CsString_utf16::shrink_to_fit();
      }

      void swap(QString16 &other) {
         CsString::CsString_utf16::swap(other);
      }

      QString16 toHtmlEscaped() const;

      [[nodiscard]] QString16 toCaseFolded() const &;
      [[nodiscard]] QString16 toCaseFolded() &&;

      [[nodiscard]] QString16 toLower() const &;
      [[nodiscard]] QString16 toLower() &&;

      [[nodiscard]] QString16 toUpper() const &;
      [[nodiscard]] QString16 toUpper() &&;

      [[nodiscard]] QByteArray toLatin1() const;
      [[nodiscard]] QByteArray toUtf8() const;

      std::wstring toStdWString() const;

      [[nodiscard]] QString16 trimmed() const &;
      [[nodiscard]] QString16 trimmed() &&;

      void truncate(size_type length);

      const uint16_t *utf16() const {
         return CsString::CsString_utf16::constData();
      }

      // static
      static QString16 fromLatin1(const QByteArray &str);
      static QString16 fromLatin1(const char *str, size_type numOfChars = -1);

      static QString16 fromUtf8(const QByteArray &str);
      static QString16 fromUtf8(const char *str, size_type numOfChars = -1);

      static QString16 fromUtf16(const char16_t *str, size_type numOfChars = -1);
      static QString16 fromUtf8(const QString8 &str);

      static QString16 fromStdWString(const std::wstring &str, size_type numOfChars = -1);
      static QString16 fromStdString(const std::string &str, size_type numOfChars = -1);

      // wrappers
      template <typename SP = QStringParser, typename ...Ts>
      [[nodiscard]] QString16 formatArg(Ts &&... args) const
      {
         return SP::template formatArg<QString16>(*this, std::forward<Ts>(args)...);
      }

      template <typename SP = QStringParser, typename ...Ts>
      [[nodiscard]] QString16 formatArgs(Ts &&... args) const
      {
         return SP::template formatArgs<QString16>(*this, std::forward<Ts>(args)...);
      }

      template <typename V, typename SP = QStringParser>
      [[nodiscard]] static QString16 number(V value, int base  = 10)
      {
         return SP::template number<QString16>(value, base);
      }

      template <typename SP = QStringParser>
      [[nodiscard]] static QString16 number(double value, char format = 'g', int precision = 6)
      {
         return SP::template number<QString16>(value, format, precision);
      }

      template <typename SP = QStringParser, typename ...Ts>
      QString16 section(QChar32 separator, Ts... args) const {
         return SP::section(*this, QString16(separator), args...);
      }

      template <typename SP = QStringParser, typename ...Ts>
      QString16 section(const QString16 &separator, Ts... args) const {
         return SP::section(*this, separator, args...);
      }

      template <typename SP = QStringParser, typename ...Ts>
      auto split(QChar32 separator, Ts... args) const
      {
         return SP::split(*this, separator, args...);
      }

      template <typename SP = QStringParser, typename ...Ts>
      auto split(const QString16 &separator, Ts... args) const
      {
         return SP::split(*this, separator, args...);
      }

      template <typename SP = QStringParser, typename ...Ts>
      auto split(const QRegularExpression16 &separator, Ts... args) const
      {
         return SP::split(*this, separator, args...);
      }

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

      // iterators
      iterator begin() {
         return CsString::CsString_utf16::begin();
      }

      const_iterator begin() const {
         return CsString::CsString_utf16::cbegin();
      }

      const_iterator cbegin() const {
         return CsString::CsString_utf16::cbegin();
      }

      const_iterator constBegin() const {
         return CsString::CsString_utf16::cbegin();
      }

      iterator end() {
         return CsString::CsString_utf16::end();
      }

      const_iterator end() const {
         return CsString::CsString_utf16::cend();
      }

      const_iterator cend() const {
         return CsString::CsString_utf16::cend();
      }

      const_iterator constEnd() const {
         return CsString::CsString_utf16::cend();
      }

      reverse_iterator rbegin()  {
         return CsString::CsString_utf16::rbegin();
      }

      const_reverse_iterator rbegin() const {
         return CsString::CsString_utf16::rbegin();
      }

      reverse_iterator rend()  {
         return CsString::CsString_utf16::rend();
      }

      const_reverse_iterator rend() const {
         return CsString::CsString_utf16::rend();
      }

      const_reverse_iterator crbegin() const {
         return CsString::CsString_utf16::crbegin();
      }

      const_reverse_iterator crend() const {
         return CsString::CsString_utf16::crend();
      }

      // storage iterators
      const_storage_iterator storage_begin() const {
         return CsString::CsString_utf16::storage_begin();
      }

      const_storage_iterator storage_end() const {
         return CsString::CsString_utf16::storage_end();
      }

      const_storage_reverse_iterator storage_rbegin() const {
         return CsString::CsString_utf16::storage_rbegin();
      }

      const_storage_reverse_iterator storage_rend() const {
         return CsString::CsString_utf16::storage_rend();
      }

      // operators
      QString16 &operator=(const QString16 &other) = default;
      QString16 &operator=(QString16 && other) = default;

      QString16 &operator=(QChar32 c)  {
         CsString::CsString_utf16::operator=(c);
         return *this;
      }

      QString16 &operator=(QStringView16 str) {
         CsString::CsString_utf16::operator=(str);
         return *this;
      }

      QString16 &operator+=(QChar32 c)  {
         CsString::CsString_utf16::operator+=(c);
         return *this;
      }

      QString16 &operator+=(QChar32::SpecialCharacter c) {
         append(QChar32(c));
         return *this;
      }

      QString16 &operator+= (const QString16 & other) {
         CsString::CsString_utf16::operator+=(other);
         return *this;
      }

      QString16 &operator+= (QStringView16 str) {
         CsString::CsString_utf16::operator+=(str);
         return *this;
      }

      QChar32 operator[](size_type index) const {
         return CsString::CsString_utf16::operator[](index);
      }

#if defined(Q_OS_DARWIN)
    static QString16 fromCFString(CFStringRef string);
    CFStringRef toCFString() const;

#  if defined(__OBJC__)
    static QString16 fromNSString(const NSString *string);
    NSString *toNSString() const;
#  endif

#endif

   private:
      const_iterator cs_internal_find_fast(QChar32 c, const_iterator iter_begin) const;
      const_iterator cs_internal_find_fast(const QString16 &str, const_iterator iter_begin) const;
      const_iterator cs_internal_rfind_fast(QChar32 c, const_iterator iter_begin) const;
      const_iterator cs_internal_rfind_fast(const QString16 &str, const_iterator iter_begin) const;

      iterator replace(const_iterator iter_begin, const QString16 &str) {
         auto iter = CsString::CsString_utf16::replace(iter_begin, str);
         iter = iter.advance_storage(str.size_storage());

         return iter;
      }
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QString16 &str);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QString16 &str);

// free functions, comparisons for string literals
template <int N>
inline bool operator==(QStringView16 str1, const char16_t (&str2)[N])
{
   return std::equal(str1.storage_begin(), str1.storage_end(), str2, str2+N-1);
}

template <int N>
inline bool operator==(const char16_t (& str1)[N], QStringView16 str2)
{
   return std::equal(str1, str1+N-1, str2.storage_begin(), str2.storage_end());
}

template <int N>
inline bool operator==(QString16 str1, const char16_t (& str2)[N])
{
   return std::equal(str1.storage_begin(), str1.storage_end(), str2, str2+N-1);
}

template <int N>
inline bool operator==(const char16_t (& str1)[N], QString16 str2)
{
   return std::equal(str1, str1+N-1, str2.storage_begin(), str2.storage_end());
}

template <int N>
inline bool operator!=(QStringView16 str1, const char16_t (&str2)[N])
{
   return ! std::equal(str1.storage_begin(), str1.storage_end(), str2, str2+N-1);
}

template <int N>
inline bool operator!=(const char16_t (& str1)[N], QStringView16 str2)
{
   return ! std::equal(str1, str1+N-1, str2.storage_begin(), str2.storage_end());
}

template <int N>
inline bool operator!=(QString16 str1, const char16_t (& str2)[N])
{
   return ! std::equal(str1.storage_begin(), str1.storage_end(), str2, str2+N-1);
}

template <int N>
inline bool operator!=(const char16_t (& str1)[N], QString16 str2)
{
   return ! std::equal(str1, str1+N-1, str2.storage_begin(), str2.storage_end());
}

inline QString16 operator+(const QString16 &str1, const QString16 &str2)
{
   QString16 t(str1);
   t += str2;
   return t;
}

inline QString16 operator+(QString16 &&str1, const QString16 &str2)
{
   str1 += str2;
   return std::move(str1);
}

inline QString16::const_iterator operator+(QString16::size_type n, QString16::const_iterator iter)
{
   iter += n;
   return iter;
}

inline QString16 operator+(QChar32 c, const QString16 &str)
{
   QString16 t = str;
   t.prepend(c);
   return t;
}

inline QString16 operator+(const QString16 &str, QChar32 c)
{
   QString16 t = str;
   t += c;
   return t;
}

inline QString16 operator+(QString16 &&str, QChar32 c)
{
   str += c;
   return std::move(str);
}

// for an array of chars
template <int N>
inline const QString16 operator+(const char (&cString)[N], const QString16 &str)
{
   QString16 t(str);
   t.prepend(cString);
   return t;
}

// for an array of chars
template <int N>
inline const QString16 operator+(const QString16 &str, const char (&cString)[N])
{
   QString16 t(str);
   t += cString;
   return t;
}

// for an array of chars
template <int N>
inline QString16 operator+(QString16 &&str, const char (&cString)[N])
{
   str += cString;
   return std::move(str);
}

inline bool operator<(const QString16 &str1, const QString16 &str2)
{
   return (static_cast<CsString::CsString_utf16>(str1) < static_cast<CsString::CsString_utf16>(str2));
}

// for an array of chars
template <int N>
inline bool operator<(const char (&cString)[N], const QString16 &str)
{
   return (QString16(cString) < str);
}

// for an array of chars
template <int N>
inline bool operator<(const QString16 &str, const char (&cString)[N])
{
   return (str < QString16(cString));
}

inline bool operator<=(const QString16 &str1, const QString16 &str2)
{
   return (static_cast<CsString::CsString_utf16>(str1) <= static_cast<CsString::CsString_utf16>(str2));
}

// for an array of chars
template <int N>
inline bool operator<=(const char (&cString)[N], const QString16 &str)
{
   return (QString16(cString) <= str);
}

// for an array of chars
template <int N>
inline bool operator<=(const QString16 &str, const char (&cString)[N])
{
   return (str <= QString16(cString));
}

inline bool operator>(const QString16 &str1, const QString16 &str2)
{
   return (static_cast<CsString::CsString_utf16>(str1) > static_cast<CsString::CsString_utf16>(str2));
}

// for an array of chars
template <int N>
inline bool operator>(const char (&cString)[N], const QString16 &str)
{
   return (QString16(cString) > str);
}

// for an array of chars
template <int N>
inline bool operator>(const QString16 &str, const char (&cString)[N])
{
   return (str > QString16(cString));
}

inline bool operator>=(const QString16 &str1, const QString16 &str2)
{
   return (static_cast<CsString::CsString_utf16>(str1) >= static_cast<CsString::CsString_utf16>(str2));
}

// for an array of chars
template <int N>
inline bool operator>=(const char (&cString)[N], const QString16 &str)
{
   return (QString16(cString) >= str);
}

// for an array of chars
template <int N>
inline bool operator>=(const QString16 &str, const char (&cString)[N])
{
   return (str >= QString16(cString));
}

inline void swap(QString16 &a, QString16 &b) {
   a.swap(b);
}

QString16 cs_internal_string_normalize(const QString16 &data, QString16::NormalizationForm mode,
                  QChar32::UnicodeVersion version, int from);

#if defined(__cpp_char8_t)
   // support new data type added in C++20

   inline QString16::QString16(const char8_t *str)
   {
      *this = QString16::fromUtf8(str, -1);
   }

   inline QString16::QString16(const char8_t *str, size_type size)
   {
      *this = QString16::fromUtf8(str, size);
   }

   inline QString16 QString16::fromUtf8(const char8_t *str, size_type numOfChars)
   {
      return CsString::CsString_utf16::fromUtf8(str, numOfChars);
   }
#endif

#endif