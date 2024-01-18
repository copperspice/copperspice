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

#ifndef QPAINTERPATH_P_H
#define QPAINTERPATH_P_H

#include <qpainterpath.h>
#include <qregion.h>
#include <qlist.h>
#include <qvarlengtharray.h>

#include <qdebug.h>
#include <qvectorpath_p.h>
#include <qstroker_p.h>

class QPainterPathPrivate
{
 public:
   friend class QPainterPath;
   friend class QPainterPathData;
   friend class QPainterPathStroker;
   friend class QPainterPathStrokerPrivate;
   friend class QMatrix;
   friend class QTransform;
   friend class QVectorPath;

   friend struct cs_internal::QPainterPathPrivateDeleter;

   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPainterPath &);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPainterPath &);

   QPainterPathPrivate() : ref(1) {}

 private:
   QAtomicInt ref;
   QVector<QPainterPath::Element> elements;
};

class QPainterPathStrokerPrivate
{
 public:
   QPainterPathStrokerPrivate();

   QStroker stroker;
   QVector<qfixed> dashPattern;
   qreal dashOffset;
};

class QPolygonF;
class QVectorPathConverter;

class QVectorPathConverter
{
 public:
   QVectorPathConverter(const QVector<QPainterPath::Element> &path, uint fillRule, bool convex)
      : pathData(path, fillRule, convex), path(pathData.points.data(), path.size(),
        pathData.elements.data(), pathData.flags)
   {
   }

   QVectorPathConverter(const QVectorPathConverter &) = delete;
   QVectorPathConverter &operator=(const QVectorPathConverter &) = delete;

   const QVectorPath &vectorPath() {
      return path;
   }

   struct QVectorPathData {
      QVectorPathData(const QVector<QPainterPath::Element> &path, uint fillRule, bool convex)
         : elements(path.size()),
           points(path.size() * 2),
           flags(0) {
         int ptsPos = 0;
         bool isLines = true;
         for (int i = 0; i < path.size(); ++i) {
            const QPainterPath::Element &e = path.at(i);
            elements[i] = e.type;
            points[ptsPos++] = e.x;
            points[ptsPos++] = e.y;
            if (e.type == QPainterPath::CurveToElement) {
               flags |= QVectorPath::CurvedShapeMask;
            }

            // This is to check if the path contains only alternating lineTo/moveTo,
            // in which case we can set the LinesHint in the path. MoveTo is 0 and
            // LineTo is 1 so the i%2 gets us what we want cheaply.
            isLines = isLines && e.type == (QPainterPath::ElementType) (i % 2);
         }

         if (fillRule == Qt::WindingFill) {
            flags |= QVectorPath::WindingFill;
         } else {
            flags |= QVectorPath::OddEvenFill;
         }

         if (isLines) {
            flags |= QVectorPath::LinesShapeMask;
         } else {
            flags |= QVectorPath::AreaShapeMask;
            if (!convex) {
               flags |= QVectorPath::NonConvexShapeMask;
            }
         }

      }
      QVarLengthArray<QPainterPath::ElementType> elements;
      QVarLengthArray<qreal> points;
      uint flags;
   };

   QVectorPathData pathData;
   QVectorPath path;
};

class QPainterPathData : public QPainterPathPrivate
{
 public:
   QPainterPathData()
      : cStart(0), fillRule(Qt::OddEvenFill), dirtyBounds(false),
        dirtyControlBounds(false), pathConverter(nullptr)
   {
      require_moveTo = false;
      convex = false;
   }

   QPainterPathData(const QPainterPathData &other)
      : QPainterPathPrivate(), cStart(other.cStart), fillRule(other.fillRule),
        bounds(other.bounds), controlBounds(other.controlBounds), dirtyBounds(other.dirtyBounds),
        dirtyControlBounds(other.dirtyControlBounds), convex(other.convex), pathConverter(nullptr)
   {
      require_moveTo = false;
      elements = other.elements;
   }

   ~QPainterPathData() {
      delete pathConverter;
   }

   inline bool isClosed() const;
   inline void close();
   inline void maybeMoveTo();

   const QVectorPath &vectorPath() {
      if (!pathConverter) {
         pathConverter = new QVectorPathConverter(elements, fillRule, convex);
      }
      return pathConverter->path;
   }

   int cStart;
   Qt::FillRule fillRule;

   QRectF bounds;
   QRectF controlBounds;

   uint require_moveTo : 1;
   uint dirtyBounds : 1;
   uint dirtyControlBounds : 1;
   uint convex : 1;

   QVectorPathConverter *pathConverter;
};

inline const QPainterPath QVectorPath::convertToPainterPath() const
{
   QPainterPath path;
   path.ensureData();
   QPainterPathData *data = path.d_func();
   data->elements.reserve(m_count);
   int index = 0;
   data->elements[0].x = m_points[index++];
   data->elements[0].y = m_points[index++];

   if (m_elements) {
      data->elements[0].type = m_elements[0];
      for (int i = 1; i < m_count; ++i) {
         QPainterPath::Element element;
         element.x = m_points[index++];
         element.y = m_points[index++];
         element.type = m_elements[i];
         data->elements << element;
      }
   } else {
      data->elements[0].type = QPainterPath::MoveToElement;
      for (int i = 1; i < m_count; ++i) {
         QPainterPath::Element element;
         element.x = m_points[index++];
         element.y = m_points[index++];
         element.type = QPainterPath::LineToElement;
         data->elements << element;
      }
   }

   if (m_hints & OddEvenFill) {
      data->fillRule = Qt::OddEvenFill;
   } else {
      data->fillRule = Qt::WindingFill;
   }

   return path;
}

void Q_GUI_EXPORT qt_find_ellipse_coords(const QRectF &rect, qreal angle, qreal length,
   QPointF *startPoint, QPointF *endPoint);

inline bool QPainterPathData::isClosed() const
{
   const QPainterPath::Element &first = elements.at(cStart);
   const QPainterPath::Element &last = elements.last();
   return first.x == last.x && first.y == last.y;
}

inline void QPainterPathData::close()
{
   Q_ASSERT(ref.load() == 1);
   require_moveTo = true;
   const QPainterPath::Element &first = elements.at(cStart);
   QPainterPath::Element &last = elements.last();
   if (first.x != last.x || first.y != last.y) {
      if (qFuzzyCompare(first.x, last.x) && qFuzzyCompare(first.y, last.y)) {
         last.x = first.x;
         last.y = first.y;
      } else {
         QPainterPath::Element e = { first.x, first.y, QPainterPath::LineToElement };
         elements << e;
      }
   }
}

inline void QPainterPathData::maybeMoveTo()
{
   if (require_moveTo) {
      QPainterPath::Element e = elements.last();
      e.type = QPainterPath::MoveToElement;
      elements.append(e);
      require_moveTo = false;
   }
}

#define KAPPA qreal(0.5522847498)

#endif // QPAINTERPATH_P_H
