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

   bool begin(QPaintDevice *pdev) override;
   bool end() override;

   void clipEnabledChanged() override;
   void penChanged() override;
   void brushChanged() override;
   void brushOriginChanged() override;
   void opacityChanged() override;
   void compositionModeChanged() override;
   void renderHintsChanged() override;
   void transformChanged() override;

   void fill(const QVectorPath &path, const QBrush &brush) override;
   void clip(const QVectorPath &path, Qt::ClipOperation op) override;

   void setState(QPainterState *s) override;
   QPainterState *createState(QPainterState *orig) const override;

   QOpenGLPaintEngineState *state() {
      return static_cast<QOpenGLPaintEngineState *>(QPaintEngineEx::state());
   }

   const QOpenGLPaintEngineState *state() const {
      return static_cast<const QOpenGLPaintEngineState *>(QPaintEngineEx::state());
   }

   void updateState(const QPaintEngineState &state) override;

   void updatePen(const QPen &pen);
   void updateBrush(const QBrush &brush, const QPointF &pt);
   void updateFont(const QFont &font);
   void updateMatrix(const QTransform &matrix);
   void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
   void updateRenderHints(QPainter::RenderHints hints);
   void updateCompositionMode(QPainter::CompositionMode composition_mode);

   void drawRects(const QRectF *r, int rectCount) override;
   void drawLines(const QLineF *lines, int lineCount) override;
   void drawPoints(const QPointF *p, int pointCount) override;
   void drawRects(const QRect *r, int rectCount) override;
   void drawLines(const QLine *lines, int lineCount) override;
   void drawPoints(const QPoint *p, int pointCount) override;

   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;

   void drawPath(const QPainterPath &path) override;
   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
   void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode) override;
   void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s) override;
   void drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                  Qt::ImageConversionFlags conversionFlags) override;

   void drawTextItem(const QPointF &p, const QTextItem &ti) override;
   void drawStaticTextItem(QStaticTextItem *staticTextItem) override;

   void drawEllipse(const QRectF &rect) override;

#ifdef Q_OS_WIN
   HDC handle() const;
#else
   Qt::HANDLE handle() const;
#endif

   Type type() const override {
      return QPaintEngine::OpenGL;
   }

   bool supportsTransformations(qreal, const QTransform &) const override {
      return true;
   }

 private:
   void drawPolyInternal(const QPolygonF &pa, bool close = true);

   void drawTextureRect(int tx_width, int tx_height, const QRectF &r, const QRectF &sr, GLenum target, QGLTexture *tex);
   Q_DISABLE_COPY(QOpenGLPaintEngine)
};


QT_END_NAMESPACE

#endif // QPAINTENGINE_OPENGL_P_H
