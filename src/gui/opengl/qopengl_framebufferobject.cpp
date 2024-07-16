/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#include <qopengl_framebufferobject.h>
#include <qopengl_framebufferobject_p.h>

#include <qdebug.h>
#include <qwindow.h>
#include <qlibrary.h>
#include <qimage.h>
#include <qbytearray.h>

#include <qopengl_p.h>
#include <qopenglcontext_p.h>
#include <qopengl_extensions_p.h>
#include <qfont_p.h>

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
#define QT_RESET_GLERROR()                                \
{                                                         \
    while (QOpenGLContext::currentContext()->functions()->glGetError() != GL_NO_ERROR) {} \
}

#define QT_CHECK_GLERROR()                                \
{                                                         \
    GLenum err = QOpenGLContext::currentContext()->functions()->glGetError();  \
    if (err != GL_NO_ERROR) {                             \
        qDebug("[%s line %d] OpenGL Error: %d", __FILE__, __LINE__, (int)err); \
    }                                                     \
}
#else
#define QT_RESET_GLERROR() {}
#define QT_CHECK_GLERROR() {}
#endif

#ifndef GL_MAX_SAMPLES
#define GL_MAX_SAMPLES 0x8D57
#endif

#ifndef GL_RENDERBUFFER_SAMPLES
#define GL_RENDERBUFFER_SAMPLES 0x8CAB
#endif

#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8 0x88F0
#endif

#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24 0x81A6
#endif

#ifndef GL_DEPTH_COMPONENT24_OES
#define GL_DEPTH_COMPONENT24_OES 0x81A6
#endif

#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER 0x8CA8
#endif

#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#endif

#ifndef GL_RGB8
#define GL_RGB8                           0x8051
#endif

#ifndef GL_RGB10
#define GL_RGB10                          0x8052
#endif

#ifndef GL_RGBA8
#define GL_RGBA8                          0x8058
#endif

#ifndef GL_RGB10_A2
#define GL_RGB10_A2                       0x8059
#endif

#ifndef GL_BGRA
#define GL_BGRA                           0x80E1
#endif

#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#endif

#ifndef GL_UNSIGNED_INT_2_10_10_10_REV
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#endif

// internal
void QOpenGLFramebufferObjectFormat::detach()
{
    if (d->ref.load() != 1) {
        QOpenGLFramebufferObjectFormatPrivate *newd = new QOpenGLFramebufferObjectFormatPrivate(d);

        if (! d->ref.deref())
            delete d;

        d = newd;
    }
}

QOpenGLFramebufferObjectFormat::QOpenGLFramebufferObjectFormat()
{
    d = new QOpenGLFramebufferObjectFormatPrivate;
}

QOpenGLFramebufferObjectFormat::QOpenGLFramebufferObjectFormat(const QOpenGLFramebufferObjectFormat &other)
{
    d = other.d;
    d->ref.ref();
}

QOpenGLFramebufferObjectFormat &QOpenGLFramebufferObjectFormat::operator=(const QOpenGLFramebufferObjectFormat &other)
{
   if (d != other.d) {
      other.d->ref.ref();

      if (! d->ref.deref()) {
         delete d;
      }

      d = other.d;
   }

   return *this;
}

QOpenGLFramebufferObjectFormat::~QOpenGLFramebufferObjectFormat()
{
   if (! d->ref.deref()) {
      delete d;
   }
}

void QOpenGLFramebufferObjectFormat::setSamples(int samples)
{
    detach();
    d->samples = samples;
}

int QOpenGLFramebufferObjectFormat::samples() const
{
    return d->samples;
}

void QOpenGLFramebufferObjectFormat::setMipmap(bool enabled)
{
    detach();
    d->mipmap = enabled;
}

bool QOpenGLFramebufferObjectFormat::mipmap() const
{
    return d->mipmap;
}

void QOpenGLFramebufferObjectFormat::setAttachment(QOpenGLFramebufferObject::Attachment attachment)
{
    detach();
    d->attachment = attachment;
}

QOpenGLFramebufferObject::Attachment QOpenGLFramebufferObjectFormat::attachment() const
{
    return d->attachment;
}

void QOpenGLFramebufferObjectFormat::setTextureTarget(GLenum target)
{
    detach();
    d->target = target;
}

GLenum QOpenGLFramebufferObjectFormat::textureTarget() const
{
    return d->target;
}

void QOpenGLFramebufferObjectFormat::setInternalTextureFormat(GLenum internalTextureFormat)
{
    detach();
    d->internal_format = internalTextureFormat;
}

GLenum QOpenGLFramebufferObjectFormat::internalTextureFormat() const
{
    return d->internal_format;
}

bool QOpenGLFramebufferObjectFormat::operator==(const QOpenGLFramebufferObjectFormat& other) const
{
    if (d == other.d) {
       return true;
    } else {
       return d->equals(other.d);
    }
}

bool QOpenGLFramebufferObjectFormat::operator!=(const QOpenGLFramebufferObjectFormat& other) const
{
    return !(*this == other);
}

