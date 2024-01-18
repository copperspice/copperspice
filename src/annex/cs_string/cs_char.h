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

#ifndef LIB_CS_CHAR_H
#define LIB_CS_CHAR_H

#include <stdint.h>

#include <functional>
#include <memory>

namespace CsString {

#ifdef _WIN32

#ifdef BUILDING_LIB_CS_STRING
# define LIB_CS_STRING_EXPORT     __declspec(dllexport)
#else
# define LIB_CS_STRING_EXPORT     __declspec(dllimport)
#endif

#else
# define LIB_CS_STRING_EXPORT

#endif

template <typename E, typename A = std::allocator<typename E::storage_unit>>
class CsBasicString;

class CsChar
{
   public:
      CsChar()
         : m_char(0)
      {
      }

      template <typename T = int>
      CsChar(char c)
         : m_char(static_cast<unsigned char>(c))
      {
#ifndef CS_STRING_ALLOW_UNSAFE
   static_assert(! std::is_same<T, T>::value, "Unsafe operations not allowed, unknown encoding for this operation");
#endif
      }

      CsChar(char32_t c)
         : m_char(c)
      {
      }

      CsChar(int c)
         : m_char(c)
      {
      }

      CsChar(const CsChar &other)
         : m_char(other.m_char)
      {
      }

#ifdef __cpp_char8_t
      // support new data type added in C++20

      CsChar(char8_t c)
         : m_char(c)
      {
      }
#endif

      inline bool operator!=(const CsChar &other) const;
      inline bool operator==(const CsChar &other) const;

      inline bool operator<(const CsChar &other) const;
      inline bool operator<=(const CsChar &other) const;
      inline bool operator>(const CsChar &other) const;
      inline bool operator>=(const CsChar &other) const;

      inline CsChar &operator=(char c) &;
      inline CsChar &operator=(char32_t c) &;
      inline CsChar &operator=(CsChar c) &;

      inline uint32_t unicode() const;

   private:
      uint32_t m_char;
};

// comparisons
inline bool CsChar::operator!=(const CsChar &other) const
{
   return m_char != other.m_char;
}

inline bool CsChar::operator==(const CsChar &other) const
{
   return m_char == other.m_char;
}

inline bool CsChar::operator<(const CsChar &other) const
{
   return m_char < other.m_char;
}

inline bool CsChar::operator<=(const CsChar &other) const
{
   return m_char <= other.m_char;
}

inline bool CsChar::operator>(const CsChar &other) const
{
   return m_char > other.m_char;
}

inline bool CsChar::operator>=(const CsChar &other) const
{
   return m_char >= other.m_char;
}

inline CsChar &CsChar::operator=(char c) &
{
   m_char = c;
   return *this;
}

inline CsChar &CsChar::operator=(char32_t c) &
{
   m_char = c;
   return *this;
}

inline CsChar &CsChar::operator=(CsChar c) &
{
   m_char = c.m_char;
   return *this;
}

inline uint32_t CsChar::unicode() const
{
   return m_char;
}

} // namespace

namespace std {
   template<>
   struct hash<CsString::CsChar>
   {
      inline size_t operator()(const CsString::CsChar &key) const
      {
         return key.unicode();
      }
   };
}

#endif