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

#ifndef CS_REGEX_TRAITS_H
#define CS_REGEX_TRAITS_H

#include "regex/regex_fwd.h"
#include "regex/regex_traits_defaults.h"

namespace cs_regex_ns {

template <class charT, class implementationT >
struct regex_traits : public implementationT {
   regex_traits() : implementationT() {}
};

// provide default implementations of the optional interfaces in addition the required standard ones

namespace cs_regex_detail_ns {

template<class T>
struct has_boost_extensions_tag {
   static const bool value = false;
};

template <class BaseT>
struct default_wrapper : public BaseT {
   using char_type   = typename BaseT::char_type;
   using string_type = typename BaseT::string_type;

   std::string error_string(cs_regex_ns::regex_constants::error_type e) const {
      return cs_regex_ns::cs_regex_detail_ns::get_default_error_string(e);
   }

   cs_regex_ns::regex_constants::syntax_type syntax_type(char_type c) const {
      auto value = BaseT::toInt(c);

      if (value < 0x80)  {
         return get_default_syntax_type(static_cast<char>(value));

      } else {
         return cs_regex_ns::regex_constants::syntax_char;
      }
   }

   cs_regex_ns::regex_constants::escape_syntax_type escape_syntax_type(char_type c) const {
      auto value = BaseT::toInt(c);

      if (value < 0x80)  {
         return get_default_escape_syntax_type(static_cast<char>(value));

      } else {
         return cs_regex_ns::regex_constants::escape_type_identity;
      }
   }

   intmax_t toi(typename string_type::const_iterator &p1, const typename string_type::const_iterator p2, int radix) const {
      return cs_regex_ns::cs_regex_detail_ns::global_toi(p1, p2, radix, *this);
   }

   char_type translate(char_type c, bool icase)const {
      return (icase ? this->translate_nocase(c) : this->translate(c));
   }

   char_type translate(char_type c)const {
      return BaseT::translate(c);
   }
};

template <class BaseT, bool has_extensions>
struct compute_wrapper_base {
   typedef BaseT type;
};


template <class BaseT>
struct compute_wrapper_base<BaseT, false> {
   typedef default_wrapper<BaseT> type;
};

} // namespace

template <class BaseT>
struct regex_traits_wrapper
   : public cs_regex_ns::cs_regex_detail_ns::compute_wrapper_base<BaseT,
     cs_regex_ns::cs_regex_detail_ns::has_boost_extensions_tag<BaseT>::value>::type {
   regex_traits_wrapper() {}

 private:
   regex_traits_wrapper(const regex_traits_wrapper &);
   regex_traits_wrapper &operator=(const regex_traits_wrapper &);
};

} // namespace


#endif

