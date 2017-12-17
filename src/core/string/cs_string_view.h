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

      // initialize with a range from another string type container
      CsBasicStringView(const_iterator begin, const_iterator end)
         : m_begin(begin), m_end(end)
      { }


      // methods


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

/*
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
*/

   private:
      const_iterator m_begin;
      const_iterator m_end;
};


}

#endif
