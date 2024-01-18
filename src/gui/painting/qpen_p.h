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

#ifndef QPEN_P_H
#define QPEN_P_H

#include <qglobal.h>



class QPenPrivate
{
 public:
   QPenPrivate(const QBrush &brush, qreal width, Qt::PenStyle, Qt::PenCapStyle,
      Qt::PenJoinStyle _joinStyle, bool defaultWidth = true);

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
   uint defaultWidth : 1; // default-constructed width? used for cosmetic pen compatibility
};

#endif // QPEN_P_H
