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

#ifndef CS_MATCH_RESULTS_H
#define CS_MATCH_RESULTS_H

#include <cassert>

#include "regex/regex_sub_match.h"

namespace cs_regex_ns {

namespace cs_regex_detail_ns {

template <typename Traits>
class named_subexpressions;

}

template <typename Traits, typename Allocator = std::allocator<sub_match<typename Traits::string_type::const_iterator>>>
class match_results
{
 public:
   using string_type        = typename Traits::string_type;
   using char_type          = typename Traits::char_type;

   using string_iterator    = typename Traits::string_type::const_iterator;

   using capture_names_type = cs_regex_detail_ns::named_subexpressions<Traits>;

   using vector_type        = std::vector<sub_match<string_iterator>, Allocator>;
   using value_type         = typename vector_type::value_type;
   using size_type          = typename vector_type::size_type;
   using difference_type    = typename vector_type::difference_type;

   using reference          = value_type &;
   using const_reference    = const value_type &;

   using iterator           = typename vector_type::iterator;
   using const_iterator     = typename vector_type::const_iterator;

   explicit match_results(const Allocator &a = Allocator())
      : m_capture_list(a), m_base(), m_null(), m_last_closed_paren(0), m_is_singular(true)
   {}

   match_results(const match_results &m)
      : m_capture_list(m.m_capture_list), m_capture_names(m.m_capture_names),
        m_last_closed_paren(m.m_last_closed_paren), m_is_singular(m.m_is_singular) {

      if (! m_is_singular) {
         m_base = m.m_base;
         m_null = m.m_null;
      }
   }

   match_results &operator=(const match_results &m) {
      m_capture_list      = m.m_capture_list;
      m_capture_names     = m.m_capture_names;
      m_last_closed_paren = m.m_last_closed_paren;
      m_is_singular       = m.m_is_singular;

      if (! m_is_singular) {
         m_base = m.m_base;
         m_null = m.m_null;
      }

      return *this;
   }

   ~match_results() {}

   size_type size() const {
      return empty() ? 0 : m_capture_list.size() - 2;
   }

   size_type max_size() const {
      return m_capture_list.max_size();
   }

   bool empty() const {
      return m_capture_list.size() < 2;
   }

   difference_type length(size_type index = 0) const {
      if (m_is_singular) {
         raise_logic_error();
      }

      index += 2;

      if ((index < m_capture_list.size()) && (index > 0)) {
         return m_capture_list[index].length();
      }

      return 0;
   }

   difference_type length(const char_type *capture_name) const {
      if (m_is_singular) {
         raise_logic_error();
      }

      const char_type *end = capture_name;

      while (*end) {
         ++end;
      }

      return length(named_subexpression_index(capture_name, end));
   }

   difference_type length(const string_type &capture_name) const {
      return length(named_subexpression_index(capture_name.begin(), capture_name.end()));
   }

   difference_type position(size_type index = 0) const {
      if (m_is_singular) {
         raise_logic_error();
      }

      index += 2;

      if (index < m_capture_list.size()) {
         const sub_match<string_iterator> &s = m_capture_list[index];

         if (s.matched || (index == 2)) {
            return std::distance(m_base, s.first);
         }
      }

      return ~static_cast<difference_type>(0);
   }

   difference_type position(const char_type *capture_name) const {
      const char_type *end = capture_name;

      while (*end) {
         ++end;
      }

      return position(named_subexpression_index(capture_name, end));
   }

   difference_type position(const string_type &capture_name) const {
      return position(named_subexpression_index(capture_name.begin(), capture_name.end()));
   }

   string_type str(size_type index = 0) const {
      if (m_is_singular) {
         raise_logic_error();
      }

      index += 2;
      string_type result;

      if (index < m_capture_list.size() && (index > 0)) {
         const sub_match<string_iterator> &s = m_capture_list[index];

         if (s.matched) {
            result = s.str();
         }
      }

      return result;
   }

   string_type str(const char_type *capture_name) const {
      return (*this)[capture_name].str();
   }

   string_type str(const string_type &capture_name) const {
      return (*this)[capture_name].str();
   }

   const_reference named_subexpression(const char_type *i, const char_type *j) const {
      // scan for the leftmost *matched* subexpression with the specified named:

      if (m_is_singular) {
         raise_logic_error();
      }

      typename cs_regex_detail_ns::named_subexpressions<Traits>::range_type r = m_capture_names->equal_range(i, j);

      while ((r.first != r.second) && ((*this)[r.first->index].matched == false)) {
         ++r.first;
      }

      return r.first != r.second ? (*this)[r.first->index] : m_null;
   }

   const_reference named_subexpression(string_iterator first, string_iterator last) const {

      if (first == last) {
         return m_null;
      }

      std::vector<char_type> s;

      while (first != last) {
         s.insert(s.end(), *first);
         ++first;
      }

      return named_subexpression(&*s.begin(), &*s.begin() + s.size());
   }

