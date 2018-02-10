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

#ifndef CS_REGEX_MATCH_H
#define CS_REGEX_MATCH_H

#include "regex/perl_matcher.h"

namespace cs_regex_ns {

// returns true if the specified regular expression matches
// the whole of the input.  Fills in what matched in m.

template <class BidiIterator, class Allocator, class charT, class traits>
bool regex_match(BidiIterator first, BidiIterator last, match_results<BidiIterator, Allocator> &m,
                  const basic_regex<charT, traits> &e, match_flag_type flags = match_default)
{
   cs_regex_detail_ns::perl_matcher<BidiIterator, Allocator, traits> matcher(first, last, m, e, flags, first);
   return matcher.match();
}

template <class iterator, class charT, class traits>
bool regex_match(iterator first, iterator last, const basic_regex<charT, traits> &e, match_flag_type flags = match_default)
{
   match_results<iterator> m;
   return regex_match(first, last, m, e, flags | regex_constants::match_any);
}

template <class charT, class Allocator, class traits>
inline bool regex_match(const charT *str, match_results<const charT *, Allocator> &m,
                  const basic_regex<charT, traits> &e, match_flag_type flags = match_default)
{
   return regex_match(str, str + traits::length(str), m, e, flags);
}

template <class ST, class SA, class Allocator, class charT, class traits>
inline bool regex_match(const std::basic_string<charT, ST, SA> &s,
                  match_results<typename std::basic_string<charT, ST, SA>::const_iterator, Allocator> &m,
                  const basic_regex<charT, traits> &e, match_flag_type flags = match_default)
{
   return regex_match(s.begin(), s.end(), m, e, flags);
}

template <class charT, class traits>
inline bool regex_match(const charT *str, const basic_regex<charT, traits> &e, match_flag_type flags = match_default)
{
   match_results<traits> m;
   return regex_match(str, str + traits::length(str), m, e, flags | regex_constants::match_any);
}

template <class ST, class SA, class charT, class traits>
inline bool regex_match(const std::basic_string<charT, ST, SA> &s, const basic_regex<charT, traits> &e,
                  match_flag_type flags = match_default)
{
   match_results<traits> m;
   return regex_match(s.begin(), s.end(), m, e, flags | regex_constants::match_any);
}

} // namespace

#endif

