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

#include <qdatastream.h>
#include <qdebug.h>
#include <qmargins.h>

QDataStream &operator<<(QDataStream &stream, const QMargins &margin)
{
   stream << margin.left() << margin.top() << margin.right() << margin.bottom();

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QMargins &margin)
{
   int left, top, right, bottom;

   stream >> left;
   margin.setLeft(left);

   stream >> top;
   margin.setTop(top);

   stream >> right;
   margin.setRight(right);

   stream >> bottom;
   margin.setBottom(bottom);

   return stream;
}

QDataStream &operator<<(QDataStream &stream, const QMarginsF &marginF)
{
   stream << double(marginF.left()) << double(marginF.top()) << double(marginF.right()) << double(marginF.bottom());

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QMarginsF &marginF)
{
   double left, top, right, bottom;

   stream >> left;
   stream >> top;
   stream >> right;
   stream >> bottom;

   marginF = QMarginsF(qreal(left), qreal(top), qreal(right), qreal(bottom));

   return stream;
}

QDebug operator<<(QDebug dbg, const QMargins &m)
{
   dbg.nospace() << "QMargins(" << m.left() << ", "
         << m.top() << ", " << m.right() << ", " << m.bottom() << ')';

   return dbg.space();
}
