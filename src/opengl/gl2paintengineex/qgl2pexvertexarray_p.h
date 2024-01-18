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

#ifndef QGL2PEXVERTEXARRAY_P_H
#define QGL2PEXVERTEXARRAY_P_H

#include <qrectf.h>
#include <qvector.h>

#include <qvectorpath_p.h>
#include <qgl_p.h>

class QGLPoint
{

 public:
   QGLPoint(GLfloat new_x, GLfloat new_y) :
      x(new_x), y(new_y) {};

   QGLPoint(const QPointF &p) :
      x(p.x()), y(p.y()) {};

   QGLPoint(const QPointF *p) :
      x(p->x()), y(p->y()) {};

   GLfloat x;
   GLfloat y;

   operator QPointF() {
      return QPointF(x, y);
   }

   operator QPointF() const {
      return QPointF(x, y);
   }
};

struct QGLRect {
   QGLRect(const QRectF &r)
      :  left(r.left()), top(r.top()), right(r.right()), bottom(r.bottom()) {}

   QGLRect(GLfloat l, GLfloat t, GLfloat r, GLfloat b)
      : left(l), top(t), right(r), bottom(b) {}

   GLfloat left;
   GLfloat top;
   GLfloat right;
   GLfloat bottom;

   operator QRectF() const {
      return QRectF(left, top, right - left, bottom - top);
   }
};

class QGL2PEXVertexArray
{
 public:
   QGL2PEXVertexArray()
      : maxX(-2e10), maxY(-2e10), minX(2e10), minY(2e10), boundingRectDirty(true) {
   }

   inline void addRect(const QRectF &rect) {
      qreal top = rect.top();
      qreal left = rect.left();
      qreal bottom = rect.bottom();
      qreal right = rect.right();

      vertexArray << QGLPoint(left, top)
                  << QGLPoint(right, top)
                  << QGLPoint(right, bottom)
                  << QGLPoint(right, bottom)
                  << QGLPoint(left, bottom)
                  << QGLPoint(left, top);
   }

   inline void addQuad(const QRectF &rect) {
      qreal top = rect.top();
      qreal left = rect.left();
      qreal bottom = rect.bottom();
      qreal right = rect.right();

      vertexArray << QGLPoint(left, top)
                  << QGLPoint(right, top)
                  << QGLPoint(left, bottom)
                  << QGLPoint(right, bottom);

   }

   inline void addVertex(const GLfloat x, const GLfloat y) {
      vertexArray.append(QGLPoint(x, y));
   }

   void addPath(const QVectorPath &path, GLfloat curveInverseScale, bool outline = true);
   void clear();

   QGLPoint *data() {
      return vertexArray.data();
   }

   const int *stops() const {
      return vertexArrayStops.data();
   }

   int stopCount() const {
      return vertexArrayStops.size();
   }

   QGLRect boundingRect() const;

   int vertexCount() const {
      return vertexArray.size();
   }

   void lineToArray(const GLfloat x, const GLfloat y);

 private:
   QVector<QGLPoint> vertexArray;
   QVector<int>      vertexArrayStops;

   GLfloat     maxX;
   GLfloat     maxY;
   GLfloat     minX;
   GLfloat     minY;
   bool        boundingRectDirty;
   void addClosingLine(int index);
   void addCentroid(const QVectorPath &path, int subPathIndex);
};

#endif
