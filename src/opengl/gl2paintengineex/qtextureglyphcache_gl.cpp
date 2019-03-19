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

#include "qtextureglyphcache_gl_p.h"
#include "qpaintengineex_opengl2_p.h"
#include "qglengineshadersource_p.h"

#if defined QT_OPENGL_ES_2 && ! defined(QT_NO_EGL)
#include "qeglcontext_p.h"
#endif

QT_BEGIN_NAMESPACE

#ifdef Q_OS_WIN
extern Q_GUI_EXPORT bool qt_cleartype_enabled;
#endif

QAtomicInt qgltextureglyphcache_serial_number = 1;

QGLTextureGlyphCache::QGLTextureGlyphCache(const QGLContext *context, QFontEngineGlyphCache::Type type,
      const QTransform &matrix)
   : QImageTextureGlyphCache(type, matrix), QGLContextGroupResourceBase(), ctx(0)
   , pex(0), m_blitProgram(0), m_filterMode(Nearest)
   , m_serialNumber(qgltextureglyphcache_serial_number.fetchAndAddRelaxed(1))
{
#ifdef QT_GL_TEXTURE_GLYPH_CACHE_DEBUG
   qDebug(" -> QGLTextureGlyphCache() %p for context %p.", this, ctx);
#endif
   setContext(context);

   m_vertexCoordinateArray[0] = -1.0f;
   m_vertexCoordinateArray[1] = -1.0f;
   m_vertexCoordinateArray[2] =  1.0f;
   m_vertexCoordinateArray[3] = -1.0f;
   m_vertexCoordinateArray[4] =  1.0f;
   m_vertexCoordinateArray[5] =  1.0f;
   m_vertexCoordinateArray[6] = -1.0f;
   m_vertexCoordinateArray[7] =  1.0f;

   m_textureCoordinateArray[0] = 0.0f;
   m_textureCoordinateArray[1] = 0.0f;
   m_textureCoordinateArray[2] = 1.0f;
   m_textureCoordinateArray[3] = 0.0f;
   m_textureCoordinateArray[4] = 1.0f;
   m_textureCoordinateArray[5] = 1.0f;
   m_textureCoordinateArray[6] = 0.0f;
   m_textureCoordinateArray[7] = 1.0f;
}

QGLTextureGlyphCache::~QGLTextureGlyphCache()
{
#ifdef QT_GL_TEXTURE_GLYPH_CACHE_DEBUG
   qDebug(" -> ~QGLTextureGlyphCache() %p.", this);
#endif
   delete m_blitProgram;
}

void QGLTextureGlyphCache::setContext(const QGLContext *context)
{
   ctx = context;
   m_h = 0;
}

void QGLTextureGlyphCache::createTextureData(int width, int height)
{
   if (ctx == 0) {
      qWarning("QGLTextureGlyphCache::createTextureData: Called with no context");
      return;
   }

   // create in QImageTextureGlyphCache baseclass is meant to be called
   // only to create the initial image and does not preserve the content,
   // so we don't call when this function is called from resize.
   if ((!QGLFramebufferObject::hasOpenGLFramebufferObjects() || ctx->d_ptr->workaround_brokenFBOReadBack) &&
         image().isNull()) {
      QImageTextureGlyphCache::createTextureData(width, height);
   }

   // Make the lower glyph texture size 16 x 16.
   if (width < 16) {
      width = 16;
   }
   if (height < 16) {
      height = 16;
   }

   QGLGlyphTexture *glyphTexture = m_textureResource.value(ctx);
   glGenTextures(1, &glyphTexture->m_texture);
   glBindTexture(GL_TEXTURE_2D, glyphTexture->m_texture);

   glyphTexture->m_width = width;
   glyphTexture->m_height = height;

   if (m_type == QFontEngineGlyphCache::Raster_RGBMask) {
      QVarLengthArray<uchar> data(width * height * 4);
      for (int i = 0; i < data.size(); ++i) {
         data[i] = 0;
      }
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
   } else {
      QVarLengthArray<uchar> data(width * height);
      for (int i = 0; i < data.size(); ++i) {
         data[i] = 0;
      }
      glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &data[0]);
   }

   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   m_filterMode = Nearest;
}

