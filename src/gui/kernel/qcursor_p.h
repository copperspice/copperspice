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

#ifndef QCURSOR_P_H
#define QCURSOR_P_H

#include <qatomic.h>
#include <qglobal.h>
#include <qnamespace.h>
#include <qpixmap.h>


class QBitmap;

class QCursorData
{
 public:
   QCursorData(Qt::CursorShape s = Qt::ArrowCursor);
   ~QCursorData();

   void update();

   static void initialize();
   static void cleanup();

   static QCursorData *setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY,
      qreal devicePixelRatio);

   QAtomicInt ref;
   Qt::CursorShape cshape;
   QBitmap *bm;
   QBitmap *bmm;
   QPixmap pixmap;
   short hx;
   short hy;

   static bool initialized;
};

extern QCursorData *qt_cursorTable[Qt::LastCursor + 1]; // qcursor.cpp


#endif
