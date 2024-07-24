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

#include <qpainter.h>
#include <qpainter_p.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qdebug.h>
#include <qglyphrun.h>
#include <qimage.h>
#include <qmath.h>
#include <qmutex.h>
#include <qpaintdevice.h>
#include <qpaintengine.h>
#include <qpainterpath.h>
#include <qpicture.h>
#include <qpixmapcache.h>
#include <qplatform_integration.h>
#include <qplatform_theme.h>
#include <qpolygon.h>
#include <qstatictext.h>
#include <qstyle.h>
#include <qtextlayout.h>
#include <qthread.h>
#include <qvarlengtharray.h>
#include <qwidget.h>

#include <qapplication_p.h>
#include <qemulationpaintengine_p.h>
#include <qfontengine_p.h>
#include <qglyphrun_p.h>
#include <qhexstring_p.h>
#include <qmath_p.h>
#include <qpaintengine_p.h>
#include <qpaintengine_raster_p.h>
#include <qpainterpath_p.h>
#include <qrawfont_p.h>
#include <qstatictext_p.h>
#include <qstylehelper_p.h>
#include <qtextengine_p.h>
#include <qwidget_p.h>

#define QGradient_StretchToDevice     0x10000000
#define QPaintEngine_OpaqueBackground 0x40000000

extern QPixmap qt_pixmapForBrush(int style, bool invert);

void qt_format_text(const QFont &font,
   const QRectF &_r, int tf, const QTextOption *option, const QString &str, QRectF *brect,
   int tabstops, int *tabarray, int tabarraylen, QPainter *painter);

static void drawTextItemDecoration(QPainter *painter, const QPointF &pos, const QFontEngine *fe, QTextEngine *textEngine,
   QTextCharFormat::UnderlineStyle underlineStyle,
   QTextItem::RenderFlags flags, qreal width, const QTextCharFormat &charFormat);

// Helper function to calculate left most position, width and flags for decoration drawing
Q_GUI_EXPORT void qt_draw_decoration_for_glyphs(QPainter *painter, const glyph_t *glyphArray,
   const QFixedPoint *positions, int glyphCount,
   QFontEngine *fontEngine, const QFont &font,
   const QTextCharFormat &charFormat);

static inline QGradient::CoordinateMode coordinateMode(const QBrush &brush)
{
   switch (brush.style()) {
      case Qt::LinearGradientPattern:
      case Qt::RadialGradientPattern:
      case Qt::ConicalGradientPattern:
         return brush.gradient()->coordinateMode();

      default:
         break;
   }

   return QGradient::LogicalMode;
}

extern bool qHasPixmapTexture(const QBrush &);

static inline bool is_brush_transparent(const QBrush &brush)
{
   Qt::BrushStyle s = brush.style();

   if (s != Qt::TexturePattern) {
      return s >= Qt::Dense1Pattern && s <= Qt::DiagCrossPattern;
   }

   if (qHasPixmapTexture(brush)) {
      return brush.texture().isQBitmap() || brush.texture().hasAlphaChannel();
   } else {
      const QImage texture = brush.textureImage();
      return texture.hasAlphaChannel() || (texture.depth() == 1 && texture.colorCount() == 0);
   }
}

static inline bool is_pen_transparent(const QPen &pen)
{
   return pen.style() > Qt::SolidLine || is_brush_transparent(pen.brush());
}

static inline uint line_emulation(uint emulation)
{
   return emulation & (QPaintEngine::PrimitiveTransform
         | QPaintEngine::AlphaBlend
         | QPaintEngine::Antialiasing
         | QPaintEngine::BrushStroke
         | QPaintEngine::ConstantOpacity
         | QGradient_StretchToDevice
         | QPaintEngine::ObjectBoundingModeGradients
         | QPaintEngine_OpaqueBackground);
}

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
static bool qt_painter_thread_test(int devType, int engineType, const char *what)
{
   const QPlatformIntegration *platformIntegration = QGuiApplicationPrivate::platformIntegration();

   switch (devType) {
      case QInternal::Image:
      case QInternal::Printer:
      case QInternal::Picture:
         // can be drawn onto these devices safely from any thread
         break;

      default:
         if (QThread::currentThread() != qApp->thread()
               // pixmaps cannot be targets unless threaded pixmaps are supported
               && (devType != QInternal::Pixmap || !platformIntegration->hasCapability(QPlatformIntegration::ThreadedPixmaps))
               // framebuffer objects and such cannot be targets unless threaded GL is supported
               && (devType != QInternal::OpenGL || !platformIntegration->hasCapability(QPlatformIntegration::ThreadedOpenGL))
               // widgets cannot be targets except for QGLWidget
               && (devType != QInternal::Widget || !platformIntegration->hasCapability(QPlatformIntegration::ThreadedOpenGL)
               || (engineType != QPaintEngine::OpenGL && engineType != QPaintEngine::OpenGL2))) {
            qDebug("qt_painter_thread() Unsafe to use %s outside the GUI thread", what);

            return false;
         }

         break;
   }

   return true;
}
#endif

void QPainterPrivate::checkEmulation()
{
   Q_ASSERT(extended);

   bool doEmulation = false;
   if (state->bgMode == Qt::OpaqueMode) {
      doEmulation = true;
   }

   const QGradient *bg = state->brush.gradient();
   if (bg && bg->coordinateMode() > QGradient::LogicalMode) {
      doEmulation = true;
   }

   const QGradient *pg = qpen_brush(state->pen).gradient();
   if (pg && pg->coordinateMode() > QGradient::LogicalMode) {
      doEmulation = true;
   }

   if (doEmulation && extended->flags() & QPaintEngineEx::DoNotEmulate) {
      return;
   }

   if (doEmulation) {
      if (extended != emulationEngine) {
         if (! emulationEngine) {
            emulationEngine = new QEmulationPaintEngine(extended);
         }
         extended = emulationEngine;
         extended->setState(state);
      }
   } else if (emulationEngine == extended) {
      extended = emulationEngine->real_engine;
   }
}

QPainterPrivate::~QPainterPrivate()
{
   delete emulationEngine;

   qDeleteAll(states);
   delete dummyState;
}

QTransform QPainterPrivate::viewTransform() const
{
   if (state->VxF) {
      qreal scaleW = qreal(state->vw) / qreal(state->ww);
      qreal scaleH = qreal(state->vh) / qreal(state->wh);
      return QTransform(scaleW, 0, 0, scaleH,
            state->vx - state->wx * scaleW, state->vy - state->wy * scaleH);
   }
   return QTransform();
}

qreal QPainterPrivate::effectiveDevicePixelRatio() const
{
   // Special cases for devices that does not support PdmDevicePixelRatio go here
   if (device->devType() == QInternal::Printer) {
      return qreal(1);
   }

   return qMax(qreal(1), device->devicePixelRatioF());
}

QTransform QPainterPrivate::hidpiScaleTransform() const
{
   const qreal devicePixelRatio = effectiveDevicePixelRatio();
   return QTransform::fromScale(devicePixelRatio, devicePixelRatio);
}
/*
   \internal
   Returns true if using a shared painter; otherwise false.
*/
bool QPainterPrivate::attachPainterPrivate(QPainter *q, QPaintDevice *pdev)
{
   Q_ASSERT(q);
   Q_ASSERT(pdev);

   QPainter *sp = pdev->sharedPainter();

   if (!sp) {
      return false;
   }

   // Save the current state of the shared painter and assign
   // the current d_ptr to the shared painter's d_ptr.
   sp->save();

   if (!sp->d_ptr->d_ptrs) {
      // Allocate space for 4 d-pointers (enough for up to 4 sub-sequent
      // redirections within the same paintEvent(), which should be enough
      // in 99% of all cases). E.g: A renders B which renders C which renders D.
      sp->d_ptr->d_ptrs_size = 4;
      sp->d_ptr->d_ptrs = (QPainterPrivate **)malloc(4 * sizeof(QPainterPrivate *));
      Q_CHECK_PTR(sp->d_ptr->d_ptrs);

   } else if (sp->d_ptr->refcount - 1 == sp->d_ptr->d_ptrs_size) {
      // However, to support corner cases we grow the array dynamically if needed.
      sp->d_ptr->d_ptrs_size <<= 1;
      const int newSize = sp->d_ptr->d_ptrs_size * sizeof(QPainterPrivate *);
      sp->d_ptr->d_ptrs = q_check_ptr((QPainterPrivate **)realloc(sp->d_ptr->d_ptrs, newSize));
   }

   sp->d_ptr->d_ptrs[++sp->d_ptr->refcount - 2] = q->d_ptr.data();
   q->d_ptr.take();
   q->d_ptr.reset(sp->d_ptr.data());

   Q_ASSERT(q->d_ptr->state);

   // Now initialize the painter with correct widget properties.
   q->initFrom(pdev);
   QPoint offset;
   pdev->redirected(&offset);
   offset += q->d_ptr->engine->coordinateOffset();

   // Update system rect.
   q->d_ptr->state->ww = q->d_ptr->state->vw = pdev->width();
   q->d_ptr->state->wh = q->d_ptr->state->vh = pdev->height();

   // Update matrix.
   if (q->d_ptr->state->WxF) {
      q->d_ptr->state->redirectionMatrix = q->d_ptr->state->matrix;
      q->d_ptr->state->redirectionMatrix *= q->d_ptr->hidpiScaleTransform().inverted();
      q->d_ptr->state->redirectionMatrix.translate(-offset.x(), -offset.y());
      q->d_ptr->state->worldMatrix = QTransform();
      q->d_ptr->state->WxF = false;
   } else {
      q->d_ptr->state->redirectionMatrix = QTransform::fromTranslate(-offset.x(), -offset.y());
   }
   q->d_ptr->updateMatrix();

   QPaintEnginePrivate *enginePrivate = q->d_ptr->engine->d_func();
   if (enginePrivate->currentClipDevice == pdev) {
      enginePrivate->systemStateChanged();
      return true;
   }

   // Update system transform and clip.
   enginePrivate->currentClipDevice = pdev;
   enginePrivate->setSystemTransform(q->d_ptr->state->matrix);
   return true;
}

void QPainterPrivate::detachPainterPrivate(QPainter *q)
{
   Q_ASSERT(refcount > 1);
   Q_ASSERT(q);

   QPainterPrivate *original = d_ptrs[--refcount - 1];
   if (inDestructor) {
      inDestructor = false;
      if (original) {
         original->inDestructor = true;
      }

   } else if (!original) {
      original = new QPainterPrivate(q);
   }

   d_ptrs[refcount - 1] = nullptr;
   q->restore();
   q->d_ptr.take();
   q->d_ptr.reset(original);

   if (emulationEngine) {
      extended = emulationEngine->real_engine;
      delete emulationEngine;
      emulationEngine = nullptr;
   }
}

void QPainterPrivate::draw_helper(const QPainterPath &originalPath, DrawOperation op)
{
   if (originalPath.isEmpty()) {
      return;
   }

   QPaintEngine::PaintEngineFeatures gradientStretch =
      QPaintEngine::PaintEngineFeatures(QGradient_StretchToDevice
         | QPaintEngine::ObjectBoundingModeGradients);

   const bool mustEmulateObjectBoundingModeGradients = extended
      || ((state->emulationSpecifier & QPaintEngine::ObjectBoundingModeGradients)
         && !engine->hasFeature(QPaintEngine::PatternTransform));

   if (!(state->emulationSpecifier & ~gradientStretch)
      && !mustEmulateObjectBoundingModeGradients) {
      drawStretchedGradient(originalPath, op);
      return;
   } else if (state->emulationSpecifier & QPaintEngine_OpaqueBackground) {
      drawOpaqueBackground(originalPath, op);
      return;
   }

   Q_Q(QPainter);

   qreal strokeOffsetX = 0, strokeOffsetY = 0;

   QPainterPath path = originalPath * state->matrix;
   QRectF pathBounds = path.boundingRect();
   QRectF strokeBounds;
   bool doStroke = (op & StrokeDraw) && (state->pen.style() != Qt::NoPen);
   if (doStroke) {
      qreal penWidth = state->pen.widthF();
      if (penWidth == 0) {
         strokeOffsetX = 1;
         strokeOffsetY = 1;
      } else {
         // In case of complex xform
         if (state->matrix.type() > QTransform::TxScale) {
            QPainterPathStroker stroker;
            stroker.setWidth(penWidth);
            stroker.setJoinStyle(state->pen.joinStyle());
            stroker.setCapStyle(state->pen.capStyle());
            QPainterPath stroke = stroker.createStroke(originalPath);
            strokeBounds = (stroke * state->matrix).boundingRect();
         } else {
            strokeOffsetX = qAbs(penWidth * state->matrix.m11() / 2.0);
            strokeOffsetY = qAbs(penWidth * state->matrix.m22() / 2.0);
         }
      }
   }

   QRect absPathRect;
   if (!strokeBounds.isEmpty()) {
      absPathRect = strokeBounds.intersected(QRectF(0, 0, device->width(), device->height())).toAlignedRect();
   } else {
      absPathRect = pathBounds.adjusted(-strokeOffsetX, -strokeOffsetY, strokeOffsetX, strokeOffsetY)
         .intersected(QRectF(0, 0, device->width(), device->height())).toAlignedRect();
   }

   if (q->hasClipping()) {
      bool hasPerspectiveTransform = false;
      for (int i = 0; i < state->clipInfo.size(); ++i) {
         const QPainterClipInfo &info = state->clipInfo.at(i);
         if (info.matrix.type() == QTransform::TxProject) {
            hasPerspectiveTransform = true;
            break;
         }
      }
      // avoid mapping QRegions with perspective transforms
      if (! hasPerspectiveTransform) {
         // The trick with txinv and invMatrix is done in order to
         // avoid transforming the clip to logical coordinates, and
         // then back to device coordinates. This is a problem with
         // QRegion/QRect based clips, since they use integer
         // coordinates and converting to/from logical coordinates will
         // lose precision.
         bool old_txinv = txinv;
         QTransform old_invMatrix = invMatrix;
         txinv = true;
         invMatrix = QTransform();
         QPainterPath clipPath = q->clipPath();
         QRectF r = clipPath.boundingRect().intersected(absPathRect);
         absPathRect = r.toAlignedRect();
         txinv = old_txinv;
         invMatrix = old_invMatrix;
      }
   }

   if (absPathRect.width() <= 0 || absPathRect.height() <= 0) {
      return;
   }

   QImage image(absPathRect.width(), absPathRect.height(), QImage::Format_ARGB32_Premultiplied);
   image.fill(0);

   QPainter p(&image);

   p.d_ptr->helper_device = helper_device;

   p.setOpacity(state->opacity);
   p.translate(-absPathRect.x(), -absPathRect.y());
   p.setTransform(state->matrix, true);
   p.setPen(doStroke ? state->pen : QPen(Qt::NoPen));
   p.setBrush((op & FillDraw) ? state->brush : QBrush(Qt::NoBrush));
   p.setBackground(state->bgBrush);
   p.setBackgroundMode(state->bgMode);
   p.setBrushOrigin(state->brushOrigin);

   p.setRenderHint(QPainter::Antialiasing, state->renderHints & QPainter::Antialiasing);
   p.setRenderHint(QPainter::SmoothPixmapTransform,
      state->renderHints & QPainter::SmoothPixmapTransform);

   p.drawPath(originalPath);
   p.end();

   q->save();
   state->matrix = QTransform();
   if (extended) {
      extended->transformChanged();
   } else {
      state->dirtyFlags |= QPaintEngine::DirtyTransform;
      updateState(state);
   }
   engine->drawImage(absPathRect,
      image,
      QRectF(0, 0, absPathRect.width(), absPathRect.height()),
      Qt::OrderedDither | Qt::OrderedAlphaDither);
   q->restore();
}

void QPainterPrivate::drawOpaqueBackground(const QPainterPath &path, DrawOperation op)
{
   Q_Q(QPainter);

   q->setBackgroundMode(Qt::TransparentMode);

   if (op & FillDraw && state->brush.style() != Qt::NoBrush) {
      q->fillPath(path, state->bgBrush.color());
      q->fillPath(path, state->brush);
   }

   if (op & StrokeDraw && state->pen.style() != Qt::NoPen) {
      q->strokePath(path, QPen(state->bgBrush.color(), state->pen.width()));
      q->strokePath(path, state->pen);
   }

   q->setBackgroundMode(Qt::OpaqueMode);
}

static inline QBrush stretchGradientToUserSpace(const QBrush &brush, const QRectF &boundingRect)
{
   Q_ASSERT(brush.style() >= Qt::LinearGradientPattern
      && brush.style() <= Qt::ConicalGradientPattern);

   QTransform gradientToUser(boundingRect.width(), 0, 0, boundingRect.height(),
      boundingRect.x(), boundingRect.y());

   QGradient g = *brush.gradient();
   g.setCoordinateMode(QGradient::LogicalMode);

   QBrush b(g);
   b.setTransform(gradientToUser * b.transform());
   return b;
}

void QPainterPrivate::drawStretchedGradient(const QPainterPath &path, DrawOperation op)
{
   Q_Q(QPainter);

   const qreal sw = helper_device->width();
   const qreal sh = helper_device->height();

   bool changedPen = false;
   bool changedBrush = false;
   bool needsFill = false;

   const QPen pen = state->pen;
   const QBrush brush = state->brush;

   const QGradient::CoordinateMode penMode = coordinateMode(pen.brush());
   const QGradient::CoordinateMode brushMode = coordinateMode(brush);

   QRectF boundingRect;

   // Draw the xformed fill if the brush is a stretch gradient.
   if ((op & FillDraw) && brush.style() != Qt::NoBrush) {
      if (brushMode == QGradient::StretchToDeviceMode) {
         q->setPen(Qt::NoPen);
         changedPen = pen.style() != Qt::NoPen;
         q->scale(sw, sh);
         updateState(state);

         const qreal isw = 1.0 / sw;
         const qreal ish = 1.0 / sh;
         QTransform inv(isw, 0, 0, ish, 0, 0);
         engine->drawPath(path * inv);
         q->scale(isw, ish);
      } else {
         needsFill = true;

         if (brushMode == QGradient::ObjectBoundingMode) {
            Q_ASSERT(engine->hasFeature(QPaintEngine::PatternTransform));
            boundingRect = path.boundingRect();
            q->setBrush(stretchGradientToUserSpace(brush, boundingRect));
            changedBrush = true;
         }
      }
   }

   if ((op & StrokeDraw) && pen.style() != Qt::NoPen) {
      // Draw the xformed outline if the pen is a stretch gradient.
      if (penMode == QGradient::StretchToDeviceMode) {
         q->setPen(Qt::NoPen);
         changedPen = true;

         if (needsFill) {
            updateState(state);
            engine->drawPath(path);
         }

         q->scale(sw, sh);
         q->setBrush(pen.brush());
         changedBrush = true;
         updateState(state);

         QPainterPathStroker stroker;
         stroker.setDashPattern(pen.style());
         stroker.setWidth(pen.widthF());
         stroker.setJoinStyle(pen.joinStyle());
         stroker.setCapStyle(pen.capStyle());
         stroker.setMiterLimit(pen.miterLimit());
         QPainterPath stroke = stroker.createStroke(path);

         const qreal isw = 1.0 / sw;
         const qreal ish = 1.0 / sh;
         QTransform inv(isw, 0, 0, ish, 0, 0);
         engine->drawPath(stroke * inv);
         q->scale(isw, ish);
      } else {
         if (!needsFill && brush.style() != Qt::NoBrush) {
            q->setBrush(Qt::NoBrush);
            changedBrush = true;
         }

         if (penMode == QGradient::ObjectBoundingMode) {
            Q_ASSERT(engine->hasFeature(QPaintEngine::PatternTransform));

            // avoid computing the bounding rect twice
            if (! needsFill || brushMode != QGradient::ObjectBoundingMode) {
               boundingRect = path.boundingRect();
            }

            QPen p = pen;
            p.setBrush(stretchGradientToUserSpace(pen.brush(), boundingRect));
            q->setPen(p);
            changedPen = true;
         } else if (changedPen) {
            q->setPen(pen);
            changedPen = false;
         }

         updateState(state);
         engine->drawPath(path);
      }
   } else if (needsFill) {
      if (pen.style() != Qt::NoPen) {
         q->setPen(Qt::NoPen);
         changedPen = true;
      }

      updateState(state);
      engine->drawPath(path);
   }

   if (changedPen) {
      q->setPen(pen);
   }

   if (changedBrush) {
      q->setBrush(brush);
   }
}

