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

#ifndef QTRIANGULATOR_P_H
#define QTRIANGULATOR_P_H

#include <QtCore/qvector.h>
#include <qvectorpath_p.h>

QT_BEGIN_NAMESPACE

class QVertexIndexVector
{
 public:
   enum Type {
      UnsignedInt,
      UnsignedShort
   };

   inline Type type() const {
      return t;
   }

   inline void setDataUint(const QVector<quint32> &data) {
      t = UnsignedInt;
      indices32 = data;
   }

   inline void setDataUshort(const QVector<quint16> &data) {
      t = UnsignedShort;
      indices16 = data;
   }

   inline const void *data() const {
      if (t == UnsignedInt) {
         return indices32.data();
      }
      return indices16.data();
   }

   inline int size() const {
      if (t == UnsignedInt) {
         return indices32.size();
      }
      return indices16.size();
   }

   inline QVertexIndexVector &operator = (const QVertexIndexVector &other) {
      if (t == UnsignedInt) {
         indices32 = other.indices32;
      } else {
         indices16 = other.indices16;
      }

      return *this;
   }

 private:

   Type t;
   QVector<quint32> indices32;
   QVector<quint16> indices16;
};

struct QTriangleSet {
   inline QTriangleSet() { }
   inline QTriangleSet(const QTriangleSet &other) : vertices(other.vertices), indices(other.indices) { }
   QTriangleSet &operator = (const QTriangleSet &other) {
      vertices = other.vertices;
      indices = other.indices;
      return *this;
   }

   // The vertices of a triangle are given by: (x[i[n]], y[i[n]]), (x[j[n]], y[j[n]]), (x[k[n]], y[k[n]]), n = 0, 1, ...
   QVector<qreal> vertices; // [x[0], y[0], x[1], y[1], x[2], ...]
   QVertexIndexVector indices; // [i[0], j[0], k[0], i[1], j[1], k[1], i[2], ...]
};

struct QPolylineSet {
   inline QPolylineSet() { }
   inline QPolylineSet(const QPolylineSet &other) : vertices(other.vertices), indices(other.indices) { }
   QPolylineSet &operator = (const QPolylineSet &other) {
      vertices = other.vertices;
      indices = other.indices;
      return *this;
   }

   QVector<qreal> vertices; // [x[0], y[0], x[1], y[1], x[2], ...]
   QVertexIndexVector indices;
};

// The vertex coordinates of the returned triangle set will be rounded to a grid with a mesh size
// of 1/32. The polygon is first transformed, then scaled by 32, the coordinates are rounded to
// integers, the polygon is triangulated, and then scaled back by 1/32.
// 'hint' should be a combination of QVectorPath::Hints.
// 'lod' is the level of detail. Default is 1. Curves are split into more lines when 'lod' is higher.
QTriangleSet qTriangulate(const qreal *polygon, int count,
                          uint hint = QVectorPath::PolygonHint | QVectorPath::OddEvenFill, const QTransform &matrix = QTransform());
QTriangleSet qTriangulate(const QVectorPath &path, const QTransform &matrix = QTransform(), qreal lod = 1);
QTriangleSet qTriangulate(const QPainterPath &path, const QTransform &matrix = QTransform(), qreal lod = 1);
QPolylineSet qPolyline(const QVectorPath &path, const QTransform &matrix = QTransform(), qreal lod = 1);
QPolylineSet qPolyline(const QPainterPath &path, const QTransform &matrix = QTransform(), qreal lod = 1);

QT_END_NAMESPACE

#endif
