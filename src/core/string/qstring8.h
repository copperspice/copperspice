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

#ifndef QSTRING8_H
#define QSTRING8_H

#define CS_STRING_ALLOW_UNSAFE

#include <cstddef>
#include <string>

#include <qglobal.h>
#include <qbytearray.h>

#include <cs_string.h>
#include <qchar32.h>
#include <qstringview.h>

class QStringParser;

template <typename S>
class QRegularExpression;

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

         size_type operator-(iterator other) const {
            return CsString::CsString::iterator::operator-(other);
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

         size_type operator-(const_iterator other) const {
            return CsString::CsString::const_iterator::operator-(other);
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

      using difference_type = std::ptrdiff_t;
      using size_type       = std::ptrdiff_t;
      using value_type      = QChar32;

      using Iterator        = iterator;
      using ConstIterator   = const_iterator;

      using const_reverse_iterator = CsString::CsStringReverseIterator<const_iterator>;
      using reverse_iterator       = CsString::CsStringReverseIterator<iterator>;

      QString8() = default;
      QString8(const QString8 &other) = default;
      QString8(QString8 &&other) = default;

      QString8(QChar32 c);
      QString8(size_type numOfChars, QChar32 c);

      QString8(const QChar32 *data, size_type numOfChars = -1)  {

         if (numOfChars == -1) {
            const QChar32 *p = data;

            while (p->unicode() != 0) {
               ++p;
            }

            CsString::CsString::append(data, p);

         } else {
            CsString::CsString::append(data, data + numOfChars);

         }
      }

      // for an array of chars
      template <int N>
      QString8(const char (&cStr)[N])
         : CsString::CsString(cStr)
      { }

#ifdef CS_STRING_ALLOW_UNSAFE
      QString8(const QByteArray &str)
         : QString8(fromUtf8(str))
      { }
#endif

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

      QString8(QStringView8 str)
         : CsString::CsString( str )
      {
      }

      ~QString8() = default;

      using CsString::CsString::append;         // internal
      using CsString::CsString::operator=;      // internal
      using CsString::CsString::operator+=;     // internal

      // methods
      QString8 &append(QChar32 c)  {
         CsString::CsString::append(c);
         return *this;
      }

      QString8 &append(char c)  {
         CsString::CsString::append(c);
         return *this;
      }

      QString8 &append(const QString8 &other)  {
         CsString::CsString::append(other);
         return *this;
      }

      QString8 &append(const QChar32 *data, size_type numOfChars)  {
         CsString::CsString::append(data, data + numOfChars);
         return *this;
      }

      QString8 &append(const_iterator iter_begin, const_iterator iter_end)  {
         CsString::CsString::append(iter_begin, iter_end);
         return *this;
      }

      QString8 &append(QStringView8 str) {
         CsString::CsString::append(str.cbegin(), str.cend());
         return *this;
      }

      QString8 &append(QStringView8 str, size_type indexStart, size_type numOfChars) {
         CsString::CsString::append(str, indexStart, numOfChars);
         return *this;
      }

      QChar32 at(size_type index) const {
         return CsString::CsString::operator[](index);
      }

      QChar32 back() const {
         return CsString::CsString::back();
      }

      void chop(size_type n);

      void clear() {
         CsString::CsString::clear();
      }

      size_type count() const {
         return CsString::CsString::size();
      }

      size_type count(QChar32 c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      size_type count(const QString8 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      size_type count(QStringView8 str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      size_type count(const QRegularExpression<QString8> &regExp) const;

      int compare(const QString8 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      static int compare(const QString8 &str1, const QString8 &str2, Qt::CaseSensitivity cs = Qt::CaseSensitive) {
         return str1.compare(str2, cs);
      }

      int compare(QStringView8 str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      static int compare(const QString8 &str1, QStringView8 str2, Qt::CaseSensitivity cs = Qt::CaseSensitive) {
         return str1.compare(str2, cs);
      }

      const char *constData() const {
         return reinterpret_cast<const char *>(CsString::CsString::constData());
      }

      bool contains(QChar32 c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool contains(const QString8 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool contains(QStringView8 str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      bool contains(QRegularExpression<QString8> &regExp) const {
         return indexOfFast(regExp) != end();
      }

      const char *data() const {
         return constData();
      }

      bool endsWith(QChar32 c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool endsWith(const QString8 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool endsWith(QStringView8 str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

      QString8 &fill(QChar32 c, size_type numOfChars = -1);

      QChar32 first() const {
         return CsString::CsString::front();
      }

      QChar32 front() const {
         return CsString::CsString::front();
      }

      // using iterators
      const_iterator indexOfFast(QChar32 c) const {
         return indexOfFast(c, cbegin(), Qt::CaseSensitive);
      }

      const_iterator indexOfFast(QChar32 c, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString::find_fast(c, from);

         } else {
            return cs_internal_find_fast(c, from);

         }
      }

      const_iterator indexOfFast(const QString8 &str) const {
         return indexOfFast(str, cbegin(), Qt::CaseSensitive);
      }

      const_iterator indexOfFast(const QString8 &str, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString::find_fast(str, from);

         } else {
            return cs_internal_find_fast(str, from);
         }
      }

      const_iterator indexOfFast(QStringView8 str, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString::find_fast(str, from);

         } else {
            return cs_internal_find_fast(str, from);
         }
      }

      const_iterator indexOfFast(const QRegularExpression<QString8> &regExp) const {
         return indexOfFast(regExp, begin());
      }

      const_iterator indexOfFast(const QRegularExpression<QString8> &regExp, const_iterator from) const;

      const_iterator lastIndexOfFast(QChar32 c) const {
         return lastIndexOfFast(c, cend(), Qt::CaseSensitive);
      }

      const_iterator lastIndexOfFast(QChar32 c, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
      {
         if (cs == Qt::CaseSensitive) {
            return CsString::CsString::rfind_fast(c, from);

         } else {
            return cs_internal_rfind_fast(c, from);

         }
      }

      const_iterator lastIndexOfFast(const QString8 &str) const {
         return lastIndexOfFast(str, cend(), Qt::CaseSensitive);
      }

      const_iterator lastIndexOfFast(const QString8 &str, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString::rfind_fast(str, from);

         } else {
            return cs_internal_rfind_fast(str, from);
         }
      }


      const_iterator lastIndexOfFast(QStringView8 str, const_iterator from, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString::rfind_fast(str, from);

         } else {
            return cs_internal_rfind_fast(str, from);
         }
      }

      const_iterator lastIndexOfFast(const QRegularExpression<QString8> &regExp) const {
         return lastIndexOfFast(regExp, end());
      }

      const_iterator lastIndexOfFast(const QRegularExpression<QString8> &regExp, const_iterator from) const;

      // using indexes
      size_type indexOf(QChar32 c, size_type from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString::find(c, from);

         } else {
            QString8 tmp1 = this->toCaseFolded();
            return tmp1.CsString::CsString::find(c.toCaseFolded(), from);
         }
      }

      size_type indexOf(const QString8 &str, size_type from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString::find(str, from);

         } else {
            QString8 tmp1 = this->toCaseFolded();
            return tmp1.CsString::CsString::find(str.toCaseFolded(), from);
         }
      }

      size_type lastIndexOf(QChar32 c, size_type from = -1, Qt::CaseSensitivity cs  = Qt::CaseSensitive) const  {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString::rfind(c, from);

         } else {
            QString8 tmp1 = this->toCaseFolded();
            return tmp1.CsString::CsString::rfind(c.toCaseFolded(), from);
         }
      }

      size_type lastIndexOf(const QString8 &str, size_type from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {

         if (cs == Qt::CaseSensitive) {
            return CsString::CsString::rfind(str, from);

         } else {
            QString8 tmp1 = this->toCaseFolded();
            return tmp1.CsString::CsString::rfind(str.toCaseFolded(), from);
         }
      }

      QString8 &insert (size_type indexStart, const QString8 &str)  {
         CsString::CsString::insert(indexStart, str);
         return *this;
      }

      QString8 &insert(size_type indexStart, QChar32 c) {
         CsString::CsString::insert(indexStart, 1, c);
         return *this;
      }

      QString8 &insert(size_type indexStart, const QChar32 *data, size_type numOfChars) {
         CsString::CsString::insert(begin() + indexStart, data, data + numOfChars);
         return *this;
      }

      QString8 &insert(const_iterator first, const QString8 &str) {
         CsString::CsString::insert(first, str);
         return *this;
      }

      template<typename Iterator>
      QString8 &insert(const_iterator first, Iterator begin, Iterator end) {
         CsString::CsString::insert(first, begin, end);
         return *this;
      }

      QString8 &insert(size_type indexStart, QStringView8 str) {
         CsString::CsString::insert(indexStart, str);
         return *this;
      }

      QString8 &insert(size_type indexStart, QStringView8 str, size_type srcStart, size_type numOfChars) {
         CsString::CsString::insert(indexStart, str, srcStart, numOfChars);
         return *this;
      }

      bool empty() const {
         return CsString::CsString::empty();
      }

      bool isEmpty() const {
         return empty();
      }

      QChar32 last()const {
         return CsString::CsString::back();
      }

      // internal method
      bool isSimpleText() const;

      QString8 left(size_type numOfChars) const Q_REQUIRED_RESULT;
      QStringView8 leftView(size_type numOfChars) const Q_REQUIRED_RESULT;
      QString8 leftJustified(size_type width, QChar32 fill = UCHAR(' '), bool trunc = false) const Q_REQUIRED_RESULT;

      size_type length() const {
         return CsString::CsString::size();
      }

/*
      // not implemented, not documented

      int localeAwareCompare(const QString8 &str) const;

      static int localeAwareCompare(const QString8 &str1, const QString8 &str2) {
         return s1.localeAwareCompare(s2);
      }

      int localeAwareCompare(QStringView8 str) const;

      static int localeAwareCompare(const QString8 &str1, QStringView8 str2) {
         return s1.localeAwareCompare(s2);
      }
*/

      QString8 mid(size_type indexStart, size_type numOfChars = -1) const Q_REQUIRED_RESULT;
      QStringView8 midView (size_type indexStart, size_type numOfChars = -1) const;

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

      QString8 &prepend(const QChar32 *data, size_type numOfChars)  {
         CsString::CsString::insert(begin(), data, data + numOfChars);
         return *this;
      }

      QString8 &prepend(char c) {
         CsString::CsString::insert(begin(), c);
         return *this;
      }

      QString8 &prepend(const_iterator iter_begin, const_iterator iter_end)  {
         CsString::CsString::insert(begin(), iter_begin, iter_end);
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

      QString8 &remove(size_type indexStart, size_type numOfChars);
      QString8 &remove(QChar32 c, Qt::CaseSensitivity cs = Qt::CaseSensitive);
      QString8 &remove(const QString8 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive);


      QString8 &remove(const QRegularExpression<QString8> &regExp) {
         replace(regExp, QString8());
         return *this;
      }

      QString8 &replace(QChar32 before, QChar32 after, Qt::CaseSensitivity cs = Qt::CaseSensitive);

      QString8 &replace(const QChar32 *before, size_type beforeSize, const QChar32 *after, size_type afterSize,
                  Qt::CaseSensitivity cs = Qt::CaseSensitive);

      QString8 &replace(const QString8 &before, const QString8 &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
      QString8 &replace(QChar32 c, const QString8 &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);

      QString8 &replace(size_type indexStart, size_type numOfChars, QChar32 c) {
         CsString::CsString::replace(indexStart, numOfChars, 1, c);
         return *this;
      }

      QString8 &replace(size_type indexStart, size_type numOfChars, const QChar32 *data, size_type sizeStr)
      {
         replace(indexStart, numOfChars, QString8(data, sizeStr));
         return *this;
      }

      QString8 &replace(size_type indexStart, size_type numOfChars, const QString8 &str) {
         CsString::CsString::replace(indexStart, numOfChars, str);
         return *this;
      }

      QString8 &replace(size_type indexStart, size_type numOfChars, const QString8 &str, size_type sizeStr)
      {
         CsString::CsString::replace(indexStart, numOfChars, str.left(sizeStr));
         return *this;
      }

      template <typename Iterator>
      QString8 &replace(const_iterator first1, const_iterator last1, Iterator first2, Iterator last2) {
         CsString::CsString::replace(first1, last1, first2, last2);
         return *this;
      }

      QString8 &replace(const_iterator first, const_iterator last, const QString8 &str) {
         CsString::CsString::replace(first, last, str);
         return *this;
      }

      QString8 &replace(const QRegularExpression<QString8> &regExp, const QString8 &after);

      void resize(size_type numOfChars) {
         return CsString::CsString::resize(numOfChars);
      }

      void resize(size_type numOfChars, QChar32 c) {
         return CsString::CsString::resize(numOfChars, c);
      }

      QString8 right(size_type numOfChars) const Q_REQUIRED_RESULT;
      QStringView8 rightView(size_type numOfChars) const Q_REQUIRED_RESULT;
      QString8 rightJustified(size_type width, QChar32 fill = UCHAR(' '), bool trunc = false) const Q_REQUIRED_RESULT;

      QString8 simplified() const & Q_REQUIRED_RESULT;
      QString8 simplified() && Q_REQUIRED_RESULT;

      size_type size() const {
         return CsString::CsString::size();
      }

      bool startsWith(QChar32 c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool startsWith(const QString8 &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
      bool startsWith(QStringView8 str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

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

      QByteArray toLatin1() const Q_REQUIRED_RESULT;
      QByteArray toUtf8() const Q_REQUIRED_RESULT;
      QByteArray toUtf16() const Q_REQUIRED_RESULT;

      std::string toStdString() const {
         return std::string(cbegin().codePointBegin(), cend().codePointBegin() );
      }

      QString8 trimmed() const & Q_REQUIRED_RESULT;
      QString8 trimmed() && Q_REQUIRED_RESULT;

      void truncate(size_type length);

      const uint8_t *utf8() const {
         return CsString::CsString::constData();
      }

      // static
      static QString8 fromLatin1(const QByteArray &str);
      static QString8 fromLatin1(const char *str, size_type numOfChars = -1);

      static QString8 fromUtf8(const QByteArray &str);
      static QString8 fromUtf8(const char *str, size_type numOfChars = -1);
      static QString8 fromUtf16(const char16_t *str, size_type numOfChars = -1);

      // wrappers
      template <typename SP = QStringParser, typename ...Ts>
      QString8 formatArg(Ts... args) const
      {
         return SP::formatArg(*this, args...);
      }

      template <typename SP = QStringParser, typename ...Ts>
      QString8 formatArgs(Ts... args) const
      {
         return SP::formatArgs(*this, args...);
      }

      template <typename V, typename SP = QStringParser>
      static QString8 number(V value, int base  = 10)
      {
         return SP::template number<QString8>(value, base);
      }

      template <typename SP = QStringParser>
      static QString8 number(double value, char format = 'g', int precision = 6)
      {
         return SP::template number<QString8>(value, format, precision);
      }

      template <typename SP = QStringParser, typename ...Ts>
      QString8 section(QChar32 separator, Ts... args) const {
         return SP::section(*this, QString8(separator), args...);
      }

      template <typename SP = QStringParser, typename ...Ts>
      QString8 section(const QString8 &separator, Ts... args) const {
         return SP::section(*this, separator, args...);
      }

      template <typename SP = QStringParser, typename ...Ts>
      auto split(QChar32 separator, Ts... args) const
      {
         return SP::split(*this, separator, args...);
      }

      template <typename SP = QStringParser, typename ...Ts>
      auto split(const QString8 &separator, Ts... args) const
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
      QString8 &operator=(const QString8 &other) = default;
      QString8 &operator=(QString8 && other) = default;

      QString8 &operator=(QChar32 c)  {
         CsString::CsString::operator=(c);
         return *this;
      }

      QString8 &operator=(QStringView8 str) {
         CsString::CsString::operator=(str);
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

      QString8 &operator+= (const QString8 & other) {
         CsString::CsString::operator+=(other);
         return *this;
      }

      QString8 &operator+= (QStringView8 str) {
         CsString::CsString::operator+=(str);
         return *this;
      }

      QChar32 operator[](size_type index) const {
         return CsString::CsString::operator[](index);
      }

   private:
      const_iterator cs_internal_find_fast(QChar32 c, const_iterator iter_begin) const;
      const_iterator cs_internal_find_fast(const QString8 &str, const_iterator iter_begin) const;
      const_iterator cs_internal_rfind_fast(QChar32 c, const_iterator iter_begin) const;
      const_iterator cs_internal_rfind_fast(const QString8 &str, const_iterator iter_begin) const;

      iterator replace(const_iterator iter, const QString8 &str) {
         return CsString::CsString::replace(iter, str);
      }
};

#if ! defined(QT_NO_DATASTREAM)
   Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QString8 &);
   Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QString8 &);
#endif


// free functions, comparisons for string literals
inline bool operator==(const QString8 &str1, const QString8 &str2)
{
   return (static_cast<CsString::CsString>(str1) == static_cast<CsString::CsString>(str2));
}

// for an array of chars
template <int N>
inline bool operator==(const char (&cStr)[N], const QString8 &str)
{
   return (static_cast<CsString::CsString>(str) == cStr);
}

// for an array of chars
template <int N>
inline bool operator==(const QString8 &str, const char (&cStr)[N])
{
   return (static_cast<CsString::CsString>(str) == cStr);
}

inline bool operator!=(const QString8 &str1, const QString8 &str2)
{
   return ! (str1 == str2);
}

// for an array of chars
template <int N>
inline bool operator!=(const char (&cStr)[N], const QString8 &str)
{
   return ! (str == cStr);
}

// for an array of chars
template <int N>
inline bool operator!=(const QString8 &str, const char (&cStr)[N])
{
   return ! (str == cStr);
}

inline QString8 operator+(const QString8 &str1, const QString8 &str2)
{
   QString8 t(str1);
   t += str2;
   return t;
}

inline QString8 &&operator+(QString8 &&str1, const QString8 &str2)
{
   str1 += str2;
   return std::move(str1);
}

inline QString8 operator+(QChar32 c, const QString8 &str)
{
   QString8 t = str;
   t.prepend(c);
   return t;
}

inline QString8 operator+(const QString8 &str, QChar32 c)
{
   QString8 t = str;
   t += c;
   return t;
}

inline QString8 &&operator+(QString8 &&str, QChar32 c)
{
   str += c;
   return std::move(str);
}

// for an array of chars
template <int N>
inline const QString8 operator+(const char (&cStr)[N], const QString8 &str)
{
   QString8 t(str);
   t.prepend(cStr);
   return t;
}

// for an array of chars
template <int N>
inline const QString8 operator+(const QString8 &str, const char (&cStr)[N])
{
   QString8 t(str);
   t += cStr;
   return t;
}

// for an array of chars
template <int N>
inline QString8 &&operator+(QString8 &&str, const char (&cStr)[N])
{
   str += cStr;
   return std::move(str);
}

inline bool operator<(const QString8 &str1, const QString8 &str2)
{
   return (static_cast<CsString::CsString>(str1) < static_cast<CsString::CsString>(str2));
}

// for an array of chars
template <int N>
inline bool operator<(const char (&cStr)[N], const QString8 &str)
{
   return ! (static_cast<CsString::CsString>(str) >= cStr);
}

// for an array of chars
template <int N>
inline bool operator<(const QString8 &str, const char (&cStr)[N])
{
   return (static_cast<CsString::CsString>(str) < cStr);
}

inline bool operator<=(const QString8 &str1, const QString8 &str2)
{
   return (static_cast<CsString::CsString>(str1) <= static_cast<CsString::CsString>(str2));
}

// for an array of chars
template <int N>
inline bool operator<=(const char (&cStr)[N], const QString8 &str)
{
   return ! (static_cast<CsString::CsString>(str) > cStr);
}

// for an array of chars
template <int N>
inline bool operator<=(const QString8 &str, const char (&cStr)[N])
{
   return (static_cast<CsString::CsString>(str) <= cStr);
}

inline bool operator>(const QString8 &str1, const QString8 &str2)
{
   return (static_cast<CsString::CsString>(str1) > static_cast<CsString::CsString>(str2));
}

// for an array of chars
template <int N>
inline bool operator>(const char (&cStr)[N], const QString8 &str)
{
   return ! (static_cast<CsString::CsString>(str) <= cStr);
}

// for an array of chars
template <int N>
inline bool operator>(const QString8 &str, const char (&cStr)[N])
{
   return (static_cast<CsString::CsString>(str) > cStr);
}

inline bool operator>=(const QString8 &str1, const QString8 &str2)
{
   return (static_cast<CsString::CsString>(str1) >= static_cast<CsString::CsString>(str2));
}

// for an array of chars
template <int N>
inline bool operator>=(const char (&cStr)[N], const QString8 &str)
{
   return ! (static_cast<CsString::CsString>(str) < cStr);
}

// for an array of chars
template <int N>
inline bool operator>=(const QString8 &str, const char (&cStr)[N])
{
   return (static_cast<CsString::CsString>(str) >= cStr);
}

inline void swap(QString8 &a, QString8 &b) {
   a.swap(b);
}

QString8 cs_internal_string_normalize(const QString8 &data, QString8::NormalizationForm mode,
                  QChar32::UnicodeVersion version, int from);

#endif