bool QOpenGLFramebufferObjectPrivate::checkFramebufferStatus(QOpenGLContext *ctx) const
{
    if (! ctx) {
       return false;   // Context no longer exists.
    }

    GLenum status = ctx->functions()->glCheckFramebufferStatus(GL_FRAMEBUFFER);

    switch(status) {

    case GL_NO_ERROR:
    case GL_FRAMEBUFFER_COMPLETE:
        return true;

    case GL_FRAMEBUFFER_UNSUPPORTED:
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
        qDebug("QOpenGLFramebufferObject::checkFramebufferStatus() Unsupported framebuffer format");
#endif

        break;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
        qDebug("QOpenGLFramebufferObject::checkFramebufferStatus() Framebuffer incomplete attachment");
#endif
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
        qDebug("QOpenGLFramebufferObject::checkFramebufferStatus() Framebuffer incomplete, missing attachment");
#endif
        break;

#ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT
    case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT:
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
        qDebug("QOpenGLFramebufferObject::checkFramebufferStatus() Framebuffer incomplete, duplicate attachment");
#endif

        break;
#endif

#ifdef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
        qDebug("QOpenGLFramebufferObject::checkFramebufferStatus() Framebuffer incomplete, attached images must have same dimensions");
#endif

        break;
#endif

#ifdef GL_FRAMEBUFFER_INCOMPLETE_FORMATS
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
        qDebug("QOpenGLFramebufferObject::checkFramebufferStatus() Framebuffer incomplete, attached images must have same format");
#endif
        break;
#endif

#ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
        qDebug("QOpenGLFramebufferObject::checkFramebufferStatus() Framebuffer incomplete, missing draw buffer");
#endif

        break;
#endif

#ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
        qDebug("QOpenGLFramebufferObject::checkFramebufferStatus() Framebuffer incomplete, missing read buffer");
#endif

        break;
#endif

#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
        qDebug("QOpenGLFramebufferObject::checkFramebufferStatus() Framebuffer incomplete, attachments "
              "must have same number of samples per pixel");
#endif
        break;
#endif

    default:
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
        qDebug() <<"QOpenGLFramebufferObject::checkFramebufferStatus() An undefined error has occurred: " << status;
#endif

        break;
    }

    return false;
}

namespace
{
void freeFramebufferFunc(QOpenGLFunctions *funcs, GLuint id) {
   funcs->glDeleteFramebuffers(1, &id);
}

void freeRenderbufferFunc(QOpenGLFunctions *funcs, GLuint id) {
   funcs->glDeleteRenderbuffers(1, &id);
}

void freeTextureFunc(QOpenGLFunctions *funcs, GLuint id) {
   funcs->glDeleteTextures(1, &id);
}
}

void QOpenGLFramebufferObjectPrivate::init(QOpenGLFramebufferObject *, const QSize &size,
      QOpenGLFramebufferObject::Attachment attachment, GLenum texture_target, GLenum internal_format,
      GLint samples, bool mipmap)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();

    funcs.initializeOpenGLFunctions();

    if (! funcs.hasOpenGLFeature(QOpenGLFunctions::Framebuffers)) {
        return;
    }

    // Fall back using a normal non-msaa FBO if we do not have support for MSAA
    if (! funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample)
            || ! funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferBlit)) {
        samples = 0;

    } else if (! ctx->isOpenGLES() || ctx->format().majorVersion() >= 3) {
        GLint maxSamples;
        funcs.glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        samples = qBound(0, int(samples), int(maxSamples));
    }

    colorAttachments.append(ColorAttachment(size, internal_format));
    dsSize = size;

    samples = qMax(0, samples);
    requestedSamples = samples;

    target = texture_target;

    QT_RESET_GLERROR(); // reset error state
    GLuint fbo = 0;

    funcs.glGenFramebuffers(1, &fbo);
    funcs.glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    QOpenGLContextPrivate::get(ctx)->qgl_current_fbo_invalid = true;

    QT_CHECK_GLERROR();

    format.setTextureTarget(target);
    format.setInternalTextureFormat(internal_format);
    format.setMipmap(mipmap);

    if (samples == 0) {
       initTexture(0);
    } else {
       initColorBuffer(0, &samples);
    }

    format.setSamples(int(samples));

    initDepthStencilAttachments(ctx, attachment);

    if (valid) {
       fbo_guard = new QOpenGLSharedResourceGuard(ctx, fbo, freeFramebufferFunc);
    } else {
       funcs.glDeleteFramebuffers(1, &fbo);
    }

    QT_CHECK_GLERROR();
}

