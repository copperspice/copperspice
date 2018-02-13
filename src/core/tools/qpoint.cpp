/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
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

#include <qpoint.h>
#include <qdatastream.h>
#include <qdebug.h>

QDataStream &operator<<(QDataStream &s, const QPoint &p)
{
   s << (qint32)p.x() << (qint32)p.y();

   return s;
}

QDataStream &operator>>(QDataStream &s, QPoint &p)
{
   qint32 x, y;
   s >> x;
   p.rx() = x;
   s >> y;
   p.ry() = y;

   return s;
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

QDataStream &operator<<(QDataStream &s, const QPointF &p)
{
   s << double(p.x()) << double(p.y());
   return s;
}

QDataStream &operator>>(QDataStream &s, QPointF &p)
{
   double x, y;
   s >> x;
   s >> y;
   p.setX(qreal(x));
   p.setY(qreal(y));
   return s;
}
