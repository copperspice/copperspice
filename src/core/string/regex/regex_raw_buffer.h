/***********************************************************************
*
* Copyright (c) 2017-2018 Barbara Geller
* Copyright (c) 2017-2018 Ansel Sermersheim
* Copyright (c) 1998-2009 John Maddock
* All rights reserved.
*
* This file is part of CsString
*
* CsString is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
***********************************************************************/

/*
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef CS_REGEX_RAW_BUFFER_H
#define CS_REGEX_RAW_BUFFER_H

#include <algorithm>
#include <cstddef>

#include "regex/r_config.h"

namespace cs_regex_ns {

namespace cs_regex_detail_ns {

struct empty_padding {};

union padding {
   void *p;
   unsigned int i;
};

template <int N>
struct padding3 {
   enum {
      padding_size = 8,
      padding_mask = 7
   };
};

template<>
struct padding3<2> {
   enum {
      padding_size = 2,
      padding_mask = 1
   };
};

template<>
struct padding3<4> {
   enum {
      padding_size = 4,
      padding_mask = 3
   };
};

template<>
struct padding3<8> {
   enum {
      padding_size = 8,
      padding_mask = 7
   };
};

template<>
struct padding3<16> {
   enum {
      padding_size = 16,
      padding_mask = 15
   };
};

enum {
   padding_size = padding3<sizeof(padding)>::padding_size,
   padding_mask = padding3<sizeof(padding)>::padding_mask
};

// basically this is a simplified vector<unsigned char>
// this is used by basic_regex for expression storage

class LIB_CS_STRING_EXPORT raw_storage
{
 public:
   typedef std::size_t           size_type;
   typedef unsigned char        *pointer;

 private:
   pointer last, start, end;

 public:
   raw_storage();
   raw_storage(size_type n);

   ~raw_storage() {
      ::operator delete (start);
   }

   void resize(size_type n);

   void *extend(size_type n) {
      if (size_type(last - end) < n) {
         resize(n + (end - start));
      }
      pointer result = end;
      end += n;
      return result;
   }

   void *insert(size_type pos, size_type n);

   size_type size() {
      return end - start;
   }

   size_type capacity() {
      return last - start;
   }

   void *data() const {
      return start;
   }

   size_type index(void *ptr) {
      return static_cast<pointer>(ptr) - static_cast<pointer>(data());
   }

   void clear() {
      end = start;
   }

   void align() {
      // move end up to a boundary:
      end = start + (((end - start) + padding_mask) & ~padding_mask);
   }
   void swap(raw_storage &that) {
      std::swap(start, that.start);
      std::swap(end, that.end);
      std::swap(last, that.last);
   }
};

inline raw_storage::raw_storage()
{
   last = start = end = 0;
}

inline raw_storage::raw_storage(size_type n)
{
   start = end = static_cast<pointer>(::operator new (n));
   last = start + n;
}

} // namespace

} // namespace

#endif






