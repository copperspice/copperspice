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

#ifndef QPOINTER_H
#define QPOINTER_H

#include <qsharedpointer.h>

class CSInternalRefCount;
class QObject;
class QVariant;

template <class T>
class QPointer
{
 public:
   QPointer()
   { }

#if ! defined(CS_DOXYPRESS)
   template <class X = CSInternalRefCount>
#endif
   QPointer(T *ptr)
      : wp( X::get_m_self(ptr).template staticCast<T>() )
   {
      static_assert( std::is_base_of_v<QObject, T>, "T must be a class which inherits from QObject");
   }

   ~QPointer () = default;

   QPointer(const QPointer &other) = default;
   QPointer &operator=(const QPointer &other) = default;

   QPointer(QPointer &&other) = default;
   QPointer &operator=(QPointer && other) = default;

#if ! defined(CS_DOXYPRESS)
   template <class X = CSInternalRefCount>
#endif
   QPointer<T> &operator=(T *ptr) {
      static_assert( std::is_base_of_v<QObject, T>, "T must be a class which inherits from QObject");

      wp = X::get_m_self(ptr).template staticCast<T>();
      return *this;
   }

   T *data() const {
      return wp.data();
   }

   T *operator->() const {
      return data();
   }

   T &operator*() const {
      return *data();
   }

   operator T *() const {
      return data();
   }

   void clear() {
      wp.clear();
   }

   bool isNull() const {
      return wp.isNull();
   }

 private:
   QWeakPointer<T> wp;
};

template <class T>
inline bool operator==(const T *ptr1, const QPointer<T> &ptr2)
{
   return ptr1 == ptr2.operator->();
}

template<class T>
inline bool operator==(const QPointer<T> &ptr1, const T *ptr2)
{
   return ptr1.operator->() == ptr2;
}

template <class T>
inline bool operator==(T *o, const QPointer<T> &ptr2)
{
   return o == ptr2.operator->();
}

template<class T>
inline bool operator==(const QPointer<T> &ptr1, T *ptr2)
{
   return ptr1.operator->() == ptr2;
}

template<class T>
inline bool operator==(const QPointer<T> &ptr1, const QPointer<T> &ptr2)
{
   return ptr1.operator->() == ptr2.operator->();
}

template <class T>
inline bool operator!=(const T *ptr1, const QPointer<T> &ptr2)
{
   return ptr1 != ptr2.operator->();
}

template<class T>
inline bool operator!= (const QPointer<T> &ptr1, const T *ptr2)
{
   return ptr1.operator->() != ptr2;
}

template <class T>
inline bool operator!=(T *ptr1, const QPointer<T> &ptr2)
{
   return ptr1 != ptr2.operator->();
}

template<class T>
inline bool operator!= (const QPointer<T> &ptr1, T *ptr2)
{
   return ptr1.operator->() != ptr2;
}

template<class T>
inline bool operator!= (const QPointer<T> &ptr1, const QPointer<T> &ptr2)
{
   return ptr1.operator->() != ptr2.operator->();
}

#endif
