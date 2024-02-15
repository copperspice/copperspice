/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QTCONCURRENTFUNCTION_WRAPPERS_H
#define QTCONCURRENTFUNCTION_WRAPPERS_H

#include <qglobal.h>
#include <qstringlist.h>
#include <qtconcurrentcompilertest.h>

namespace QtConcurrent {

template <typename T>
class FunctionWrapper0
{
 public:
   using FunctionPointerType = T (*)();
   using result_type         = T;

   FunctionWrapper0(FunctionPointerType _functionPointer)
      : functionPointer(_functionPointer)
   { }

   T operator()() {
      return functionPointer();
   }

 private:
   FunctionPointerType functionPointer;
};

template <typename T, typename U>
class FunctionWrapper1
{
 public:
   using FunctionPointerType = T (*)(U u);
   using result_type         = T;

   FunctionWrapper1(FunctionPointerType _functionPointer)
      : functionPointer(_functionPointer)
   { }

   T operator()(U u) {
      return functionPointer(u);
   }

 private:
   FunctionPointerType functionPointer;
};

template <typename T, typename U, typename V>
class FunctionWrapper2
{
 public:
   using FunctionPointerType = T (*)(U u, V v);
   using result_type         = T;

   FunctionWrapper2(FunctionPointerType _functionPointer)
      : functionPointer(_functionPointer)
   { }

   T operator()(U u, V v) {
      return functionPointer(u, v);
   }

 private:
   FunctionPointerType functionPointer;
};

template <typename T, typename C>
class MemberFunctionWrapper
{
 public:
   using FunctionPointerType = T (C::*)();
   using result_type         = T;

   MemberFunctionWrapper(FunctionPointerType _functionPointer)
      : functionPointer(_functionPointer)
   { }

   T operator()(C &c) {
      return (c.*functionPointer)();
   }

 private:
   FunctionPointerType functionPointer;
};

template <typename T, typename C, typename U>
class MemberFunctionWrapper1
{
 public:
   using FunctionPointerType = T (C::*)(U);
   using result_type         = T;

   MemberFunctionWrapper1(FunctionPointerType _functionPointer)
      : functionPointer(_functionPointer)
   {
   }

   T operator()(C &c, U u) {
      return (c.*functionPointer)(u);
   }

 private:
   FunctionPointerType functionPointer;
};

template <typename T, typename C>
class ConstMemberFunctionWrapper
{
 public:
   using FunctionPointerType = T (C::*)() const;
   using result_type         = T;

   ConstMemberFunctionWrapper(FunctionPointerType _functionPointer)
      : functionPointer(_functionPointer)
   { }

   T operator()(const C &c) const {
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
   using result_type = void;

   template <class C, class U>
   void operator()(C &c, const U &u) const {
      return c.push_back(u);
   }

   template <class C, class U>
   void operator()(C &c, U &&u) const {
      return c.push_back(u);
   }
};

template <typename Functor, bool foo = HasResultType<Functor>::Value>
struct LazyResultType {
   using Type = typename Functor::result_type;
};

template <typename Functor>
struct LazyResultType<Functor, false> {
   using Type = void;
};

template <class T>
struct ReduceResultType;

template <class U, class V>
struct ReduceResultType<void(*)(U &, V)> {
   using ResultType = U;
};

template <class T, class C, class U>
struct ReduceResultType<T(C::*)(U)> {
   using ResultType = C;
};

template <class InputSequence, class MapFunctor>
struct MapResultType {
   using ResultType = typename LazyResultType<MapFunctor>::Type;
};

template <class U, class V>
struct MapResultType<void, U (*)(V)> {
   using ResultType = U;
};

template <class T, class C>
struct MapResultType<void, T(C::*)() const> {
     using ResultType = T;
};

template <template <typename> class InputSequence, typename MapFunctor, typename T>
struct MapResultType<InputSequence<T>, MapFunctor> {
   using ResultType = InputSequence<typename LazyResultType<MapFunctor>::Type>;
};

template <template <typename> class InputSequence, class T, class U, class V>
struct MapResultType<InputSequence<T>, U (*)(V)> {
   using ResultType = InputSequence<U>;
};

template <template <typename> class InputSequence, class T, class U, class C>
struct MapResultType<InputSequence<T>, U(C::*)() const> {
   using ResultType = InputSequence<U>;
};

template <class MapFunctor>
struct MapResultType<QStringList, MapFunctor> {
   using ResultType = QList<typename LazyResultType<MapFunctor>::Type>;
};

template <class U, class V>
struct MapResultType<QStringList, U (*)(V)> {
   using ResultType = QList<U>;
};

template <class U, class C>
struct MapResultType<QStringList, U(C::*)() const> {
   using ResultType = QList<U>;
};

} // namespace QtPrivate

#endif
