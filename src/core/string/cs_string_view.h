/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* This file is part of CsString.
*
* CsString is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

#ifndef LIB_CS_STRING_VIEW_H
#define LIB_CS_STRING_VIEW_H

#include <cs_string.h>

namespace CsString {

template <typename S>
class CsBasicStringView;

using CsStringView       = CsBasicStringView<CsBasicString<utf8>>;
using CsStringView_utf8  = CsBasicStringView<CsBasicString<utf8>>;
using CsStringView_utf16 = CsBasicStringView<CsBasicString<utf16>>;

template <typename S>
class CsBasicStringView
{
   public:
      using difference_type        = typename S::difference_type;
      using size_type              = typename S::difference_type;
      using value_type             = typename S::value_type;

      using const_iterator         = typename S::const_iterator;
      using iterator               = typename S::const_iterator;
      using const_reverse_iterator = typename S::const_reverse_iterator;
      using reverse_iterator       = typename S::const_reverse_iterator;

      static constexpr const size_type npos = -1;

      CsBasicStringView() = default;

      CsBasicStringView(const CsBasicStringView &str) = default;
      CsBasicStringView(CsBasicStringView &&str) = default;

      CsBasicStringView(const S &str)
         : m_begin(str.begin()), m_end(str.end())
      { }

      // initialize with a range from another string type container
      CsBasicStringView(const_iterator begin, const_iterator end)
         : m_begin(begin), m_end(end)
      { }

      // operators
      CsBasicStringView<S> &operator=(const CsBasicStringView &str) = default;
      CsBasicStringView<S> &operator=(CsBasicStringView &&str) = default;

      value_type operator[](size_type index) const;

      // methods
      value_type at(size_type index) const;
      value_type back() const;

      int compare(CsBasicStringView str) const;

      bool empty() const;
      bool endsWith(CsBasicStringView str) const;

      // ** uses an iterator, returns an iterator
      const_iterator find_fast(value_type c) const;
      const_iterator find_fast(value_type c, const_iterator iter_begin) const;

      const_iterator find_fast(CsBasicStringView str) const;
      const_iterator find_fast(CsBasicStringView str, const_iterator iter_begin) const;

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
      template <typename T, typename  = typename std::enable_if<std::is_same<T, const char *>::value ||
                  std::is_same<T, char *>::value>::type>
      const_iterator find_fast(const T &str, const_iterator iter_begin) const;

      // for an array of chars
      template <int N>
      const_iterator find_fast(const char (&str)[N]) const;

      // for an array of chars
      template <int N>
      const_iterator find_fast(const char (&str)[N], const_iterator iter_begin) const;

      //
      const_iterator rfind_fast(value_type c) const;
      const_iterator rfind_fast(value_type c, const_iterator iter_end) const;

      const_iterator rfind_fast(CsBasicStringView str) const;
      const_iterator rfind_fast(CsBasicStringView str, const_iterator iter_end) const;

      value_type front() const;
      size_type length()  const;

      size_type size() const;
      bool startsWith(CsBasicStringView str) const;

      CsBasicStringView<S> remove_prefix(size_type size) const;
      CsBasicStringView<S> remove_suffix(size_type size) const;

      CsBasicStringView<S> substr(size_type indexStart = 0, size_type size = npos) const;
      void swap(CsBasicStringView &str);

      // iterators
      iterator begin() {
         return m_begin;
      }

      const_iterator begin() const {
         return m_begin;
      }

      const_iterator cbegin() const {
         return m_begin;
      }

      const_iterator constBegin() const {
         return m_begin;
      }

      iterator end() {
         return m_end;
      }

      const_iterator end() const {
         return m_end;
      }

      const_iterator cend() const {
         return m_end;
      }

      const_iterator constEnd() const {
         return m_end;
      }

      reverse_iterator rbegin()  {
         return m_end;
      }

      const_reverse_iterator rbegin() const {
         return m_end;
      }

      reverse_iterator rend()  {
         return m_begin;
      }

      const_reverse_iterator rend() const {
         return m_begin;
      }

      const_reverse_iterator crbegin() const {
         return m_end;
      }

      const_reverse_iterator crend() const {
         return m_begin;
      }

