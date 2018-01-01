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

#ifndef QTCONCURRENTFUNCTIONWRAPPERS_H
#define QTCONCURRENTFUNCTIONWRAPPERS_H

#include <QtCore/qglobal.h>
#include <qtconcurrentcompilertest.h>
#include <qstringlist.h>

QT_BEGIN_NAMESPACE

namespace QtConcurrent {

template <typename T>
class FunctionWrapper0
{
 public:
   typedef T (*FunctionPointerType)();
   typedef T result_type;
   inline FunctionWrapper0(FunctionPointerType _functionPointer)
      : functionPointer(_functionPointer) { }

   inline T operator()() {
      return functionPointer();
   }

 private:
   FunctionPointerType functionPointer;
};

template <typename T, typename U>
class FunctionWrapper1
{
 public:
   typedef T (*FunctionPointerType)(U u);
   typedef T result_type;

   inline FunctionWrapper1(FunctionPointerType _functionPointer)
      : functionPointer(_functionPointer) { }

   inline T operator()(U u) {
      return functionPointer(u);
   }

 private:
   FunctionPointerType functionPointer;
};

template <typename T, typename U, typename V>
class FunctionWrapper2
{
 public:
   typedef T (*FunctionPointerType)(U u, V v);
   typedef T result_type;
   inline FunctionWrapper2(FunctionPointerType _functionPointer)
      : functionPointer(_functionPointer) { }

   inline T operator()(U u, V v) {
      return functionPointer(u, v);
   }
 private:
   FunctionPointerType functionPointer;
};

template <typename T, typename C>
class MemberFunctionWrapper
{
 public:
   typedef T (C::*FunctionPointerType)();
   typedef T result_type;
   inline MemberFunctionWrapper(FunctionPointerType _functionPointer)
      : functionPointer(_functionPointer) { }

   inline T operator()(C &c) {
      return (c.*functionPointer)();
   }

 private:
   FunctionPointerType functionPointer;
};

template <typename T, typename C, typename U>
class MemberFunctionWrapper1
{
 public:
   typedef T (C::*FunctionPointerType)(U);
   typedef T result_type;

   inline MemberFunctionWrapper1(FunctionPointerType _functionPointer)
      : functionPointer(_functionPointer) {
   }

   inline T operator()(C &c, U u) {
      return (c.*functionPointer)(u);
   }

 private:
   FunctionPointerType functionPointer;
};

template <typename T, typename C>
class ConstMemberFunctionWrapper
{
 public:
   typedef T (C::*FunctionPointerType)() const;
   typedef T result_type;
   inline ConstMemberFunctionWrapper(FunctionPointerType _functionPointer)
      : functionPointer(_functionPointer) { }

   inline T operator()(const C &c) const {
      return (c.*functionPointer)();
   }

 private:
   FunctionPointerType functionPointer;
};

} // namespace QtConcurrent.

namespace QtPrivate {

template <typename T>
const T &createFunctionWrapper(const T &t)
{
   return t;
}

template <typename T, typename U>
QtConcurrent::FunctionWrapper1<T, U> createFunctionWrapper(T (*func)(U))
{
   return QtConcurrent::FunctionWrapper1<T, U>(func);
}

template <typename T, typename C>
QtConcurrent::MemberFunctionWrapper<T, C> createFunctionWrapper(T (C::*func)())
{
   return QtConcurrent::MemberFunctionWrapper<T, C>(func);
}

template <typename T, typename C, typename U>
QtConcurrent::MemberFunctionWrapper1<T, C, U> createFunctionWrapper(T (C::*func)(U))
{
   return QtConcurrent::MemberFunctionWrapper1<T, C, U>(func);
}

template <typename T, typename C>
QtConcurrent::ConstMemberFunctionWrapper<T, C> createFunctionWrapper(T (C::*func)() const)
{
   return QtConcurrent::ConstMemberFunctionWrapper<T, C>(func);
}

struct PushBackWrapper {
   typedef void result_type;

   template <class C, class U>
   inline void operator()(C &c, const U &u) const {
      return c.push_back(u);
   }

   template <class C, class U>
   inline void operator()(C &c, U &&u) const {
      return c.push_back(u);
   }

};

template <typename Functor, bool foo = HasResultType<Functor>::Value>
struct LazyResultType {
   typedef typename Functor::result_type Type;
};

template <typename Functor>
struct LazyResultType<Functor, false> {
   typedef void Type;
};

template <class T>
struct ReduceResultType;

template <class U, class V>
struct ReduceResultType<void(*)(U &, V)> {
   typedef U ResultType;
};

template <class T, class C, class U>
struct ReduceResultType<T(C::*)(U)> {
   typedef C ResultType;
};

template <class InputSequence, class MapFunctor>
struct MapResultType {
   typedef typename LazyResultType<MapFunctor>::Type ResultType;
};

template <class U, class V>
struct MapResultType<void, U (*)(V)> {
   typedef U ResultType;
};

template <class T, class C>
struct MapResultType<void, T(C::*)() const> {
   typedef T ResultType;
};


template <template <typename> class InputSequence, typename MapFunctor, typename T>
struct MapResultType<InputSequence<T>, MapFunctor> {
   typedef InputSequence<typename LazyResultType<MapFunctor>::Type> ResultType;
};

template <template <typename> class InputSequence, class T, class U, class V>
struct MapResultType<InputSequence<T>, U (*)(V)> {
   typedef InputSequence<U> ResultType;
};

template <template <typename> class InputSequence, class T, class U, class C>
struct MapResultType<InputSequence<T>, U(C::*)() const> {
   typedef InputSequence<U> ResultType;
};

template <class MapFunctor>
struct MapResultType<QStringList, MapFunctor> {
   typedef QList<typename LazyResultType<MapFunctor>::Type> ResultType;
};

template <class U, class V>
struct MapResultType<QStringList, U (*)(V)> {
   typedef QList<U> ResultType;
};

template <class U, class C>
struct MapResultType<QStringList, U(C::*)() const> {
   typedef QList<U> ResultType;
};

} // namespace QtPrivate.


QT_END_NAMESPACE

#endif
