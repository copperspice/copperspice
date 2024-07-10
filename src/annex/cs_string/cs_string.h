/***********************************************************************
*
* Copyright (c) 2017-2024 Barbara Geller
* Copyright (c) 2017-2024 Ansel Sermersheim
*
* This file is part of CsString.
*
* CsString is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
* CsString is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

#ifndef LIB_CS_STRING_H
#define LIB_CS_STRING_H

#include <algorithm>
#include <cstring>
#include <cstddef>
#include <exception>
#include <iterator>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <cs_char.h>
#include <cs_string_iterator.h>
#include <cs_encoding.h>

#define UCHAR(x)   (U ## x)

namespace CsString {

template <typename S>
class CsBasicStringView;

using CsString       = CsBasicString<utf8>;
using CsString_utf8  = CsBasicString<utf8>;
using CsString_utf16 = CsBasicString<utf16>;

template <typename E, typename A>
class CsBasicString
{
   public:
      using difference_type        = std::ptrdiff_t;
      using size_type              = std::ptrdiff_t;
      using value_type             = CsChar;

      using const_iterator         = CsStringIterator<E, A>;
      using iterator               = CsStringIterator<E, A>;
      using const_reverse_iterator = CsStringReverseIterator<const_iterator>;
      using reverse_iterator       = CsStringReverseIterator<iterator>;

      using const_storage_iterator         = typename std::vector<typename E::storage_unit, A>::const_iterator;
      using const_storage_reverse_iterator = typename std::vector<typename E::storage_unit, A>::const_reverse_iterator;

      static constexpr const size_type npos = -1;

      CsBasicString()
         : m_string(1, 0)
      {
      }

      explicit CsBasicString(const A &a)
         : m_string(1, 0, a)
      {
      }

      // for a const char * and char *
      template <typename T, typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      CsBasicString(const T &str, const A &a = A());

      // for an array of chars
      template <int N>
      CsBasicString(const char (&str)[N], const A &a = A());


      // for a const char * and char *
      template <typename T, typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      CsBasicString(const T &str, size_type size, const A &a = A());

      // for an array of chars
      template <int N>
      CsBasicString(const char (&str)[N], size_type size, const A &a = A());


      CsBasicString(const char16_t *str, const A &a = A());
      CsBasicString(const char16_t *str, size_type size, const A &a = A());

      CsBasicString(const char32_t *str, const A &a = A());
      CsBasicString(const char32_t *str, size_type size, const A &a = A());


      // substring constructors
      CsBasicString(const CsBasicString &str, size_type indexStart, const A &a = A());
      CsBasicString(const CsBasicString &str, size_type indexStart, size_type size, const A &a = A());

      // fixed size string with same single code point
      CsBasicString(size_type count, CsChar c, const A &a = A());

      // unknown encoding
      // CsBasicString(std::initializer_list<char> str, const A &a = A());

      template <typename U, typename = typename std::enable_if< std::is_convertible<
            decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      CsBasicString(CsBasicStringView<U> str, const A &a = A());

      template <typename U, typename = typename std::enable_if< std::is_convertible<
            decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      CsBasicString(CsBasicStringView<U> str, size_type indexStart, size_type size, const A &a = A());

      // copy a range of CsChar from another string type container
      template <typename Iterator>
      CsBasicString(Iterator begin, Iterator end, const A &a = A());

      CsBasicString(const_iterator begin, const_iterator end, const A &a = A());

      // copy constructor
      CsBasicString(const CsBasicString &str) = default;
      CsBasicString(const CsBasicString &str, const A &a);

      // move constructor
      CsBasicString(CsBasicString && str) = default;
      CsBasicString(CsBasicString && str, const A &a);


#if defined(__cpp_char8_t)
      // support new data type added in C++20

      CsBasicString(const char8_t *str, const A &a = A());
      CsBasicString(const char8_t *str, size_type size, const A &a = A());

      static CsBasicString fromUtf8(const char8_t *str, size_type numOfChars = -1, const A &a = A());
#endif


      // ** operators
      CsBasicString &operator=(const CsBasicString &str) = default;
      CsBasicString &operator=(CsBasicString &&str) = default;

      // unknown encoding
      // CsBasicString &operator=(char c);

      CsBasicString &operator=(CsChar c);

      // for a const char * and char * -or- an array of chars
      template <typename T>
      CsBasicString &operator=(const T &str);

      // unknown encoding
      // CsBasicString &operator=(std::initializer_list<char> list);

      template <typename U, typename = typename std::enable_if< std::is_convertible<
            decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      CsBasicString &operator=(CsBasicStringView<U> str);

      CsBasicString &operator+=(const CsBasicString &str);

      // unknown encoding
      // CsBasicString &operator+=(char c);

      CsBasicString &operator+=(CsChar c);

      // for a const char * and char * -or- an array of chars
      template <typename T>
      CsBasicString &operator+=(const T &str);

      // unknown encoding
      // CsBasicString &operator+=(std::initializer_list<char> list);

      template <typename U, typename = typename std::enable_if< std::is_convertible<
            decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      CsBasicString &operator+=(CsBasicStringView<U> str);

      CsChar operator[](size_type index) const;

      const_iterator advance(const_iterator begin, size_type count) const;
      iterator advance(iterator begin, size_type count);

      // ** methods
      CsBasicString &append(const CsBasicString &str);
      CsBasicString &append(const CsBasicString &str, size_type indexStart, size_type size = npos);

      CsBasicString &append(CsChar c);
      CsBasicString &append(size_type count, CsChar c);

      template <typename Iterator>
      CsBasicString &append(Iterator begin, Iterator end);

      CsBasicString &append(const_iterator begin, const_iterator end);


      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      CsBasicString &append(const T &str, size_type size);

      // for an array of chars
      template <int N>
      CsBasicString &append(const char (&str)[N], size_type size);


      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      CsBasicString &append(const T &str);


      // for an array of chars
      template <int N>
      CsBasicString &append(const char (&str)[N]);

      // unknown encoding
      // CsBasicString &append(std::initializer_list<char> str);

      template <typename U, typename = typename std::enable_if< std::is_convertible<
            decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      CsBasicString &append(CsBasicStringView<U> str);

      template <typename U, typename = typename std::enable_if< std::is_convertible<
            decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      CsBasicString &append(CsBasicStringView<U> str, size_type indexStart, size_type size);

      CsBasicString &assign(size_type count, CsChar c);
      CsBasicString &assign(const CsBasicString &str);
      CsBasicString &assign(const CsBasicString &str, size_type indexStart, size_type size = npos);
      CsBasicString &assign(CsBasicString &&str);

      // for a const char * and char * -or- an array of chars
      template <typename T>
      CsBasicString &assign(const T &str, size_type size);

      // for a const char * and char * -or- an array of chars
      template <typename T>
      CsBasicString &assign(const T &str);

      template <typename Iterator>
      CsBasicString &assign(Iterator begin, Iterator end);

      // unknown encoding
      // CsBasicString &assign(std::initializer_list<CsChar> str)

      template <typename U, typename = typename std::enable_if< std::is_convertible<
         decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      CsBasicString &assign(CsBasicStringView<U> str);

      template <typename U, typename = typename std::enable_if< std::is_convertible<
        decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      CsBasicString &assign(CsBasicStringView<U> str, size_type indexStart, size_type size);

      CsChar at(size_type index) const;
      CsChar back() const;
      void clear();

      bool empty() const;

      CsBasicString &erase(size_type indexStart = 0, size_type size = npos);
      iterator erase(const_iterator iter);
      iterator erase(const_iterator iter_begin, const_iterator iter_end);

      // ** uses an iterator, returns an iterator
      const_iterator find_fast(CsChar c) const;
      const_iterator find_fast(CsChar c, const_iterator iter_begin) const;

      const_iterator find_fast(const CsBasicString &str) const;
      const_iterator find_fast(const CsBasicString &str, const_iterator iter_begin) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      const_iterator find_fast(const T &str, const_iterator iter_begin, size_type size) const;

      // for an array of chars
      template <int N>
      const_iterator find_fast(const char (&str)[N], const_iterator iter_begin, size_type size) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      const_iterator find_fast(const T &str) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      const_iterator find_fast(const T &str, const_iterator iter_begin) const;

      // for an array of chars
      template <int N>
      const_iterator find_fast(const char (&str)[N]) const;

      // for an array of chars
      template <int N>
      const_iterator find_fast(const char (&str)[N], const_iterator iter_begin) const;

      //
      const_iterator rfind_fast(CsChar c) const;
      const_iterator rfind_fast(CsChar c, const_iterator iter_end) const;

      const_iterator rfind_fast(const CsBasicString &str) const;
      const_iterator rfind_fast(const CsBasicString &str, const_iterator iter_end) const;

      // ** uses an index, returns an index
      size_type find(const CsBasicString &str, size_type indexStart = 0) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      size_type find(const T &str, size_type indexStart, size_type size) const;

      // for an array of chars
      template <int N>
      size_type find(const char (&str)[N], size_type indexStart, size_type size) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      size_type find(const T &str, size_type indexStart = 0) const;

      // for an array of chars
      template <int N>
      size_type find(const char (&str)[N], size_type indexStart = 0) const;

      size_type find(CsChar c, size_type indexStart = 0) const;

      // template <typename U, typename = typename std::enable_if< std::is_convertible<
      //   decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      // size_type find(CsBasicStringView<U> str, size_type indexStart = 0) const;


      size_type find_first_of(const CsBasicString &str, size_type indexStart = 0) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      size_type find_first_of(const T &str, size_type indexStart, size_type size) const;

      // for an array of chars
      template <int N>
      size_type find_first_of(const char (&str)[N], size_type indexStart, size_type size) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      size_type find_first_of(const T &str, size_type indexStart = 0) const;

      // for an array of chars
      template <int N>
      size_type find_first_of(const char (&str)[N], size_type indexStart = 0) const;

      size_type find_first_of(CsChar c, size_type indexStart = 0) const;

      // template <typename U, typename = typename std::enable_if< std::is_convertible<
      //   decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      // size_type find_first_of(CsBasicStringView<U> str, size_type indexStart = 0 ) const;


      size_type find_last_of(const CsBasicString &str, size_type indexStart = npos) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      size_type find_last_of(const T &str, size_type indexStart, size_type size) const;

      // for an array of chars
      template <int N>
      size_type find_last_of(const char (&str)[N], size_type indexStart, size_type size) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      size_type find_last_of(const T &str, size_type indexStart = npos) const;

      // for an array of chars
      template <int N>
      size_type find_last_of(const char (&str)[N], size_type indexStart = npos) const;

      size_type find_last_of(CsChar c, size_type indexStart = npos) const;

      // template <typename U, typename = typename std::enable_if< std::is_convertible<
      //   decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      // size_type find_last_of(CsBasicStringView<U> str, size_type indexStart = npos) const


      size_type find_first_not_of(const CsBasicString &str, size_type indexStart = 0) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      size_type find_first_not_of(const T &str, size_type indexStart, size_type size) const;

      // for an array of chars
      template <int N>
      size_type find_first_not_of(const char (&str)[N], size_type indexStart, size_type size) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      size_type find_first_not_of(const T &strc, size_type indexStart = 0) const;

      // for an array of chars
      template <int N>
      size_type find_first_not_of(const char (&str)[N], size_type indexStart = 0) const;

      size_type find_first_not_of(CsChar c, size_type indexStart= 0) const;

      // template <typename U, typename = typename std::enable_if< std::is_convertible<
      //   decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      // size_type find_first_not_of(CsBasicStringView<U> str, size_type indexStart = 0) const;


      size_type find_last_not_of(const CsBasicString &str, size_type indexStart = npos) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      size_type find_last_not_of(const T &str, size_type indexStart, size_type size) const;

      // for an array of chars
      template <int N>
      size_type find_last_not_of(const char (&str)[N], size_type indexStart, size_type size) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      size_type find_last_not_of(const T &str, size_type indexStart = npos) const;

      // for an array of chars
      template <int N>
      size_type find_last_not_of(const char (&str)[N], size_type indexStart = npos) const;

      size_type find_last_not_of(CsChar c, size_type indexStart = npos) const;

      // template <typename U, typename = typename std::enable_if< std::is_convertible<
      //   decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      // size_type find_last_of(CsBasicStringView<U> str, size_type indexStart = npos) const;


      size_type rfind(const CsBasicString &str, size_type indexStart = npos) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      size_type rfind(const T &str, size_type indexStart, size_type size) const;

      // for an array of chars
      template <int N>
      size_type rfind(const char (&str)[N], size_type indexStart, size_type size) const;

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      size_type rfind(const T &str, size_type indexStart = npos) const;

      // for an array of chars
      template <int N>
      size_type rfind(const char (&str)[N], size_type indexStart = npos) const;

      size_type rfind(CsChar c, size_type indexStart = npos) const;

      // template <typename U, typename = typename std::enable_if< std::is_convertible<
      //   decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      // size_type rfind(CsBasicStringView<U> str, size_type indexStart = npos) const;

      CsChar front() const;

      static CsBasicString fromUtf8(const char *str, size_type numOfChars = -1, const A &a = A());
      static CsBasicString fromUtf16(const char16_t *str, size_type numOfChars = -1, const A &a = A());

      A getAllocator() const;

      CsBasicString &insert(size_type indexStart, size_type count, CsChar c);

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      CsBasicString &insert(size_type indexStart, const T &str);

      // for an array of chars
      template <int N>
      CsBasicString &insert(size_type indexStart, const char (&str)[N]);

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      CsBasicString &insert(size_type indexStart, const T &str, size_type size);

      // for an array of chars
      template <int N>
      CsBasicString &insert(size_type indexStart, const char (&str)[N], size_type size);

      CsBasicString &insert(size_type indexStart, const CsBasicString &str);

      CsBasicString &insert(size_type indexStart, const CsBasicString &str,
                  size_type srcStart, size_type size = npos);

      iterator insert(const_iterator posStart, CsChar c);
      iterator insert(const_iterator posStart, size_type count, CsChar c);
      iterator insert(const_iterator posStart, const CsBasicString &str);

      // for a const char * and char *
      template <typename T,  typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      iterator insert(const_iterator posStart, const T &str, size_type size);

      // for an array of chars
      template <int N>
      iterator insert(const_iterator posStart, const char (&str)[N], size_type size);

      template <typename Iterator>
      iterator insert(const_iterator posStart, Iterator begin, Iterator end);

      // unknown encoding
      // iterator insert(const_iterator posStart, std::initializer_list<CsChar> str);

      template <typename U, typename = typename std::enable_if< std::is_convertible<
            decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      iterator insert(size_type indexStart, CsBasicStringView<U> str);

      template <typename U, typename = typename std::enable_if< std::is_convertible<
            decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      iterator insert(size_type indexStart, CsBasicStringView<U> str, size_type srcStart, size_type size = npos);

      void pop_back();
      void push_back(CsChar c);

      CsBasicString &replace(size_type indexStart, size_type size, const CsBasicString &str);
      CsBasicString &replace(const_iterator first, const_iterator last, const CsBasicString &str);

      CsBasicString &replace(size_type indexStart, size_type count, const CsBasicString &str,
                  size_type srcStart, size_type size = npos);

      template <class Iterator>
      CsBasicString &replace(const_iterator first1, const_iterator last1, Iterator first2, Iterator last2);

      // for a const char * and char *
      template <typename T, typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      CsBasicString &replace(size_type indexStart, size_type count, const T &str, size_type size);

      // for an array of chars
      template <int N>
      CsBasicString &replace(size_type indexStart, size_type count, const char (&str)[N], size_type size);

      // for a const char * and char *
      template <typename T, typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      CsBasicString &replace(const_iterator first, const_iterator last, const T &str, size_type size);

      // for an array of chars
      template <int N>
      CsBasicString &replace(const_iterator first, const_iterator last, const char (&str)[N], size_type size);

      // for a const char * and char *
      template <typename T, typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      CsBasicString &replace(size_type indexStart, size_type size, const T &str);

      // for an array of chars
      template <int N>
      CsBasicString &replace(size_type indexStart, size_type size, const char (&str)[N]);

      // for a const char * and char *
      template <typename T, typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      CsBasicString &replace(const_iterator first, const_iterator last, const T &str);

      // for an array of chars
      template <int N>
      CsBasicString &replace(const_iterator first, const_iterator last, const char (&str)[N]);

      CsBasicString &replace(size_type indexStart, size_type size, size_type count, CsChar c);
      CsBasicString &replace(const_iterator first, const_iterator last, size_type count, CsChar c);

      // unknown encoding
      // CsBasicString &replace(const_iterator first, const_iterator last, std::initializer_list<CsChar> str);

      // template <typename U, typename = typename std::enable_if< std::is_convertible<
      //   decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      // basic_string &replace(size_type pos, size_type size, CsBasicStringView<U> str);

      // template <typename U, typename = typename std::enable_if< std::is_convertible<
      //   decltype(*(std::declval<typename U::const_iterator>())), CsChar>::value>::type>
      // basic_string &replace(const_iterator first, const_iterator last, CsBasicStringView<U> str);

      template <class T>
      CsBasicString &replace(size_type indexStart, size_type count, const T &str,
                  size_type srcStart, size_type size = npos);

      template <class T>
      CsBasicString &replace(const_iterator first, const_iterator last, const T &str,
                  size_type srcStart, size_type size = npos);

      iterator replace(const_iterator iter, const CsBasicString &str);

      void resize(size_type size);
      void resize(size_type size, CsChar c);

      void shrink_to_fit();

      size_type size_storage() const;

      // following 3 do the same thing
      size_type size_codePoints() const;
      size_type size() const;
      size_type length() const;

      CsBasicString substr(size_type indexStart = 0, size_type size = npos) const;
      void swap(CsBasicString &str);

      // ** iterators
      const_iterator begin() const;
      const_iterator cbegin() const;

      const_iterator end() const;
      const_iterator cend() const;

      const_reverse_iterator rbegin() const;
      const_reverse_iterator crbegin() const;

      const_reverse_iterator rend() const;
      const_reverse_iterator crend() const;

      // storage iterators
      const_storage_iterator storage_begin() const;
      const_storage_iterator storage_end() const;

      const_storage_reverse_iterator storage_rbegin() const;
      const_storage_reverse_iterator storage_rend() const;

      // returns const ptr to the raw data
      const typename E::storage_unit *constData() const
      {
         return &m_string[0];
      }

   private:
      using str_type = typename std::vector<typename E::storage_unit, A>;
      using str_iter = typename std::vector<typename E::storage_unit, A>::const_iterator;

      str_type m_string;
};

// constructors
template <typename E, typename A>
CsBasicString<E, A>::CsBasicString(const CsBasicString &str, const A &a)
   : m_string(str.m_string, a)
{
}

template <typename E, typename A>
CsBasicString<E, A>::CsBasicString(CsBasicString &&str, const A &a)
   : m_string(std::move(str.m_string), a)
{
}

template <typename E, typename A>
template <typename T, typename>
CsBasicString<E, A>::CsBasicString(const T &str, const A &a)
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // make this safe by treating str as utf8
   *this = CsBasicString::fromUtf8(str, -1, a);
}

template <typename E, typename A>
template <int N>
CsBasicString<E, A>::CsBasicString(const char (&str)[N], const A &a)
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   *this = CsBasicString::fromUtf8(str, N-1, a);
}

template <typename E, typename A>
template <typename T,  typename>
CsBasicString<E, A>::CsBasicString(const T &str, size_type size, const A &a)
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   *this = CsBasicString::fromUtf8(str, size, a);
}

template <typename E, typename A>
template <int N>
CsBasicString<E, A>::CsBasicString(const char (&str)[N], size_type size, const A &a)
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   *this = CsBasicString::fromUtf8(str, size, a);
}

template <typename E, typename A>
CsBasicString<E, A>::CsBasicString(const char16_t *str, const A &a)
{
   *this = CsBasicString::fromUtf16(str, -1, a);
}

template <typename E, typename A>
CsBasicString<E, A>::CsBasicString(const char16_t *str, size_type size, const A &a)
{
   *this = CsBasicString::fromUtf16(str, size, a);
}

template <typename E, typename A>
CsBasicString<E, A>::CsBasicString(const char32_t *str, const A &a)
   : m_string(1, 0, a)
{
   const char32_t *c = str;

   while (*c != 0) {
      E::insert(m_string, m_string.end() - 1, *c);
      ++c;
   }
}

template <typename E, typename A>
CsBasicString<E, A>::CsBasicString(const char32_t *str, size_type size, const A &a)
   : m_string(1, 0, a)
{
   const char32_t *c = str;

   size_type count = 0;

   while (*c != 0) {

      if (size >= 0) {
         if (count >= size) {
            break;
         }

         ++count;
      }

      E::insert(m_string, m_string.end() - 1, *c);
      ++c;
   }
}

template <typename E, typename A>
CsBasicString<E, A>::CsBasicString(const CsBasicString &str, size_type indexStart, const A &a)
   : m_string(1, 0, a)
{
   const_iterator iter_begin = str.cbegin();
   const_iterator iter_end   = str.cend();

   for (size_type i = 0; i < indexStart && iter_begin != str.cend(); ++i)  {
      ++iter_begin;
   }

   if (iter_begin == str.cend()) {
      // index_start > length
      return;
   }

   append(iter_begin, iter_end);
}

template <typename E, typename A>
CsBasicString<E, A>::CsBasicString(const CsBasicString &str, size_type indexStart, size_type size, const A &a)
   : m_string(1, 0, a)
{
   const_iterator iter_begin = str.cbegin();
   const_iterator iter_end;

   for (size_type i = 0; i < indexStart && iter_begin != str.cend(); ++i)  {
      ++iter_begin;
   }

   if (iter_begin == str.cend()) {
      // indexStart > length
      return;
   }

   if (size >= 0) {
      iter_end = iter_begin;

      for (size_type i = 0; i < size && iter_end != str.cend(); ++i)  {
         ++iter_end;
      }

   } else {
      iter_end = str.cend();

   }

   append(iter_begin, iter_end);
}

template <typename E, typename A>
CsBasicString<E, A>::CsBasicString(size_type count, CsChar c, const A &a)
   : m_string(1, 0, a)
{
   E::insert(m_string, m_string.end() - 1, c, count);
}

template <typename E, typename A>
template <typename U, typename>
CsBasicString<E, A>::CsBasicString(CsBasicStringView<U> str, const A &a)
   : CsBasicString(str.begin(), str.end(), a)
{
   static_assert(std::is_base_of<CsBasicString<E,A>, U>::value,
      "Unable to construct a CsBasicString using a CsBasicStringView, encoding E is "
      "incompatible with the encoding for U");
}

template <typename E, typename A>
template <typename U,  typename>
CsBasicString<E, A>::CsBasicString(CsBasicStringView<U> str, size_type indexStart, size_type size, const A &a)
   : m_string(1, 0, a)
{
 static_assert(std::is_base_of<CsBasicString<E,A>, U>::value,
      "Unable to construct a CsBasicString using a CsBasicStringView, encoding E is "
      "incompatible with the encoding for U");

   typename U::const_iterator iter_begin = str.cbegin();
   typename U::const_iterator iter_end;

   for (size_type i = 0; i < indexStart && iter_begin != str.cend(); ++i)  {
      ++iter_begin;
   }

   if (iter_begin == str.cend()) {
      // indexStart > length
      return;
   }

   if (size >= 0) {
      iter_end = iter_begin;

      for (size_type i = 0; i < size && iter_end != str.cend(); ++i)  {
         ++iter_end;
      }

   } else {
      iter_end = str.cend();

   }

   append(iter_begin, iter_end);
}

template <typename E, typename A>
template <typename Iterator>
CsBasicString<E, A>::CsBasicString(Iterator begin, Iterator end, const A &a)
   : m_string(1, 0, a)
{
   for (Iterator item = begin; item != end; ++item) {
      E::insert(m_string, m_string.end() - 1, *item);
   }
}

template <typename E, typename A>
CsBasicString<E, A>::CsBasicString(const_iterator begin, const_iterator end, const A &a)
   : m_string(begin.codePointBegin(), end.codePointBegin(), a)
{
   m_string.push_back(0);
}

// operators
template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::operator=(CsChar c)
{
   m_string.clear();
   m_string.push_back(0);

   append(c);
   return *this;
}

template <typename E, typename A>
template <typename T>
CsBasicString<E, A> &CsBasicString<E, A>::operator=(const T &str)
{
   // str is a const char * -or- an array of chars

   m_string.clear();
   m_string.push_back(0);

   append(str);
   return *this;
}

template <typename E, typename A>
template <typename U, typename>
CsBasicString<E, A> &CsBasicString<E,A>::operator=(CsBasicStringView<U> str)
{
   static_assert(std::is_base_of<CsBasicString<E,A>, U>::value,
      "Unable to construct a CsBasicString using a CsBasicStringView, encoding E is "
      "incompatible with the encoding for U");

   m_string.clear();
   m_string.push_back(0);

   append(str);
   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::operator+=(const CsBasicString &str)
{
   append(str);
   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::operator+=(CsChar c)
{
   append(c);
   return *this;
}

template <typename E, typename A>
template <typename T>
CsBasicString<E, A> &CsBasicString<E, A>::operator+=(const T &str)
{
   // str is a const char * -or- an array of chars

   append(str);
   return *this;
}

template <typename E, typename A>
template <typename U, typename>
CsBasicString<E, A> &CsBasicString<E,A>::operator+=(CsBasicStringView<U> str)
{
   static_assert(std::is_base_of<CsBasicString<E,A>, U>::value,
      "Unable to construct a CsBasicString using a CsBasicStringView, encoding E is "
      "incompatible with the encoding for U");

   append(str);
   return *this;
}

template <typename E, typename A>
CsChar CsBasicString<E, A>::operator[](size_type index) const
{
   const_iterator iter = begin();
   std::advance(iter, index);

   return *iter;
}


// methods
template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::advance(const_iterator begin, size_type count) const
{
   return const_iterator(E::advance(begin.codePointBegin(), cend().codePointBegin(), count));
}

template <typename E, typename A>
typename CsBasicString<E, A>::iterator CsBasicString<E, A>::advance(iterator begin, size_type count)
{
   return iterator(E::advance(begin.codePointBegin(), end().codePointBegin(), count));
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::append(const CsBasicString &str)
{
   insert(end(), str);
   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::append(const CsBasicString &str, size_type indexStart, size_type size)
{
   size_type stringLen = this->size();
   insert(stringLen, str, indexStart, size);

   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::append(CsChar c)
{
   E::insert(m_string, m_string.end() - 1, c);
   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::append(size_type count, CsChar c)
{
   E::insert(m_string, m_string.end() - 1, c, count);
   return *this;
}

template <typename E, typename A>
template <typename Iterator>
CsBasicString<E, A> &CsBasicString<E, A>::append(Iterator begin, Iterator end)
{
   for (Iterator item = begin; item != end; ++item) {
      E::insert(m_string, m_string.end() - 1, *item);
   }

   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::append(const_iterator begin, const_iterator end)
{
   m_string.insert(m_string.end() - 1, begin.codePointBegin(), end.codePointBegin());

   return *this;
}

template <typename E, typename A>
template <typename T,  typename>
CsBasicString<E, A> &CsBasicString<E, A>::append(const T &str, size_type size)
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   if (str == nullptr) {
      return *this;
   }

   // make this safe by treating str as utf8
   this->append(CsBasicString::fromUtf8(str, size));

   return *this;
}

template <typename E, typename A>
template <int N>
CsBasicString<E, A> &CsBasicString<E, A>::append(const char (&str)[N], size_type size)
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   this->append(CsBasicString::fromUtf8(str, size));

   return *this;
}

template <typename E, typename A>
template <typename T,  typename>
CsBasicString<E, A> &CsBasicString<E, A>::append(const T &str)
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   if (str == nullptr) {
      return *this;
   }

   // make this safe by treating str as utf8
   this->append(CsBasicString::fromUtf8(str));

   return *this;
}

template <typename E, typename A>
template <int N>
CsBasicString<E, A> &CsBasicString<E, A>::append(const char (&str)[N])
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   this->append(CsBasicString::fromUtf8(str, N-1));

   return *this;
}

template <typename E, typename A>
template <typename U,  typename>
CsBasicString<E, A> &CsBasicString<E, A>::append(CsBasicStringView<U> str)
{
 static_assert(std::is_base_of<CsBasicString<E,A>, U>::value,
      "Unable to construct a CsBasicString using a CsBasicStringView, encoding E is "
      "incompatible with the encoding for U");

   append(str.cbegin(), str.cend());
   return *this;
}

template <typename E, typename A>
template <typename U,  typename>
CsBasicString<E, A> &CsBasicString<E, A>::append(CsBasicStringView<U> str, size_type indexStart, size_type size)
{
   static_assert(std::is_base_of<CsBasicString<E,A>, U>::value,
      "Unable to construct a CsBasicString using a CsBasicStringView, encoding E is "
      "incompatible with the encoding for U");

   typename U::const_iterator iter_begin = str.cbegin();
   typename U::const_iterator iter_end;

   for (size_type i = 0; i < indexStart && iter_begin != str.cend(); ++i)  {
      ++iter_begin;
   }

   if (iter_begin == str.cend()) {
      return *this;
   }

   if (size >= 0) {
      iter_end = iter_begin;

      for (size_type i = 0; i < size && iter_end != str.cend(); ++i)  {
         ++iter_end;
      }

   } else {
      iter_end = str.cend();

   }

   append(iter_begin, iter_end);
   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::assign(size_type count, CsChar c)
{
   clear();
   append(count, c);

   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::assign(const CsBasicString &str)
{
   clear();
   append(str);

   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::assign(const CsBasicString &str, size_type indexStart, size_type size)
{
   clear();
   append(str, indexStart, size);

   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::assign(CsBasicString &&str)
{
   clear();
   append(std::move(str));

   return *this;
}

template <typename E, typename A>
template <typename T>
CsBasicString<E, A> &CsBasicString<E, A>::assign(const T &str, size_type size)
{
   // str is a const char * -or- an array of chars

   clear();
   append(str, size);

   return *this;
}

template <typename E, typename A>
template <typename T>
CsBasicString<E, A> &CsBasicString<E, A>::assign(const T &str)
{
   // str is a const char * -or- an array of chars

   clear();
   append(str);

   return *this;
}

template <typename E, typename A>
template <typename Iterator>
CsBasicString<E, A> &CsBasicString<E, A>::assign(Iterator begin, Iterator end)
{
   clear();
   append(begin, end);

   return *this;
}

template <typename E, typename A>
template <typename U,  typename>
CsBasicString<E, A> &CsBasicString<E, A>::assign(CsBasicStringView<U> str)
{
 static_assert(std::is_base_of<CsBasicString<E,A>, U>::value,
      "Unable to construct a CsBasicString using a CsBasicStringView, encoding E is "
      "incompatible with the encoding for U");

   clear();
   append(str.cbegin(), str.cend());

   return *this;
}

template <typename E, typename A>
template <typename U,  typename>
CsBasicString<E, A> &CsBasicString<E, A>::assign(CsBasicStringView<U> str, size_type indexStart, size_type size)
{
   static_assert(std::is_base_of<CsBasicString<E,A>, U>::value,
      "Unable to construct a CsBasicString using a CsBasicStringView, encoding E is "
      "incompatible with the encoding for U");

   clear();
   append(str, indexStart, size);

   return *this;
}

template <typename E, typename A>
CsChar CsBasicString<E, A>::at(size_type index) const
{
   const_iterator iter = begin();
   std::advance(iter, index);

   return *iter;
}

template <typename E, typename A>
CsChar CsBasicString<E, A>::back() const
{
   return *(--end());
}

template <typename E, typename A>
void CsBasicString<E, A>::clear()
{
   m_string.clear();
   m_string.push_back(0);
}

template <typename E, typename A>
bool CsBasicString<E, A>::empty() const
{
   return (m_string.size() == 1);
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::erase(size_type indexStart, size_type size)
{
   const_iterator iter_begin = cbegin();
   const_iterator iter_end;

   for (size_type i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (iter_begin == cend()) {
      return *this;
   }

   if (size >= 0) {
      iter_end = iter_begin;

      for (size_type i = 0; i < size && iter_end != cend(); ++i)  {
         ++iter_end;
      }

   } else {
      iter_end = cend();

   }

   erase(iter_begin, iter_end);

   return *this;
}

template <typename E, typename A>
typename CsBasicString<E, A>::iterator CsBasicString<E, A>::erase(const_iterator iter)
{
   auto [vbegin, vend] = iter.codePointRange();
   auto retval = m_string.erase(vbegin, vend);

   return const_iterator(retval);
}

template <typename E, typename A>
typename CsBasicString<E, A>::iterator CsBasicString<E, A>::erase(const_iterator iter_begin, const_iterator iter_end)
{
   auto retval = m_string.erase(iter_begin.codePointBegin(), iter_end.codePointBegin());
   return const_iterator(retval);
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::find_fast(const CsBasicString &str) const
{
   return find_fast(str, begin());
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::find_fast(const CsBasicString &str,
                  const_iterator iter_begin) const
{
   const_iterator iter_end = end();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   if (str.empty()) {
      return iter_begin;
   }

   auto iter = iter_begin;
   auto ch   = str[0];

   while (iter != iter_end)   {

      if (*iter == ch)  {
         auto text_iter    = iter + 1;
         auto pattern_iter = str.begin() + 1;

         while (text_iter != iter_end && pattern_iter != str.end())  {

            if (*text_iter == *pattern_iter)  {
               ++text_iter;
               ++pattern_iter;

            } else {
               break;

            }
         }

         if (pattern_iter == str.end()) {
            // found a match
            return iter;
         }
      }

      ++iter;
   }

   return iter_end;
}

// for a const char * and char *
template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::find_fast(const T &str, const_iterator iter_begin,
                  size_type size) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   const_iterator iter_end = cend();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   if (str == nullptr || str[0] == '\0') {
      return iter_begin;
   }

   // make this safe by treating str as utf8
   return find_fast(CsBasicString::fromUtf8(str, size), iter_begin);
}

// for an array of chars
template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::find_fast(const char (&str)[N], const_iterator iter_begin,
                  size_type size) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return find_fast(CsBasicString::fromUtf8(str, size), iter_begin);
}

// for a const char * and char *
template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::find_fast(const T &str) const
{
   return find_fast(str, cbegin());
}

// for a const char * and char *
template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::find_fast(const T &str, const_iterator iter_begin) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   const_iterator iter_end = cend();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   if (str == nullptr || str[0] == '\0') {
      return iter_begin;
   }

   // make this safe by treating str as utf8
   return find_fast(CsBasicString::fromUtf8(str), iter_begin);
}

// for an array of chars
template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::find_fast(const char (&str)[N]) const
{
   return find_fast(str, cbegin());
}

// for an array of chars
template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::find_fast(const char (&str)[N], const_iterator iter_begin) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return find_fast(CsBasicString::fromUtf8(str, N-1), iter_begin);
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::find_fast(CsChar c) const
{
   return find_fast(c, begin());
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::find_fast(CsChar c, const_iterator iter_begin) const
{
   const_iterator iter_end = end();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   auto iter = iter_begin;

   while (iter != iter_end)   {

      if (*iter == c)  {
         return iter;
      }

      ++iter;
   }

   return iter_end;
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::rfind_fast(CsChar c) const
{
   return rfind_fast(c, end());
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::rfind_fast(CsChar c, const_iterator iter_end) const
{
   const_iterator iter_begin = begin();

   if (iter_begin == iter_end) {
      return end();
   }

   auto iter = iter_end;

   while (iter != begin())   {
      --iter;

      if (*iter == c)  {
         // found a match
         return iter;
      }
   }

   return end();
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::rfind_fast(const CsBasicString &str) const
{
   return rfind_fast(str, end());
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::rfind_fast(const CsBasicString &str,
                  const_iterator iter_end) const
{
   const_iterator iter_begin = begin();

   if (iter_begin == iter_end) {
      return end();
   }

   if (str.empty()) {
      return iter_end;
   }

   auto iter    = iter_end;
   auto str_end = str.end();
   auto ch      = str[0];

   while (iter != begin())   {
      --iter;

      if (*iter == ch)  {

         auto text_iter    = iter + 1;
         auto pattern_iter = str.begin() + 1;

         while (text_iter != end() && pattern_iter != str_end)  {

            if (*text_iter == *pattern_iter)  {
               ++text_iter;
               ++pattern_iter;

            } else {
               break;

            }
         }

         if (pattern_iter == str_end) {
            // found a match
            return iter;
         }
      }

   }

   return end();
}

template <typename E, typename A>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find(CsChar c, size_type indexStart) const
{
   const_iterator iter_begin = cbegin();

   for (size_type i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (iter_begin == cend()) {
      // indexStart > length
      return -1;
   }

   size_type retval = indexStart;

   while (iter_begin != end())   {

      if (*iter_begin == c)  {
         return retval;
      }

      ++iter_begin;
      ++retval;
   }

   return -1;
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find(const T &str, size_type indexStart,
                  size_type size) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // make this safe by treating str as utf8
   return find(CsBasicString::fromUtf8(str, size), indexStart);
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find(const char (&str)[N], size_type indexStart,
                  size_type size) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return find(CsBasicString::fromUtf8(str, size), indexStart);
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find(const T &str, size_type indexStart) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   size_type stringLen = this->size();

   if (str == nullptr || *str == '\0') {

      if (indexStart > stringLen) {
         return -1;
      } else {
         return indexStart;
      }
   }

   if (indexStart >= stringLen) {
      return -1;
   }

   // make this safe by treating str as utf8
   return find(CsBasicString::fromUtf8(str), indexStart);
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find(const char (&str)[N], size_type indexStart) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return find(CsBasicString::fromUtf8(str, N-1), indexStart);
}

template <typename E, typename A>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find(const CsBasicString &str, size_type indexStart) const
{
   size_type stringLen = this->size();

   if (str.empty()) {

      if (indexStart > stringLen) {
         return -1;
      } else {
         return indexStart;
      }
   }

   if (indexStart >= stringLen) {
      return -1;
   }

   size_type retval = indexStart;
   auto iter        = begin() + indexStart;
   auto ch          = str[0];

   while (iter != end())   {

      if (*iter == ch)  {
         auto text_iter    = iter + 1;
         auto pattern_iter = str.begin() + 1;

         while (text_iter != end() && pattern_iter != str.end())  {

            if (*text_iter == *pattern_iter)  {
               ++text_iter;
               ++pattern_iter;

            } else {
               break;

            }
         }

         if (pattern_iter == str.end()) {
            // found a match
            return retval;
         }
      }

      ++iter;
      ++retval;
   }

   return -1;
}

template <typename E, typename A>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_first_of(CsChar c, size_type indexStart) const
{
   return find(c, indexStart);
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_first_of(const T &str, size_type indexStart,
                  size_type size) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   if (str == nullptr || *str == '\0' || indexStart >= this->size()) {
      return -1;
   }

   // make this safe by treating str as utf8
   return find_first_of(CsBasicString::fromUtf8(str, size), indexStart);
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_first_of(const char (&str)[N], size_type indexStart,
                  size_type size) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return find_first_of(CsBasicString::fromUtf8(str, size), indexStart);
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_first_of(const T &str, size_type indexStart) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   if (str == nullptr || *str == '\0' || indexStart >= this->size()) {
      return -1;
   }

   // make this safe by treating str as utf8
   return find_first_of(CsBasicString::fromUtf8(str), indexStart);
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_first_of(const char (&str)[N],
                  size_type indexStart) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return find_first_of(CsBasicString::fromUtf8(str, N-1), indexStart);
}

template <typename E, typename A>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_first_of(const CsBasicString &str,
                  size_type indexStart) const
{
   if (str.empty() || indexStart >= this->size()) {
      return -1;
   }

   size_type retval = indexStart;
   auto iter        = begin() + indexStart;

   while (iter != end())   {
      for (auto c : str) {

         if (*iter == c) {
            // found a match
            return retval;
         }
      }

      ++iter;
      ++retval;
   }

   return -1;
}

template <typename E, typename A>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_last_of(CsChar c, size_type indexStart) const
{
   return rfind(c, indexStart);
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_last_of(const T &str, size_type indexStart,
                  size_type size) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   size_type stringLen = this->size();

   if (str == nullptr || *str == '\0' || indexStart >= stringLen) {
      return -1;
   }

   // make this safe by treating str as utf8
   return find_last_of(CsBasicString::fromUtf8(str, size), indexStart);
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_last_of(const char (&str)[N],
                  size_type indexStart, size_type size) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return find_last_of(CsBasicString::fromUtf8(str, size), indexStart);
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_last_of(const T &str, size_type indexStart) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   size_type stringLen = this->size();

   if (str == nullptr || *str == '\0' || indexStart >= stringLen) {
      return -1;
   }

   // make this safe by treating str as utf8
   return find_last_of(CsBasicString::fromUtf8(str), indexStart);
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_last_of(const char (&str)[N],
                  size_type indexStart) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return find_last_of(CsBasicString::fromUtf8(str, N-1), indexStart);
}

template <typename E, typename A>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_last_of(const CsBasicString &str,
                  size_type indexStart) const
{
   size_type stringLen = this->size();

   if (str.empty() || indexStart >= stringLen) {
      return -1;
   }

   size_type retval;
   const_iterator iter;

   if (indexStart >= 0 && indexStart < stringLen) {
      retval = indexStart + 1;
      iter   = begin() + indexStart + 1;

   } else {
      retval = stringLen;
      iter   = end();

   }

   while (iter != begin())   {
      --iter;
      --retval;

      for (CsChar c : str) {

         if (*iter == c) {
            // found a match
            return retval;
         }
      }
   }

   return -1;
}

template <typename E, typename A>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_first_not_of(CsChar c, size_type indexStart) const
{
   if (indexStart >= this->size()) {
      return -1;
   }

   size_type retval = indexStart;
   auto iter        = begin() + indexStart;

   while (iter != end())   {

      if (*iter != c)  {
         return retval;
      }

      ++iter;
      ++retval;
   }

   return -1;
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_first_not_of(const T &str,
                  size_type indexStart, size_type size) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   size_type stringLen = this->size();

   if (str == nullptr || *str == '\0') {

      if (indexStart >= stringLen) {
         return -1;
      } else {
         return indexStart;
      }
   }

   if (indexStart >= stringLen) {
      return -1;
   }

   // make this safe by treating str as utf8
   return find_first_not_of(CsBasicString::fromUtf8(str, size), indexStart);
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_first_not_of(const char (&str)[N],
                  size_type indexStart, size_type size) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return find_first_not_of(CsBasicString::fromUtf8(str, size), indexStart);
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_first_not_of(const T &str,
                  size_type indexStart) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   size_type stringLen = this->size();

   if (str == nullptr || *str == '\0') {

      if (indexStart >= stringLen) {
         return -1;
      } else {
         return indexStart;
      }
   }

   if (indexStart >= stringLen) {
      return -1;
   }

   // make this safe by treating str as utf8
   return find_first_not_of(CsBasicString::fromUtf8(str), indexStart);
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_first_not_of(const char (&str)[N],
                  size_type indexStart) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return find_first_not_of(CsBasicString::fromUtf8(str, N-1), indexStart);
}

template <typename E, typename A>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_first_not_of(const CsBasicString &str,
                  size_type indexStart) const
{
   size_type stringLen = this->size();

   if (str.empty()) {

      if (indexStart >= stringLen) {
         return -1;
      } else {
         return indexStart;
      }
   }

   if (indexStart >= stringLen) {
      return -1;
   }

   size_type retval = indexStart;
   auto iter        = begin() + indexStart;

   auto str_end     = str.end();

   while (iter != end())   {
      const_iterator pattern_iter = str.begin();

      while (pattern_iter != str_end)  {

         if (*iter == *pattern_iter) {
            // found a match between the current character in m_string and the pattern
            break;
         }

         ++pattern_iter;
      }

      if (pattern_iter == str_end) {
         // current character in m_string did not match the pattern
         return retval;
      }

      ++iter;
      ++retval;
   }

   return -1;
}

template <typename E, typename A>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_last_not_of(CsChar c, size_type indexStart) const
{
   size_type stringLen = this->size();

   if (indexStart >= stringLen) {
      return -1;
   }

   size_type retval;
   const_iterator iter;

   if (indexStart >= 0 && indexStart < stringLen) {
      retval = indexStart + 1;
      iter   = begin() + indexStart + 1;

   } else {
      retval = stringLen;
      iter   = end();

   }

   while (iter != begin())   {
      --iter;
      --retval;

      if (*iter != c)  {
         return retval;
      }
   }

   return -1;
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_last_not_of(const T &str, size_type indexStart,
                  size_type size) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   size_type stringLen = this->size();

   if (str == nullptr || *str == '\0') {

      if (indexStart > stringLen || indexStart == -1) {
         return stringLen - 1;
      } else {
         return indexStart;
      }
   }

   // make this safe by treating str as utf8
   return find_last_not_of(CsBasicString::fromUtf8(str, size), indexStart);
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_last_not_of(const char (&str)[N],
                  size_type indexStart, size_type size) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return find_last_not_of(CsBasicString::fromUtf8(str, size), indexStart);
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_last_not_of(const T &str, size_type indexStart) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   size_type stringLen = this->size();

   if (str == nullptr || *str == '\0') {

      if (indexStart > stringLen || indexStart == -1) {
         return stringLen - 1;
      } else {
         return indexStart;
      }
   }

   // make this safe by treating str as utf8
   return find_last_not_of(CsBasicString::fromUtf8(str), indexStart);
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_last_not_of(const char (&str)[N],
                  size_type indexStart) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return find_last_not_of(CsBasicString::fromUtf8(str, N-1), indexStart);
}

template <typename E, typename A>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::find_last_not_of(const CsBasicString &str,
                  size_type indexStart) const
{
   size_type stringLen = this->size();

   if (str.empty()) {

      if (indexStart > stringLen || indexStart == -1) {
         return stringLen - 1;
      } else {
         return indexStart;
      }
   }

   size_type retval;
   const_iterator iter;

   if (indexStart >= 0 && indexStart < stringLen) {
      retval = indexStart + 1;
      iter   = begin() + indexStart + 1;

   } else {
      retval = stringLen;
      iter   = end();

   }

   const_iterator str_end = str.end();

   while (iter != begin())   {
      --iter;
      --retval;

      const_iterator pattern_iter = str.begin();

      while (pattern_iter != str_end)  {

         if (*iter == *pattern_iter) {
            // found a match between the current character in m_string and the pattern
            break;
         }

         ++pattern_iter;
      }

      if (pattern_iter == str_end) {
         // current character in m_string did not match the pattern
         return retval;
      }
   }

   return -1;
}

template <typename E, typename A>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::rfind(CsChar c, size_type indexStart) const
{
   size_type stringLen = this->size();

   size_type retval;
   const_iterator iter;

   if (indexStart >= 0 && indexStart < stringLen) {
      retval = indexStart + 1;
      iter   = begin() + indexStart + 1;

   } else {
      retval = stringLen;
      iter   = end();

   }

   while (iter != begin())   {
      --iter;
      --retval;

      if (*iter == c)  {
         return retval;
      }
   }

   return -1;
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::rfind(const T &str, size_type indexStart,
                  size_type size) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   size_type stringLen = this->size();

   if (str == nullptr || *str == '\0') {

      if (indexStart > stringLen || indexStart == -1) {
         return stringLen;
      } else {
         return indexStart;
      }
   }

   // make this safe by treating str as utf8
   return rfind(CsBasicString::fromUtf8(str, size), indexStart);
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::rfind(const char (&str)[N],
                  size_type indexStart, size_type size) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return rfind(CsBasicString::fromUtf8(str, size), indexStart);
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::rfind(const T &str, size_type indexStart) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   size_type stringLen = this->size();

   if (str == nullptr || *str == '\0') {

      if (indexStart > stringLen || indexStart == -1) {
         return stringLen;
      } else {
         return indexStart;
      }
   }

   // make this safe by treating str as utf8
   return rfind(CsBasicString::fromUtf8(str), indexStart);
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::rfind(const char (&str)[N], size_type indexStart) const
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return rfind(CsBasicString::fromUtf8(str, N-1), indexStart);
}

template <typename E, typename A>
typename CsBasicString<E, A>::size_type CsBasicString<E, A>::rfind(const CsBasicString &str, size_type indexStart) const
{
   size_type stringLen = this->size();

   if (str.empty()) {

      if (indexStart > stringLen || indexStart == -1) {
         return stringLen;
      } else {
         return indexStart;
      }
   }

   size_type retval;
   const_iterator iter;

   if (indexStart >= 0 && indexStart < stringLen) {
      retval = indexStart + 1;
      iter   = begin() + indexStart + 1;

   } else {
      retval = stringLen;
      iter   = end();

   }

   const_iterator str_end = str.end();
   auto ch = str[0];

   while (iter != begin())   {
      --iter;
      --retval;

      if (*iter == ch)  {

         auto text_iter    = iter + 1;
         auto pattern_iter = str.begin() + 1;

         while (text_iter != end() && pattern_iter != str_end)  {

            if (*text_iter == *pattern_iter)  {
               ++text_iter;
               ++pattern_iter;

            } else {
               break;

            }
         }

         if (pattern_iter == str_end) {
            // found a match
            return retval;
         }
      }

   }

   return -1;
}

template <typename E, typename A>
CsChar CsBasicString<E, A>::front() const
{
   return *begin();
}

template <typename E, typename A>
CsBasicString<E,A> CsBasicString<E, A>::fromUtf8(const char *str, size_type numOfChars, const A &a)
{
   CsBasicString retval(a);

   if (str == nullptr) {
      return retval;
   }

   if (numOfChars < 0) {
      numOfChars = std::strlen(str);
   }

   int multi_size = 0;
   char32_t data  = 0;

   for (int i = 0; i < numOfChars; ++i) {

      if ((str[i] & 0x80) == 0) {

         if (multi_size != 0) {
            // multi byte sequence was too short
            multi_size = 0;
            retval.append(UCHAR('\uFFFD'));
         }

         retval.append(static_cast<char32_t>(str[i]));

      } else if ((str[i] & 0xC0) == 0x80) {
         // continuation char

         data = (data << 6) | (static_cast<char32_t>(str[i]) & 0x3F);

         if (multi_size == 2 && data >= 0x80 && data <= 0x7FF) {
            multi_size = 0;
            retval.append(data);

         } else if (multi_size == 3 && data >= 0x800 && data <= 0xFFFF) {
            multi_size = 0;
            retval.append(data);

         } else if (multi_size == 4 && data >= 0x10000 && data <= 0x10FFFF) {
            multi_size = 0;
            retval.append(data);

         }

      } else if ((str[i] & 0xE0) == 0xC0) {
         // begin two byte sequence

         if (multi_size != 0) {
            // preceding multi byte sequence was too short
            retval.append(UCHAR('\uFFFD'));
         }

         multi_size = 2;
         data = static_cast<char32_t>(str[i]) & 0x1F;

      } else if ((str[i] & 0xF0) == 0xE0) {
         // begin three byte sequence

         if (multi_size != 0) {
            // preceding multi byte sequence was too short
            retval.append(UCHAR('\uFFFD'));
         }

         multi_size = 3;
         data = static_cast<char32_t>(str[i]) & 0x0F;

      } else if ((str[i] & 0xF8) == 0xF0) {
        // begin four byte sequence

         if (multi_size != 0) {
            // preceding multi byte sequence was too short
            retval.append(UCHAR('\uFFFD'));
         }

         multi_size = 4;
         data = static_cast<char32_t>(str[i]) & 0x07;

      } else {
         // invalid character

         if (multi_size != 0) {
            // preceding multi byte sequence was too short
            multi_size = 0;
            retval.append(UCHAR('\uFFFD'));
         }

         retval.append(UCHAR('\uFFFD'));
      }
   }

   if (multi_size != 0) {
      // invalid character at the end of the string
      retval.append(UCHAR('\uFFFD'));
   }

   return retval;
}

#if defined(__cpp_char8_t)
   // support new data type added in C++20

template <typename E, typename A>
CsBasicString<E,A> CsBasicString<E, A>::fromUtf8(const char8_t *str, size_type numOfChars, const A &a)
{
   CsBasicString retval(a);

   if (str == nullptr) {
      return retval;
   }

   if (numOfChars < 0) {
      numOfChars = std::char_traits<char8_t>::length(str);
   }

   int multi_size = 0;
   char32_t data  = 0;

   for (int i = 0; i < numOfChars; ++i) {

      if ((str[i] & 0x80) == 0) {

         if (multi_size != 0) {
            // multi byte sequence was too short
            multi_size = 0;
            retval.append(UCHAR('\uFFFD'));
         }

         retval.append(static_cast<char32_t>(str[i]));

      } else if ((str[i] & 0xC0) == 0x80) {
         // continuation char

         data = (data << 6) | (static_cast<char32_t>(str[i]) & 0x3F);

         if (multi_size == 2 && data >= 0x80 && data <= 0x7FF) {
            multi_size = 0;
            retval.append(data);

         } else if (multi_size == 3 && data >= 0x800 && data <= 0xFFFF) {
            multi_size = 0;
            retval.append(data);

         } else if (multi_size == 4 && data >= 0x10000 && data <= 0x10FFFF) {
            multi_size = 0;
            retval.append(data);

         }

      } else if ((str[i] & 0xE0) == 0xC0) {
         // begin two byte sequence

         if (multi_size != 0) {
            // preceding multi byte sequence was too short
            retval.append(UCHAR('\uFFFD'));
         }

         multi_size = 2;
         data = static_cast<char32_t>(str[i]) & 0x1F;

      } else if ((str[i] & 0xF0) == 0xE0) {
         // begin three byte sequence

         if (multi_size != 0) {
            // preceding multi byte sequence was too short
            retval.append(UCHAR('\uFFFD'));
         }

         multi_size = 3;
         data = static_cast<char32_t>(str[i]) & 0x0F;

      } else if ((str[i] & 0xF8) == 0xF0) {
        // begin four byte sequence

         if (multi_size != 0) {
            // preceding multi byte sequence was too short
            retval.append(UCHAR('\uFFFD'));
         }

         multi_size = 4;
         data = static_cast<char32_t>(str[i]) & 0x07;

      } else {
         // invalid character

         if (multi_size != 0) {
            // preceding multi byte sequence was too short
            multi_size = 0;
            retval.append(UCHAR('\uFFFD'));
         }

         retval.append(UCHAR('\uFFFD'));
      }
   }

   if (multi_size != 0) {
      // invalid character at the end of the string
      retval.append(UCHAR('\uFFFD'));
   }

   return retval;
}

#endif

template <typename E, typename A>
CsBasicString<E,A> CsBasicString<E, A>::fromUtf16(const char16_t *str, size_type numOfChars, const A &a)
{
   CsBasicString retval(a);

   if (str == nullptr) {
      return retval;
   }

   if (numOfChars < 0) {
      numOfChars = std::char_traits<char16_t>::length(str);
   }

   char32_t data = 0;

   for (int i = 0; i < numOfChars; ++i) {

      char16_t value = str[i];

      if (value < 0xD800 || value > 0xDFFF) {
         // not a surrogate, value must be less than 0xFFFF

         if (data == 0) {
            // do nothing

         } else {
            // invalid character
            retval.append(UCHAR('\uFFFD'));
            data = 0;
         }

         retval.append(static_cast<char32_t>(str[i]));

      } else if (value >= 0xD800 && value <= 0xDBFF) {
         // high surrogates

         if (data == 0) {
            // do nothing

         } else {
            // invalid character
            retval.append(UCHAR('\uFFFD'));
            data = 0;
         }

         data = static_cast<char32_t>(value) & 0x3FF;

      } else if (value >= 0xDC00 && value <= 0xDFFF) {
         // low surrogates

         if (data == 0) {
            // invalid character
            retval.append(UCHAR('\uFFFD'));

         } else {
            data = (data << 10) | (static_cast<char32_t>(value) & 0x3FF);
            data |= 0x010000;

            retval.append(data);
         }

         data = 0;

      } else {
         // invalid character ( unreachable code )
         retval.append(UCHAR('\uFFFD'));
      }
   }

   if (data != 0) {
      // invalid character at the end of the string
      retval.append(UCHAR('\uFFFD'));
   }

   return retval;
}

template <typename E, typename A>
A CsBasicString<E, A>::getAllocator() const
{
   return m_string.get_allocator();
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::insert(size_type indexStart, size_type count, CsChar c)
{
   const_iterator iter_begin = cbegin();
   size_type i;

   for (i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (i != indexStart) {
      throw std::out_of_range("CsString::insert index out of range");
   }

   E::insert(m_string, iter_begin.codePointBegin(), c, count);

   return *this;
}

template <typename E, typename A>
template <typename T,  typename>
CsBasicString<E, A> &CsBasicString<E, A>::insert(size_type indexStart, const T &str)
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   if (str == nullptr || *str == '\0') {
      return *this;
   }

   // make this safe by treating str as utf8
   return insert(indexStart, CsBasicString::fromUtf8(str));
}

template <typename E, typename A>
template <int N>
CsBasicString<E, A> &CsBasicString<E, A>::insert(size_type indexStart, const char (&str)[N])
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return insert(indexStart, CsBasicString::fromUtf8(str, N -1));
}

template <typename E, typename A>
template <typename T,  typename>
CsBasicString<E, A> &CsBasicString<E, A>::insert(size_type indexStart, const T &str, size_type size)
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   if (str == nullptr || *str == '\0') {
      return *this;
   }

   // make this safe by treating str as utf8
   return insert(indexStart, CsBasicString::fromUtf8(str, size));
}

template <typename E, typename A>
template <int N>
CsBasicString<E, A> &CsBasicString<E, A>::insert(size_type indexStart, const char (&str)[N], size_type size)
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return insert(indexStart, CsBasicString::fromUtf8(str, size));
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::insert(size_type indexStart, const CsBasicString &str)
{
   const_iterator iter_begin = cbegin();
   size_type i;

   for (i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (i != indexStart) {
      throw std::out_of_range("CsString::insert index out of range");
   }

   for (CsChar c : str) {
      str_iter iter_tmp = E::insert(m_string, iter_begin.codePointBegin(), c);

      iter_begin = CsStringIterator<E, A>(iter_tmp);
      ++iter_begin;
   }

   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::insert(size_type indexStart, const CsBasicString &str,
                  size_type srcStart, size_type size)
{
   const_iterator iter_begin = cbegin();
   size_type i;

   for (i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (i != indexStart) {
      throw std::out_of_range("CsString::insert index out of range");
   }

   const_iterator srcIter_begin = str.begin() + srcStart;
   const_iterator srcIter_end   = srcIter_begin + size;

   for (auto srcIter = srcIter_begin; srcIter != srcIter_end; ++srcIter) {
      // *srcIter is a CsChar
      str_iter iter_tmp = E::insert(m_string, iter_begin.codePointBegin(), *srcIter);

      iter_begin = CsStringIterator<E, A>(iter_tmp);
      ++iter_begin;
   }

   return *this;
}

template <typename E, typename A>
typename CsBasicString<E, A>::iterator CsBasicString<E, A>::insert(const_iterator posStart, CsChar c)
{
   str_iter iter_tmp = E::insert(m_string, posStart.codePointBegin(), c);
   return CsStringIterator<E, A>(iter_tmp);
}

template <typename E, typename A>
typename CsBasicString<E, A>::iterator CsBasicString<E, A>::insert(const_iterator posStart, size_type count, CsChar c)
{
   str_iter iter_tmp = E::insert(m_string, posStart.codePointBegin(), c, count);
   return CsStringIterator<E, A>(iter_tmp);
}

template <typename E, typename A>
typename CsBasicString<E, A>::iterator CsBasicString<E, A>::insert(const_iterator posStart, const CsBasicString &str)
{
   const_iterator iter = posStart;
   size_type count = 0;

   for (auto c : str) {
      str_iter iter_tmp = E::insert(m_string, iter.codePointBegin(), c);

      iter = CsStringIterator<E, A>(iter_tmp);
      ++iter;

      ++count;
   }

   return (iter - count);
}

template <typename E, typename A>
template <typename T,  typename>
typename CsBasicString<E, A>::iterator CsBasicString<E, A>::insert(const_iterator posStart, const T &str,
                  size_type size)
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<E, E>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   // str is a const char *
   if (str == nullptr || *str == '\0') {
      return posStart;
   }

   // make this safe by treating str as utf8
   return insert(posStart, CsBasicString::fromUtf8(str, size));
}

template <typename E, typename A>
template <int N>
typename CsBasicString<E, A>::iterator CsBasicString<E, A>::insert(const_iterator posStart, const char (&str)[N], size_type size)
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return insert(posStart, CsBasicString::fromUtf8(str, size));
}

template <typename E, typename A>
template <typename Iterator>
typename CsBasicString<E, A>::iterator CsBasicString<E, A>::insert(const_iterator posStart,
                  Iterator begin, Iterator end)
{
   const_iterator iter = posStart;
   size_type count = 0;

   for (auto item = begin; item != end; ++item) {
      CsChar c = *item;

      str_iter iter_tmp = E::insert(m_string, iter.codePointBegin(), c);

      iter = CsStringIterator<E, A>(iter_tmp);
      ++iter;

      ++count;
   }

   return (iter - count);
}

template <typename E, typename A>
template <typename U,  typename>
typename CsBasicString<E, A>::iterator CsBasicString<E, A>::insert(size_type indexStart, CsBasicStringView<U> str)
{
   static_assert(std::is_base_of<CsBasicString<E,A>, U>::value,
      "Unable to construct a CsBasicString using a CsBasicStringView, encoding E is "
      "incompatible with the encoding for U");

   const_iterator iter_begin = cbegin();
   size_type i;

   for (i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (i != indexStart) {
      throw std::out_of_range("CsString::insert index out of range");
   }

   for (CsChar c : str) {
      str_iter iter_tmp = E::insert(m_string, iter_begin.codePointBegin(), c);

      iter_begin = CsStringIterator<E, A>(iter_tmp);
      ++iter_begin;
   }

   return iter_begin;
}

template <typename E, typename A>
template <typename U,  typename>
typename CsBasicString<E, A>::iterator CsBasicString<E, A>::insert(size_type indexStart, CsBasicStringView<U> str,
                  size_type srcStart, size_type size)
{
   static_assert(std::is_base_of<CsBasicString<E,A>, U>::value,
      "Unable to construct a CsBasicString using a CsBasicStringView, encoding E is "
      "incompatible with the encoding for U");

   const_iterator iter_begin = cbegin();
   size_type i;

   for (i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (i != indexStart) {
      throw std::out_of_range("CsString::insert index out of range");
   }

   typename U::const_iterator srcIter_begin = str.begin() + srcStart;
   typename U::const_iterator srcIter_end   = srcIter_begin + size;

   for (auto srcIter = srcIter_begin; srcIter != srcIter_end; ++srcIter) {
      // *srcIter is a CsChar
      str_iter iter_tmp = E::insert(m_string, iter_begin.codePointBegin(), *srcIter);

      iter_begin = CsStringIterator<E, A>(iter_tmp);
      ++iter_begin;
   }

   return iter_begin;
}

template <typename E, typename A>
auto CsBasicString<E, A>::length() const -> size_type
{
   return size();
}

template <typename E, typename A>
void CsBasicString<E, A>::pop_back()
{
   if (empty())  {
      return;
   }

   const_iterator iter = --end();
   erase(iter);
}

template <typename E, typename A>
void CsBasicString<E, A>::push_back(CsChar c)
{
   append(c);
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::replace(size_type indexStart, size_type size, const CsBasicString &str)
{
   const_iterator iter_begin = cbegin();
   const_iterator iter_end;
   size_type i;

   for (i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (i != indexStart) {
      throw std::out_of_range("CsString::replace index out of range");
   }

   if (size >= 0) {
      iter_end = iter_begin;

      for (size_type j = 0; j < size && iter_end != cend(); ++j)  {
         ++iter_end;
      }

   } else {
      iter_end = cend();

   }

   auto iter = erase(iter_begin, iter_end);
   insert(iter, str);

   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::replace(const_iterator first, const_iterator last, const CsBasicString &str)
{
   auto iter = erase(first, last);
   insert(iter, str);

   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::replace(size_type indexStart, size_type count, const CsBasicString &str,
                  size_type srcStart, size_type size)
{
   const_iterator iter_begin = cbegin();
   const_iterator iter_end;
   size_type i;

   for (i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (i != indexStart) {
      throw std::out_of_range("CsString::replace index out of range");
   }

   if (count >= 0) {
      iter_end = iter_begin;

      for (size_type j = 0; j < count && iter_end != cend(); ++j)  {
         ++iter_end;
      }

   } else {
      iter_end = cend();

   }

   const_iterator srcIter_begin = str.begin() + srcStart;
   const_iterator srcIter_end   = srcIter_begin + size;

   auto iter = erase(iter_begin, iter_end);
   insert(iter, srcIter_begin, srcIter_end);

   return *this;
}

template <typename E, typename A>
template <class Iterator>
CsBasicString<E, A> &CsBasicString<E, A>::replace(const_iterator first1, const_iterator last1,
                  Iterator first2, Iterator last2)
{
   auto iter = erase(first1, last1);
   insert(iter, first2, last2);

   return *this;
}

template <typename E, typename A>
template <typename T,  typename>
CsBasicString<E, A> &CsBasicString<E, A>::replace(size_type indexStart, size_type count,
                  const T &str, size_type size)
{
   // str is a const char *

   // make this safe by treating str as utf8
   return replace(indexStart, count, CsBasicString::fromUtf8(str, size));
}

template <typename E, typename A>
template <int N>
CsBasicString<E, A> &CsBasicString<E, A>::replace(size_type indexStart, size_type count, const char (&str)[N], size_type size)
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return replace(indexStart, count, CsBasicString::fromUtf8(str, size));
}

template <typename E, typename A>
template <typename T,  typename>
CsBasicString<E, A> &CsBasicString<E, A>::replace(const_iterator first, const_iterator last,
                  const T &str, size_type size)
{
   // str is a const char *

   auto iter = erase(first, last);
   insert(iter, str, size);

   return *this;
}

template <typename E, typename A>
template <int N>
CsBasicString<E, A> &CsBasicString<E, A>::replace(const_iterator first, const_iterator last,
                  const char (&str)[N], size_type size )
{
   auto iter = erase(first, last);
   insert(iter, str, size);

   return *this;
}

template <typename E, typename A>
template <typename T,  typename>
CsBasicString<E, A> &CsBasicString<E, A>::replace(size_type indexStart, size_type count, const T &str)
{
   // str is a const char *

   // make this safe by treating str as utf8
   return replace(indexStart, count, CsBasicString::fromUtf8(str));
}

template <typename E, typename A>
template <int N>
CsBasicString<E, A> &CsBasicString<E, A>::replace(size_type indexStart, size_type count, const char (&str)[N])
{
#if defined(Q_CC_MSVC)
 static_assert("¿"[0] == static_cast<char>(0xC2), "Compiler runtime encoding was not set to UTF-8");
#endif

   // make this safe by treating str as utf8
   return replace(indexStart, count, CsBasicString::fromUtf8(str, N-1));
}

template <typename E, typename A>
template <typename T,  typename>
CsBasicString<E, A> &CsBasicString<E, A>::replace(const_iterator first, const_iterator last, const T &str)
{
   // str is a const char *

   auto iter = erase(first, last);
   insert(iter, str);

   return *this;
}

template <typename E, typename A>
template <int N>
CsBasicString<E, A> &CsBasicString<E, A>::replace(const_iterator first, const_iterator last,
                  const char (&str)[N])
{
   auto iter = erase(first, last);
   insert(iter, str);

   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::replace(size_type indexStart, size_type size,
                  size_type count, CsChar c)
{
   const_iterator iter_begin = cbegin();
   const_iterator iter_end;
   size_type i;

   for (i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (i != indexStart) {
      throw std::out_of_range("CsString::replace index out of range");
   }

   if (size >= 0) {
      iter_end = iter_begin;

      for (size_type j = 0; j < size && iter_end != cend(); ++j)  {
         ++iter_end;
      }

   } else {
      iter_end = cend();

   }

   auto iter = erase(iter_begin, iter_end);
   insert(iter, count, c);

   return *this;
}

template <typename E, typename A>
CsBasicString<E, A> &CsBasicString<E, A>::replace(const_iterator first, const_iterator last,
                  size_type count, CsChar c)
{
   auto iter = erase(first, last);
   insert(iter, count, c);

   return *this;
}

template <typename E, typename A>
template <class T>
CsBasicString<E, A> &CsBasicString<E, A>::replace(size_type indexStart, size_type count, const T &str,
                  size_type srcStart, size_type size)
{
   // str is a const char *

   // make this safe by treating str as utf8
   return replace(indexStart, count, CsBasicString::fromUtf8(str + srcStart, size));
}

template <typename E, typename A>
template <class T>
CsBasicString<E, A> &CsBasicString<E, A>::replace(const_iterator first, const_iterator last, const T &str,
                  size_type srcStart, size_type size)
{
   auto iter = erase(first, last);
   insert(iter, str, srcStart, size);

   return *this;
}

template <typename E, typename A>
typename CsBasicString<E, A>::iterator CsBasicString<E, A>::replace(const_iterator iter, const CsBasicString &str)
{
   auto tmpIter = erase(iter);
   return insert(tmpIter, str);
}

template <typename E, typename A>
void CsBasicString<E, A>::resize(size_type size)
{
   if (size < 0) {
      size = 0;
   }

   size_type stringLen = this->size();
   size_type count     = size - stringLen;

   CsChar c;

   if (count > 0) {
      append(count, c);

   } else if (count < 0) {
      auto end   = this->end();
      auto begin = end + count;

      erase(begin, end);
   }
}

template <typename E, typename A>
void CsBasicString<E, A>::resize(size_type size, CsChar c)
{
   if (size < 0) {
      size = 0;
   }

   size_type stringLen = this->size();
   size_type count     = size - stringLen;

   if (count > 0) {
      append(count, c);

   } else if (count < 0) {
      erase(size, -count);

   }
}

template <typename E, typename A>
auto CsBasicString<E, A>::size_storage() const -> size_type
{
   // remove one for the null terminator
   return m_string.size() - 1;
}

template <typename E, typename A>
auto CsBasicString<E, A>::size_codePoints() const -> size_type
{
   return size();
}

template <typename E, typename A>
auto CsBasicString<E, A>::size() const -> size_type
{
   return E::distance(m_string.cbegin(), m_string.cend() - 1);
}

template <typename E, typename A>
void CsBasicString<E, A>::shrink_to_fit()
{
   m_string.shrink_to_fit();
}

template <typename E, typename A>
CsBasicString<E, A> CsBasicString<E, A>::substr(size_type indexStart, size_type size) const
{
   const_iterator iter_begin = cbegin();
   const_iterator iter_end;

   for (size_type i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (iter_begin == cend()) {
      // indexStart > length
      return CsBasicString();
   }

   if (size >= 0) {
      iter_end = iter_begin;

      for (size_type i = 0; i < size && iter_end != cend(); ++i)  {
         ++iter_end;
      }

   } else {
      iter_end = cend();

   }

   return CsBasicString(iter_begin, iter_end);
}

template <typename E, typename A>
void CsBasicString<E, A>::swap(CsBasicString &str)
{
   m_string.swap(str.m_string);
}

// iterator methods
template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::begin() const
{
   return CsStringIterator<E, A> (m_string.begin());
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::cbegin() const
{
   return CsStringIterator<E, A> (m_string.cbegin());
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::end() const
{

   return CsStringIterator<E, A> (m_string.end() - 1);
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_iterator CsBasicString<E, A>::cend() const
{

   return CsStringIterator<E, A> (m_string.cend() - 1);
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_reverse_iterator CsBasicString<E, A>::rbegin() const
{
   return const_reverse_iterator(end());
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_reverse_iterator CsBasicString<E, A>::crbegin() const
{
   return const_reverse_iterator(cend());
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_reverse_iterator CsBasicString<E, A>::rend() const
{
   return const_reverse_iterator(begin());
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_reverse_iterator CsBasicString<E, A>::crend() const
{
   return const_reverse_iterator(cbegin());
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_storage_iterator CsBasicString<E, A>::storage_begin() const
{
   return m_string.cbegin();
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_storage_iterator CsBasicString<E, A>::storage_end() const
{
   return m_string.cend() - 1;
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_storage_reverse_iterator CsBasicString<E, A>::storage_rbegin() const
{
   return m_string.crbegin() + 1;
}

template <typename E, typename A>
typename CsBasicString<E, A>::const_storage_reverse_iterator CsBasicString<E, A>::storage_rend() const
{
   return m_string.crend();
}

// functions
template <typename E_FROM, typename A_FROM, typename E_TO, typename A_TO>
void convert(const CsBasicString<E_FROM, A_FROM> &str_from, CsBasicString<E_TO, A_TO> &str_to)
{
   str_to.assign(str_from.begin(), str_from.end());
}

template <typename E, typename A>
void swap(CsBasicString<E, A> &str1, CsBasicString<E, A> &str2)
{
   str1.swap(str2);
}

template <typename E1, typename A1, typename E2, typename A2>
bool operator==(const CsBasicString<E1, A1> &str1, const CsBasicString<E2, A2> &str2)
{
   // E1 and E2 are different

   auto iter1 = str1.begin();
   auto iter2 = str2.begin();

   auto end1 = str1.end();
   auto end2 = str2.end();

   while (iter1 != end1 && iter2 != end2) {

      if (*iter1 != *iter2) {
        return false;
      }

      ++iter1;
      ++iter2;
   }

   if (iter1 == end1 && iter2 == end2) {
      return true;
   }

   return false;
}

template <typename E, typename A>
bool operator==(const CsBasicString<E, A> &str1, const CsBasicString<E, A> &str2)
{
   // E is the same

   // are the vectors equal
   return std::equal(str1.storage_begin(), str1.storage_end(), str2.storage_begin(), str2.storage_end());
}

inline bool operator==(const CsString_utf8 &str1, const CsString_utf8 &str2)
{
   // are the vectors equal
   return std::equal(str1.storage_begin(), str1.storage_end(), str2.storage_begin(), str2.storage_end());
}

inline bool operator==(const CsString_utf16 &str1, const CsString_utf16 &str2)
{
   // are the vectors equal
   return std::equal(str1.storage_begin(), str1.storage_end(), str2.storage_begin(), str2.storage_end());
}

template <int N>
inline bool operator==(const CsString_utf8 &str1, const char (& str2)[N])
{
   return std::equal(str1.storage_begin(), str1.storage_end(), str2, str2+N-1,
      [] (auto a, auto b) { return static_cast<uint8_t>(a) == static_cast<uint8_t>(b);} );
}

template <int N>
inline bool operator==(const char (& str1)[N], const CsString_utf8 &str2)
{
   return std::equal(str1, str1+N-1, str2.storage_begin(), str2.storage_end(),
      [] (auto a, auto b) { return static_cast<uint8_t>(a) == static_cast<uint8_t>(b);} );
}

template <int N>
inline bool operator==(const CsString_utf16 &str1, const char16_t (& str2)[N])
{
   return std::equal(str1.storage_begin(), str1.storage_end(), str2, str2+N-1);
}

template <int N>
inline bool operator==(const char16_t (& str1)[N], const CsString_utf16 &str2)
{
   return std::equal(str1, str1+N-1, str2.storage_begin(), str2.storage_end());
}

template <typename E1, typename A1, typename E2, typename A2>
bool operator!=(const CsBasicString<E1, A1> &str1, const CsBasicString<E2, A2> &str2)
{
   return ! (str1 == str2);
}

template <typename E, typename A>
bool operator!=(const CsBasicString<E, A> &str1, const CsBasicString<E, A> &str2)
{
   return ! (str1 == str2);
}

inline bool operator!=(const CsString_utf8 &str1, const CsString_utf8 &str2)
{
   return ! (str1 == str2);
}

inline bool operator!=(const CsString_utf16 &str1, const CsString_utf16 &str2)
{
   return ! (str1 == str2);
}

inline CsString_utf8 operator+(CsString_utf8 str1, const CsString_utf8 &str2)
{
   str1.append(str2);

   return str1;
}

inline CsString_utf16 operator+(CsString_utf16 str1, const CsString_utf16 &str2)
{
   str1.append(str2);

   return str1;
}

template <typename E, typename A>
CsBasicString<E, A> operator+(const CsBasicString<E, A> &str, CsChar c)
{
   CsBasicString<E, A> retval = str;
   retval.append(c);

   return retval;
}

template <typename E, typename A>
CsBasicString<E, A> operator+(CsChar c, const CsBasicString<E, A> &str)
{
   CsBasicString<E, A> retval = str;
   retval.insert(0, 1, c);

   return retval;
}

template <typename E, typename A>
CsBasicString<E, A> operator+(CsBasicString<E, A> &&str, CsChar c)
{
   str.append(c);
   return str;
}

template <typename E, typename A>
CsBasicString<E, A> operator+(CsChar c, CsBasicString<E, A> &&str)
{
   str.insert(0, 1, c);
   return str;
}

template <typename E, typename A, typename T, typename = typename std::enable_if<std::is_array<T>::value &&
                  std::is_same<char, typename std::remove_extent<T>::type>::value>::type>
CsBasicString<E, A> operator+(const CsBasicString<E, A> &str1, const T &str2)
{
   CsBasicString<E, A> retval = str1;
   retval.append(str2);

   return retval;
}

template <typename E, typename A, typename T, typename = typename std::enable_if<std::is_array<T>::value &&
                  std::is_same<char, typename std::remove_extent<T>::type>::value>::type>
CsBasicString<E, A> operator+(const T &str1, const CsBasicString<E, A> &str2)
{
   CsBasicString<E, A> retval = str1;
   retval.append(str2);

   return retval;
}

template <typename E1, typename A1, typename E2, typename A2>
bool operator<(const CsBasicString<E1, A1> &str1, const CsBasicString<E2, A2> &str2)
{
   return std::lexicographical_compare(str1.begin(), str1.end(), str2.begin(), str2.end());
}

template <typename E1, typename A1, typename E2, typename A2>
bool operator<=(const CsBasicString<E1, A1> &str1, const CsBasicString<E2, A2> &str2)
{
   return ! (str1 > str2);
}

template <typename E1, typename A1, typename E2, typename A2>
bool operator>(const CsBasicString<E1, A1> &str1, const CsBasicString<E2, A2> &str2)
{
   return std::lexicographical_compare(str2.begin(), str2.end(), str1.begin(), str1.end());
}

template <typename E1, typename A1, typename E2, typename A2>
bool operator>=(const CsBasicString<E1, A1> &str1, const CsBasicString<E2, A2> &str2)
{
   return ! (str1 < str2);
}

#if defined(__cpp_char8_t)
   // support new data type added in C++20

   template <typename E, typename A>
   CsBasicString<E, A>::CsBasicString(const char8_t *str, const A &a)
   {
      *this = CsBasicString::fromUtf8(str, -1, a);
   }

   template <typename E, typename A>
   CsBasicString<E, A>::CsBasicString(const char8_t *str, size_type size, const A &a)
   {
      *this = CsBasicString::fromUtf8(str, size, a);
   }
#endif


}  // namespace

#endif
