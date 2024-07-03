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

#ifndef QOUTLINEMAPPER_P_H
#define QOUTLINEMAPPER_P_H

#include <qrect.h>
#include <qtransform.h>
#include <qpainterpath.h>
#include <qvector.h>

#include <qrasterdefs_p.h>
#include <qpaintengineex_p.h>

// This limitations comes from qgrayraster.c. Any higher and
// rasterization of shapes will produce incorrect results.
static constexpr const int QT_RASTER_COORD_LIMIT = 32767;

Q_GUI_EXPORT bool qt_scaleForTransform(const QTransform &transform, qreal *scale);

class QOutlineMapper
{
 public:
   QOutlineMapper()
      : m_in_clip_elements(false)
   {
   }

   //  Sets up the matrix to be used for conversion. This also
   //  sets up the qt_path_iterator function that is used as a callback to get points.
   void setMatrix(const QTransform &m) {
      m_m11 = m.m11();
      m_m12 = m.m12();
      m_m13 = m.m13();
      m_m21 = m.m21();
      m_m22 = m.m22();
      m_m23 = m.m23();
      m_m33 = m.m33();
      m_dx = m.dx();
      m_dy = m.dy();
      m_txop = m.type();
      qreal scale;
      qt_scaleForTransform(m, &scale);
      m_curve_threshold = scale == 0 ? qreal(0.25) : (qreal(0.25) / scale);
   }

   void beginOutline(Qt::FillRule fillRule) {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("QOutlineMapper::beginOutline() rule = %d", fillRule);
#endif

      m_valid = true;
      m_elements.clear();
      m_element_types.clear();
      m_points.clear();
      m_tags.clear();
      m_contours.clear();

      m_outline.flags = fillRule == Qt::WindingFill
         ? QT_FT_OUTLINE_NONE : QT_FT_OUTLINE_EVEN_ODD_FILL;

      m_subpath_start = 0;
   }

   void endOutline();

   void clipElements(const QPointF *points, const QPainterPath::ElementType *types, int count);

   void convertElements(const QPointF *points, const QPainterPath::ElementType *types, int count);

   void moveTo(const QPointF &pt) {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("QOutlineMapper::moveTo() pointf = (%f, %f)", pt.x(), pt.y());
#endif

      closeSubpath();
      m_subpath_start = m_elements.size();
      m_elements << pt;
      m_element_types << QPainterPath::MoveToElement;
   }

   void lineTo(const QPointF &pt) {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("QOutlineMapper::lineTo() pointf = (%f, %f)", pt.x(), pt.y());
#endif

      m_elements.append(pt);
      m_element_types << QPainterPath::LineToElement;
   }

   void curveTo(const QPointF &cp1, const QPointF &cp2, const QPointF &ep);

   void closeSubpath() {
      int element_count = m_elements.size();

      if (element_count > 0) {
         if (m_elements.at(element_count - 1) != m_elements.at(m_subpath_start)) {
            // Put the object on the stack to avoid the odd case where
            // lineTo reallocs the databuffer and the QPointF & will be invalidated.
            QPointF pt = m_elements.at(m_subpath_start);

            // only do lineTo if we have element_type array...
            if (m_element_types.size()) {
               lineTo(pt);
            } else {
               m_elements << pt;
            }

         }
      }
   }

   QT_FT_Outline *outline() {
      if (m_valid) {
         return &m_outline;
      }

      return nullptr;
   }

   QT_FT_Outline *convertPath(const QPainterPath &path);
   QT_FT_Outline *convertPath(const QVectorPath &path);

   inline const QPainterPath::ElementType *elementTypes() const {

      if (m_element_types.size() == 0) {
         return nullptr;
      } else {
         return m_element_types.data();
      }
   }

   QVector<QPainterPath::ElementType> m_element_types;
   QVector<QPointF> m_elements;

   QVector<QT_FT_Vector> m_points;
   QVector<char> m_tags;
   QVector<int> m_contours;

   QRect m_clip_rect;
   QRectF controlPointRect;          // only valid after endOutline()

   QT_FT_Outline m_outline;
   uint m_txop;

   int m_subpath_start;

   // Matrix
   qreal m_m11;
   qreal m_m12;
   qreal m_m13;
   qreal m_m21;
   qreal m_m22;
   qreal m_m23;
   qreal m_m33;
   qreal m_dx;
   qreal m_dy;

   qreal m_curve_threshold;

   bool m_valid;
   bool m_in_clip_elements;
};

#endif // QOUTLINEMAPPER_P_H