void QOpenGLFramebufferObjectPrivate::initTexture(int idx)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    GLuint texture = 0;

    funcs.glGenTextures(1, &texture);
    funcs.glBindTexture(target, texture);

    funcs.glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    funcs.glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    funcs.glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    funcs.glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    ColorAttachment &color(colorAttachments[idx]);

    GLuint pixelType = GL_UNSIGNED_BYTE;
    if (color.internalFormat == GL_RGB10_A2 || color.internalFormat == GL_RGB10) {
        pixelType = GL_UNSIGNED_INT_2_10_10_10_REV;
    }

    funcs.glTexImage2D(target, 0, color.internalFormat, color.size.width(), color.size.height(), 0,
          GL_RGBA, pixelType, nullptr);

    if (format.mipmap()) {
        int width = color.size.width();
        int height = color.size.height();
        int level = 0;

        while (width > 1 || height > 1) {
            width = qMax(1, width >> 1);
            height = qMax(1, height >> 1);
            ++level;

            funcs.glTexImage2D(target, level, color.internalFormat, width, height, 0,
                  GL_RGBA, pixelType, nullptr);
        }
    }

    funcs.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, target, texture, 0);

    QT_CHECK_GLERROR();

    funcs.glBindTexture(target, 0);
    valid = checkFramebufferStatus(ctx);

    if (valid) {
        color.guard = new QOpenGLSharedResourceGuard(ctx, texture, freeTextureFunc);
    } else {
        funcs.glDeleteTextures(1, &texture);
    }
}

void QOpenGLFramebufferObjectPrivate::initColorBuffer(int idx, GLint *samples)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    GLuint color_buffer = 0;


    ColorAttachment &color(colorAttachments[idx]);
    GLenum storageFormat = color.internalFormat;

    // ES requires a sized format. The older desktop extension does not. Correct the format on ES.
    if (ctx->isOpenGLES() && color.internalFormat == GL_RGBA) {
        if (funcs.hasOpenGLExtension(QOpenGLExtensions::Sized8Formats)) {
            storageFormat = GL_RGBA8;
        } else {
            storageFormat = GL_RGBA4;
        }
    }

    funcs.glGenRenderbuffers(1, &color_buffer);
    funcs.glBindRenderbuffer(GL_RENDERBUFFER, color_buffer);

    funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, *samples, storageFormat,
         color.size.width(), color.size.height());

    funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx,
         GL_RENDERBUFFER, color_buffer);

    QT_CHECK_GLERROR();
    valid = checkFramebufferStatus(ctx);

    if (valid) {
        // Query the actual number of samples. This can be greater than the requested
        // value since the typically supported values are 0, 4, 8, ..., and the
        // requests are mapped to the next supported value.
        funcs.glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, samples);
        color.guard = new QOpenGLSharedResourceGuard(ctx, color_buffer, freeRenderbufferFunc);
    } else {
        funcs.glDeleteRenderbuffers(1, &color_buffer);
    }
}

