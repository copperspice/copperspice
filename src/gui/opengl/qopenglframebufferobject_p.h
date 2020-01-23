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

#ifndef QOPENGLFRAMEBUFFEROBJECT_P_H
#define QOPENGLFRAMEBUFFEROBJECT_P_H

#include <qopenglframebufferobject.h>
#include <qopenglcontext_p.h>
#include <qopenglextensions_p.h>

class QOpenGLFramebufferObjectFormatPrivate
{
public:
    QOpenGLFramebufferObjectFormatPrivate()
        : ref(1),
          samples(0),
          attachment(QOpenGLFramebufferObject::NoAttachment),
          target(GL_TEXTURE_2D),
          mipmap(false)
    {
#ifndef QT_OPENGL_ES_2
        // There is nothing that says QOpenGLFramebufferObjectFormat needs a current
        // context, so we need a fallback just to be safe, even though in pratice there
        // will usually be a context current.
        QOpenGLContext *ctx = QOpenGLContext::currentContext();
        const bool isES = ctx ? ctx->isOpenGLES() : QOpenGLContext::openGLModuleType() != QOpenGLContext::LibGL;
        internal_format = isES ? GL_RGBA : GL_RGBA8;
#else
        internal_format = GL_RGBA;
#endif
    }
    QOpenGLFramebufferObjectFormatPrivate
            (const QOpenGLFramebufferObjectFormatPrivate *other)
        : ref(1),
          samples(other->samples),
          attachment(other->attachment),
          target(other->target),
          internal_format(other->internal_format),
          mipmap(other->mipmap)
    {
    }
    bool equals(const QOpenGLFramebufferObjectFormatPrivate *other)
    {
        return samples == other->samples &&
               attachment == other->attachment &&
               target == other->target &&
               internal_format == other->internal_format &&
               mipmap == other->mipmap;
    }

    QAtomicInt ref;
    int samples;
    QOpenGLFramebufferObject::Attachment attachment;
    GLenum target;
    GLenum internal_format;
    uint mipmap : 1;
};

class QOpenGLFramebufferObjectPrivate
{
public:
    QOpenGLFramebufferObjectPrivate() : fbo_guard(0), depth_buffer_guard(0)
                                  , stencil_buffer_guard(0)
                                  , valid(false) {}
    ~QOpenGLFramebufferObjectPrivate() {}

    void init(QOpenGLFramebufferObject *q, const QSize &size,
              QOpenGLFramebufferObject::Attachment attachment,
              GLenum texture_target, GLenum internal_format,
              GLint samples = 0, bool mipmap = false);
    void initTexture(int idx);
    void initColorBuffer(int idx, GLint *samples);
    void initDepthStencilAttachments(QOpenGLContext *ctx, QOpenGLFramebufferObject::Attachment attachment);

    bool checkFramebufferStatus(QOpenGLContext *ctx) const;
    QOpenGLSharedResourceGuard *fbo_guard;
    QOpenGLSharedResourceGuard *depth_buffer_guard;
    QOpenGLSharedResourceGuard *stencil_buffer_guard;
    GLenum target;
    QSize dsSize;
    QOpenGLFramebufferObjectFormat format;
    int requestedSamples;
    uint valid : 1;
    QOpenGLFramebufferObject::Attachment fbo_attachment;
    QOpenGLExtensions funcs;

    struct ColorAttachment {
        ColorAttachment() : internalFormat(0), guard(0) { }
        ColorAttachment(const QSize &size, GLenum internalFormat)
            : size(size), internalFormat(internalFormat), guard(0) { }
        QSize size;
        GLenum internalFormat;
        QOpenGLSharedResourceGuard *guard;
    };
    QVector<ColorAttachment> colorAttachments;

    inline GLuint fbo() const { return fbo_guard ? fbo_guard->id() : 0; }
};


#endif
