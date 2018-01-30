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

#ifndef CS_REGEX_FWD_H
#define CS_REGEX_FWD_H

#include "regex/r_config.h"

namespace cs_regex_ns {


template <class charT, class implementationT>
struct regex_traits;

template <class charT, class traits>
class basic_regex;


namespace cs_regex_detail_ns {

template <class T>
struct regex_iterator_traits : public std::iterator_traits<T> {};

} // namespace


} // namespace


#endif