void QOpenGLFramebufferObjectPrivate::initDepthStencilAttachments(QOpenGLContext *ctx,
      QOpenGLFramebufferObject::Attachment attachment)
{
    // Use the same sample count for all attachments. format.samples() already contains
    // the actual number of samples for the color attachment and is not suitable. Use
    // requestedSamples instead.
    const int samples = requestedSamples;

    // free existing attachments
    if (depth_buffer_guard) {
        funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        depth_buffer_guard->free();
    }

    if (stencil_buffer_guard) {
        funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        if (stencil_buffer_guard != depth_buffer_guard) {
            stencil_buffer_guard->free();
        }
    }

    depth_buffer_guard    = nullptr;
    stencil_buffer_guard  = nullptr;

    GLuint depth_buffer   = 0;
    GLuint stencil_buffer = 0;

    // In practice, a combined depth-stencil buffer is supported by all desktop platforms, while a
    // separate stencil buffer is not. On embedded devices however, a combined depth-stencil buffer
    // might not be supported while separate buffers are, according to QTBUG-12861.

    if (attachment == QOpenGLFramebufferObject::CombinedDepthStencil
        && funcs.hasOpenGLExtension(QOpenGLExtensions::PackedDepthStencil))
    {
        // depth and stencil buffer needs another extension
        funcs.glGenRenderbuffers(1, &depth_buffer);
        funcs.glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
        Q_ASSERT(funcs.glIsRenderbuffer(depth_buffer));

        if (samples != 0 && funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample)) {
           funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                 GL_DEPTH24_STENCIL8, dsSize.width(), dsSize.height());
        } else {
            funcs.glRenderbufferStorage(GL_RENDERBUFFER,
               GL_DEPTH24_STENCIL8, dsSize.width(), dsSize.height());
        }

        stencil_buffer = depth_buffer;
        funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
        funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil_buffer);

        valid = checkFramebufferStatus(ctx);
        if (! valid) {
            funcs.glDeleteRenderbuffers(1, &depth_buffer);
            stencil_buffer = depth_buffer = 0;
        }
    }

    if (depth_buffer == 0 && (attachment == QOpenGLFramebufferObject::CombinedDepthStencil
        || (attachment == QOpenGLFramebufferObject::Depth)))
    {
        funcs.glGenRenderbuffers(1, &depth_buffer);
        funcs.glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
        Q_ASSERT(funcs.glIsRenderbuffer(depth_buffer));

        if (samples != 0 && funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample)) {
            if (ctx->isOpenGLES()) {
                if (funcs.hasOpenGLExtension(QOpenGLExtensions::Depth24)) {
                    funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                          GL_DEPTH_COMPONENT24, dsSize.width(), dsSize.height());
                } else {
                    funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                          GL_DEPTH_COMPONENT16, dsSize.width(), dsSize.height());
                }

            } else {
                funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                      GL_DEPTH_COMPONENT, dsSize.width(), dsSize.height());
            }

        } else {
            if (ctx->isOpenGLES()) {
                if (funcs.hasOpenGLExtension(QOpenGLExtensions::Depth24)) {
                    funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                          dsSize.width(), dsSize.height());
                } else {
                    funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,
                          dsSize.width(), dsSize.height());
                }

            } else {
                funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                      dsSize.width(), dsSize.height());
            }
        }

        funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
        valid = checkFramebufferStatus(ctx);

        if (! valid) {
            funcs.glDeleteRenderbuffers(1, &depth_buffer);
            depth_buffer = 0;
        }
    }

    if (stencil_buffer == 0 && (attachment == QOpenGLFramebufferObject::CombinedDepthStencil)) {
        funcs.glGenRenderbuffers(1, &stencil_buffer);
        funcs.glBindRenderbuffer(GL_RENDERBUFFER, stencil_buffer);
        Q_ASSERT(funcs.glIsRenderbuffer(stencil_buffer));

#ifdef QT_OPENGL_ES
        GLenum storage = GL_STENCIL_INDEX8;
#else
        GLenum storage = ctx->isOpenGLES() ? GL_STENCIL_INDEX8 : GL_STENCIL_INDEX;
#endif

        if (samples != 0 && funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample)) {
            funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, storage,
                  dsSize.width(), dsSize.height());
        } else {
            funcs.glRenderbufferStorage(GL_RENDERBUFFER, storage, dsSize.width(), dsSize.height());
        }

        funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
              GL_RENDERBUFFER, stencil_buffer);

        valid = checkFramebufferStatus(ctx);

        if (! valid) {
            funcs.glDeleteRenderbuffers(1, &stencil_buffer);
            stencil_buffer = 0;
        }
    }

    // The FBO might have become valid after removing the depth or stencil buffer.
    valid = checkFramebufferStatus(ctx);

    if (depth_buffer && stencil_buffer) {
        fbo_attachment = QOpenGLFramebufferObject::CombinedDepthStencil;
    } else if (depth_buffer) {
        fbo_attachment = QOpenGLFramebufferObject::Depth;
    } else {
        fbo_attachment = QOpenGLFramebufferObject::NoAttachment;
    }

    if (valid) {
        if (depth_buffer) {
            depth_buffer_guard = new QOpenGLSharedResourceGuard(ctx, depth_buffer, freeRenderbufferFunc);
        }

        if (stencil_buffer) {
            if (stencil_buffer == depth_buffer) {
                stencil_buffer_guard = depth_buffer_guard;
            } else {
                stencil_buffer_guard = new QOpenGLSharedResourceGuard(ctx, stencil_buffer, freeRenderbufferFunc);
            }
        }

    } else {
        if (depth_buffer) {
            funcs.glDeleteRenderbuffers(1, &depth_buffer);
        }

        if (stencil_buffer && depth_buffer != stencil_buffer) {
            funcs.glDeleteRenderbuffers(1, &stencil_buffer);
        }
    }

    QT_CHECK_GLERROR();

    format.setAttachment(fbo_attachment);
}

static inline GLenum effectiveInternalFormat(GLenum internalFormat)
{
    if (! internalFormat) {

#ifdef QT_OPENGL_ES_2
        internalFormat = GL_RGBA;
#else
        internalFormat = QOpenGLContext::currentContext()->isOpenGLES() ? GL_RGBA : GL_RGBA8;
#endif

    }

    return internalFormat;
}

QOpenGLFramebufferObject::QOpenGLFramebufferObject(const QSize &size, GLenum target)
    : d_ptr(new QOpenGLFramebufferObjectPrivate)
{
    Q_D(QOpenGLFramebufferObject);
    d->init(this, size, NoAttachment, target, effectiveInternalFormat(0));
}

QOpenGLFramebufferObject::QOpenGLFramebufferObject(int width, int height, GLenum target)
    : d_ptr(new QOpenGLFramebufferObjectPrivate)
{
    Q_D(QOpenGLFramebufferObject);
    d->init(this, QSize(width, height), NoAttachment, target, effectiveInternalFormat(0));
}

QOpenGLFramebufferObject::QOpenGLFramebufferObject(const QSize &size, const QOpenGLFramebufferObjectFormat &format)
    : d_ptr(new QOpenGLFramebufferObjectPrivate)
{
    Q_D(QOpenGLFramebufferObject);
    d->init(this, size, format.attachment(), format.textureTarget(), format.internalTextureFormat(),
          format.samples(), format.mipmap());
}