   size_type internal_named_subexpression_index(string_iterator i, string_iterator j) const {

      // Scan for the leftmost *matched* subexpression with the specified named.
      // If none found then return the leftmost expression with that name, otherwise an invalid index

      if (m_is_singular) {
         raise_logic_error();
      }

      typename cs_regex_detail_ns::named_subexpressions<Traits>::range_type s;
      typename cs_regex_detail_ns::named_subexpressions<Traits>::range_type r;

      r = m_capture_names->equal_range(i, j);
      s = r;

      while ((r.first != r.second) && ((*this)[r.first->index].matched == false)) {
         ++r.first;
      }

      if (r.first == r.second) {
         r = s;
      }

      return r.first != r.second ? r.first->index : -20;
   }

   size_type named_subexpression_index(string_iterator first, string_iterator last) const {

      if (first == last) {
         return -1;
      }

      return internal_named_subexpression_index(first, last);
   }

   const_reference operator[](size_type index) const {
      if (m_is_singular && m_capture_list.empty()) {
         raise_logic_error();
      }

      index += 2;

      if (index < m_capture_list.size() && (index >= 0)) {
         return m_capture_list[index];
      }

      return m_null;
   }

   const_reference operator[](const string_type &capture_name) const {
      return named_subexpression(capture_name.begin(), capture_name.end());
   }

/*
   const_reference operator[](const char_type *capture_name) const {
      const char_type *end = capture_name;

      while (*end) {
         ++end;
      }

      return named_subexpression(capture_name, end);
   }
*/

   const_reference prefix() const {
      if (m_is_singular) {
         raise_logic_error();
      }

      return (*this)[-1];
   }

   const_reference suffix() const {
      if (m_is_singular) {
         raise_logic_error();
      }

      return (*this)[-2];
   }

   const_iterator begin() const {
      return (m_capture_list.size() > 2) ? (m_capture_list.begin() + 2) : m_capture_list.end();
   }

   const_iterator end() const {
      return m_capture_list.end();
   }

   const_reference get_last_closed_paren()const {
      if (m_is_singular) {
         raise_logic_error();
      }

      return m_last_closed_paren == 0 ? m_null : (*this)[m_last_closed_paren];
   }

   Allocator get_allocator() const {
      return m_capture_list.get_allocator();
   }

   void swap(match_results &that) {
      std::swap(m_capture_list, that.m_capture_list);
      std::swap(m_capture_names, that.m_capture_names);
      std::swap(m_last_closed_paren, that.m_last_closed_paren);

      if (m_is_singular) {
         if (! that.m_is_singular) {
            m_base = that.m_base;
            m_null = that.m_null;
         }

      } else if (that.m_is_singular) {
         that.m_base = m_base;
         that.m_null = m_null;

      } else {
         std::swap(m_base, that.m_base);
         std::swap(m_null, that.m_null);
      }

      std::swap(m_is_singular, that.m_is_singular);
   }

   bool operator==(const match_results &that)const {
      if (m_is_singular) {
         return that.m_is_singular;
      } else if (that.m_is_singular) {
         return false;
      }
      return (m_capture_list == that.m_capture_list) && (m_base == that.m_base) && (m_last_closed_paren == that.m_last_closed_paren);
   }

   bool operator!=(const match_results &that)const {
      return !(*this == that);
   }

   // private access functions
   void set_second(string_iterator i) {
      assert(m_capture_list.size() > 2);

      m_capture_list[2].second  = i;
      m_capture_list[2].matched = true;
      m_capture_list[0].first   = i;
      m_capture_list[0].matched = (m_capture_list[0].first != m_capture_list[0].second);

      m_null.first   = i;
      m_null.second  = i;
      m_null.matched = false;
      m_is_singular = false;
   }

   void set_second(string_iterator i, size_type pos, bool m = true, bool escape_k = false) {
      if (pos) {
         m_last_closed_paren = static_cast<int>(pos);
      }

      pos += 2;
      assert(m_capture_list.size() > pos);
      m_capture_list[pos].second = i;
      m_capture_list[pos].matched = m;

      if ((pos == 2) && !escape_k) {
         m_capture_list[0].first = i;
         m_capture_list[0].matched = (m_capture_list[0].first != m_capture_list[0].second);
         m_null.first = i;
         m_null.second = i;
         m_null.matched = false;
         m_is_singular = false;
      }
   }

   void set_size(size_type n, string_iterator i, string_iterator j) {
      value_type v(j);

      size_type len = m_capture_list.size();

      if (len > n + 2) {
         m_capture_list.erase(m_capture_list.begin() + n + 2, m_capture_list.end());
         std::fill(m_capture_list.begin(), m_capture_list.end(), v);

      } else {
         std::fill(m_capture_list.begin(), m_capture_list.end(), v);

         if (n + 2 != len) {
            m_capture_list.insert(m_capture_list.end(), n + 2 - len, v);
         }
      }

      m_capture_list[1].first = i;
      m_last_closed_paren = 0;
   }

   void set_base(string_iterator pos) {
      m_base = pos;
   }

   string_iterator base()const {
      return m_base;
   }

