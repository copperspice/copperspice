/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

/*
    When the active program changes, we need to update it's uniforms.
    We could track state for each program and only update stale uniforms
        - Could lead to lots of overhead if there's a lot of programs
    We could update all the uniforms when the program changes
        - Could end up updating lots of uniforms which don't need updating

    Updating uniforms should be cheap, so the overhead of updating up-to-date
    uniforms should be minimal. It's also less complex.

    Things which _may_ cause a different program to be used:
        - Change in brush/pen style
        - Change in painter opacity
        - Change in composition mode

    Whenever we set a mode on the shader manager - it needs to tell us if it had
    to switch to a different program.

    The shader manager should only switch when we tell it to. E.g. if we set a new
    brush style and then switch to transparent painter, we only want it to compile
    and use the correct program when we really need it.
*/

// #define QT_OPENGL_CACHE_AS_VBOS

#include "qglgradientcache_p.h"
#include "qpaintengineex_opengl2_p.h"
#include <string.h>
#include <qmath.h>
#include <qgl_p.h>
#include <qmath_p.h>
#include <qpaintengineex_p.h>
#include <QPaintEngine>
#include <qpainter_p.h>
#include <qfontengine_p.h>
#include <qpixmapdata_gl_p.h>
#include <qdatabuffer_p.h>
#include <qstatictext_p.h>
#include <qtriangulator_p.h>
#include "qglengineshadermanager_p.h"
#include "qgl2pexvertexarray_p.h"
#include "qtriangulatingstroker_p.h"
#include "qtextureglyphcache_gl_p.h"

#include <QDebug>

QT_BEGIN_NAMESPACE

inline static bool isPowerOfTwo(uint x)
{
   return x && !(x & (x - 1));
}

#if defined(Q_OS_WIN)
extern Q_GUI_EXPORT bool qt_cleartype_enabled;
#endif

#ifdef Q_OS_MAC
extern bool qt_applefontsmoothing_enabled;
#endif

#if !defined(QT_MAX_CACHED_GLYPH_SIZE)
#  define QT_MAX_CACHED_GLYPH_SIZE 64
#endif

Q_GUI_EXPORT QImage qt_imageForBrush(int brushStyle, bool invert);

////////////////////////////////// Private Methods //////////////////////////////////////////

QGL2PaintEngineExPrivate::~QGL2PaintEngineExPrivate()
{
   delete shaderManager;

   while (pathCaches.size()) {
      QVectorPath::CacheEntry *e = *(pathCaches.constBegin());
      e->cleanup(e->engine, e->data);
      e->data = 0;
      e->engine = 0;
   }

   if (elementIndicesVBOId != 0) {
      glDeleteBuffers(1, &elementIndicesVBOId);
      elementIndicesVBOId = 0;
   }
}

void QGL2PaintEngineExPrivate::updateTextureFilter(GLenum target, GLenum wrapMode, bool smoothPixmapTransform,
      GLuint id)
{
   //    glActiveTexture(GL_TEXTURE0 + QT_BRUSH_TEXTURE_UNIT); //### Is it always this texture unit?
   if (id != GLuint(-1) && id == lastTextureUsed) {
      return;
   }

   lastTextureUsed = id;

   if (smoothPixmapTransform) {
      glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   } else {
      glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   }
   glTexParameterf(target, GL_TEXTURE_WRAP_S, wrapMode);
   glTexParameterf(target, GL_TEXTURE_WRAP_T, wrapMode);
}


inline QColor qt_premultiplyColor(QColor c, GLfloat opacity)
{
   qreal alpha = c.alphaF() * opacity;
   c.setAlphaF(alpha);
   c.setRedF(c.redF() * alpha);
   c.setGreenF(c.greenF() * alpha);
   c.setBlueF(c.blueF() * alpha);
   return c;
}


void QGL2PaintEngineExPrivate::setBrush(const QBrush &brush)
{
   if (qbrush_fast_equals(currentBrush, brush)) {
      return;
   }

   const Qt::BrushStyle newStyle = qbrush_style(brush);
   Q_ASSERT(newStyle != Qt::NoBrush);

   currentBrush = brush;
   if (!currentBrushPixmap.isNull()) {
      currentBrushPixmap = QPixmap();
   }
   brushUniformsDirty = true; // All brushes have at least one uniform

   if (newStyle > Qt::SolidPattern) {
      brushTextureDirty = true;
   }

   if (currentBrush.style() == Qt::TexturePattern
         && qHasPixmapTexture(brush) && brush.texture().isQBitmap()) {
      shaderManager->setSrcPixelType(QGLEngineShaderManager::TextureSrcWithPattern);
   } else {
      shaderManager->setSrcPixelType(newStyle);
   }
   shaderManager->optimiseForBrushTransform(currentBrush.transform().type());
}


void QGL2PaintEngineExPrivate::useSimpleShader()
{
   shaderManager->useSimpleProgram();

   if (matrixDirty) {
      updateMatrix();
   }
}

void QGL2PaintEngineExPrivate::updateBrushTexture()
{
   Q_Q(QGL2PaintEngineEx);
   //     qDebug("QGL2PaintEngineExPrivate::updateBrushTexture()");
   Qt::BrushStyle style = currentBrush.style();

   if ( (style >= Qt::Dense1Pattern) && (style <= Qt::DiagCrossPattern) ) {
      // Get the image data for the pattern
      QImage texImage = qt_imageForBrush(style, false);

      glActiveTexture(GL_TEXTURE0 + QT_BRUSH_TEXTURE_UNIT);
      ctx->d_func()->bindTexture(texImage, GL_TEXTURE_2D, GL_RGBA, QGLContext::InternalBindOption);
#if !defined(QT_NO_DEBUG) && defined(QT_OPENGL_ES_2)
      QGLFunctions funcs(QGLContext::currentContext());
      bool npotSupported = funcs.hasOpenGLFeature(QGLFunctions::NPOTTextures);
      bool isNpot = !isPowerOfTwo(texImage.size().width())
                    || !isPowerOfTwo(texImage.size().height());
      if (isNpot && !npotSupported) {
         qWarning("GL2 Paint Engine: This system does not support the REPEAT wrap mode for non-power-of-two textures.");
      }
#endif
      updateTextureFilter(GL_TEXTURE_2D, GL_REPEAT, q->state()->renderHints & QPainter::SmoothPixmapTransform);
   } else if (style >= Qt::LinearGradientPattern && style <= Qt::ConicalGradientPattern) {
      // Gradiant brush: All the gradiants use the same texture

      const QGradient *g = currentBrush.gradient();

      // We apply global opacity in the fragment shaders, so we always pass 1.0
      // for opacity to the cache.
      GLuint texId = QGL2GradientCache::cacheForContext(ctx)->getBuffer(*g, 1.0);

      glActiveTexture(GL_TEXTURE0 + QT_BRUSH_TEXTURE_UNIT);
      glBindTexture(GL_TEXTURE_2D, texId);

      if (g->spread() == QGradient::RepeatSpread || g->type() == QGradient::ConicalGradient) {
         updateTextureFilter(GL_TEXTURE_2D, GL_REPEAT, q->state()->renderHints & QPainter::SmoothPixmapTransform);
      } else if (g->spread() == QGradient::ReflectSpread) {
         updateTextureFilter(GL_TEXTURE_2D, GL_MIRRORED_REPEAT_IBM, q->state()->renderHints & QPainter::SmoothPixmapTransform);
      } else {
         updateTextureFilter(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, q->state()->renderHints & QPainter::SmoothPixmapTransform);
      }
   } else if (style == Qt::TexturePattern) {
      currentBrushPixmap = currentBrush.texture();

      int max_texture_size = ctx->d_func()->maxTextureSize();
      if (currentBrushPixmap.width() > max_texture_size || currentBrushPixmap.height() > max_texture_size) {
         currentBrushPixmap = currentBrushPixmap.scaled(max_texture_size, max_texture_size, Qt::KeepAspectRatio);
      }

      glActiveTexture(GL_TEXTURE0 + QT_BRUSH_TEXTURE_UNIT);
      QGLTexture *tex = ctx->d_func()->bindTexture(currentBrushPixmap, GL_TEXTURE_2D, GL_RGBA,
                        QGLContext::InternalBindOption |
                        QGLContext::CanFlipNativePixmapBindOption);
      GLenum wrapMode = GL_REPEAT;
#ifdef QT_OPENGL_ES_2
      // should check for GL_OES_texture_npot or GL_IMG_texture_npot extension
      if (!isPowerOfTwo(currentBrushPixmap.width()) || !isPowerOfTwo(currentBrushPixmap.height())) {
         wrapMode = GL_CLAMP_TO_EDGE;
      }
#endif
      updateTextureFilter(GL_TEXTURE_2D, wrapMode, q->state()->renderHints & QPainter::SmoothPixmapTransform);
      textureInvertedY = tex->options & QGLContext::InvertedYBindOption ? -1 : 1;
   }
   brushTextureDirty = false;
}


void QGL2PaintEngineExPrivate::updateBrushUniforms()
{
   //     qDebug("QGL2PaintEngineExPrivate::updateBrushUniforms()");
   Qt::BrushStyle style = currentBrush.style();

   if (style == Qt::NoBrush) {
      return;
   }

   QTransform brushQTransform = currentBrush.transform();

   if (style == Qt::SolidPattern) {
      QColor col = qt_premultiplyColor(currentBrush.color(), (GLfloat)q->state()->opacity);
      shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::FragmentColor), col);
   } else {
      // All other brushes have a transform and thus need the translation point:
      QPointF translationPoint;

      if (style <= Qt::DiagCrossPattern) {
         QColor col = qt_premultiplyColor(currentBrush.color(), (GLfloat)q->state()->opacity);

         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::PatternColor), col);

         QVector2D halfViewportSize(width * 0.5, height * 0.5);
         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::HalfViewportSize), halfViewportSize);
      } else if (style == Qt::LinearGradientPattern) {
         const QLinearGradient *g = static_cast<const QLinearGradient *>(currentBrush.gradient());

         QPointF realStart = g->start();
         QPointF realFinal = g->finalStop();
         translationPoint = realStart;

         QPointF l = realFinal - realStart;

         QVector3D linearData(
            l.x(),
            l.y(),
            1.0f / (l.x() * l.x() + l.y() * l.y())
         );

         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::LinearData), linearData);

         QVector2D halfViewportSize(width * 0.5, height * 0.5);
         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::HalfViewportSize), halfViewportSize);
      } else if (style == Qt::ConicalGradientPattern) {
         const QConicalGradient *g = static_cast<const QConicalGradient *>(currentBrush.gradient());
         translationPoint   = g->center();

         GLfloat angle = -(g->angle() * 2 * Q_PI) / 360.0;

         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Angle), angle);

         QVector2D halfViewportSize(width * 0.5, height * 0.5);
         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::HalfViewportSize), halfViewportSize);
      } else if (style == Qt::RadialGradientPattern) {
         const QRadialGradient *g = static_cast<const QRadialGradient *>(currentBrush.gradient());
         QPointF realCenter = g->center();
         QPointF realFocal  = g->focalPoint();
         qreal   realRadius = g->centerRadius() - g->focalRadius();
         translationPoint   = realFocal;

         QPointF fmp = realCenter - realFocal;
         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Fmp), fmp);

         GLfloat fmp2_m_radius2 = -fmp.x() * fmp.x() - fmp.y() * fmp.y() + realRadius * realRadius;
         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Fmp2MRadius2), fmp2_m_radius2);
         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Inverse2Fmp2MRadius2),
               GLfloat(1.0 / (2.0 * fmp2_m_radius2)));
         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::SqrFr),
               GLfloat(g->focalRadius() * g->focalRadius()));
         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::BRadius),
               GLfloat(2 * (g->centerRadius() - g->focalRadius()) * g->focalRadius()),
               g->focalRadius(),
               g->centerRadius() - g->focalRadius());

         QVector2D halfViewportSize(width * 0.5, height * 0.5);
         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::HalfViewportSize), halfViewportSize);
      } else if (style == Qt::TexturePattern) {
         const QPixmap &texPixmap = currentBrush.texture();

         if (qHasPixmapTexture(currentBrush) && currentBrush.texture().isQBitmap()) {
            QColor col = qt_premultiplyColor(currentBrush.color(), (GLfloat)q->state()->opacity);
            shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::PatternColor), col);
         }

         QSizeF invertedTextureSize(1.0 / texPixmap.width(), 1.0 / texPixmap.height());
         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::InvertedTextureSize),
               invertedTextureSize);

         QVector2D halfViewportSize(width * 0.5, height * 0.5);
         shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::HalfViewportSize), halfViewportSize);
      } else {
         qWarning("QGL2PaintEngineEx: Unimplemented fill style");
      }

      const QPointF &brushOrigin = q->state()->brushOrigin;
      QTransform matrix = q->state()->matrix;
      matrix.translate(brushOrigin.x(), brushOrigin.y());

      QTransform translate(1, 0, 0, 1, -translationPoint.x(), -translationPoint.y());
      qreal m22 = -1;
      qreal dy = height;
      if (device->isFlipped()) {
         m22 = 1;
         dy = 0;
      }
      QTransform gl_to_qt(1, 0, 0, m22, 0, dy);
      QTransform inv_matrix;
      if (style == Qt::TexturePattern && textureInvertedY == -1) {
         inv_matrix = gl_to_qt * (QTransform(1, 0, 0, -1, 0,
                                             currentBrush.texture().height()) * brushQTransform * matrix).inverted() * translate;
      } else {
         inv_matrix = gl_to_qt * (brushQTransform * matrix).inverted() * translate;
      }

      shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::BrushTransform), inv_matrix);
      shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::BrushTexture), QT_BRUSH_TEXTURE_UNIT);
   }
   brushUniformsDirty = false;
}


