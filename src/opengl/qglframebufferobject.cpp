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

#include <qglframebufferobject.h>
#include <qglframebufferobject_p.h>

#include <qdebug.h>
#include <qimage.h>
#include <qlibrary.h>
#include <qwindow.h>

#include <qgl_p.h>
#include <qfont_p.h>
#include <qpaintengineex_opengl2_p.h>

QImage cs_glRead_frameBuffer(const QSize &, bool, bool);

#define QGL_FUNC_CONTEXT  const QGLContext *ctx = QGLContext::currentContext();
#define QGL_FUNCP_CONTEXT const QGLContext *ctx = QGLContext::currentContext();

#if defined(CS_SHOW_DEBUG_OPENGL)

#define QT_RESET_GLERROR()                                \
{                                                         \
   while (QOpenGLContext::currentContext()->functions()->glGetError() != GL_NO_ERROR) { } \
}

#define QT_CHECK_GLERROR()                                \
{                                                         \
   GLenum err = QOpenGLContext::currentContext()->functions()->glGetError();  \
   if (err != GL_NO_ERROR) {                              \
      qDebug("[%s line %d] GL Error: %d", __FILE__, __LINE__, (int)err);      \
   }                                                      \
}

#else

#define QT_RESET_GLERROR() { }
#define QT_CHECK_GLERROR() { }

#endif

// #### TODO Properly #ifdef this class to use #define symbols actually defined
// by OpenGL/ES includes
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

void QGLFramebufferObjectFormat::detach()
{
   if (d->ref.load() != 1) {
      QGLFramebufferObjectFormatPrivate *newd
         = new QGLFramebufferObjectFormatPrivate(d);
      if (!d->ref.deref()) {
         delete d;
      }
      d = newd;
   }
}

QGLFramebufferObjectFormat::QGLFramebufferObjectFormat()
{
   d = new QGLFramebufferObjectFormatPrivate;
}

QGLFramebufferObjectFormat::QGLFramebufferObjectFormat(const QGLFramebufferObjectFormat &other)
{
   d = other.d;
   d->ref.ref();
}

QGLFramebufferObjectFormat &QGLFramebufferObjectFormat::operator=(const QGLFramebufferObjectFormat &other)
{
   if (d != other.d) {
      other.d->ref.ref();
      if (!d->ref.deref()) {
         delete d;
      }
      d = other.d;
   }
   return *this;
}

QGLFramebufferObjectFormat::~QGLFramebufferObjectFormat()
{
   if (!d->ref.deref()) {
      delete d;
   }
}

void QGLFramebufferObjectFormat::setSamples(int samples)
{
   detach();
   d->samples = samples;
}

int QGLFramebufferObjectFormat::samples() const
{
   return d->samples;
}

void QGLFramebufferObjectFormat::setMipmap(bool enabled)
{
   detach();
   d->mipmap = enabled;
}

bool QGLFramebufferObjectFormat::mipmap() const
{
   return d->mipmap;
}

void QGLFramebufferObjectFormat::setAttachment(QGLFramebufferObject::Attachment attachment)
{
   detach();
   d->attachment = attachment;
}

QGLFramebufferObject::Attachment QGLFramebufferObjectFormat::attachment() const
{
   return d->attachment;
}

void QGLFramebufferObjectFormat::setTextureTarget(GLenum target)
{
   detach();
   d->target = target;
}

GLenum QGLFramebufferObjectFormat::textureTarget() const
{
   return d->target;
}

void QGLFramebufferObjectFormat::setInternalTextureFormat(GLenum internalTextureFormat)
{
   detach();
   d->internal_format = internalTextureFormat;
}

GLenum QGLFramebufferObjectFormat::internalTextureFormat() const
{
   return d->internal_format;
}

bool QGLFramebufferObjectFormat::operator==(const QGLFramebufferObjectFormat &other) const
{
   if (d == other.d) {
      return true;
   } else {
      return d->equals(other.d);
   }
}

bool QGLFramebufferObjectFormat::operator!=(const QGLFramebufferObjectFormat &other) const
{
   return !(*this == other);
}

