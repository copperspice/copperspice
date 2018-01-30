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

#ifndef CS_OBJECT_CACHE_H
#define CS_OBJECT_CACHE_H

#include <cassert>
#include <map>
#include <list>
#include <stdexcept>
#include <string>

namespace cs_regex_ns {

template <class Key, class Object>
class object_cache
{
 public:
   typedef std::pair< std::shared_ptr<Object const>, Key const *> value_type;
   typedef std::list<value_type> list_type;
   typedef typename list_type::iterator list_iterator;
   typedef std::map<Key, list_iterator> map_type;
   typedef typename map_type::iterator map_iterator;
   typedef typename list_type::size_type size_type;
   static std::shared_ptr<Object const> get(const Key &k, size_type l_max_cache_size);

 private:
   static std::shared_ptr<Object const> do_get(const Key &k, size_type l_max_cache_size);

   struct data {
      list_type   cont;
      map_type    index;
   };

};

template <class Key, class Object>
std::shared_ptr<Object const> object_cache<Key, Object>::get(const Key &k, size_type l_max_cache_size)
{
   static std::mutex mutex;
   std::lock_guard<std::mutex> l(mutex);

   return do_get(k, l_max_cache_size);
}

template <class Key, class Object>
std::shared_ptr<Object const> object_cache<Key, Object>::do_get(const Key &k, size_type l_max_cache_size)
{
   typedef typename object_cache<Key, Object>::data object_data;
   typedef typename map_type::size_type map_size_type;

   static object_data s_data;

   // see if the object is already in the cache:
   map_iterator mpos = s_data.index.find(k);

   if (mpos != s_data.index.end()) {

      // We have a cached item, bump it up the list and return it:

      if (--(s_data.cont.end()) != mpos->second) {
         // splice out the item we want to move:
         list_type temp;
         temp.splice(temp.end(), s_data.cont, mpos->second);
         // and now place it at the end of the list:
         s_data.cont.splice(s_data.cont.end(), temp, temp.begin());

         assert(*(s_data.cont.back().second) == k);
         // update index with new position:

         mpos->second = --(s_data.cont.end());
         assert(&(mpos->first) == mpos->second->second);
         assert(&(mpos->first) == s_data.cont.back().second);
      }
      return s_data.cont.back().first;
   }
   //
   // if we get here then the item is not in the cache,
   // so create it:
   //
   std::shared_ptr<Object const> result(new Object(k));

   //
   // Add it to the list, and index it:
   //
   s_data.cont.push_back(value_type(result, static_cast<Key const *>(0)));
   s_data.index.insert(std::make_pair(k, --(s_data.cont.end())));
   s_data.cont.back().second = &(s_data.index.find(k)->first);
   map_size_type s = s_data.index.size();

   assert(s_data.index[k]->first.get() == result.get());
   assert(&(s_data.index.find(k)->first) == s_data.cont.back().second);
   assert(s_data.index.find(k)->first == k);

   if (s > l_max_cache_size) {
      //
      // We have too many items in the list, so we need to start
      // popping them off the back of the list, but only if they're
      // being held uniquely by us:
      //
      list_iterator pos = s_data.cont.begin();
      list_iterator last = s_data.cont.end();
      while ((pos != last) && (s > l_max_cache_size)) {
         if (pos->first.unique()) {
            list_iterator condemmed(pos);
            ++pos;

            // now remove the items from our containers,
            // then order has to be as follows:
            assert(s_data.index.find(*(condemmed->second)) != s_data.index.end());
            s_data.index.erase(*(condemmed->second));
            s_data.cont.erase(condemmed);
            --s;
         } else {
            ++pos;
         }
      }

      assert(s_data.index[k]->first.get() == result.get());
      assert(&(s_data.index.find(k)->first) == s_data.cont.back().second);
      assert(s_data.index.find(k)->first == k);
   }
   return result;
}

}

#endif
