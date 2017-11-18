/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QVARIANT_P_H
#define QVARIANT_P_H

// takes a type, returns the internal void* pointer cast
// to a pointer of the input type

#include <qglobal.h>
#include <qvariant.h>

template <typename T>
inline const T *v_cast(const QVariant::Private *d, T * = 0)
{
   return ((sizeof(T) > sizeof(QVariant::Private::Data))
           ? static_cast<const T *>(d->data.shared->ptr)
           : static_cast<const T *>(static_cast<const void *>(&d->data.c)));
}

template <typename T>
inline T *v_cast(QVariant::Private *d, T * = 0)
{
   return ((sizeof(T) > sizeof(QVariant::Private::Data))
           ? static_cast<T *>(d->data.shared->ptr)
           : static_cast<T *>(static_cast<void *>(&d->data.c)));
}


//a simple template that avoids to allocate 2 memory chunks when creating a QVariant
template <class T> class QVariantPrivateSharedEx : public QVariant::PrivateShared
{
 public:
   QVariantPrivateSharedEx() : QVariant::PrivateShared(&m_t) { }
   QVariantPrivateSharedEx(const T &t) : QVariant::PrivateShared(&m_t), m_t(t) { }

 private:
   T m_t;
};

// constructs a new variant if copy is 0, otherwise copy-constructs
template <class T>
inline void v_construct(QVariant::Private *x, const void *copy, T * = 0)
{
   if (sizeof(T) > sizeof(QVariant::Private::Data)) {
      x->data.shared = copy ? new QVariantPrivateSharedEx<T>(*static_cast<const T *>(copy))
                       : new QVariantPrivateSharedEx<T>;
      x->is_shared = true;

   } else {
      if (copy) {
         new (&x->data.ptr) T(*static_cast<const T *>(copy));
      } else {
         new (&x->data.ptr) T;
      }
   }
}

template <class T>
inline void v_construct(QVariant::Private *x, const T &t)
{
   if (sizeof(T) > sizeof(QVariant::Private::Data)) {
      x->data.shared = new QVariantPrivateSharedEx<T>(t);
      x->is_shared = true;
   } else {
      new (&x->data.ptr) T(t);
   }
}

// deletes the internal structures
template <class T>
inline void v_clear(QVariant::Private *d, T * = 0)
{

   if (sizeof(T) > sizeof(QVariant::Private::Data)) {
      //now we need to cast
      //because QVariant::PrivateShared doesn't have a virtual destructor
      delete static_cast<QVariantPrivateSharedEx<T>*>(d->data.shared);

   } else {
      v_cast<T>(d)->~T();
   }

}

Q_CORE_EXPORT const QVariant::Handler *qcoreVariantHandler();

#endif // QVARIANT_P_H
