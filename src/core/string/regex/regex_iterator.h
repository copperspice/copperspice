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

#ifndef CS_REGEX_ITERATOR_H
#define CS_REGEX_ITERATOR_H

namespace cs_regex_ns {

template <class BidirectionalIterator, class charT, class traits>
class regex_iterator_implementation
{
   typedef basic_regex<charT, traits> regex_type;

   match_results<BidirectionalIterator> what;  // current match
   BidirectionalIterator                base;  // start of sequence
   BidirectionalIterator                end;   // end of sequence
   const regex_type                     re;    // the expression
   match_flag_type                      flags; // flags for matching

 public:
   regex_iterator_implementation(const regex_type *p, BidirectionalIterator last, match_flag_type f)
      : base(), end(last), re(*p), flags(f) {}

   bool init(BidirectionalIterator first) {
      base = first;
      return regex_search(first, end, what, re, flags);
   }

   bool compare(const regex_iterator_implementation &that) {
      if (this == &that) {
         return true;
      }

      return (&re.get_data() == &that.re.get_data()) && (end == that.end) && (flags == that.flags) && (what[0].first == that.what[0].first) &&
             (what[0].second == that.what[0].second);
   }

   const match_results<BidirectionalIterator> &get() {
      return what;
   }

   bool next() {

      BidirectionalIterator next_start = what[0].second;
      match_flag_type f(flags);

      if (!what.length() || (f & regex_constants::match_posix)) {
         f |= regex_constants::match_not_initial_null;
      }

      bool result = regex_search(next_start, end, what, re, f, base);
      if (result) {
         what.set_base(base);
      }
      return result;
   }

 private:
   regex_iterator_implementation &operator=(const regex_iterator_implementation &);
};

template <class BidirectionalIterator, class charT, class traits>
class regex_iterator
   : public std::iterator <std::forward_iterator_tag, match_results<BidirectionalIterator>,
     typename cs_regex_detail_ns::regex_iterator_traits<BidirectionalIterator>::difference_type,
     const match_results<BidirectionalIterator> *, const match_results<BidirectionalIterator> & >
{
 private:
   typedef regex_iterator_implementation<BidirectionalIterator, charT, traits> impl;
   typedef std::shared_ptr<impl> pimpl;

 public:
   typedef          basic_regex<charT, traits>                              regex_type;
   typedef          match_results<BidirectionalIterator>                    value_type;
   typedef          const value_type                                        *pointer;
   typedef          const value_type                                        &reference;
   typedef          std::forward_iterator_tag                               iterator_category;

   typedef typename cs_regex_detail_ns::regex_iterator_traits<BidirectionalIterator>::difference_type difference_type;

   regex_iterator() {}
   regex_iterator(BidirectionalIterator a, BidirectionalIterator b,
                  const regex_type &re,
                  match_flag_type m = match_default)
      : pdata(new impl(&re, b, m)) {
      if (!pdata->init(a)) {
         pdata.reset();
      }
   }

   regex_iterator(const regex_iterator &that)
      : pdata(that.pdata)
   {}

   regex_iterator &operator=(const regex_iterator &that) {
      pdata = that.pdata;
      return *this;
   }
   bool operator==(const regex_iterator &that)const {
      if ((pdata.get() == 0) || (that.pdata.get() == 0)) {
         return pdata.get() == that.pdata.get();
      }
      return pdata->compare(*(that.pdata.get()));
   }

   bool operator!=(const regex_iterator &that)const {
      return !(*this == that);
   }

   const value_type &operator*()const {
      return pdata->get();
   }

   const value_type *operator->()const {
      return &(pdata->get());
   }
   regex_iterator &operator++() {
      cow();
      if (0 == pdata->next()) {
         pdata.reset();
      }
      return *this;
   }
   regex_iterator operator++(int) {
      regex_iterator result(*this);
      ++(*this);
      return result;
   }

 private:
   pimpl pdata;

   void cow() {
      // copy-on-write
      if (pdata.get() && ! pdata.unique()) {
         pdata.reset(new impl(*(pdata.get())));
      }
   }
};

} // namespace

#endif