void QGLTextureGlyphCache::resizeTextureData(int width, int height)
{
   if (ctx == 0) {
      qWarning("QGLTextureGlyphCache::resizeTextureData: Called with no context");
      return;
   }
   QGLGlyphTexture *glyphTexture = m_textureResource.value(ctx);

   int oldWidth = glyphTexture->m_width;
   int oldHeight = glyphTexture->m_height;

   // Make the lower glyph texture size 16 x 16.
   if (width < 16) {
      width = 16;
   }
   if (height < 16) {
      height = 16;
   }

   GLuint oldTexture = glyphTexture->m_texture;
   createTextureData(width, height);

   if (!QGLFramebufferObject::hasOpenGLFramebufferObjects() || ctx->d_ptr->workaround_brokenFBOReadBack) {
      QImageTextureGlyphCache::resizeTextureData(width, height);
      Q_ASSERT(image().depth() == 8);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, oldHeight, GL_ALPHA, GL_UNSIGNED_BYTE, image().constBits());
      glDeleteTextures(1, &oldTexture);
      return;
   }

   // ### the QTextureGlyphCache API needs to be reworked to allow
   // ### resizeTextureData to fail

   glBindFramebuffer(GL_FRAMEBUFFER_EXT, glyphTexture->m_fbo);

   GLuint tmp_texture;
   glGenTextures(1, &tmp_texture);
   glBindTexture(GL_TEXTURE_2D, tmp_texture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, oldWidth, oldHeight, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   m_filterMode = Nearest;
   glBindTexture(GL_TEXTURE_2D, 0);
   glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                          GL_TEXTURE_2D, tmp_texture, 0);

   glActiveTexture(GL_TEXTURE0 + QT_IMAGE_TEXTURE_UNIT);
   glBindTexture(GL_TEXTURE_2D, oldTexture);

   if (pex != 0) {
      pex->transferMode(BrushDrawingMode);
   }

   glDisable(GL_STENCIL_TEST);
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_SCISSOR_TEST);
   glDisable(GL_BLEND);

   glViewport(0, 0, oldWidth, oldHeight);

   QGLShaderProgram *blitProgram = 0;
   if (pex == 0) {
      if (m_blitProgram == 0) {
         m_blitProgram = new QGLShaderProgram(ctx);

         {
            QString source;
            source.append(QLatin1String(qglslMainWithTexCoordsVertexShader));
            source.append(QLatin1String(qglslUntransformedPositionVertexShader));

            QGLShader *vertexShader = new QGLShader(QGLShader::Vertex, m_blitProgram);
            vertexShader->compileSourceCode(source);

            m_blitProgram->addShader(vertexShader);
         }

         {
            QString source;
            source.append(QLatin1String(qglslMainFragmentShader));
            source.append(QLatin1String(qglslImageSrcFragmentShader));

            QGLShader *fragmentShader = new QGLShader(QGLShader::Fragment, m_blitProgram);
            fragmentShader->compileSourceCode(source);

            m_blitProgram->addShader(fragmentShader);
         }

         m_blitProgram->bindAttributeLocation("vertexCoordsArray", QT_VERTEX_COORDS_ATTR);
         m_blitProgram->bindAttributeLocation("textureCoordArray", QT_TEXTURE_COORDS_ATTR);

         m_blitProgram->link();
      }

      glVertexAttribPointer(QT_VERTEX_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, m_vertexCoordinateArray);
      glVertexAttribPointer(QT_TEXTURE_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, m_textureCoordinateArray);

      m_blitProgram->bind();
      m_blitProgram->enableAttributeArray(int(QT_VERTEX_COORDS_ATTR));
      m_blitProgram->enableAttributeArray(int(QT_TEXTURE_COORDS_ATTR));
      m_blitProgram->disableAttributeArray(int(QT_OPACITY_ATTR));

      blitProgram = m_blitProgram;

   } else {
      pex->setVertexAttributePointer(QT_VERTEX_COORDS_ATTR, m_vertexCoordinateArray);
      pex->setVertexAttributePointer(QT_TEXTURE_COORDS_ATTR, m_textureCoordinateArray);

      pex->shaderManager->useBlitProgram();
      blitProgram = pex->shaderManager->blitProgram();
   }

   blitProgram->setUniformValue("imageTexture", QT_IMAGE_TEXTURE_UNIT);

   glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

   glBindTexture(GL_TEXTURE_2D, glyphTexture->m_texture);

   glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, oldWidth, oldHeight);

   glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                             GL_RENDERBUFFER_EXT, 0);
   glDeleteTextures(1, &tmp_texture);
   glDeleteTextures(1, &oldTexture);

   glBindFramebuffer(GL_FRAMEBUFFER_EXT, ctx->d_ptr->current_fbo);

   if (pex != 0) {
      glViewport(0, 0, pex->width, pex->height);
      pex->updateClipScissorTest();
   }
}

