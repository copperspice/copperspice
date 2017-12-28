/***********************************************************************
*
* Copyright (c) 2017-2017 Barbara Geller
* Copyright (c) 2017-2017 Ansel Sermersheim
* All rights reserved.
*
* This file is part of CsString
*
* CsString is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
***********************************************************************/

#ifndef LIB_CS_STRING_VIEW_H
#define LIB_CS_STRING_VIEW_H

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
      using size_type      = std::ptrdiff_t;

      using const_iterator = typename S::const_iterator;
      using iterator       = typename S::const_iterator;

      using const_reverse_iterator = typename S::const_reverse_iterator;
      using reverse_iterator       = typename S::const_reverse_iterator;

      static constexpr const size_type npos = -1;

      CsBasicStringView() = default;

      CsBasicStringView(const S &str)
         : m_begin(str.begin()), m_end(str.end())
      { }

      // initialize with a range from another string type container
      CsBasicStringView(const_iterator begin, const_iterator end)
         : m_begin(begin), m_end(end)
      { }

      // methods
      CsChar at(size_type index) const;
      CsChar back() const;
      bool empty() const;

      // ** uses an iterator, returns an iterator
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

      const_iterator find_fast(CsChar c) const;
      const_iterator find_fast(CsChar c, const_iterator iter_begin) const;

      const_iterator rfind_fast(CsBasicStringView str) const;
      const_iterator rfind_fast(CsBasicStringView str, const_iterator iter_end) const;

      CsChar front() const;

      size_type size() const;
      size_type length()  const;

      CsBasicStringView substr(size_type indexStart = 0, size_type size = npos) const;

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

template <typename S>
CsChar CsBasicStringView<S>::at(size_type index) const
{
   const_iterator iter = begin();
   std::advance(iter, index);

   return *iter;
}

template <typename S>
CsChar CsBasicStringView<S>::back() const
{
   return *(--end());
}

template <typename S>
bool CsBasicStringView<S>::empty() const
{
   return (m_begin == m_end);
}

template <typename S>
CsChar CsBasicStringView<S>::front() const
{
   return *begin();
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
auto CsBasicStringView<S>::length() const -> size_type
{
   return size();
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

}

#endif
