/***********************************************************************
*
* Copyright (c) 2017-2018 Barbara Geller
* Copyright (c) 2017-2018 Ansel Sermersheim
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

class LIB_CS_STRING_EXPORT CsCharArrow
{
   public:
      CsCharArrow (CsChar c)
         : m_data(c)
      { }

      const CsChar *operator->() const {
         return &m_data;
      }

   private:
      CsChar m_data;
};

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
      CsCharArrow operator->() const;

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

      typename std::pair<v_iter, v_iter> codePointRange() const;
      v_iter codePointBegin() const;
      v_iter codePointEnd() const;

   private:
      explicit CsStringIterator(v_iter data);
      v_iter m_iter;

      friend class CsBasicString<E, A>;
};

template <typename E, typename A>
CsChar CsStringIterator<E,A>::operator*() const
{
   return E::getCodePoint(m_iter);
}

template <typename E, typename A>
CsCharArrow CsStringIterator<E,A>::operator->() const
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
   size_type retval = 0;

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

// reverse iterator defreference needs to return by value
template <typename T>
class LIB_CS_STRING_EXPORT CsStringReverseIterator : public std::reverse_iterator<T>
{
   public:
      CsStringReverseIterator() = default;

      CsStringReverseIterator(T iter)
         : std::reverse_iterator<T>(iter)
      {
      }

      template <typename U>
      CsStringReverseIterator(CsStringReverseIterator<U> iter)
         : std::reverse_iterator<T>(iter.base())
      {
      }

      decltype(std::declval<T>().operator*())  operator*() const;
      decltype(std::declval<T>().operator->()) operator->() const;
};

template <typename T>
decltype(std::declval<T>().operator*()) CsStringReverseIterator<T>::operator*() const
{
   auto tmp = this->base();
   return (--tmp).operator*();
}

template <typename T>
decltype(std::declval<T>().operator->()) CsStringReverseIterator<T>::operator->() const
{
   auto tmp = this->base();
   return (--tmp).operator->();
}

}

#endif
