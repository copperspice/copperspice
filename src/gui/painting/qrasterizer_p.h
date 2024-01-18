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

#ifndef QRASTERIZER_P_H
#define QRASTERIZER_P_H

#include <qglobal.h>
#include <qpainter.h>
#include <qdrawhelper_p.h>
#include <qrasterdefs_p.h>

struct QSpanData;

class QRasterBuffer;
class QRasterizerPrivate;
class QRasterizer

{
 public:
   QRasterizer();
   ~QRasterizer();

   void setAntialiased(bool antialiased);
   void setClipRect(const QRect &clipRect);
   void setLegacyRoundingEnabled(bool legacyRoundingEnabled);

   void initialize(ProcessSpans blend, void *data);

   void rasterize(const QT_FT_Outline *outline, Qt::FillRule fillRule);
   void rasterize(const QPainterPath &path, Qt::FillRule fillRule);

   // width should be in units of |a-b|
   void rasterizeLine(const QPointF &a, const QPointF &b, qreal width, bool squareCap = false);

 private:
   QRasterizerPrivate *d;
};


#endif
