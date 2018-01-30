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

#ifndef CS_REGEX_TRAITS_DEFAULTS_H
#define CS_REGEX_TRAITS_DEFAULTS_H

#include <type_traits>

#include "regex/r_config.h"

namespace cs_regex_ns {

namespace cs_regex_detail_ns {

LIB_CS_STRING_EXPORT const char                          *get_default_syntax(cs_regex_ns::regex_constants::syntax_type n);
LIB_CS_STRING_EXPORT regex_constants::syntax_type         get_default_syntax_type(char c);
LIB_CS_STRING_EXPORT regex_constants::escape_syntax_type  get_default_escape_syntax_type(char c);
LIB_CS_STRING_EXPORT std::string                          get_default_collate_name(const std::string &name);

// **

template <class charT>
inline bool is_extended(charT c)
{
   using unsigned_type = typename std::make_unsigned<charT>::type;
   return (sizeof(charT) > 1) && (static_cast<unsigned_type>(c) >= 256u);
}

inline bool is_extended(char)
{
   return false;
}

// **

template <class charT>
inline bool is_combining(charT c);

template <>
inline bool is_combining<char>(char)
{
   return false;
}

template <>
inline bool is_combining<signed char>(signed char)
{
   return false;
}

template <>
inline bool is_combining<unsigned char>(unsigned char)
{
   return false;
}

// ** line separator

template <class charT>
inline bool is_separator(charT c)
{
   return (c == static_cast<charT>('\n')) || (c == static_cast<charT>('\r')) || (c == static_cast<charT>('\f'))
          || (c == static_cast<charT>(u'\u2028'))
          || (c == static_cast<charT>(u'\u2029'))
          || (c == static_cast<charT>('\x85'));
}

template <>
inline bool is_separator<char>(char c)
{
   return (c == '\n') || (c == '\r') || (c == '\f');
}


// get the state_id of a character clasification from the traits classes then transform that state_id into a bitmask

template <class charT>
int global_value(charT c)
{
   static const charT zero = '0';
   static const charT nine = '9';
   static const charT a = 'a';
   static const charT f = 'f';
   static const charT A = 'A';
   static const charT F = 'F';

   if (c > f) {
      return -1;
   }
   if (c >= a) {
      return 10 + (c - a);
   }
   if (c > F) {
      return -1;
   }
   if (c >= A) {
      return 10 + (c - A);
   }
   if (c > nine) {
      return -1;
   }
   if (c >= zero) {
      return c - zero;
   }
   return -1;
}

template <class traits>
intmax_t global_toi(typename traits::string_type::const_iterator &p1, const typename traits::string_type::const_iterator p2,
                    int radix, const traits &t)
{
   intmax_t limit = (std::numeric_limits<intmax_t>::max)() / radix;
   intmax_t next_value = t.value(*p1, radix);

   if ((p1 == p2) || (next_value < 0) || (next_value >= radix)) {
      return -1;
   }

   intmax_t result = 0;

   while (p1 != p2) {
      next_value = t.value(*p1, radix);

      if ((next_value < 0) || (next_value >= radix)) {
         break;
      }

      result *= radix;
      result += next_value;
      ++p1;

      if (result > limit) {
         return -1;
      }
   }

   return result;
}

template <class S>
inline const S &get_escape_R_string()
{
   static const char e2[] = { '(', '?', '>', '\\', 'x', '0', 'D', '\\', 'x', '0', 'A', '?',
                              '|', '[', '\\', 'x', '0', 'A', '\\', 'x', '0', 'B', '\\', 'x',
                              '0', 'C', '\\', 'x', '8', '5', ']', ')', '\0'
                            };

   static S retval{e2};

   return retval;
}

inline regex_constants::syntax_type get_default_syntax_type(char c)
{
   // char_syntax determines how the compiler treats a given character in a regular expression

   static regex_constants::syntax_type char_syntax[] = {
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_newline,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,              //  32
      regex_constants::syntax_not,               //  !
      regex_constants::syntax_char,              //  "
      regex_constants::syntax_hash,              //  #
      regex_constants::syntax_dollar,            //  $
      regex_constants::syntax_char,              //  %
      regex_constants::syntax_char,              /*&*/
      regex_constants::escape_type_end_buffer,   /*'*/
      regex_constants::syntax_open_mark,         /*(*/
      regex_constants::syntax_close_mark,        /*)*/
      regex_constants::syntax_star,              /***/
      regex_constants::syntax_plus,              /*+*/
      regex_constants::syntax_comma,             /*,*/
      regex_constants::syntax_dash,              /*-*/
      regex_constants::syntax_dot,               /*.*/
      regex_constants::syntax_char,              /*/*/
      regex_constants::syntax_digit,             /*0*/
      regex_constants::syntax_digit,             /*1*/
      regex_constants::syntax_digit,             /*2*/
      regex_constants::syntax_digit,             /*3*/
      regex_constants::syntax_digit,             /*4*/
      regex_constants::syntax_digit,             /*5*/
      regex_constants::syntax_digit,             /*6*/
      regex_constants::syntax_digit,             /*7*/
      regex_constants::syntax_digit,             /*8*/
      regex_constants::syntax_digit,             /*9*/
      regex_constants::syntax_colon,             /*:*/
      regex_constants::syntax_char,              /*;*/
      regex_constants::escape_type_left_word,    /*<*/
      regex_constants::syntax_equal,             /*=*/
      regex_constants::escape_type_right_word,   /*>*/
      regex_constants::syntax_question,          /*?*/
      regex_constants::syntax_char,              /*@*/
      regex_constants::syntax_char,              /*A*/
      regex_constants::syntax_char,              /*B*/
      regex_constants::syntax_char,              /*C*/
      regex_constants::syntax_char,              /*D*/
      regex_constants::syntax_char,              /*E*/
      regex_constants::syntax_char,              /*F*/
      regex_constants::syntax_char,              /*G*/
      regex_constants::syntax_char,              /*H*/
      regex_constants::syntax_char,              /*I*/
      regex_constants::syntax_char,              /*J*/
      regex_constants::syntax_char,              /*K*/
      regex_constants::syntax_char,              /*L*/
      regex_constants::syntax_char,              /*M*/
      regex_constants::syntax_char,              /*N*/
      regex_constants::syntax_char,              /*O*/
      regex_constants::syntax_char,              /*P*/
      regex_constants::syntax_char,              /*Q*/
      regex_constants::syntax_char,              /*R*/
      regex_constants::syntax_char,              /*S*/
      regex_constants::syntax_char,              /*T*/
      regex_constants::syntax_char,              /*U*/
      regex_constants::syntax_char,              /*V*/
      regex_constants::syntax_char,              /*W*/
      regex_constants::syntax_char,              /*X*/
      regex_constants::syntax_char,              /*Y*/
      regex_constants::syntax_char,              /*Z*/
      regex_constants::syntax_open_set,          /*[*/
      regex_constants::syntax_escape,            /*\*/
      regex_constants::syntax_close_set,         /*]*/
      regex_constants::syntax_caret,             /*^*/
      regex_constants::syntax_char,              /*_*/
      regex_constants::syntax_char,              /*`*/
      regex_constants::syntax_char,              /*a*/
      regex_constants::syntax_char,              /*b*/
      regex_constants::syntax_char,              /*c*/
      regex_constants::syntax_char,              /*d*/
      regex_constants::syntax_char,              /*e*/
      regex_constants::syntax_char,              /*f*/
      regex_constants::syntax_char,              /*g*/
      regex_constants::syntax_char,              /*h*/
      regex_constants::syntax_char,              /*i*/
      regex_constants::syntax_char,              /*j*/
      regex_constants::syntax_char,              /*k*/
      regex_constants::syntax_char,              /*l*/
      regex_constants::syntax_char,              /*m*/
      regex_constants::syntax_char,              /*n*/
      regex_constants::syntax_char,              /*o*/
      regex_constants::syntax_char,              /*p*/
      regex_constants::syntax_char,              /*q*/
      regex_constants::syntax_char,              /*r*/
      regex_constants::syntax_char,              /*s*/
      regex_constants::syntax_char,              /*t*/
      regex_constants::syntax_char,              /*u*/
      regex_constants::syntax_char,              /*v*/
      regex_constants::syntax_char,              /*w*/
      regex_constants::syntax_char,              /*x*/
      regex_constants::syntax_char,              /*y*/
      regex_constants::syntax_char,              /*z*/
      regex_constants::syntax_open_brace,        /*{*/
      regex_constants::syntax_or,                /*|*/
      regex_constants::syntax_close_brace,       /*}*/
      regex_constants::syntax_char,              /*~*/
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
      regex_constants::syntax_char,
   };

   return char_syntax[(unsigned char)c];
}

inline regex_constants::escape_syntax_type get_default_escape_syntax_type(char c)
{
   // char_syntax determines how the compiler treats a given character  in a regular expression.

   static regex_constants::escape_syntax_type char_syntax[] = {
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,        //  32
      regex_constants::escape_type_identity,        /*!*/
      regex_constants::escape_type_identity,        /*"*/
      regex_constants::escape_type_identity,        /*#*/
      regex_constants::escape_type_identity,        /*$*/
      regex_constants::escape_type_identity,        /*%*/
      regex_constants::escape_type_identity,        /*&*/
      regex_constants::escape_type_end_buffer,      /*'*/
      regex_constants::syntax_open_mark,            /*(*/
      regex_constants::syntax_close_mark,            /*)*/
      regex_constants::escape_type_identity,        /***/
      regex_constants::syntax_plus,                 /*+*/
      regex_constants::escape_type_identity,        /*,*/
      regex_constants::escape_type_identity,        /*-*/
      regex_constants::escape_type_identity,        /*.*/
      regex_constants::escape_type_identity,        /*/*/
      regex_constants::escape_type_decimal,        /*0*/
      regex_constants::escape_type_backref,        /*1*/
      regex_constants::escape_type_backref,        /*2*/
      regex_constants::escape_type_backref,        /*3*/
      regex_constants::escape_type_backref,        /*4*/
      regex_constants::escape_type_backref,        /*5*/
      regex_constants::escape_type_backref,        /*6*/
      regex_constants::escape_type_backref,        /*7*/
      regex_constants::escape_type_backref,        /*8*/
      regex_constants::escape_type_backref,        /*9*/
      regex_constants::escape_type_identity,        /*:*/
      regex_constants::escape_type_identity,        /*;*/
      regex_constants::escape_type_left_word,        /*<*/
      regex_constants::escape_type_identity,        /*=*/
      regex_constants::escape_type_right_word,        /*>*/
      regex_constants::syntax_question,              /*?*/
      regex_constants::escape_type_identity,         /*@*/
      regex_constants::escape_type_start_buffer,     /*A*/
      regex_constants::escape_type_not_word_assert,  /*B*/
      regex_constants::escape_type_C,                /*C*/
      regex_constants::escape_type_not_class,        /*D*/
      regex_constants::escape_type_E,                /*E*/
      regex_constants::escape_type_not_class,        /*F*/
      regex_constants::escape_type_G,                /*G*/
      regex_constants::escape_type_not_class,        /*H*/
      regex_constants::escape_type_not_class,        /*I*/
      regex_constants::escape_type_not_class,        /*J*/
      regex_constants::escape_type_reset_start_mark, /*K*/
      regex_constants::escape_type_not_class,        /*L*/
      regex_constants::escape_type_not_class,        /*M*/
      regex_constants::escape_type_named_char,       /*N*/
      regex_constants::escape_type_not_class,        /*O*/
      regex_constants::escape_type_not_property,     /*P*/
      regex_constants::escape_type_Q,                /*Q*/
      regex_constants::escape_type_line_ending,      /*R*/
      regex_constants::escape_type_not_class,        /*S*/
      regex_constants::escape_type_not_class,        /*T*/
      regex_constants::escape_type_not_class,        /*U*/
      regex_constants::escape_type_not_class,        /*V*/
      regex_constants::escape_type_not_class,        /*W*/
      regex_constants::escape_type_X,                /*X*/
      regex_constants::escape_type_not_class,        /*Y*/
      regex_constants::escape_type_Z,                /*Z*/
      regex_constants::escape_type_identity,           /*[*/
      regex_constants::escape_type_identity,           /*\*/
      regex_constants::escape_type_identity,           /*]*/
      regex_constants::escape_type_identity,           /*^*/
      regex_constants::escape_type_identity,           /*_*/
      regex_constants::escape_type_start_buffer,       /*`*/
      regex_constants::escape_type_control_a,          /*a*/
      regex_constants::escape_type_word_assert,        /*b*/
      regex_constants::escape_type_ascii_control,      /*c*/
      regex_constants::escape_type_class,              /*d*/
      regex_constants::escape_type_e,                  /*e*/
      regex_constants::escape_type_control_f,          /*f*/
      regex_constants::escape_type_extended_backref,   /*g*/
      regex_constants::escape_type_class,              /*h*/
      regex_constants::escape_type_class,              /*i*/
      regex_constants::escape_type_class,              /*j*/
      regex_constants::escape_type_extended_backref,   /*k*/
      regex_constants::escape_type_class,           /*l*/
      regex_constants::escape_type_class,           /*m*/
      regex_constants::escape_type_control_n,       /*n*/
      regex_constants::escape_type_class,           /*o*/
      regex_constants::escape_type_property,        /*p*/
      regex_constants::escape_type_class,           /*q*/
      regex_constants::escape_type_control_r,       /*r*/
      regex_constants::escape_type_class,           /*s*/
      regex_constants::escape_type_control_t,       /*t*/
      regex_constants::escape_type_class,           /*u*/
      regex_constants::escape_type_control_v,       /*v*/
      regex_constants::escape_type_class,           /*w*/
      regex_constants::escape_type_hex,             /*x*/
      regex_constants::escape_type_class,           /*y*/
      regex_constants::escape_type_end_buffer,      /*z*/
      regex_constants::syntax_open_brace,           /*{*/
      regex_constants::syntax_or,                   /*|*/
      regex_constants::syntax_close_brace,          /*}*/
      regex_constants::escape_type_identity,        /*~*/
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
      regex_constants::escape_type_identity,
   };

   return char_syntax[(unsigned char)c];
}


} // namespace
} // namespace

#endif
