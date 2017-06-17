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

#ifndef QCONTAINERFWD_H
#define QCONTAINERFWD_H

#include <qglobal.h>

QT_BEGIN_NAMESPACE

template <typename Key> class qHashFunc;
template <typename Key> class qHashEqual;
template <typename Key, typename Val, typename Hash = qHashFunc<Key>, typename KeyEqual = qHashEqual<Key>> class QHash;
template <typename Key, typename Val, typename Hash = qHashFunc<Key>, typename KeyEqual = qHashEqual<Key>> class QMultiHash;

template <typename Key> class qMapCompare;
template <typename Key, typename Val, typename Compare = qMapCompare<Key>> class QMap;
template <typename Key, typename Val, typename Compare = qMapCompare<Key>> class QMultiMap;

template <typename Val> class QList;
template <typename Val> class QLinkedList;
template <typename Val> class QQueue;
template <typename Val> class QSet;
template <typename Val> class QStack;
template <typename Val> class QVector;

template <typename Key, typename Val> class  QCache;
template <typename T1,  typename T2>  struct QPair;

template <typename Val, int Prealloc = 256> class QVarLengthArray;

QT_END_NAMESPACE

#endif
