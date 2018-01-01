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

#ifndef QTEXTUREGLYPHCACHE_GL_P_H
#define QTEXTUREGLYPHCACHE_GL_P_H

#include <qtextureglyphcache_p.h>
#include <qgl_p.h>
#include <qglshaderprogram.h>
#include <qglframebufferobject.h>

// #define QT_GL_TEXTURE_GLYPH_CACHE_DEBUG

QT_BEGIN_NAMESPACE

class QGL2PaintEngineExPrivate;

struct QGLGlyphTexture {
   QGLGlyphTexture(const QGLContext *ctx)
      : m_fbo(0)
      , m_width(0)
      , m_height(0) {
      if (ctx && QGLFramebufferObject::hasOpenGLFramebufferObjects() && !ctx->d_ptr->workaround_brokenFBOReadBack) {
         glGenFramebuffers(1, &m_fbo);
      }

#ifdef QT_GL_TEXTURE_GLYPH_CACHE_DEBUG
      qDebug(" -> QGLGlyphTexture() %p for context %p.", this, ctx);
#endif
   }

   ~QGLGlyphTexture() {
      const QGLContext *ctx = QGLContext::currentContext();
#ifdef QT_GL_TEXTURE_GLYPH_CACHE_DEBUG
      qDebug("~QGLGlyphTexture() %p for context %p.", this, ctx);
#endif
      // At this point, the context group is made current, so it's safe to
      // release resources without a makeCurrent() call
      if (ctx) {
         if (m_fbo) {
            glDeleteFramebuffers(1, &m_fbo);
         }
         if (m_width || m_height) {
            glDeleteTextures(1, &m_texture);
         }
      }
   }

   GLuint m_texture;
   GLuint m_fbo;
   int m_width;
   int m_height;
};

class Q_OPENGL_EXPORT QGLTextureGlyphCache : public QImageTextureGlyphCache, public QGLContextGroupResourceBase
{
 public:
   QGLTextureGlyphCache(const QGLContext *context, QFontEngineGlyphCache::Type type, const QTransform &matrix);
   ~QGLTextureGlyphCache();

   void createTextureData(int width, int height) override;
   void resizeTextureData(int width, int height) override;
   void fillTexture(const Coord &c, glyph_t glyph, QFixed subPixelPosition) override;
   int glyphPadding() const override;
   int maxTextureWidth() const override;
   int maxTextureHeight() const override;

   GLuint texture() const {
      QGLTextureGlyphCache *that = const_cast<QGLTextureGlyphCache *>(this);
      QGLGlyphTexture *glyphTexture = that->m_textureResource.value(ctx);
      return glyphTexture ? glyphTexture->m_texture : 0;
   }

   int width() const {
      QGLTextureGlyphCache *that = const_cast<QGLTextureGlyphCache *>(this);
      QGLGlyphTexture *glyphTexture = that->m_textureResource.value(ctx);
      return glyphTexture ? glyphTexture->m_width : 0;
   }

   int height() const {
      QGLTextureGlyphCache *that = const_cast<QGLTextureGlyphCache *>(this);
      QGLGlyphTexture *glyphTexture = that->m_textureResource.value(ctx);
      return glyphTexture ? glyphTexture->m_height : 0;
   }

   void setPaintEnginePrivate(QGL2PaintEngineExPrivate *p) {
      pex = p;
   }

   void setContext(const QGLContext *context) ;
   const QGLContext *context() const {
      return ctx;
   }

   int serialNumber() const {
      return m_serialNumber;
   }

   enum FilterMode {
      Nearest,
      Linear
   };

   FilterMode filterMode() const {
      return m_filterMode;
   }
   void setFilterMode(FilterMode m) {
      m_filterMode = m;
   }

   void clear();

   void contextDeleted(const QGLContext *context) override {
      if (ctx == context) {
         ctx = 0;
      }
   }

   void freeResource(void *) override{
      ctx = 0;
   }

 private:
   QGLContextGroupResource<QGLGlyphTexture> m_textureResource;

   const QGLContext *ctx;
   QGL2PaintEngineExPrivate *pex;
   QGLShaderProgram *m_blitProgram;
   FilterMode m_filterMode;

   GLfloat m_vertexCoordinateArray[8];
   GLfloat m_textureCoordinateArray[8];

   int m_serialNumber;
};

QT_END_NAMESPACE

#endif

