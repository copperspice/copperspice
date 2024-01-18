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

#ifndef QCONTAINERFWD_H
#define QCONTAINERFWD_H

#include <utility>

template <typename Key>
class qHashFunc;

template <typename Key>
class qHashEqual;

template <typename Key>
class qMapCompare;

//
template <typename Key, typename Val, typename Compare = qMapCompare<Key>>
class QFlatMap;

template <typename Key, typename Val, typename Compare = qMapCompare<Key>>
class QMap;

template <typename Key, typename Val, typename Compare = qMapCompare<Key>>
class QMultiMap;

//
template <typename Key, typename Val, typename Hash = qHashFunc<Key>, typename KeyEqual = qHashEqual<Key>>
class QHash;

template <typename Key, typename Val, typename Hash = qHashFunc<Key>, typename KeyEqual = qHashEqual<Key>>
class QMultiHash;

template <typename Val> class QList;
template <typename Val> class QLinkedList;
template <typename Val> class QQueue;
template <typename Val> class QSet;
template <typename Val> class QStack;
template <typename Val> class QVector;

template <typename Key, typename Val>
class QCache;

template <class T1, class T2>
using QPair = std::pair<T1, T2>;

template <typename Val, int Prealloc = 256>
class QVarLengthArray;

#endif