QOpenGLFramebufferObject::QOpenGLFramebufferObject(int width, int height, const QOpenGLFramebufferObjectFormat &format)
    : d_ptr(new QOpenGLFramebufferObjectPrivate)
{
    Q_D(QOpenGLFramebufferObject);
    d->init(this, QSize(width, height), format.attachment(), format.textureTarget(),
          format.internalTextureFormat(), format.samples(), format.mipmap());
}

QOpenGLFramebufferObject::QOpenGLFramebufferObject(int width, int height, Attachment attachment,
      GLenum target, GLenum internalFormat)
    : d_ptr(new QOpenGLFramebufferObjectPrivate)
{
    Q_D(QOpenGLFramebufferObject);
    d->init(this, QSize(width, height), attachment, target, effectiveInternalFormat(internalFormat));
}

QOpenGLFramebufferObject::QOpenGLFramebufferObject(const QSize &size, Attachment attachment,
      GLenum target, GLenum internalFormat)
    : d_ptr(new QOpenGLFramebufferObjectPrivate)
{
    Q_D(QOpenGLFramebufferObject);
    d->init(this, size, attachment, target, effectiveInternalFormat(internalFormat));
}

QOpenGLFramebufferObject::~QOpenGLFramebufferObject()
{
    Q_D(QOpenGLFramebufferObject);

    if (isBound()) {
       release();
    }

    for (const QOpenGLFramebufferObjectPrivate::ColorAttachment &color : d->colorAttachments) {
        if (color.guard) {
           color.guard->free();
        }
    }

    d->colorAttachments.clear();

    if (d->depth_buffer_guard) {
        d->depth_buffer_guard->free();
    }

    if (d->stencil_buffer_guard && d->stencil_buffer_guard != d->depth_buffer_guard) {
        d->stencil_buffer_guard->free();
    }

    if (d->fbo_guard) {
        d->fbo_guard->free();
    }

    QOpenGLContextPrivate *contextPrv = QOpenGLContextPrivate::get(QOpenGLContext::currentContext());

    if (contextPrv && contextPrv->qgl_current_fbo == this) {
        contextPrv->qgl_current_fbo_invalid = true;
        contextPrv->qgl_current_fbo = nullptr;
    }
}

void QOpenGLFramebufferObject::addColorAttachment(const QSize &size, GLenum internalFormat)
{
    Q_D(QOpenGLFramebufferObject);

    if (! QOpenGLContext::currentContext()->functions()->hasOpenGLFeature(QOpenGLFunctions::MultipleRenderTargets)) {
        qWarning("QOpenGLFramebufferObject::addColorAttachment() "
              "Multiple render targets not supported, ignoring extra color attachment request");
        return;
    }

    QOpenGLFramebufferObjectPrivate::ColorAttachment color(size, effectiveInternalFormat(internalFormat));
    d->colorAttachments.append(color);

    const int idx = d->colorAttachments.count() - 1;

    if (d->requestedSamples == 0) {
        d->initTexture(idx);
    } else {
        GLint samples = d->requestedSamples;
        d->initColorBuffer(idx, &samples);
    }
}

void QOpenGLFramebufferObject::addColorAttachment(int width, int height, GLenum internalFormat)
{
    addColorAttachment(QSize(width, height), internalFormat);
}

bool QOpenGLFramebufferObject::isValid() const
{
    Q_D(const QOpenGLFramebufferObject);
    return d->valid && d->fbo_guard && d->fbo_guard->id();
}

bool QOpenGLFramebufferObject::bind()
{
    if (! isValid()) {
        return false;
    }

    Q_D(QOpenGLFramebufferObject);
    QOpenGLContext *current = QOpenGLContext::currentContext();

    if (! current) {
        return false;
    }

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (current->shareGroup() != d->fbo_guard->group()) {
        qDebug("QOpenGLFramebufferObject::bind() Called from incompatible context");
    }
#endif

    d->funcs.glBindFramebuffer(GL_FRAMEBUFFER, d->fbo());

    QOpenGLContextPrivate::get(current)->qgl_current_fbo_invalid = true;
    QOpenGLContextPrivate::get(current)->qgl_current_fbo = this;

    if (d->format.samples() == 0) {
        // Create new textures to replace the ones stolen via takeTexture()
        for (int i = 0; i < d->colorAttachments.count(); ++i) {
            if (! d->colorAttachments[i].guard) {
                d->initTexture(i);
            }
        }
    }

    return d->valid;
}

bool QOpenGLFramebufferObject::release()
{
    if (! isValid()) {
        return false;
    }

    QOpenGLContext *current = QOpenGLContext::currentContext();
    if (! current) {
        return false;
    }

    Q_D(QOpenGLFramebufferObject);

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (current->shareGroup() != d->fbo_guard->group()) {
       qDebug("QOpenGLFramebufferObject::release() Called from incompatible context");
    }
#endif

    if (current) {
        d->funcs.glBindFramebuffer(GL_FRAMEBUFFER, current->defaultFramebufferObject());

        QOpenGLContextPrivate *contextPrv = QOpenGLContextPrivate::get(current);
        contextPrv->qgl_current_fbo_invalid = true;
        contextPrv->qgl_current_fbo = nullptr;
    }

    return true;
}

