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

#ifndef LIB_CS_STRING_ITERATOR_H
#define LIB_CS_STRING_ITERATOR_H

#include <cstddef>
#include <vector>

#include <cs_char.h>

namespace CsString {

template <typename E, typename A>
class LIB_CS_STRING_EXPORT CsStringIterator
{
   using v_iter = typename std::vector<typename E::storage_unit, A>::const_iterator;

   public:
      using size_type         = std::ptrdiff_t;
      using difference_type   = int;
      using value_type        = CsChar;
      using pointer           = CsChar *;
      using reference         = CsChar;
      using iterator_category = std::random_access_iterator_tag;

      CsStringIterator() = default;

      CsChar operator*() const;
      CsChar operator->() const;

      CsChar operator[](size_type x) const;

      // comparisons
      bool operator!=(const CsStringIterator &other) const;
      bool operator==(const CsStringIterator &other) const;
      bool operator<(const CsStringIterator &other) const;
      bool operator<=(const CsStringIterator &other) const;
      bool operator>(const CsStringIterator &other) const;
      bool operator>=(const CsStringIterator &other) const;

      // math
      CsStringIterator &operator+=(size_type x);
      CsStringIterator &operator-=(size_type x);

      CsStringIterator operator+(size_type x) const;
      CsStringIterator &operator++();
      CsStringIterator operator++(int);

      CsStringIterator operator-(size_type x) const;
      size_type operator-(CsStringIterator other) const;
      CsStringIterator &operator--();
      CsStringIterator operator--(int);

   private:
      explicit CsStringIterator(v_iter data);

      typename std::pair<v_iter, v_iter> codePointRange() const;
      v_iter codePointBegin() const;
      v_iter codePointEnd() const;

      v_iter m_iter;

      friend class CsBasicString<E, A>;
};

template <typename E, typename A>
CsChar CsStringIterator<E,A>::operator*() const
{
   return E::getCodePoint(m_iter);
}

template <typename E, typename A>
CsChar CsStringIterator<E,A>::operator->() const
{
    return E::getCodePoint(m_iter);
}

template <typename E, typename A>
CsChar CsStringIterator<E,A>:: operator[](size_type x) const
{
   // calls operator+()
   return *(*this + x);
}

// comparisons
template <typename E, typename A>
bool CsStringIterator <E,A>::operator!=(const CsStringIterator &other) const
{
   return m_iter != other.m_iter;
}

template <typename E, typename A>
bool CsStringIterator <E,A>::operator==(const CsStringIterator &other) const
{
   return m_iter == other.m_iter;
}

template <typename E, typename A>
bool CsStringIterator <E,A>::operator<(const CsStringIterator &other) const
{
   return m_iter < other.m_iter;
}

template <typename E, typename A>
bool CsStringIterator <E,A>::operator<=(const CsStringIterator &other) const
{
   return m_iter <= other.m_iter;
}

template <typename E, typename A>
bool CsStringIterator <E,A>::operator>(const CsStringIterator &other) const
{
   return m_iter > other.m_iter;
}

template <typename E, typename A>
bool CsStringIterator <E,A>::operator>=(const CsStringIterator &other) const
{
   return m_iter >= other.m_iter;
}

// math
template <typename E, typename A>
CsStringIterator<E,A> &CsStringIterator<E,A>::operator+=(size_type x)
{
   m_iter += E::walk(x, m_iter);

   return *this;
}
template <typename E, typename A>
CsStringIterator<E,A> &CsStringIterator<E,A>::operator-=(size_type x)
{
   m_iter += E::walk(-x, m_iter);
   return *this;
}

template <typename E, typename A>
CsStringIterator<E,A> CsStringIterator<E,A>::operator+(size_type x) const
{
   auto iter = m_iter + E::walk(x, m_iter);
   return CsStringIterator(iter);
}

template <typename E, typename A>
CsStringIterator<E,A> &CsStringIterator<E,A>::operator++()
{
   m_iter += E::walk(1, m_iter);
   return *this;
}

template <typename E, typename A>
CsStringIterator<E,A> CsStringIterator<E,A>::operator++(int)
{
   CsStringIterator retval = *this;
   m_iter += E::walk(1, m_iter);

   return retval;
}

template <typename E, typename A>
CsStringIterator<E,A> CsStringIterator<E,A>::operator-(size_type x) const
{
   auto retval = m_iter + E::walk(-x, m_iter);
   return CsStringIterator(retval);
}

template <typename E, typename A>
typename CsStringIterator<E, A>::size_type CsStringIterator <E,A>::operator-(CsStringIterator other) const
{
   int retval = 0;

   CsStringIterator a = *this;
   CsStringIterator b = other;

   if (a < b) {

      while (a != b) {
         ++a;
         --retval;
      }

   } else {
      while (a != b) {
         ++b;
         ++retval;
      }
   }

   return retval;
}

template <typename E, typename A>
CsStringIterator<E,A> &CsStringIterator<E,A>::operator--()
{
   m_iter+= E::walk(-1, m_iter);
   return *this;
}

template <typename E, typename A>
CsStringIterator<E,A> CsStringIterator<E,A>::operator--(int)
{
   CsStringIterator retval = *this;
   m_iter += E::walk(-1, m_iter);

   return retval;
}

// private methods
template <typename E, typename A>
CsStringIterator<E,A>::CsStringIterator(v_iter data)
{
   m_iter = data;
}

template <typename E, typename A>
auto CsStringIterator<E,A>::codePointRange() const -> typename std::pair<v_iter, v_iter>
{
   return std::make_pair(m_iter, m_iter + E::walk(1, m_iter));
}

template <typename E, typename A>
auto CsStringIterator<E,A>::codePointBegin() const -> v_iter
{
   return m_iter;
}

template <typename E, typename A>
auto CsStringIterator<E,A>::codePointEnd() const -> v_iter
{
   return m_iter + E::walk(1, m_iter);
}

}

#endif