void QPainterPrivate::updateMatrix()
{
   state->matrix = state->WxF ? state->worldMatrix : QTransform();
   if (state->VxF) {
      state->matrix *= viewTransform();
   }

   txinv = false;                                // no inverted matrix
   state->matrix *= state->redirectionMatrix;
   if (extended) {
      extended->transformChanged();
   } else {
      state->dirtyFlags |= QPaintEngine::DirtyTransform;
   }

   state->matrix *= hidpiScaleTransform();
}

void QPainterPrivate::updateInvMatrix()
{
   Q_ASSERT(txinv == false);
   txinv = true;                                // creating inverted matrix
   invMatrix = state->matrix.inverted();
}

extern bool qt_isExtendedRadialGradient(const QBrush &brush);

void QPainterPrivate::updateEmulationSpecifier(QPainterState *s)
{
   bool alpha = false;
   bool linearGradient = false;
   bool radialGradient = false;
   bool extendedRadialGradient = false;
   bool conicalGradient = false;
   bool patternBrush = false;
   bool xform = false;
   bool complexXform = false;

   bool skip = true;

   // Pen and brush properties (we have to check both if one changes because the
   // one that's unchanged can still be in a state which requires emulation)
   if (s->state() & (QPaintEngine::DirtyPen | QPaintEngine::DirtyBrush | QPaintEngine::DirtyHints)) {
      // Check Brush stroke emulation
      if (!s->pen.isSolid() && !engine->hasFeature(QPaintEngine::BrushStroke)) {
         s->emulationSpecifier |= QPaintEngine::BrushStroke;
      } else {
         s->emulationSpecifier &= ~QPaintEngine::BrushStroke;
      }

      skip = false;

      QBrush penBrush = (qpen_style(s->pen) == Qt::NoPen) ? QBrush(Qt::NoBrush) : qpen_brush(s->pen);
      Qt::BrushStyle brushStyle = qbrush_style(s->brush);
      Qt::BrushStyle penBrushStyle = qbrush_style(penBrush);
      alpha = (penBrushStyle != Qt::NoBrush
            && (penBrushStyle < Qt::LinearGradientPattern && penBrush.color().alpha() != 255)
            && !penBrush.isOpaque())
         || (brushStyle != Qt::NoBrush
            && (brushStyle < Qt::LinearGradientPattern && s->brush.color().alpha() != 255)
            && !s->brush.isOpaque());
      linearGradient = ((penBrushStyle == Qt::LinearGradientPattern) ||
            (brushStyle == Qt::LinearGradientPattern));
      radialGradient = ((penBrushStyle == Qt::RadialGradientPattern) ||
            (brushStyle == Qt::RadialGradientPattern));
      extendedRadialGradient = radialGradient && (qt_isExtendedRadialGradient(penBrush) || qt_isExtendedRadialGradient(s->brush));

      conicalGradient = ((penBrushStyle == Qt::ConicalGradientPattern) ||
            (brushStyle == Qt::ConicalGradientPattern));
      patternBrush = (((penBrushStyle > Qt::SolidPattern
                  && penBrushStyle < Qt::LinearGradientPattern)
               || penBrushStyle == Qt::TexturePattern) ||
            ((brushStyle > Qt::SolidPattern
                  && brushStyle < Qt::LinearGradientPattern)
               || brushStyle == Qt::TexturePattern));

      bool penTextureAlpha = false;
      if (penBrush.style() == Qt::TexturePattern)
         penTextureAlpha = qHasPixmapTexture(penBrush)
            ? (penBrush.texture().depth() > 1) && penBrush.texture().hasAlpha()
            : penBrush.textureImage().hasAlphaChannel();
      bool brushTextureAlpha = false;
      if (s->brush.style() == Qt::TexturePattern) {
         brushTextureAlpha = qHasPixmapTexture(s->brush)
            ? (s->brush.texture().depth() > 1) && s->brush.texture().hasAlpha()
            : s->brush.textureImage().hasAlphaChannel();
      }
      if (((penBrush.style() == Qt::TexturePattern && penTextureAlpha)
            || (s->brush.style() == Qt::TexturePattern && brushTextureAlpha))
         && !engine->hasFeature(QPaintEngine::MaskedBrush)) {
         s->emulationSpecifier |= QPaintEngine::MaskedBrush;
      } else {
         s->emulationSpecifier &= ~QPaintEngine::MaskedBrush;
      }
   }

   if (s->state() & (QPaintEngine::DirtyHints
         | QPaintEngine::DirtyOpacity
         | QPaintEngine::DirtyBackgroundMode)) {
      skip = false;
   }

   if (skip) {
      return;
   }

   // XForm properties
   if (s->state() & QPaintEngine::DirtyTransform) {
      xform = !s->matrix.isIdentity();
      complexXform = !s->matrix.isAffine();
   } else if (s->matrix.type() >= QTransform::TxTranslate) {
      xform = true;
      complexXform = !s->matrix.isAffine();
   }

   const bool brushXform = (s->brush.transform().type() != QTransform::TxNone);
   const bool penXform = (s->pen.brush().transform().type() != QTransform::TxNone);

   const bool patternXform = patternBrush && (xform || brushXform || penXform);

   // Check alphablending
   if (alpha && !engine->hasFeature(QPaintEngine::AlphaBlend)) {
      s->emulationSpecifier |= QPaintEngine::AlphaBlend;
   } else {
      s->emulationSpecifier &= ~QPaintEngine::AlphaBlend;
   }

   // Linear gradient emulation
   if (linearGradient && !engine->hasFeature(QPaintEngine::LinearGradientFill)) {
      s->emulationSpecifier |= QPaintEngine::LinearGradientFill;
   } else {
      s->emulationSpecifier &= ~QPaintEngine::LinearGradientFill;
   }

   // Radial gradient emulation
   if (extendedRadialGradient || (radialGradient && !engine->hasFeature(QPaintEngine::RadialGradientFill))) {
      s->emulationSpecifier |= QPaintEngine::RadialGradientFill;
   } else {
      s->emulationSpecifier &= ~QPaintEngine::RadialGradientFill;
   }

   // Conical gradient emulation
   if (conicalGradient && !engine->hasFeature(QPaintEngine::ConicalGradientFill)) {
      s->emulationSpecifier |= QPaintEngine::ConicalGradientFill;
   } else {
      s->emulationSpecifier &= ~QPaintEngine::ConicalGradientFill;
   }

   // Pattern brushes
   if (patternBrush && !engine->hasFeature(QPaintEngine::PatternBrush)) {
      s->emulationSpecifier |= QPaintEngine::PatternBrush;
   } else {
      s->emulationSpecifier &= ~QPaintEngine::PatternBrush;
   }

   // Pattern XForms
   if (patternXform && !engine->hasFeature(QPaintEngine::PatternTransform)) {
      s->emulationSpecifier |= QPaintEngine::PatternTransform;
   } else {
      s->emulationSpecifier &= ~QPaintEngine::PatternTransform;
   }

   // Primitive XForms
   if (xform && !engine->hasFeature(QPaintEngine::PrimitiveTransform)) {
      s->emulationSpecifier |= QPaintEngine::PrimitiveTransform;
   } else {
      s->emulationSpecifier &= ~QPaintEngine::PrimitiveTransform;
   }

   // Perspective XForms
   if (complexXform && !engine->hasFeature(QPaintEngine::PerspectiveTransform)) {
      s->emulationSpecifier |= QPaintEngine::PerspectiveTransform;
   } else {
      s->emulationSpecifier &= ~QPaintEngine::PerspectiveTransform;
   }

   // Constant opacity
   if (state->opacity != 1 && !engine->hasFeature(QPaintEngine::ConstantOpacity)) {
      s->emulationSpecifier |= QPaintEngine::ConstantOpacity;
   } else {
      s->emulationSpecifier &= ~QPaintEngine::ConstantOpacity;
   }

   bool gradientStretch = false;
   bool objectBoundingMode = false;
   if (linearGradient || conicalGradient || radialGradient) {
      QGradient::CoordinateMode brushMode = coordinateMode(s->brush);
      QGradient::CoordinateMode penMode = coordinateMode(s->pen.brush());

      gradientStretch |= (brushMode == QGradient::StretchToDeviceMode);
      gradientStretch |= (penMode == QGradient::StretchToDeviceMode);

      objectBoundingMode |= (brushMode == QGradient::ObjectBoundingMode);
      objectBoundingMode |= (penMode == QGradient::ObjectBoundingMode);
   }
   if (gradientStretch) {
      s->emulationSpecifier |= QGradient_StretchToDevice;
   } else {
      s->emulationSpecifier &= ~QGradient_StretchToDevice;
   }

   if (objectBoundingMode && !engine->hasFeature(QPaintEngine::ObjectBoundingModeGradients)) {
      s->emulationSpecifier |= QPaintEngine::ObjectBoundingModeGradients;
   } else {
      s->emulationSpecifier &= ~QPaintEngine::ObjectBoundingModeGradients;
   }

   // Opaque backgrounds...
   if (s->bgMode == Qt::OpaqueMode &&
      (is_pen_transparent(s->pen) || is_brush_transparent(s->brush))) {
      s->emulationSpecifier |= QPaintEngine_OpaqueBackground;
   } else {
      s->emulationSpecifier &= ~QPaintEngine_OpaqueBackground;
   }
}

void QPainterPrivate::updateStateImpl(QPainterState *newState)
{
   // ### we might have to call QPainter::begin() here
   if (!engine->state) {
      engine->state = newState;
      engine->setDirty(QPaintEngine::AllDirty);
   }

   if (engine->state->painter() != newState->painter)
      // ### this could break with clip regions vs paths.
   {
      engine->setDirty(QPaintEngine::AllDirty);
   }

   // Upon restore, revert all changes since last save
   else if (engine->state != newState) {
      newState->dirtyFlags |= QPaintEngine::DirtyFlags(static_cast<QPainterState *>(engine->state)->changeFlags);
   }

   // We need to store all changes made so that restore can deal with them
   else {
      newState->changeFlags |= newState->dirtyFlags;
   }

   updateEmulationSpecifier(newState);

   // Unset potential dirty background mode
   newState->dirtyFlags &= ~(QPaintEngine::DirtyBackgroundMode
         | QPaintEngine::DirtyBackground);

   engine->state = newState;
   engine->updateState(*newState);
   engine->clearDirty(QPaintEngine::AllDirty);

}

void QPainterPrivate::updateState(QPainterState *newState)
{
   if (!newState) {
      engine->state = newState;

   } else if (newState->state() || engine->state != newState) {
      updateStateImpl(newState);
   }
}

QPainter::QPainter()
   : d_ptr(new QPainterPrivate(this))
{
}

QPainter::QPainter(QPaintDevice *pd)
   : d_ptr(nullptr)
{
   Q_ASSERT(pd != nullptr);

   if (!QPainterPrivate::attachPainterPrivate(this, pd)) {
      d_ptr.reset(new QPainterPrivate(this));
      begin(pd);
   }

   Q_ASSERT(d_ptr);
}

/*!
    Destroys the painter.
*/
QPainter::~QPainter()
{
   d_ptr->inDestructor = true;

   try {
      if (isActive())
      {
         end();
      } else if (d_ptr->refcount > 1)
      {
         d_ptr->detachPainterPrivate(this);
      }

   } catch (...) {
      // don't throw anything in the destructor.
   }

   if (d_ptr) {
      // Make sure we haven't messed things up.
      Q_ASSERT(d_ptr->inDestructor);
      d_ptr->inDestructor = false;
      Q_ASSERT(d_ptr->refcount == 1);
      if (d_ptr->d_ptrs) {
         free(d_ptr->d_ptrs);
      }
   }
}

QPaintDevice *QPainter::device() const
{
   Q_D(const QPainter);
   if (isActive() && d->engine->d_func()->currentClipDevice) {
      return d->engine->d_func()->currentClipDevice;
   }

   return d->original_device;
}

bool QPainter::isActive() const
{
   Q_D(const QPainter);
   return d->engine;
}

void QPainter::initFrom(const QPaintDevice *device)
{
   Q_ASSERT_X(device, "QPainter::initFrom()", "QPaintDevice can not be a nullptr");

   Q_D(QPainter);
   if (!d->engine) {
      qWarning("QPainter::initFrom() Painter engine is not active");
      return;
   }

   device->initPainter(this);

   if (d->extended) {
      d->extended->penChanged();
   } else if (d->engine) {
      d->engine->setDirty(QPaintEngine::DirtyPen);
      d->engine->setDirty(QPaintEngine::DirtyBrush);
      d->engine->setDirty(QPaintEngine::DirtyFont);
   }
}

void QPainter::save()
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::save() Painter engine not active");
      return;
   }

   if (d->extended) {
      d->state = d->extended->createState(d->states.back());
      d->extended->setState(d->state);
   } else {
      d->updateState(d->state);
      d->state = new QPainterState(d->states.back());
      d->engine->state = d->state;
   }
   d->states.push_back(d->state);
}

void QPainter::restore()
{
   Q_D(QPainter);
   if (d->states.size() <= 1) {
      qWarning("QPainter::restore() Unbalanced save / restore states");
      return;
   } else if (!d->engine) {
      qWarning("QPainter::restore() Painter engine not active");
      return;
   }

   QPainterState *tmp = d->state;
   d->states.pop_back();
   d->state = d->states.back();
   d->txinv = false;

   if (d->extended) {
      d->checkEmulation();
      d->extended->setState(d->state);
      delete tmp;
      return;
   }

   // trigger clip update if the clip path/region has changed since
   // last save
   if (!d->state->clipInfo.isEmpty()
      && (tmp->changeFlags & (QPaintEngine::DirtyClipRegion | QPaintEngine::DirtyClipPath))) {
      // reuse the tmp state to avoid any extra allocs...
      tmp->dirtyFlags = QPaintEngine::DirtyClipPath;
      tmp->clipOperation = Qt::NoClip;
      tmp->clipPath = QPainterPath();
      d->engine->updateState(*tmp);
      // replay the list of clip states,
      for (int i = 0; i < d->state->clipInfo.size(); ++i) {
         const QPainterClipInfo &info = d->state->clipInfo.at(i);
         tmp->matrix = info.matrix;
         tmp->matrix *= d->state->redirectionMatrix;
         tmp->clipOperation = info.operation;
         if (info.clipType == QPainterClipInfo::RectClip) {
            tmp->dirtyFlags = QPaintEngine::DirtyClipRegion | QPaintEngine::DirtyTransform;
            tmp->clipRegion = info.rect;
         } else if (info.clipType == QPainterClipInfo::RegionClip) {
            tmp->dirtyFlags = QPaintEngine::DirtyClipRegion | QPaintEngine::DirtyTransform;
            tmp->clipRegion = info.region;
         } else { // clipType == QPainterClipInfo::PathClip
            tmp->dirtyFlags = QPaintEngine::DirtyClipPath | QPaintEngine::DirtyTransform;
            tmp->clipPath = info.path;
         }
         d->engine->updateState(*tmp);
      }


      //Since we've updated the clip region anyway, pretend that the clip path hasn't changed:
      d->state->dirtyFlags &= ~(QPaintEngine::DirtyClipPath | QPaintEngine::DirtyClipRegion);
      tmp->changeFlags &= ~uint(QPaintEngine::DirtyClipPath | QPaintEngine::DirtyClipRegion);
      tmp->changeFlags |= QPaintEngine::DirtyTransform;
   }

   d->updateState(d->state);
   delete tmp;
}

static inline void qt_cleanup_painter_state(QPainterPrivate *d)
{
   d->states.clear();
   delete d->state;
   d->state  = nullptr;
   d->engine = nullptr;
   d->device = nullptr;
}

bool QPainter::begin(QPaintDevice *pd)
{
   Q_ASSERT(pd);

   if (pd->painters > 0) {
      qWarning("QPainter::begin() Paint device can only be used by one QPainter at a time");
      return false;
   }

   if (d_ptr->engine) {
      qWarning("QPainter::begin() Painter engine already active");
      return false;
   }

   if (QPainterPrivate::attachPainterPrivate(this, pd)) {
      return true;
   }

   Q_D(QPainter);

   d->helper_device   = pd;
   d->original_device = pd;


   QPoint redirectionOffset;
   // We know for sure that redirection is broken when the widget is inside
   // its paint event, so it's safe to use our hard-coded redirection. However,
   // there IS one particular case we still need to support, and that's
   // when people call QPainter::setRedirected in the widget's paint event right
   // before any painter is created (or QPainter::begin is called). In that
   // particular case our hard-coded redirection is restored and the redirection
   // is retrieved from QPainter::redirected (as before).

   QPaintDevice *rpd = pd->redirected(&redirectionOffset);

   if (rpd) {
      pd = rpd;
   }

   if (pd->devType() == QInternal::Pixmap) {
      static_cast<QPixmap *>(pd)->detach();
   } else if (pd->devType() == QInternal::Image) {
      static_cast<QImage *>(pd)->detach();
   }

   d->engine = pd->paintEngine();

   if (! d->engine) {
      qWarning("QPainter::begin() Paint device engine is invalid, type: %d", pd->devType());
      return false;
   }

   d->device = pd;

   d->extended = d->engine->isExtended() ? static_cast<QPaintEngineEx *>(d->engine) : nullptr;
   if (d->emulationEngine) {
      d->emulationEngine->real_engine = d->extended;
   }

   // Setup new state
   Q_ASSERT(!d->state);
   d->state = d->extended ? d->extended->createState(nullptr) : new QPainterState;
   d->state->painter = this;
   d->states.push_back(d->state);

   d->state->redirectionMatrix.translate(-redirectionOffset.x(), -redirectionOffset.y());
   d->state->brushOrigin = QPointF();

   // Slip a painter state into the engine before we do any other operations
   if (d->extended) {
      d->extended->setState(d->state);
   } else {
      d->engine->state = d->state;
   }

   switch (pd->devType()) {
      case QInternal::Pixmap: {
         QPixmap *pm = static_cast<QPixmap *>(pd);
         Q_ASSERT(pm);
         if (pm->isNull()) {
            qWarning("QPainter::begin() Unable to paint on a invalid pixmap");
            qt_cleanup_painter_state(d);
            return false;
         }

         if (pm->depth() == 1) {
            d->state->pen = QPen(Qt::color1);
            d->state->brush = QBrush(Qt::color0);
         }
         break;
      }

      case QInternal::Image: {
         QImage *img = static_cast<QImage *>(pd);
         Q_ASSERT(img);

         if (img->isNull()) {
            qWarning("QPainter::begin() Unable to paint on a invalid image");
            qt_cleanup_painter_state(d);
            return false;

         } else if (img->format() == QImage::Format_Indexed8) {
            // Painting on indexed8 images is not supported.
            qWarning("QPainter::begin() Unable to paint on an image with a format of QImage::Format_Indexed8");
            qt_cleanup_painter_state(d);
            return false;
         }

         if (img->depth() == 1) {
            d->state->pen = QPen(Qt::color1);
            d->state->brush = QBrush(Qt::color0);
         }
         break;
      }

      default:
         break;
   }

   if (d->state->ww == 0) {
      // For compat with 3.x painter defaults
      d->state->ww = d->state->wh = d->state->vw = d->state->vh = 1024;
   }

   d->engine->setPaintDevice(pd);

   bool begun = d->engine->begin(pd);
   if (!begun) {
      qWarning("QPainter::begin() Failure in painting");
      if (d->engine->isActive()) {
         end();
      } else {
         qt_cleanup_painter_state(d);
      }
      return false;
   } else {
      d->engine->setActive(begun);
   }

   // Copy painter properties from original paint device,
   // required for QPixmap::grabWidget()
   if (d->original_device->devType() == QInternal::Widget) {
      initFrom(d->original_device);
   } else {
      d->state->layoutDirection = Qt::LayoutDirectionAuto;
      // make sure we have a font compatible with the paintdevice
      d->state->deviceFont = d->state->font = QFont(d->state->deviceFont, device());
   }

   QRect systemRect = d->engine->systemRect();
   if (!systemRect.isEmpty()) {
      d->state->ww = d->state->vw = systemRect.width();
      d->state->wh = d->state->vh = systemRect.height();
   } else {
      d->state->ww = d->state->vw = pd->metric(QPaintDevice::PdmWidth);
      d->state->wh = d->state->vh = pd->metric(QPaintDevice::PdmHeight);
   }

   const QPoint coordinateOffset = d->engine->coordinateOffset();
   d->state->redirectionMatrix.translate(-coordinateOffset.x(), -coordinateOffset.y());

   Q_ASSERT(d->engine->isActive());

   if (!d->state->redirectionMatrix.isIdentity() || d->effectiveDevicePixelRatio() > 1)  {
      d->updateMatrix();
   }

   Q_ASSERT(d->engine->isActive());
   d->state->renderHints = QPainter::TextAntialiasing;
   ++d->device->painters;

   d->state->emulationSpecifier = 0;

   return true;
}

