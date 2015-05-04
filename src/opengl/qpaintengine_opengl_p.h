/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPAINTENGINE_OPENGL_P_H
#define QPAINTENGINE_OPENGL_P_H

#include <qpaintengineex_p.h>

QT_BEGIN_NAMESPACE

class QOpenGLPaintEnginePrivate;
class QGLTexture;

class QOpenGLPaintEngineState : public QPainterState
{
 public:
   QOpenGLPaintEngineState(QOpenGLPaintEngineState &other);
   QOpenGLPaintEngineState();
   ~QOpenGLPaintEngineState();

   QRegion clipRegion;
   bool hasClipping;
   QRect fastClip;
   uint depthClipId;
};

class QOpenGLPaintEngine : public QPaintEngineEx
{
   Q_DECLARE_PRIVATE(QOpenGLPaintEngine)
 public:
   QOpenGLPaintEngine();
   ~QOpenGLPaintEngine();

   bool begin(QPaintDevice *pdev);
   bool end();

   // new stuff
   void clipEnabledChanged();
   void penChanged();
   void brushChanged();
   void brushOriginChanged();
   void opacityChanged();
   void compositionModeChanged();
   void renderHintsChanged();
   void transformChanged();

   void fill(const QVectorPath &path, const QBrush &brush);
   void clip(const QVectorPath &path, Qt::ClipOperation op);

   void setState(QPainterState *s);
   QPainterState *createState(QPainterState *orig) const;
   inline QOpenGLPaintEngineState *state() {
      return static_cast<QOpenGLPaintEngineState *>(QPaintEngineEx::state());
   }
   inline const QOpenGLPaintEngineState *state() const {
      return static_cast<const QOpenGLPaintEngineState *>(QPaintEngineEx::state());
   }


   // old stuff
   void updateState(const QPaintEngineState &state);

   void updatePen(const QPen &pen);
   void updateBrush(const QBrush &brush, const QPointF &pt);
   void updateFont(const QFont &font);
   void updateMatrix(const QTransform &matrix);
   void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
   void updateRenderHints(QPainter::RenderHints hints);
   void updateCompositionMode(QPainter::CompositionMode composition_mode);

   void drawRects(const QRectF *r, int rectCount);
   void drawLines(const QLineF *lines, int lineCount);
   void drawPoints(const QPointF *p, int pointCount);
   void drawRects(const QRect *r, int rectCount);
   void drawLines(const QLine *lines, int lineCount);
   void drawPoints(const QPoint *p, int pointCount);

   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);

   void drawPath(const QPainterPath &path);
   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
   void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);
   void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);
   void drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                  Qt::ImageConversionFlags conversionFlags);
   void drawTextItem(const QPointF &p, const QTextItem &ti);
   void drawStaticTextItem(QStaticTextItem *staticTextItem);

   void drawEllipse(const QRectF &rect);

#ifdef Q_OS_WIN
   HDC handle() const;
#else
   Qt::HANDLE handle() const;
#endif
   inline Type type() const {
      return QPaintEngine::OpenGL;
   }
   bool supportsTransformations(qreal, const QTransform &) const {
      return true;
   }

 private:
   void drawPolyInternal(const QPolygonF &pa, bool close = true);
   void drawTextureRect(int tx_width, int tx_height, const QRectF &r, const QRectF &sr,
                        GLenum target, QGLTexture *tex);
   Q_DISABLE_COPY(QOpenGLPaintEngine)
};


QT_END_NAMESPACE

#endif // QPAINTENGINE_OPENGL_P_H
