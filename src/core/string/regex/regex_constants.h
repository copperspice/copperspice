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

#ifndef CS_REGEX_CONSTANTS_H
#define CS_REGEX_CONSTANTS_H

namespace cs_regex_ns {

// handles error codes and flags

class LIB_CS_STRING_EXPORT regbase
{
 public:
   enum flag_type_ {

      // Divide the flags up into logical groups
      // bits 0-7 indicate main synatx type.
      // bits 8-15 indicate syntax subtype.
      // bits 16-31 indicate options that are common to all regex syntaxes
      // In all cases the default is 0.

      // main synatx group
      perl_syntax_group  = 0,                           // default
      basic_syntax_group = 1,                           // POSIX basic
      literal            = 2,                           // all characters are literals
      main_option_type = literal | basic_syntax_group | perl_syntax_group, // everything


      // options specific to perl group
      no_bk_refs = 1 << 8,                              // \d not allowed
      no_perl_ex = 1 << 9,                              // disable perl extensions
      no_mod_m   = 1 << 10,                             // disable Perl m modifier
      mod_x      = 1 << 11,                             // Perl x modifier
      mod_s      = 1 << 12,                             // force s modifier on (overrides match_not_dot_newline)
      no_mod_s   = 1 << 13,                             // force s modifier off (overrides match_not_dot_newline)


      // options specific to basic group
      no_char_classes = 1 << 8,                         // [[:CLASS:]] not allowed
      no_intervals    = 1 << 9,                         // {x,y} not allowed
      bk_plus_qm      = 1 << 10,                        // uses \+ and \?
      bk_vbar         = 1 << 11,                        // use \| for alternatives
      emacs_ex        = 1 << 12,                        // enables emacs extensions

      // options common to all groups
      no_escape_in_lists = 1 << 16,                     // '\' not special inside [...]
      newline_alt = 1 << 17,                            // \n is the same as |
      no_except = 1 << 18,                              // no exception on error
      failbit = 1 << 19,                                // error flag
      icase = 1 << 20,                                  // characters are matched regardless of case
      nocollate = 0,                                    // don't use locale specific collation (deprecated)
      collate = 1 << 21,                                // use locale specific collation
      nosubs = 1 << 22,                                 // don't mark sub-expressions
      save_subexpression_location = 1 << 23,            // save subexpression locations
      no_empty_expressions = 1 << 24,                   // no empty expressions allowed
      optimize = 0,                                     // not really supported


      basic      = basic_syntax_group | collate | no_escape_in_lists,
      extended   = no_bk_refs | collate | no_perl_ex | no_escape_in_lists,
      normal     = 0,
      emacs      = basic_syntax_group | collate | emacs_ex | bk_vbar,
      awk        = no_bk_refs | collate | no_perl_ex,
      grep       = basic | newline_alt,
      egrep      = extended | newline_alt,
      sed        = basic,
      perl       = normal,
      ECMAScript = normal,
      JavaScript = normal,
      JScript    = normal
   };
   typedef unsigned int flag_type;

   enum restart_info {
      restart_any       = 0,
      restart_word      = 1,
      restart_line      = 2,
      restart_buf       = 3,
      restart_continue  = 4,
      restart_lit       = 5,
      restart_fixed_lit = 6,
      restart_count     = 7
   };
};


// provide std lib proposal compatible constants

namespace regex_constants {

enum flag_type_ {

   no_except = cs_regex_ns::regbase::no_except,
   failbit = cs_regex_ns::regbase::failbit,
   literal = cs_regex_ns::regbase::literal,
   icase = cs_regex_ns::regbase::icase,
   nocollate = cs_regex_ns::regbase::nocollate,
   collate = cs_regex_ns::regbase::collate,
   nosubs = cs_regex_ns::regbase::nosubs,
   optimize = cs_regex_ns::regbase::optimize,
   bk_plus_qm = cs_regex_ns::regbase::bk_plus_qm,
   bk_vbar = cs_regex_ns::regbase::bk_vbar,
   no_intervals = cs_regex_ns::regbase::no_intervals,
   no_char_classes = cs_regex_ns::regbase::no_char_classes,
   no_escape_in_lists = cs_regex_ns::regbase::no_escape_in_lists,
   no_mod_m = cs_regex_ns::regbase::no_mod_m,
   mod_x = cs_regex_ns::regbase::mod_x,
   mod_s = cs_regex_ns::regbase::mod_s,
   no_mod_s = cs_regex_ns::regbase::no_mod_s,
   save_subexpression_location = cs_regex_ns::regbase::save_subexpression_location,
   no_empty_expressions = cs_regex_ns::regbase::no_empty_expressions,

