/***********************************************************************
*
* Copyright (c) 2017-2025 Barbara Geller
* Copyright (c) 2017-2025 Ansel Sermersheim
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

#ifndef CS_STATES_H
#define CS_STATES_H

#include <regex/regex_raw_buffer.h>

#include <climits>

namespace cs_regex_ns {

namespace cs_regex_detail_ns {

/*** mask_type *******************************************************
Whenever we have a choice of two alternatives, we use an array of bytes
to indicate which of the two alternatives it is possible to take for any
given input character.  If mask_take is set, then we can take the next
state, and if mask_skip is set then we can take the alternative.
***********************************************************************/
enum mask_type {
   mask_take = 1,
   mask_skip = 2,
   mask_init = 4,
   mask_any = mask_skip | mask_take,
   mask_all = mask_any
};


// function overload resolution to detect whether we have narrow or wide character strings:
struct _narrow_type {};
struct _wide_type {};

template <class charT>
struct is_byte;

template <>
struct is_byte<char>         {
   using width_type = _narrow_type;
};

template <>
struct is_byte<unsigned char> {
   using width_type = _narrow_type;
};

template <>
struct is_byte<signed char>  {
   using width_type = _narrow_type;
};

template <class charT>
struct is_byte               {
   using width_type = _wide_type;
};


// Every record in the state machine falls into one of the following types:
enum syntax_element_type {
   // start of a marked sub-expression, or perl-style (?...) extension
   syntax_element_startmark = 0,

   // end of a marked sub-expression, or perl-style (?...) extension
   syntax_element_endmark = syntax_element_startmark + 1,

   // any sequence of literal characters
   syntax_element_literal = syntax_element_endmark + 1,

   // start of line assertion: ^
   syntax_element_start_line = syntax_element_literal + 1,

   // end of line assertion $
   syntax_element_end_line = syntax_element_start_line + 1,

   // match any character: .
   syntax_element_wild = syntax_element_end_line + 1,

   // end of expression: we have a match when we get here
   syntax_element_match = syntax_element_wild + 1,

   // perl style word boundary: \b
   syntax_element_word_boundary = syntax_element_match + 1,

   // perl style within word boundary: \B
   syntax_element_within_word = syntax_element_word_boundary + 1,

   // start of word assertion: \<
   syntax_element_word_start = syntax_element_within_word + 1,

   // end of word assertion: \>
   syntax_element_word_end = syntax_element_word_start + 1,
   // start of buffer assertion: \`
   syntax_element_buffer_start = syntax_element_word_end + 1,
   // end of buffer assertion: \'
   syntax_element_buffer_end = syntax_element_buffer_start + 1,
   // backreference to previously matched sub-expression
   syntax_element_backref = syntax_element_buffer_end + 1,
   // either a wide character set [..] or one with multicharacter collating elements:
   syntax_element_long_set = syntax_element_backref + 1,
   // narrow character set: [...]
   syntax_element_set = syntax_element_long_set + 1,
   // jump to a new state in the machine:
   syntax_element_jump = syntax_element_set + 1,
   // choose between two production states:
   syntax_element_alt = syntax_element_jump + 1,
   // a repeat
   syntax_element_rep = syntax_element_alt + 1,
   // match a combining character sequence
   syntax_element_combining = syntax_element_rep + 1,
   // perl style soft buffer end: \z
   syntax_element_soft_buffer_end = syntax_element_combining + 1,
   // perl style continuation: \G
   syntax_element_restart_continue = syntax_element_soft_buffer_end + 1,
   // single character repeats:
   syntax_element_dot_rep = syntax_element_restart_continue + 1,
   syntax_element_char_rep = syntax_element_dot_rep + 1,
   syntax_element_short_set_rep = syntax_element_char_rep + 1,
   syntax_element_long_set_rep = syntax_element_short_set_rep + 1,
   // a backstep for lookbehind repeats:
   syntax_element_backstep = syntax_element_long_set_rep + 1,
   // an assertion that a mark was matched:
   syntax_element_assert_backref = syntax_element_backstep + 1,
   syntax_element_toggle_case = syntax_element_assert_backref + 1,