// This assumes the shader manager has already setup the correct shader program
void QGL2PaintEngineExPrivate::updateMatrix()
{
   //     qDebug("QGL2PaintEngineExPrivate::updateMatrix()");

   const QTransform &transform = q->state()->matrix;

   // The projection matrix converts from Qt's coordinate system to GL's coordinate system
   //    * GL's viewport is 2x2, Qt's is width x height
   //    * GL has +y -> -y going from bottom -> top, Qt is the other way round
   //    * GL has [0,0] in the center, Qt has it in the top-left
   //
   // This results in the Projection matrix below, which is multiplied by the painter's
   // transformation matrix, as shown below:
   //
   //                Projection Matrix                      Painter Transform
   // ------------------------------------------------   ------------------------
   // | 2.0 / width  |      0.0      |     -1.0      |   |  m11  |  m21  |  dx  |
   // |     0.0      | -2.0 / height |      1.0      | * |  m12  |  m22  |  dy  |
   // |     0.0      |      0.0      |      1.0      |   |  m13  |  m23  |  m33 |
   // ------------------------------------------------   ------------------------
   //
   // NOTE: The resultant matrix is also transposed, as GL expects column-major matracies

   const GLfloat wfactor = 2.0f / width;
   GLfloat hfactor = -2.0f / height;

   GLfloat dx = transform.dx();
   GLfloat dy = transform.dy();

   if (device->isFlipped()) {
      hfactor *= -1;
      dy -= height;
   }

   // Non-integer translates can have strange effects for some rendering operations such as
   // anti-aliased text rendering. In such cases, we snap the translate to the pixel grid.
   if (snapToPixelGrid && transform.type() == QTransform::TxTranslate) {
      // 0.50 needs to rounded down to 0.0 for consistency with raster engine:
      dx = ceilf(dx - 0.5f);
      dy = ceilf(dy - 0.5f);
   }
   pmvMatrix[0][0] = (wfactor * transform.m11())  - transform.m13();
   pmvMatrix[1][0] = (wfactor * transform.m21())  - transform.m23();
   pmvMatrix[2][0] = (wfactor * dx) - transform.m33();
   pmvMatrix[0][1] = (hfactor * transform.m12())  + transform.m13();
   pmvMatrix[1][1] = (hfactor * transform.m22())  + transform.m23();
   pmvMatrix[2][1] = (hfactor * dy) + transform.m33();
   pmvMatrix[0][2] = transform.m13();
   pmvMatrix[1][2] = transform.m23();
   pmvMatrix[2][2] = transform.m33();

   // 1/10000 == 0.0001, so we have good enough res to cover curves
   // that span the entire widget...
   inverseScale = qMax(1 / qMax( qMax(qAbs(transform.m11()), qAbs(transform.m22())),
                                 qMax(qAbs(transform.m12()), qAbs(transform.m21())) ),
                       qreal(0.0001));

   matrixDirty = false;
   matrixUniformDirty = true;

   // Set the PMV matrix attribute. As we use an attributes rather than uniforms, we only
   // need to do this once for every matrix change and persists across all shader programs.
   glVertexAttrib3fv(QT_PMV_MATRIX_1_ATTR, pmvMatrix[0]);
   glVertexAttrib3fv(QT_PMV_MATRIX_2_ATTR, pmvMatrix[1]);
   glVertexAttrib3fv(QT_PMV_MATRIX_3_ATTR, pmvMatrix[2]);

   dasher.setInvScale(inverseScale);
   stroker.setInvScale(inverseScale);
}


void QGL2PaintEngineExPrivate::updateCompositionMode()
{
   // NOTE: The entire paint engine works on pre-multiplied data - which is why some of these
   //       composition modes look odd.
   //     qDebug() << "QGL2PaintEngineExPrivate::updateCompositionMode() - Setting GL composition mode for " << q->state()->composition_mode;
   switch (q->state()->composition_mode) {
      case QPainter::CompositionMode_SourceOver:
         glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
         break;
      case QPainter::CompositionMode_DestinationOver:
         glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
         break;
      case QPainter::CompositionMode_Clear:
         glBlendFunc(GL_ZERO, GL_ZERO);
         break;
      case QPainter::CompositionMode_Source:
         glBlendFunc(GL_ONE, GL_ZERO);
         break;
      case QPainter::CompositionMode_Destination:
         glBlendFunc(GL_ZERO, GL_ONE);
         break;
      case QPainter::CompositionMode_SourceIn:
         glBlendFunc(GL_DST_ALPHA, GL_ZERO);
         break;
      case QPainter::CompositionMode_DestinationIn:
         glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
         break;
      case QPainter::CompositionMode_SourceOut:
         glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ZERO);
         break;
      case QPainter::CompositionMode_DestinationOut:
         glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
         break;
      case QPainter::CompositionMode_SourceAtop:
         glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         break;
      case QPainter::CompositionMode_DestinationAtop:
         glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA);
         break;
      case QPainter::CompositionMode_Xor:
         glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         break;
      case QPainter::CompositionMode_Plus:
         glBlendFunc(GL_ONE, GL_ONE);
         break;
      default:
         qWarning("Unsupported composition mode");
         break;
   }

   compositionModeDirty = false;
}

static inline void setCoords(GLfloat *coords, const QGLRect &rect)
{
   coords[0] = rect.left;
   coords[1] = rect.top;
   coords[2] = rect.right;
   coords[3] = rect.top;
   coords[4] = rect.right;
   coords[5] = rect.bottom;
   coords[6] = rect.left;
   coords[7] = rect.bottom;
}

void QGL2PaintEngineExPrivate::drawTexture(const QGLRect &dest, const QGLRect &src, const QSize &textureSize,
      bool opaque, bool pattern)
{
   // Setup for texture drawing
   currentBrush = noBrush;
   shaderManager->setSrcPixelType(pattern ? QGLEngineShaderManager::PatternSrc : QGLEngineShaderManager::ImageSrc);

   if (snapToPixelGrid) {
      snapToPixelGrid = false;
      matrixDirty = true;
   }

   if (prepareForDraw(opaque)) {
      shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::ImageTexture),
            QT_IMAGE_TEXTURE_UNIT);
   }

   if (pattern) {
      QColor col = qt_premultiplyColor(q->state()->pen.color(), (GLfloat)q->state()->opacity);
      shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::PatternColor), col);
   }

   GLfloat dx = 1.0 / textureSize.width();
   GLfloat dy = 1.0 / textureSize.height();

   QGLRect srcTextureRect(src.left * dx, src.top * dy, src.right * dx, src.bottom * dy);

   setCoords(staticVertexCoordinateArray, dest);
   setCoords(staticTextureCoordinateArray, srcTextureRect);

   glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void QGL2PaintEngineEx::beginNativePainting()
{
   Q_D(QGL2PaintEngineEx);
   ensureActive();
   d->transferMode(BrushDrawingMode);

   d->nativePaintingActive = true;

   QGLContext *ctx = d->ctx;
   glUseProgram(0);

   // Disable all the vertex attribute arrays:
   for (int i = 0; i < QT_GL_VERTEX_ARRAY_TRACKED_COUNT; ++i) {
      glDisableVertexAttribArray(i);
   }

#ifndef QT_OPENGL_ES_2
   const QGLFormat &fmt = d->device->format();
   if (fmt.majorVersion() < 3 || (fmt.majorVersion() == 3 && fmt.minorVersion() < 1)
         || (fmt.majorVersion() == 3 && fmt.minorVersion() == 1 && d->hasCompatibilityExtension)
         || fmt.profile() == QGLFormat::CompatibilityProfile) {
      // be nice to people who mix OpenGL 1.x code with QPainter commands
      // by setting modelview and projection matrices to mirror the GL 1
      // paint engine
      const QTransform &mtx = state()->matrix;

      float mv_matrix[4][4] = {
         { float(mtx.m11()), float(mtx.m12()),     0, float(mtx.m13()) },
         { float(mtx.m21()), float(mtx.m22()),     0, float(mtx.m23()) },
         {                0,                0,     1,                0 },
         {  float(mtx.dx()),  float(mtx.dy()),     0, float(mtx.m33()) }
      };

      const QSize sz = d->device->size();

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, sz.width(), sz.height(), 0, -999999, 999999);

      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(&mv_matrix[0][0]);
   }
#else
   Q_UNUSED(ctx);
#endif

   d->lastTextureUsed = GLuint(-1);
   d->dirtyStencilRegion = QRect(0, 0, d->width, d->height);
   d->resetGLState();

   d->shaderManager->setDirty();

   d->needsSync = true;
}

void QGL2PaintEngineExPrivate::resetGLState()
{
   glDisable(GL_BLEND);
   glActiveTexture(GL_TEXTURE0);
   glDisable(GL_STENCIL_TEST);
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_SCISSOR_TEST);
   glDepthMask(true);
   glDepthFunc(GL_LESS);
   glClearDepth(1);
   glStencilMask(0xff);
   glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
   glStencilFunc(GL_ALWAYS, 0, 0xff);
   ctx->d_func()->setVertexAttribArrayEnabled(QT_TEXTURE_COORDS_ATTR, false);
   ctx->d_func()->setVertexAttribArrayEnabled(QT_VERTEX_COORDS_ATTR, false);
   ctx->d_func()->setVertexAttribArrayEnabled(QT_OPACITY_ATTR, false);
#ifndef QT_OPENGL_ES_2
   // gl_Color, corresponding to vertex attribute 3, may have been changed
   float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
   glVertexAttrib4fv(3, color);
#endif
}

void QGL2PaintEngineEx::endNativePainting()
{
   Q_D(QGL2PaintEngineEx);
   d->needsSync = true;
   d->nativePaintingActive = false;
}

void QGL2PaintEngineEx::invalidateState()
{
   Q_D(QGL2PaintEngineEx);
   d->needsSync = true;
}

bool QGL2PaintEngineEx::isNativePaintingActive() const
{
   Q_D(const QGL2PaintEngineEx);
   return d->nativePaintingActive;
}

void QGL2PaintEngineExPrivate::transferMode(EngineMode newMode)
{
   if (newMode == mode) {
      return;
   }

   if (mode == TextDrawingMode || imageDrawingMode) {
      lastTextureUsed = GLuint(-1);
   }

   if (newMode == TextDrawingMode) {
      shaderManager->setHasComplexGeometry(true);
   } else {
      shaderManager->setHasComplexGeometry(false);
   }

   imageDrawingMode = false;

   if (newMode == ImageDrawingMode) {
      setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, staticVertexCoordinateArray);
      setVertexAttributePointer(QT_TEXTURE_COORDS_ATTR, staticTextureCoordinateArray);
      imageDrawingMode = true;
   }

   if (newMode == ImageArrayDrawingMode || newMode == ImageArrayWithOpacityDrawingMode) {
      setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, (GLfloat *)vertexCoordinateArray.data());
      setVertexAttributePointer(QT_TEXTURE_COORDS_ATTR, (GLfloat *)textureCoordinateArray.data());
      imageDrawingMode = true;
   }

   if (newMode == ImageArrayWithOpacityDrawingMode) {
      setVertexAttributePointer(QT_OPACITY_ATTR, (GLfloat *)opacityArray.data());
   }

   // This needs to change when we implement high-quality anti-aliasing...
   if (newMode != TextDrawingMode) {
      shaderManager->setMaskType(QGLEngineShaderManager::NoMask);
   }

   mode = newMode;
}

