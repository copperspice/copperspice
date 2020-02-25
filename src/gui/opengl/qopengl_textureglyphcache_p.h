/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
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

#ifndef QOPENGLTEXTUREGLYPHCACHE_P_H
#define QOPENGLTEXTUREGLYPHCACHE_P_H

#include <qtextureglyphcache_p.h>

#include <qopengl_shaderprogram.h>
#include <qopengl_buffer.h>
#include <qopengl_vertexarrayobject.h>
#include <qopenglfunctions.h>
#include <qopenglcontext_p.h>

// #define QT_GL_TEXTURE_GLYPH_CACHE_DEBUG

class QOpenGL2PaintEngineExPrivate;

class QOpenGLGlyphTexture : public QOpenGLSharedResource
{
public:
    explicit QOpenGLGlyphTexture(QOpenGLContext *ctx)
        : QOpenGLSharedResource(ctx->shareGroup())
        , m_width(0)
        , m_height(0)
    {
        if (!ctx->d_func()->workaround_brokenFBOReadBack)
            QOpenGLFunctions(ctx).glGenFramebuffers(1, &m_fbo);

#ifdef QT_GL_TEXTURE_GLYPH_CACHE_DEBUG
        qDebug(" -> QOpenGLGlyphTexture() %p for context %p.", this, ctx);
#endif
    }

    void freeResource(QOpenGLContext *context) override
    {
        QOpenGLContext *ctx = context;
#ifdef QT_GL_TEXTURE_GLYPH_CACHE_DEBUG
        qDebug("~QOpenGLGlyphTexture() %p for context %p.", this, ctx);
#endif
        if (!ctx->d_func()->workaround_brokenFBOReadBack)
            ctx->functions()->glDeleteFramebuffers(1, &m_fbo);
        if (m_width || m_height)
            ctx->functions()->glDeleteTextures(1, &m_texture);
    }

    void invalidateResource() override
    {
        m_texture = 0;
        m_fbo = 0;
        m_width = 0;
        m_height = 0;
    }

    GLuint m_texture;
    GLuint m_fbo;
    int m_width;
    int m_height;
};

class Q_GUI_EXPORT QOpenGLTextureGlyphCache : public QImageTextureGlyphCache
{
public:
    QOpenGLTextureGlyphCache(QFontEngine::GlyphFormat glyphFormat, const QTransform &matrix);
    ~QOpenGLTextureGlyphCache();

    virtual void createTextureData(int width, int height) override;
    virtual void resizeTextureData(int width, int height) override;
    virtual void fillTexture(const Coord &c, glyph_t glyph, QFixed subPixelPosition) override;
    virtual int glyphPadding() const override;
    virtual int maxTextureWidth() const override;
    virtual int maxTextureHeight() const override;

    inline GLuint texture() const {
        QOpenGLTextureGlyphCache *that = const_cast<QOpenGLTextureGlyphCache *>(this);
        QOpenGLGlyphTexture *glyphTexture = that->m_textureResource;
        return glyphTexture ? glyphTexture->m_texture : 0;
    }

    inline int width() const {
        QOpenGLTextureGlyphCache *that = const_cast<QOpenGLTextureGlyphCache *>(this);
        QOpenGLGlyphTexture *glyphTexture = that->m_textureResource;
        return glyphTexture ? glyphTexture->m_width : 0;
    }
    inline int height() const {
        QOpenGLTextureGlyphCache *that = const_cast<QOpenGLTextureGlyphCache *>(this);
        QOpenGLGlyphTexture *glyphTexture = that->m_textureResource;
        return glyphTexture ? glyphTexture->m_height : 0;
    }

    inline void setPaintEnginePrivate(QOpenGL2PaintEngineExPrivate *p) { pex = p; }

    inline const QOpenGLContextGroup *contextGroup() const { return m_textureResource ? m_textureResource->group() : 0; }

    inline int serialNumber() const { return m_serialNumber; }

    enum FilterMode {
        Nearest,
        Linear
    };
    FilterMode filterMode() const { return m_filterMode; }
    void setFilterMode(FilterMode m) { m_filterMode = m; }

    void clear();

private:
    void setupVertexAttribs();

    QOpenGLGlyphTexture *m_textureResource;

    QOpenGL2PaintEngineExPrivate *pex;
    QOpenGLShaderProgram *m_blitProgram;
    FilterMode m_filterMode;

    GLfloat m_vertexCoordinateArray[8];
    GLfloat m_textureCoordinateArray[8];

    int m_serialNumber;

    QOpenGLBuffer m_buffer;
    QOpenGLVertexArrayObject m_vao;
};

#endif

