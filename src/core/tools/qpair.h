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

#ifndef QPAIR_H
#define QPAIR_H

#include <utility>
#include <qdatastream.h>

template <class T1, class T2>
using QPair = std::pair<T1, T2>;

template <class T1, class T2>
QPair<T1, T2> qMakePair(const T1 &x, const T2 &y)
{
   return QPair<T1, T2>(x, y);
}

template <class T1, class T2>
inline QDataStream &operator>>(QDataStream &s, QPair<T1, T2> &p)
{
   s >> p.first >> p.second;
   return s;
}

template <class T1, class T2>
inline QDataStream &operator<<(QDataStream &s, const QPair<T1, T2> &p)
{
   s << p.first << p.second;
   return s;
}

#endif
