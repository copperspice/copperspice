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
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
*/

#include <cassert>
#include <cstring>
#include <memory>

#include "regex/r_config.h"
#include "regex/regex_raw_buffer.h"

namespace cs_regex_ns {

namespace cs_regex_detail_ns {

void raw_storage::resize(size_type n)
{
   size_type newsize = start ? last - start : 1024;

   while (newsize < n) {
      newsize *= 2;
   }

   size_type datasize = end - start;

   // extend newsize to WORD/DWORD boundary
   newsize = (newsize + padding_mask) & ~(padding_mask);

   // allocate and copy data
   pointer ptr = static_cast<pointer>(operator new (newsize));

   if (start) {
      std::memcpy(ptr, start, datasize);
   }

   // get rid of old buffer
   operator delete (start);

   // set up pointers
   start = ptr;
   end   = ptr + datasize;
   last  = ptr + newsize;
}

void *raw_storage::insert(size_type pos, size_type n)
{
   assert(pos <= size_type(end - start));

   if (size_type(last - end) < n) {
      resize(n + (end - start));
   }

   void *result = start + pos;
   std::memmove(start + pos + n, start + pos, (end - start) - pos);
   end += n;

   return result;
}

} // namespace

} // namespace
