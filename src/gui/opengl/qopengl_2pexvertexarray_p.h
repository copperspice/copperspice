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

#ifndef QOPENGL2PEXVERTEXARRAY_P_H
#define QOPENGL2PEXVERTEXARRAY_P_H

#include <qrectf.h>
#include <qvector.h>

#include <qvectorpath_p.h>
#include <qopenglcontext_p.h>

class QOpenGLPoint
{
public:
    QOpenGLPoint(GLfloat new_x, GLfloat new_y) :
        x(new_x), y(new_y) {};

    QOpenGLPoint(const QPointF &p) :
        x(p.x()), y(p.y()) {};

    QOpenGLPoint(const QPointF* p) :
        x(p->x()), y(p->y()) {};

    GLfloat x;
    GLfloat y;

    operator QPointF() {return QPointF(x,y);}
    operator QPointF() const {return QPointF(x,y);}
};

struct QOpenGLRect
{
    QOpenGLRect(const QRectF &r)
        :  left(r.left()), top(r.top()), right(r.right()), bottom(r.bottom())
    {
    }

    QOpenGLRect(GLfloat l, GLfloat t, GLfloat r, GLfloat b)
        : left(l), top(t), right(r), bottom(b)
    {
    }

    GLfloat left;
    GLfloat top;
    GLfloat right;
    GLfloat bottom;

    operator QRectF() const {return QRectF(left, top, right-left, bottom-top);}
};

class QOpenGL2PEXVertexArray
{
public:
    QOpenGL2PEXVertexArray()
        : maxX(-2e10), maxY(-2e10), minX(2e10), minY(2e10), boundingRectDirty(true)
    {
    }

    inline void addRect(const QRectF &rect)
    {
        qreal top = rect.top();
        qreal left = rect.left();
        qreal bottom = rect.bottom();
        qreal right = rect.right();

        vertexArray << QOpenGLPoint(left, top)
                    << QOpenGLPoint(right, top)
                    << QOpenGLPoint(right, bottom)
                    << QOpenGLPoint(right, bottom)
                    << QOpenGLPoint(left, bottom)
                    << QOpenGLPoint(left, top);
    }

    inline void addQuad(const QRectF &rect)
    {
        qreal top = rect.top();
        qreal left = rect.left();
        qreal bottom = rect.bottom();
        qreal right = rect.right();

        vertexArray << QOpenGLPoint(left, top)
                    << QOpenGLPoint(right, top)
                    << QOpenGLPoint(left, bottom)
                    << QOpenGLPoint(right, bottom);

    }

    inline void addVertex(const GLfloat x, const GLfloat y)
    {
        vertexArray.append(QOpenGLPoint(x, y));
    }

    void addPath(const QVectorPath &path, GLfloat curveInverseScale, bool outline = true);
    void clear();

    QOpenGLPoint *data() {
      return vertexArray.data();
    }

    const int *stops() const {
       return vertexArrayStops.data();
    }

    int stopCount() const { return vertexArrayStops.size(); }
    QOpenGLRect boundingRect() const;

    int vertexCount() const { return vertexArray.size(); }

    void lineToArray(const GLfloat x, const GLfloat y);

private:
    QVector<QOpenGLPoint> vertexArray;
    QVector<int> vertexArrayStops;

    GLfloat     maxX;
    GLfloat     maxY;
    GLfloat     minX;
    GLfloat     minY;
    bool        boundingRectDirty;
    void addClosingLine(int index);
    void addCentroid(const QVectorPath &path, int subPathIndex);
};

#endif
