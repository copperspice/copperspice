/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QVARIANT_P_H
#define QVARIANT_P_H

// takes a type, returns the internal void* pointer cast to a pointer of the input type

#include <qglobal.h>
#include <qvariant.h>

template <typename T>
inline const T *v_cast(const QVariant::Private *d, T * = nullptr)
{
   return ((sizeof(T) > sizeof(QVariant::Private::Data))
           ? static_cast<const T *>(d->data.shared->ptr)
           : static_cast<const T *>(static_cast<const void *>(&d->data.c)));
}

template <typename T>
inline T *v_cast(QVariant::Private *d, T * = nullptr)
{
   return ((sizeof(T) > sizeof(QVariant::Private::Data))
           ? static_cast<T *>(d->data.shared->ptr)
           : static_cast<T *>(static_cast<void *>(&d->data.c)));
}


// simple template that avoids to allocate 2 memory chunks when creating a QVariant
template <class T> class QVariantPrivateSharedEx : public QVariant::PrivateShared
{
 public:
   QVariantPrivateSharedEx() : QVariant::PrivateShared(&m_t) { }
   QVariantPrivateSharedEx(const T &t) : QVariant::PrivateShared(&m_t), m_t(t) { }

 private:
   T m_t;
};

// constructs a new variant if copy is nullptr, otherwise copy-constructs
template <class T, typename = typename std::enable_if<(sizeof(T) > sizeof(QVariant::Private::Data))>::type>
inline void v_construct(QVariant::Private *x, const void *copy, T * = nullptr)
{
   x->data.shared = copy ? new QVariantPrivateSharedEx<T>(*static_cast<const T *>(copy)) : new QVariantPrivateSharedEx<T>;
   x->is_shared   = true;
}

template <class T, typename = typename std::enable_if<sizeof(T) <= sizeof(QVariant::Private::Data)>::type, typename = void>
inline void v_construct(QVariant::Private *x, const void *copy, T * = nullptr)
{
   if (copy) {
      new (&x->data) T(*static_cast<const T *>(copy));

   } else {
      new (&x->data) T;

   }
}

template <class T, typename = typename std::enable_if<(sizeof(T) > sizeof(QVariant::Private::Data))>::type>
inline void v_construct(QVariant::Private *x, const T &t)
{
   x->data.shared = new QVariantPrivateSharedEx<T>(t);
   x->is_shared   = true;
}

template <class T, typename = typename std::enable_if<sizeof(T) <= sizeof(QVariant::Private::Data)>::type, typename = void>
inline void v_construct(QVariant::Private *x, const T &t)
{
   new (&x->data) T(t);
}

// deletes the internal structures
template <class T, typename = typename std::enable_if<(sizeof(T) > sizeof(QVariant::Private::Data))>::type>
inline void v_clear(QVariant::Private *d, T * = nullptr)
{
   // now we need to cast because QVariant::PrivateShared does not have a virtual destructor
   delete static_cast<QVariantPrivateSharedEx<T>*>(d->data.shared);
}

template <class T, typename = typename std::enable_if<sizeof(T) <= sizeof(QVariant::Private::Data)>::type, typename = void>
inline void v_clear(QVariant::Private *d, T * = nullptr)
{
   v_cast<T>(d)->~T();
}

Q_CORE_EXPORT const QVariant::Handler *qcoreVariantHandler();

#endif
