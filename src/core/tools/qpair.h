/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPAIR_H
#define QPAIR_H

#include <QtCore/qdatastream.h>

QT_BEGIN_NAMESPACE

template <class T1, class T2>
struct QPair {
   typedef T1 first_type;
   typedef T2 second_type;

   QPair() : first(), second() {}
   QPair(const T1 &t1, const T2 &t2) : first(t1), second(t2) {}
   // compiler-generated copy/move ctor/assignment operators are fine!

   T1 first;
   T2 second;
};

template <class T1, class T2>
Q_INLINE_TEMPLATE bool operator==(const QPair<T1, T2> &p1, const QPair<T1, T2> &p2)
{
   return p1.first == p2.first && p1.second == p2.second;
}

template <class T1, class T2>
Q_INLINE_TEMPLATE bool operator!=(const QPair<T1, T2> &p1, const QPair<T1, T2> &p2)
{
   return !(p1 == p2);
}

template <class T1, class T2>
Q_INLINE_TEMPLATE bool operator<(const QPair<T1, T2> &p1, const QPair<T1, T2> &p2)
{
   return p1.first < p2.first || (!(p2.first < p1.first) && p1.second < p2.second);
}

template <class T1, class T2>
Q_INLINE_TEMPLATE bool operator>(const QPair<T1, T2> &p1, const QPair<T1, T2> &p2)
{
   return p2 < p1;
}

template <class T1, class T2>
Q_INLINE_TEMPLATE bool operator<=(const QPair<T1, T2> &p1, const QPair<T1, T2> &p2)
{
   return !(p2 < p1);
}

template <class T1, class T2>
Q_INLINE_TEMPLATE bool operator>=(const QPair<T1, T2> &p1, const QPair<T1, T2> &p2)
{
   return !(p1 < p2);
}

template <class T1, class T2>
Q_OUTOFLINE_TEMPLATE QPair<T1, T2> qMakePair(const T1 &x, const T2 &y)
{
   return QPair<T1, T2>(x, y);
}

#ifndef QT_NO_DATASTREAM
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

QT_END_NAMESPACE

#endif // QPAIR_H