void QGLFBOGLPaintDevice::setFBO(QGLFramebufferObject *f,
   QGLFramebufferObject::Attachment attachment)
{
   fbo = f;
   m_thisFBO = fbo->d_func()->fbo(); // This shouldn't be needed

   // The context that the fbo was created in may not have depth
   // and stencil buffers, but the fbo itself might.
   fboFormat = QGLContext::currentContext()->format();
   if (attachment == QGLFramebufferObject::CombinedDepthStencil) {
      fboFormat.setDepth(true);
      fboFormat.setStencil(true);
   } else if (attachment == QGLFramebufferObject::Depth) {
      fboFormat.setDepth(true);
      fboFormat.setStencil(false);
   } else {
      fboFormat.setDepth(false);
      fboFormat.setStencil(false);
   }

   GLenum format = f->format().internalTextureFormat();
   reqAlpha = (format != GL_RGB

#ifdef GL_RGB5
         && format != GL_RGB5
#endif

#ifdef GL_RGB8
         && format != GL_RGB8
#endif
      );
}

QGLContext *QGLFBOGLPaintDevice::context() const
{
   return const_cast<QGLContext *>(QGLContext::currentContext());
}

bool QGLFramebufferObjectPrivate::checkFramebufferStatus() const
{
   QGL_FUNCP_CONTEXT;
   if (! ctx) {
      return false;   // Context no longer exists.
   }

   GLenum status = ctx->contextHandle()->functions()->glCheckFramebufferStatus(GL_FRAMEBUFFER);

   switch (status) {
      case GL_NO_ERROR:
      case GL_FRAMEBUFFER_COMPLETE:
         return true;

      case GL_FRAMEBUFFER_UNSUPPORTED:
#if defined(CS_SHOW_DEBUG_OPENGL)
         qDebug("QGLFramebufferObject() Unsupported framebuffer format.");
#endif
         break;

      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
#if defined(CS_SHOW_DEBUG_OPENGL)
         qDebug("QGLFramebufferObject() Framebuffer incomplete attachment.");
#endif
         break;

      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
#if defined(CS_SHOW_DEBUG_OPENGL)
         qDebug("QGLFramebufferObject() Framebuffer incomplete, missing attachment.");
#endif
         break;

#ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT
      case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT:
#if defined(CS_SHOW_DEBUG_OPENGL)
         qDebug("QGLFramebufferObject() Framebuffer incomplete, duplicate attachment.");
#endif
         break;
#endif

#ifdef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
#if defined(CS_SHOW_DEBUG_OPENGL)
         qDebug("QGLFramebufferObject() Framebuffer incomplete, attached images must have same dimensions.");
#endif
         break;
#endif

#ifdef GL_FRAMEBUFFER_INCOMPLETE_FORMATS
      case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
#if defined(CS_SHOW_DEBUG_OPENGL)
         qDebug("QGLFramebufferObject() Framebuffer incomplete, attached images must have same format.");
#endif
         break;
#endif

#ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
#if defined(CS_SHOW_DEBUG_OPENGL)
         qDebug("QGLFramebufferObject() Framebuffer incomplete, missing draw buffer.");
#endif
         break;
#endif

#ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
#if defined(CS_SHOW_DEBUG_OPENGL)
         qDebug("QGLFramebufferObject() Framebuffer incomplete, missing read buffer.");
#endif
         break;
#endif

#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
      case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
#if defined(CS_SHOW_DEBUG_OPENGL)
         qDebug("QGLFramebufferObject() Framebuffer incomplete, attachments must have same number of samples per pixel.");
#endif
         break;
#endif

      default:
#if defined(CS_SHOW_DEBUG_OPENGL)
         qDebug() << "QGLFramebufferObject() An undefined error has occurred: " << status;
#endif
         break;
   }

   return false;
}

namespace {
void freeFramebufferFunc(QGLContext *ctx, GLuint id)
{
   Q_ASSERT(ctx);
   ctx->contextHandle()->functions()->glDeleteFramebuffers(1, &id);
}

void freeRenderbufferFunc(QGLContext *ctx, GLuint id)
{
   Q_ASSERT(ctx);
   ctx->contextHandle()->functions()->glDeleteRenderbuffers(1, &id);
}

void freeTextureFunc(QGLContext *ctx, GLuint id)
{
   ctx->contextHandle()->functions()->glDeleteTextures(1, &id);
}

}