struct QGL2PEVectorPathCache {
#ifdef QT_OPENGL_CACHE_AS_VBOS
   GLuint vbo;
   GLuint ibo;
#else
   float *vertices;
   void *indices;
#endif
   int vertexCount;
   int indexCount;
   GLenum primitiveType;
   qreal iscale;
};

void QGL2PaintEngineExPrivate::cleanupVectorPath(QPaintEngineEx *engine, void *data)
{
   QGL2PEVectorPathCache *c = (QGL2PEVectorPathCache *) data;
#ifdef QT_OPENGL_CACHE_AS_VBOS
   Q_ASSERT(engine->type() == QPaintEngine::OpenGL2);
   static_cast<QGL2PaintEngineEx *>(engine)->d_func()->unusedVBOSToClean << c->vbo;
   if (c->ibo) {
      d->unusedIBOSToClean << c->ibo;
   }
#else
   Q_UNUSED(engine);
   free(c->vertices);
   free(c->indices);
#endif
   delete c;
}

// Assumes everything is configured for the brush you want to use
void QGL2PaintEngineExPrivate::fill(const QVectorPath &path)
{
   transferMode(BrushDrawingMode);

   if (snapToPixelGrid) {
      snapToPixelGrid = false;
      matrixDirty = true;
   }

   // Might need to call updateMatrix to re-calculate inverseScale
   if (matrixDirty) {
      updateMatrix();
   }

   const QPointF *const points = reinterpret_cast<const QPointF *>(path.points());

   // Check to see if there's any hints
   if (path.shape() == QVectorPath::RectangleHint) {
      QGLRect rect(points[0].x(), points[0].y(), points[2].x(), points[2].y());
      prepareForDraw(currentBrush.isOpaque());
      composite(rect);
   } else if (path.isConvex()) {

      if (path.isCacheable()) {
         QVectorPath::CacheEntry *data = path.lookupCacheData(q);
         QGL2PEVectorPathCache *cache;

         bool updateCache = false;

         if (data) {
            cache = (QGL2PEVectorPathCache *) data->data;
            // Check if scale factor is exceeded for curved paths and generate curves if so...
            if (path.isCurved()) {
               qreal scaleFactor = cache->iscale / inverseScale;
               if (scaleFactor < 0.5 || scaleFactor > 2.0) {
#ifdef QT_OPENGL_CACHE_AS_VBOS
                  glDeleteBuffers(1, &cache->vbo);
                  cache->vbo = 0;
                  Q_ASSERT(cache->ibo == 0);
#else
                  free(cache->vertices);
                  Q_ASSERT(cache->indices == 0);
#endif
                  updateCache = true;
               }
            }
         } else {
            cache = new QGL2PEVectorPathCache;
            data = const_cast<QVectorPath &>(path).addCacheData(q, cache, cleanupVectorPath);
            updateCache = true;
         }

         // Flatten the path at the current scale factor and fill it into the cache struct.
         if (updateCache) {
            vertexCoordinateArray.clear();
            vertexCoordinateArray.addPath(path, inverseScale, false);
            int vertexCount = vertexCoordinateArray.vertexCount();
            int floatSizeInBytes = vertexCount * 2 * sizeof(float);
            cache->vertexCount = vertexCount;
            cache->indexCount = 0;
            cache->primitiveType = GL_TRIANGLE_FAN;
            cache->iscale = inverseScale;
#ifdef QT_OPENGL_CACHE_AS_VBOS
            glGenBuffers(1, &cache->vbo);
            glBindBuffer(GL_ARRAY_BUFFER, cache->vbo);
            glBufferData(GL_ARRAY_BUFFER, floatSizeInBytes, vertexCoordinateArray.data(), GL_STATIC_DRAW);
            cache->ibo = 0;
#else
            cache->vertices = (float *) malloc(floatSizeInBytes);
            memcpy(cache->vertices, vertexCoordinateArray.data(), floatSizeInBytes);
            cache->indices = 0;
#endif
         }

         prepareForDraw(currentBrush.isOpaque());
#ifdef QT_OPENGL_CACHE_AS_VBOS
         glBindBuffer(GL_ARRAY_BUFFER, cache->vbo);
         setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, 0);
#else
         setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, cache->vertices);
#endif
         glDrawArrays(cache->primitiveType, 0, cache->vertexCount);

      } else {
         //        printf(" - Marking path as cachable...\n");
         // Tag it for later so that if the same path is drawn twice, it is assumed to be static and thus cachable
         path.makeCacheable();
         vertexCoordinateArray.clear();
         vertexCoordinateArray.addPath(path, inverseScale, false);
         prepareForDraw(currentBrush.isOpaque());
         drawVertexArrays(vertexCoordinateArray, GL_TRIANGLE_FAN);
      }

   } else {
      bool useCache = path.isCacheable();
      if (useCache) {
         QRectF bbox = path.controlPointRect();
         // If the path doesn't fit within these limits, it is possible that the triangulation will fail.
         useCache &= (bbox.left() > -0x8000 * inverseScale)
                     && (bbox.right() < 0x8000 * inverseScale)
                     && (bbox.top() > -0x8000 * inverseScale)
                     && (bbox.bottom() < 0x8000 * inverseScale);
      }

      if (useCache) {
         QVectorPath::CacheEntry *data = path.lookupCacheData(q);
         QGL2PEVectorPathCache *cache;

         bool updateCache = false;

         if (data) {
            cache = (QGL2PEVectorPathCache *) data->data;
            // Check if scale factor is exceeded for curved paths and generate curves if so...
            if (path.isCurved()) {
               qreal scaleFactor = cache->iscale / inverseScale;
               if (scaleFactor < 0.5 || scaleFactor > 2.0) {
#ifdef QT_OPENGL_CACHE_AS_VBOS
                  glDeleteBuffers(1, &cache->vbo);
                  glDeleteBuffers(1, &cache->ibo);
#else
                  free(cache->vertices);
                  free(cache->indices);
#endif
                  updateCache = true;
               }
            }
         } else {
            cache = new QGL2PEVectorPathCache;
            data = const_cast<QVectorPath &>(path).addCacheData(q, cache, cleanupVectorPath);
            updateCache = true;
         }

         // Flatten the path at the current scale factor and fill it into the cache struct.
         if (updateCache) {
            QTriangleSet polys = qTriangulate(path, QTransform().scale(1 / inverseScale, 1 / inverseScale));
            cache->vertexCount = polys.vertices.size() / 2;
            cache->indexCount = polys.indices.size();
            cache->primitiveType = GL_TRIANGLES;
            cache->iscale = inverseScale;
#ifdef QT_OPENGL_CACHE_AS_VBOS
            glGenBuffers(1, &cache->vbo);
            glGenBuffers(1, &cache->ibo);
            glBindBuffer(GL_ARRAY_BUFFER, cache->vbo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cache->ibo);

            if (QGLExtensions::glExtensions() & QGLExtensions::ElementIndexUint) {
               glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quint32) * polys.indices.size(), polys.indices.data(), GL_STATIC_DRAW);
            } else {
               glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quint16) * polys.indices.size(), polys.indices.data(), GL_STATIC_DRAW);
            }

            QVarLengthArray<float> vertices(polys.vertices.size());
            for (int i = 0; i < polys.vertices.size(); ++i) {
               vertices[i] = float(inverseScale * polys.vertices.at(i));
            }
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
#else
            cache->vertices = (float *) malloc(sizeof(float) * polys.vertices.size());
            if (QGLExtensions::glExtensions() & QGLExtensions::ElementIndexUint) {
               cache->indices = (quint32 *) malloc(sizeof(quint32) * polys.indices.size());
               memcpy(cache->indices, polys.indices.data(), sizeof(quint32) * polys.indices.size());
            } else {
               cache->indices = (quint16 *) malloc(sizeof(quint16) * polys.indices.size());
               memcpy(cache->indices, polys.indices.data(), sizeof(quint16) * polys.indices.size());
            }
            for (int i = 0; i < polys.vertices.size(); ++i) {
               cache->vertices[i] = float(inverseScale * polys.vertices.at(i));
            }
#endif
         }

         prepareForDraw(currentBrush.isOpaque());
#ifdef QT_OPENGL_CACHE_AS_VBOS
         glBindBuffer(GL_ARRAY_BUFFER, cache->vbo);
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cache->ibo);
         setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, 0);
         if (QGLExtensions::glExtensions() & QGLExtensions::ElementIndexUint) {
            glDrawElements(cache->primitiveType, cache->indexCount, GL_UNSIGNED_INT, 0);
         } else {
            glDrawElements(cache->primitiveType, cache->indexCount, GL_UNSIGNED_SHORT, 0);
         }
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
         glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
         setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, cache->vertices);
         if (QGLExtensions::glExtensions() & QGLExtensions::ElementIndexUint) {
            glDrawElements(cache->primitiveType, cache->indexCount, GL_UNSIGNED_INT, (qint32 *)cache->indices);
         } else {
            glDrawElements(cache->primitiveType, cache->indexCount, GL_UNSIGNED_SHORT, (qint16 *)cache->indices);
         }
#endif

      } else {
         //        printf(" - Marking path as cachable...\n");
         // Tag it for later so that if the same path is drawn twice, it is assumed to be static and thus cachable
         path.makeCacheable();

         if (!device->format().stencil()) {
            // If there is no stencil buffer, triangulate the path instead.

            QRectF bbox = path.controlPointRect();
            // If the path doesn't fit within these limits, it is possible that the triangulation will fail.
            bool withinLimits = (bbox.left() > -0x8000 * inverseScale)
                                && (bbox.right() < 0x8000 * inverseScale)
                                && (bbox.top() > -0x8000 * inverseScale)
                                && (bbox.bottom() < 0x8000 * inverseScale);
            if (withinLimits) {
               QTriangleSet polys = qTriangulate(path, QTransform().scale(1 / inverseScale, 1 / inverseScale));

               QVarLengthArray<float> vertices(polys.vertices.size());
               for (int i = 0; i < polys.vertices.size(); ++i) {
                  vertices[i] = float(inverseScale * polys.vertices.at(i));
               }

               prepareForDraw(currentBrush.isOpaque());
               setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, vertices.constData());
               if (QGLExtensions::glExtensions() & QGLExtensions::ElementIndexUint) {
                  glDrawElements(GL_TRIANGLES, polys.indices.size(), GL_UNSIGNED_INT, polys.indices.data());
               } else {
                  glDrawElements(GL_TRIANGLES, polys.indices.size(), GL_UNSIGNED_SHORT, polys.indices.data());
               }
            } else {
               // We can't handle big, concave painter paths with OpenGL without stencil buffer.
               qWarning("Painter path exceeds +/-32767 pixels.");
            }
            return;
         }

         // The path is too complicated & needs the stencil technique
         vertexCoordinateArray.clear();
         vertexCoordinateArray.addPath(path, inverseScale, false);

         fillStencilWithVertexArray(vertexCoordinateArray, path.hasWindingFill());

         glStencilMask(0xff);
         glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);

         if (q->state()->clipTestEnabled) {
            // Pass when high bit is set, replace stencil value with current clip
            glStencilFunc(GL_NOTEQUAL, q->state()->currentClip, GL_STENCIL_HIGH_BIT);
         } else if (path.hasWindingFill()) {
            // Pass when any bit is set, replace stencil value with 0
            glStencilFunc(GL_NOTEQUAL, 0, 0xff);
         } else {
            // Pass when high bit is set, replace stencil value with 0
            glStencilFunc(GL_NOTEQUAL, 0, GL_STENCIL_HIGH_BIT);
         }
         prepareForDraw(currentBrush.isOpaque());

         // Stencil the brush onto the dest buffer
         composite(vertexCoordinateArray.boundingRect());
         glStencilMask(0);
         updateClipScissorTest();
      }
   }
}