void QGLTextureGlyphCache::fillTexture(const Coord &c, glyph_t glyph, QFixed subPixelPosition)
{
   if (ctx == 0) {
      qWarning("QGLTextureGlyphCache::fillTexture: Called with no context");
      return;
   }

   QGLGlyphTexture *glyphTexture = m_textureResource.value(ctx);
   if (!QGLFramebufferObject::hasOpenGLFramebufferObjects() || ctx->d_ptr->workaround_brokenFBOReadBack) {
      QImageTextureGlyphCache::fillTexture(c, glyph, subPixelPosition);

      glBindTexture(GL_TEXTURE_2D, glyphTexture->m_texture);
      const QImage &texture = image();
      const uchar *bits = texture.constBits();
      bits += c.y * texture.bytesPerLine() + c.x;
      for (int i = 0; i < c.h; ++i) {
         glTexSubImage2D(GL_TEXTURE_2D, 0, c.x, c.y + i, c.w, 1, GL_ALPHA, GL_UNSIGNED_BYTE, bits);
         bits += texture.bytesPerLine();
      }
      return;
   }

   QImage mask = textureMapForGlyph(glyph, subPixelPosition);
   const int maskWidth = mask.width();
   const int maskHeight = mask.height();

   if (mask.format() == QImage::Format_Mono) {
      mask = mask.convertToFormat(QImage::Format_Indexed8);
      for (int y = 0; y < maskHeight; ++y) {
         uchar *src = (uchar *) mask.scanLine(y);
         for (int x = 0; x < maskWidth; ++x) {
            src[x] = -src[x];   // convert 0 and 1 into 0 and 255
         }
      }
   } else if (mask.format() == QImage::Format_RGB32) {
      // Make the alpha component equal to the average of the RGB values.
      // This is needed when drawing sub-pixel antialiased text on translucent targets.
      for (int y = 0; y < maskHeight; ++y) {
         quint32 *src = (quint32 *) mask.scanLine(y);
         for (int x = 0; x < maskWidth; ++x) {
            uchar r = src[x] >> 16;
            uchar g = src[x] >> 8;
            uchar b = src[x];
            quint32 avg = (quint32(r) + quint32(g) + quint32(b) + 1) / 3; // "+1" for rounding.
            src[x] = (src[x] & 0x00ffffff) | (avg << 24);
         }
      }
   }

   glBindTexture(GL_TEXTURE_2D, glyphTexture->m_texture);
   if (mask.format() == QImage::Format_RGB32) {
      glTexSubImage2D(GL_TEXTURE_2D, 0, c.x, c.y, maskWidth, maskHeight, GL_BGRA, GL_UNSIGNED_BYTE, mask.bits());
   } else {
      // glTexSubImage2D() might cause some garbage to appear in the texture if the mask width is
      // not a multiple of four bytes. The bug appeared on a computer with 32-bit Windows Vista
      // and nVidia GeForce 8500GT. GL_UNPACK_ALIGNMENT is set to four bytes, 'mask' has a
      // multiple of four bytes per line, and most of the glyph shows up correctly in the
      // texture, which makes me think that this is a driver bug.
      // One workaround is to make sure the mask width is a multiple of four bytes, for instance
      // by converting it to a format with four bytes per pixel. Another is to copy one line at a
      // time.

      if (!ctx->d_ptr->workaround_brokenAlphaTexSubImage_init) {
          // do not know which driver versions exhibit this bug, so be conservative for now
         const QString &vendorStr = cs_glGetString(GL_VENDOR);

         ctx->d_ptr->workaround_brokenAlphaTexSubImage = vendorStr.indexOf("NVIDIA") >= 0;
         ctx->d_ptr->workaround_brokenAlphaTexSubImage_init = true;
      }

      if (ctx->d_ptr->workaround_brokenAlphaTexSubImage) {
         for (int i = 0; i < maskHeight; ++i) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, c.x, c.y + i, maskWidth, 1, GL_ALPHA, GL_UNSIGNED_BYTE, mask.scanLine(i));
         }

      } else {
         glTexSubImage2D(GL_TEXTURE_2D, 0, c.x, c.y, maskWidth, maskHeight, GL_ALPHA, GL_UNSIGNED_BYTE, mask.bits());
      }
   }
}

int QGLTextureGlyphCache::glyphPadding() const
{
   return 1;
}

int QGLTextureGlyphCache::maxTextureWidth() const
{
   if (ctx == 0) {
      return QImageTextureGlyphCache::maxTextureWidth();
   } else {
      return ctx->d_ptr->maxTextureSize();
   }
}

int QGLTextureGlyphCache::maxTextureHeight() const
{
   if (ctx == 0) {
      return QImageTextureGlyphCache::maxTextureHeight();
   }

   if (ctx->d_ptr->workaround_brokenTexSubImage) {
      return qMin(1024, ctx->d_ptr->maxTextureSize());
   } else {
      return ctx->d_ptr->maxTextureSize();
   }
}

void QGLTextureGlyphCache::clear()
{
   if (ctx != 0) {
      m_textureResource.cleanup(ctx);

      m_w = 0;
      m_h = 0;
      m_cx = 0;
      m_cy = 0;
      m_currentRowHeight = 0;
      coords.clear();
   }
}

QT_END_NAMESPACE
