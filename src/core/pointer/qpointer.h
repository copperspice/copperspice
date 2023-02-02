/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

class QVariant;

template <class T>
class QPointer
{
   template<typename U>
   struct TypeSelector {
      typedef QObject Type;
   };

   template<typename U>
   struct TypeSelector<const U> {
      typedef const QObject Type;
   };

   typedef typename TypeSelector<T>::Type QObjectType;
   QWeakPointer<QObjectType> wp;

 public:
   inline QPointer() { }

   inline QPointer(T *ptr)
      : wp(ptr, true)
   { }

   ~QPointer () = default;

   // compiler-generated copy/move ctor/assignment operators are fine

   inline QPointer<T> &operator=(T *ptr) {
      wp.assign(static_cast<QObjectType *>(ptr));
      return *this;
   }

   inline T *data() const {
      return static_cast<T *>( wp.data());
   }
   inline T *operator->() const {
      return data();
   }
   inline T &operator*() const {
      return *data();
   }
   inline operator T *() const {
      return data();
   }

   inline bool isNull() const {
      return wp.isNull();
   }

   inline void clear() {
      wp.clear();
   }
};

template <class T>
inline bool operator==(const T *o, const QPointer<T> &p)
{
   return o == p.operator->();
}

template<class T>
inline bool operator==(const QPointer<T> &p, const T *o)
{
   return p.operator->() == o;
}

template <class T>
inline bool operator==(T *o, const QPointer<T> &p)
{
   return o == p.operator->();
}

template<class T>
inline bool operator==(const QPointer<T> &p, T *o)
{
   return p.operator->() == o;
}

template<class T>
inline bool operator==(const QPointer<T> &p1, const QPointer<T> &p2)
{
   return p1.operator->() == p2.operator->();
}

template <class T>
inline bool operator!=(const T *o, const QPointer<T> &p)
{
   return o != p.operator->();
}

template<class T>
inline bool operator!= (const QPointer<T> &p, const T *o)
{
   return p.operator->() != o;
}

template <class T>
inline bool operator!=(T *o, const QPointer<T> &p)
{
   return o != p.operator->();
}

template<class T>
inline bool operator!= (const QPointer<T> &p, T *o)
{
   return p.operator->() != o;
}

template<class T>
inline bool operator!= (const QPointer<T> &p1, const QPointer<T> &p2)
{
   return p1.operator->() != p2.operator->() ;
}

template<typename T>
QPointer<T> qPointerFromVariant(const QVariant &variant)
{
   return QPointer<T>(qobject_cast<T *>(QtSharedPointer::weakPointerFromVariant_internal(variant).data()));
}

#endif // QPOINTER_H
