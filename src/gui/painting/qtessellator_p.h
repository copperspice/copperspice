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

#ifndef QTESSELLATOR_P_H
#define QTESSELLATOR_P_H

#include <qpoint.h>
#include <qrect.h>

QT_BEGIN_NAMESPACE

class QTessellatorPrivate;

typedef int Q27Dot5;
#define Q27Dot5ToDouble(i) ((i)/32.)
#define FloatToQ27Dot5(i) (int)((i) * 32)
#define IntToQ27Dot5(i) ((i) << 5)
#define Q27Dot5ToXFixed(i) ((i) << 11)
#define Q27Dot5Factor 32

class Q_GUI_EXPORT QTessellator
{
 public:
   QTessellator();
   virtual ~QTessellator();

   QRectF tessellate(const QPointF *points, int nPoints);
   void tessellateConvex(const QPointF *points, int nPoints);
   void tessellateRect(const QPointF &a, const QPointF &b, qreal width);

   void setWinding(bool w);

   struct Vertex {
      Q27Dot5 x;
      Q27Dot5 y;
   };
   struct Trapezoid {
      Q27Dot5 top;
      Q27Dot5 bottom;
      const Vertex *topLeft;
      const Vertex *bottomLeft;
      const Vertex *topRight;
      const Vertex *bottomRight;
   };
   virtual void addTrap(const Trapezoid &trap) = 0;

 private:
   friend class QTessellatorPrivate;
   QTessellatorPrivate *d;
};

QT_END_NAMESPACE

#endif
