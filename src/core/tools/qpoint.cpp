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

#include <qpoint.h>

#include <qdatastream.h>
#include <qdebug.h>

QDataStream &operator<<(QDataStream &stream, const QPoint &point)
{
   stream << (qint32)point.x() << (qint32)point.y();

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QPoint &point)
{
   qint32 x, y;
   stream >> x;

   point.rx() = x;
   stream >> y;
   point.ry() = y;

   return stream;
}

int QPoint::manhattanLength() const
{
   return qAbs(x()) + qAbs(y());
}

QDebug operator<<(QDebug dbg, const QPoint &p)
{
   dbg.nospace() << "QPoint(" << p.x() << ',' << p.y() << ')';
   return dbg.space();
}

QDebug operator<<(QDebug d, const QPointF &p)
{
   d.nospace() << "QPointF(" << p.x() << ", " << p.y() << ')';
   return d.space();
}

qreal QPointF::manhattanLength() const
{
   return qAbs(x()) + qAbs(y());
}

QDataStream &operator<<(QDataStream &stream, const QPointF &pointF)
{
   stream << double(pointF.x()) << double(pointF.y());

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QPointF &pointF)
{
   double x, y;
   stream >> x;
   stream >> y;

   pointF.setX(qreal(x));
   pointF.setY(qreal(y));

   return stream;
}
