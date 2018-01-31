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

#ifndef CS_BASIC_REGEX_H
#define CS_BASIC_REGEX_H

#include <cassert>
#include <functional>

namespace cs_regex_ns {

namespace cs_regex_detail_ns {

template <class charT, class traits>
class basic_regex_parser;

template <class I>
void bubble_down_one(I first, I last)
{
   if (first != last) {
      I next = last - 1;

      while ((next != first) && (*next < * (next - 1))) {
         (next - 1)->swap(*next);
         --next;
      }
   }
}

template <class Iterator>
inline int hash_value_from_capture_name(Iterator i, Iterator j)
{
   std::size_t retval = 0;

   for (auto iter = i; iter != j; ++iter) {
      retval = retval ^ (std::hash<typename std::remove_cv<typename std::remove_reference<decltype(*i)>::type>::type> {}(*iter));
   }

   return static_cast<int>(retval);
}

class named_subexpressions
{
 public:
   struct name {

      template <class Iter>
      name(const Iter i, const Iter j, int idx)
         : index(idx) {
         hash = hash_value_from_capture_name(i, j);
      }

      name(int h, int idx)
         : index(idx), hash(h)
      {  }

      int index;
      int hash;

      bool operator < (const name &other) const {
         return hash < other.hash;
      }

      bool operator == (const name &other) const {
         return hash == other.hash;
      }

      void swap(name &other) {
         std::swap(index, other.index);
         std::swap(hash, other.hash);
      }
   };

   using const_iterator = std::vector<name>::const_iterator;
   using range_type     = std::pair<const_iterator, const_iterator>;

   named_subexpressions() {}

   template <class Iter>
   void set_name(const Iter i, const Iter j, int index) {
      m_sub_names.push_back(name(i, j, index));
      bubble_down_one(m_sub_names.begin(), m_sub_names.end());
   }

   template <class Iter>
   int get_id(const Iter i, const Iter j) const {
      name t(i, j, 0);
      auto pos = std::lower_bound(m_sub_names.begin(), m_sub_names.end(), t);

      if ((pos != m_sub_names.end()) && (*pos == t)) {
         return pos->index;
      }

      return -1;
   }

   template <class Iter>
   range_type equal_range(const Iter i, const Iter j) const {
      name t(i, j, 0);

      return std::equal_range(m_sub_names.begin(), m_sub_names.end(), t);
   }

   int get_id(int h) const {
      name t(h, 0);

      auto pos = std::lower_bound(m_sub_names.begin(), m_sub_names.end(), t);

      if ((pos != m_sub_names.end()) && (*pos == t)) {
         return pos->index;
      }

      return -1;
   }

   range_type equal_range(int h) const {
      name t(h, 0);
      return std::equal_range(m_sub_names.begin(), m_sub_names.end(), t);
   }

 private:
   std::vector<name> m_sub_names;
};


// represents the data we wish to expose to the matching algorithms

template <class charT, class traits>
struct regex_data : public named_subexpressions {
   using flag_type = regex_constants::syntax_option_type;

   regex_data(const std::shared_ptr< cs_regex_ns::regex_traits_wrapper<traits> > &t)
      : m_ptraits(t), m_expression(0), m_expression_len(0), m_disable_match_any(false)
   {}

   regex_data()
      : m_ptraits(new cs_regex_ns::regex_traits_wrapper<traits>()), m_expression(0),
        m_expression_len(0), m_disable_match_any(false)
   {}

   std::shared_ptr<cs_regex_ns::regex_traits_wrapper<traits>>  m_ptraits;      // traits class instance

   flag_type            m_flags;                      // flags with which we were compiled
   int                  m_status;                     // error code (0 implies OK)
   const charT          *m_expression;                // the original expression
   std::ptrdiff_t       m_expression_len;             // the length of the original expression
   std::size_t          m_mark_count;                 // the number of marked sub-expressions


   unsigned             m_restart_type;               // search optimisation type
   unsigned char        m_startmap[1 << CHAR_BIT];    // which characters can start a match
   unsigned int         m_can_be_null;                // whether we can match a null string

   bool                 m_has_recursions;             // whether we have recursive expressions
   bool                 m_disable_match_any;          // when set we need to disable the match_any flag, causes different/buggy behaviour.

   cs_regex_detail_ns::re_syntax_base  *m_first_state;             // the first state of the machine
   cs_regex_detail_ns::raw_storage      m_data;                    // the buffer in which our states are constructed
   typename traits::char_class_type     m_word_mask;               // mask used to determine if a character is a word character

   std::vector <std::pair <std::size_t, std::size_t > > m_subs;    // position of sub-expressions within the *string*
};

// class basic_regex_implementation
// implementation class for basic_regex

template <class charT, class traits>
class basic_regex_implementation : public regex_data<charT, traits>
{

 public:
   using flag_type         = regex_constants::syntax_option_type;
   using difference_type   = std::ptrdiff_t;
   using size_type         = std::size_t;
   using locale_type       = typename traits::locale_type;
   using const_iterator    = const charT*;

   basic_regex_implementation()
   {}