void QGL2PaintEngineExPrivate::fillStencilWithVertexArray(const float *data,
      int count,
      int *stops,
      int stopCount,
      const QGLRect &bounds,
      StencilFillMode mode)
{
   Q_ASSERT(count || stops);

   //     qDebug("QGL2PaintEngineExPrivate::fillStencilWithVertexArray()");
   glStencilMask(0xff); // Enable stencil writes

   if (dirtyStencilRegion.intersects(currentScissorBounds)) {
      QVector<QRect> clearRegion = dirtyStencilRegion.intersected(currentScissorBounds).rects();
      glClearStencil(0); // Clear to zero
      for (int i = 0; i < clearRegion.size(); ++i) {
#ifndef QT_GL_NO_SCISSOR_TEST
         setScissor(clearRegion.at(i));
#endif
         glClear(GL_STENCIL_BUFFER_BIT);
      }

      dirtyStencilRegion -= currentScissorBounds;

#ifndef QT_GL_NO_SCISSOR_TEST
      updateClipScissorTest();
#endif
   }

   glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Disable color writes
   useSimpleShader();
   glEnable(GL_STENCIL_TEST); // For some reason, this has to happen _after_ the simple shader is use()'d

   if (mode == WindingFillMode) {
      Q_ASSERT(stops && !count);
      if (q->state()->clipTestEnabled) {
         // Flatten clip values higher than current clip, and set high bit to match current clip
         glStencilFunc(GL_LEQUAL, GL_STENCIL_HIGH_BIT | q->state()->currentClip, ~GL_STENCIL_HIGH_BIT);
         glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
         composite(bounds);

         glStencilFunc(GL_EQUAL, GL_STENCIL_HIGH_BIT, GL_STENCIL_HIGH_BIT);
      } else if (!stencilClean) {
         // Clear stencil buffer within bounding rect
         glStencilFunc(GL_ALWAYS, 0, 0xff);
         glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
         composite(bounds);
      }

      // Inc. for front-facing triangle
      glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_INCR_WRAP);
      // Dec. for back-facing "holes"
      glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR_WRAP, GL_DECR_WRAP);
      glStencilMask(~GL_STENCIL_HIGH_BIT);
      drawVertexArrays(data, stops, stopCount, GL_TRIANGLE_FAN);

      if (q->state()->clipTestEnabled) {
         // Clear high bit of stencil outside of path
         glStencilFunc(GL_EQUAL, q->state()->currentClip, ~GL_STENCIL_HIGH_BIT);
         glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
         glStencilMask(GL_STENCIL_HIGH_BIT);
         composite(bounds);
      }
   } else if (mode == OddEvenFillMode) {
      glStencilMask(GL_STENCIL_HIGH_BIT);
      glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT); // Simply invert the stencil bit
      drawVertexArrays(data, stops, stopCount, GL_TRIANGLE_FAN);

   } else { // TriStripStrokeFillMode
      Q_ASSERT(count && !stops); // tristrips generated directly, so no vertexArray or stops
      glStencilMask(GL_STENCIL_HIGH_BIT);
#if 0
      glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT); // Simply invert the stencil bit
      setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, data);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
#else

      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
      if (q->state()->clipTestEnabled) {
         glStencilFunc(GL_LEQUAL, q->state()->currentClip | GL_STENCIL_HIGH_BIT,
                       ~GL_STENCIL_HIGH_BIT);
      } else {
         glStencilFunc(GL_ALWAYS, GL_STENCIL_HIGH_BIT, 0xff);
      }
      setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, data);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
#endif
   }

   // Enable color writes & disable stencil writes
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

/*
    If the maximum value in the stencil buffer is GL_STENCIL_HIGH_BIT - 1,
    restore the stencil buffer to a pristine state.  The current clip region
    is set to 1, and the rest to 0.
*/
void QGL2PaintEngineExPrivate::resetClipIfNeeded()
{
   if (maxClip != (GL_STENCIL_HIGH_BIT - 1)) {
      return;
   }

   Q_Q(QGL2PaintEngineEx);

   useSimpleShader();
   glEnable(GL_STENCIL_TEST);
   glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

   QRectF bounds = q->state()->matrix.inverted().mapRect(QRectF(0, 0, width, height));
   QGLRect rect(bounds.left(), bounds.top(), bounds.right(), bounds.bottom());

   // Set high bit on clip region
   glStencilFunc(GL_LEQUAL, q->state()->currentClip, 0xff);
   glStencilOp(GL_KEEP, GL_INVERT, GL_INVERT);
   glStencilMask(GL_STENCIL_HIGH_BIT);
   composite(rect);

   // Reset clipping to 1 and everything else to zero
   glStencilFunc(GL_NOTEQUAL, 0x01, GL_STENCIL_HIGH_BIT);
   glStencilOp(GL_ZERO, GL_REPLACE, GL_REPLACE);
   glStencilMask(0xff);
   composite(rect);

   q->state()->currentClip = 1;
   q->state()->canRestoreClip = false;

   maxClip = 1;

   glStencilMask(0x0);
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

bool QGL2PaintEngineExPrivate::prepareForDraw(bool srcPixelsAreOpaque)
{
   if (brushTextureDirty && !imageDrawingMode) {
      updateBrushTexture();
   }

   if (compositionModeDirty) {
      updateCompositionMode();
   }

   if (matrixDirty) {
      updateMatrix();
   }

   const bool stateHasOpacity = q->state()->opacity < 0.99f;
   if (q->state()->composition_mode == QPainter::CompositionMode_Source
         || (q->state()->composition_mode == QPainter::CompositionMode_SourceOver
             && srcPixelsAreOpaque && !stateHasOpacity)) {
      glDisable(GL_BLEND);
   } else {
      glEnable(GL_BLEND);
   }

   QGLEngineShaderManager::OpacityMode opacityMode;
   if (mode == ImageArrayWithOpacityDrawingMode) {
      opacityMode = QGLEngineShaderManager::AttributeOpacity;
   } else {
      opacityMode = stateHasOpacity ? QGLEngineShaderManager::UniformOpacity
                    : QGLEngineShaderManager::NoOpacity;
      if (stateHasOpacity && !imageDrawingMode) {
         // Using a brush
         bool brushIsPattern = (currentBrush.style() >= Qt::Dense1Pattern) &&
                               (currentBrush.style() <= Qt::DiagCrossPattern);

         if ((currentBrush.style() == Qt::SolidPattern) || brushIsPattern) {
            opacityMode = QGLEngineShaderManager::NoOpacity;   // Global opacity handled by srcPixel shader
         }
      }
   }
   shaderManager->setOpacityMode(opacityMode);

   bool changed = shaderManager->useCorrectShaderProg();
   // If the shader program needs changing, we change it and mark all uniforms as dirty
   if (changed) {
      // The shader program has changed so mark all uniforms as dirty:
      brushUniformsDirty = true;
      opacityUniformDirty = true;
      matrixUniformDirty = true;
   }

   if (brushUniformsDirty && !imageDrawingMode) {
      updateBrushUniforms();
   }

   if (opacityMode == QGLEngineShaderManager::UniformOpacity && opacityUniformDirty) {
      shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::GlobalOpacity),
            (GLfloat)q->state()->opacity);
      opacityUniformDirty = false;
   }

   if (matrixUniformDirty && shaderManager->hasComplexGeometry()) {
      shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::Matrix),
            pmvMatrix);
      matrixUniformDirty = false;
   }

   return changed;
}

void QGL2PaintEngineExPrivate::composite(const QGLRect &boundingRect)
{
   setCoords(staticVertexCoordinateArray, boundingRect);
   setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, staticVertexCoordinateArray);
   glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

// Draws the vertex array as a set of <vertexArrayStops.size()> triangle fans.
void QGL2PaintEngineExPrivate::drawVertexArrays(const float *data, int *stops, int stopCount,
      GLenum primitive)
{
   // Now setup the pointer to the vertex array:
   setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, (GLfloat *)data);

   int previousStop = 0;
   for (int i = 0; i < stopCount; ++i) {
      int stop = stops[i];
      /*
              qDebug("Drawing triangle fan for vertecies %d -> %d:", previousStop, stop-1);
              for (int i=previousStop; i<stop; ++i)
                  qDebug("   %02d: [%.2f, %.2f]", i, vertexArray.data()[i].x, vertexArray.data()[i].y);
      */
      glDrawArrays(primitive, previousStop, stop - previousStop);
      previousStop = stop;
   }
}

/////////////////////////////////// Public Methods //////////////////////////////////////////

QGL2PaintEngineEx::QGL2PaintEngineEx()
   : QPaintEngineEx(*(new QGL2PaintEngineExPrivate(this)))
{
}

QGL2PaintEngineEx::~QGL2PaintEngineEx()
{
}

void QGL2PaintEngineEx::fill(const QVectorPath &path, const QBrush &brush)
{
   Q_D(QGL2PaintEngineEx);

   if (qbrush_style(brush) == Qt::NoBrush) {
      return;
   }
   ensureActive();
   d->setBrush(brush);
   d->fill(path);
}

Q_GUI_EXPORT bool qt_scaleForTransform(const QTransform &transform, qreal *scale); // qtransform.cpp


void QGL2PaintEngineEx::stroke(const QVectorPath &path, const QPen &pen)
{
   Q_D(QGL2PaintEngineEx);

   const QBrush &penBrush = qpen_brush(pen);
   if (qpen_style(pen) == Qt::NoPen || qbrush_style(penBrush) == Qt::NoBrush) {
      return;
   }

   QOpenGL2PaintEngineState *s = state();
   if (pen.isCosmetic() && !qt_scaleForTransform(s->transform(), 0)) {
      // QTriangulatingStroker class is not meant to support cosmetically sheared strokes.
      QPaintEngineEx::stroke(path, pen);
      return;
   }

   ensureActive();
   d->setBrush(penBrush);
   d->stroke(path, pen);
}

void QGL2PaintEngineExPrivate::stroke(const QVectorPath &path, const QPen &pen)
{
   const QOpenGL2PaintEngineState *s = q->state();
   if (snapToPixelGrid) {
      snapToPixelGrid = false;
      matrixDirty = true;
   }

   const Qt::PenStyle penStyle = qpen_style(pen);
   const QBrush &penBrush = qpen_brush(pen);
   const bool opaque = penBrush.isOpaque() && s->opacity > 0.99;

   transferMode(BrushDrawingMode);

   // updateMatrix() is responsible for setting the inverse scale on
   // the strokers, so we need to call it here and not wait for
   // prepareForDraw() down below.
   updateMatrix();

   QRectF clip = q->state()->matrix.inverted().mapRect(q->state()->clipEnabled
                 ? q->state()->rectangleClip
                 : QRectF(0, 0, width, height));

   if (penStyle == Qt::SolidLine) {
      stroker.process(path, pen, clip);

   } else { // Some sort of dash
      dasher.process(path, pen, clip);

      QVectorPath dashStroke(dasher.points(),
                             dasher.elementCount(),
                             dasher.elementTypes());
      stroker.process(dashStroke, pen, clip);
   }

   if (!stroker.vertexCount()) {
      return;
   }

   if (opaque) {
      prepareForDraw(opaque);
      setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, stroker.vertices());
      glDrawArrays(GL_TRIANGLE_STRIP, 0, stroker.vertexCount() / 2);

      //         QBrush b(Qt::green);
      //         d->setBrush(&b);
      //         d->prepareForDraw(true);
      //         glDrawArrays(GL_LINE_STRIP, 0, d->stroker.vertexCount() / 2);

   } else {
      qreal width = qpen_widthf(pen) / 2;
      if (width == 0) {
         width = 0.5;
      }
      qreal extra = pen.joinStyle() == Qt::MiterJoin
                    ? qMax(pen.miterLimit() * width, width)
                    : width;

      if (pen.isCosmetic()) {
         extra = extra * inverseScale;
      }

      QRectF bounds = path.controlPointRect().adjusted(-extra, -extra, extra, extra);

      fillStencilWithVertexArray(stroker.vertices(), stroker.vertexCount() / 2,
                                 0, 0, bounds, QGL2PaintEngineExPrivate::TriStripStrokeFillMode);

      glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);

      // Pass when any bit is set, replace stencil value with 0
      glStencilFunc(GL_NOTEQUAL, 0, GL_STENCIL_HIGH_BIT);
      prepareForDraw(false);

      // Stencil the brush onto the dest buffer
      composite(bounds);

      glStencilMask(0);

      updateClipScissorTest();
   }
}

void QGL2PaintEngineEx::penChanged() { }
void QGL2PaintEngineEx::brushChanged() { }
void QGL2PaintEngineEx::brushOriginChanged() { }

void QGL2PaintEngineEx::opacityChanged()
{
   //    qDebug("QGL2PaintEngineEx::opacityChanged()");
   Q_D(QGL2PaintEngineEx);
   state()->opacityChanged = true;

   Q_ASSERT(d->shaderManager);
   d->brushUniformsDirty = true;
   d->opacityUniformDirty = true;
}

void QGL2PaintEngineEx::compositionModeChanged()
{
   //     qDebug("QGL2PaintEngineEx::compositionModeChanged()");
   Q_D(QGL2PaintEngineEx);
   state()->compositionModeChanged = true;
   d->compositionModeDirty = true;
}

