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

#ifndef QMAPFUNC_H
#define QMAPFUNC_H

#include <qglobal.h>

template <typename Key>
inline bool qMapLessThanKey(const Key &key1, const Key &key2)
{
   return key1 < key2;
}

template <typename Ptr>
inline bool qMapLessThanKey(Ptr *key1, Ptr *key2)
{
   static_assert(sizeof(quintptr) == sizeof(Ptr *), "qMapLessThanKey: quintptr is not large enough to contain a ptr");
   return quintptr(key1) < quintptr(key2);
}

template <typename Ptr>
inline bool qMapLessThanKey(const Ptr *key1, const Ptr *key2)
{
   static_assert(sizeof(quintptr) == sizeof(const Ptr *), "qMapLessThanKey: quintptr is not large enough to contain a ptr");
   return quintptr(key1) < quintptr(key2);
}

template <typename Key>
class qMapCompare
{
 public:
   bool operator()(const Key &a, const Key &b)  const {
      return qMapLessThanKey(a, b);
   }
};

#endif