   basic_regex_implementation(const std::shared_ptr< cs_regex_ns::regex_traits_wrapper<traits> > &t)
      : regex_data<charT, traits>(t)
   {}

   void assign(const typename traits::string_type::const_iterator iter_first,
               const typename traits::string_type::const_iterator iter_last, flag_type f) {

      regex_data<charT, traits> *pdat = this;
      basic_regex_parser<charT, traits> parser(pdat);
      parser.parse(iter_first, iter_last, f);
   }

   locale_type imbue(locale_type l) {
      return this->m_ptraits->imbue(l);
   }

   locale_type getloc() const {
      return this->m_ptraits->getloc();
   }

   std::basic_string<charT> str() const {
      std::basic_string<charT> result;

      if (this->m_status == 0) {
         result = std::basic_string<charT>(this->m_expression, this->m_expression_len);
      }

      return result;
   }

   const_iterator expression() const {
      return this->m_expression;
   }

   std::pair<const_iterator, const_iterator> subexpression(std::size_t n) const {
      const std::pair<std::size_t, std::size_t> &pi = this->m_subs.at(n);
      std::pair<const_iterator, const_iterator> p(expression() + pi.first, expression() + pi.second);
      return p;
   }

   const_iterator begin() const {
      return (this->m_status ? 0 : this->m_expression);
   }

   const_iterator end() const {
      return (this->m_status ? 0 : this->m_expression + this->m_expression_len);
   }

   flag_type flags() const {
      return this->m_flags;
   }

   size_type size() const {
      return this->m_expression_len;
   }

   int status() const {
      return this->m_status;
   }

   size_type mark_count() const {
      return this->m_mark_count - 1;
   }

   const cs_regex_detail_ns::re_syntax_base *get_first_state() const {
      return this->m_first_state;
   }
   unsigned get_restart_type() const {
      return this->m_restart_type;
   }

   const unsigned char *get_map() const {
      return this->m_startmap;
   }

   const cs_regex_ns::regex_traits_wrapper<traits> &get_traits() const {
      return *(this->m_ptraits);
   }

   bool can_be_null() const {
      return this->m_can_be_null;
   }

   const regex_data<charT, traits> &get_data() const {
      basic_regex_implementation<charT, traits> const *p = this;
      return *static_cast<const regex_data<charT, traits>*>(p);
   }
};

} // namespace


template <class charT, class traits>
class basic_regex : public regbase
{
 public:
   using seq_type = typename traits::string_type;

   typedef std::size_t                           traits_size_type;
   typedef charT                                 char_type;
   typedef traits                                traits_type;

   typedef charT                                 value_type;
   typedef charT                                &reference;
   typedef const charT                          &const_reference;
   typedef const charT                          *const_iterator;
   typedef const_iterator                        iterator;
   typedef std::ptrdiff_t                        difference_type;
   typedef std::size_t                           size_type;
   typedef regex_constants::syntax_option_type   flag_type;

   // locale_type placeholder for actual locale type used by the traits class to localise *this
   typedef typename traits::locale_type          locale_type;

   explicit basic_regex()
   {}

   basic_regex(const typename traits::string_type::const_iterator iter_first,
                  const typename traits::string_type::const_iterator iter_last,
                  flag_type f = regex_constants::normal) {

      assign(iter_first, iter_last, f);
   }

   basic_regex(const basic_regex &other) = default;
   basic_regex(basic_regex &&other) = default;

   ~basic_regex()
   {}

   basic_regex &operator=(const basic_regex &other) = default;
   basic_regex &operator=(basic_regex &&other) = default;

   basic_regex &operator=(const typename traits::string_type &str) {
      return assign(str.begin(), str.end());
   }

   basic_regex &assign(const basic_regex &other) {
      return *this = other;
   }

   basic_regex &assign(basic_regex &&other) {
      return *this = std::move(other);
   }

   basic_regex &assign(const typename traits::string_type::const_iterator iter_first,
                       const typename traits::string_type::const_iterator iter_last, flag_type f = regex_constants::normal) {
      return do_assign(iter_first, iter_last, f);
   }

/* BROOM - unused

   template <class ST, class SA>
   unsigned int set_expression(const std::basic_string<charT, ST, SA> &p, flag_type f = regex_constants::normal) {
      return set_expression(p.data(), p.data() + p.size(), f);
   }
*/

   template <class ST, class SA>
   basic_regex &operator=(const std::basic_string<charT, ST, SA> &p) {
      return assign(p.data(), p.data() + p.size(), regex_constants::normal);
   }

   // locale
   locale_type imbue(locale_type l);

   locale_type getloc() const {
      return m_pimpl.get() ? m_pimpl->getloc() : locale_type();
   }

   flag_type flags() const {
      return m_pimpl.get() ? m_pimpl->flags() : 0;
   }

   std::basic_string<charT> str() const {
      return m_pimpl.get() ? m_pimpl->str() : std::basic_string<charT>();
   }

   std::pair<const_iterator, const_iterator> subexpression(std::size_t n) const {
      if (! m_pimpl.get()) {
         throw (std::logic_error("Can not access subexpressions for an invalid regex."));
      }

      return m_pimpl->subexpression(n);
   }

