/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
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

#include <qopengl_2pexvertexarray_p.h>

#include <qbezier_p.h>

void QOpenGL2PEXVertexArray::clear()
{
    vertexArray.clear();
    vertexArrayStops.clear();
    boundingRectDirty = true;
}

QOpenGLRect QOpenGL2PEXVertexArray::boundingRect() const
{
    if (boundingRectDirty) {
        return QOpenGLRect(0.0, 0.0, 0.0, 0.0);
    } else {
        return QOpenGLRect(minX, minY, maxX, maxY);
    }
}

void QOpenGL2PEXVertexArray::addClosingLine(int index)
{
    QPointF point(vertexArray.at(index));

    if (point != QPointF(vertexArray.last())) {
        vertexArray.append(point);
    }
}

void QOpenGL2PEXVertexArray::addCentroid(const QVectorPath &path, int subPathIndex)
{
    const QPointF *const points = reinterpret_cast<const QPointF *>(path.points());
    const QPainterPath::ElementType *const elements = path.elements();

    QPointF sum = points[subPathIndex];
    int count = 1;

    for (int i = subPathIndex + 1; i < path.elementCount() && (! elements || elements[i] != QPainterPath::MoveToElement); ++i) {
        sum += points[i];
        ++count;
    }

    const QPointF centroid = sum / qreal(count);
    vertexArray.append(centroid);
}

void QOpenGL2PEXVertexArray::addPath(const QVectorPath &path, GLfloat curveInverseScale, bool outline)
{
    const QPointF* const points = reinterpret_cast<const QPointF*>(path.points());
    const QPainterPath::ElementType* const elements = path.elements();

    if (boundingRectDirty) {
        minX = maxX = points[0].x();
        minY = maxY = points[0].y();
        boundingRectDirty = false;
    }

    if (!outline && !path.isConvex())
        addCentroid(path, 0);

    int lastMoveTo = vertexArray.size();
    vertexArray.append(points[0]); // The first element is always a moveTo

    do {
        if (! elements) {
           // If the path has a null elements pointer, the elements implicitly
           // start with a moveTo (already added) and continue with lineTos:
           for (int i=1; i<path.elementCount(); ++i)
              lineToArray(points[i].x(), points[i].y());

           break;
        }

        for (int i=1; i<path.elementCount(); ++i) {
            switch (elements[i]) {

            case QPainterPath::MoveToElement:
                if (! outline) {
                   addClosingLine(lastMoveTo);
                }

                vertexArrayStops.append(vertexArray.size());

                if (! outline) {
                    if (! path.isConvex()) {
                       addCentroid(path, i);
                    }

                    lastMoveTo = vertexArray.size();
                }

                lineToArray(points[i].x(), points[i].y()); // Add the moveTo as a new vertex
                break;

            case QPainterPath::LineToElement:
                lineToArray(points[i].x(), points[i].y());
                break;

            case QPainterPath::CurveToElement: {
                QBezier b = QBezier::fromPoints(*(((const QPointF *) points) + i - 1),
                      points[i], points[i+1], points[i+2]);

                QRectF bounds = b.bounds();
                // threshold based on same algorithm as in qtriangulatingstroker.cpp
                int threshold = qMin<float>(64, qMax(bounds.width(), bounds.height()) * 3.14f / (curveInverseScale * 6));

                if (threshold < 3) {
                  threshold = 3;
                }

                qreal one_over_threshold_minus_1 = qreal(1) / (threshold - 1);

                for (int t = 0; t < threshold; ++t) {
                   QPointF pt = b.pointAt(t * one_over_threshold_minus_1);
                   lineToArray(pt.x(), pt.y());
                }

                i += 2;
                break;
            }

            default:
                break;
            }
        }
    } while (false);

    if (! outline) {
        addClosingLine(lastMoveTo);
    }

    vertexArrayStops.append(vertexArray.size());
}

void QOpenGL2PEXVertexArray::lineToArray(const GLfloat x, const GLfloat y)
{
    vertexArray.append(QOpenGLPoint(x, y));

    if (x > maxX)
        maxX = x;
    else if (x < minX)
        minX = x;
    if (y > maxY)
        maxY = y;
    else if (y < minY)
        minY = y;
}