void QGL2PaintEngineEx::renderHintsChanged()
{
   state()->renderHintsChanged = true;

#if !defined(QT_OPENGL_ES_2)
   if ((state()->renderHints & QPainter::Antialiasing)
         || (state()->renderHints & QPainter::HighQualityAntialiasing)) {
      glEnable(GL_MULTISAMPLE);
   } else {
      glDisable(GL_MULTISAMPLE);
   }
#endif

   Q_D(QGL2PaintEngineEx);
   d->lastTextureUsed = GLuint(-1);
   d->brushTextureDirty = true;
   //    qDebug("QGL2PaintEngineEx::renderHintsChanged() not implemented!");
}

void QGL2PaintEngineEx::transformChanged()
{
   Q_D(QGL2PaintEngineEx);
   d->matrixDirty = true;
   state()->matrixChanged = true;
}


static const QRectF scaleRect(const QRectF &r, qreal sx, qreal sy)
{
   return QRectF(r.x() * sx, r.y() * sy, r.width() * sx, r.height() * sy);
}

void QGL2PaintEngineEx::drawPixmap(const QRectF &dest, const QPixmap &pixmap, const QRectF &src)
{
   Q_D(QGL2PaintEngineEx);
   QGLContext *ctx = d->ctx;

   int max_texture_size = ctx->d_func()->maxTextureSize();
   if (pixmap.width() > max_texture_size || pixmap.height() > max_texture_size) {
      QPixmap scaled = pixmap.scaled(max_texture_size, max_texture_size, Qt::KeepAspectRatio);

      const qreal sx = scaled.width() / qreal(pixmap.width());
      const qreal sy = scaled.height() / qreal(pixmap.height());

      drawPixmap(dest, scaled, scaleRect(src, sx, sy));
      return;
   }

   ensureActive();
   d->transferMode(ImageDrawingMode);

   glActiveTexture(GL_TEXTURE0 + QT_IMAGE_TEXTURE_UNIT);
   QGLTexture *texture =
      ctx->d_func()->bindTexture(pixmap, GL_TEXTURE_2D, GL_RGBA,
                                 QGLContext::InternalBindOption
                                 | QGLContext::CanFlipNativePixmapBindOption);

   GLfloat top = texture->options & QGLContext::InvertedYBindOption ? (pixmap.height() - src.top()) : src.top();
   GLfloat bottom = texture->options & QGLContext::InvertedYBindOption ? (pixmap.height() - src.bottom()) : src.bottom();
   QGLRect srcRect(src.left(), top, src.right(), bottom);

   bool isBitmap = pixmap.isQBitmap();
   bool isOpaque = !isBitmap && !pixmap.hasAlpha();

   d->updateTextureFilter(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE,
                          state()->renderHints & QPainter::SmoothPixmapTransform, texture->id);
   d->drawTexture(dest, srcRect, pixmap.size(), isOpaque, isBitmap);
}

void QGL2PaintEngineEx::drawImage(const QRectF &dest, const QImage &image, const QRectF &src,
                                  Qt::ImageConversionFlags)
{
   Q_D(QGL2PaintEngineEx);
   QGLContext *ctx = d->ctx;

   int max_texture_size = ctx->d_func()->maxTextureSize();
   if (image.width() > max_texture_size || image.height() > max_texture_size) {
      QImage scaled = image.scaled(max_texture_size, max_texture_size, Qt::KeepAspectRatio);

      const qreal sx = scaled.width() / qreal(image.width());
      const qreal sy = scaled.height() / qreal(image.height());

      drawImage(dest, scaled, scaleRect(src, sx, sy));
      return;
   }

   ensureActive();
   d->transferMode(ImageDrawingMode);

   glActiveTexture(GL_TEXTURE0 + QT_IMAGE_TEXTURE_UNIT);

   QGLTexture *texture = ctx->d_func()->bindTexture(image, GL_TEXTURE_2D, GL_RGBA, QGLContext::InternalBindOption);
   GLuint id = texture->id;

   d->updateTextureFilter(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE,
                          state()->renderHints & QPainter::SmoothPixmapTransform, id);
   d->drawTexture(dest, src, image.size(), !image.hasAlphaChannel());
}

void QGL2PaintEngineEx::drawStaticTextItem(QStaticTextItem *textItem)
{
   Q_D(QGL2PaintEngineEx);

   ensureActive();

   QPainterState *s = state();
   float det = s->matrix.determinant();

   // don't try to cache huge fonts or vastly transformed fonts
   QFontEngine *fontEngine = textItem->fontEngine();
   const qreal pixelSize = fontEngine->fontDef.pixelSize;

   if (pixelSize * pixelSize * qAbs(det) < QT_MAX_CACHED_GLYPH_SIZE * QT_MAX_CACHED_GLYPH_SIZE && det >= 0.25f && det <= 4.f) {

      QFontEngineGlyphCache::Type glyphType = fontEngine->glyphFormat >= 0
                  ? QFontEngineGlyphCache::Type(textItem->fontEngine()->glyphFormat) : d->glyphCacheType;

      if (glyphType == QFontEngineGlyphCache::Raster_RGBMask) {
         if (!QGLFramebufferObject::hasOpenGLFramebufferObjects()
               || d->device->alphaRequested() || s->matrix.type() > QTransform::TxTranslate
               || (s->composition_mode != QPainter::CompositionMode_Source
                   && s->composition_mode != QPainter::CompositionMode_SourceOver)) {
            glyphType = QFontEngineGlyphCache::Raster_A8;
         }
      }

      d->drawCachedGlyphs(glyphType, textItem);

   } else {
      QPaintEngineEx::drawStaticTextItem(textItem);
   }
}

bool QGL2PaintEngineEx::drawTexture(const QRectF &dest, GLuint textureId, const QSize &size, const QRectF &src)
{
   Q_D(QGL2PaintEngineEx);
   if (!d->shaderManager) {
      return false;
   }

   ensureActive();
   d->transferMode(ImageDrawingMode);

#ifndef QT_OPENGL_ES_2
   QGLContext *ctx = d->ctx;
#endif
   glActiveTexture(GL_TEXTURE0 + QT_IMAGE_TEXTURE_UNIT);
   glBindTexture(GL_TEXTURE_2D, textureId);

   QGLRect srcRect(src.left(), src.bottom(), src.right(), src.top());

   d->updateTextureFilter(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE,
                          state()->renderHints & QPainter::SmoothPixmapTransform, textureId);
   d->drawTexture(dest, srcRect, size, false);
   return true;
}

void QGL2PaintEngineEx::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
   Q_D(QGL2PaintEngineEx);

   ensureActive();
   QOpenGL2PaintEngineState *s = state();

   const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

   QTransform::TransformationType txtype = s->matrix.type();

   float det = s->matrix.determinant();
   bool drawCached = txtype < QTransform::TxProject;

   // do not try to cache huge fonts or vastly transformed fonts
   const qreal pixelSize = ti.fontEngine->fontDef.pixelSize;

   if (pixelSize * pixelSize * qAbs(det) >= QT_MAX_CACHED_GLYPH_SIZE * QT_MAX_CACHED_GLYPH_SIZE ||
                  det < 0.25f || det > 4.f) {
      drawCached = false;
   }

   QFontEngineGlyphCache::Type glyphType = ti.fontEngine->glyphFormat >= 0
                  ? QFontEngineGlyphCache::Type(ti.fontEngine->glyphFormat) : d->glyphCacheType;

   if (glyphType == QFontEngineGlyphCache::Raster_RGBMask) {
      if (! QGLFramebufferObject::hasOpenGLFramebufferObjects()
            || d->device->alphaRequested() || txtype > QTransform::TxTranslate
            || (state()->composition_mode != QPainter::CompositionMode_Source
                && state()->composition_mode != QPainter::CompositionMode_SourceOver)) {
         glyphType = QFontEngineGlyphCache::Raster_A8;
      }
   }

   if (drawCached) {
      QVarLengthArray<QFixedPoint> positions;
      QVarLengthArray<glyph_t> glyphs;
      QTransform matrix = QTransform::fromTranslate(p.x(), p.y());

      ti.fontEngine->getGlyphPositions(ti.glyphs, matrix, ti.flags, glyphs, positions);

      {
         QStaticTextItem staticTextItem;

         staticTextItem.setFontEngine(ti.fontEngine);
         staticTextItem.glyphs         = glyphs.data();
         staticTextItem.numGlyphs      = glyphs.size();
         staticTextItem.glyphPositions = positions.data();

         d->drawCachedGlyphs(glyphType, &staticTextItem);
      }
      return;
   }

   QPaintEngineEx::drawTextItem(p, ti);
}

namespace {

class QOpenGLStaticTextUserData: public QStaticTextUserData
{
 public:
   QOpenGLStaticTextUserData()
      : QStaticTextUserData(OpenGLUserData), cacheSize(0, 0), cacheSerialNumber(0) {
   }

   ~QOpenGLStaticTextUserData() {
   }

   QSize cacheSize;
   QGL2PEXVertexArray vertexCoordinateArray;
   QGL2PEXVertexArray textureCoordinateArray;
   QFontEngineGlyphCache::Type glyphType;
   int cacheSerialNumber;
};

}

#if defined(Q_OS_WIN)
static bool fontSmoothingApproximately(qreal target)
{
   extern Q_GUI_EXPORT qreal qt_fontsmoothing_gamma; // qapplication_win.cpp
   return (qAbs(qt_fontsmoothing_gamma - target) < 0.2);
}
#endif

static inline qreal qt_sRGB_to_linear_RGB(qreal f)
{
   return f > 0.04045 ? qPow((f + 0.055) / 1.055, 2.4) : f / 12.92;
}

// #define QT_OPENGL_DRAWCACHEDGLYPHS_INDEX_ARRAY_VBO