bool QPainter::end()
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::end() Painter engine is not active");
      qt_cleanup_painter_state(d);
      return false;
   }

   if (d->refcount > 1) {
      d->detachPainterPrivate(this);
      return true;
   }

   bool ended = true;

   if (d->engine->isActive()) {
      ended = d->engine->end();
      d->updateState(nullptr);

      --d->device->painters;
      if (d->device->painters == 0) {
         d->engine->setPaintDevice(nullptr);
         d->engine->setActive(false);
      }
   }

   if (d->states.size() > 1) {
      qWarning("QPainter::end() Painter ended with extra saved states");
   }

   if (d->engine->autoDestruct()) {
      delete d->engine;
   }

   if (d->emulationEngine) {
      delete d->emulationEngine;
      d->emulationEngine = nullptr;
   }

   if (d->extended) {
      d->extended = nullptr;
   }

   qt_cleanup_painter_state(d);

   return ended;
}

QPaintEngine *QPainter::paintEngine() const
{
   Q_D(const QPainter);
   return d->engine;
}

void QPainter::beginNativePainting()
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::beginNativePainting() Painter engine not active");
      return;
   }

   if (d->extended) {
      d->extended->beginNativePainting();
   }
}

void QPainter::endNativePainting()
{
   Q_D(const QPainter);
   if (!d->engine) {
      qWarning("QPainter::beginNativePainting() Painter engine not active");
      return;
   }

   if (d->extended) {
      d->extended->endNativePainting();
   } else {
      d->engine->syncState();
   }
}

QFontMetrics QPainter::fontMetrics() const
{
   Q_D(const QPainter);
   if (!d->engine) {
      qWarning("QPainter::fontMetrics() Painter engine not active");
      return QFontMetrics(QFont());
   }
   return QFontMetrics(d->state->font);
}

QFontInfo QPainter::fontInfo() const
{
   Q_D(const QPainter);
   if (!d->engine) {
      qWarning("QPainter::fontInfo() Painter engine not active");
      return QFontInfo(QFont());
   }
   return QFontInfo(d->state->font);
}

qreal QPainter::opacity() const
{
   Q_D(const QPainter);
   if (!d->engine) {
      qWarning("QPainter::opacity() Painter engine not active");
      return 1.0;
   }
   return d->state->opacity;
}

void QPainter::setOpacity(qreal opacity)
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::setOpacity() Painter engine not active");
      return;
   }

   opacity = qMin(qreal(1), qMax(qreal(0), opacity));

   if (opacity == d->state->opacity) {
      return;
   }

   d->state->opacity = opacity;

   if (d->extended) {
      d->extended->opacityChanged();
   } else {
      d->state->dirtyFlags |= QPaintEngine::DirtyOpacity;
   }
}

QPoint QPainter::brushOrigin() const
{
   Q_D(const QPainter);

   if (! d->engine) {
      qWarning("QPainter::brushOrigin() Painter engine not active");
      return QPoint();
   }

   return QPointF(d->state->brushOrigin).toPoint();
}

void QPainter::setBrushOrigin(const QPointF &p)
{
   Q_D(QPainter);

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::setBrushOrigin() pointf = (%.2f, %.2f)", p.x(), p.y());
#endif

   if (! d->engine) {
      qWarning("QPainter::setBrushOrigin() Painter engine not active");
      return;
   }

   d->state->brushOrigin = p;

   if (d->extended) {
      d->extended->brushOriginChanged();
      return;
   }

   d->state->dirtyFlags |= QPaintEngine::DirtyBrushOrigin;
}

void QPainter::setCompositionMode(CompositionMode mode)
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::setCompositionMode() Painter engine not active");
      return;
   }

   if (d->state->composition_mode == mode) {
      return;
   }
   if (d->extended) {
      d->state->composition_mode = mode;
      d->extended->compositionModeChanged();
      return;
   }

   if (mode >= QPainter::RasterOp_SourceOrDestination) {
      if (!d->engine->hasFeature(QPaintEngine::RasterOpModes)) {
         qWarning("QPainter::setCompositionMode() Raster operation modes are not supported on this device");
         return;
      }

   } else if (mode >= QPainter::CompositionMode_Plus) {
      if (!d->engine->hasFeature(QPaintEngine::BlendModes)) {
         qWarning("QPainter::setCompositionMode() Blend modes not are supported on this device");
         return;
      }

   } else if (!d->engine->hasFeature(QPaintEngine::PorterDuff)) {
      if (mode != CompositionMode_Source && mode != CompositionMode_SourceOver) {
         qWarning("QPainter::setCompositionMode() PorterDuff modes are not supported on this device");
         return;
      }
   }

   d->state->composition_mode = mode;
   d->state->dirtyFlags |= QPaintEngine::DirtyCompositionMode;
}

QPainter::CompositionMode QPainter::compositionMode() const
{
   Q_D(const QPainter);
   if (!d->engine) {
      qWarning("QPainter::compositionMode() Painter engine not active");
      return QPainter::CompositionMode_SourceOver;
   }
   return d->state->composition_mode;
}

const QBrush &QPainter::background() const
{
   Q_D(const QPainter);

   if (!d->engine) {
      qWarning("QPainter::background() Painter engine not active");
      return d->fakeState()->brush;
   }

   return d->state->bgBrush;
}

bool QPainter::hasClipping() const
{
   Q_D(const QPainter);

   if (!d->engine) {
      qWarning("QPainter::hasClipping() Painter engine not active");
      return false;
   }

   return d->state->clipEnabled && d->state->clipOperation != Qt::NoClip;
}

void QPainter::setClipping(bool enable)
{
   Q_D(QPainter);

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::setClipping() enable = %s, was = %s", enable ? "on" : "off", hasClipping() ? "on" : "off");
#endif

   if (! d->engine) {
      qWarning("QPainter::setClipping() Painter engine not active");
      return;
   }

   if (hasClipping() == enable) {
      return;
   }

   if (enable && (d->state->clipInfo.isEmpty() || d->state->clipInfo.last().operation == Qt::NoClip)) {
      return;
   }
   d->state->clipEnabled = enable;

   if (d->extended) {
      d->extended->clipEnabledChanged();
      return;
   }

   d->state->dirtyFlags |= QPaintEngine::DirtyClipEnabled;
   d->updateState(d->state);
}

QRegion QPainter::clipRegion() const
{
   Q_D(const QPainter);

   if (!d->engine) {
      qWarning("QPainter::clipRegion() Painter engine not active");
      return QRegion();
   }

   QRegion region;
   bool lastWasNothing = true;

   if (! d->txinv) {
      const_cast<QPainter *>(this)->d_ptr->updateInvMatrix();
   }

   // ### Falcon: Use QPainterPath
   for (int i = 0; i < d->state->clipInfo.size(); ++i) {
      const QPainterClipInfo &info = d->state->clipInfo.at(i);
      switch (info.clipType) {

         case QPainterClipInfo::RegionClip: {
            QTransform matrix = (info.matrix * d->invMatrix);
            if (lastWasNothing) {
               region = info.region * matrix;
               lastWasNothing = false;
               continue;
            }
            if (info.operation == Qt::IntersectClip) {
               region &= info.region * matrix;

            } else if (info.operation == Qt::NoClip) {
               lastWasNothing = true;
               region = QRegion();

            } else {
               region = info.region * matrix;
            }
            break;
         }

         case QPainterClipInfo::PathClip: {
            QTransform matrix = (info.matrix * d->invMatrix);
            if (lastWasNothing) {
               region = QRegion((info.path * matrix).toFillPolygon().toPolygon(),
                     info.path.fillRule());
               lastWasNothing = false;
               continue;
            }
            if (info.operation == Qt::IntersectClip) {
               region &= QRegion((info.path * matrix).toFillPolygon().toPolygon(),
                     info.path.fillRule());

            } else if (info.operation == Qt::NoClip) {
               lastWasNothing = true;
               region = QRegion();

            } else {
               region = QRegion((info.path * matrix).toFillPolygon().toPolygon(),
                     info.path.fillRule());
            }
            break;
         }

         case QPainterClipInfo::RectClip: {
            QTransform matrix = (info.matrix * d->invMatrix);
            if (lastWasNothing) {
               region = QRegion(info.rect) * matrix;
               lastWasNothing = false;
               continue;
            }
            if (info.operation == Qt::IntersectClip) {
               // Use rect intersection if possible.
               if (matrix.type() <= QTransform::TxScale) {
                  region &= matrix.mapRect(info.rect);
               } else {
                  region &= matrix.map(QRegion(info.rect));
               }



            } else if (info.operation == Qt::NoClip) {
               lastWasNothing = true;
               region = QRegion();
            } else {
               region = QRegion(info.rect) * matrix;
            }
            break;
         }

         case QPainterClipInfo::RectFClip: {
            QTransform matrix = (info.matrix * d->invMatrix);
            if (lastWasNothing) {
               region = QRegion(info.rectf.toRect()) * matrix;
               lastWasNothing = false;
               continue;
            }
            if (info.operation == Qt::IntersectClip) {
               // Use rect intersection if possible.
               if (matrix.type() <= QTransform::TxScale) {
                  region &= matrix.mapRect(info.rectf.toRect());
               } else {
                  region &= matrix.map(QRegion(info.rectf.toRect()));
               }

            } else if (info.operation == Qt::NoClip) {
               lastWasNothing = true;
               region = QRegion();
            } else {
               region = QRegion(info.rectf.toRect()) * matrix;
            }
            break;
         }
      }
   }

   return region;
}

extern QPainterPath qt_regionToPath(const QRegion &region);

QPainterPath QPainter::clipPath() const
{
   Q_D(const QPainter);

   // ### Since we do not support path intersections and path unions yet,
   // we just use clipRegion() here...
   if (!d->engine) {
      qWarning("QPainter::clipPath() Painter engine not active");
      return QPainterPath();
   }

   // No clip, return empty
   if (d->state->clipInfo.size() == 0) {
      return QPainterPath();
   } else {

      // Update inverse matrix, used below.
      if (! d->txinv) {
         const_cast<QPainter *>(this)->d_ptr->updateInvMatrix();
      }

      // For the simple case avoid conversion.
      if (d->state->clipInfo.size() == 1
         && d->state->clipInfo.at(0).clipType == QPainterClipInfo::PathClip) {
         QTransform matrix = (d->state->clipInfo.at(0).matrix * d->invMatrix);
         return d->state->clipInfo.at(0).path * matrix;

      } else if (d->state->clipInfo.size() == 1
         && d->state->clipInfo.at(0).clipType == QPainterClipInfo::RectClip) {
         QTransform matrix = (d->state->clipInfo.at(0).matrix * d->invMatrix);
         QPainterPath path;
         path.addRect(d->state->clipInfo.at(0).rect);
         return path * matrix;
      } else {
         // Fallback to clipRegion() for now, since we don't have isect/unite for paths
         return qt_regionToPath(clipRegion());
      }
   }
}

QRectF QPainter::clipBoundingRect() const
{
   Q_D(const QPainter);

   if (!d->engine) {
      qWarning("QPainter::clipBoundingRect() Painter engine not active");
      return QRectF();
   }

   // Accumulate the bounding box in device space. This is not 100%
   // precise, but it fits within the guarantee and it is reasonably fast
   QRectF bounds;

   for (int i = 0; i < d->state->clipInfo.size(); ++i) {
      QRectF r;
      const QPainterClipInfo &info = d->state->clipInfo.at(i);

      if (info.clipType == QPainterClipInfo::RectClip) {
         r = info.rect;
      } else if (info.clipType == QPainterClipInfo::RectFClip) {
         r = info.rectf;
      } else if (info.clipType == QPainterClipInfo::RegionClip) {
         r = info.region.boundingRect();
      } else {
         r = info.path.boundingRect();
      }

      r = info.matrix.mapRect(r);

      if (i == 0) {
         bounds = r;

      } else if (info.operation == Qt::IntersectClip) {
         bounds &= r;

      }
   }

   // Map the rectangle back into logical space using the inverse matrix
   if (!d->txinv) {
      const_cast<QPainter *>(this)->d_ptr->updateInvMatrix();
   }

   return d->invMatrix.mapRect(bounds);
}

void QPainter::setClipRect(const QRectF &rect, Qt::ClipOperation op)
{
   Q_D(QPainter);

   if (d->extended) {

      if (!d->engine) {
         qWarning("QPainter::setClipRect() Painter engine not active");
         return;
      }

      bool simplifyClipOp = (paintEngine()->type() != QPaintEngine::Picture);
      if (simplifyClipOp && (!d->state->clipEnabled && op != Qt::NoClip)) {
         op = Qt::ReplaceClip;
      }
      qreal right = rect.x() + rect.width();
      qreal bottom = rect.y() + rect.height();
      qreal pts[] = { rect.x(), rect.y(),
            right, rect.y(),
            right, bottom,
            rect.x(), bottom
         };

      QVectorPath vp(pts, 4, nullptr, QVectorPath::RectangleHint);
      d->state->clipEnabled = true;
      d->extended->clip(vp, op);

      if (op == Qt::ReplaceClip || op == Qt::NoClip) {
         d->state->clipInfo.clear();
      }

      d->state->clipInfo << QPainterClipInfo(rect, op, d->state->matrix);
      d->state->clipOperation = op;
      return;
   }

   if (qreal(int(rect.top())) == rect.top()
      && qreal(int(rect.bottom())) == rect.bottom()
      && qreal(int(rect.left())) == rect.left()
      && qreal(int(rect.right())) == rect.right()) {
      setClipRect(rect.toRect(), op);
      return;
   }

   if (rect.isEmpty()) {
      setClipRegion(QRegion(), op);
      return;
   }

   QPainterPath path;
   path.addRect(rect);
   setClipPath(path, op);
}

void QPainter::setClipRect(const QRect &rect, Qt::ClipOperation op)
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::setClipRect() Painter engine not active");
      return;
   }
   bool simplifyClipOp = (paintEngine()->type() != QPaintEngine::Picture);

   if (simplifyClipOp && (!d->state->clipEnabled && op != Qt::NoClip)) {
      op = Qt::ReplaceClip;
   }

   if (d->extended) {
      d->state->clipEnabled = true;
      d->extended->clip(rect, op);
      if (op == Qt::ReplaceClip || op == Qt::NoClip) {
         d->state->clipInfo.clear();
      }
      d->state->clipInfo << QPainterClipInfo(rect, op, d->state->matrix);
      d->state->clipOperation = op;
      return;
   }

   if (simplifyClipOp && d->state->clipOperation == Qt::NoClip && op == Qt::IntersectClip)  {
      op = Qt::ReplaceClip;
   }

   d->state->clipRegion = rect;
   d->state->clipOperation = op;
   if (op == Qt::NoClip || op == Qt::ReplaceClip) {
      d->state->clipInfo.clear();
   }
   d->state->clipInfo << QPainterClipInfo(rect, op, d->state->matrix);
   d->state->clipEnabled = true;
   d->state->dirtyFlags |= QPaintEngine::DirtyClipRegion | QPaintEngine::DirtyClipEnabled;
   d->updateState(d->state);
}

void QPainter::setClipRegion(const QRegion &r, Qt::ClipOperation op)
{
   Q_D(QPainter);

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   QRect rect = r.boundingRect();

   qDebug("QPainter::setClipRegion() size = %lld, bounding rectangle = [%d,%d,%d,%d]",
         r.rects().size(), rect.x(), rect.y(), rect.width(), rect.height());
#endif

   if (! d->engine) {
      qWarning("QPainter::setClipRegion() Painter engine not active");
      return;
   }

   bool simplifyClipOp = (paintEngine()->type() != QPaintEngine::Picture);

   if (simplifyClipOp && (!d->state->clipEnabled && op != Qt::NoClip)) {
      op = Qt::ReplaceClip;
   }

   if (d->extended) {
      d->state->clipEnabled = true;
      d->extended->clip(r, op);
      if (op == Qt::NoClip || op == Qt::ReplaceClip) {
         d->state->clipInfo.clear();
      }
      d->state->clipInfo << QPainterClipInfo(r, op, d->state->matrix);
      d->state->clipOperation = op;
      return;
   }

   if (simplifyClipOp && d->state->clipOperation == Qt::NoClip && op == Qt::IntersectClip) {
      op = Qt::ReplaceClip;
   }

   d->state->clipRegion = r;
   d->state->clipOperation = op;
   if (op == Qt::NoClip || op == Qt::ReplaceClip) {
      d->state->clipInfo.clear();
   }

   d->state->clipInfo << QPainterClipInfo(r, op, d->state->matrix);
   d->state->clipEnabled = true;
   d->state->dirtyFlags |= QPaintEngine::DirtyClipRegion | QPaintEngine::DirtyClipEnabled;
   d->updateState(d->state);
}

void QPainter::setWorldMatrix(const QMatrix &matrix, bool combine)
{
   setWorldTransform(QTransform(matrix), combine);
}

// obsolete
const QMatrix &QPainter::worldMatrix() const
{
   Q_D(const QPainter);
   if (!d->engine) {
      qWarning("QPainter::worldMatrix() Painter engine not active");
      return d->fakeState()->transform.toAffine();
   }
   return d->state->worldMatrix.toAffine();
}

// obsolete
void QPainter::setMatrix(const QMatrix &matrix, bool combine)
{
   setWorldTransform(QTransform(matrix), combine);
}

// obsolete
const QMatrix &QPainter::matrix() const
{
   return worldMatrix();
}

// obsolete
QMatrix QPainter::combinedMatrix() const
{
   return combinedTransform().toAffine();
}

