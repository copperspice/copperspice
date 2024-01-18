/***********************************************************************
*
* Copyright (c) 2017-2024 Barbara Geller
* Copyright (c) 2017-2024 Ansel Sermersheim
*
* Copyright (c) 1998-2009 John Maddock
*
* This file is part of CopperSpice.
*
* CopperSpice is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

/*
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef CS_REGEX_MERGE_H
#define CS_REGEX_MERGE_H

namespace cs_regex_ns {

template <class OutputIterator, class Iterator, class traits, class charT>
inline OutputIterator regex_merge(OutputIterator out, Iterator first, Iterator last,
      const basic_regex<charT, traits> &e, const charT *fmt, match_flag_type flags = match_default)
{
   return regex_replace(out, first, last, e, fmt, flags);
}

template <class OutputIterator, class Iterator, class traits, class charT>
inline OutputIterator regex_merge(OutputIterator out, Iterator first, Iterator last,
      const basic_regex<charT, traits> &e, const std::basic_string<charT> &fmt,
      match_flag_type flags = match_default)
{
   return regex_merge(out, first, last, e, fmt.c_str(), flags);
}

template <class traits, class charT>
inline std::basic_string<charT> regex_merge(const std::basic_string<charT> &s,
      const basic_regex<charT, traits> &e, const charT *fmt,
      match_flag_type flags = match_default)
{
   return regex_replace(s, e, fmt, flags);
}

template <class traits, class charT>
inline std::basic_string<charT> regex_merge(const std::basic_string<charT> &s,
      const basic_regex<charT, traits> &e, const std::basic_string<charT> &fmt,
      match_flag_type flags = match_default)
{
   return regex_replace(s, e, fmt, flags);
}

}   // end namespace

#endif