   const_iterator begin() const {
      return (m_pimpl.get() ? m_pimpl->begin() : 0);
   }

   const_iterator end() const {
      return (m_pimpl.get() ? m_pimpl->end() : 0);
   }

   void swap(basic_regex &that) noexcept {
      m_pimpl.swap(that.m_pimpl);
   }

   size_type size() const {
      return (m_pimpl.get() ? m_pimpl->size() : 0);
   }

   size_type max_size() const {
      return UINT_MAX;
   }

   bool empty() const {
      return (m_pimpl.get() ? 0 != m_pimpl->status() : true);
   }

   size_type mark_count() const {
      return (m_pimpl.get() ? m_pimpl->mark_count() : 0);
   }

   int status() const {
      return (m_pimpl.get() ? m_pimpl->status() : regex_constants::error_empty);
   }

   int compare(const basic_regex &that) const {
      if (m_pimpl.get() == that.m_pimpl.get()) {
         return 0;
      }

      if (!m_pimpl.get()) {
         return -1;
      }

      if (! that.m_pimpl.get()) {
         return 1;
      }

      if (status() != that.status()) {
         return status() - that.status();
      }

      if (flags() != that.flags()) {
         return flags() - that.flags();
      }
      return str().compare(that.str());
   }

   bool operator==(const basic_regex &e) const {
      return compare(e) == 0;
   }

   bool operator != (const basic_regex &e) const {
      return compare(e) != 0;
   }

   bool operator<(const basic_regex &e) const {
      return compare(e) < 0;
   }

   bool operator>(const basic_regex &e) const {
      return compare(e) > 0;
   }

   bool operator<=(const basic_regex &e) const {
      return compare(e) <= 0;
   }

   bool operator>=(const basic_regex &e) const {
      return compare(e) >= 0;
   }

   // access methods
   const cs_regex_detail_ns::re_syntax_base *get_first_state() const {
      assert(0 != m_pimpl.get());
      return m_pimpl->get_first_state();
   }

   unsigned get_restart_type() const {
      assert(0 != m_pimpl.get());
      return m_pimpl->get_restart_type();
   }

   const unsigned char *get_map() const {
      assert(0 != m_pimpl.get());
      return m_pimpl->get_map();
   }

   const cs_regex_ns::regex_traits_wrapper<traits> &get_traits() const {
      assert(0 != m_pimpl.get());
      return m_pimpl->get_traits();
   }

   bool can_be_null() const {
      assert(0 != m_pimpl.get());
      return m_pimpl->can_be_null();
   }

   const cs_regex_detail_ns::regex_data<charT, traits> &get_data() const {
      assert(0 != m_pimpl.get());
      return m_pimpl->get_data();
   }

   std::shared_ptr<cs_regex_detail_ns::named_subexpressions > get_named_subs() const {
      return m_pimpl;
   }

 private:
   std::shared_ptr<cs_regex_detail_ns::basic_regex_implementation<charT, traits> > m_pimpl;

   basic_regex<charT, traits> &do_assign(const typename traits::string_type::const_iterator iter_first,
                  const typename traits::string_type::const_iterator iter_last, flag_type f);
};


// out of line members, these are the only members that mutate the basic_regex object,
// and are designed to provide the strong exception guarentee
// (in the event of a throw, the state of the object remains unchanged).

template <class charT, class traits>
basic_regex<charT, traits> &basic_regex<charT, traits>::do_assign(const typename traits::string_type::const_iterator iter_first,
      const typename traits::string_type::const_iterator iter_last, flag_type f)
{
   std::shared_ptr<cs_regex_detail_ns::basic_regex_implementation<charT, traits> > temp;

   if (! m_pimpl.get())   {
      temp = std::shared_ptr<cs_regex_detail_ns::basic_regex_implementation<charT, traits> >(new
             cs_regex_detail_ns::basic_regex_implementation<charT, traits>());
   } else {
      temp = std::shared_ptr<cs_regex_detail_ns::basic_regex_implementation<charT, traits> >(new
             cs_regex_detail_ns::basic_regex_implementation<charT, traits>(m_pimpl->m_ptraits));
   }

   temp->assign(iter_first, iter_last, f);
   temp.swap(m_pimpl);

   return *this;
}

template <class charT, class traits>
typename basic_regex<charT, traits>::locale_type basic_regex<charT, traits>::imbue(locale_type l)
{
   std::shared_ptr<cs_regex_detail_ns::basic_regex_implementation<charT, traits> > temp(new
         cs_regex_detail_ns::basic_regex_implementation<charT, traits>());

   locale_type result = temp->imbue(l);
   temp.swap(m_pimpl);

   return result;
}

template <class charT, class traits>
void swap(basic_regex<charT, traits> &e1, basic_regex<charT, traits> &e2)
{
   e1.swap(e2);
}

template <class charT, class traits, class traits2>
std::basic_ostream<charT, traits> &operator << (std::basic_ostream<charT, traits> &os, const basic_regex<charT, traits2> &e)
{
   return (os << e.str());
}

} // namespace

#endif