// obsolete
const QMatrix &QPainter::deviceMatrix() const
{
   Q_D(const QPainter);
   if (!d->engine) {
      qWarning("QPainter::deviceMatrix() Painter engine not active");
      return d->fakeState()->transform.toAffine();
   }
   return d->state->matrix.toAffine();
}

// obsolete
void QPainter::resetMatrix()
{
   resetTransform();
}

void QPainter::setWorldMatrixEnabled(bool enable)
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::setMatrixEnabled() Painter engine not active");
      return;
   }

   if (enable == d->state->WxF) {
      return;
   }

   d->state->WxF = enable;
   d->updateMatrix();
}

bool QPainter::worldMatrixEnabled() const
{
   Q_D(const QPainter);
   if (!d->engine) {
      qWarning("QPainter::worldMatrixEnabled() Painter engine not active");
      return false;
   }
   return d->state->WxF;
}

// obsolete
void QPainter::setMatrixEnabled(bool enable)
{
   setWorldMatrixEnabled(enable);
}

// obsolete
bool QPainter::matrixEnabled() const
{
   return worldMatrixEnabled();
}

void QPainter::scale(qreal sx, qreal sy)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::scale() Scale factor, sx = %f, sy = %f", sx, sy);
#endif

   Q_D(QPainter);
   if (!d->engine) {
      qWarning("QPainter::scale() Painter engine not active");
      return;
   }

   d->state->worldMatrix.scale(sx, sy);
   d->state->WxF = true;
   d->updateMatrix();
}

void QPainter::shear(qreal sh, qreal sv)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::shear() Shear factor, sh = %f, sv = %f", sh, sv);
#endif

   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::shear() Painter engine not active");
      return;
   }

   d->state->worldMatrix.shear(sh, sv);
   d->state->WxF = true;
   d->updateMatrix();
}

void QPainter::rotate(qreal a)
{
   Q_D(QPainter);

   if (! d->engine) {
      qWarning("QPainter::rotate() Painter engine not active");
      return;
   }

   d->state->worldMatrix.rotate(a);
   d->state->WxF = true;
   d->updateMatrix();
}

void QPainter::translate(const QPointF &offset)
{
   qreal dx = offset.x();
   qreal dy = offset.y();

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::translate() Offset dx = %f, dy = %f", dx, dy);
#endif

   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::translate() Painter engine not active");
      return;
   }

   d->state->worldMatrix.translate(dx, dy);
   d->state->WxF = true;
   d->updateMatrix();
}

void QPainter::setClipPath(const QPainterPath &path, Qt::ClipOperation op)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   QRectF b = path.boundingRect();

   qDebug("QPainter::setClipPath() size = %d, op = %d, bounding rectangle = [%.2f,%.2f,%.2f,%.2f]",
         path.elementCount(), op, b.x(), b.y(), b.width(), b.height());
#endif

   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::setClipPath() Painter engine not active");
      return;
   }

   if ((!d->state->clipEnabled && op != Qt::NoClip)) {
      op = Qt::ReplaceClip;
   }

   if (d->extended) {
      d->state->clipEnabled = true;
      d->extended->clip(path, op);
      if (op == Qt::NoClip || op == Qt::ReplaceClip) {
         d->state->clipInfo.clear();
      }
      d->state->clipInfo << QPainterClipInfo(path, op, d->state->matrix);
      d->state->clipOperation = op;
      return;
   }

   if (d->state->clipOperation == Qt::NoClip && op == Qt::IntersectClip) {
      op = Qt::ReplaceClip;
   }

   d->state->clipPath = path;
   d->state->clipOperation = op;
   if (op == Qt::NoClip || op == Qt::ReplaceClip) {
      d->state->clipInfo.clear();
   }
   d->state->clipInfo << QPainterClipInfo(path, op, d->state->matrix);
   d->state->clipEnabled = true;
   d->state->dirtyFlags |= QPaintEngine::DirtyClipPath | QPaintEngine::DirtyClipEnabled;
   d->updateState(d->state);
}

void QPainter::strokePath(const QPainterPath &path, const QPen &pen)
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::strokePath() Painter engine not active");
      return;
   }

   if (path.isEmpty()) {
      return;
   }

   if (d->extended) {
      const QGradient *g = qpen_brush(pen).gradient();
      if (!g || g->coordinateMode() == QGradient::LogicalMode) {
         d->extended->stroke(qtVectorPathForPath(path), pen);
         return;
      }
   }

   QBrush oldBrush = d->state->brush;
   QPen oldPen = d->state->pen;

   setPen(pen);
   setBrush(Qt::NoBrush);

   drawPath(path);

   // Reset old state
   setPen(oldPen);
   setBrush(oldBrush);
}

void QPainter::fillPath(const QPainterPath &path, const QBrush &brush)
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::fillPath() Painter engine not active");
      return;
   }

   if (path.isEmpty()) {
      return;
   }

   if (d->extended) {
      const QGradient *g = brush.gradient();
      if (!g || g->coordinateMode() == QGradient::LogicalMode) {
         d->extended->fill(qtVectorPathForPath(path), brush);
         return;
      }
   }

   QBrush oldBrush = d->state->brush;
   QPen oldPen = d->state->pen;

   setPen(Qt::NoPen);
   setBrush(brush);

   drawPath(path);

   // Reset old state
   setPen(oldPen);
   setBrush(oldBrush);
}

void QPainter::drawPath(const QPainterPath &path)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   QRectF pathBounds = path.boundingRect();

   qDebug("QPainter::drawPath() size = %d, bounding rectangle = [%.2f,%.2f,%.2f,%.2f]",
         path.elementCount(), pathBounds.x(), pathBounds.y(), pathBounds.width(), pathBounds.height());
#endif

   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::drawPath() Painter engine not active");
      return;
   }

   if (d->extended) {
      d->extended->drawPath(path);
      return;
   }
   d->updateState(d->state);

   if (d->engine->hasFeature(QPaintEngine::PainterPaths) && d->state->emulationSpecifier == 0) {
      d->engine->drawPath(path);
   } else {
      d->draw_helper(path);
   }
}

void QPainter::drawRects(const QRectF *rectPtr, int rectCount)
{
   Q_D(QPainter);

   if (! d->engine) {
      qWarning("QPainter::drawRects() Painter engine not active");
      return;
   }

   if (rectCount <= 0) {
      return;
   }

   if (d->extended) {
      d->extended->drawRects(rectPtr, rectCount);
      return;
   }

   d->updateState(d->state);

   if (! d->state->emulationSpecifier) {
      d->engine->drawRects(rectPtr, rectCount);
      return;
   }

   if (d->state->emulationSpecifier == QPaintEngine::PrimitiveTransform
         && d->state->matrix.type() == QTransform::TxTranslate) {

      for (int i = 0; i < rectCount; ++i) {
         QRectF r(rectPtr[i].x() + d->state->matrix.dx(), rectPtr[i].y() + d->state->matrix.dy(),
               rectPtr[i].width(), rectPtr[i].height());

         d->engine->drawRects(&r, 1);
      }

   } else {
      if (d->state->brushNeedsResolving() || d->state->penNeedsResolving()) {
         for (int i = 0; i < rectCount; ++i) {
            QPainterPath rectPath;
            rectPath.addRect(rectPtr[i]);
            d->draw_helper(rectPath, QPainterPrivate::StrokeAndFillDraw);
         }

      } else {
         QPainterPath rectPath;
         for (int i = 0; i < rectCount; ++i) {
            rectPath.addRect(rectPtr[i]);
         }

         d->draw_helper(rectPath, QPainterPrivate::StrokeAndFillDraw);
      }
   }
}

void QPainter::drawRects(const QRect *rectPtr, int rectCount)
{
   Q_D(QPainter);

   if (! d->engine) {
      qWarning("QPainter::drawRects() Painter engine not active");
      return;
   }

   if (rectCount <= 0) {
      return;
   }

   if (d->extended) {
      d->extended->drawRects(rectPtr, rectCount);
      return;
   }

   d->updateState(d->state);

   if (! d->state->emulationSpecifier) {
      d->engine->drawRects(rectPtr, rectCount);
      return;
   }

   if (d->state->emulationSpecifier == QPaintEngine::PrimitiveTransform
         && d->state->matrix.type() == QTransform::TxTranslate) {

      for (int i = 0; i < rectCount; ++i) {
         QRectF rTmp(rectPtr[i].x() + d->state->matrix.dx(), rectPtr[i].y() + d->state->matrix.dy(),
            rectPtr[i].width(), rectPtr[i].height());

         d->engine->drawRects(&rTmp, 1);
      }

   } else {
      if (d->state->brushNeedsResolving() || d->state->penNeedsResolving()) {
         for (int i = 0; i < rectCount; ++i) {
            QPainterPath rectPath;
            rectPath.addRect(rectPtr[i]);
            d->draw_helper(rectPath, QPainterPrivate::StrokeAndFillDraw);
         }

      } else {
         QPainterPath rectPath;
         for (int i = 0; i < rectCount; ++i) {
            rectPath.addRect(rectPtr[i]);
         }

         d->draw_helper(rectPath, QPainterPrivate::StrokeAndFillDraw);
      }
   }
}

void QPainter::drawPoints(const QPointF *pointPtr, int pointCount)
{
   Q_D(QPainter);

   if (! d->engine) {
      qWarning("QPainter::drawPoints() Painter engine not active");
      return;
   }

   if (pointCount <= 0) {
      return;
   }

   if (d->extended) {
      d->extended->drawPoints(pointPtr, pointCount);
      return;
   }

   d->updateState(d->state);

   if (! d->state->emulationSpecifier) {
      d->engine->drawPoints(pointPtr, pointCount);
      return;
   }

   if (d->state->emulationSpecifier == QPaintEngine::PrimitiveTransform
         && d->state->matrix.type() == QTransform::TxTranslate) {

      // ### use drawPoints function
      for (int i = 0; i < pointCount; ++i) {
         QPointF pt(pointPtr[i].x() + d->state->matrix.dx(), pointPtr[i].y() + d->state->matrix.dy());
         d->engine->drawPoints(&pt, 1);
      }

   } else {
      QPen pen = d->state->pen;
      bool flat_pen = pen.capStyle() == Qt::FlatCap;
      if (flat_pen) {
         save();
         pen.setCapStyle(Qt::SquareCap);
         setPen(pen);
      }

      QPainterPath path;
      for (int i = 0; i < pointCount; ++i) {
         path.moveTo(pointPtr[i].x(), pointPtr[i].y());
         path.lineTo(pointPtr[i].x() + 0.0001, pointPtr[i].y());
      }

      d->draw_helper(path, QPainterPrivate::StrokeDraw);

      if (flat_pen) {
         restore();
      }
   }
}

void QPainter::drawPoints(const QPoint *pointPtr, int pointCount)
{
   Q_D(QPainter);

   if (! d->engine) {
      qWarning("QPainter::drawPoints() Painter engine not active");
      return;
   }

   if (pointCount <= 0) {
      return;
   }

   if (d->extended) {
      d->extended->drawPoints(pointPtr, pointCount);
      return;
   }

   d->updateState(d->state);

   if (! d->state->emulationSpecifier) {
      d->engine->drawPoints(pointPtr, pointCount);
      return;
   }

   if (d->state->emulationSpecifier == QPaintEngine::PrimitiveTransform
         && d->state->matrix.type() == QTransform::TxTranslate) {

      // ### use drawPoints function
      for (int i = 0; i < pointCount; ++i) {
         QPointF pt(pointPtr[i].x() + d->state->matrix.dx(), pointPtr[i].y() + d->state->matrix.dy());
         d->engine->drawPoints(&pt, 1);
      }

   } else {
      QPen pen = d->state->pen;
      bool flat_pen = (pen.capStyle() == Qt::FlatCap);
      if (flat_pen) {
         save();
         pen.setCapStyle(Qt::SquareCap);
         setPen(pen);
      }
      QPainterPath path;
      for (int i = 0; i < pointCount; ++i) {
         path.moveTo(pointPtr[i].x(), pointPtr[i].y());
         path.lineTo(pointPtr[i].x() + 0.0001, pointPtr[i].y());
      }

      d->draw_helper(path, QPainterPrivate::StrokeDraw);
      if (flat_pen) {
         restore();
      }
   }
}

void QPainter::setBackgroundMode(Qt::BGMode mode)
{
   Q_D(QPainter);

   if (! d->engine) {
      qWarning("QPainter::setBackgroundMode() Painter engine not active");
      return;
   }
   if (d->state->bgMode == mode) {
      return;
   }

   d->state->bgMode = mode;

   if (d->extended) {
      d->checkEmulation();
   } else {
      d->state->dirtyFlags |= QPaintEngine::DirtyBackgroundMode;
   }
}

Qt::BGMode QPainter::backgroundMode() const
{
   Q_D(const QPainter);

   if (!d->engine) {
      qWarning("QPainter::backgroundMode() Painter engine not active");
      return Qt::TransparentMode;
   }

   return d->state->bgMode;
}

void QPainter::setPen(const QColor &color)
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::setPen() Painter engine not active");
      return;
   }

   QPen pen(color.isValid() ? color : QColor(Qt::black));
   if (d->state->pen == pen) {
      return;
   }

   d->state->pen = pen;
   if (d->extended) {
      d->extended->penChanged();
   } else {
      d->state->dirtyFlags |= QPaintEngine::DirtyPen;
   }
}

void QPainter::setPen(const QPen &pen)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::setPen() color = %04x, brushStyle = %d, style = %d, cap = %d, join = %d",
         pen.color().rgb(), pen.brush().style(), pen.style(), pen.capStyle(), pen.joinStyle());
#endif

   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::setPen() Painter engine not active");
      return;
   }

   if (d->state->pen == pen) {
      return;
   }

   d->state->pen = pen;

   if (d->extended) {
      d->checkEmulation();
      d->extended->penChanged();
      return;
   }

   d->state->dirtyFlags |= QPaintEngine::DirtyPen;
}

void QPainter::setPen(Qt::PenStyle style)
{
   Q_D(QPainter);
   if (!d->engine) {
      qWarning("QPainter::setPen() Painter engine not active");
      return;
   }

   QPen pen = QPen(style);

   if (d->state->pen == pen) {
      return;
   }

   d->state->pen = pen;
   if (d->extended) {
      d->extended->penChanged();
   } else {
      d->state->dirtyFlags |= QPaintEngine::DirtyPen;
   }

}

const QPen &QPainter::pen() const
{
   Q_D(const QPainter);

   if (!d->engine) {
      qWarning("QPainter::pen() Painter engine not active");
      return d->fakeState()->pen;
   }

   return d->state->pen;
}

void QPainter::setBrush(const QBrush &brush)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::setBrush() color = %04x, style = %d", brush.color().rgb(), brush.style());
#endif

   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::setBrush() Painter engine not active");
      return;
   }

   if (d->state->brush.d == brush.d) {
      return;
   }

   if (d->extended) {
      d->state->brush = brush;
      d->checkEmulation();
      d->extended->brushChanged();
      return;
   }

   d->state->brush = brush;
   d->state->dirtyFlags |= QPaintEngine::DirtyBrush;
}

void QPainter::setBrush(Qt::BrushStyle style)
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::setBrush() Painter engine not active");
      return;
   }

   if (d->state->brush.style() == style &&
         (style == Qt::NoBrush || (style == Qt::SolidPattern && d->state->brush.color() == QColor(0, 0, 0)))) {
      return;
   }

   d->state->brush = QBrush(Qt::black, style);

   if (d->extended) {
      d->extended->brushChanged();
   } else {
      d->state->dirtyFlags |= QPaintEngine::DirtyBrush;
   }
}

const QBrush &QPainter::brush() const
{
   Q_D(const QPainter);

   if (! d->engine) {
      qWarning("QPainter::brush() Painter engine not active");
      return d->fakeState()->brush;
   }

   return d->state->brush;
}

void QPainter::setBackground(const QBrush &bg)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::setBackground() color = %04x, style = %d", bg.color().rgb(), bg.style());
#endif

   Q_D(QPainter);

   if (! d->engine) {
      qWarning("QPainter::setBackground() Painter engine not active");
      return;
   }

   d->state->bgBrush = bg;

   if (!d->extended) {
      d->state->dirtyFlags |= QPaintEngine::DirtyBackground;
   }
}

void QPainter::setFont(const QFont &font)
{
   Q_D(QPainter);

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::setFont() family = %s, pointSize = %d", csPrintable(font.family()), font.pointSize());
#endif

   if (! d->engine) {
      qWarning("QPainter::setFont() Painter engine not active");
      return;
   }

   d->state->font = QFont(font.resolve(d->state->deviceFont), device());

   if (! d->extended) {
      d->state->dirtyFlags |= QPaintEngine::DirtyFont;
   }
}

const QFont &QPainter::font() const
{
   Q_D(const QPainter);

   if (! d->engine) {
      qWarning("QPainter::font() Painter engine not active");
      return d->fakeState()->font;
   }

   return d->state->font;
}

void QPainter::drawRoundedRect(const QRectF &rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawRoundedRect() bounding rectangle = [%.2f,%.2f,%.2f,%.2f]",
         rect.x(), rect.y(), rect.width(), rect.height());
#endif

   Q_D(QPainter);

   if (!d->engine) {
      return;
   }

   if (xRadius <= 0 || yRadius <= 0) {             // draw normal rectangle
      drawRect(rect);
      return;
   }

   if (d->extended) {
      d->extended->drawRoundedRect(rect, xRadius, yRadius, mode);
      return;
   }

   QPainterPath path;
   path.addRoundedRect(rect, xRadius, yRadius, mode);
   drawPath(path);
}

void QPainter::drawRoundRect(const QRectF &r, int xRnd, int yRnd)
{
   drawRoundedRect(r, xRnd, yRnd, Qt::RelativeSize);
}

void QPainter::drawEllipse(const QRectF &r)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawEllipse() bounding rectangle = [%.2f,%.2f,%.2f,%.2f]",
         r.x(), r.y(), r.width(), r.height());
#endif

   Q_D(QPainter);

   if (! d->engine) {
      return;
   }

   QRectF rect(r.normalized());

   if (d->extended) {
      d->extended->drawEllipse(rect);
      return;
   }

   d->updateState(d->state);
   if (d->state->emulationSpecifier) {
      if (d->state->emulationSpecifier == QPaintEngine::PrimitiveTransform
         && d->state->matrix.type() == QTransform::TxTranslate) {
         rect.translate(QPointF(d->state->matrix.dx(), d->state->matrix.dy()));
      } else {
         QPainterPath path;
         path.addEllipse(rect);
         d->draw_helper(path, QPainterPrivate::StrokeAndFillDraw);
         return;
      }
   }

   d->engine->drawEllipse(rect);
}

void QPainter::drawEllipse(const QRect &r)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawEllipse() bounding rectangle = [%d,%d,%d,%d]", r.x(), r.y(), r.width(), r.height());
#endif

   Q_D(QPainter);

   if (!d->engine) {
      return;
   }

   QRect rect(r.normalized());

   if (d->extended) {
      d->extended->drawEllipse(rect);
      return;
   }

   d->updateState(d->state);

   if (d->state->emulationSpecifier) {
      if (d->state->emulationSpecifier == QPaintEngine::PrimitiveTransform
         && d->state->matrix.type() == QTransform::TxTranslate) {
         rect.translate(QPoint(qRound(d->state->matrix.dx()), qRound(d->state->matrix.dy())));
      } else {
         QPainterPath path;
         path.addEllipse(rect);
         d->draw_helper(path, QPainterPrivate::StrokeAndFillDraw);
         return;
      }
   }

   d->engine->drawEllipse(rect);
}

