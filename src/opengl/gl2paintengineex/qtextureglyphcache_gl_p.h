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

#ifndef QTEXTUREGLYPHCACHE_GL_P_H
#define QTEXTUREGLYPHCACHE_GL_P_H

#include <qtextureglyphcache_p.h>

#include <qglframebufferobject.h>
#include <qglshaderprogram.h>
#include <qopenglfunctions.h>

#include <qgl_p.h>

class QGL2PaintEngineExPrivate;

struct QGLGlyphTexture : public QOpenGLSharedResource
{
   QGLGlyphTexture(const QGLContext *ctx)
        : QOpenGLSharedResource(ctx->contextHandle()->shareGroup())
        , m_fbo(0), m_width(0), m_height(0) {
        if (ctx && QGLFramebufferObject::hasOpenGLFramebufferObjects() && ! ctx->d_ptr->workaround_brokenFBOReadBack) {
            ctx->contextHandle()->functions()->glGenFramebuffers(1, &m_fbo);
        }

#if defined(CS_SHOW_DEBUG_OPENGL)
      qDebug(" -> QGLGlyphTexture() %p for context %p.", this, ctx);
#endif
   }

    void freeResource(QOpenGLContext *context) override
    {
        const QGLContext *ctx = QGLContext::fromOpenGLContext(context);

#if defined(CS_SHOW_DEBUG_OPENGL)
        qDebug("~QGLGlyphTexture() %p for context %p.", this, ctx);
#else
        (void) ctx;
#endif

        if (ctx && m_fbo)
            ctx->contextHandle()->functions()->glDeleteFramebuffers(1, &m_fbo);
        if (m_width || m_height)
            ctx->contextHandle()->functions()->glDeleteTextures(1, &m_texture);
    }

    void invalidateResource() override {
        m_texture = 0;
        m_fbo     = 0;
        m_width   = 0;
        m_height  = 0;
    }

   GLuint m_texture;
   GLuint m_fbo;
   int m_width;
   int m_height;
};

class Q_OPENGL_EXPORT QGLTextureGlyphCache : public QImageTextureGlyphCache
{
 public:
   QGLTextureGlyphCache(QFontEngine::GlyphFormat format, const QTransform &matrix);
   ~QGLTextureGlyphCache();

   void createTextureData(int width, int height) override;
   void resizeTextureData(int width, int height) override;
   void fillTexture(const Coord &c, glyph_t glyph, QFixed subPixelPosition) override;
   int glyphPadding() const override;
   int maxTextureWidth() const override;
   int maxTextureHeight() const override;

   GLuint texture() const {
      QGLTextureGlyphCache *that = const_cast<QGLTextureGlyphCache *>(this);
      QGLGlyphTexture *glyphTexture = that->m_textureResource;
      return glyphTexture ? glyphTexture->m_texture : 0;
   }

   int width() const {
      QGLTextureGlyphCache *that = const_cast<QGLTextureGlyphCache *>(this);
      QGLGlyphTexture *glyphTexture = that->m_textureResource;
      return glyphTexture ? glyphTexture->m_width : 0;
   }

   int height() const {
      QGLTextureGlyphCache *that = const_cast<QGLTextureGlyphCache *>(this);
      QGLGlyphTexture *glyphTexture = that->m_textureResource;
      return glyphTexture ? glyphTexture->m_height : 0;
   }

   void setPaintEnginePrivate(QGL2PaintEngineExPrivate *p) {
      pex = p;
   }

   inline const QOpenGLContextGroup *contextGroup() const {
      return m_textureResource ? m_textureResource->group() : nullptr;
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

 private:
   QGLGlyphTexture *m_textureResource;

   QGL2PaintEngineExPrivate *pex;
   QGLShaderProgram *m_blitProgram;
   FilterMode m_filterMode;

   GLfloat m_vertexCoordinateArray[8];
   GLfloat m_textureCoordinateArray[8];

   int m_serialNumber;
};

#endif

