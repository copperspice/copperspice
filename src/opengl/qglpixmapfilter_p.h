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

#ifndef QGLPIXMAPFILTER_P_H
#define QGLPIXMAPFILTER_P_H

#include <qpixmapfilter_p.h>
#include <QtOpenGL/qgl.h>

QT_BEGIN_NAMESPACE

class QGLPixelBuffer;

class QGLPixmapFilterBase
{

 public:
   virtual ~QGLPixmapFilterBase() {}

 protected:
   void bindTexture(const QPixmap &src) const;
   void drawImpl(QPainter *painter, const QPointF &pos, const QPixmap &src, const QRectF &srcRect = QRectF()) const;

   virtual bool processGL(QPainter *painter, const QPointF &pos, const QPixmap &src, const QRectF &srcRect) const = 0;
};

template <typename Filter>
class QGLPixmapFilter : public Filter, public QGLPixmapFilterBase
{
 public:
   void draw(QPainter *painter, const QPointF &pos, const QPixmap &src, const QRectF &srcRect = QRectF()) const  override{
      const QRectF source = srcRect.isNull() ? QRectF(src.rect()) : srcRect;

      if (painter) {
         drawImpl(painter, pos, src, source);
      }
   }
};

QT_END_NAMESPACE

#endif // QGLPIXMAPFILTER_P_H