GLuint QOpenGLFramebufferObject::texture() const
{
    Q_D(const QOpenGLFramebufferObject);
    return d->colorAttachments[0].guard ? d->colorAttachments[0].guard->id() : 0;
}

QVector<GLuint> QOpenGLFramebufferObject::textures() const
{
    Q_D(const QOpenGLFramebufferObject);
    QVector<GLuint> ids;

    if (d->format.samples() != 0) {
       return ids;
    }

    ids.reserve(d->colorAttachments.count());

    for (const QOpenGLFramebufferObjectPrivate::ColorAttachment &color : d->colorAttachments) {
       ids.append(color.guard ? color.guard->id() : 0);
    }

    return ids;
}

GLuint QOpenGLFramebufferObject::takeTexture()
{
    return takeTexture(0);
}

GLuint QOpenGLFramebufferObject::takeTexture(int colorAttachmentIndex)
{
    Q_D(QOpenGLFramebufferObject);
    GLuint id = 0;

    if (isValid() && d->format.samples() == 0 && d->colorAttachments.count() > colorAttachmentIndex) {
        QOpenGLContext *current = QOpenGLContext::currentContext();

        if (current && current->shareGroup() == d->fbo_guard->group() && isBound()) {
            release();
        }

        id = d->colorAttachments[colorAttachmentIndex].guard ? d->colorAttachments[colorAttachmentIndex].guard->id() : 0;


        // Do not call free() on texture_guard, just null it out.
        // This way the texture will not be deleted when the guard is destroyed.
        d->colorAttachments[colorAttachmentIndex].guard = nullptr;
    }

    return id;
}

QSize QOpenGLFramebufferObject::size() const
{
    Q_D(const QOpenGLFramebufferObject);
    return d->dsSize;
}

QVector<QSize> QOpenGLFramebufferObject::sizes() const
{
    Q_D(const QOpenGLFramebufferObject);

    QVector<QSize> sz;
    sz.reserve(d->colorAttachments.size());

    for (const QOpenGLFramebufferObjectPrivate::ColorAttachment &color : d->colorAttachments) {
        sz.append(color.size);
    }

    return sz;
}

QOpenGLFramebufferObjectFormat QOpenGLFramebufferObject::format() const
{
    Q_D(const QOpenGLFramebufferObject);
    return d->format;
}

static inline QImage qt_gl_read_framebuffer_rgba8(const QSize &size, bool include_alpha, QOpenGLContext *context)
{
    QOpenGLFunctions *funcs = context->functions();
    const int w = size.width();
    const int h = size.height();

    bool isOpenGL12orBetter = ! context->isOpenGLES() &&
         (context->format().majorVersion() >= 2 || context->format().minorVersion() >= 2);

    if (isOpenGL12orBetter) {
        QImage img(size, include_alpha ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32);
        funcs->glReadPixels(0, 0, w, h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, img.bits());
        return img;
    }

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    // Without GL_UNSIGNED_INT_8_8_8_8_REV, GL_BGRA only makes sense on little endian.
    const bool has_bgra_ext = context->isOpenGLES()
                              ? context->hasExtension("GL_EXT_read_format_bgra")
                              : context->hasExtension("GL_EXT_bgra");

#ifndef Q_OS_IOS
    const char *renderer = reinterpret_cast<const char *>(funcs->glGetString(GL_RENDERER));
    const char *ver = reinterpret_cast<const char *>(funcs->glGetString(GL_VERSION));

    // Blacklist GPU chipsets that have problems with their BGRA support.
    const bool blackListed = (qstrcmp(renderer, "PowerVR Rogue G6200") == 0
                             && ::strstr(ver, "1.3") != nullptr) ||
                             (qstrcmp(renderer, "Mali-T760") == 0
                             && ::strstr(ver, "3.1") != nullptr) ||
                             (qstrcmp(renderer, "Mali-T720") == 0
                             && ::strstr(ver, "3.1") != nullptr) || qstrcmp(renderer, "PowerVR SGX 554") == 0;
#else
    const bool blackListed = true;
#endif

    const bool supports_bgra = has_bgra_ext && !blackListed;

    if (supports_bgra) {
        QImage img(size, include_alpha ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32);
        funcs->glReadPixels(0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, img.bits());
        return img;
    }
#endif

    QImage rgbaImage(size, include_alpha ? QImage::Format_RGBA8888_Premultiplied : QImage::Format_RGBX8888);
    funcs->glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgbaImage.bits());

    return rgbaImage;
}

static inline QImage qt_gl_read_framebuffer_rgb10a2(const QSize &size, bool include_alpha, QOpenGLContext *context)
{
    // assume OpenGL 1.2+ or ES 3.0+ here
    QImage img(size, include_alpha ? QImage::Format_A2BGR30_Premultiplied : QImage::Format_BGR30);
    context->functions()->glReadPixels(0, 0, size.width(), size.height(), GL_RGBA,
          GL_UNSIGNED_INT_2_10_10_10_REV, img.bits());

    return img;
}

