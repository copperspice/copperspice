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

#ifndef QSHAREDPOINTER_H
#define QSHAREDPOINTER_H

#include <cs_enable_shared.h>
#include <cs_shared_pointer.h>
#include <cs_weak_pointer.h>

#include <qglobal.h>

class CSInternalRefCount;
class QObject;

#if ! defined(CS_DOXYPRESS)

template <typename T>
class QSharedPointer : public CsPointer::CsSharedPointer<T>
{
 public:
   using CsPointer::CsSharedPointer<T>::CsSharedPointer;

   QSharedPointer(CsPointer::CsSharedPointer<T> p) noexcept
      : CsPointer::CsSharedPointer<T>(std::move(p))
   {
   }

   QSharedPointer(const QSharedPointer &) = default;
   QSharedPointer &operator=(const QSharedPointer &) = default;

   QSharedPointer(QSharedPointer &&other) = default;
   QSharedPointer &operator=(QSharedPointer && other) = default;

   bool isNull() const {
      return CsPointer::CsSharedPointer<T>::is_null();
   }

   template <typename U>
   QSharedPointer<U> constCast() const {
      return CsPointer::const_pointer_cast<U>(*this);
   }

   template <typename U>
   QSharedPointer<U> dynamicCast() const {
      return CsPointer::dynamic_pointer_cast<U>(*this);
   }

   template <typename U>
   QSharedPointer<U> objectCast() const {
      return CsPointer::dynamic_pointer_cast<U>(*this);
   }

   template <typename U>
   QSharedPointer<U> staticCast() const {
      return CsPointer::static_pointer_cast<U>(*this);
   }
};

#endif

template <typename T>
using QEnableSharedFromThis = CsPointer::CsEnableSharedFromThis<T>;

// free functions
template <typename T>
uint qHash(const QSharedPointer<T> &ptr, uint seed = 0)
{
   return qHash(ptr.data(), seed);
}

template <typename T>
void swap(QSharedPointer<T> &ptr1, QSharedPointer<T> &ptr2) noexcept
{
   ptr1.swap(ptr2);
}

template < typename T, typename... Args, typename = typename std::enable_if_t < ! std::is_array_v<T >>>
QSharedPointer<T> QMakeShared(Args && ... args)
{
   return CsPointer::make_shared<T>(std::forward<Args>(args)...);
}

template <typename U, typename T>
[[deprecated("Use qSharedPointerStaticCast()")]]
QSharedPointer<U> qSharedPointerCast(const QSharedPointer<T> &ptr)
{
   return ptr.template staticCast<U>();
}

template <typename U, typename T>
QSharedPointer<U> qSharedPointerDynamicCast(const QSharedPointer<T> &ptr)
{
   return ptr.template dynamicCast<U>();
}

template <typename U, typename T>
QSharedPointer<U> qSharedPointerConstCast(const QSharedPointer<T> &ptr)
{
   return ptr.template constCast<U>();
}

template <typename U, typename T>
QSharedPointer<U> qSharedPointerObjectCast(const QSharedPointer<T> &ptr)
{
   return ptr.template objectCast<U>();
}

template <typename U, typename T>
QSharedPointer<U> qSharedPointerStaticCast(const QSharedPointer<T> &ptr)
{
   return ptr.template staticCast<U>();
}

#if ! defined(CS_DOXYPRESS)

template <typename T>
class QWeakPointer : public CsPointer::CsWeakPointer<T>
{
public:
   using CsPointer::CsWeakPointer<T>::CsWeakPointer;

   QWeakPointer(CsPointer::CsWeakPointer<T> other) noexcept
      : CsPointer::CsWeakPointer<T>(std::move(other))
   {
   }

   template <typename Base = QObject, typename X = CSInternalRefCount>
   QWeakPointer(T *ptr)
      : CsPointer::CsWeakPointer<T>( X::get_m_self(ptr).template staticCast<T>() )
   {
      // T must inherit from QObject
   }

   template <typename Base = QObject, typename = std::enable_if_t<std::is_base_of_v<Base, T>>>
   T * data() const noexcept {
      return CsPointer::CsWeakPointer<T>::lock().data();
   }

   bool isNull() const {
      return CsPointer::CsWeakPointer<T>::is_null();
   }
};

#endif

#endif
