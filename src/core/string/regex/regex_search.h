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

#ifndef CS_REGEX_SEARCH_H
#define CS_REGEX_SEARCH_H

#include "regex/perl_matcher.h"

namespace cs_regex_ns {

template <class BidiIterator, class Allocator, class charT, class Traits>
bool regex_search(BidiIterator first, BidiIterator last, match_results<Traits, Allocator> &m,
                  const basic_regex<charT, Traits> &e, match_flag_type flags, BidiIterator base)
{
   if (e.flags() & regex_constants::failbit) {
      return false;
   }

   cs_regex_detail_ns::perl_matcher<BidiIterator, Allocator, Traits> matcher(first, last, m, e, flags, base);

   return matcher.find();
}

template <class BidiIterator, class Allocator, class charT, class Traits>
bool regex_search(BidiIterator first, BidiIterator last, match_results<Traits, Allocator> &m,
                  const basic_regex<charT, Traits> &e, match_flag_type flags = match_default)
{
   return regex_search(first, last, m, e, flags, first);
}

template <class charT, class Allocator, class Traits>
bool regex_search(const charT *str, match_results<const charT *, Allocator> &m,
                  const basic_regex<charT, Traits> &e, match_flag_type flags = match_default)
{
   return regex_search(str, str + Traits::length(str), m, e, flags);
}

template <class ST, class SA, class Allocator, class charT, class Traits>
bool regex_search(const std::basic_string<charT, ST, SA> &s,
                  match_results<typename std::basic_string<charT, ST, SA>::const_iterator, Allocator> &m,
                  const basic_regex<charT, Traits> &e, match_flag_type flags = match_default)
{
   return regex_search(s.begin(), s.end(), m, e, flags);
}

template <class BidiIterator, class charT, class Traits>
bool regex_search(BidiIterator first, BidiIterator last, const basic_regex<charT, Traits> &e, match_flag_type flags = match_default)
{
   if (e.flags() & regex_constants::failbit) {
      return false;
   }

   using match_alloc_type =  typename match_results<Traits>::allocator_type;

   match_results<Traits> m;

   cs_regex_detail_ns::perl_matcher<BidiIterator, match_alloc_type, Traits> matcher(first, last, m, e,
                  flags | regex_constants::match_any, first);

   return matcher.find();
}

template <class charT, class Traits>
bool regex_search(const charT *str, const basic_regex<charT, Traits> &e, match_flag_type flags = match_default)
{
   return regex_search(str, str + Traits::length(str), e, flags);
}

template <class ST, class SA, class charT, class Traits>
bool regex_search(const std::basic_string<charT, ST, SA> &s,
                  const basic_regex<charT, Traits> &e, match_flag_type flags = match_default)
{
   return regex_search(s.begin(), s.end(), e, flags);
}


}

#endif
