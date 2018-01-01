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

#ifndef QPIXMAPFILTER_VG_P_H
#define QPIXMAPFILTER_VG_P_H

#include <qpixmapdata_vg_p.h>
#include <qpixmapfilter_p.h>
#include <qvarlengtharray.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_SHIVAVG)

class QVGPixmapConvolutionFilter : public QPixmapConvolutionFilter
{
    Q_OBJECT
public:
    QVGPixmapConvolutionFilter();
    ~QVGPixmapConvolutionFilter();

    void draw(QPainter *painter, const QPointF &dest, const QPixmap &src, const QRectF &srcRect) const;
};

class QVGPixmapColorizeFilter : public QPixmapColorizeFilter
{
    Q_OBJECT

public:
    QVGPixmapColorizeFilter();
    ~QVGPixmapColorizeFilter();

    void draw(QPainter *painter, const QPointF &dest, const QPixmap &src, const QRectF &srcRect) const;
};

class QVGPixmapDropShadowFilter : public QPixmapDropShadowFilter
{
    Q_OBJECT
public:
    QVGPixmapDropShadowFilter();
    ~QVGPixmapDropShadowFilter();

    void draw(QPainter *p, const QPointF &pos, const QPixmap &px, const QRectF &src) const;
};

class QVGPixmapBlurFilter : public QPixmapBlurFilter
{
    Q_OBJECT
public:
    QVGPixmapBlurFilter(QObject *parent = nullptr);
    ~QVGPixmapBlurFilter();

    void draw(QPainter *painter, const QPointF &dest, const QPixmap &src, const QRectF &srcRect = QRectF()) const;
};

#endif

QT_END_NAMESPACE

#endif