void QGLFramebufferObjectPrivate::init(QGLFramebufferObject *q, const QSize &sz,
      QGLFramebufferObject::Attachment attachment, GLenum texture_target,
      GLenum internal_format, GLint samples, bool mipmap)
{
   QGLContext *ctx = const_cast<QGLContext *>(QGLContext::currentContext());

   funcs.initializeOpenGLFunctions();

   if (! funcs.hasOpenGLFeature(QOpenGLFunctions::Framebuffers)) {
      return;
   }

   ctx->d_ptr->refreshCurrentFbo();

   size = sz;
   target = texture_target;
   // texture dimensions

   QT_RESET_GLERROR(); // reset error state
   GLuint fbo = 0;

   funcs.glGenFramebuffers(1, &fbo);
   funcs.glBindFramebuffer(GL_FRAMEBUFFER, fbo);

   GLuint texture        = 0;
   GLuint color_buffer   = 0;
   GLuint depth_buffer   = 0;
   GLuint stencil_buffer = 0;

   QT_CHECK_GLERROR();

   // init texture
   if (samples == 0) {
      funcs.glGenTextures(1, &texture);
      funcs.glBindTexture(target, texture);
      funcs.glTexImage2D(target, 0, internal_format, size.width(), size.height(), 0,
               GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

      if (mipmap) {
         int width  = size.width();
         int height = size.height();
         int level  = 0;

         while (width > 1 || height > 1) {
            width = qMax(1, width >> 1);
            height = qMax(1, height >> 1);
            ++level;
            funcs.glTexImage2D(target, level, internal_format, width, height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
         }
      }

      funcs.glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      funcs.glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      funcs.glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      funcs.glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      funcs.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, texture, 0);

      QT_CHECK_GLERROR();

      valid = checkFramebufferStatus();
      funcs.glBindTexture(target, 0);

      color_buffer = 0;

   } else {
      mipmap = false;
      GLint maxSamples;
      funcs.glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

      samples = qBound(0, int(samples), int(maxSamples));

      funcs.glGenRenderbuffers(1, &color_buffer);
      funcs.glBindRenderbuffer(GL_RENDERBUFFER, color_buffer);
      if (funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample) && samples > 0) {
         funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
            internal_format, size.width(), size.height());

      } else {
         samples = 0;
         funcs.glRenderbufferStorage(GL_RENDERBUFFER, internal_format,
            size.width(), size.height());
      }

      funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
         GL_RENDERBUFFER, color_buffer);

      QT_CHECK_GLERROR();

      valid = checkFramebufferStatus();

      if (valid) {
         funcs.glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);
      }
   }

   // In practice, a combined depth-stencil buffer is supported by all desktop platforms, while a
   // separate stencil buffer is not. On embedded devices however, a combined depth-stencil buffer
   // might not be supported while separate buffers are, according to QTBUG-12861.

   if (attachment == QGLFramebufferObject::CombinedDepthStencil
      && funcs.hasOpenGLExtension(QOpenGLExtensions::PackedDepthStencil)) {
      // depth and stencil buffer needs another extension
      funcs.glGenRenderbuffers(1, &depth_buffer);
      funcs.glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);

      Q_ASSERT(funcs.glIsRenderbuffer(depth_buffer));

      if (samples != 0 && funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample))
         funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
            GL_DEPTH24_STENCIL8, size.width(), size.height());
      else
         funcs.glRenderbufferStorage(GL_RENDERBUFFER,
            GL_DEPTH24_STENCIL8, size.width(), size.height());

      stencil_buffer = depth_buffer;
      funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
      funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil_buffer);

      valid = checkFramebufferStatus();
      if (! valid) {
         funcs.glDeleteRenderbuffers(1, &depth_buffer);
         stencil_buffer = depth_buffer = 0;
      }
   }

   if (depth_buffer == 0 && (attachment == QGLFramebufferObject::CombinedDepthStencil
         || (attachment == QGLFramebufferObject::Depth))) {
      funcs.glGenRenderbuffers(1, &depth_buffer);
      funcs.glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);

      Q_ASSERT(funcs.glIsRenderbuffer(depth_buffer));

      if (samples != 0 && funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample)) {

#ifdef QT_OPENGL_ES
         if (funcs.hasOpenGLExtension(QOpenGLExtensions::Depth24)) {
            funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
               GL_DEPTH_COMPONENT24_OES, size.width(), size.height());
         } else {
            funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
               GL_DEPTH_COMPONENT16, size.width(), size.height());
         }