   private:
      const_iterator m_begin;
      const_iterator m_end;
};

// free functions
template <typename S>
inline bool operator==(CsBasicStringView<S> &str1, CsBasicStringView<S> &str2)
{
   return str1.compare(str2) == 0;
}

template <typename S>
inline bool operator!=(CsBasicStringView<S> &str1, CsBasicStringView<S> &str2)
{
   return str1.compare(str2) != 0;
}

template <typename S>
inline bool operator<(CsBasicStringView<S> &str1, CsBasicStringView<S> &str2)
{
   return str1.compare(str2) < 0;
}

template <typename S>
inline bool operator>(CsBasicStringView<S> &str1, CsBasicStringView<S> &str2)
{
   return str1.compare(str2) > 0;
}

template <typename S>
inline bool operator<=(CsBasicStringView<S> &str1, CsBasicStringView<S> &str2)
{
   return str1.compare(str2) <= 0;
}

template <typename S>
inline bool operator>=(CsBasicStringView<S> &str1, CsBasicStringView<S> &str2)
{
   return str1.compare(str2) >= 0;
}

// operators
template <typename S>
typename CsBasicStringView<S>::value_type CsBasicStringView<S>::operator[](size_type index) const
{
   const_iterator iter = begin();
   std::advance(iter, index);

   return *iter;
}

// methods
template <typename S>
typename CsBasicStringView<S>::value_type CsBasicStringView<S>::at(size_type index) const
{
   const_iterator iter = begin();
   std::advance(iter, index);

   return *iter;
}

template <typename S>
typename CsBasicStringView<S>::value_type CsBasicStringView<S>::back() const
{
   return *(--end());
}

template <typename S>
int CsBasicStringView<S>::compare(CsBasicStringView str) const
{
   auto iter_a = cbegin();
   auto iter_b = str.cbegin();

   while (iter_a != cend() && iter_b != str.cend()) {

      auto value_a = *iter_a;
      auto value_b = *iter_b;

      if (value_a < value_b) {
         return -1;

      } else if (value_a > value_b) {
         return 1;

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

template <typename S>
bool CsBasicStringView<S>::empty() const
{
   return (m_begin == m_end);
}

template <typename S>
bool CsBasicStringView<S>::endsWith(CsBasicStringView<S> str) const
{
   if (str.empty() ){
      return true;

   } else if (empty()) {
      return false;
   }

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
}

template <typename S>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::find_fast(CsBasicStringView str) const
{
   return find_fast(str, begin());
}

template <typename S>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::find_fast(CsBasicStringView str,
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

   while (iter != iter_end) {

      if (*iter == str[0])  {
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
template <typename S>
template <typename T,  typename>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::find_fast(const T &str, const_iterator iter_begin,
                  size_type size) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<T, T>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   const_iterator iter_end = end();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   if (str == nullptr || str[0] == 0) {
      return iter_begin;
   }

   auto iter = iter_begin;

   while (iter != iter_end)   {

      if (*iter == str[0])  {
         auto text_iter    = iter + 1;
         auto pattern_iter = str  + 1;

         int count = 0;

         while (text_iter != iter_end && *pattern_iter != 0 && count < size)  {

            if (*text_iter == *pattern_iter)  {
               ++text_iter;
               ++pattern_iter;

               ++count;

            } else {
               break;

            }
         }

         if (*pattern_iter == 0) {
            // found a match
            return iter;
         }
      }

      ++iter;
   }

   return iter_end;
}

// for an array of chars
template <typename S>
template <int N>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::find_fast(const char (&str)[N], const_iterator iter_begin,
                  size_type size) const
{
   // make this safe

   const_iterator iter_end = end();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   if (str == nullptr || str[0] == 0) {
      return iter_begin;
   }

   auto iter = iter_begin;

   while (iter != iter_end)   {

      if (*iter == str[0])  {
         auto text_iter    = iter + 1;
         auto pattern_iter = str  + 1;

         int count = 0;

         while (text_iter != iter_end && *pattern_iter != 0 && count < size)  {

            if (*text_iter == *pattern_iter)  {
               ++text_iter;
               ++pattern_iter;

               ++count;

            } else {
               break;

            }
         }

         if (*pattern_iter == 0) {
            // found a match
            return iter;
         }
      }

      ++iter;
   }

   return iter_end;
}

// for a const char * and char *
template <typename S>
template <typename T,  typename>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::find_fast(const T &str) const
{
   return find_fast(str, begin());
}

// for a const char * and char *
template <typename S>
template <typename T,  typename>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::find_fast(const T &str, const_iterator iter_begin) const
{
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<T, T>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif

   const_iterator iter_end = end();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   if (str == nullptr || str[0] == 0) {
      return iter_begin;
   }

   auto iter = iter_begin;

   while (iter != iter_end)   {

      if (*iter == str[0])  {
         auto text_iter    = iter + 1;
         auto pattern_iter = str  + 1;

         while (text_iter != iter_end && *pattern_iter != 0)  {

            if (*text_iter == *pattern_iter)  {
               ++text_iter;
               ++pattern_iter;

            } else {
               break;

            }
         }

         if (*pattern_iter == 0) {
            // found a match
            return iter;
         }
      }

      ++iter;
   }

   return iter_end;
}

// for an array of chars
template <typename S>
template <int N>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::find_fast(const char (&str)[N]) const
{
   return find_fast(str, begin());
}

// for an array of chars
template <typename S>
template <int N>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::find_fast(const char (&str)[N],
                  const_iterator iter_begin) const
{
   // make this safe (find which encoding the compiler is using, convert str from X to E)

   const_iterator iter_end = end();

   if (iter_begin == iter_end) {
      return iter_end;
   }

   if (N == 0) {
      return iter_begin;
   }

   auto iter = iter_begin;

   while (iter != iter_end)   {

      if (*iter == str[0])  {
         auto text_iter    = iter + 1;
         auto pattern_iter = str  + 1;

         while (text_iter != iter_end && pattern_iter != str + N)  {

            if (*text_iter == *pattern_iter)  {
               ++text_iter;
               ++pattern_iter;

            } else {
               break;

            }
         }

         if (pattern_iter == str + N) {
            // found a match
            return iter;
         }
      }

      ++iter;
   }

   return iter_end;
}

template <typename S>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::find_fast(value_type c) const
{
   return find_fast(c, begin());
}

template <typename S>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::find_fast(value_type c, const_iterator iter_begin) const
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

template <typename S>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::rfind_fast(value_type c) const
{
   return rfind_fast(c, end());
}

template <typename S>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::rfind_fast(value_type c, const_iterator iter_end) const
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

template <typename S>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::rfind_fast(CsBasicStringView str) const
{
   return rfind_fast(str, end());
}

template <typename S>
typename CsBasicStringView<S>::const_iterator CsBasicStringView<S>::rfind_fast(CsBasicStringView str, const_iterator iter_end) const
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

   while (iter != begin())   {
      --iter;

      if (*iter == str[0])  {

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

template <typename S>
typename CsBasicStringView<S>::value_type CsBasicStringView<S>::front() const
{
   return *begin();
}

template <typename S>
auto CsBasicStringView<S>::length() const -> size_type
{
   return size();
}

template <typename S>
bool CsBasicStringView<S>::startsWith(CsBasicStringView<S> str) const
{
   if (str.empty()) {
      return true;

   } else if (empty()) {
      return false;

   }

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
}

template <typename S>
auto CsBasicStringView<S>::size() const -> size_type
{
   size_type retval = 0;

   for (auto item = begin(); item != end(); ++item) {
      ++retval;
   }

   return retval;
}

template <typename S>
CsBasicStringView<S> CsBasicStringView<S>::remove_prefix(size_type size) const
{
   const_iterator iter_begin = cbegin();

   for (size_type i = 0; i < size && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (iter_begin == cend()) {
      // index > size()
      return CsBasicStringView();
   }

   return CsBasicStringView(iter_begin, cend());
}

template <typename S>
CsBasicStringView<S> CsBasicStringView<S>::remove_suffix(size_type size) const
{
   const_iterator iter_end = cend();

   for (size_type i = 0; i < size && iter_end != cbegin(); ++i)  {
      --iter_end;
   }

   return CsBasicStringView(cbegin(), iter_end);
}

template <typename S>
CsBasicStringView<S> CsBasicStringView<S>::substr(size_type indexStart, size_type size) const
{
   const_iterator iter_begin = cbegin();
   const_iterator iter_end;

   for (size_type i = 0; i < indexStart && iter_begin != cend(); ++i)  {
      ++iter_begin;
   }

   if (iter_begin == cend()) {
      // index > size()
      return CsBasicStringView();
   }

   if (size >= 0) {
      iter_end = iter_begin;

      for (size_type i = 0; i < size && iter_end != cend(); ++i)  {
         ++iter_end;
      }

   } else {
      iter_end = cend();

   }

   return CsBasicStringView(iter_begin, iter_end);
}

template <typename S>
void CsBasicStringView<S>::swap(CsBasicStringView &str)
{
   swap(m_begin, str.m_begin);
   swap(m_end, str.m_end);
}


}

#endif