void QGL2PaintEngineExPrivate::drawCachedGlyphs(QFontEngineGlyphCache::Type glyphType, QStaticTextItem *staticTextItem)
{
   Q_Q(QGL2PaintEngineEx);

   QOpenGL2PaintEngineState *s = q->state();

   void *cacheKey = const_cast<QGLContext *>(QGLContextPrivate::contextGroup(ctx)->context());
   bool recreateVertexArrays = false;

   QGLTextureGlyphCache *cache = (QGLTextureGlyphCache *) staticTextItem->fontEngine()->glyphCache(cacheKey, glyphType, QTransform());

   if (! cache || cache->cacheType() != glyphType || cache->context() == 0) {
      cache = new QGLTextureGlyphCache(ctx, glyphType, QTransform());
      staticTextItem->fontEngine()->setGlyphCache(cacheKey, cache);
      cache->insert(ctx, cache);
      recreateVertexArrays = true;
   }

   if (staticTextItem->userDataNeedsUpdate) {
      recreateVertexArrays = true;

   } else if (staticTextItem->userData() == 0) {
      recreateVertexArrays = true;

   } else if (staticTextItem->userData()->type != QStaticTextUserData::OpenGLUserData) {
      recreateVertexArrays = true;

   } else {
      QOpenGLStaticTextUserData *userData = static_cast<QOpenGLStaticTextUserData *>(staticTextItem->userData());

      if (userData->glyphType != glyphType) {
         recreateVertexArrays = true;
      } else if (userData->cacheSerialNumber != cache->serialNumber()) {
         recreateVertexArrays = true;
      }
   }

   // We only need to update the cache with new glyphs if we are actually going to recreate the vertex arrays.
   // If the cache size has changed, we do need to regenerate the vertices, but we don't need to repopulate the
   // cache so this text is performed before we test if the cache size has changed.
   if (recreateVertexArrays) {
      cache->setPaintEnginePrivate(this);

      if (!cache->populate(staticTextItem->fontEngine(), staticTextItem->numGlyphs,
                           staticTextItem->glyphs, staticTextItem->glyphPositions)) {
         // No space for glyphs in cache. We need to reset it and try again.
         cache->clear();

         cache->populate(staticTextItem->fontEngine(), staticTextItem->numGlyphs,
                         staticTextItem->glyphs, staticTextItem->glyphPositions);
      }

      cache->fillInPendingGlyphs();
   }

   if (cache->width() == 0 || cache->height() == 0) {
      return;
   }

   transferMode(TextDrawingMode);

   int margin = cache->glyphMargin();

   GLfloat dx = 1.0 / cache->width();
   GLfloat dy = 1.0 / cache->height();

   // Use global arrays by default
   QGL2PEXVertexArray *vertexCoordinates = &vertexCoordinateArray;
   QGL2PEXVertexArray *textureCoordinates = &textureCoordinateArray;

   if (staticTextItem->useBackendOptimizations) {
      QOpenGLStaticTextUserData *userData = 0;

      if (staticTextItem->userData() == 0
            || staticTextItem->userData()->type != QStaticTextUserData::OpenGLUserData) {

         userData = new QOpenGLStaticTextUserData();
         staticTextItem->setUserData(userData);

      } else {
         userData = static_cast<QOpenGLStaticTextUserData *>(staticTextItem->userData());
      }

      userData->glyphType = glyphType;
      userData->cacheSerialNumber = cache->serialNumber();

      // Use cache if backend optimizations is turned on
      vertexCoordinates = &userData->vertexCoordinateArray;
      textureCoordinates = &userData->textureCoordinateArray;

      QSize size(cache->width(), cache->height());
      if (userData->cacheSize != size) {
         recreateVertexArrays = true;
         userData->cacheSize = size;
      }
   }

   if (recreateVertexArrays) {
      vertexCoordinates->clear();
      textureCoordinates->clear();

      bool supportsSubPixelPositions = staticTextItem->fontEngine()->supportsSubPixelPositions();
      for (int i = 0; i < staticTextItem->numGlyphs; ++i) {
         QFixed subPixelPosition;
         if (supportsSubPixelPositions) {
            subPixelPosition = cache->subPixelPositionForX(staticTextItem->glyphPositions[i].x);
         }

         QTextureGlyphCache::GlyphAndSubPixelPosition glyph(staticTextItem->glyphs[i], subPixelPosition);

         const QTextureGlyphCache::Coord &c = cache->coords[glyph];
         if (c.isNull()) {
            continue;
         }

         int x = qFloor(staticTextItem->glyphPositions[i].x) + c.baseLineX - margin;
         int y = qFloor(staticTextItem->glyphPositions[i].y) - c.baseLineY - margin;

         vertexCoordinates->addQuad(QRectF(x, y, c.w, c.h));
         textureCoordinates->addQuad(QRectF(c.x * dx, c.y * dy, c.w * dx, c.h * dy));
      }

      staticTextItem->userDataNeedsUpdate = false;
   }

   int numGlyphs = vertexCoordinates->vertexCount() / 4;
   if (numGlyphs == 0) {
      return;
   }

   if (elementIndices.size() < numGlyphs * 6) {
      Q_ASSERT(elementIndices.size() % 6 == 0);
      int j = elementIndices.size() / 6 * 4;
      while (j < numGlyphs * 4) {
         elementIndices.append(j + 0);
         elementIndices.append(j + 0);
         elementIndices.append(j + 1);
         elementIndices.append(j + 2);
         elementIndices.append(j + 3);
         elementIndices.append(j + 3);

         j += 4;
      }

#if defined(QT_OPENGL_DRAWCACHEDGLYPHS_INDEX_ARRAY_VBO)
      if (elementIndicesVBOId == 0) {
         glGenBuffers(1, &elementIndicesVBOId);
      }

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementIndicesVBOId);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementIndices.size() * sizeof(GLushort),
                   elementIndices.constData(), GL_STATIC_DRAW);
#endif
   } else {
#if defined(QT_OPENGL_DRAWCACHEDGLYPHS_INDEX_ARRAY_VBO)
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementIndicesVBOId);
#endif
   }

   setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, (GLfloat *)vertexCoordinates->data());
   setVertexAttributePointer(QT_TEXTURE_COORDS_ATTR, (GLfloat *)textureCoordinates->data());

   if (!snapToPixelGrid) {
      snapToPixelGrid = true;
      matrixDirty = true;
   }

   QBrush pensBrush = q->state()->pen.brush();

   bool srgbFrameBufferEnabled = false;
   if (pensBrush.style() == Qt::SolidPattern &&
         (ctx->d_ptr->extension_flags & QGLExtensions::SRGBFrameBuffer)) {

#if defined(Q_OS_MAC)
      if (glyphType == QFontEngineGlyphCache::Raster_RGBMask)
#elif defined(Q_OS_WIN)
      if (glyphType != QFontEngineGlyphCache::Raster_RGBMask || fontSmoothingApproximately(2.1))
#else
      if (false)
#endif
      {
         QColor c = pensBrush.color();
         qreal red = qt_sRGB_to_linear_RGB(c.redF());
         qreal green = qt_sRGB_to_linear_RGB(c.greenF());
         qreal blue = qt_sRGB_to_linear_RGB(c.blueF());
         c = QColor::fromRgbF(red, green, blue, c.alphaF());
         pensBrush.setColor(c);

         glEnable(FRAMEBUFFER_SRGB_EXT);
         srgbFrameBufferEnabled = true;
      }
   }

   setBrush(pensBrush);

   if (glyphType == QFontEngineGlyphCache::Raster_RGBMask) {
      // Subpixel antialiasing with gamma correction
      QPainter::CompositionMode compMode = q->state()->composition_mode;
      Q_ASSERT(compMode == QPainter::CompositionMode_Source
               || compMode == QPainter::CompositionMode_SourceOver);

      shaderManager->setMaskType(QGLEngineShaderManager::SubPixelMaskPass1);

      if (pensBrush.style() == Qt::SolidPattern) {
         // Solid patterns can get away with only one pass.
         QColor c = pensBrush.color();
         qreal oldOpacity = q->state()->opacity;
         if (compMode == QPainter::CompositionMode_Source) {
            c = qt_premultiplyColor(c, q->state()->opacity);
            q->state()->opacity = 1;
            opacityUniformDirty = true;
         }

         compositionModeDirty = false; // I can handle this myself, thank you very much
         prepareForDraw(false); // Text always causes src pixels to be transparent

         // prepareForDraw() have set the opacity on the current shader, so the opacity state can now be reset.
         if (compMode == QPainter::CompositionMode_Source) {
            q->state()->opacity = oldOpacity;
            opacityUniformDirty = true;
         }

         glEnable(GL_BLEND);
         glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR);
         glBlendColor(c.redF(), c.greenF(), c.blueF(), c.alphaF());
      } else {
         // Other brush styles need two passes.

         qreal oldOpacity = q->state()->opacity;
         if (compMode == QPainter::CompositionMode_Source) {
            q->state()->opacity = 1;
            opacityUniformDirty = true;
            pensBrush = Qt::white;
            setBrush(pensBrush);
         }

         compositionModeDirty = false; // I can handle this myself, thank you very much
         prepareForDraw(false); // Text always causes src pixels to be transparent
         glEnable(GL_BLEND);
         glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

         glActiveTexture(GL_TEXTURE0 + QT_MASK_TEXTURE_UNIT);
         glBindTexture(GL_TEXTURE_2D, cache->texture());
#if !defined(QT_NO_DEBUG) && defined(QT_OPENGL_ES_2)
         QGLFunctions funcs(QGLContext::currentContext());
         bool npotSupported = funcs.hasOpenGLFeature(QGLFunctions::NPOTTextures);
         bool isNpot = !isPowerOfTwo(cache->width())
                       || !isPowerOfTwo(cache->height());
         if (isNpot && !npotSupported) {
            qWarning("GL2 Paint Engine: This system does not support the REPEAT wrap mode for non-power-of-two textures.");
         }
#endif
         updateTextureFilter(GL_TEXTURE_2D, GL_REPEAT, false);

#if defined(QT_OPENGL_DRAWCACHEDGLYPHS_INDEX_ARRAY_VBO)
         glDrawElements(GL_TRIANGLE_STRIP, 6 * numGlyphs, GL_UNSIGNED_SHORT, 0);
#else
         glDrawElements(GL_TRIANGLE_STRIP, 6 * numGlyphs, GL_UNSIGNED_SHORT, elementIndices.data());
#endif

         shaderManager->setMaskType(QGLEngineShaderManager::SubPixelMaskPass2);

         if (compMode == QPainter::CompositionMode_Source) {
            q->state()->opacity = oldOpacity;
            opacityUniformDirty = true;
            pensBrush = q->state()->pen.brush();
            setBrush(pensBrush);
         }

         compositionModeDirty = false;
         prepareForDraw(false); // Text always causes src pixels to be transparent
         glEnable(GL_BLEND);
         glBlendFunc(GL_ONE, GL_ONE);
      }
      compositionModeDirty = true;
   } else {
      // Greyscale/mono glyphs

      shaderManager->setMaskType(QGLEngineShaderManager::PixelMask);
      prepareForDraw(false); // Text always causes src pixels to be transparent
   }

   QGLTextureGlyphCache::FilterMode filterMode = (s->matrix.type() > QTransform::TxTranslate) ?
         QGLTextureGlyphCache::Linear : QGLTextureGlyphCache::Nearest;
   if (lastMaskTextureUsed != cache->texture() || cache->filterMode() != filterMode) {

      glActiveTexture(GL_TEXTURE0 + QT_MASK_TEXTURE_UNIT);
      if (lastMaskTextureUsed != cache->texture()) {
         glBindTexture(GL_TEXTURE_2D, cache->texture());
         lastMaskTextureUsed = cache->texture();
      }

      if (cache->filterMode() != filterMode) {
         if (filterMode == QGLTextureGlyphCache::Linear) {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         } else {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
         }
         cache->setFilterMode(filterMode);
      }
   }

#if defined(QT_OPENGL_DRAWCACHEDGLYPHS_INDEX_ARRAY_VBO)
   glDrawElements(GL_TRIANGLE_STRIP, 6 * numGlyphs, GL_UNSIGNED_SHORT, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#else
   glDrawElements(GL_TRIANGLE_STRIP, 6 * numGlyphs, GL_UNSIGNED_SHORT, elementIndices.data());
#endif

   if (srgbFrameBufferEnabled) {
      glDisable(FRAMEBUFFER_SRGB_EXT);
   }

}

void QGL2PaintEngineEx::drawPixmapFragments(const QPainter::PixmapFragment *fragments, int fragmentCount,
      const QPixmap &pixmap,
      QPainter::PixmapFragmentHints hints)
{
   Q_D(QGL2PaintEngineEx);
   // Use fallback for extended composition modes.
   if (state()->composition_mode > QPainter::CompositionMode_Plus) {
      QPaintEngineEx::drawPixmapFragments(fragments, fragmentCount, pixmap, hints);
      return;
   }

   QSize size = pixmap.size();

   ensureActive();
   int max_texture_size = d->ctx->d_func()->maxTextureSize();
   if (size.width() > max_texture_size || size.height() > max_texture_size) {
      QPixmap scaled = pixmap.scaled(max_texture_size, max_texture_size, Qt::KeepAspectRatio);
      d->drawPixmapFragments(fragments, fragmentCount, scaled, size, hints);
   } else {
      d->drawPixmapFragments(fragments, fragmentCount, pixmap, size, hints);
   }
}

void QGL2PaintEngineEx::drawPixmapFragments(const QRectF *targetRects, const QRectF *sourceRects, int fragmentCount,
      const QPixmap &pixmap,
      QPainter::PixmapFragmentHints hints)
{
   Q_D(QGL2PaintEngineEx);
   // Use fallback for extended composition modes.
   if (state()->composition_mode > QPainter::CompositionMode_Plus) {
      QPaintEngineEx::drawPixmapFragments(targetRects, sourceRects, fragmentCount, pixmap, hints);
      return;
   }

   QSize size = pixmap.size();

   ensureActive();
   int max_texture_size = d->ctx->d_func()->maxTextureSize();
   if (size.width() > max_texture_size || size.height() > max_texture_size) {
      QPixmap scaled = pixmap.scaled(max_texture_size, max_texture_size, Qt::KeepAspectRatio);
      d->drawPixmapFragments(targetRects, sourceRects, fragmentCount, scaled, size, hints);
   } else {
      d->drawPixmapFragments(targetRects, sourceRects, fragmentCount, pixmap, size, hints);
   }
}