#else
         if (ctx->contextHandle()->isOpenGLES()) {
            if (funcs.hasOpenGLExtension(QOpenGLExtensions::Depth24))
               funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                  GL_DEPTH_COMPONENT24, size.width(), size.height());
            else
               funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                  GL_DEPTH_COMPONENT16, size.width(), size.height());
         } else {
            funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
               GL_DEPTH_COMPONENT, size.width(), size.height());
         }
#endif
      } else {
#ifdef QT_OPENGL_ES
         if (funcs.hasOpenGLExtension(QOpenGLExtensions::Depth24)) {
            funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES,
               size.width(), size.height());
         } else {
            funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,
               size.width(), size.height());
         }
#else
         if (ctx->contextHandle()->isOpenGLES()) {
            if (funcs.hasOpenGLExtension(QOpenGLExtensions::Depth24)) {
               funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                  size.width(), size.height());
            } else {
               funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,
                  size.width(), size.height());
            }
         } else {
            funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.width(), size.height());
         }
#endif
      }

      funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
      valid = checkFramebufferStatus();
      if (! valid) {
         funcs.glDeleteRenderbuffers(1, &depth_buffer);
         depth_buffer = 0;
      }
   }

   if (stencil_buffer == 0 && (attachment == QGLFramebufferObject::CombinedDepthStencil)) {
      funcs.glGenRenderbuffers(1, &stencil_buffer);
      funcs.glBindRenderbuffer(GL_RENDERBUFFER, stencil_buffer);
      Q_ASSERT(funcs.glIsRenderbuffer(stencil_buffer));

#ifdef QT_OPENGL_ES
      GLenum storage = GL_STENCIL_INDEX8;
#else
      GLenum storage = ctx->contextHandle()->isOpenGLES() ? GL_STENCIL_INDEX8 : GL_STENCIL_INDEX;
#endif

      if (samples != 0 && funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample)) {
         funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, storage, size.width(), size.height());
      } else {
         funcs.glRenderbufferStorage(GL_RENDERBUFFER, storage, size.width(), size.height());
      }

      funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil_buffer);
      valid = checkFramebufferStatus();
      if (! valid) {
         funcs.glDeleteRenderbuffers(1, &stencil_buffer);
         stencil_buffer = 0;
      }
   }

   // The FBO might have become valid after removing the depth or stencil buffer.
   valid = checkFramebufferStatus();

   if (depth_buffer && stencil_buffer) {
      fbo_attachment = QGLFramebufferObject::CombinedDepthStencil;
   } else if (depth_buffer) {
      fbo_attachment = QGLFramebufferObject::Depth;
   } else {
      fbo_attachment = QGLFramebufferObject::NoAttachment;
   }

   funcs.glBindFramebuffer(GL_FRAMEBUFFER, ctx->d_ptr->current_fbo);

   if (valid) {
      fbo_guard = createSharedResourceGuard(ctx, fbo, freeFramebufferFunc);

      if (color_buffer) {
         color_buffer_guard = createSharedResourceGuard(ctx, color_buffer, freeRenderbufferFunc);
      } else {
         texture_guard = createSharedResourceGuard(ctx, texture, freeTextureFunc);
      }

      if (depth_buffer) {
         depth_buffer_guard = createSharedResourceGuard(ctx, depth_buffer, freeRenderbufferFunc);
      }

      if (stencil_buffer) {
         if (stencil_buffer == depth_buffer) {
            stencil_buffer_guard = depth_buffer_guard;
         } else {
            stencil_buffer_guard = createSharedResourceGuard(ctx, stencil_buffer, freeRenderbufferFunc);
         }
      }

   } else {
      if (color_buffer) {
         funcs.glDeleteRenderbuffers(1, &color_buffer);
      } else {
         funcs.glDeleteTextures(1, &texture);
      }
      if (depth_buffer) {
         funcs.glDeleteRenderbuffers(1, &depth_buffer);
      }
      if (stencil_buffer && depth_buffer != stencil_buffer) {
         funcs.glDeleteRenderbuffers(1, &stencil_buffer);
      }
      funcs.glDeleteFramebuffers(1, &fbo);
   }
   QT_CHECK_GLERROR();

   format.setTextureTarget(target);
   format.setSamples(int(samples));
   format.setAttachment(fbo_attachment);
   format.setInternalTextureFormat(internal_format);
   format.setMipmap(mipmap);
   glDevice.setFBO(q, attachment);
}