void QPainter::drawArc(const QRectF &r, int a, int alen)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawArc() bounding rectangle = [%.2f,%.2f,%.2f,%.2f], angle = %d, sweep = %d",
         r.x(), r.y(), r.width(), r.height(), a / 16, alen / 16);
#endif

   Q_D(QPainter);

   if (! d->engine) {
      return;
   }

   QRectF rect = r.normalized();

   QPainterPath path;
   path.arcMoveTo(rect, a / 16.0);
   path.arcTo(rect, a / 16.0, alen / 16.0);
   strokePath(path, d->state->pen);
}

void QPainter::drawPie(const QRectF &r, int a, int alen)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawPie() bounding rect = [%.2f,%.2f,%.2f,%.2f], angle = %d, sweep = %d",
         r.x(), r.y(), r.width(), r.height(), a / 16, alen / 16);
#endif

   Q_D(QPainter);

   if (!d->engine) {
      return;
   }

   if (a > (360 * 16)) {
      a = a % (360 * 16);

   } else if (a < 0) {
      a = a % (360 * 16);

      if (a < 0) {
         a += (360 * 16);
      }
   }

   QRectF rect = r.normalized();

   QPainterPath path;
   path.moveTo(rect.center());
   path.arcTo(rect.x(), rect.y(), rect.width(), rect.height(), a / 16.0, alen / 16.0);
   path.closeSubpath();
   drawPath(path);

}

void QPainter::drawChord(const QRectF &r, int a, int alen)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawChord() bounding rectangle = [%.2f,%.2f,%.2f,%.2f], angle = %d, sweep = %d",
         r.x(), r.y(), r.width(), r.height(), a / 16, alen / 16);
#endif

   Q_D(QPainter);

   if (!d->engine) {
      return;
   }

   QRectF rect = r.normalized();

   QPainterPath path;
   path.arcMoveTo(rect, a / 16.0);
   path.arcTo(rect, a / 16.0, alen / 16.0);
   path.closeSubpath();
   drawPath(path);
}

void QPainter::drawLines(const QLineF *lines, int lineCount)
{
   Q_D(QPainter);

   if (!d->engine || lineCount < 1) {
      return;
   }

   if (d->extended) {
      d->extended->drawLines(lines, lineCount);
      return;
   }

   d->updateState(d->state);

   uint lineEmulation = line_emulation(d->state->emulationSpecifier);

   if (lineEmulation) {
      if (lineEmulation == QPaintEngine::PrimitiveTransform
         && d->state->matrix.type() == QTransform::TxTranslate) {
         for (int i = 0; i < lineCount; ++i) {
            QLineF line = lines[i];
            line.translate(d->state->matrix.dx(), d->state->matrix.dy());
            d->engine->drawLines(&line, 1);
         }
      } else {
         QPainterPath linePath;
         for (int i = 0; i < lineCount; ++i) {
            linePath.moveTo(lines[i].p1());
            linePath.lineTo(lines[i].p2());
         }
         d->draw_helper(linePath, QPainterPrivate::StrokeDraw);
      }
      return;
   }

   d->engine->drawLines(lines, lineCount);
}

void QPainter::drawLines(const QLine *lines, int lineCount)
{
   Q_D(QPainter);

   if (!d->engine || lineCount < 1) {
      return;
   }

   if (d->extended) {
      d->extended->drawLines(lines, lineCount);
      return;
   }

   d->updateState(d->state);

   uint lineEmulation = line_emulation(d->state->emulationSpecifier);

   if (lineEmulation) {
      if (lineEmulation == QPaintEngine::PrimitiveTransform
         && d->state->matrix.type() == QTransform::TxTranslate) {
         for (int i = 0; i < lineCount; ++i) {
            QLineF line = lines[i];
            line.translate(d->state->matrix.dx(), d->state->matrix.dy());
            d->engine->drawLines(&line, 1);
         }
      } else {
         QPainterPath linePath;
         for (int i = 0; i < lineCount; ++i) {
            linePath.moveTo(lines[i].p1());
            linePath.lineTo(lines[i].p2());
         }
         d->draw_helper(linePath, QPainterPrivate::StrokeDraw);
      }
      return;
   }
   d->engine->drawLines(lines, lineCount);
}

void QPainter::drawLines(const QPointF *pointPairs, int lineCount)
{
   Q_ASSERT(sizeof(QLineF) == 2 * sizeof(QPointF));

   drawLines((const QLineF *)pointPairs, lineCount);
}

void QPainter::drawLines(const QPoint *pointPairs, int lineCount)
{
   Q_ASSERT(sizeof(QLine) == 2 * sizeof(QPoint));

   drawLines((const QLine *)pointPairs, lineCount);
}

void QPainter::drawPolyline(const QPointF *pointPtr, int pointCount)
{
   Q_D(QPainter);

   if (! d->engine || pointCount < 2) {
      return;
   }

   if (d->extended) {
      d->extended->drawPolygon(pointPtr, pointCount, QPaintEngine::PolylineMode);
      return;
   }

   d->updateState(d->state);

   uint lineEmulation = line_emulation(d->state->emulationSpecifier);

   if (lineEmulation) {

      //         if (lineEmulation == QPaintEngine::PrimitiveTransform
      //              && d->state->matrix.type() == QTransform::TxTranslate) {
      //         } else {

      QPainterPath polylinePath(pointPtr[0]);
      for (int i = 1; i < pointCount; ++i) {
         polylinePath.lineTo(pointPtr[i]);
      }

      d->draw_helper(polylinePath, QPainterPrivate::StrokeDraw);

      //         }

   } else {
      d->engine->drawPolygon(pointPtr, pointCount, QPaintEngine::PolylineMode);
   }
}

void QPainter::drawPolyline(const QPoint *pointPtr, int pointCount)
{
   Q_D(QPainter);

   if (! d->engine || pointCount < 2) {
      return;
   }

   if (d->extended) {
      d->extended->drawPolygon(pointPtr, pointCount, QPaintEngine::PolylineMode);
      return;
   }

   d->updateState(d->state);

   uint lineEmulation = line_emulation(d->state->emulationSpecifier);

   if (lineEmulation) {
      // ###
      //         if (lineEmulation == QPaintEngine::PrimitiveTransform
      //             && d->state->matrix.type() == QTransform::TxTranslate) {
      //         } else {

      QPainterPath polylinePath(pointPtr[0]);
      for (int i = 1; i < pointCount; ++i) {
         polylinePath.lineTo(pointPtr[i]);
      }

      d->draw_helper(polylinePath, QPainterPrivate::StrokeDraw);

      //         }

   } else {
      d->engine->drawPolygon(pointPtr, pointCount, QPaintEngine::PolylineMode);
   }
}

void QPainter::drawPolygon(const QPointF *pointPtr, int pointCount, Qt::FillRule fillRule)
{
   Q_D(QPainter);

   if (! d->engine || pointCount < 2) {
      return;
   }

   if (d->extended) {
      d->extended->drawPolygon(pointPtr, pointCount, QPaintEngine::PolygonDrawMode(fillRule));
      return;
   }

   d->updateState(d->state);

   uint emulationSpecifier = d->state->emulationSpecifier;

   if (emulationSpecifier) {
      QPainterPath polygonPath(pointPtr[0]);
      for (int i = 1; i < pointCount; ++i) {
         polygonPath.lineTo(pointPtr[i]);
      }

      polygonPath.closeSubpath();
      polygonPath.setFillRule(fillRule);

      d->draw_helper(polygonPath);
      return;
   }

   d->engine->drawPolygon(pointPtr, pointCount, QPaintEngine::PolygonDrawMode(fillRule));
}

void QPainter::drawPolygon(const QPoint *pointPtr, int pointCount, Qt::FillRule fillRule)
{
   Q_D(QPainter);

   if (! d->engine || pointCount < 2) {
      return;
   }

   if (d->extended) {
      d->extended->drawPolygon(pointPtr, pointCount, QPaintEngine::PolygonDrawMode(fillRule));
      return;
   }

   d->updateState(d->state);

   uint emulationSpecifier = d->state->emulationSpecifier;

   if (emulationSpecifier) {
      QPainterPath polygonPath(pointPtr[0]);

      for (int i = 1; i < pointCount; ++i) {
         polygonPath.lineTo(pointPtr[i]);
      }
      polygonPath.closeSubpath();

      polygonPath.setFillRule(fillRule);
      d->draw_helper(polygonPath);

      return;
   }

   d->engine->drawPolygon(pointPtr, pointCount, QPaintEngine::PolygonDrawMode(fillRule));
}

void QPainter::drawConvexPolygon(const QPoint *pointPtr, int pointCount)
{
   Q_D(QPainter);

   if (! d->engine || pointCount < 2) {
      return;
   }

   if (d->extended) {
      d->extended->drawPolygon(pointPtr, pointCount, QPaintEngine::ConvexMode);
      return;
   }

   d->updateState(d->state);

   uint emulationSpecifier = d->state->emulationSpecifier;

   if (emulationSpecifier) {
      QPainterPath polygonPath(pointPtr[0]);

      for (int i = 1; i < pointCount; ++i) {
         polygonPath.lineTo(pointPtr[i]);
      }

      polygonPath.closeSubpath();
      polygonPath.setFillRule(Qt::WindingFill);

      d->draw_helper(polygonPath);

      return;
   }

   d->engine->drawPolygon(pointPtr, pointCount, QPaintEngine::ConvexMode);
}

void QPainter::drawConvexPolygon(const QPointF *pointPtr, int pointCount)
{
   Q_D(QPainter);

   if (! d->engine || pointCount < 2) {
      return;
   }

   if (d->extended) {
      d->extended->drawPolygon(pointPtr, pointCount, QPaintEngine::ConvexMode);
      return;
   }

   d->updateState(d->state);

   uint emulationSpecifier = d->state->emulationSpecifier;

   if (emulationSpecifier) {
      QPainterPath polygonPath(pointPtr[0]);

      for (int i = 1; i < pointCount; ++i) {
         polygonPath.lineTo(pointPtr[i]);
      }

      polygonPath.closeSubpath();
      polygonPath.setFillRule(Qt::WindingFill);
      d->draw_helper(polygonPath);

      return;
   }

   d->engine->drawPolygon(pointPtr, pointCount, QPaintEngine::ConvexMode);
}

static inline QPointF roundInDeviceCoordinates(const QPointF &p, const QTransform &m)
{
   return m.inverted().map(QPointF(m.map(p).toPoint()));
}

void QPainter::drawPixmap(const QPointF &p, const QPixmap &pm)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawPixmap() pointf = [%.2f,%.2f], pixmap = [%d,%d]",
         p.x(), p.y(), pm.width(), pm.height());
#endif

   Q_D(QPainter);

   if (! d->engine || pm.isNull()) {
      return;
   }

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qt_painter_thread_test(d->device->devType(), d->engine->type(), "drawPixmap()");
#endif

   if (d->extended) {
      d->extended->drawPixmap(p, pm);
      return;
   }

   qreal x = p.x();
   qreal y = p.y();

   int w = pm.width();
   int h = pm.height();

   if (w <= 0) {
      return;
   }

   // Emulate opaque background for bitmaps
   if (d->state->bgMode == Qt::OpaqueMode && pm.isQBitmap()) {
      fillRect(QRectF(x, y, w, h), d->state->bgBrush.color());
   }

   d->updateState(d->state);

   if ((d->state->matrix.type() > QTransform::TxTranslate
         && !d->engine->hasFeature(QPaintEngine::PixmapTransform))
      || (!d->state->matrix.isAffine() && !d->engine->hasFeature(QPaintEngine::PerspectiveTransform))
      || (d->state->opacity != 1.0 && !d->engine->hasFeature(QPaintEngine::ConstantOpacity))) {
      save();
      // If there is no rotation involved we have to make sure we use the
      // antialiased and not the aliased coordinate system by rounding the coordinates.
      if (d->state->matrix.type() <= QTransform::TxScale) {
         const QPointF p = roundInDeviceCoordinates(QPointF(x, y), d->state->matrix);
         x = p.x();
         y = p.y();
      }
      translate(x, y);
      setBackgroundMode(Qt::TransparentMode);
      setRenderHint(Antialiasing, renderHints() & SmoothPixmapTransform);
      QBrush brush(d->state->pen.color(), pm);
      setBrush(brush);
      setPen(Qt::NoPen);
      setBrushOrigin(QPointF(0, 0));

      drawRect(pm.rect());
      restore();

   } else {
      if (!d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
         x += d->state->matrix.dx();
         y += d->state->matrix.dy();
      }

      qreal scale = pm.devicePixelRatio();
      d->engine->drawPixmap(QRectF(x, y, w / scale, h / scale), pm, QRectF(0, 0, w, h));
   }
}

void QPainter::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawPixmap() target rectangle = [%.2f,%.2f,%.2f,%.2f], pixmap = [%d,%d], source rectangle = [%.2f,%.2f,%.2f,%.2f]",
         r.x(), r.y(), r.width(), r.height(), pm.width(), pm.height(), sr.x(), sr.y(), sr.width(), sr.height());
#endif

   Q_D(QPainter);

   if (! d->engine || pm.isNull()) {
      return;
   }

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qt_painter_thread_test(d->device->devType(), d->engine->type(), "drawPixmap()");
#endif

   qreal x = r.x();
   qreal y = r.y();
   qreal w = r.width();
   qreal h = r.height();
   qreal sx = sr.x();
   qreal sy = sr.y();
   qreal sw = sr.width();
   qreal sh = sr.height();

   const qreal pmscale = pm.devicePixelRatio();

   // Sanity-check clipping
   if (sw <= 0) {
      sw = pm.width() - sx;
   }

   if (sh <= 0) {
      sh = pm.height() - sy;
   }

   if (w < 0) {
      w = sw / pmscale;
   }

   if (h < 0) {
      h = sh / pmscale;
   }

   if (sx < 0) {
      qreal w_ratio = sx * w / sw;
      x -= w_ratio;
      w += w_ratio;
      sw += sx;
      sx = 0;
   }

   if (sy < 0) {
      qreal h_ratio = sy * h / sh;
      y -= h_ratio;
      h += h_ratio;
      sh += sy;
      sy = 0;
   }

   if (sw + sx > pm.width()) {
      qreal delta = sw - (pm.width() - sx);
      qreal w_ratio = delta * w / sw;
      sw -= delta;
      w -= w_ratio;
   }

   if (sh + sy > pm.height()) {
      qreal delta = sh - (pm.height() - sy);
      qreal h_ratio = delta * h / sh;
      sh -= delta;
      h -= h_ratio;
   }

   if (w == 0 || h == 0 || sw <= 0 || sh <= 0) {
      return;
   }

   if (d->extended) {
      d->extended->drawPixmap(QRectF(x, y, w, h), pm, QRectF(sx, sy, sw, sh));
      return;
   }

   // Emulate opaque background for bitmaps
   if (d->state->bgMode == Qt::OpaqueMode && pm.isQBitmap()) {
      fillRect(QRectF(x, y, w, h), d->state->bgBrush.color());
   }

   d->updateState(d->state);

   if ((d->state->matrix.type() > QTransform::TxTranslate
         && !d->engine->hasFeature(QPaintEngine::PixmapTransform))
      || (!d->state->matrix.isAffine() && !d->engine->hasFeature(QPaintEngine::PerspectiveTransform))
      || (d->state->opacity != 1.0 && !d->engine->hasFeature(QPaintEngine::ConstantOpacity))
      || ((sw != w || sh != h) && !d->engine->hasFeature(QPaintEngine::PixmapTransform))) {
      save();
      // If there is no rotation involved we have to make sure we use the
      // antialiased and not the aliased coordinate system by rounding the coordinates.
      if (d->state->matrix.type() <= QTransform::TxScale) {
         const QPointF p = roundInDeviceCoordinates(QPointF(x, y), d->state->matrix);
         x = p.x();
         y = p.y();
      }

      if (d->state->matrix.type() <= QTransform::TxTranslate && sw == w && sh == h) {
         sx = qRound(sx);
         sy = qRound(sy);
         sw = qRound(sw);
         sh = qRound(sh);
      }

      translate(x, y);
      scale(w / sw, h / sh);
      setBackgroundMode(Qt::TransparentMode);
      setRenderHint(Antialiasing, renderHints() & SmoothPixmapTransform);
      QBrush brush;

      if (sw == pm.width() && sh == pm.height()) {
         brush = QBrush(d->state->pen.color(), pm);
      } else {
         brush = QBrush(d->state->pen.color(), pm.copy(sx, sy, sw, sh));
      }

      setBrush(brush);
      setPen(Qt::NoPen);

      drawRect(QRectF(0, 0, sw, sh));
      restore();
   } else {
      if (!d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
         x += d->state->matrix.dx();
         y += d->state->matrix.dy();
      }
      d->engine->drawPixmap(QRectF(x, y, w, h), pm, QRectF(sx, sy, sw, sh));
   }
}

void QPainter::drawImage(const QPointF &p, const QImage &image)
{
   Q_D(QPainter);

   if (!d->engine || image.isNull()) {
      return;
   }

   if (d->extended) {
      d->extended->drawImage(p, image);
      return;
   }

   qreal x = p.x();
   qreal y = p.y();

   int w = image.width();
   int h = image.height();
   qreal scale = image.devicePixelRatio();

   d->updateState(d->state);

   if (((d->state->matrix.type() > QTransform::TxTranslate)
         && !d->engine->hasFeature(QPaintEngine::PixmapTransform))
      || (!d->state->matrix.isAffine() && !d->engine->hasFeature(QPaintEngine::PerspectiveTransform))
      || (d->state->opacity != 1.0 && !d->engine->hasFeature(QPaintEngine::ConstantOpacity))) {
      save();
      // If there is no rotation involved we have to make sure we use the
      // antialiased and not the aliased coordinate system by rounding the coordinates.
      if (d->state->matrix.type() <= QTransform::TxScale) {
         const QPointF p = roundInDeviceCoordinates(QPointF(x, y), d->state->matrix);
         x = p.x();
         y = p.y();
      }
      translate(x, y);
      setBackgroundMode(Qt::TransparentMode);
      setRenderHint(Antialiasing, renderHints() & SmoothPixmapTransform);
      QBrush brush(image);
      setBrush(brush);
      setPen(Qt::NoPen);
      setBrushOrigin(QPointF(0, 0));

      drawRect(QRect(QPoint(0, 0), image.size() / scale));
      restore();

      return;
   }

   if (d->state->matrix.type() == QTransform::TxTranslate
      && !d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
      x += d->state->matrix.dx();
      y += d->state->matrix.dy();
   }

   d->engine->drawImage(QRectF(x, y, w / scale, h / scale), image, QRectF(0, 0, w, h), Qt::AutoColor);
}

