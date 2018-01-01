/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QTCONCURRENTRUN_H
#define QTCONCURRENTRUN_H

#include <qglobal.h>
#include <qtconcurrentrunbase.h>
#include <qtconcurrentstoredfunctioncall.h>

QT_BEGIN_NAMESPACE

namespace QtConcurrent {

template <typename T>
QFuture<T> run(T (*functionPointer)())
{
   return (new StoredFunctorCall0<T, T (*)()>(functionPointer))->start();
}

template <typename T, typename Param1, typename Arg1>
QFuture<T> run(T (*functionPointer)(Param1), const Arg1 &arg1)
{
   return (new StoredFunctorCall1<T, T (*)(Param1), Arg1>(functionPointer, arg1))->start();
}

template <typename T, typename Param1, typename Arg1, typename Param2, typename Arg2>
QFuture<T> run(T (*functionPointer)(Param1, Param2), const Arg1 &arg1, const Arg2 &arg2)
{
   return (new StoredFunctorCall2<T, T (*)(Param1, Param2), Arg1, Arg2>(functionPointer, arg1, arg2))->start();
}

template <typename T, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3>
QFuture<T> run(T (*functionPointer)(Param1, Param2, Param3), const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3)
{
   return (new StoredFunctorCall3<T, T (*)(Param1, Param2, Param3), Arg1, Arg2, Arg3>(functionPointer, arg1, arg2, arg3))->start();
}

template <typename T, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3, 
      typename Param4, typename Arg4>
QFuture<T> run(T (*functionPointer)(Param1, Param2, Param3, Param4), const Arg1 &arg1, const Arg2 &arg2,
      const Arg3 &arg3, const Arg4 &arg4)
{
   return (new StoredFunctorCall4<T, T (*)(Param1, Param2, Param3, Param4), Arg1, Arg2, Arg3, Arg4>(functionPointer, arg1,
          arg2, arg3, arg4))->start();
}

template <typename T, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3, 
      typename Param4, typename Arg4, typename Param5, typename Arg5>
QFuture<T> run(T (*functionPointer)(Param1, Param2, Param3, Param4, Param5), const Arg1 &arg1, const Arg2 &arg2,
      const Arg3 &arg3, const Arg4 &arg4, const Arg5 &arg5)
{
   return (new StoredFunctorCall5<T, T (*)(Param1, Param2, Param3, Param4, Param5), Arg1, Arg2, Arg3, Arg4, Arg5>
          (functionPointer, arg1, arg2, arg3, arg4, arg5))->start();
}

template <typename Functor>
auto run(Functor functor) 
-> typename std::enable_if< ! QtPrivate::HasResultType<Functor>::Value, QFuture<decltype(functor())> >::type
{
   typedef decltype(functor()) result_type;
   return (new StoredFunctorCall0<result_type, Functor>(functor))->start();
}

template <typename Functor, typename Arg1>
auto run(Functor functor, const Arg1 &arg1)
-> typename std::enable_if< !QtPrivate::HasResultType<Functor>::Value, QFuture<decltype(functor(arg1))> >::type
{
   typedef decltype(functor(arg1)) result_type;
   return (new StoredFunctorCall1<result_type, Functor, Arg1>(functor, arg1))->start();
}

template <typename Functor, typename Arg1, typename Arg2>
auto run(Functor functor, const Arg1 &arg1, const Arg2 &arg2)
-> typename std::enable_if< !QtPrivate::HasResultType<Functor>::Value,QFuture<decltype(functor(arg1, arg2))> >::type 
{
   typedef decltype(functor(arg1, arg2)) result_type;
   return (new StoredFunctorCall2<result_type, Functor, Arg1, Arg2>(functor, arg1, arg2))->start();
}

template <typename Functor, typename Arg1, typename Arg2, typename Arg3>
auto run(Functor functor, const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3)
-> typename std::enable_if< !QtPrivate::HasResultType<Functor>::Value, QFuture<decltype(functor(arg1, arg2, arg3))> >::type 
{
   typedef decltype(functor(arg1, arg2, arg3)) result_type;
   return (new StoredFunctorCall3<result_type, Functor, Arg1, Arg2, Arg3>(functor, arg1, arg2, arg3))->start();
}

template <typename Functor, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
auto run(Functor functor, const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3, const Arg4 &arg4)
-> typename std::enable_if< !QtPrivate::HasResultType<Functor>::Value, QFuture<decltype(functor(arg1, arg2, arg3, arg4))> >::type 
{
   typedef decltype(functor(arg1, arg2, arg3, arg4)) result_type;
   return (new StoredFunctorCall4<result_type, Functor, Arg1, Arg2, Arg3, Arg4>(functor, arg1, arg2, arg3, arg4))->start();
}

template <typename Functor, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
auto run(Functor functor, const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3, const Arg4 &arg4, const Arg5 &arg5)
-> typename std::enable_if< !QtPrivate::HasResultType<Functor>::Value, QFuture<decltype(functor(arg1, arg2, arg3, arg4, arg5))> >::type 
{
   typedef decltype(functor(arg1, arg2, arg3, arg4, arg5)) result_type;
   return (new StoredFunctorCall5<result_type, Functor, Arg1, Arg2, Arg3, Arg4, Arg5>(functor, arg1, arg2, arg3, arg4, arg5))->start();
}

template <typename FunctionObject>
QFuture<typename FunctionObject::result_type> run(FunctionObject functionObject)
{
   return (new StoredFunctorCall0<QT_TYPENAME FunctionObject::result_type, FunctionObject>(functionObject))->start();
}

template <typename FunctionObject, typename Arg1>
QFuture<typename FunctionObject::result_type> run(FunctionObject functionObject, const Arg1 &arg1)
{
   return (new StoredFunctorCall1<QT_TYPENAME FunctionObject::result_type, FunctionObject, Arg1>(functionObject, arg1))->start();
}

template <typename FunctionObject, typename Arg1, typename Arg2>
QFuture<typename FunctionObject::result_type> run(FunctionObject functionObject, const Arg1 &arg1, const Arg2 &arg2)
{
   return (new StoredFunctorCall2<QT_TYPENAME FunctionObject::result_type, FunctionObject, Arg1, Arg2>(functionObject, arg1, arg2))->start();
}

template <typename FunctionObject, typename Arg1, typename Arg2, typename Arg3>
QFuture<typename FunctionObject::result_type> run(FunctionObject functionObject, const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3)
{
   return (new StoredFunctorCall3<QT_TYPENAME FunctionObject::result_type, FunctionObject, Arg1, Arg2, Arg3>
          (functionObject, arg1, arg2, arg3))->start();
}

template <typename FunctionObject, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
QFuture<typename FunctionObject::result_type> run(FunctionObject functionObject, const Arg1 &arg1, const Arg2 &arg2,
      const Arg3 &arg3, const Arg4 &arg4)
{
   return (new StoredFunctorCall4<QT_TYPENAME FunctionObject::result_type, FunctionObject, Arg1, Arg2, Arg3, Arg4>
          (functionObject, arg1, arg2, arg3, arg4))->start();
}

template <typename FunctionObject, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
QFuture<typename FunctionObject::result_type> run(FunctionObject functionObject, const Arg1 &arg1, const Arg2 &arg2,
      const Arg3 &arg3, const Arg4 &arg4, const Arg5 &arg5)
{
   return (new StoredFunctorCall5<QT_TYPENAME FunctionObject::result_type, FunctionObject, Arg1, Arg2, Arg3, Arg4, Arg5>
          (functionObject, arg1, arg2, arg3, arg4, arg5))->start();
}

template <typename FunctionObject>
QFuture<typename FunctionObject::result_type> run(FunctionObject *functionObject)
{
   return (new QT_TYPENAME SelectStoredFunctorPointerCall0<QT_TYPENAME FunctionObject::result_type, FunctionObject>::type(
          functionObject))->start();
}

template <typename FunctionObject, typename Arg1>
QFuture<typename FunctionObject::result_type> run(FunctionObject *functionObject, const Arg1 &arg1)
{
   return (new QT_TYPENAME
          SelectStoredFunctorPointerCall1<QT_TYPENAME FunctionObject::result_type, FunctionObject, Arg1>::type(functionObject, arg1))->start();
}

template <typename FunctionObject, typename Arg1, typename Arg2>
QFuture<typename FunctionObject::result_type> run(FunctionObject *functionObject, const Arg1 &arg1, const Arg2 &arg2)
{
   return (new QT_TYPENAME
          SelectStoredFunctorPointerCall2<QT_TYPENAME FunctionObject::result_type, FunctionObject, Arg1, Arg2>::type(
          functionObject, arg1, arg2))->start();
}

template <typename FunctionObject, typename Arg1, typename Arg2, typename Arg3>
QFuture<typename FunctionObject::result_type> run(FunctionObject *functionObject, const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3)
{
   return (new QT_TYPENAME
          SelectStoredFunctorPointerCall3<QT_TYPENAME FunctionObject::result_type, FunctionObject, Arg1, Arg2, Arg3>::type(
          functionObject, arg1, arg2, arg3))->start();
}

template <typename FunctionObject, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
QFuture<typename FunctionObject::result_type> run(FunctionObject *functionObject, const Arg1 &arg1, const Arg2 &arg2, 
      const Arg3 &arg3, const Arg4 &arg4)
{
   return (new QT_TYPENAME
          SelectStoredFunctorPointerCall4<QT_TYPENAME FunctionObject::result_type, FunctionObject, Arg1, Arg2, Arg3, Arg4>::type(
          functionObject, arg1, arg2, arg3, arg4))->start();
}

template <typename FunctionObject, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
QFuture<typename FunctionObject::result_type> run(FunctionObject *functionObject, const Arg1 &arg1, const Arg2 &arg2,
      const Arg3 &arg3, const Arg4 &arg4, const Arg5 &arg5)
{
   return (new QT_TYPENAME
          SelectStoredFunctorPointerCall5<QT_TYPENAME FunctionObject::result_type, FunctionObject, Arg1, Arg2, Arg3, Arg4, Arg5>::type(
          functionObject, arg1, arg2, arg3, arg4, arg5))->start();
}

template <typename T, typename Class>
QFuture<T> run(const Class &object, T (Class::*fn)())
{
   return (new QT_TYPENAME SelectStoredMemberFunctionCall0<T, Class>::type(fn, object))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1>
QFuture<T> run(const Class &object, T (Class::*fn)(Param1), const Arg1 &arg1)
{
   return (new QT_TYPENAME SelectStoredMemberFunctionCall1<T, Class, Param1, Arg1>::type(fn, object, arg1))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2>
QFuture<T> run(const Class &object, T (Class::*fn)(Param1, Param2), const Arg1 &arg1, const Arg2 &arg2)
{
   return (new QT_TYPENAME SelectStoredMemberFunctionCall2<T, Class, Param1, Arg1, Param2, Arg2>::type(fn, object, arg1, arg2))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3>
QFuture<T> run(const Class &object, T (Class::*fn)(Param1, Param2, Param3), const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3)
{
   return (new QT_TYPENAME SelectStoredMemberFunctionCall3<T, Class, Param1, Arg1, Param2, Arg2, Param3, Arg3>::type(fn,
      object, arg1, arg2, arg3))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2, 
      typename Param3, typename Arg3, typename Param4, typename Arg4>
QFuture<T> run(const Class &object, T (Class::*fn)(Param1, Param2, Param3, Param4), const Arg1 &arg1, const Arg2 &arg2,
      const Arg3 &arg3, const Arg4 &arg4)
{
   return (new QT_TYPENAME
      SelectStoredMemberFunctionCall4<T, Class, Param1, Arg1, Param2, Arg2, Param3, Arg3, Param4, Arg4>::type(fn, object,
      arg1, arg2, arg3, arg4))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3, 
      typename Param4, typename Arg4, typename Param5, typename Arg5>
QFuture<T> run(const Class &object, T (Class::*fn)(Param1, Param2, Param3, Param4, Param5), const Arg1 &arg1,
   const Arg2 &arg2, const Arg3 &arg3, const Arg4 &arg4, const Arg5 &arg5)
{
   return (new QT_TYPENAME
          SelectStoredMemberFunctionCall5<T, Class, Param1, Arg1, Param2, Arg2, Param3, Arg3, Param4, Arg4, Param5, Arg5>::type(
          fn, object, arg1, arg2, arg3, arg4, arg5))->start();
}

template <typename T, typename Class>
QFuture<T> run(const Class &object, T (Class::*fn)() const)
{
   return (new QT_TYPENAME SelectStoredConstMemberFunctionCall0<T, Class>::type(fn, object))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1>
QFuture<T> run(const Class &object, T (Class::*fn)(Param1) const, const Arg1 &arg1)
{
   return (new QT_TYPENAME SelectStoredConstMemberFunctionCall1<T, Class, Param1, Arg1>::type(fn, object, arg1))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2>
QFuture<T> run(const Class &object, T (Class::*fn)(Param1, Param2) const, const Arg1 &arg1, const Arg2 &arg2)
{
   return (new QT_TYPENAME SelectStoredConstMemberFunctionCall2<T, Class, Param1, Arg1, Param2, Arg2>::type(fn, object, arg1, arg2))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3>
QFuture<T> run(const Class &object, T (Class::*fn)(Param1, Param2, Param3) const, const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3)
{
   return (new QT_TYPENAME SelectStoredConstMemberFunctionCall3<T, Class, Param1, Arg1, Param2, Arg2, Param3, Arg3>::type(
      fn, object, arg1, arg2, arg3))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3, 
      typename Param4, typename Arg4>
QFuture<T> run(const Class &object, T (Class::*fn)(Param1, Param2, Param3, Param4) const, const Arg1 &arg1,
               const Arg2 &arg2, const Arg3 &arg3, const Arg4 &arg4)
{
   return (new QT_TYPENAME
          SelectStoredConstMemberFunctionCall4<T, Class, Param1, Arg1, Param2, Arg2, Param3, Arg3, Param4, Arg4>::type(fn, object,
          arg1, arg2, arg3, arg4))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3,
      typename Param4, typename Arg4, typename Param5, typename Arg5>
QFuture<T> run(const Class &object, T (Class::*fn)(Param1, Param2, Param3, Param4, Param5) const, const Arg1 &arg1,
      const Arg2 &arg2, const Arg3 &arg3, const Arg4 &arg4, const Arg5 &arg5)
{
   return (new QT_TYPENAME
          SelectStoredConstMemberFunctionCall5<T, Class, Param1, Arg1, Param2, Arg2, Param3, Arg3, Param4, Arg4, Param5, Arg5>::type(
          fn, object, arg1, arg2, arg3, arg4, arg5))->start();
}

template <typename T, typename Class>
QFuture<T> run(Class *object, T (Class::*fn)())
{
   return (new QT_TYPENAME SelectStoredMemberFunctionPointerCall0<T, Class>::type(fn, object))->start();
}


template <typename T, typename Class, typename Param1, typename Arg1>
QFuture<T> run(Class *object, T (Class::*fn)(Param1), const Arg1 &arg1)
{
   return (new QT_TYPENAME SelectStoredMemberFunctionPointerCall1<T, Class, Param1, Arg1>::type(fn, object,
           arg1))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2>
QFuture<T> run(Class *object, T (Class::*fn)(Param1, Param2), const Arg1 &arg1, const Arg2 &arg2)
{
   return (new QT_TYPENAME SelectStoredMemberFunctionPointerCall2<T, Class, Param1, Arg1, Param2, Arg2>::type(fn, object,
           arg1, arg2))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3>
QFuture<T> run(Class *object, T (Class::*fn)(Param1, Param2, Param3), const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3)
{
   return (new QT_TYPENAME
           SelectStoredMemberFunctionPointerCall3<T, Class, Param1, Arg1, Param2, Arg2, Param3, Arg3>::type(fn, object, arg1, arg2,
           arg3))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3, 
      typename Param4, typename Arg4>
QFuture<T> run(Class *object, T (Class::*fn)(Param1, Param2, Param3, Param4), const Arg1 &arg1, const Arg2 &arg2, 
      const Arg3 &arg3, const Arg4 &arg4)
{
   return (new QT_TYPENAME
           SelectStoredMemberFunctionPointerCall4<T, Class, Param1, Arg1, Param2, Arg2, Param3, Arg3, Param4, Arg4>::type(fn,
           object, arg1, arg2, arg3, arg4))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3, 
      typename Param4, typename Arg4, typename Param5, typename Arg5>
QFuture<T> run(Class *object, T (Class::*fn)(Param1, Param2, Param3, Param4, Param5), const Arg1 &arg1,
      const Arg2 &arg2, const Arg3 &arg3, const Arg4 &arg4, const Arg5 &arg5)
{
   return (new QT_TYPENAME
          SelectStoredMemberFunctionPointerCall5<T, Class, Param1, Arg1, Param2, Arg2, Param3, Arg3, Param4, Arg4, Param5, Arg5>::type(
          fn, object, arg1, arg2, arg3, arg4, arg5))->start();
}

template <typename T, typename Class>
QFuture<T> run(const Class *object, T (Class::*fn)() const)
{
   return (new QT_TYPENAME SelectStoredConstMemberFunctionPointerCall0<T, Class>::type(fn, object))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1>
QFuture<T> run(const Class *object, T (Class::*fn)(Param1) const, const Arg1 &arg1)
{
   return (new QT_TYPENAME SelectStoredConstMemberFunctionPointerCall1<T, Class, Param1, Arg1>::type(fn, object,
           arg1))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2>
QFuture<T> run(const Class *object, T (Class::*fn)(Param1, Param2) const, const Arg1 &arg1, const Arg2 &arg2)
{
   return (new QT_TYPENAME SelectStoredConstMemberFunctionPointerCall2<T, Class, Param1, Arg1, Param2, Arg2>::type(fn,
           object, arg1, arg2))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3>
QFuture<T> run(const Class *object, T (Class::*fn)(Param1, Param2, Param3) const, const Arg1 &arg1, const Arg2 &arg2,
               const Arg3 &arg3)
{
   return (new QT_TYPENAME
           SelectStoredConstMemberFunctionPointerCall3<T, Class, Param1, Arg1, Param2, Arg2, Param3, Arg3>::type(fn, object, arg1,
                 arg2, arg3))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3, 
      typename Param4, typename Arg4>
QFuture<T> run(const Class *object, T (Class::*fn)(Param1, Param2, Param3, Param4) const, const Arg1 &arg1, const Arg2 &arg2, 
      const Arg3 &arg3, const Arg4 &arg4)
{
   return (new QT_TYPENAME
          SelectStoredConstMemberFunctionPointerCall4<T, Class, Param1, Arg1, Param2, Arg2, Param3, Arg3, Param4, Arg4>::type(fn,
          object, arg1, arg2, arg3, arg4))->start();
}

template <typename T, typename Class, typename Param1, typename Arg1, typename Param2, typename Arg2, typename Param3, typename Arg3, 
      typename Param4, typename Arg4, typename Param5, typename Arg5>
QFuture<T> run(const Class *object, T (Class::*fn)(Param1, Param2, Param3, Param4, Param5) const, const Arg1 &arg1,
               const Arg2 &arg2, const Arg3 &arg3, const Arg4 &arg4, const Arg5 &arg5)
{
   return (new QT_TYPENAME
          SelectStoredConstMemberFunctionPointerCall5<T, Class, Param1, Arg1, Param2, Arg2, Param3, Arg3, Param4, Arg4, Param5, Arg5>::type(
          fn, object, arg1, arg2, arg3, arg4, arg5))->start();
}

} //namespace QtConcurrent

QT_END_NAMESPACE

#endif