void QGL2PaintEngineExPrivate::drawPixmapFragments(const QPainter::PixmapFragment *fragments,
      int fragmentCount, const QPixmap &pixmap,
      const QSize &size, QPainter::PixmapFragmentHints hints)
{
   GLfloat dx = 1.0f / size.width();
   GLfloat dy = 1.0f / size.height();

   vertexCoordinateArray.clear();
   textureCoordinateArray.clear();
   opacityArray.reset();

   if (snapToPixelGrid) {
      snapToPixelGrid = false;
      matrixDirty = true;
   }

   bool allOpaque = true;

   for (int i = 0; i < fragmentCount; ++i) {
      qreal s = 0;
      qreal c = 1;
      if (fragments[i].rotation != 0) {
         s = qFastSin(fragments[i].rotation * Q_PI / 180);
         c = qFastCos(fragments[i].rotation * Q_PI / 180);
      }

      qreal right = 0.5 * fragments[i].scaleX * fragments[i].width;
      qreal bottom = 0.5 * fragments[i].scaleY * fragments[i].height;
      QGLPoint bottomRight(right * c - bottom * s, right * s + bottom * c);
      QGLPoint bottomLeft(-right * c - bottom * s, -right * s + bottom * c);

      vertexCoordinateArray.addVertex(bottomRight.x + fragments[i].x, bottomRight.y + fragments[i].y);
      vertexCoordinateArray.addVertex(-bottomLeft.x + fragments[i].x, -bottomLeft.y + fragments[i].y);
      vertexCoordinateArray.addVertex(-bottomRight.x + fragments[i].x, -bottomRight.y + fragments[i].y);
      vertexCoordinateArray.addVertex(-bottomRight.x + fragments[i].x, -bottomRight.y + fragments[i].y);
      vertexCoordinateArray.addVertex(bottomLeft.x + fragments[i].x, bottomLeft.y + fragments[i].y);
      vertexCoordinateArray.addVertex(bottomRight.x + fragments[i].x, bottomRight.y + fragments[i].y);

      QGLRect src(fragments[i].sourceLeft * dx, fragments[i].sourceTop * dy,
                  (fragments[i].sourceLeft + fragments[i].width) * dx,
                  (fragments[i].sourceTop + fragments[i].height) * dy);

      textureCoordinateArray.addVertex(src.right, src.bottom);
      textureCoordinateArray.addVertex(src.right, src.top);
      textureCoordinateArray.addVertex(src.left, src.top);
      textureCoordinateArray.addVertex(src.left, src.top);
      textureCoordinateArray.addVertex(src.left, src.bottom);
      textureCoordinateArray.addVertex(src.right, src.bottom);

      qreal opacity = fragments[i].opacity * q->state()->opacity;
      opacityArray << opacity << opacity << opacity << opacity << opacity << opacity;
      allOpaque &= (opacity >= 0.99f);
   }

   glActiveTexture(GL_TEXTURE0 + QT_IMAGE_TEXTURE_UNIT);
   QGLTexture *texture = ctx->d_func()->bindTexture(pixmap, GL_TEXTURE_2D, GL_RGBA,
                         QGLContext::InternalBindOption
                         | QGLContext::CanFlipNativePixmapBindOption);

   if (texture->options & QGLContext::InvertedYBindOption) {
      // Flip texture y-coordinate.
      QGLPoint *data = textureCoordinateArray.data();
      for (int i = 0; i < 6 * fragmentCount; ++i) {
         data[i].y = 1 - data[i].y;
      }
   }

   transferMode(allOpaque ? ImageArrayDrawingMode : ImageArrayWithOpacityDrawingMode);

   bool isBitmap = pixmap.isQBitmap();
   bool isOpaque = !isBitmap && (!pixmap.hasAlpha() || (hints & QPainter::OpaqueHint)) && allOpaque;

   updateTextureFilter(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE,
                       q->state()->renderHints & QPainter::SmoothPixmapTransform, texture->id);

   // Setup for texture drawing
   currentBrush = noBrush;
   shaderManager->setSrcPixelType(isBitmap ? QGLEngineShaderManager::PatternSrc
                                  : QGLEngineShaderManager::ImageSrc);
   if (prepareForDraw(isOpaque)) {
      shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::ImageTexture),
            QT_IMAGE_TEXTURE_UNIT);
   }

   if (isBitmap) {
      QColor col = qt_premultiplyColor(q->state()->pen.color(), (GLfloat)q->state()->opacity);
      shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::PatternColor), col);
   }

   glDrawArrays(GL_TRIANGLES, 0, 6 * fragmentCount);
}

void QGL2PaintEngineExPrivate::drawPixmapFragments(const QRectF *targetRects, const QRectF *sourceRects,
      int fragmentCount,
      const QPixmap &pixmap, const QSize &size,
      QPainter::PixmapFragmentHints hints)
{
   GLfloat dx = 1.0f / size.width();
   GLfloat dy = 1.0f / size.height();

   vertexCoordinateArray.clear();
   textureCoordinateArray.clear();

   if (snapToPixelGrid) {
      snapToPixelGrid = false;
      matrixDirty = true;
   }

   for (int i = 0; i < fragmentCount; ++i) {
      vertexCoordinateArray.addVertex(targetRects[i].right(), targetRects[i].bottom());
      vertexCoordinateArray.addVertex(targetRects[i].right(), targetRects[i].top());
      vertexCoordinateArray.addVertex(targetRects[i].left(), targetRects[i].top());
      vertexCoordinateArray.addVertex(targetRects[i].left(), targetRects[i].top());
      vertexCoordinateArray.addVertex(targetRects[i].left(), targetRects[i].bottom());
      vertexCoordinateArray.addVertex(targetRects[i].right(), targetRects[i].bottom());

      QRectF sourceRect = sourceRects ? sourceRects[i] : QRectF(0, 0, size.width(), size.height());

      QGLRect src(sourceRect.left() * dx, sourceRect.top() * dy,
                  sourceRect.right() * dx, sourceRect.bottom() * dy);

      textureCoordinateArray.addVertex(src.right, src.bottom);
      textureCoordinateArray.addVertex(src.right, src.top);
      textureCoordinateArray.addVertex(src.left, src.top);
      textureCoordinateArray.addVertex(src.left, src.top);
      textureCoordinateArray.addVertex(src.left, src.bottom);
      textureCoordinateArray.addVertex(src.right, src.bottom);
   }

   glActiveTexture(GL_TEXTURE0 + QT_IMAGE_TEXTURE_UNIT);
   QGLTexture *texture = ctx->d_func()->bindTexture(pixmap, GL_TEXTURE_2D, GL_RGBA,
                         QGLContext::InternalBindOption
                         | QGLContext::CanFlipNativePixmapBindOption);

   if (texture->options & QGLContext::InvertedYBindOption) {
      // Flip texture y-coordinate.
      QGLPoint *data = textureCoordinateArray.data();
      for (int i = 0; i < 6 * fragmentCount; ++i) {
         data[i].y = 1 - data[i].y;
      }
   }

   transferMode(ImageArrayDrawingMode);

   bool isBitmap = pixmap.isQBitmap();
   bool isOpaque = !isBitmap && (!pixmap.hasAlpha() || (hints & QPainter::OpaqueHint));

   updateTextureFilter(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE,
                       q->state()->renderHints & QPainter::SmoothPixmapTransform, texture->id);

   // Setup for texture drawing
   currentBrush = noBrush;
   shaderManager->setSrcPixelType(isBitmap ? QGLEngineShaderManager::PatternSrc
                                  : QGLEngineShaderManager::ImageSrc);
   if (prepareForDraw(isOpaque)) {
      shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::ImageTexture),
            QT_IMAGE_TEXTURE_UNIT);
   }

   if (isBitmap) {
      QColor col = qt_premultiplyColor(q->state()->pen.color(), (GLfloat)q->state()->opacity);
      shaderManager->currentProgram()->setUniformValue(location(QGLEngineShaderManager::PatternColor), col);
   }

   glDrawArrays(GL_TRIANGLES, 0, 6 * fragmentCount);
}

bool QGL2PaintEngineEx::begin(QPaintDevice *pdev)
{
   Q_D(QGL2PaintEngineEx);

   //     qDebug("QGL2PaintEngineEx::begin()");
   if (pdev->devType() == QInternal::OpenGL) {
      d->device = static_cast<QGLPaintDevice *>(pdev);
   } else {
      d->device = QGLPaintDevice::getDevice(pdev);
   }

   if (!d->device) {
      return false;
   }

   d->ctx = d->device->context();
   d->ctx->d_ptr->active_engine = this;

   const QSize sz = d->device->size();
   d->width = sz.width();
   d->height = sz.height();
   d->mode = BrushDrawingMode;
   d->imageDrawingMode = false;
   d->brushTextureDirty = true;
   d->brushUniformsDirty = true;
   d->matrixUniformDirty = true;
   d->matrixDirty = true;
   d->compositionModeDirty = true;
   d->opacityUniformDirty = true;
   d->needsSync = true;
   d->useSystemClip = !systemClip().isEmpty();
   d->currentBrush = QBrush();

   d->dirtyStencilRegion = QRect(0, 0, d->width, d->height);
   d->stencilClean = true;

   // Calling begin paint should make the correct context current. So, any
   // code which calls into GL or otherwise needs a current context *must*
   // go after beginPaint:
   d->device->beginPaint();

#if !defined(QT_OPENGL_ES_2)
   QGLExtensionMatcher extensions;
   d->hasCompatibilityExtension = extensions.match("GL_ARB_compatibility");

   bool success = qt_resolve_version_2_0_functions(d->ctx)
                  && qt_resolve_buffer_extensions(d->ctx)
                  && (!QGLFramebufferObject::hasOpenGLFramebufferObjects()
                      || qt_resolve_framebufferobject_extensions(d->ctx));
   Q_ASSERT(success);
   Q_UNUSED(success);
#endif

   d->shaderManager = new QGLEngineShaderManager(d->ctx);

   glDisable(GL_STENCIL_TEST);
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_SCISSOR_TEST);

#if !defined(QT_OPENGL_ES_2)
   glDisable(GL_MULTISAMPLE);
#endif

   d->glyphCacheType = QFontEngineGlyphCache::Raster_A8;

#if !defined(QT_OPENGL_ES_2)
#if defined(Q_OS_WIN)
   if (qt_cleartype_enabled
         && (fontSmoothingApproximately(1.0) || fontSmoothingApproximately(2.1)))
#endif
#if defined(Q_OS_MAC)
      if (qt_applefontsmoothing_enabled)
#endif
         d->glyphCacheType = QFontEngineGlyphCache::Raster_RGBMask;
#endif

#if defined(QT_OPENGL_ES_2)
   // OpenGL ES can't switch MSAA off, so if the gl paint device is
   // multisampled, it's always multisampled.
   d->multisamplingAlwaysEnabled = d->device->format().sampleBuffers();
#else
   d->multisamplingAlwaysEnabled = false;
#endif

   return true;
}

bool QGL2PaintEngineEx::end()
{
   Q_D(QGL2PaintEngineEx);
   QGLContext *ctx = d->ctx;

   glUseProgram(0);
   d->transferMode(BrushDrawingMode);
   d->device->endPaint();

#if defined(Q_WS_X11)
   // On some (probably all) drivers, deleting an X pixmap which has been bound to a texture
   // before calling glFinish/swapBuffers renders garbage. Presumably this is because X deletes
   // the pixmap behind the driver's back before it's had a chance to use it. To fix this, we
   // reference all QPixmaps which have been bound to stop them being deleted and only deref
   // them here, after swapBuffers, where they can be safely deleted.
   ctx->d_func()->boundPixmaps.clear();
#endif
   d->ctx->d_ptr->active_engine = 0;

   d->resetGLState();

   delete d->shaderManager;
   d->shaderManager = 0;
   d->currentBrush = QBrush();

#ifdef QT_OPENGL_CACHE_AS_VBOS
   if (!d->unusedVBOSToClean.isEmpty()) {
      glDeleteBuffers(d->unusedVBOSToClean.size(), d->unusedVBOSToClean.constData());
      d->unusedVBOSToClean.clear();
   }
   if (!d->unusedIBOSToClean.isEmpty()) {
      glDeleteBuffers(d->unusedIBOSToClean.size(), d->unusedIBOSToClean.constData());
      d->unusedIBOSToClean.clear();
   }
#endif

   return false;
}

void QGL2PaintEngineEx::ensureActive()
{
   Q_D(QGL2PaintEngineEx);
   QGLContext *ctx = d->ctx;

   if (isActive() && ctx->d_ptr->active_engine != this) {
      ctx->d_ptr->active_engine = this;
      d->needsSync = true;
   }

   d->device->ensureActiveTarget();

   if (d->needsSync) {
      d->transferMode(BrushDrawingMode);
      glViewport(0, 0, d->width, d->height);
      d->needsSync = false;
      d->lastMaskTextureUsed = 0;
      d->shaderManager->setDirty();
      d->ctx->d_func()->syncGlState();
      for (int i = 0; i < 3; ++i) {
         d->vertexAttribPointers[i] = (GLfloat *) - 1;   // Assume the pointers are clobbered
      }
      setState(state());
   }
}