void QPainter::drawImage(const QRectF &targetRect, const QImage &image, const QRectF &sourceRect,
   Qt::ImageConversionFlags flags)
{
   Q_D(QPainter);

   if (!d->engine || image.isNull()) {
      return;
   }

   qreal x = targetRect.x();
   qreal y = targetRect.y();
   qreal w = targetRect.width();
   qreal h = targetRect.height();
   qreal sx = sourceRect.x();
   qreal sy = sourceRect.y();
   qreal sw = sourceRect.width();
   qreal sh = sourceRect.height();
   qreal imageScale = image.devicePixelRatio();

   // Sanity-check clipping
   if (sw <= 0) {
      sw = image.width() - sx;
   }

   if (sh <= 0) {
      sh = image.height() - sy;
   }

   if (w < 0) {
      w = sw / imageScale;
   }

   if (h < 0) {
      h = sh / imageScale;
   }

   if (sx < 0) {
      qreal w_ratio = sx * w / sw;
      x -= w_ratio;
      w += w_ratio;
      sw += sx;
      sx = 0;
   }

   if (sy < 0) {
      qreal h_ratio = sy * h / sh;
      y -= h_ratio;
      h += h_ratio;
      sh += sy;
      sy = 0;
   }

   if (sw + sx > image.width()) {
      qreal delta = sw - (image.width() - sx);
      qreal w_ratio = delta * w / sw;
      sw -= delta;
      w -= w_ratio;
   }

   if (sh + sy > image.height()) {
      qreal delta = sh - (image.height() - sy);
      qreal h_ratio = delta * h / sh;
      sh -= delta;
      h -= h_ratio;
   }

   if (w == 0 || h == 0 || sw <= 0 || sh <= 0) {
      return;
   }

   if (d->extended) {
      d->extended->drawImage(QRectF(x, y, w, h), image, QRectF(sx, sy, sw, sh), flags);
      return;
   }

   d->updateState(d->state);

   if (((d->state->matrix.type() > QTransform::TxTranslate || (sw != w || sh != h))
         && !d->engine->hasFeature(QPaintEngine::PixmapTransform))
      || (!d->state->matrix.isAffine() && !d->engine->hasFeature(QPaintEngine::PerspectiveTransform))
      || (d->state->opacity != 1.0 && !d->engine->hasFeature(QPaintEngine::ConstantOpacity))) {
      save();
      // If there is no rotation involved we have to make sure we use the
      // antialiased and not the aliased coordinate system by rounding the coordinates.
      if (d->state->matrix.type() <= QTransform::TxScale) {
         const QPointF p = roundInDeviceCoordinates(QPointF(x, y), d->state->matrix);
         x = p.x();
         y = p.y();
      }

      if (d->state->matrix.type() <= QTransform::TxTranslate && sw == w && sh == h) {
         sx = qRound(sx);
         sy = qRound(sy);
         sw = qRound(sw);
         sh = qRound(sh);
      }
      translate(x, y);
      scale(w / sw, h / sh);
      setBackgroundMode(Qt::TransparentMode);
      setRenderHint(Antialiasing, renderHints() & SmoothPixmapTransform);
      QBrush brush(image);
      setBrush(brush);
      setPen(Qt::NoPen);
      setBrushOrigin(QPointF(-sx, -sy));

      drawRect(QRectF(0, 0, sw, sh));
      restore();
      return;
   }

   if (d->state->matrix.type() == QTransform::TxTranslate
      && !d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
      x += d->state->matrix.dx();
      y += d->state->matrix.dy();
   }

   d->engine->drawImage(QRectF(x, y, w, h), image, QRectF(sx, sy, sw, sh), flags);
}

void QPainter::drawGlyphRun(const QPointF &position, const QGlyphRun &glyphRun)
{
   Q_D(QPainter);

   if (! d->engine) {
      qWarning("QPainter::drawGlyphRun() Painter engine not active");
      return;
   }

   QRawFont font = glyphRun.rawFont();
   if (! font.isValid()) {
      return;
   }

   QGlyphRunPrivate *glyphRun_d = QGlyphRunPrivate::get(glyphRun);

   const quint32 *glyphIndexes = glyphRun_d->glyphIndexData;
   const QPointF *glyphPositions = glyphRun_d->glyphPositionData;

   int count = qMin(glyphRun_d->glyphIndexDataSize, glyphRun_d->glyphPositionDataSize);
   QVarLengthArray<QFixedPoint, 128> fixedPointPositions(count);

   std::shared_ptr<QRawFontPrivate> fontD = QRawFontPrivate::get(font);

   bool engineRequiresPretransformedGlyphPositions = d->extended
      ? d->extended->requiresPretransformedGlyphPositions(fontD->fontEngine, d->state->matrix)
      : d->engine->type() != QPaintEngine::CoreGraphics && ! d->state->matrix.isAffine();

   for (int i = 0; i < count; ++i) {
      QPointF processedPosition = position + glyphPositions[i];

      if (engineRequiresPretransformedGlyphPositions) {
         processedPosition = d->state->transform().map(processedPosition);
      }

      fixedPointPositions[i] = QFixedPoint::fromPointF(processedPosition);
   }

   d->drawGlyphs(glyphIndexes, fixedPointPositions.data(), count, fontD->fontEngine,
      glyphRun.overline(), glyphRun.underline(), glyphRun.strikeOut());
}

void QPainterPrivate::drawGlyphs(const quint32 *glyphArray, QFixedPoint *positions,
   int glyphCount, QFontEngine *fontEngine, bool overline, bool underline, bool strikeOut)
{
   Q_Q(QPainter);

   updateState(state);

   QFixed leftMost;
   QFixed rightMost;
   QFixed baseLine;

   for (int i = 0; i < glyphCount; ++i) {
      glyph_metrics_t gm = fontEngine->boundingBox(glyphArray[i]);

      if (i == 0 || leftMost > positions[i].x) {
         leftMost = positions[i].x;
      }

      // We don't support glyphs that do not share a common baseline. If this turns out to
      // be a relevant use case, then we need to find clusters of glyphs that share a baseline
      // and do a drawTextItemDecorations call per cluster.
      if (i == 0 || baseLine < positions[i].y) {
         baseLine = positions[i].y;
      }

      // We use the advance rather than the actual bounds to match the algorithm in drawText()
      if (i == 0 || rightMost < positions[i].x + gm.xoff) {
         rightMost = positions[i].x + gm.xoff;
      }
   }

   QFixed width = rightMost - leftMost;

   if (extended != nullptr && state->matrix.isAffine()) {
      QStaticTextItem staticTextItem;
      staticTextItem.color = state->pen.color();
      staticTextItem.font  = state->font;
      staticTextItem.setFontEngine(fontEngine);
      staticTextItem.numGlyphs = glyphCount;
      staticTextItem.glyphs    = reinterpret_cast<glyph_t *>(const_cast<glyph_t *>(glyphArray));
      staticTextItem.glyphPositions = positions;
      staticTextItem.usesRawFont = true;

      extended->drawStaticTextItem(&staticTextItem);

   } else {
      QTextItemInt textItem;
      textItem.fontEngine = fontEngine;

      QVarLengthArray<QFixed, 128> advances(glyphCount);
      QVarLengthArray<QGlyphJustification, 128> glyphJustifications(glyphCount);
      QVarLengthArray<QGlyphAttributes, 128> glyphAttributes(glyphCount);

      memset(glyphAttributes.data(), 0, glyphAttributes.size() * sizeof(QGlyphAttributes));

      std::fill_n(advances.data(), advances.size(), QFixed());
      std::fill_n(glyphJustifications.data(), glyphJustifications.size(), QGlyphJustification());

      textItem.glyphs.numGlyphs      = glyphCount;
      textItem.glyphs.glyphs         = const_cast<glyph_t *>(glyphArray);
      textItem.glyphs.offsets        = positions;
      textItem.glyphs.advances       = advances.data();
      textItem.glyphs.justifications = glyphJustifications.data();
      textItem.glyphs.attributes     = glyphAttributes.data();

      engine->drawTextItem(QPointF(0, 0), textItem);
   }

   QTextItemInt::RenderFlags flags;
   if (underline) {
      flags |= QTextItemInt::Underline;
   }

   if (overline) {
      flags |= QTextItemInt::Overline;
   }

   if (strikeOut) {
      flags |= QTextItemInt::StrikeOut;
   }

   drawTextItemDecoration(q, QPointF(leftMost.toReal(), baseLine.toReal()), fontEngine, nullptr,
      (underline ? QTextCharFormat::SingleUnderline : QTextCharFormat::NoUnderline),
      flags, width.toReal(), QTextCharFormat());
}

void QPainter::drawText(const QPointF &p, const QString &str)
{
   drawText(p, str, 0, 0);
}

void QPainter::drawStaticText(const QPointF &topLeftPosition, const QStaticText &staticText)
{
   Q_D(QPainter);

   if (! d->engine || staticText.text().isEmpty() || pen().style() == Qt::NoPen) {
      return;
   }

   QStaticTextPrivate *staticText_d = const_cast<QStaticTextPrivate *>(QStaticTextPrivate::get(&staticText));

   if (font() != staticText_d->font) {
      staticText_d->font = font();
      staticText_d->needsRelayout = true;
   }

   QFontEngine *fe = staticText_d->font.d->engineForScript(QChar::Script_Common);

   if (fe->type() == QFontEngine::Multi) {
      fe = static_cast<QFontEngineMulti *>(fe)->engine(0);
   }


   // If we don't have an extended paint engine, or if the painter is projected,
   // we go through standard code path

   if (d->extended == nullptr || ! d->state->matrix.isAffine() || ! fe->supportsTransformation(d->state->matrix)) {
      staticText_d->paintText(topLeftPosition, this);
      return;
   }

   bool engineRequiresPretransform = d->extended->requiresPretransformedGlyphPositions(fe, d->state->matrix);

   if (staticText_d->untransformedCoordinates && engineRequiresPretransform) {
      // The coordinates are untransformed, and the engine can't deal with that
      // nativly, so we have to pre-transform the static text.
      staticText_d->untransformedCoordinates = false;
      staticText_d->needsRelayout = true;

   } else if (!staticText_d->untransformedCoordinates && !engineRequiresPretransform) {
      // The coordinates are already transformed, but the engine can handle that
      // nativly, so undo the transform of the static text.
      staticText_d->untransformedCoordinates = true;
      staticText_d->needsRelayout = true;
   }

   // Don't recalculate entire layout because of translation, rather add the dx and dy
   // into the position to move each text item the correct distance.
   QPointF transformedPosition = topLeftPosition;
   if (! staticText_d->untransformedCoordinates) {
      transformedPosition = transformedPosition * d->state->matrix;
   }
   QTransform oldMatrix;

   // The translation has been applied to transformedPosition. Remove translation
   // component from matrix.
   if (d->state->matrix.isTranslating() && !staticText_d->untransformedCoordinates) {
      qreal m11 = d->state->matrix.m11();
      qreal m12 = d->state->matrix.m12();
      qreal m13 = d->state->matrix.m13();
      qreal m21 = d->state->matrix.m21();
      qreal m22 = d->state->matrix.m22();
      qreal m23 = d->state->matrix.m23();
      qreal m33 = d->state->matrix.m33();

      oldMatrix = d->state->matrix;
      d->state->matrix.setMatrix(m11, m12, m13, m21, m22, m23, 0.0, 0.0, m33);
   }

   // If the transform is not identical to the text transform,
   // we have to relayout the text (for other transformations than plain translation)

   bool staticTextNeedsReinit = staticText_d->needsRelayout;
   if (! staticText_d->untransformedCoordinates && staticText_d->matrix != d->state->matrix) {
      staticText_d->matrix = d->state->matrix;
      staticTextNeedsReinit = true;
   }

   // Recreate the layout of the static text because the matrix or font has changed
   if (staticTextNeedsReinit) {
      staticText_d->init();
   }

   if (transformedPosition != staticText_d->position) { // Translate to actual position
      QFixed fx = QFixed::fromReal(transformedPosition.x());
      QFixed fy = QFixed::fromReal(transformedPosition.y());
      QFixed oldX = QFixed::fromReal(staticText_d->position.x());
      QFixed oldY = QFixed::fromReal(staticText_d->position.y());

      for (int item = 0; item < staticText_d->itemCount; ++item) {
         QStaticTextItem *textItem = staticText_d->items + item;

         for (int i = 0; i < textItem->numGlyphs; ++i) {
            textItem->glyphPositions[i].x += fx - oldX;
            textItem->glyphPositions[i].y += fy - oldY;
         }
         textItem->userDataNeedsUpdate = true;
      }

      staticText_d->position = transformedPosition;
   }

   QPen oldPen = d->state->pen;
   QColor currentColor = oldPen.color();

   for (int i = 0; i < staticText_d->itemCount; ++i) {
      QStaticTextItem *item = staticText_d->items + i;

      if (item->color.isValid() && currentColor != item->color) {
         setPen(item->color);
         currentColor = item->color;
      }
      d->extended->drawStaticTextItem(item);

      qt_draw_decoration_for_glyphs(this, item->glyphs, item->glyphPositions,
         item->numGlyphs, item->fontEngine(), staticText_d->font, QTextCharFormat());
   }

   if (currentColor != oldPen.color()) {
      setPen(oldPen);
   }

   if (! staticText_d->untransformedCoordinates && oldMatrix.isTranslating()) {
      d->state->matrix = oldMatrix;
   }
}

// internal
void QPainter::drawText(const QPointF &p, const QString &str, int tf, int justificationPadding)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawText() pointf = [%.2f,%.2f], text str = %s", p.x(), p.y(), csPrintable(str));
#endif

   Q_D(QPainter);

   if (! d->engine || str.isEmpty() || pen().style() == Qt::NoPen) {
      return;
   }

   if (tf & Qt::TextBypassShaping) {
      // skip harfbuzz complex shaping, shape using glyph advances only
      int len = str.length();
      int numGlyphs = len;

      QVarLengthGlyphLayoutArray glyphs(len);
      QFontEngine *fontEngine = d->state->font.d->engineForScript(QChar::Script_Common);

      if (! fontEngine->stringToCMap(str, &glyphs, &numGlyphs, Qt::EmptyFlag)) {
         // error - may want to throw
      }

      QTextItemInt gf(glyphs, &d->state->font, str.begin(), str.end(), fontEngine);
      drawTextItem(p, gf);

      return;
   }

   QStackTextEngine engine(str, d->state->font);
   engine.option.setTextDirection(d->state->layoutDirection);

   if (tf & (Qt::TextForceLeftToRight | Qt::TextForceRightToLeft)) {
      engine.ignoreBidi = true;
      engine.option.setTextDirection((tf & Qt::TextForceLeftToRight) ? Qt::LeftToRight : Qt::RightToLeft);
   }

   engine.itemize();
   QScriptLine line;
   line.length = str.length();
   engine.shapeLine(line);

   int nItems = engine.layoutData->items.size();
   QVarLengthArray<int> visualOrder(nItems);
   QVarLengthArray<uchar> levels(nItems);

   for (int i = 0; i < nItems; ++i) {
      levels[i] = engine.layoutData->items[i].analysis.bidiLevel;
   }
   QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

   if (justificationPadding > 0) {
      engine.option.setAlignment(Qt::AlignJustify);
      engine.forceJustification = true;

      // this works because justify() is only interested in the difference between width and textWidth
      line.width = justificationPadding;
      engine.justify(line);
   }
   QFixed x = QFixed::fromReal(p.x());

   for (int i = 0; i < nItems; ++i) {
      int item = visualOrder[i];
      const QScriptItem &si = engine.layoutData->items.at(item);

      if (si.analysis.flags >= QScriptAnalysis::TabOrObject) {
         x += si.width;
         continue;
      }

      QFont f = engine.font(si);
      QTextItemInt gf(si, &f);

      gf.glyphs = engine.shapedGlyphs(&si);
      gf.m_iter = engine.layoutData->string.begin() + si.position;
      gf.m_end  = gf.m_iter + engine.length(item);

      if (engine.forceJustification) {
         for (int j = 0; j < gf.glyphs.numGlyphs; ++j) {
            gf.width += gf.glyphs.effectiveAdvance(j);
         }

      } else {
         gf.width = si.width;
      }

      gf.logClusters = engine.logClusters(&si);
      drawTextItem(QPointF(x.toReal(), p.y()), gf);
      x += gf.width;
   }
}

void QPainter::drawText(const QRect &r, int flags, const QString &str, QRect *br)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawText() bounding rectangle = [%d,%d,%d,%d], flags = %d, text str= %s",
      r.x(), r.y(), r.width(), r.height(), flags, str.toLatin1().constData());
#endif

   Q_D(QPainter);

   if (! d->engine || str.length() == 0 || pen().style() == Qt::NoPen) {
      return;
   }

   if (! d->extended) {
      d->updateState(d->state);
   }

   QRectF bounds;
   qt_format_text(d->state->font, r, flags, nullptr, str, br ? &bounds : nullptr, 0, nullptr, 0, this);

   if (br) {
      *br = bounds.toAlignedRect();
   }
}

void QPainter::drawText(const QRectF &r, int flags, const QString &str, QRectF *br)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawText() bounding rectangle = [%.2f,%.2f,%.2f,%.2f], flags = %d, text str = %s",
         r.x(), r.y(), r.width(), r.height(), flags, csPrintable(str));
#endif

   Q_D(QPainter);

   if (! d->engine || str.length() == 0 || pen().style() == Qt::NoPen) {
      return;
   }

   if (! d->extended) {
      d->updateState(d->state);
   }

   qt_format_text(d->state->font, r, flags, nullptr, str, br, 0, nullptr, 0, this);
}

void QPainter::drawText(const QRectF &r, const QString &text, const QTextOption &o)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawText() bounding rectangle = [%.2f,%.2f,%.2f,%.2f], text str = %s",
         r.x(), r.y(), r.width(), r.height(), csPrintable(text));
#endif

   Q_D(QPainter);

   if (! d->engine || text.length() == 0 || pen().style() == Qt::NoPen) {
      return;
   }

   if (! d->extended) {
      d->updateState(d->state);
   }

   qt_format_text(d->state->font, r, 0, &o, text, nullptr, 0, nullptr, 0, this);
}

static QPixmap generateWavyPixmap(qreal maxRadius, const QPen &pen)
{
   const qreal radiusBase = qMax(qreal(1), maxRadius);

   QString key = "WaveUnderline_" + pen.color().name() + HexString<qreal>(radiusBase) + HexString<qreal>(pen.widthF());;

   QPixmap pixmap;
   if (QPixmapCache::find(key, pixmap)) {
      return pixmap;
   }

   const qreal halfPeriod = qMax(qreal(2), qreal(radiusBase * 1.61803399)); // the golden ratio
   const int width    = qCeil(100 / (2 * halfPeriod)) * (2 * halfPeriod);
   const qreal radius = qFloor(radiusBase * 2) / 2.;

   QPainterPath path;

   qreal xs = 0;
   qreal ys = radius;

   while (xs < width) {
      xs += halfPeriod;
      ys = -ys;
      path.quadTo(xs - halfPeriod / 2, ys, xs, 0);
   }

   pixmap = QPixmap(width, radius * 2);
   pixmap.fill(Qt::transparent);
   {
      QPen wavePen = pen;
      wavePen.setCapStyle(Qt::SquareCap);

      // This is to protect against making the line too fat, as happens on Mac OS X
      // due to it having a rather thick width for the regular underline.
      const qreal maxPenWidth = .8 * radius;

      if (wavePen.widthF() > maxPenWidth) {
         wavePen.setWidthF(maxPenWidth);
      }

      QPainter imgPainter(&pixmap);
      imgPainter.setPen(wavePen);
      imgPainter.setRenderHint(QPainter::Antialiasing);
      imgPainter.translate(0, radius);
      imgPainter.drawPath(path);
   }

   QPixmapCache::insert(key, pixmap);

   return pixmap;
}