   basic = cs_regex_ns::regbase::basic,
   extended = cs_regex_ns::regbase::extended,
   normal = cs_regex_ns::regbase::normal,
   emacs = cs_regex_ns::regbase::emacs,
   awk = cs_regex_ns::regbase::awk,
   grep = cs_regex_ns::regbase::grep,
   egrep = cs_regex_ns::regbase::egrep,
   sed          = basic,
   perl         = normal,
   ECMAScript   = normal,
   JavaScript   = normal,
   JScript      = normal
};

typedef cs_regex_ns::regbase::flag_type syntax_option_type;

using syntax_type        = unsigned char;
using escape_syntax_type = unsigned char;

// values chosen are binary compatible with previous version

static const syntax_type syntax_char = 0;
static const syntax_type syntax_open_mark = 1;
static const syntax_type syntax_close_mark = 2;
static const syntax_type syntax_dollar = 3;
static const syntax_type syntax_caret = 4;
static const syntax_type syntax_dot = 5;
static const syntax_type syntax_star = 6;
static const syntax_type syntax_plus = 7;
static const syntax_type syntax_question = 8;
static const syntax_type syntax_open_set = 9;
static const syntax_type syntax_close_set = 10;
static const syntax_type syntax_or = 11;
static const syntax_type syntax_escape = 12;
static const syntax_type syntax_dash = 14;
static const syntax_type syntax_open_brace = 15;
static const syntax_type syntax_close_brace = 16;
static const syntax_type syntax_digit = 17;
static const syntax_type syntax_comma = 27;
static const syntax_type syntax_equal = 37;
static const syntax_type syntax_colon = 36;
static const syntax_type syntax_not = 53;

// extensions
static const syntax_type syntax_hash = 13;
static const syntax_type syntax_newline = 26;

// escapes
static const escape_syntax_type escape_type_word_assert     = 18;
static const escape_syntax_type escape_type_not_word_assert = 19;
static const escape_syntax_type escape_type_control_f       = 29;
static const escape_syntax_type escape_type_control_n       = 30;
static const escape_syntax_type escape_type_control_r       = 31;
static const escape_syntax_type escape_type_control_t       = 32;
static const escape_syntax_type escape_type_control_v       = 33;
static const escape_syntax_type escape_type_hex             = 34;
static const escape_syntax_type escape_type_ascii_control   = 35;         // not used
static const escape_syntax_type escape_type_unicode = 0;                  // not used
static const escape_syntax_type escape_type_identity = 0;                 // not used
static const escape_syntax_type escape_type_backref = syntax_digit;
static const escape_syntax_type escape_type_decimal = syntax_digit;       // not used
static const escape_syntax_type escape_type_class = 22;
static const escape_syntax_type escape_type_not_class = 23;

// extensions
static const escape_syntax_type escape_type_left_word = 20;
static const escape_syntax_type escape_type_right_word = 21;
static const escape_syntax_type escape_type_start_buffer = 24;            // for \`
static const escape_syntax_type escape_type_end_buffer = 25;              // for \'
static const escape_syntax_type escape_type_control_a = 28;               // for \a
static const escape_syntax_type escape_type_e = 38;                       // for \e
static const escape_syntax_type escape_type_E = 47;                       // for \Q\E
static const escape_syntax_type escape_type_Q = 48;                       // for \Q\E
static const escape_syntax_type escape_type_X = 49;                       // for \X
static const escape_syntax_type escape_type_C = 50;                       // for \C
static const escape_syntax_type escape_type_Z = 51;                       // for \Z
static const escape_syntax_type escape_type_G = 52;                       // for \G

static const escape_syntax_type escape_type_property = 54;                // for \p
static const escape_syntax_type escape_type_not_property = 55;            // for \P
static const escape_syntax_type escape_type_named_char = 56;              // for \N
static const escape_syntax_type escape_type_extended_backref = 57;        // for \g
static const escape_syntax_type escape_type_reset_start_mark = 58;        // for \K
static const escape_syntax_type escape_type_line_ending = 59;             // for \R

static const escape_syntax_type syntax_max = 60;

} // namespace

} // namespace

#endif