static QImage qt_gl_read_framebuffer(const QSize &size, GLenum internal_format, bool include_alpha, bool flip)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    QOpenGLFunctions *funcs = ctx->functions();

    while (funcs->glGetError()) {
       // do nothing
    }

    switch (internal_format) {
       case GL_RGB:
       case GL_RGB8:
           return qt_gl_read_framebuffer_rgba8(size, false, ctx).mirrored(false, flip);

       case GL_RGB10:
           return qt_gl_read_framebuffer_rgb10a2(size, false, ctx).mirrored(false, flip);

       case GL_RGB10_A2:
           return qt_gl_read_framebuffer_rgb10a2(size, include_alpha, ctx).mirrored(false, flip);

       case GL_RGBA:
       case GL_RGBA8:
       default:
           return qt_gl_read_framebuffer_rgba8(size, include_alpha, ctx).mirrored(false, flip);
    }

    // error, may want to throw

    return QImage();
}

Q_GUI_EXPORT QImage qt_gl_read_framebuffer(const QSize &size, bool alpha_format, bool include_alpha)
{
    return qt_gl_read_framebuffer(size, alpha_format ? GL_RGBA : GL_RGB, include_alpha, true);
}

QImage QOpenGLFramebufferObject::toImage(bool flipped) const
{
    return toImage(flipped, 0);
}

// consider removing this method and make it a default argument instead.
QImage QOpenGLFramebufferObject::toImage() const
{
    return toImage(true, 0);
}

QImage QOpenGLFramebufferObject::toImage(bool flipped, int colorAttachmentIndex) const
{
    Q_D(const QOpenGLFramebufferObject);

    if (! d->valid) {
        return QImage();
    }

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (! ctx) {
        qWarning("QOpenGLFramebufferObject::toImage() called without a current context");
        return QImage();
    }

    if (d->colorAttachments.count() <= colorAttachmentIndex) {
        qWarning("QOpenGLFramebufferObject::toImage() Called for missing color attachment");
        return QImage();
    }

    GLuint prevFbo = 0;
    ctx->functions()->glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &prevFbo);

    if (prevFbo != d->fbo()) {
        const_cast<QOpenGLFramebufferObject *>(this)->bind();
    }

    QImage image;
    QOpenGLExtraFunctions *extraFuncs = ctx->extraFunctions();

    // qt_gl_read_framebuffer does not work on a multisample FBO
    if (format().samples() != 0) {
        QRect rect(QPoint(0, 0), size());

        if (extraFuncs->hasOpenGLFeature(QOpenGLFunctions::MultipleRenderTargets)) {
            QOpenGLFramebufferObject temp(d->colorAttachments[colorAttachmentIndex].size,
                  QOpenGLFramebufferObjectFormat());

            blitFramebuffer(&temp, rect, const_cast<QOpenGLFramebufferObject *>(this), rect,
                  GL_COLOR_BUFFER_BIT, GL_NEAREST, colorAttachmentIndex, 0);

            image = temp.toImage(flipped);

        } else {
            QOpenGLFramebufferObject temp(size(), QOpenGLFramebufferObjectFormat());
            blitFramebuffer(&temp, rect, const_cast<QOpenGLFramebufferObject *>(this), rect);
            image = temp.toImage(flipped);
        }

    } else {
        if (extraFuncs->hasOpenGLFeature(QOpenGLFunctions::MultipleRenderTargets)) {
            extraFuncs->glReadBuffer(GL_COLOR_ATTACHMENT0 + colorAttachmentIndex);
            image = qt_gl_read_framebuffer(d->colorAttachments[colorAttachmentIndex].size,
                  d->colorAttachments[colorAttachmentIndex].internalFormat, true, flipped);

            extraFuncs->glReadBuffer(GL_COLOR_ATTACHMENT0);

        } else {
            image = qt_gl_read_framebuffer(d->colorAttachments[0].size,
                  d->colorAttachments[0].internalFormat, true, flipped);
        }
    }

    if (prevFbo != d->fbo()) {
        ctx->functions()->glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    }

    return image;
}

bool QOpenGLFramebufferObject::bindDefault()
{
    QOpenGLContext *ctx = const_cast<QOpenGLContext *>(QOpenGLContext::currentContext());

    if (ctx) {
        ctx->functions()->glBindFramebuffer(GL_FRAMEBUFFER, ctx->defaultFramebufferObject());
        QOpenGLContextPrivate::get(ctx)->qgl_current_fbo_invalid = true;
        QOpenGLContextPrivate::get(ctx)->qgl_current_fbo = nullptr;
    }

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    else {
        qDebug("QOpenGLFramebufferObject::bindDefault() Called without current context");
    }
#endif

    return ctx != nullptr;
}

bool QOpenGLFramebufferObject::hasOpenGLFramebufferObjects()
{
    return QOpenGLContext::currentContext()->functions()->hasOpenGLFeature(QOpenGLFunctions::Framebuffers);
}