static void drawTextItemDecoration(QPainter *painter, const QPointF &pos, const QFontEngine *fe, QTextEngine *textEngine,
   QTextCharFormat::UnderlineStyle underlineStyle,
   QTextItem::RenderFlags flags, qreal width,
   const QTextCharFormat &charFormat)
{
   if (underlineStyle == QTextCharFormat::NoUnderline
      && !( flags & (QTextItem::StrikeOut | QTextItem::Overline))) {
      return;
   }

   const QPen oldPen = painter->pen();
   const QBrush oldBrush = painter->brush();
   painter->setBrush(Qt::NoBrush);
   QPen pen = oldPen;
   pen.setStyle(Qt::SolidLine);
   pen.setWidthF(fe->lineThickness().toReal());
   pen.setCapStyle(Qt::FlatCap);

   QLineF line(qFloor(pos.x()), pos.y(), qFloor(pos.x() + width), pos.y());

   const qreal underlineOffset = fe->underlinePosition().toReal();

   if (underlineStyle == QTextCharFormat::SpellCheckUnderline) {
      QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme();

      if (theme) {
         underlineStyle = QTextCharFormat::UnderlineStyle(theme->themeHint(QPlatformTheme::SpellCheckUnderlineStyle).toInt());
      }
   }

   if (underlineStyle == QTextCharFormat::WaveUnderline) {
      painter->save();
      painter->translate(0, pos.y() + 1);
      qreal maxHeight = fe->descent().toReal() - qreal(1);

      QColor uc = charFormat.underlineColor();
      if (uc.isValid()) {
         pen.setColor(uc);
      }

      // Adapt wave to underlineOffset or pen width, whatever is larger, to make it work on all platforms
      const QPixmap wave = generateWavyPixmap(qMin(qMax(underlineOffset, pen.widthF()), maxHeight / qreal(2.)), pen);
      const int descent = qFloor(maxHeight);

      painter->setBrushOrigin(painter->brushOrigin().x(), 0);
      painter->fillRect(pos.x(), 0, qCeil(width), qMin(wave.height(), descent), wave);
      painter->restore();

   } else if (underlineStyle != QTextCharFormat::NoUnderline) {
      // Deliberately ceil the offset to avoid the underline coming too close to
      // the text above it, but limit it to stay within descent.
      qreal adjustedUnderlineOffset = std::ceil(underlineOffset) + 0.5;

      if (underlineOffset <= fe->descent().toReal()) {
         adjustedUnderlineOffset = qMin(adjustedUnderlineOffset, fe->descent().toReal() - qreal(0.5));
      }

      const qreal underlinePos = pos.y() + adjustedUnderlineOffset;

      QColor uc = charFormat.underlineColor();
      if (uc.isValid()) {
         pen.setColor(uc);
      }

      pen.setStyle((Qt::PenStyle)(underlineStyle));
      painter->setPen(pen);

      QLineF underline(line.x1(), underlinePos, line.x2(), underlinePos);
      if (textEngine) {
         textEngine->addUnderline(painter, underline);
      } else {
         painter->drawLine(underline);
      }
   }

   pen.setStyle(Qt::SolidLine);
   pen.setColor(oldPen.color());

   if (flags & QTextItem::StrikeOut) {
      QLineF strikeOutLine = line;
      strikeOutLine.translate(0., - fe->ascent().toReal() / 3.);
      painter->setPen(pen);

      if (textEngine) {
         textEngine->addStrikeOut(painter, strikeOutLine);
      } else {
         painter->drawLine(strikeOutLine);
      }
   }

   if (flags & QTextItem::Overline) {
      QLineF overline = line;
      overline.translate(0., - fe->ascent().toReal());
      painter->setPen(pen);

      if (textEngine) {
         textEngine->addOverline(painter, overline);
      } else {
         painter->drawLine(overline);
      }
   }

   painter->setPen(oldPen);
   painter->setBrush(oldBrush);
}

Q_GUI_EXPORT void qt_draw_decoration_for_glyphs(QPainter *painter, const glyph_t *glyphArray,
      const QFixedPoint *positions, int glyphCount, QFontEngine *fontEngine, const QFont &font,
      const QTextCharFormat &charFormat)
{
   if (!(font.underline() || font.strikeOut() || font.overline())) {
      return;
   }

   QFixed leftMost;
   QFixed rightMost;
   QFixed baseLine;
   for (int i = 0; i < glyphCount; ++i) {
      glyph_metrics_t gm = fontEngine->boundingBox(glyphArray[i]);
      if (i == 0 || leftMost > positions[i].x) {
         leftMost = positions[i].x;
      }

      // We don't support glyphs that do not share a common baseline. If this turns out to
      // be a relevant use case, then we need to find clusters of glyphs that share a baseline
      // and do a drawTextItemDecorations call per cluster.
      if (i == 0 || baseLine < positions[i].y) {
         baseLine = positions[i].y;
      }

      // We use the advance rather than the actual bounds to match the algorithm in drawText()
      if (i == 0 || rightMost < positions[i].x + gm.xoff) {
         rightMost = positions[i].x + gm.xoff;
      }
   }

   QFixed width = rightMost - leftMost;
   QTextItem::RenderFlags flags = Qt::EmptyFlag;

   if (font.underline()) {
      flags |= QTextItem::Underline;
   }

   if (font.overline()) {
      flags |= QTextItem::Overline;
   }

   if (font.strikeOut()) {
      flags |= QTextItem::StrikeOut;
   }

   drawTextItemDecoration(painter, QPointF(leftMost.toReal(), baseLine.toReal()),
      fontEngine, nullptr, font.underline() ? QTextCharFormat::SingleUnderline : QTextCharFormat::NoUnderline,
      flags, width.toReal(), charFormat);
}

void QPainter::drawTextItem(const QPointF &p, const QTextItem &ti)
{
   Q_D(QPainter);
   d->drawTextItem(p, ti, static_cast<QTextEngine *>(nullptr));
}

void QPainterPrivate::drawTextItem(const QPointF &p, const QTextItem &_ti, QTextEngine *textEngine)
{
   Q_Q(QPainter);

   if (! engine) {
      return;
   }

   QTextItemInt &ti = const_cast<QTextItemInt &>(static_cast<const QTextItemInt &>(_ti));

   if (! extended && state->bgMode == Qt::OpaqueMode) {
      QRectF rect(p.x(), p.y() - ti.ascent.toReal(), ti.width.toReal(), (ti.ascent + ti.descent).toReal());
      q->fillRect(rect, state->bgBrush);
   }

   if (q->pen().style() == Qt::NoPen) {
      return;
   }

   const QPainter::RenderHints oldRenderHints = state->renderHints;

   if (! (state->renderHints & QPainter::Antialiasing) && state->matrix.type() >= QTransform::TxScale) {
      // draw antialias decoration (underline/overline/strikeout) with transformed text

      bool aa = true;
      const QTransform &m = state->matrix;

      if (state->matrix.type() < QTransform::TxShear) {
         bool isPlain90DegreeRotation =
            (qFuzzyIsNull(m.m11())
               && qFuzzyIsNull(m.m12() - qreal(1))
               && qFuzzyIsNull(m.m21() + qreal(1))
               && qFuzzyIsNull(m.m22())
            )
            ||
            (qFuzzyIsNull(m.m11() + qreal(1))
               && qFuzzyIsNull(m.m12())
               && qFuzzyIsNull(m.m21())
               && qFuzzyIsNull(m.m22() + qreal(1))
            )
            ||
            (qFuzzyIsNull(m.m11())
               && qFuzzyIsNull(m.m12() + qreal(1))
               && qFuzzyIsNull(m.m21() - qreal(1))
               && qFuzzyIsNull(m.m22())
            )
            ;
         aa = ! isPlain90DegreeRotation;
      }

      if (aa) {
         q->setRenderHint(QPainter::Antialiasing, true);
      }
   }

   if (! extended) {
      updateState(state);
   }

   if (! ti.glyphs.numGlyphs) {
      // nothing to do

   } else if (ti.fontEngine->type() == QFontEngine::Multi) {
      QFontEngineMulti *multi = static_cast<QFontEngineMulti *>(ti.fontEngine);

      const QGlyphLayout &glyphs = ti.glyphs;
      int which = glyphs.glyphs[0] >> 24;

      qreal x = p.x();
      qreal y = p.y();

      bool rtl = ti.flags & QTextItem::RightToLeft;
      if (rtl) {
         x += ti.width.toReal();
      }

      int start = 0;
      int end;
      int i;

      for (end = 0; end < ti.glyphs.numGlyphs; ++end) {
         const int e = glyphs.glyphs[end] >> 24;

         if (e == which) {
            continue;
         }

         multi->ensureEngineAt(which);
         QTextItemInt ti2 = ti.midItem(multi->engine(which), start, end - start);
         ti2.width = 0;

         // set the high byte to zero and calc the width
         for (i = start; i < end; ++i) {
            glyphs.glyphs[i] = glyphs.glyphs[i] & 0xffffff;
            ti2.width += ti.glyphs.effectiveAdvance(i);
         }

         if (rtl) {
            x -= ti2.width.toReal();
         }

         if (extended) {
            extended->drawTextItem(QPointF(x, y), ti2);
         } else {
            engine->drawTextItem(QPointF(x, y), ti2);
         }

         if (! rtl) {
            x += ti2.width.toReal();
         }

         // reset the high byte for all glyphs and advance to the next sub-string
         const int hi = which << 24;

         for (i = start; i < end; ++i) {
            glyphs.glyphs[i] = hi | glyphs.glyphs[i];
         }

         // change engine
         start = end;
         which = e;
      }

      multi->ensureEngineAt(which);
      QTextItemInt ti2 = ti.midItem(multi->engine(which), start, end - start);
      ti2.width = 0;

      // set the high byte to zero and calc the width
      for (i = start; i < end; ++i) {
         glyphs.glyphs[i] = glyphs.glyphs[i] & 0xffffff;
         ti2.width += ti.glyphs.effectiveAdvance(i);
      }

      if (rtl) {
         x -= ti2.width.toReal();
      }

      if (extended) {
         extended->drawTextItem(QPointF(x, y), ti2);

      } else {
         engine->drawTextItem(QPointF(x, y), ti2);

      }

      // reset the high byte for all glyphs
      const int hi = which << 24;
      for (i = start; i < end; ++i) {
         glyphs.glyphs[i] = hi | glyphs.glyphs[i];
      }

   } else {

      if (extended) {
         extended->drawTextItem(p, ti);

      } else {
         engine->drawTextItem(p, ti);
      }
   }

   drawTextItemDecoration(q, p, ti.fontEngine, textEngine, ti.underlineStyle,
      ti.flags, ti.width.toReal(), ti.charFormat);

   if (state->renderHints != oldRenderHints) {
      state->renderHints = oldRenderHints;

      if (extended) {
         extended->renderHintsChanged();
      } else {
         state->dirtyFlags |= QPaintEngine::DirtyHints;
      }
   }
}

QRect QPainter::boundingRect(const QRect &rect, int flags, const QString &str)
{
   if (str.isEmpty()) {
      return QRect(rect.x(), rect.y(), 0, 0);
   }

   QRect brect;
   drawText(rect, flags | Qt::TextDontPrint, str, &brect);
   return brect;
}

QRectF QPainter::boundingRect(const QRectF &rect, int flags, const QString &str)
{
   if (str.isEmpty()) {
      return QRectF(rect.x(), rect.y(), 0, 0);
   }
   QRectF brect;
   drawText(rect, flags | Qt::TextDontPrint, str, &brect);
   return brect;
}

QRectF QPainter::boundingRect(const QRectF &r, const QString &text, const QTextOption &o)
{
   Q_D(QPainter);

   if (!d->engine || text.length() == 0) {
      return QRectF(r.x(), r.y(), 0, 0);
   }

   QRectF br;
   qt_format_text(d->state->font, r, Qt::TextDontPrint, &o, text, &br, 0, nullptr, 0, this);

   return br;
}

void QPainter::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &sp)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::drawTiledPixmap() target rectangle = [%.2f,%.2f,%.2f,%.2f], pixmap = [%d,%d], offset = [%.2f,%.2f]",
         r.x(), r.y(), r.width(), r.height(), pixmap.width(), pixmap.height(), sp.x(), sp.y());
#endif

   Q_D(QPainter);

   if (!d->engine || pixmap.isNull() || r.isEmpty()) {
      return;
   }

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qt_painter_thread_test(d->device->devType(), d->engine->type(), "drawTiledPixmap()");
#endif

   qreal sw = pixmap.width();
   qreal sh = pixmap.height();
   qreal sx = sp.x();
   qreal sy = sp.y();
   if (sx < 0) {
      sx = qRound(sw) - qRound(-sx) % qRound(sw);
   } else {
      sx = qRound(sx) % qRound(sw);
   }
   if (sy < 0) {
      sy = qRound(sh) - -qRound(sy) % qRound(sh);
   } else {
      sy = qRound(sy) % qRound(sh);
   }


   if (d->extended) {
      d->extended->drawTiledPixmap(r, pixmap, QPointF(sx, sy));
      return;
   }

   if (d->state->bgMode == Qt::OpaqueMode && pixmap.isQBitmap()) {
      fillRect(r, d->state->bgBrush);
   }

   d->updateState(d->state);
   if ((d->state->matrix.type() > QTransform::TxTranslate
         && !d->engine->hasFeature(QPaintEngine::PixmapTransform))
      || (d->state->opacity != 1.0 && !d->engine->hasFeature(QPaintEngine::ConstantOpacity))) {
      save();
      setBackgroundMode(Qt::TransparentMode);
      setRenderHint(Antialiasing, renderHints() & SmoothPixmapTransform);
      setBrush(QBrush(d->state->pen.color(), pixmap));
      setPen(Qt::NoPen);

      // If there is no rotation involved we have to make sure we use the
      // antialiased and not the aliased coordinate system by rounding the coordinates.
      if (d->state->matrix.type() <= QTransform::TxScale) {
         const QPointF p = roundInDeviceCoordinates(r.topLeft(), d->state->matrix);

         if (d->state->matrix.type() <= QTransform::TxTranslate) {
            sx = qRound(sx);
            sy = qRound(sy);
         }

         setBrushOrigin(QPointF(r.x() - sx, r.y() - sy));
         drawRect(QRectF(p, r.size()));
      } else {
         setBrushOrigin(QPointF(r.x() - sx, r.y() - sy));
         drawRect(r);
      }
      restore();
      return;
   }

   qreal x = r.x();
   qreal y = r.y();
   if (d->state->matrix.type() == QTransform::TxTranslate
      && !d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
      x += d->state->matrix.dx();
      y += d->state->matrix.dy();
   }

   d->engine->drawTiledPixmap(QRectF(x, y, r.width(), r.height()), pixmap, QPointF(sx, sy));
}

#ifndef QT_NO_PICTURE

void QPainter::drawPicture(const QPointF &p, const QPicture &picture)
{
   Q_D(QPainter);

   if (!d->engine) {
      return;
   }

   if (!d->extended) {
      d->updateState(d->state);
   }

   save();
   translate(p);
   const_cast<QPicture *>(&picture)->play(this);
   restore();
}

#endif

void QPainter::eraseRect(const QRectF &r)
{
   Q_D(QPainter);

   fillRect(r, d->state->bgBrush);
}

static inline bool needsResolving(const QBrush &brush)
{
   Qt::BrushStyle s = brush.style();
   return ((s == Qt::LinearGradientPattern || s == Qt::RadialGradientPattern ||
            s == Qt::ConicalGradientPattern) &&
         brush.gradient()->coordinateMode() == QGradient::ObjectBoundingMode);
}

void QPainter::fillRect(const QRectF &r, const QBrush &brush)
{
   Q_D(QPainter);

   if (!d->engine) {
      return;
   }

   if (d->extended) {
      const QGradient *g = brush.gradient();
      if (!g || g->coordinateMode() == QGradient::LogicalMode) {
         d->extended->fillRect(r, brush);
         return;
      }
   }

   QPen oldPen = pen();
   QBrush oldBrush = this->brush();
   setPen(Qt::NoPen);
   if (brush.style() == Qt::SolidPattern) {
      d->colorBrush.setStyle(Qt::SolidPattern);
      d->colorBrush.setColor(brush.color());
      setBrush(d->colorBrush);
   } else {
      setBrush(brush);
   }

   drawRect(r);
   setBrush(oldBrush);
   setPen(oldPen);
}

void QPainter::fillRect(const QRect &r, const QBrush &brush)
{
   Q_D(QPainter);

   if (!d->engine) {
      return;
   }

   if (d->extended) {
      const QGradient *g = brush.gradient();
      if (!g || g->coordinateMode() == QGradient::LogicalMode) {
         d->extended->fillRect(r, brush);
         return;
      }
   }

   QPen oldPen = pen();
   QBrush oldBrush = this->brush();
   setPen(Qt::NoPen);
   if (brush.style() == Qt::SolidPattern) {
      d->colorBrush.setStyle(Qt::SolidPattern);
      d->colorBrush.setColor(brush.color());
      setBrush(d->colorBrush);
   } else {
      setBrush(brush);
   }

   drawRect(r);
   setBrush(oldBrush);
   setPen(oldPen);
}

void QPainter::fillRect(const QRect &r, const QColor &color)
{
   Q_D(QPainter);

   if (!d->engine) {
      return;
   }

   if (d->extended) {
      d->extended->fillRect(r, color);
      return;
   }

   fillRect(r, QBrush(color));
}

void QPainter::fillRect(const QRectF &r, const QColor &color)
{
   Q_D(QPainter);

   if (!d->engine) {
      return;
   }

   if (d->extended) {
      d->extended->fillRect(r, color);
      return;
   }

   fillRect(r, QBrush(color));
}

void QPainter::setRenderHint(RenderHint hint, bool on)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::setRenderHint() hint = %x, state = %s", hint, on ? "on" : "off");
#endif

   setRenderHints(hint, on);
}

void QPainter::setRenderHints(RenderHints hints, bool on)
{
   Q_D(QPainter);

   if (! d->engine) {
      qWarning("QPainter::setRenderHint() Painter engine not active");
      return;
   }

   if (on) {
      d->state->renderHints |= hints;
   } else {
      d->state->renderHints &= ~hints;
   }

   if (d->extended) {
      d->extended->renderHintsChanged();
   } else {
      d->state->dirtyFlags |= QPaintEngine::DirtyHints;
   }
}

QPainter::RenderHints QPainter::renderHints() const
{
   Q_D(const QPainter);

   if (!d->engine) {
      return Qt::EmptyFlag;
   }

   return d->state->renderHints;
}

bool QPainter::viewTransformEnabled() const
{
   Q_D(const QPainter);

   if (!d->engine) {
      qWarning("QPainter::viewTransformEnabled() Painter engine not active");
      return false;
   }

   return d->state->VxF;
}

void QPainter::setWindow(const QRect &r)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::setWindow() bounding rectangle = [%d,%d,%d,%d]", r.x(), r.y(), r.width(), r.height());
#endif

   Q_D(QPainter);

   if (! d->engine) {
      qWarning("QPainter::setWindow() Painter engine not active");
      return;
   }

   d->state->wx = r.x();
   d->state->wy = r.y();
   d->state->ww = r.width();
   d->state->wh = r.height();

   d->state->VxF = true;
   d->updateMatrix();
}

QRect QPainter::window() const
{
   Q_D(const QPainter);

   if (!d->engine) {
      qWarning("QPainter::window() Painter engine not active");
      return QRect();
   }

   return QRect(d->state->wx, d->state->wy, d->state->ww, d->state->wh);
}

void QPainter::setViewport(const QRect &r)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   qDebug("QPainter::setViewport() bounding rectangle = [%d,%d,%d,%d]", r.x(), r.y(), r.width(), r.height());
#endif

   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::setViewport() Painter engine not active");
      return;
   }

   d->state->vx = r.x();
   d->state->vy = r.y();
   d->state->vw = r.width();
   d->state->vh = r.height();

   d->state->VxF = true;
   d->updateMatrix();
}

QRect QPainter::viewport() const
{
   Q_D(const QPainter);

   if (!d->engine) {
      qWarning("QPainter::viewport() Painter engine not active");
      return QRect();
   }

   return QRect(d->state->vx, d->state->vy, d->state->vw, d->state->vh);
}

