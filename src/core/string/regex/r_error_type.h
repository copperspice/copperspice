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

#ifndef CS_ERROR_TYPE_H
#define CS_ERROR_TYPE_H

namespace cs_regex_ns {

namespace regex_constants {

enum error_type {

   error_ok = 0,            /* not used */
   error_no_match = 1,      /* not used */
   error_bad_pattern = 2,
   error_collate = 3,
   error_ctype = 4,
   error_escape = 5,
   error_backref = 6,
   error_brack = 7,
   error_paren = 8,
   error_brace = 9,
   error_badbrace = 10,
   error_range = 11,
   error_space = 12,
   error_badrepeat = 13,
   error_end = 14,          /* not used */
   error_size = 15,
   error_right_paren = 16,  /* not used */
   error_empty = 17,
   error_complexity = 18,
   error_stack = 19,
   error_perl_extension = 20,
   error_unknown = 21
};

} // namespace

namespace cs_regex_detail_ns {

inline const char *get_default_error_string(cs_regex_ns::regex_constants::error_type n)
{
   static const char *const s_default_error_messages[] = {
      "Success",                                                            /* REG_NOERROR 0 error_ok */
      "No match",                                                           /* REG_NOMATCH 1 error_no_match */
      "Invalid regular expression.",                                        /* REG_BADPAT 2 error_bad_pattern */
      "Invalid collation character.",                                       /* REG_ECOLLATE 3 error_collate */
      "Invalid character class name, collating name, or character range.",  /* REG_ECTYPE 4 error_ctype */
      "Invalid or unterminated escape sequence.",                           /* REG_EESCAPE 5 error_escape */
      "Invalid back reference: specified capturing group does not exist.",  /* REG_ESUBREG 6 error_backref */
      "Unmatched [ or [^ in character class declaration.",                  /* REG_EBRACK 7 error_brack */
      "Unmatched marking parenthesis ( or \\(.",                            /* REG_EPAREN 8 error_paren */
      "Unmatched quantified repeat operator { or \\{.",                     /* REG_EBRACE 9 error_brace */
      "Invalid content of repeat range.",                                   /* REG_BADBR 10 error_badbrace */
      "Invalid range end in character class",                               /* REG_ERANGE 11 error_range */
      "Out of memory.",                                                     /* REG_ESPACE 12 error_space NOT USED */
      "Invalid preceding regular expression prior to repetition operator.", /* REG_BADRPT 13 error_badrepeat */
      "Premature end of regular expression",                                /* REG_EEND 14 error_end NOT USED */
      "Regular expression is too large.",                                   /* REG_ESIZE 15 error_size NOT USED */
      "Unmatched ) or \\)",                                                 /* REG_ERPAREN 16 error_right_paren NOT USED */
      "Empty regular expression.",                                          /* REG_EMPTY 17 error_empty */

      "The complexity of matching the regular expression exceeded predefined bounds.  "
      "Try refactoring the regular expression to make each choice made by the state machine unambiguous.  "
      "This exception is thrown to prevent \"eternal\" matches that take an "

      "indefinite period time to locate.",                                  /* REG_ECOMPLEXITY 18 error_complexity */
      "Ran out of stack space trying to match the regular expression.",     /* REG_ESTACK 19 error_stack */
      "Invalid or unterminated Perl (?...) sequence.",                      /* REG_E_PERL 20 error_perl */
      "Unknown error.",                                                     /* REG_E_UNKNOWN 21 error_unknown */
   };

   return (n > cs_regex_ns::regex_constants::error_unknown) ? s_default_error_messages[cs_regex_ns::regex_constants::error_unknown] :
          s_default_error_messages[n];
}


} // namespace
} // namespace

#endif