GLuint QOpenGLFramebufferObject::handle() const
{
    Q_D(const QOpenGLFramebufferObject);
    return d->fbo();
}

QOpenGLFramebufferObject::Attachment QOpenGLFramebufferObject::attachment() const
{
    Q_D(const QOpenGLFramebufferObject);

    if (d->valid) {
        return d->fbo_attachment;
    }

    return NoAttachment;
}

void QOpenGLFramebufferObject::setAttachment(QOpenGLFramebufferObject::Attachment attachment)
{
    Q_D(QOpenGLFramebufferObject);

    if (attachment == d->fbo_attachment || ! isValid()) {
       return;
    }

    QOpenGLContext *current = QOpenGLContext::currentContext();

    if (! current) {
        return;
    }

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (current->shareGroup() != d->fbo_guard->group()) {
        qDebug("QOpenGLFramebufferObject::setAttachment() called from incompatible context");
    }
#endif

    d->funcs.glBindFramebuffer(GL_FRAMEBUFFER, d->fbo());
    QOpenGLContextPrivate::get(current)->qgl_current_fbo_invalid = true;
    d->initDepthStencilAttachments(current, attachment);
}

bool QOpenGLFramebufferObject::isBound() const
{
    Q_D(const QOpenGLFramebufferObject);

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        return false;
    }

    GLint fbo = 0;
    ctx->functions()->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);

    return GLuint(fbo) == d->fbo();
}

bool QOpenGLFramebufferObject::hasOpenGLFramebufferBlit()
{
    return QOpenGLExtensions(QOpenGLContext::currentContext()).hasOpenGLExtension(QOpenGLExtensions::FramebufferBlit);
}

void QOpenGLFramebufferObject::blitFramebuffer(QOpenGLFramebufferObject *target,
      QOpenGLFramebufferObject *source, GLbitfield buffers, GLenum filter)
{
    if (! target && !source) {
       return;
    }

    QSize targetSize;
    QSize sourceSize;

    if (target) {
        targetSize = target->size();
    }

    if (source) {
        sourceSize = source->size();
    }

    if (targetSize.isEmpty()) {
        targetSize = sourceSize;

    } else if (sourceSize.isEmpty()) {
        sourceSize = targetSize;
    }

    blitFramebuffer(target, QRect(QPoint(0, 0), targetSize),
          source, QRect(QPoint(0, 0), sourceSize), buffers, filter);
}

void QOpenGLFramebufferObject::blitFramebuffer(QOpenGLFramebufferObject *target, const QRect &targetRect,
      QOpenGLFramebufferObject *source, const QRect &sourceRect, GLbitfield buffers, GLenum filter)
{
    blitFramebuffer(target, targetRect, source, sourceRect, buffers, filter, 0, 0);
}

void QOpenGLFramebufferObject::blitFramebuffer(QOpenGLFramebufferObject *target, const QRect &targetRect,
      QOpenGLFramebufferObject *source, const QRect &sourceRect, GLbitfield buffers,
      GLenum filter, int readColorAttachmentIndex, int drawColorAttachmentIndex)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();

    if (! ctx) {
       return;
    }

    QOpenGLExtensions extensions(ctx);

    if (! extensions.hasOpenGLExtension(QOpenGLExtensions::FramebufferBlit)) {
      return;
    }

    GLuint prevFbo = 0;
    ctx->functions()->glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &prevFbo);

    const int sx0 = sourceRect.left();
    const int sx1 = sourceRect.left() + sourceRect.width();
    const int sy0 = sourceRect.top();
    const int sy1 = sourceRect.top() + sourceRect.height();

    const int tx0 = targetRect.left();
    const int tx1 = targetRect.left() + targetRect.width();
    const int ty0 = targetRect.top();
    const int ty1 = targetRect.top() + targetRect.height();

    const GLuint defaultFboId = ctx->defaultFramebufferObject();

    extensions.glBindFramebuffer(GL_READ_FRAMEBUFFER, source ? source->handle() : defaultFboId);
    extensions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target ? target->handle() : defaultFboId);

    if (extensions.hasOpenGLFeature(QOpenGLFunctions::MultipleRenderTargets)) {
        extensions.glReadBuffer(GL_COLOR_ATTACHMENT0 + readColorAttachmentIndex);

        if (target != nullptr) {
            GLenum drawBuf = GL_COLOR_ATTACHMENT0 + drawColorAttachmentIndex;
            extensions.glDrawBuffers(1, &drawBuf);
        }
    }

    extensions.glBlitFramebuffer(sx0, sy0, sx1, sy1, tx0, ty0, tx1, ty1, buffers, filter);

    if (extensions.hasOpenGLFeature(QOpenGLFunctions::MultipleRenderTargets)) {
        extensions.glReadBuffer(GL_COLOR_ATTACHMENT0);
    }

    ctx->functions()->glBindFramebuffer(GL_FRAMEBUFFER, prevFbo); // sets both READ and DRAW
}
