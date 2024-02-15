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

#ifndef CS_PATTERN_EXCEPT_H
#define CS_PATTERN_EXCEPT_H

#include <regex/r_config.h>

#include <stdexcept>
#include <cstddef>

namespace cs_regex_ns {

class LIB_CS_STRING_EXPORT regex_error : public std::runtime_error
{
 public:
   explicit regex_error(const std::string &s, regex_constants::error_type err = regex_constants::error_unknown, std::ptrdiff_t pos = 0)
      : std::runtime_error(s), m_error_code(err), m_position(pos) {
   }

   explicit regex_error(regex_constants::error_type err)
      : std::runtime_error(cs_regex_ns::cs_regex_detail_ns::get_default_error_string(err)), m_error_code(err), m_position(0) {
   }

   ~regex_error() noexcept {
   }

   regex_constants::error_type code() const {
      return m_error_code;
   }

   std::ptrdiff_t position() const {
      return m_position;
   }

   void raise() const {
      throw *this;
   }

 private:
   regex_constants::error_type m_error_code;
   std::ptrdiff_t m_position;
};

using bad_pattern    = regex_error;
using bad_expression = regex_error;

namespace cs_regex_detail_ns {

template <class traits>
void raise_error(const traits &t, regex_constants::error_type code)
{
   std::runtime_error err(t.error_string(code));
   throw (err);
}

}   // end namespace

}   // end namespace

#endif
