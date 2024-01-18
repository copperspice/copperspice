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

#ifndef CS_PROTECTED_CALL_H
#define CS_PROTECTED_CALL_H

namespace cs_regex_ns {

namespace cs_regex_detail_ns {

class LIB_CS_STRING_EXPORT abstract_protected_call
{
 public:
   bool execute() const;

   virtual ~abstract_protected_call() {}

 private:
   virtual bool call()const = 0;
};

template <class T>
class concrete_protected_call : public abstract_protected_call
{
 public:
   using  proc_type = bool (T::*)();

   concrete_protected_call(T *o, proc_type p)
      : obj(o), proc(p) {}

 private:
   virtual bool call()const;

   T *obj;
   proc_type proc;
};

template <class T>
bool concrete_protected_call<T>::call()const
{
   return (obj->*proc)();
}

}   // end namespace

}   // end namespace

#endif
