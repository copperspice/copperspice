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

#ifndef QPEN_P_H
#define QPEN_P_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QPenPrivate
{
 public:
   QPenPrivate(const QBrush &brush, qreal width, Qt::PenStyle, Qt::PenCapStyle, Qt::PenJoinStyle _joinStyle);

   QAtomicInt ref;
   qreal width;
   QBrush brush;
   Qt::PenStyle style;
   Qt::PenCapStyle capStyle;
   Qt::PenJoinStyle joinStyle;
   mutable QVector<qreal> dashPattern;
   qreal dashOffset;
   qreal miterLimit;
   uint cosmetic : 1;
};

QT_END_NAMESPACE

#endif // QPEN_P_H
