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

#ifndef CS_CONFIG_H
#define CS_CONFIG_H

#if defined(_UNICODE) && ! defined(UNICODE)
#   define UNICODE
#endif

#ifndef CS_REGEX_MAX_STATE_COUNT
#  define CS_REGEX_MAX_STATE_COUNT 100000000
#endif

#ifndef CS_REGEX_BLOCKSIZE
#   define CS_REGEX_BLOCKSIZE 4096
#endif

#if CS_REGEX_BLOCKSIZE < 512
#   error "CS_REGEX_BLOCKSIZE must be at least 512"
#endif

#ifndef CS_REGEX_MAX_BLOCKS
#   define CS_REGEX_MAX_BLOCKS 1024
#endif

#ifndef CS_REGEX_MAX_CACHE_BLOCKS
#   define CS_REGEX_MAX_CACHE_BLOCKS 16
#endif


#ifdef _WIN32

#ifdef BUILDING_LIB_CS_STRING
# define LIB_CS_STRING_EXPORT     __declspec(dllexport)
#else
# define LIB_CS_STRING_EXPORT     __declspec(dllimport)
#endif

#else
# define LIB_CS_STRING_EXPORT

#endif

#include "regex/r_error_type.h"
#include "regex/regex_constants.h"

#endif