QGLFramebufferObject::QGLFramebufferObject(const QSize &size, GLenum target)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);

   d->init(this, size, NoAttachment, target,
#ifndef QT_OPENGL_ES_2
      QOpenGLContext::currentContext()->isOpenGLES() ? GL_RGBA : GL_RGBA8
#else
      GL_RGBA
#endif
   );
}

QGLFramebufferObject::QGLFramebufferObject(int width, int height, GLenum target)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);

   d->init(this, QSize(width, height), NoAttachment, target,
#ifndef QT_OPENGL_ES_2
      QOpenGLContext::currentContext()->isOpenGLES() ? GL_RGBA : GL_RGBA8
#else
      GL_RGBA
#endif
   );
}

QGLFramebufferObject::QGLFramebufferObject(const QSize &size, const QGLFramebufferObjectFormat &format)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);
   d->init(this, size, format.attachment(), format.textureTarget(), format.internalTextureFormat(),
      format.samples(), format.mipmap());
}

QGLFramebufferObject::QGLFramebufferObject(int width, int height, const QGLFramebufferObjectFormat &format)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);
   d->init(this, QSize(width, height), format.attachment(), format.textureTarget(),
      format.internalTextureFormat(), format.samples(), format.mipmap());
}

QGLFramebufferObject::QGLFramebufferObject(int width, int height, Attachment attachment,
      GLenum target, GLenum internal_format)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);

   if (!internal_format)
#ifdef QT_OPENGL_ES_2
      internal_format = GL_RGBA;
#else
      internal_format = QOpenGLContext::currentContext()->isOpenGLES() ? GL_RGBA : GL_RGBA8;
#endif
   d->init(this, QSize(width, height), attachment, target, internal_format);
}

QGLFramebufferObject::QGLFramebufferObject(const QSize &size, Attachment attachment,
      GLenum target, GLenum internal_format)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);

   if (!internal_format)
#ifdef QT_OPENGL_ES_2
      internal_format = GL_RGBA;
#else
      internal_format = QOpenGLContext::currentContext()->isOpenGLES() ? GL_RGBA : GL_RGBA8;
#endif

   d->init(this, size, attachment, target, internal_format);
}

QGLFramebufferObject::~QGLFramebufferObject()
{
   Q_D(QGLFramebufferObject);

   delete d->engine;

   if (d->texture_guard) {
      d->texture_guard->free();
   }
   if (d->color_buffer_guard) {
      d->color_buffer_guard->free();
   }
   if (d->depth_buffer_guard) {
      d->depth_buffer_guard->free();
   }
   if (d->stencil_buffer_guard && d->stencil_buffer_guard != d->depth_buffer_guard) {
      d->stencil_buffer_guard->free();
   }
   if (d->fbo_guard) {
      d->fbo_guard->free();
   }
}

bool QGLFramebufferObject::isValid() const
{
   Q_D(const QGLFramebufferObject);
   return d->valid && d->fbo_guard && d->fbo_guard->id();
}

bool QGLFramebufferObject::bind()
{
   if (!isValid()) {
      return false;
   }

   Q_D(QGLFramebufferObject);
   QGL_FUNC_CONTEXT;

   if (!ctx) {
      return false;   // Context no longer exists.
   }

   const QGLContext *current = QGLContext::currentContext();

#if defined(CS_SHOW_DEBUG_OPENGL)
   if (! current || QGLContextPrivate::contextGroup(current) != QGLContextPrivate::contextGroup(ctx)) {
      qDebug("QGLFramebufferObject::bind() Called from incompatible context");
   }
#endif

   d->funcs.glBindFramebuffer(GL_FRAMEBUFFER, d->fbo());
   d->valid = d->checkFramebufferStatus();

   if (d->valid && current) {
      current->d_ptr->setCurrentFbo(d->fbo());
   }

   return d->valid;
}

