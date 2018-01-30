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

} // namespac

#endif


