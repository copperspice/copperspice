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

#ifndef LIB_CS_ENCODING_H
#define LIB_CS_ENCODING_H

#include <stdint.h>
#include <vector>

#include <cs_char.h>

namespace CsString {

class LIB_CS_STRING_EXPORT utf8
{
   public:
      using size_type    = std::ptrdiff_t;
      using storage_unit = uint8_t;

      template <typename Iterator>
      static Iterator advance(Iterator iter_begin, Iterator iter_end, size_type count)
      {
         auto iter = iter_begin;
         storage_unit value = 0;

         while (iter != iter_end && count != 0)  {

            value = *iter;
            if (value < 0x80 || value > 0xBf) {
               --count;
            }

            ++iter;
         }

         if (value >= 0xBF) {
            while (iter != iter_end && *iter >= 0x80 && *iter <= 0xBF) {
               ++iter;
            }
         }

         return iter;
      }

      template <typename Iterator>
      static size_type distance(Iterator iter_begin, Iterator iter_end)
      {
         size_type retval = 0;

         for (auto iter = iter_begin; iter != iter_end; ++iter) {
            storage_unit value = *iter;

            if (value < 0x80 || value > 0xBF) {
               // ascii or first byte of a multi byte sequence
               ++retval;
            }
         }

         return retval;
      }

      template <typename Container>
      static typename Container::const_iterator insert(Container &str1,
                  typename Container::const_iterator iter, CsChar c, size_type count = 1)
      {
         uint32_t value = c.unicode();

         for (size_type x = 0; x < count; ++x)  {
            if (value <= 0x007F)  {
               iter = str1.insert(iter, value);

            } else if (value <= 0x07FF) {
               iter = str1.insert(iter, ((value)      & 0x3F) | 0x80);
               iter = str1.insert(iter, ((value >> 6) & 0x1F) | 0xC0);

            } else if (value <= 0xFFFF) {
               iter = str1.insert(iter, ((value      ) & 0x3F) | 0x80);
               iter = str1.insert(iter, ((value >> 6 ) & 0x3F) | 0x80);
               iter = str1.insert(iter, ((value >> 12) & 0x0F) | 0xE0);

            } else  {
               iter = str1.insert(iter, ((value      ) & 0x3F) | 0x80);
               iter = str1.insert(iter, ((value >> 6 ) & 0x3F) | 0x80);
               iter = str1.insert(iter, ((value >> 12) & 0x3F) | 0x80);
               iter = str1.insert(iter, ((value >> 18) & 0x07) | 0xF0);

            }
         }

         return iter;
      }

      static size_type walk(size_type len, std::vector<storage_unit>::const_iterator iter)
      {
         size_type retval = 0;
         size_type count  = 0;

         if (len >= 0) {
            // walk forward

            for (size_type x = 0; x < len; ++x) {
               uint8_t value = *iter;

               count = numOfBytes(value);
               iter += count;

               retval += count;
            }

         } else  {
            // walk backwards

            for (size_type x = 0; x > len; --x) {

               while (true) {
                  --iter;
                  --retval;

                  uint8_t value = *iter;

                  if ((value & 0xC0) != 0x80)  {
                     // at the beginning of a char
                     break;
                  }
               }
            }
         }

         return retval;
      }

      static CsChar getCodePoint(std::vector<storage_unit>::const_iterator iter)
      {
         char32_t value = 0;
         uint8_t  tmp   = *iter;

         if ((tmp & 0x80) == 0)  {
            value = tmp;

         } else if ((tmp & 0xE0) == 0xC0) {
            value = (tmp & 0x1F) << 6;

            tmp    = iter[1];
            value |= (tmp & 0x3F);


         } else if ((tmp & 0xF0) == 0xE0) {
            value = (tmp & 0x0F) << 12;

            tmp    =  iter[1];
            value |= (tmp & 0x3F) << 6;

            tmp    =  iter[2];
            value |= (tmp & 0x3F);

         } else {
            value = (tmp & 0x07) << 18;

            tmp    =  iter[1];
            value |= (tmp & 0x3F) << 12;

            tmp    =  iter[2];
            value |= (tmp & 0x3F) << 6;

            tmp    =  iter[3];
            value |= (tmp & 0x3F);

         }

         return CsChar(value);
      }

   private:
      static size_type numOfBytes(uint8_t value)
      {
         if ((value & 0x80) == 0) {
            return 1;

         } else if ((value & 0xE0) == 0xC0) {
            return 2;

         } else if ((value & 0xF0) == 0xE0) {
            return 3;

         } else if ((value & 0xF8) == 0xF0) {
            return 4;

         }

         return 1;
      }
};

class LIB_CS_STRING_EXPORT utf16
{
   public:
      using size_type    = std::ptrdiff_t;
      using storage_unit = uint16_t;

      template <typename Iterator>
      static Iterator advance(Iterator iter_begin, Iterator iter_end, size_type count)
      {
         auto iter = iter_begin;
         storage_unit value = 0;

         while (iter != iter_end && count != 0)  {

            value = *iter;
            if (value < 0xDC00 || value > 0xDFFF) {
               // not a low surrogate
               --count;
            }

            ++iter;
         }

         if (value >= 0xD800 && value <= 0xDBFF) {
            ++iter;
         }

         return iter;
      }

      template <typename Iterator>
      static size_type distance(Iterator iter_begin, Iterator iter_end)
      {
         size_type retval = 0;

         for (auto iter = iter_begin; iter != iter_end; ++iter) {
            storage_unit value = *iter;

            if (value < 0xDC00 || value > 0xDFFF) {
               // not a low surrogate
               ++retval;
            }
         }

         return retval;
      }

      template <typename Container>
      static typename Container::const_iterator insert(Container &str1,
                  typename Container::const_iterator iter, CsChar c, size_type count = 1)
      {
         uint32_t value = c.unicode();

         for (size_type x = 0; x < count; ++x)  {

            if ((value <= 0xD7FF) || ((value >= 0xE000) && (value <= 0xFFFF)))  {
               iter = str1.insert(iter, value);

            } else  {
               value -= 0x010000;

               iter = str1.insert(iter, ((value      ) & 0x03FF) + 0xDC00);
               iter = str1.insert(iter, ((value >> 10) & 0x03FF) + 0xD800);
            }

         }

         return iter;
      }

      static size_type walk(size_type len, std::vector<storage_unit>::const_iterator iter)
      {
         size_type retval = 0;
         size_type count  = 0;

         if (len >= 0) {
            // walk forward

            for (size_type x = 0; x < len; ++x) {
               uint16_t value = *iter;

               count = numOfBytes(value);
               iter += count;

               retval += count;
            }

         } else  {
            // walk backwards

            for (size_type x = 0; x > len; --x) {

               while (true) {
                  --iter;
                  --retval;

                  uint16_t value = *iter;

                  if ((value & 0xFC00) != 0xDC00) {
                     // at the beginning of a char
                     break;
                  }
               }

               // inside of the for loop
            }
         }

         return retval;
      }

      static CsChar getCodePoint(std::vector<storage_unit>::const_iterator iter)
      {
         char32_t value = 0;
         uint16_t tmp   = *iter;

         if ((tmp & 0xFC00) != 0xD800) {
            value = tmp;

         } else {
            value = (tmp & 0x03FF) << 10;

            tmp    =  iter[1];
            value |= (tmp & 0x03FF);
            value |= 0x010000;
         }

         return CsChar(value);
      }

   private:
      static size_type numOfBytes(uint16_t value)
      {
         if ((value & 0xFC00) == 0xD800) {
            return 2;
         }

         return 1;
      }
};

}

#endif