void QPainter::setViewTransformEnabled(bool enable)
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::setViewTransformEnabled() Painter engine not active");
      return;
   }

   if (enable == d->state->VxF) {
      return;
   }

   d->state->VxF = enable;
   d->updateMatrix();
}

void QPainter::setRedirected(const QPaintDevice *device, QPaintDevice *replacement, const QPoint &offset)
{
   (void) replacement;
   (void) offset;

   Q_ASSERT(device != nullptr);
   qWarning("QPainter::setRedirected() Deprecated method, use QWidget::render() instead");
}

// obsolete
void QPainter::restoreRedirected(const QPaintDevice *device)
{
   (void) device;

   qWarning("QPainter::restoreRedirected() Deprecated method, use QWidget::render() instead");
}

QPaintDevice *QPainter::redirected(const QPaintDevice *device, QPoint *offset)
{
   (void) device;
   (void) offset;

   return nullptr;
}

void qt_format_text(const QFont &fnt, const QRectF &_r, int tf, const QString &str, QRectF *brect,
   int tabstops, int *ta, int tabarraylen, QPainter *painter)
{
   qt_format_text(fnt, _r, tf, nullptr, str, brect, tabstops, ta, tabarraylen, painter);
}

void qt_format_text(const QFont &fnt, const QRectF &_r, int tf, const QTextOption *option,
   const QString &str, QRectF *brect, int tabstops, int *ta, int tabarraylen, QPainter *painter)
{
   Q_ASSERT( !((tf & ~Qt::TextDontPrint) != 0 && option != nullptr) ); //  either have an option or flags

   if (option) {
      tf |= option->alignment();

      if (option->wrapMode() != QTextOption::NoWrap) {
         tf |= Qt::TextWordWrap;
      }

      if (option->flags() & QTextOption::IncludeTrailingSpaces) {
         tf |= Qt::TextIncludeTrailingSpaces;
      }

      if (option->tabStop() >= 0 || !option->tabArray().isEmpty()) {
         tf |= Qt::TextExpandTabs;
      }
   }

   // we need to copy r here to protect against the case (&r == brect)
   QRectF r(_r);

   bool dontclip      = (tf & Qt::TextDontClip);
   bool wordwrap      = (tf & Qt::TextWordWrap) || (tf & Qt::TextWrapAnywhere);
   bool singleline    = (tf & Qt::TextSingleLine);
   bool showmnemonic  = (tf & Qt::TextShowMnemonic);
   bool hidemnmemonic = (tf & Qt::TextHideMnemonic);

   Qt::LayoutDirection layout_direction;

   if (tf & Qt::TextForceLeftToRight) {
      layout_direction = Qt::LeftToRight;

   } else if (tf & Qt::TextForceRightToLeft) {
      layout_direction = Qt::RightToLeft;

   } else if (option) {
      layout_direction = option->textDirection();

   } else if (painter) {
      layout_direction = painter->layoutDirection();

   } else {
      layout_direction = Qt::LeftToRight;

   }

   tf = QGuiApplicationPrivate::visualAlignment(layout_direction, QFlag(tf));

   bool isRightToLeft = layout_direction == Qt::RightToLeft;
   bool expandtabs = ((tf & Qt::TextExpandTabs) &&
         (((tf & Qt::AlignLeft) && !isRightToLeft) ||
            ((tf & Qt::AlignRight) && isRightToLeft)));

   if (! painter) {
      tf |= Qt::TextDontPrint;
   }

   uint maxUnderlines = 0;

   QFontMetricsF fm(fnt);
   QString text = str;
   int offset   = 0;

start_lengthVariant:
   bool hasMoreLengthVariants = false;

   // Replace tabs by spaces
   int old_offset = offset;

   for (; offset < text.length(); offset++) {
      QChar chr = text.at(offset);

      if (chr == '\r' || (singleline && chr == '\n')) {
         text.replace(offset, 1, ' ');

      } else if (chr == '\n') {
         text.replace(offset, 1, QChar(QChar::LineSeparator));

      } else if (chr == '&') {
         ++maxUnderlines;

      } else if (chr == '\t') {
         if (! expandtabs) {
            text.replace(offset, 1, ' ');

         } else if (!tabarraylen && ! tabstops) {
            tabstops = qRound(fm.width('x') * 8);
         }

      } else if (chr == QChar(ushort(0x9c))) {
         // string with multiple length variants
         hasMoreLengthVariants = true;
         break;
      }
   }

   QVector<QTextLayout::FormatRange> underlineFormats;
   int length = offset - old_offset;

   if ((hidemnmemonic || showmnemonic) && maxUnderlines > 0) {

      QString::const_iterator iter = text.begin() + old_offset;
      int len = length;

      while (len) {
         if (*iter == '&') {
            iter = text.erase(iter, iter + 1);

            --length;
            --len;

            if (! len) {
               break;
            }

            if (*iter != '&' && ! hidemnmemonic && ! (tf & Qt::TextDontPrint)) {
               QTextLayout::FormatRange range;
               range.start  = iter - (text.constBegin() + old_offset);
               range.length = 1;
               range.format.setFontUnderline(true);
               underlineFormats.append(range);
            }

#ifdef Q_OS_DARWIN
         } else if (hidemnmemonic) {

            QStringView tmp(iter, text.end());

            if (tmp.startsWith("(&") && len > 4 && tmp[2] != '&' && tmp[3] == ')')  {
               int n = 0;
               QString::const_iterator tmp_iter = iter;

               while (tmp_iter != text.begin()) {
                  --tmp_iter;

                  if (! tmp_iter->isSpace()) {
                     ++tmp_iter;
                     break;
                  }

                  ++n;
               }

               iter   += 4;
               length -= n + 4;
               len    -= 4;

               iter = text.erase(tmp_iter, iter + 1);
               continue;
            }
#endif
         }

         ++iter;
         --len;
      }
   }

   qreal height = 0;
   qreal width  = 0;

   QString finalText = text.mid(old_offset, length);
   QStackTextEngine engine(finalText, fnt);

   if (option) {
      engine.option = *option;
   }

   if (engine.option.tabStop() < 0 && tabstops > 0) {
      engine.option.setTabStop(tabstops);
   }

   if (engine.option.tabs().isEmpty() && ta) {
      QList<qreal> tabs;

      for (int i = 0; i < tabarraylen; i++) {
         tabs.append(qreal(ta[i]));
      }
      engine.option.setTabArray(tabs);
   }

   engine.option.setTextDirection(layout_direction);
   if (tf & Qt::AlignJustify) {
      engine.option.setAlignment(Qt::AlignJustify);
   } else {
      engine.option.setAlignment(Qt::AlignLeft);   // do not do alignment twice
   }

   if (! option && (tf & Qt::TextWrapAnywhere)) {
      engine.option.setWrapMode(QTextOption::WrapAnywhere);
   }

   if (tf & Qt::TextJustificationForced) {
      engine.forceJustification = true;
   }

   QTextLayout textLayout(&engine);
   textLayout.setCacheEnabled(true);
   textLayout.setFormats(underlineFormats);

   if (finalText.isEmpty()) {
      height = fm.height();
      width  = 0;
      tf |= Qt::TextDontPrint;

   } else {
      qreal lineWidth = 0x01000000;

      if (wordwrap || (tf & Qt::TextJustificationForced)) {
         lineWidth = qMax(0, r.width());
      }

      if (! wordwrap) {
         tf |= Qt::TextIncludeTrailingSpaces;
      }

      textLayout.beginLayout();

      qreal leading = fm.leading();
      height = -leading;

      while (true) {
         QTextLine l = textLayout.createLine();

         if (! l.isValid()) {
            break;
         }

         l.setLineWidth(lineWidth);
         height += leading;

         // Make sure lines are positioned on whole pixels
         height = qCeil(height);
         l.setPosition(QPointF(0., height));
         height += textLayout.engine()->lines[l.lineNumber()].height().toReal();

         width  = qMax(width, l.naturalTextWidth());

         if (! dontclip && ! brect && height >= r.height()) {
            break;
         }
      }

      textLayout.endLayout();
   }

   qreal yoff = 0;
   qreal xoff = 0;

   if (tf & Qt::AlignBottom) {
      yoff = r.height() - height;

   } else if (tf & Qt::AlignVCenter) {
      yoff = (r.height() - height) / 2;
   }

   if (tf & Qt::AlignRight) {
      xoff = r.width() - width;

   } else if (tf & Qt::AlignHCenter) {
      xoff = (r.width() - width) / 2;

   }

   QRectF bounds = QRectF(r.x() + xoff, r.y() + yoff, width, height);

   if (hasMoreLengthVariants && ! (tf & Qt::TextLongestVariant) && !r.contains(bounds)) {
      offset++;
      goto start_lengthVariant;
   }

   if (brect) {
      *brect = bounds;
   }

   if (! (tf & Qt::TextDontPrint)) {
      bool restore = false;

      if (! dontclip && !r.contains(bounds)) {
         restore = true;
         painter->save();
         painter->setClipRect(r, Qt::IntersectClip);
      }

      for (int i = 0; i < textLayout.lineCount(); i++) {
         QTextLine line   = textLayout.lineAt(i);

         QTextEngine *eng = textLayout.engine();
         eng->enableDelayDecorations();

         qreal advance = line.horizontalAdvance();
         xoff = 0;

         if (tf & Qt::AlignRight) {
            xoff = r.width() - advance - eng->leadingSpaceWidth(eng->lines[line.lineNumber()]).toReal();

         } else if (tf & Qt::AlignHCenter) {
            xoff = (r.width() - advance) / 2;
         }

         line.draw(painter, QPointF(r.x() + xoff, r.y() + yoff));
         eng->drawDecorations(painter);
      }

      if (restore) {
         painter->restore();
      }
   }
}

void QPainter::setLayoutDirection(Qt::LayoutDirection direction)
{
   Q_D(QPainter);

   if (d->state) {
      d->state->layoutDirection = direction;
   }
}

Qt::LayoutDirection QPainter::layoutDirection() const
{
   Q_D(const QPainter);
   return d->state ? d->state->layoutDirection : Qt::LayoutDirectionAuto;
}

QPainterState::QPainterState(const QPainterState *s)
   : brushOrigin(s->brushOrigin), font(s->font), deviceFont(s->deviceFont),
     pen(s->pen), brush(s->brush), bgBrush(s->bgBrush),
     clipRegion(s->clipRegion), clipPath(s->clipPath),
     clipOperation(s->clipOperation),
     renderHints(s->renderHints), clipInfo(s->clipInfo),
     worldMatrix(s->worldMatrix), matrix(s->matrix), redirectionMatrix(s->redirectionMatrix),
     wx(s->wx), wy(s->wy), ww(s->ww), wh(s->wh),
     vx(s->vx), vy(s->vy), vw(s->vw), vh(s->vh),
     opacity(s->opacity), WxF(s->WxF), VxF(s->VxF),
     clipEnabled(s->clipEnabled), bgMode(s->bgMode), painter(s->painter),
     layoutDirection(s->layoutDirection),
     composition_mode(s->composition_mode),
     emulationSpecifier(s->emulationSpecifier), changeFlags(0)
{
   dirtyFlags = s->dirtyFlags;
}

QPainterState::QPainterState()
   : brushOrigin(0, 0), bgBrush(Qt::white), clipOperation(Qt::NoClip),
     renderHints(Qt::EmptyFlag), wx(0), wy(0), ww(0), wh(0), vx(0), vy(0), vw(0), vh(0),
     opacity(1), WxF(false), VxF(false), clipEnabled(true),
     bgMode(Qt::TransparentMode), painter(nullptr),
     layoutDirection(QGuiApplication::layoutDirection()),
     composition_mode(QPainter::CompositionMode_SourceOver),
     emulationSpecifier(0), changeFlags(0)
{
   dirtyFlags = Qt::EmptyFlag;
}

QPainterState::~QPainterState()
{
}

void QPainterState::init(QPainter *p)
{
   bgBrush = Qt::white;
   bgMode = Qt::TransparentMode;
   WxF = false;
   VxF = false;
   clipEnabled = true;
   wx = wy = ww = wh = 0;
   vx = vy = vw = vh = 0;
   painter = p;
   pen = QPen();
   brushOrigin = QPointF(0, 0);
   brush = QBrush();
   font = deviceFont = QFont();
   clipRegion = QRegion();
   clipPath = QPainterPath();
   clipOperation = Qt::NoClip;
   clipInfo.clear();
   worldMatrix.reset();
   matrix.reset();
   layoutDirection = QGuiApplication::layoutDirection();
   composition_mode = QPainter::CompositionMode_SourceOver;
   emulationSpecifier = 0;
   dirtyFlags = Qt::EmptyFlag;
   changeFlags = 0;
   renderHints = Qt::EmptyFlag;
   opacity = 1;
}

QPen QPaintEngineState::pen() const
{
   return static_cast<const QPainterState *>(this)->pen;
}

QBrush QPaintEngineState::brush() const
{
   return static_cast<const QPainterState *>(this)->brush;
}


QPointF QPaintEngineState::brushOrigin() const
{
   return static_cast<const QPainterState *>(this)->brushOrigin;
}


QBrush QPaintEngineState::backgroundBrush() const
{
   return static_cast<const QPainterState *>(this)->bgBrush;
}

Qt::BGMode QPaintEngineState::backgroundMode() const
{
   return static_cast<const QPainterState *>(this)->bgMode;
}

QFont QPaintEngineState::font() const
{
   return static_cast<const QPainterState *>(this)->font;
}

QMatrix QPaintEngineState::matrix() const
{
   const QPainterState *st = static_cast<const QPainterState *>(this);

   return st->matrix.toAffine();
}

QTransform QPaintEngineState::transform() const
{
   const QPainterState *st = static_cast<const QPainterState *>(this);

   return st->matrix;
}

Qt::ClipOperation QPaintEngineState::clipOperation() const
{
   return static_cast<const QPainterState *>(this)->clipOperation;
}

bool QPaintEngineState::brushNeedsResolving() const
{
   const QBrush &brush = static_cast<const QPainterState *>(this)->brush;
   return needsResolving(brush);
}


bool QPaintEngineState::penNeedsResolving() const
{
   const QPen &pen = static_cast<const QPainterState *>(this)->pen;
   return needsResolving(pen.brush());
}

QRegion QPaintEngineState::clipRegion() const
{
   return static_cast<const QPainterState *>(this)->clipRegion;
}

QPainterPath QPaintEngineState::clipPath() const
{
   return static_cast<const QPainterState *>(this)->clipPath;
}

bool QPaintEngineState::isClipEnabled() const
{
   return static_cast<const QPainterState *>(this)->clipEnabled;
}

QPainter::RenderHints QPaintEngineState::renderHints() const
{
   return static_cast<const QPainterState *>(this)->renderHints;
}

QPainter::CompositionMode QPaintEngineState::compositionMode() const
{
   return static_cast<const QPainterState *>(this)->composition_mode;
}

QPainter *QPaintEngineState::painter() const
{
   return static_cast<const QPainterState *>(this)->painter;
}

qreal QPaintEngineState::opacity() const
{
   return static_cast<const QPainterState *>(this)->opacity;
}

void QPainter::setTransform(const QTransform &transform, bool combine )
{
   setWorldTransform(transform, combine);
}

const QTransform &QPainter::transform() const
{
   return worldTransform();
}

const QTransform &QPainter::deviceTransform() const
{
   Q_D(const QPainter);
   if (!d->engine) {
      qWarning("QPainter::deviceTransform() Painter engine not active");
      return d->fakeState()->transform;
   }
   return d->state->matrix;
}

void QPainter::resetTransform()
{
   Q_D(QPainter);

   if (!d->engine) {
      qWarning("QPainter::resetMatrix() Painter engine not active");
      return;
   }

   d->state->wx = d->state->wy = d->state->vx = d->state->vy = 0;                        // default view origins
   d->state->ww = d->state->vw = d->device->metric(QPaintDevice::PdmWidth);
   d->state->wh = d->state->vh = d->device->metric(QPaintDevice::PdmHeight);
   d->state->worldMatrix = QTransform();
   setMatrixEnabled(false);
   setViewTransformEnabled(false);

   if (d->extended) {
      d->extended->transformChanged();
   } else {
      d->state->dirtyFlags |= QPaintEngine::DirtyTransform;
   }
}

void QPainter::setWorldTransform(const QTransform &matrix, bool combine )
{
   Q_D(QPainter);

   if (! d->engine) {
      qWarning("QPainter::setWorldTransform() Painter engine not active");
      return;
   }

   if (combine) {
      d->state->worldMatrix = matrix * d->state->worldMatrix;   // combines
   } else {
      d->state->worldMatrix = matrix;   // set new matrix
   }

   d->state->WxF = true;
   d->updateMatrix();
}

const QTransform &QPainter::worldTransform() const
{
   Q_D(const QPainter);

   if (!d->engine) {
      qWarning("QPainter::worldTransform() Painter engine not active");
      return d->fakeState()->transform;
   }

   return d->state->worldMatrix;
}

QTransform QPainter::combinedTransform() const
{
   Q_D(const QPainter);

   if (!d->engine) {
      qWarning("QPainter::combinedTransform() Painter engine not active");
      return QTransform();
   }

   return d->state->worldMatrix * d->viewTransform() * d->hidpiScaleTransform();
}

void QPainter::drawPixmapFragments(const PixmapFragment *fragments, int fragmentCount,
   const QPixmap &pixmap, PixmapFragmentHints hints)
{
   Q_D(QPainter);

   if (! d->engine || pixmap.isNull()) {
      return;
   }

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   for (int i = 0; i < fragmentCount; ++i) {
      QRectF sourceRect(fragments[i].sourceLeft, fragments[i].sourceTop, fragments[i].width, fragments[i].height);

      if (! (QRectF(pixmap.rect()).contains(sourceRect))) {
         qDebug("QPainter::drawPixmapFragments() Source rectangle is not contained by the pixmap rectangle");
      }
   }
#endif

   if (d->engine->isExtended()) {
      d->extended->drawPixmapFragments(fragments, fragmentCount, pixmap, hints);
   } else {
      qreal oldOpacity = opacity();
      QTransform oldTransform = transform();

      for (int i = 0; i < fragmentCount; ++i) {
         QTransform transform = oldTransform;
         qreal xOffset = 0;
         qreal yOffset = 0;
         if (fragments[i].rotation == 0) {
            xOffset = fragments[i].x;
            yOffset = fragments[i].y;
         } else {
            transform.translate(fragments[i].x, fragments[i].y);
            transform.rotate(fragments[i].rotation);
         }
         setOpacity(oldOpacity * fragments[i].opacity);
         setTransform(transform);

         qreal w = fragments[i].scaleX * fragments[i].width;
         qreal h = fragments[i].scaleY * fragments[i].height;
         QRectF sourceRect(fragments[i].sourceLeft, fragments[i].sourceTop,
            fragments[i].width, fragments[i].height);
         drawPixmap(QRectF(-0.5 * w + xOffset, -0.5 * h + yOffset, w, h), pixmap, sourceRect);
      }

      setOpacity(oldOpacity);
      setTransform(oldTransform);
   }
}

QPainter::PixmapFragment QPainter::PixmapFragment::create(const QPointF &pos, const QRectF &sourceRect,
   qreal scaleX, qreal scaleY, qreal rotation,
   qreal opacity)
{
   PixmapFragment fragment = {pos.x(), pos.y(), sourceRect.x(), sourceRect.y(), sourceRect.width(),
                     sourceRect.height(), scaleX, scaleY, rotation, opacity
                  };
   return fragment;
}

void qt_draw_helper(QPainterPrivate *p, const QPainterPath &path, QPainterPrivate::DrawOperation operation)
{
   p->draw_helper(path, operation);
}