   // a recursive expression:
   syntax_element_recurse = syntax_element_toggle_case + 1,

   // Verbs:
   syntax_element_fail = syntax_element_recurse + 1,
   syntax_element_accept = syntax_element_fail + 1,
   syntax_element_commit = syntax_element_accept + 1,
   syntax_element_then = syntax_element_commit + 1
};

struct re_syntax_base;


// Points to another state in the machine.  During machine construction
// we use integral offsets, but these are converted to pointers before execution of the machine.
union offset_type {
   re_syntax_base   *p;
   std::ptrdiff_t    i;
};


// Base class for all states in the machine.
struct re_syntax_base {
   syntax_element_type   type;         // what kind of state this is
   offset_type           next;         // next state in the machine
};


// A marked parenthesis
struct re_brace : public re_syntax_base {
   // The index to match, can be zero (don't mark the sub-expression)
   // or negative (for perl style (?...) extentions):
   int index;
   bool icase;
};


// Match anything
enum {
   dont_care         = 1,
   force_not_newline = 0,
   force_newline     = 2,
   test_not_newline  = 2,
   test_newline      = 3
};

struct re_dot : public re_syntax_base {
   unsigned char mask;
};

// A string of literals, following this structure will be an
// array of characters: charT[length]
struct re_literal : public re_syntax_base {
   unsigned int length;
};


// Indicates whether we are moving to a case insensive block or not
struct re_case : public re_syntax_base {
   bool icase;
};


// A wide character set of characters, following this structure will be an array of type charT:
// First csingles null-terminated strings
// Then 2 * cranges NULL terminated strings
// Then cequivalents NULL terminated strings

template <class mask_type>
struct re_set_long : public re_syntax_base {
   unsigned int            csingles, cranges, cequivalents;
   mask_type               cclasses;
   mask_type               cnclasses;
   bool                    isnot;
   bool                    singleton;
};


// A set of narrow-characters, matches any of _map which is none-zero

struct re_set : public re_syntax_base {
   unsigned char _map[1 << CHAR_BIT];
};


// Jump to a new location in the machine (not next).
struct re_jump : public re_syntax_base {
   offset_type     alt;                 // location to jump to
};


// Jump to a new location in the machine (possibly next).

struct re_alt : public re_jump {
   unsigned char   _map[1 << CHAR_BIT]; // which characters can take the jump
   unsigned int    can_be_null;         // true if we match a NULL string
};


// Repeat a section of the machine
struct re_repeat : public re_alt {
   std::size_t   min, max;        // min and max allowable repeats
   int           state_id;        // Unique identifier for this repeat
   bool          leading;         // True if this repeat is at the start of the machine (lets us optimize some searches)
   bool          greedy;          // True if this is a greedy repeat
};


// Recurse to a particular subexpression.
struct re_recurse : public re_jump {
   int state_id;             // identifier of first nested repeat within the recursion.
};


// Used for the PRUNE, SKIP and COMMIT verbs which basically differ only in what happens
enum commit_type {
   commit_prune,
   commit_skip,
   commit_commit
};

struct re_commit : public re_syntax_base {
   commit_type action;
};


// Provides compiled size of re_jump structure (allowing for trailing alignment).
// We provide this so we know how manybytes to insert when constructing the machine
// (The value of padding_mask is defined in regex_raw_buffer.hpp).
enum re_jump_size_type {
   re_jump_size     = (sizeof(re_jump)   + padding_mask) & ~(padding_mask),
   re_repeater_size = (sizeof(re_repeat) + padding_mask) & ~(padding_mask),
   re_alt_size      = (sizeof(re_alt)    + padding_mask) & ~(padding_mask)
};


// Forward declaration: we'll need this one later
template <class charT, class traits>
struct regex_data;

template <class iterator, class charT, class traits_type, class char_classT>
iterator re_is_set_member(iterator next, iterator last, const re_set_long<char_classT> *set_,
                  const regex_data<charT, traits_type> &e, bool icase);

} // namespace

} // namespace


#endif