bool QGLFramebufferObject::release()
{
   if (!isValid()) {
      return false;
   }

   Q_D(QGLFramebufferObject);
   QGL_FUNC_CONTEXT;

   if (!ctx) {
      return false;   // Context no longer exists.
   }

   const QGLContext *current = QGLContext::currentContext();

#if defined(CS_SHOW_DEBUG_OPENGL)
   if (! current || QGLContextPrivate::contextGroup(current) != QGLContextPrivate::contextGroup(ctx)) {
      qDebug("QGLFramebufferObject::release() Called from incompatible context");
   }
#endif

   if (current) {
      current->d_ptr->setCurrentFbo(current->d_ptr->default_fbo);
      d->funcs.glBindFramebuffer(GL_FRAMEBUFFER, current->d_ptr->default_fbo);
   }

   return true;
}

GLuint QGLFramebufferObject::texture() const
{
   Q_D(const QGLFramebufferObject);
   return d->texture_guard ? d->texture_guard->id() : 0;
}

QSize QGLFramebufferObject::size() const
{
   Q_D(const QGLFramebufferObject);
   return d->size;
}

QGLFramebufferObjectFormat QGLFramebufferObject::format() const
{
   Q_D(const QGLFramebufferObject);
   return d->format;
}

QImage QGLFramebufferObject::toImage() const
{
   Q_D(const QGLFramebufferObject);
   if (!d->valid) {
      return QImage();
   }

   // cs_glRead_frameBuffer does not work on a multisample FBO
   if (format().samples() != 0) {
      QGLFramebufferObject temp(size(), QGLFramebufferObjectFormat());

      QRect rect(QPoint(0, 0), size());
      blitFramebuffer(&temp, rect, const_cast<QGLFramebufferObject *>(this), rect);

      return temp.toImage();
   }

   bool wasBound = isBound();
   if (! wasBound) {
      const_cast<QGLFramebufferObject *>(this)->bind();
   }

   QImage image = cs_glRead_frameBuffer(d->size, format().internalTextureFormat() != GL_RGB, true);
   if (!wasBound) {
      const_cast<QGLFramebufferObject *>(this)->release();
   }

   return image;
}

static QGLEngineThreadStorage<QGL2PaintEngineEx> *qt_buffer_2_engine()
{
   static QGLEngineThreadStorage<QGL2PaintEngineEx> retval;
   return &retval;
}

QPaintEngine *QGLFramebufferObject::paintEngine() const
{
   Q_D(const QGLFramebufferObject);
   if (d->engine) {
      return d->engine;
   }

   QPaintEngine *engine = qt_buffer_2_engine()->engine();
   if (engine->isActive() && engine->paintDevice() != this) {
      d->engine = new QGL2PaintEngineEx;
      return d->engine;
   }

   return engine;
}

bool QGLFramebufferObject::bindDefault()
{
   QGLContext *ctx = const_cast<QGLContext *>(QGLContext::currentContext());

   if (ctx) {
      QOpenGLFunctions functions(ctx->contextHandle());

      if (! functions.hasOpenGLFeature(QOpenGLFunctions::Framebuffers)) {
         return false;
      }

      ctx->d_ptr->setCurrentFbo(ctx->d_ptr->default_fbo);
      functions.glBindFramebuffer(GL_FRAMEBUFFER, ctx->d_ptr->default_fbo);

#if defined(CS_SHOW_DEBUG_OPENGL)
   } else {
      qDebug("QGLFramebufferObject::bindDefault() Called without current context.");
#endif

   }

   return ctx != nullptr;
}

bool QGLFramebufferObject::hasOpenGLFramebufferObjects()
{
   return qgl_hasFeature(QOpenGLFunctions::Framebuffers);
}

void QGLFramebufferObject::drawTexture(const QRectF &target, GLuint textureId, GLenum textureTarget)
{
   const_cast<QGLContext *>(QGLContext::currentContext())->drawTexture(target, textureId, textureTarget);
}

