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

#ifndef CS_ITERATOR_CATEGORY_H
#define CS_ITERATOR_CATEGORY_H

#include <iterator>

namespace cs_regex_ns {

namespace detail {

template <class I>
struct is_random_imp {
 private:
   typedef typename std::iterator_traits<I>::iterator_category cat;

 public:
   static const bool value = std::is_convertible<cat *, std::random_access_iterator_tag *>::value;
};

template <class I>
struct is_random_pointer_imp {
   static const bool value = true;
};

template <bool is_pointer_type>
struct is_random_imp_selector {

   template <class I>
   struct rebind {
      typedef is_random_imp<I> type;
   };
};

template <>
struct is_random_imp_selector<true> {

   template <class I>
   struct rebind {
      typedef is_random_pointer_imp<I> type;
   };

};

}

template <class I>
struct is_random_access_iterator {

 private:
   typedef detail::is_random_imp_selector< std::is_pointer<I>::value> selector;
   typedef typename selector::template rebind<I> bound_type;
   typedef typename bound_type::type answer;

 public:
   static const bool value = answer::value;
};

template <class I>
const bool is_random_access_iterator<I>::value;

}

#endif