void QGL2PaintEngineExPrivate::updateClipScissorTest()
{
   Q_Q(QGL2PaintEngineEx);
   if (q->state()->clipTestEnabled) {
      glEnable(GL_STENCIL_TEST);
      glStencilFunc(GL_LEQUAL, q->state()->currentClip, ~GL_STENCIL_HIGH_BIT);
   } else {
      glDisable(GL_STENCIL_TEST);
      glStencilFunc(GL_ALWAYS, 0, 0xff);
   }

#ifdef QT_GL_NO_SCISSOR_TEST
   currentScissorBounds = QRect(0, 0, width, height);
#else
   QRect bounds = q->state()->rectangleClip;
   if (!q->state()->clipEnabled) {
      if (useSystemClip) {
         bounds = systemClip.boundingRect();
      } else {
         bounds = QRect(0, 0, width, height);
      }
   } else {
      if (useSystemClip) {
         bounds = bounds.intersected(systemClip.boundingRect());
      } else {
         bounds = bounds.intersected(QRect(0, 0, width, height));
      }
   }

   currentScissorBounds = bounds;

   if (bounds == QRect(0, 0, width, height)) {
      if (ctx->d_func()->workaround_brokenScissor) {
         clearClip(0);
      }
      glDisable(GL_SCISSOR_TEST);
   } else {
      glEnable(GL_SCISSOR_TEST);
      setScissor(bounds);
   }
#endif
}

void QGL2PaintEngineExPrivate::setScissor(const QRect &rect)
{
   const int left = rect.left();
   const int width = rect.width();
   int bottom = height - (rect.top() + rect.height());
   if (device->isFlipped()) {
      bottom = rect.top();
   }
   const int height = rect.height();

   glScissor(left, bottom, width, height);
}

void QGL2PaintEngineEx::clipEnabledChanged()
{
   Q_D(QGL2PaintEngineEx);

   state()->clipChanged = true;

   if (painter()->hasClipping()) {
      d->regenerateClip();
   } else {
      d->systemStateChanged();
   }
}

void QGL2PaintEngineExPrivate::clearClip(uint value)
{
   dirtyStencilRegion -= currentScissorBounds;

   glStencilMask(0xff);
   glClearStencil(value);
   glClear(GL_STENCIL_BUFFER_BIT);
   glStencilMask(0x0);

   q->state()->needsClipBufferClear = false;
}

void QGL2PaintEngineExPrivate::writeClip(const QVectorPath &path, uint value)
{
   transferMode(BrushDrawingMode);

   if (snapToPixelGrid) {
      snapToPixelGrid = false;
      matrixDirty = true;
   }

   if (matrixDirty) {
      updateMatrix();
   }

   stencilClean = false;

   const bool singlePass = !path.hasWindingFill()
                           && (((q->state()->currentClip == maxClip - 1) && q->state()->clipTestEnabled)
                               || q->state()->needsClipBufferClear);
   const uint referenceClipValue = q->state()->needsClipBufferClear ? 1 : q->state()->currentClip;

   if (q->state()->needsClipBufferClear) {
      clearClip(1);
   }

   if (path.isEmpty()) {
      glEnable(GL_STENCIL_TEST);
      glStencilFunc(GL_LEQUAL, value, ~GL_STENCIL_HIGH_BIT);
      return;
   }

   if (q->state()->clipTestEnabled) {
      glStencilFunc(GL_LEQUAL, q->state()->currentClip, ~GL_STENCIL_HIGH_BIT);
   } else {
      glStencilFunc(GL_ALWAYS, 0, 0xff);
   }

   vertexCoordinateArray.clear();
   vertexCoordinateArray.addPath(path, inverseScale, false);

   if (!singlePass) {
      fillStencilWithVertexArray(vertexCoordinateArray, path.hasWindingFill());
   }

   glColorMask(false, false, false, false);
   glEnable(GL_STENCIL_TEST);
   useSimpleShader();

   if (singlePass) {
      // Under these conditions we can set the new stencil value in a single
      // pass, by using the current value and the "new value" as the toggles

      glStencilFunc(GL_LEQUAL, referenceClipValue, ~GL_STENCIL_HIGH_BIT);
      glStencilOp(GL_KEEP, GL_INVERT, GL_INVERT);
      glStencilMask(value ^ referenceClipValue);

      drawVertexArrays(vertexCoordinateArray, GL_TRIANGLE_FAN);
   } else {
      glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
      glStencilMask(0xff);

      if (!q->state()->clipTestEnabled && path.hasWindingFill()) {
         // Pass when any clip bit is set, set high bit
         glStencilFunc(GL_NOTEQUAL, GL_STENCIL_HIGH_BIT, ~GL_STENCIL_HIGH_BIT);
         composite(vertexCoordinateArray.boundingRect());
      }

      // Pass when high bit is set, replace stencil value with new clip value
      glStencilFunc(GL_NOTEQUAL, value, GL_STENCIL_HIGH_BIT);

      composite(vertexCoordinateArray.boundingRect());
   }

   glStencilFunc(GL_LEQUAL, value, ~GL_STENCIL_HIGH_BIT);
   glStencilMask(0);

   glColorMask(true, true, true, true);
}

void QGL2PaintEngineEx::clip(const QVectorPath &path, Qt::ClipOperation op)
{
   //     qDebug("QGL2PaintEngineEx::clip()");
   Q_D(QGL2PaintEngineEx);

   state()->clipChanged = true;

   ensureActive();

   if (op == Qt::ReplaceClip) {
      op = Qt::IntersectClip;
      if (d->hasClipOperations()) {
         d->systemStateChanged();
         state()->canRestoreClip = false;
      }
   }

#ifndef QT_GL_NO_SCISSOR_TEST
   if (!path.isEmpty() && op == Qt::IntersectClip && (path.shape() == QVectorPath::RectangleHint)) {
      const QPointF *const points = reinterpret_cast<const QPointF *>(path.points());
      QRectF rect(points[0], points[2]);

      if (state()->matrix.type() <= QTransform::TxScale
            || (state()->matrix.type() == QTransform::TxRotate
                && qFuzzyIsNull(state()->matrix.m11())
                && qFuzzyIsNull(state()->matrix.m22()))) {
         state()->rectangleClip = state()->rectangleClip.intersected(state()->matrix.mapRect(rect).toRect());
         d->updateClipScissorTest();
         return;
      }
   }
#endif

   const QRect pathRect = state()->matrix.mapRect(path.controlPointRect()).toAlignedRect();

   switch (op) {
      case Qt::NoClip:
         if (d->useSystemClip) {
            state()->clipTestEnabled = true;
            state()->currentClip = 1;
         } else {
            state()->clipTestEnabled = false;
         }
         state()->rectangleClip = QRect(0, 0, d->width, d->height);
         state()->canRestoreClip = false;
         d->updateClipScissorTest();
         break;
      case Qt::IntersectClip:
         state()->rectangleClip = state()->rectangleClip.intersected(pathRect);
         d->updateClipScissorTest();
         d->resetClipIfNeeded();
         ++d->maxClip;
         d->writeClip(path, d->maxClip);
         state()->currentClip = d->maxClip;
         state()->clipTestEnabled = true;
         break;
      case Qt::UniteClip: {
         d->resetClipIfNeeded();
         ++d->maxClip;
         if (state()->rectangleClip.isValid()) {
            QPainterPath path;
            path.addRect(state()->rectangleClip);

            // flush the existing clip rectangle to the depth buffer
            d->writeClip(qtVectorPathForPath(state()->matrix.inverted().map(path)), d->maxClip);
         }

         state()->clipTestEnabled = false;
#ifndef QT_GL_NO_SCISSOR_TEST
         QRect oldRectangleClip = state()->rectangleClip;

         state()->rectangleClip = state()->rectangleClip.united(pathRect);
         d->updateClipScissorTest();

         QRegion extendRegion = QRegion(state()->rectangleClip) - oldRectangleClip;

         if (!extendRegion.isEmpty()) {
            QPainterPath extendPath;
            extendPath.addRegion(extendRegion);

            // first clear the depth buffer in the extended region
            d->writeClip(qtVectorPathForPath(state()->matrix.inverted().map(extendPath)), 0);
         }
#endif
         // now write the clip path
         d->writeClip(path, d->maxClip);
         state()->canRestoreClip = false;
         state()->currentClip = d->maxClip;
         state()->clipTestEnabled = true;
         break;
      }
      default:
         break;
   }
}

void QGL2PaintEngineExPrivate::regenerateClip()
{
   systemStateChanged();
   replayClipOperations();
}

void QGL2PaintEngineExPrivate::systemStateChanged()
{
   Q_Q(QGL2PaintEngineEx);

   q->state()->clipChanged = true;

   if (systemClip.isEmpty()) {
      useSystemClip = false;
   } else {
      if (q->paintDevice()->devType() == QInternal::Widget && currentClipWidget) {
         QWidgetPrivate *widgetPrivate = qt_widget_private(currentClipWidget->window());
         useSystemClip = widgetPrivate->extra && widgetPrivate->extra->inRenderWithPainter;
      } else {
         useSystemClip = true;
      }
   }

   q->state()->clipTestEnabled = false;
   q->state()->needsClipBufferClear = true;

   q->state()->currentClip = 1;
   maxClip = 1;

   q->state()->rectangleClip = useSystemClip ? systemClip.boundingRect() : QRect(0, 0, width, height);
   updateClipScissorTest();

   if (systemClip.rectCount() == 1) {
      if (systemClip.boundingRect() == QRect(0, 0, width, height)) {
         useSystemClip = false;
      }
#ifndef QT_GL_NO_SCISSOR_TEST
      // scissoring takes care of the system clip
      return;
#endif
   }

   if (useSystemClip) {
      clearClip(0);

      QPainterPath path;
      path.addRegion(systemClip);

      q->state()->currentClip = 0;
      writeClip(qtVectorPathForPath(q->state()->matrix.inverted().map(path)), 1);
      q->state()->currentClip = 1;
      q->state()->clipTestEnabled = true;
   }
}

void QGL2PaintEngineEx::setState(QPainterState *new_state)
{
   //     qDebug("QGL2PaintEngineEx::setState()");

   Q_D(QGL2PaintEngineEx);

   QOpenGL2PaintEngineState *s = static_cast<QOpenGL2PaintEngineState *>(new_state);
   QOpenGL2PaintEngineState *old_state = state();

   QPaintEngineEx::setState(s);

   if (s->isNew) {
      // Newly created state object.  The call to setState()
      // will either be followed by a call to begin(), or we are
      // setting the state as part of a save().
      s->isNew = false;
      return;
   }

   // Setting the state as part of a restore().

   if (old_state == s || old_state->renderHintsChanged) {
      renderHintsChanged();
   }

   if (old_state == s || old_state->matrixChanged) {
      d->matrixDirty = true;
   }

   if (old_state == s || old_state->compositionModeChanged) {
      d->compositionModeDirty = true;
   }

   if (old_state == s || old_state->opacityChanged) {
      d->opacityUniformDirty = true;
   }

   if (old_state == s || old_state->clipChanged) {
      if (old_state && old_state != s && old_state->canRestoreClip) {
         d->updateClipScissorTest();
         glDepthFunc(GL_LEQUAL);
      } else {
         d->regenerateClip();
      }
   }
}

QPainterState *QGL2PaintEngineEx::createState(QPainterState *orig) const
{
   if (orig) {
      const_cast<QGL2PaintEngineEx *>(this)->ensureActive();
   }

   QOpenGL2PaintEngineState *s;
   if (!orig) {
      s = new QOpenGL2PaintEngineState();
   } else {
      s = new QOpenGL2PaintEngineState(*static_cast<QOpenGL2PaintEngineState *>(orig));
   }

   s->matrixChanged = false;
   s->compositionModeChanged = false;
   s->opacityChanged = false;
   s->renderHintsChanged = false;
   s->clipChanged = false;

   return s;
}

QOpenGL2PaintEngineState::QOpenGL2PaintEngineState(QOpenGL2PaintEngineState &other)
   : QPainterState(other)
{
   isNew = true;
   needsClipBufferClear = other.needsClipBufferClear;
   clipTestEnabled = other.clipTestEnabled;
   currentClip = other.currentClip;
   canRestoreClip = other.canRestoreClip;
   rectangleClip = other.rectangleClip;
}

QOpenGL2PaintEngineState::QOpenGL2PaintEngineState()
{
   isNew = true;
   needsClipBufferClear = true;
   clipTestEnabled = false;
   canRestoreClip = true;
}

QOpenGL2PaintEngineState::~QOpenGL2PaintEngineState()
{
}

QT_END_NAMESPACE
