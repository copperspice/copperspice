/***********************************************************************
*
* Copyright (c) 2017-2024 Barbara Geller
* Copyright (c) 2017-2024 Ansel Sermersheim
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

#ifndef CS_ITERATOR_CATEGORY_H
#define CS_ITERATOR_CATEGORY_H

#include <iterator>

namespace cs_regex_ns {

namespace detail {

template <class I>
struct is_random_imp {
 private:
   using cat = typename std::iterator_traits<I>::iterator_category;

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
      using type = is_random_imp<I>;
   };
};

template <>
struct is_random_imp_selector<true> {

   template <class I>
   struct rebind {
      using type = is_random_pointer_imp<I>;
   };

};

}   // end namespace

template <class I>
struct is_random_access_iterator {

 private:
   using  selector  = detail::is_random_imp_selector< std::is_pointer<I>::value>;
   using bound_type = typename selector::template rebind<I>;
   using answer     = typename bound_type::type ;

 public:
   static const bool value = answer::value;
};

template <class I>
const bool is_random_access_iterator<I>::value;

}   // end namespace

#endif

