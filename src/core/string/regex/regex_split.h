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

#ifndef CS_REGEX_SPLIT_H
#define CS_REGEX_SPLIT_H

namespace cs_regex_ns {

namespace cs_regex_detail_ns {

template <class charT, class traits>
const basic_regex<charT, traits> &get_default_expression(charT)
{
   static const charT expression_text[4] = { '\\', 's', '+', '\00', };
   static const basic_regex<charT, traits> e(expression_text);

   return e;
}

template <class OutputIterator, class charT, class traits, class allocator>
class split_pred
{
 private:
   typedef std::basic_string<charT, traits, allocator> string_type;
   typedef typename string_type::const_iterator        iterator_type;

   iterator_type  *p_last;
   OutputIterator *p_out;
   std::size_t    *p_max;
   std::size_t    initial_max;

 public:
   split_pred(iterator_type *a, OutputIterator *b, std::size_t *c)
      : p_last(a), p_out(b), p_max(c), initial_max(*c) {}

   bool operator()(const match_results<iterator_type> &what);
};

template <class OutputIterator, class charT, class traits, class Allocator>
bool split_pred<OutputIterator, charT, traits, Allocator>::operator() (const match_results<iterator_type> &what)
{
   *p_last = what[0].second;

   if (what.size() > 1) {
      // output sub-expressions only:
      for (unsigned i = 1; i < what.size(); ++i) {
         *(*p_out) = what.str(i);
         ++(*p_out);
         if (0 == --*p_max) {
            return false;
         }
      }
      return *p_max != 0;

   } else {
      // output $` only if it's not-null or not at the start of the input:
      const sub_match<iterator_type> &sub = what[-1];
      if ((sub.first != sub.second) || (*p_max != initial_max)) {
         *(*p_out) = sub.str();
         ++(*p_out);
         return --*p_max;
      }
   }

   // initial null, do nothing:
   return true;
}

} // namespace

template <class OutputIterator, class charT, class traits, class allocator, class Traits2>
std::size_t regex_split(OutputIterator out, std::basic_string<charT, traits, allocator> &s,
                  const basic_regex<charT, Traits2> &e, match_flag_type flags, std::size_t max_split)
{
   typedef typename std::basic_string<charT, traits, allocator>::const_iterator  ci_t;

   ci_t last = s.begin();
   std::size_t init_size = max_split;
   cs_regex_detail_ns::split_pred<OutputIterator, charT, traits, allocator> pred(&last, &out, &max_split);
   ci_t i, j;
   i = s.begin();
   j = s.end();
   regex_grep(pred, i, j, e, flags);

   // if there is still input left, do a final push as long as max_split
   // is not exhausted, and we're not splitting sub-expressions rather than whitespace:

   if (max_split && (last != s.end()) && (e.mark_count() == 0)) {
      *out = std::basic_string<charT, traits, allocator>((ci_t)last, (ci_t)s.end());
      ++out;
      last = s.end();
      --max_split;
   }

   // delete from the string everything that has been processed so far:
   s.erase(0, last - s.begin());

   // return the number of new records pushed:
   return init_size - max_split;
}

template <class OutputIterator, class charT, class traits, class allocator, class Traits2>
inline std::size_t regex_split(OutputIterator out, std::basic_string<charT, traits, allocator> &s,
                  const basic_regex<charT, Traits2> &e, match_flag_type flags = match_default)
{
   return regex_split(out, s, e, flags, UINT_MAX);
}

template <class OutputIterator, class charT, class traits, class allocator>
inline std::size_t regex_split(OutputIterator out, std::basic_string<charT, traits, allocator> &s)
{
   return regex_split(out, s, cs_regex_detail_ns::get_default_expression(charT(0)), match_default, UINT_MAX);
}

} // namespace

#endif