void QGLFramebufferObject::drawTexture(const QPointF &point, GLuint textureId, GLenum textureTarget)
{
   const_cast<QGLContext *>(QGLContext::currentContext())->drawTexture(point, textureId, textureTarget);
}

int QGLFramebufferObject::metric(PaintDeviceMetric metric) const
{
   Q_D(const QGLFramebufferObject);

   float dpmx = qt_defaultDpiX() * 100. / 2.54;
   float dpmy = qt_defaultDpiY() * 100. / 2.54;
   int w = d->size.width();
   int h = d->size.height();

   switch (metric) {
      case PdmWidth:
         return w;

      case PdmHeight:
         return h;

      case PdmWidthMM:
         return qRound(w * 1000 / dpmx);

      case PdmHeightMM:
         return qRound(h * 1000 / dpmy);

      case PdmNumColors:
         return 0;

      case PdmDepth:
         return 32;//d->depth;

      case PdmDpiX:
         return qRound(dpmx * 0.0254);

      case PdmDpiY:
         return qRound(dpmy * 0.0254);

      case PdmPhysicalDpiX:
         return qRound(dpmx * 0.0254);

      case PdmPhysicalDpiY:
         return qRound(dpmy * 0.0254);

      case QPaintDevice::PdmDevicePixelRatio:
         return 1;

      case QPaintDevice::PdmDevicePixelRatioScaled:
         return 1 * QPaintDevice::devicePixelRatioFScale();

      default:
         qWarning("QGLFramebufferObject::metric(), Unhandled metric type: %d.\n", metric);
         break;
   }

   return 0;
}

GLuint QGLFramebufferObject::handle() const
{
   Q_D(const QGLFramebufferObject);
   return d->fbo();
}

QGLFramebufferObject::Attachment QGLFramebufferObject::attachment() const
{
   Q_D(const QGLFramebufferObject);

   if (d->valid) {
      return d->fbo_attachment;
   }

   return NoAttachment;
}

bool QGLFramebufferObject::isBound() const
{
   Q_D(const QGLFramebufferObject);
   const QGLContext *current = QGLContext::currentContext();

   if (current) {
      current->d_ptr->refreshCurrentFbo();
      return current->d_ptr->current_fbo == d->fbo();
   }

   return false;
}

bool QGLFramebufferObject::hasOpenGLFramebufferBlit()
{
   return QOpenGLExtensions(QOpenGLContext::currentContext()).hasOpenGLExtension(QOpenGLExtensions::FramebufferBlit);
}

void QGLFramebufferObject::blitFramebuffer(QGLFramebufferObject *target, const QRect &targetRect,
   QGLFramebufferObject *source, const QRect &sourceRect, GLbitfield buffers, GLenum filter)
{
   const QGLContext *ctx = QGLContext::currentContext();
   if (! ctx || !ctx->contextHandle()) {
      return;
   }

   QOpenGLExtensions functions(ctx->contextHandle());
   if (! functions.hasOpenGLExtension(QOpenGLExtensions::FramebufferBlit)) {
      return;
   }

   QSurface *surface = ctx->contextHandle()->surface();
   const int height = static_cast<QWindow *>(surface)->height();

   const int sh = source ? source->height() : height;
   const int th = target ? target->height() : height;

   const int sx0 = sourceRect.left();
   const int sx1 = sourceRect.left() + sourceRect.width();
   const int sy0 = sh - (sourceRect.top() + sourceRect.height());
   const int sy1 = sh - sourceRect.top();

   const int tx0 = targetRect.left();
   const int tx1 = targetRect.left() + targetRect.width();
   const int ty0 = th - (targetRect.top() + targetRect.height());
   const int ty1 = th - targetRect.top();

   ctx->d_ptr->refreshCurrentFbo();

   functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, source ? source->handle() : 0);
   functions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target ? target->handle() : 0);

   functions.glBlitFramebuffer(sx0, sy0, sx1, sy1, tx0, ty0, tx1, ty1, buffers, filter);

   functions.glBindFramebuffer(GL_FRAMEBUFFER, ctx->d_ptr->current_fbo);
}