   void set_first(string_iterator i) {
      assert(m_capture_list.size() > 2);

      // set up prefix
      m_capture_list[1].second = i;
      m_capture_list[1].matched = (m_capture_list[1].first != i);

      // set up $0:
      m_capture_list[2].first = i;

      // zero out everything else
      for (size_type n = 3; n < m_capture_list.size(); ++n) {
         m_capture_list[n].first = m_capture_list[n].second = m_capture_list[0].second;
         m_capture_list[n].matched = false;
      }
   }

   void set_first(string_iterator i, size_type pos, bool escape_k = false) {
      assert(pos + 2 < m_capture_list.size());

      if (pos || escape_k) {
         m_capture_list[pos + 2].first = i;
         if (escape_k) {
            m_capture_list[1].second = i;
            m_capture_list[1].matched = (m_capture_list[1].first != m_capture_list[1].second);
         }

      } else {
         set_first(i);
      }
   }

   void maybe_assign(const match_results<Traits, Allocator> &m);

   void set_named_subs(std::shared_ptr<capture_names_type> capture_name) {
      m_capture_names = capture_name;
   }

 private:
   // error handler called when an uninitialized match_results is accessed:
   static void raise_logic_error() {
      std::logic_error e("Attempt to access an uninitialzed cs_regex_ns::::match_results<> class.");
      throw (e);
   }

   vector_type m_capture_list;                             // list of subexpressions
   std::shared_ptr<capture_names_type> m_capture_names;    // shared copy of named subs in the regex object

   string_iterator m_base;                   // where the search started from
   sub_match<string_iterator> m_null;        // a null match

   int m_last_closed_paren;                  // Last ) to be seen - used for formatting
   bool m_is_singular;                       // True if our stored iterators are singular
};


template <class Traits, class Allocator>
void match_results<Traits, Allocator>::maybe_assign(const match_results<Traits, Allocator> &m)
{
   if (m_is_singular) {
      *this = m;
      return;
   }

   const_iterator p1, p2;
   p1 = begin();
   p2 = m.begin();

   // Distances are measured from the start of *this* match, unless this isn't
   // a valid match in which case we use the start of the whole sequence.  Note that
   // no subsequent match-candidate can ever be to the left of the first match found.
   // This ensures that when we are using bidirectional iterators, that distances
   // measured are as short as possible, and therefore as efficient as possible
   // to compute.  Finally note that we don't use the "matched" data member to test
   // whether a sub-expression is a valid match, because partial matches set this
   // to false for sub-expression 0.

   string_iterator l_end = this->suffix().second;
   string_iterator l_base = (p1->first == l_end) ? this->prefix().first : (*this)[0].first;

   difference_type len1  = 0;
   difference_type len2  = 0;
   difference_type base1 = 0;
   difference_type base2 = 0;

   std::size_t i;

   for (i = 0; i < size(); ++i, ++p1, ++p2) {

      // Leftmost takes priority over longest; handle special cases
      // where distances need not be computed first (an optimisation
      // for bidirectional iterators: ensure that we don't accidently
      // compute the length of the whole sequence, as this can be really  expensive).

      if (p1->first == l_end) {
         if (p2->first != l_end) {
            // p2 must be better than p1, and no need to calculate actual distances
            base1 = 1;
            base2 = 0;
            break;

         } else {
            // *p1 and *p2 are either unmatched or match end-of sequence,
            // either way no need to calculate distances:
            if ((p1->matched == false) && (p2->matched == true)) {
               break;
            }

            if ((p1->matched == true) && (p2->matched == false)) {
               return;
            }
            continue;
         }

      } else if (p2->first == l_end) {
         // p1 better than p2, and no need to calculate distances:
         return;
      }

      base1 = std::distance(l_base, p1->first);
      base2 = std::distance(l_base, p2->first);

      assert(base1 >= 0);
      assert(base2 >= 0);

      if (base1 < base2) {
         return;
      }

      if (base2 < base1) {
         break;
      }

      len1 = std::distance(p1->first, p1->second);
      len2 = std::distance(p2->first, p2->second);

      assert(len1 >= 0);
      assert(len2 >= 0);

      if ((len1 != len2) || ((p1->matched == false) && (p2->matched == true))) {
         break;
      }

      if ((p1->matched == true) && (p2->matched == false)) {
         return;
      }
   }

   if (i == size()) {
      return;
   }

   if (base2 < base1) {
      *this = m;

   } else if ((len2 > len1) || ((p1->matched == false) && (p2->matched == true)) ) {
      *this = m;
   }
}

template <class Traits, class Allocator>
void swap(match_results<Traits, Allocator> &a, match_results<Traits, Allocator> &b)
{
   a.swap(b);
}

template <class charT, class Traits, class RegexTraits, class Allocator>
std::basic_ostream<charT, Traits> & operator<< (std::basic_ostream<charT, Traits> &os, const match_results<RegexTraits, Allocator> &s)
{
   return (os << s.str());
}


} // namespace


#endif


