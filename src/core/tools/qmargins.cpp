/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

QDataStream &operator<<(QDataStream &s, const QMargins &m)
{
    s << m.left() << m.top() << m.right() << m.bottom();

    return s;
}

QDataStream &operator>>(QDataStream &s, QMargins &m)
{
    int left, top, right, bottom;

    s >> left; m.setLeft(left);
    s >> top; m.setTop(top);
    s >> right; m.setRight(right);
    s >> bottom; m.setBottom(bottom);

    return s;
}

QDataStream &operator<<(QDataStream &s, const QMarginsF &m)
{
    s << double(m.left()) << double(m.top()) << double(m.right()) << double(m.bottom());

    return s;
}

QDataStream &operator>>(QDataStream &s, QMarginsF &m)
{
    double left, top, right, bottom;
    s >> left;
    s >> top;
    s >> right;
    s >> bottom;

    m = QMarginsF(qreal(left), qreal(top), qreal(right), qreal(bottom));

    return s;
}

QDebug operator<<(QDebug dbg, const QMargins &m)
{

   dbg.nospace() << "QMargins(" << m.left() << ", "
                 << m.top() << ", " << m.right() << ", " << m.bottom() << ')';

   return dbg.space();
}

