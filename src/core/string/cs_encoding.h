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

#ifndef LIB_CS_ENCODING_H
#define LIB_CS_ENCODING_H

#include <stdint.h>
#include <vector>

#include <cs_char.h>

namespace CsString {

class LIB_CS_STRING_EXPORT utf8
{
   public:
      using storage_unit = uint8_t;

      static void append(std::vector<storage_unit> &str1, const std::vector<storage_unit> &str2,
                  int posStart = 0, int length = -1)
      {
         // may need to verify if some prefix needs to be ignored from str2

         auto iter_begin = str2.begin();
         auto iter_end   = str2.end() - 1;

         for (int x = 0; x < posStart && iter_begin != str2.end(); ++x) {
           uint8_t value = *iter_begin;
           int count     = numOfBytes(value);

            for (int y = 0; y < count && iter_begin != str2.end(); ++y) {
               ++iter_begin;
            }
         }

         if (length != -1) {
            iter_end   = iter_begin;

            for (int x = 0; x < length && iter_end != str2.end(); ++x) {
               uint8_t value = *iter_end;
               int count     = numOfBytes(value);

               for (int y = 0; y < count && iter_end != str2.end(); ++y) {
                  ++iter_end;
               }

            }
         }

         // watch out for the null terminator
         str1.insert(str1.end() - 1, iter_begin, iter_end);
      }


      template <typename Container>
      static typename Container::const_iterator insert(Container &str1,
                  typename Container::const_iterator iter, CsChar c, int size = 1)
      {
         uint32_t value = c.unicode();

         for (int x = 0; x < size; ++x)  {
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

      static int walk(int len, std::vector<storage_unit>::const_iterator iter)
      {
         int retval = 0;
         int count  = 0;

         if (len >= 0) {
            // walk forward

            for (int x = 0; x < len; ++x) {
               uint8_t value = *iter;

               count = numOfBytes(value);
               iter += count;

               retval += count;
            }

         } else  {
            // walk backwards

            for (int x = 0; x > len; --x) {

               while (true) {
                  --iter;
                  --retval;

                  uint8_t value = *iter;

                  if ((value & 0xC0) != 0x80)  {
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
      static int numOfBytes(uint8_t value)
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
      using storage_unit = uint16_t;

      static void append(std::vector<storage_unit> &str1, const std::vector<storage_unit> &str2,
                  int posStart = 0, int length = -1)
      {
         // may need to verify if some prefix needs to be ignored from str2

         auto iter_begin = str2.begin();
         auto iter_end   = str2.end() - 1;

         for (int x = 0; x < posStart && iter_begin != str2.end(); ++x) {
           uint16_t value = *iter_begin;
           int count     = numOfBytes(value);

            for (int y = 0; y < count && iter_begin != str2.end(); ++y) {
               ++iter_begin;
            }
         }

         if (length != -1) {
            iter_end   = iter_begin;

            for (int x = 0; x < length && iter_end != str2.end(); ++x) {
               uint16_t value = *iter_end;
               int count      = numOfBytes(value);

               for (int y = 0; y < count && iter_end != str2.end(); ++y) {
                  ++iter_end;
               }

            }
         }

         // watch out for the null terminator
         str1.insert(str1.end() - 1, iter_begin, iter_end);

      };

      template <typename Container>
      static typename Container::const_iterator insert(Container &str1,
                  typename Container::const_iterator iter, CsChar c, int size = 1)
      {
         uint32_t value = c.unicode();

         for (int x = 0; x < size; ++x)  {

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

      static int walk(int len, std::vector<storage_unit>::const_iterator iter)
      {
         int retval = 0;
         int count  = 0;

         if (len >= 0) {
            // walk forward

            for (int x = 0; x < len; ++x) {
               uint16_t value = *iter;

               count = numOfBytes(value);
               iter += count;

               retval += count;
            }

         } else  {
            // walk backwards

            for (int x = 0; x > len; --x) {

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
      static int numOfBytes(uint16_t value)
      {
         if ((value & 0xFC00) == 0xD800) {
            return 2;
         }

         return 1;
      }
};

}

#endif