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

#include "qglframebufferobject.h"
#include "qglframebufferobject_p.h"

#include <qdebug.h>
#include <qgl_p.h>
#include <qfont_p.h>

#if !defined(QT_OPENGL_ES_1)
#include <qpaintengineex_opengl2_p.h>
#endif

#ifndef QT_OPENGL_ES_2
#include <qpaintengine_opengl_p.h>
#endif

#include <qglframebufferobject.h>
#include <qlibrary.h>
#include <qimage.h>

QT_BEGIN_NAMESPACE

extern QImage qt_gl_read_framebuffer(const QSize &, bool, bool);

#define QGL_FUNC_CONTEXT const QGLContext *ctx = d_ptr->fbo_guard.context();
#define QGL_FUNCP_CONTEXT const QGLContext *ctx = fbo_guard.context();

#ifndef QT_NO_DEBUG
#define QT_RESET_GLERROR()                                \
{                                                         \
    while (glGetError() != GL_NO_ERROR) {}                \
}
#define QT_CHECK_GLERROR()                                \
{                                                         \
    GLenum err = glGetError();                            \
    if (err != GL_NO_ERROR) {                             \
        qDebug("[%s line %d] GL Error: %d",               \
               __FILE__, __LINE__, (int)err);             \
    }                                                     \
}
#else
#define QT_RESET_GLERROR() {}
#define QT_CHECK_GLERROR() {}
#endif

/*!
    \class QGLFramebufferObjectFormat
    \brief The QGLFramebufferObjectFormat class specifies the format of an OpenGL
    framebuffer object.

    \since 4.6

    \ingroup painting-3D

    A framebuffer object has several characteristics:
    \list
    \i \link setSamples() Number of samples per pixels.\endlink
    \i \link setAttachment() Depth and/or stencil attachments.\endlink
    \i \link setTextureTarget() Texture target.\endlink
    \i \link setInternalTextureFormat() Internal texture format.\endlink
    \endlist

    Note that the desired attachments or number of samples per pixels might not
    be supported by the hardware driver. Call QGLFramebufferObject::format()
    after creating a QGLFramebufferObject to find the exact format that was
    used to create the frame buffer object.

    \sa QGLFramebufferObject
*/

/*!
    \internal
*/
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

/*!
    Creates a QGLFramebufferObjectFormat object for specifying
    the format of an OpenGL framebuffer object.

    By default the format specifies a non-multisample framebuffer object with no
    attachments, texture target \c GL_TEXTURE_2D, and internal format \c GL_RGBA8.
    On OpenGL/ES systems, the default internal format is \c GL_RGBA.

    \sa samples(), attachment(), internalTextureFormat()
*/

QGLFramebufferObjectFormat::QGLFramebufferObjectFormat()
{
   d = new QGLFramebufferObjectFormatPrivate;
}

/*!
    Constructs a copy of \a other.
*/

QGLFramebufferObjectFormat::QGLFramebufferObjectFormat(const QGLFramebufferObjectFormat &other)
{
   d = other.d;
   d->ref.ref();
}

/*!
    Assigns \a other to this object.
*/

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

/*!
    Destroys the QGLFramebufferObjectFormat.
*/
QGLFramebufferObjectFormat::~QGLFramebufferObjectFormat()
{
   if (!d->ref.deref()) {
      delete d;
   }
}

/*!
    Sets the number of samples per pixel for a multisample framebuffer object
    to \a samples.  The default sample count of 0 represents a regular
    non-multisample framebuffer object.

    If the desired amount of samples per pixel is not supported by the hardware
    then the maximum number of samples per pixel will be used. Note that
    multisample framebuffer objects can not be bound as textures. Also, the
    \c{GL_EXT_framebuffer_multisample} extension is required to create a
    framebuffer with more than one sample per pixel.

    \sa samples()
*/
void QGLFramebufferObjectFormat::setSamples(int samples)
{
   detach();
   d->samples = samples;
}

/*!
    Returns the number of samples per pixel if a framebuffer object
    is a multisample framebuffer object. Otherwise, returns 0.
    The default value is 0.

    \sa setSamples()
*/
int QGLFramebufferObjectFormat::samples() const
{
   return d->samples;
}

/*!
    \since 4.8

    Enables mipmapping if \a enabled is true; otherwise disables it.

    Mipmapping is disabled by default.

    If mipmapping is enabled, additional memory will be allocated for
    the mipmap levels. The mipmap levels can be updated by binding the
    texture and calling glGenerateMipmap(). Mipmapping cannot be enabled
    for multisampled framebuffer objects.

    \sa mipmap(), QGLFramebufferObject::texture()
*/
void QGLFramebufferObjectFormat::setMipmap(bool enabled)
{
   detach();
   d->mipmap = enabled;
}

/*!
    \since 4.8

    Returns true if mipmapping is enabled.

    \sa setMipmap()
*/
bool QGLFramebufferObjectFormat::mipmap() const
{
   return d->mipmap;
}

/*!
    Sets the attachment configuration of a framebuffer object to \a attachment.

    \sa attachment()
*/
void QGLFramebufferObjectFormat::setAttachment(QGLFramebufferObject::Attachment attachment)
{
   detach();
   d->attachment = attachment;
}

/*!
    Returns the configuration of the depth and stencil buffers attached to
    a framebuffer object.  The default is QGLFramebufferObject::NoAttachment.

    \sa setAttachment()
*/
QGLFramebufferObject::Attachment QGLFramebufferObjectFormat::attachment() const
{
   return d->attachment;
}

/*!
    Sets the texture target of the texture attached to a framebuffer object to
    \a target. Ignored for multisample framebuffer objects.

    \sa textureTarget(), samples()
*/
void QGLFramebufferObjectFormat::setTextureTarget(GLenum target)
{
   detach();
   d->target = target;
}

/*!
    Returns the texture target of the texture attached to a framebuffer object.
    Ignored for multisample framebuffer objects.  The default is
    \c GL_TEXTURE_2D.

    \sa setTextureTarget(), samples()
*/
GLenum QGLFramebufferObjectFormat::textureTarget() const
{
   return d->target;
}

/*!
    Sets the internal format of a framebuffer object's texture or
    multisample framebuffer object's color buffer to
    \a internalTextureFormat.

    \sa internalTextureFormat()
*/
void QGLFramebufferObjectFormat::setInternalTextureFormat(GLenum internalTextureFormat)
{
   detach();
   d->internal_format = internalTextureFormat;
}

/*!
    Returns the internal format of a framebuffer object's texture or
    multisample framebuffer object's color buffer.  The default is
    \c GL_RGBA8 on desktop OpenGL systems, and \c GL_RGBA on
    OpenGL/ES systems.

    \sa setInternalTextureFormat()
*/
GLenum QGLFramebufferObjectFormat::internalTextureFormat() const
{
   return d->internal_format;
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
void QGLFramebufferObjectFormat::setTextureTarget(QMacCompatGLenum target)
{
   detach();
   d->target = target;
}

/*! \internal */
void QGLFramebufferObjectFormat::setInternalTextureFormat(QMacCompatGLenum internalTextureFormat)
{
   detach();
   d->internal_format = internalTextureFormat;
}
#endif

/*!
    Returns true if all the options of this framebuffer object format
    are the same as \a other; otherwise returns false.
*/
bool QGLFramebufferObjectFormat::operator==(const QGLFramebufferObjectFormat &other) const
{
   if (d == other.d) {
      return true;
   } else {
      return d->equals(other.d);
   }
}

/*!
    Returns false if all the options of this framebuffer object format
    are the same as \a other; otherwise returns true.
*/
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
#ifndef QT_OPENGL_ES
               && format != GL_RGB5 && format != GL_RGB8
#endif
              );
}

QGLContext *QGLFBOGLPaintDevice::context() const
{
   QGLContext *fboContext = const_cast<QGLContext *>(fbo->d_ptr->fbo_guard.context());
   QGLContext *currentContext = const_cast<QGLContext *>(QGLContext::currentContext());

   if (QGLContextPrivate::contextGroup(fboContext) == QGLContextPrivate::contextGroup(currentContext)) {
      return currentContext;
   } else {
      return fboContext;
   }
}

bool QGLFramebufferObjectPrivate::checkFramebufferStatus() const
{
   QGL_FUNCP_CONTEXT;
   if (!ctx) {
      return false;   // Context no longer exists.
   }
   GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
   switch (status) {
      case GL_NO_ERROR:
      case GL_FRAMEBUFFER_COMPLETE_EXT:
         return true;
         break;
      case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
         qDebug("QGLFramebufferObject: Unsupported framebuffer format.");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
         qDebug("QGLFramebufferObject: Framebuffer incomplete attachment.");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
         qDebug("QGLFramebufferObject: Framebuffer incomplete, missing attachment.");
         break;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT
      case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
         qDebug("QGLFramebufferObject: Framebuffer incomplete, duplicate attachment.");
         break;
#endif
      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
         qDebug("QGLFramebufferObject: Framebuffer incomplete, attached images must have same dimensions.");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
         qDebug("QGLFramebufferObject: Framebuffer incomplete, attached images must have same format.");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
         qDebug("QGLFramebufferObject: Framebuffer incomplete, missing draw buffer.");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
         qDebug("QGLFramebufferObject: Framebuffer incomplete, missing read buffer.");
         break;
      case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT:
         qDebug("QGLFramebufferObject: Framebuffer incomplete, attachments must have same number of samples per pixel.");
         break;
      default:
         qDebug() << "QGLFramebufferObject: An undefined error has occurred: " << status;
         break;
   }
   return false;
}

void QGLFramebufferObjectPrivate::init(QGLFramebufferObject *q, const QSize &sz,
                                       QGLFramebufferObject::Attachment attachment,
                                       GLenum texture_target, GLenum internal_format,
                                       GLint samples, bool mipmap)
{
   QGLContext *ctx = const_cast<QGLContext *>(QGLContext::currentContext());
   fbo_guard.setContext(ctx);

   bool ext_detected = (QGLExtensions::glExtensions() & QGLExtensions::FramebufferObject);
   if (!ext_detected || (ext_detected && !qt_resolve_framebufferobject_extensions(ctx))) {
      return;
   }

   size = sz;
   target = texture_target;
   // texture dimensions

   QT_RESET_GLERROR(); // reset error state
   GLuint fbo = 0;
   glGenFramebuffers(1, &fbo);
   glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo);
   fbo_guard.setId(fbo);

   glDevice.setFBO(q, attachment);

   QT_CHECK_GLERROR();
   // init texture
   if (samples == 0) {
      glGenTextures(1, &texture);
      glBindTexture(target, texture);
      glTexImage2D(target, 0, internal_format, size.width(), size.height(), 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      if (mipmap) {
         glGenerateMipmap(GL_TEXTURE_2D);
      }
#ifndef QT_OPENGL_ES
      glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
      glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameterf(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
      glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                             target, texture, 0);

      QT_CHECK_GLERROR();
      valid = checkFramebufferStatus();
      glBindTexture(target, 0);

      color_buffer = 0;
   } else {
      mipmap = false;
      GLint maxSamples;
      glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);

      samples = qBound(0, int(samples), int(maxSamples));

      glGenRenderbuffers(1, &color_buffer);
      glBindRenderbuffer(GL_RENDERBUFFER_EXT, color_buffer);
      if (glRenderbufferStorageMultisampleEXT && samples > 0) {
         glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, samples,
                                             internal_format, size.width(), size.height());
      } else {
         samples = 0;
         glRenderbufferStorage(GL_RENDERBUFFER_EXT, internal_format,
                               size.width(), size.height());
      }

      glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                GL_RENDERBUFFER_EXT, color_buffer);

      QT_CHECK_GLERROR();
      valid = checkFramebufferStatus();

      if (valid) {
         glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_SAMPLES_EXT, &samples);
      }
   }

   // In practice, a combined depth-stencil buffer is supported by all desktop platforms, while a
   // separate stencil buffer is not. On embedded devices however, a combined depth-stencil buffer
   // might not be supported while separate buffers are, according to QTBUG-12861.

   if (attachment == QGLFramebufferObject::CombinedDepthStencil
         && (QGLExtensions::glExtensions() & QGLExtensions::PackedDepthStencil)) {
      // depth and stencil buffer needs another extension
      glGenRenderbuffers(1, &depth_buffer);
      Q_ASSERT(!glIsRenderbuffer(depth_buffer));
      glBindRenderbuffer(GL_RENDERBUFFER_EXT, depth_buffer);
      Q_ASSERT(glIsRenderbuffer(depth_buffer));
      if (samples != 0 && glRenderbufferStorageMultisampleEXT)
         glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, samples,
                                             GL_DEPTH24_STENCIL8_EXT, size.width(), size.height());
      else
         glRenderbufferStorage(GL_RENDERBUFFER_EXT,
                               GL_DEPTH24_STENCIL8_EXT, size.width(), size.height());

      stencil_buffer = depth_buffer;
      glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                GL_RENDERBUFFER_EXT, depth_buffer);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                GL_RENDERBUFFER_EXT, stencil_buffer);

      valid = checkFramebufferStatus();
      if (!valid) {
         glDeleteRenderbuffers(1, &depth_buffer);
         stencil_buffer = depth_buffer = 0;
      }
   }

   if (depth_buffer == 0 && (attachment == QGLFramebufferObject::CombinedDepthStencil
                             || (attachment == QGLFramebufferObject::Depth))) {
      glGenRenderbuffers(1, &depth_buffer);
      Q_ASSERT(!glIsRenderbuffer(depth_buffer));
      glBindRenderbuffer(GL_RENDERBUFFER_EXT, depth_buffer);
      Q_ASSERT(glIsRenderbuffer(depth_buffer));
      if (samples != 0 && glRenderbufferStorageMultisampleEXT) {
#ifdef QT_OPENGL_ES
         if (QGLExtensions::glExtensions() & QGLExtensions::Depth24) {
            glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, samples,
                                                GL_DEPTH_COMPONENT24_OES, size.width(), size.height());
         } else {
            glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, samples,
                                                GL_DEPTH_COMPONENT16, size.width(), size.height());
         }
#else
         glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, samples,
                                             GL_DEPTH_COMPONENT, size.width(), size.height());
#endif
      } else {
#ifdef QT_OPENGL_ES
         if (QGLExtensions::glExtensions() & QGLExtensions::Depth24) {
            glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24_OES,
                                  size.width(), size.height());
         } else {
            glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16,
                                  size.width(), size.height());
         }
#else
         glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, size.width(), size.height());
#endif
      }
      glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                GL_RENDERBUFFER_EXT, depth_buffer);
      valid = checkFramebufferStatus();
      if (!valid) {
         glDeleteRenderbuffers(1, &depth_buffer);
         depth_buffer = 0;
      }
   }

   if (stencil_buffer == 0 && (attachment == QGLFramebufferObject::CombinedDepthStencil)) {
      glGenRenderbuffers(1, &stencil_buffer);
      Q_ASSERT(!glIsRenderbuffer(stencil_buffer));
      glBindRenderbuffer(GL_RENDERBUFFER_EXT, stencil_buffer);
      Q_ASSERT(glIsRenderbuffer(stencil_buffer));
      if (samples != 0 && glRenderbufferStorageMultisampleEXT) {
#ifdef QT_OPENGL_ES
         glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, samples,
                                             GL_STENCIL_INDEX8_EXT, size.width(), size.height());
#else
         glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, samples,
                                             GL_STENCIL_INDEX, size.width(), size.height());
#endif
      } else {
#ifdef QT_OPENGL_ES
         glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX8_EXT,
                               size.width(), size.height());
#else
         glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX,
                               size.width(), size.height());
#endif
      }
      glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                GL_RENDERBUFFER_EXT, stencil_buffer);
      valid = checkFramebufferStatus();
      if (!valid) {
         glDeleteRenderbuffers(1, &stencil_buffer);
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

   glBindFramebuffer(GL_FRAMEBUFFER_EXT, ctx->d_ptr->current_fbo);
   if (!valid) {
      if (color_buffer) {
         glDeleteRenderbuffers(1, &color_buffer);
      } else {
         glDeleteTextures(1, &texture);
      }
      if (depth_buffer) {
         glDeleteRenderbuffers(1, &depth_buffer);
      }
      if (stencil_buffer && depth_buffer != stencil_buffer) {
         glDeleteRenderbuffers(1, &stencil_buffer);
      }
      glDeleteFramebuffers(1, &fbo);
      fbo_guard.setId(0);
   }
   QT_CHECK_GLERROR();

   format.setTextureTarget(target);
   format.setSamples(int(samples));
   format.setAttachment(fbo_attachment);
   format.setInternalTextureFormat(internal_format);
   format.setMipmap(mipmap);
}

/*!
    \class QGLFramebufferObject
    \brief The QGLFramebufferObject class encapsulates an OpenGL framebuffer object.
    \since 4.2

    \ingroup painting-3D

    The QGLFramebufferObject class encapsulates an OpenGL framebuffer
    object, defined by the \c{GL_EXT_framebuffer_object} extension. In
    addition it provides a rendering surface that can be painted on
    with a QPainter, rendered to using native GL calls, or both. This
    surface can be bound and used as a regular texture in your own GL
    drawing code.  By default, the QGLFramebufferObject class
    generates a 2D GL texture (using the \c{GL_TEXTURE_2D} target),
    which is used as the internal rendering target.

    \bold{It is important to have a current GL context when creating a
    QGLFramebufferObject, otherwise initialization will fail.}

    OpenGL framebuffer objects and pbuffers (see
    \l{QGLPixelBuffer}{QGLPixelBuffer}) can both be used to render to
    offscreen surfaces, but there are a number of advantages with
    using framebuffer objects instead of pbuffers:

    \list 1
    \o A framebuffer object does not require a separate rendering
    context, so no context switching will occur when switching
    rendering targets. There is an overhead involved in switching
    targets, but in general it is cheaper than a context switch to a
    pbuffer.

    \o Rendering to dynamic textures (i.e. render-to-texture
    functionality) works on all platforms. No need to do explicit copy
    calls from a render buffer into a texture, as was necessary on
    systems that did not support the \c{render_texture} extension.

    \o It is possible to attach several rendering buffers (or texture
    objects) to the same framebuffer object, and render to all of them
    without doing a context switch.

    \o The OpenGL framebuffer extension is a pure GL extension with no
    system dependant WGL, CGL, or GLX parts. This makes using
    framebuffer objects more portable.
    \endlist

    When using a QPainter to paint to a QGLFramebufferObject you should take
    care that the QGLFramebufferObject is created with the CombinedDepthStencil
    attachment for QPainter to be able to render correctly.
    Note that you need to create a QGLFramebufferObject with more than one
    sample per pixel for primitives to be antialiased when drawing using a
    QPainter. To create a multisample framebuffer object you should use one of
    the constructors that take a QGLFramebufferObject parameter, and set the
    QGLFramebufferObject::samples() property to a non-zero value.

    When painting to a QGLFramebufferObject using QPainter, the state of
    the current GL context will be altered by the paint engine to reflect
    its needs.  Applications should not rely upon the GL state being reset
    to its original conditions, particularly the current shader program,
    GL viewport, texture units, and drawing modes.

    For multisample framebuffer objects a color render buffer is created,
    otherwise a texture with the specified texture target is created.
    The color render buffer or texture will have the specified internal
    format, and will be bound to the \c GL_COLOR_ATTACHMENT0
    attachment in the framebuffer object.

    If you want to use a framebuffer object with multisampling enabled
    as a texture, you first need to copy from it to a regular framebuffer
    object using QGLContext::blitFramebuffer().

    \section1 Threading

    As of Qt 4.8, it's possible to draw into a QGLFramebufferObject
    using a QPainter in a separate thread. Note that OpenGL 2.0 or
    OpenGL ES 2.0 is required for this to work. Also, under X11, it's
    necessary to set the Qt::AA_X11InitThreads application attribute.

    \sa {Framebuffer Object Example}
*/


/*!
    \enum QGLFramebufferObject::Attachment
    \since 4.3

    This enum type is used to configure the depth and stencil buffers
    attached to the framebuffer object when it is created.

    \value NoAttachment         No attachment is added to the framebuffer object. Note that the
                                OpenGL depth and stencil tests won't work when rendering to a
                                framebuffer object without any depth or stencil buffers.
                                This is the default value.

    \value CombinedDepthStencil If the \c GL_EXT_packed_depth_stencil extension is present,
                                a combined depth and stencil buffer is attached.
                                If the extension is not present, only a depth buffer is attached.

    \value Depth                A depth buffer is attached to the framebuffer object.

    \sa attachment()
*/


/*! \fn QGLFramebufferObject::QGLFramebufferObject(const QSize &size, GLenum target)

    Constructs an OpenGL framebuffer object and binds a 2D GL texture
    to the buffer of the size \a size. The texture is bound to the
    \c GL_COLOR_ATTACHMENT0 target in the framebuffer object.

    The \a target parameter is used to specify the GL texture
    target. The default target is \c GL_TEXTURE_2D. Keep in mind that
    \c GL_TEXTURE_2D textures must have a power of 2 width and height
    (e.g. 256x512), unless you are using OpenGL 2.0 or higher.

    By default, no depth and stencil buffers are attached. This behavior
    can be toggled using one of the overloaded constructors.

    The default internal texture format is \c GL_RGBA8 for desktop
    OpenGL, and \c GL_RGBA for OpenGL/ES.

    It is important that you have a current GL context set when
    creating the QGLFramebufferObject, otherwise the initialization
    will fail.

    \sa size(), texture(), attachment()
*/

QGLFramebufferObject::QGLFramebufferObject(const QSize &size, GLenum target)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);
   d->init(this, size, NoAttachment, target, DEFAULT_FORMAT);
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
QGLFramebufferObject::QGLFramebufferObject(const QSize &size, QMacCompatGLenum target)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);
   d->init(this, size, NoAttachment, target, DEFAULT_FORMAT);
}
#endif

/*! \overload

    Constructs an OpenGL framebuffer object and binds a 2D GL texture
    to the buffer of the given \a width and \a height.

    \sa size(), texture()
*/
QGLFramebufferObject::QGLFramebufferObject(int width, int height, GLenum target)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);
   d->init(this, QSize(width, height), NoAttachment, target, DEFAULT_FORMAT);
}

/*! \overload

    Constructs an OpenGL framebuffer object of the given \a size based on the
    supplied \a format.
*/

QGLFramebufferObject::QGLFramebufferObject(const QSize &size, const QGLFramebufferObjectFormat &format)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);
   d->init(this, size, format.attachment(), format.textureTarget(), format.internalTextureFormat(),
           format.samples(), format.mipmap());
}

/*! \overload

    Constructs an OpenGL framebuffer object of the given \a width and \a height
    based on the supplied \a format.
*/

QGLFramebufferObject::QGLFramebufferObject(int width, int height, const QGLFramebufferObjectFormat &format)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);
   d->init(this, QSize(width, height), format.attachment(), format.textureTarget(),
           format.internalTextureFormat(), format.samples(), format.mipmap());
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
QGLFramebufferObject::QGLFramebufferObject(int width, int height, QMacCompatGLenum target)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);
   d->init(this, QSize(width, height), NoAttachment, target, DEFAULT_FORMAT);
}
#endif

/*! \overload

    Constructs an OpenGL framebuffer object and binds a texture to the
    buffer of the given \a width and \a height.

    The \a attachment parameter describes the depth/stencil buffer
    configuration, \a target the texture target and \a internal_format
    the internal texture format. The default texture target is \c
    GL_TEXTURE_2D, while the default internal format is \c GL_RGBA8
    for desktop OpenGL and \c GL_RGBA for OpenGL/ES.

    \sa size(), texture(), attachment()
*/
QGLFramebufferObject::QGLFramebufferObject(int width, int height, Attachment attachment,
      GLenum target, GLenum internal_format)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);
   d->init(this, QSize(width, height), attachment, target, internal_format);
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
QGLFramebufferObject::QGLFramebufferObject(int width, int height, Attachment attachment,
      QMacCompatGLenum target, QMacCompatGLenum internal_format)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);
   d->init(this, QSize(width, height), attachment, target, internal_format);
}
#endif

/*! \overload

    Constructs an OpenGL framebuffer object and binds a texture to the
    buffer of the given \a size.

    The \a attachment parameter describes the depth/stencil buffer
    configuration, \a target the texture target and \a internal_format
    the internal texture format. The default texture target is \c
    GL_TEXTURE_2D, while the default internal format is \c GL_RGBA8
    for desktop OpenGL and \c GL_RGBA for OpenGL/ES.

    \sa size(), texture(), attachment()
*/
QGLFramebufferObject::QGLFramebufferObject(const QSize &size, Attachment attachment,
      GLenum target, GLenum internal_format)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);
   d->init(this, size, attachment, target, internal_format);
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
QGLFramebufferObject::QGLFramebufferObject(const QSize &size, Attachment attachment,
      QMacCompatGLenum target, QMacCompatGLenum internal_format)
   : d_ptr(new QGLFramebufferObjectPrivate)
{
   Q_D(QGLFramebufferObject);
   d->init(this, size, attachment, target, internal_format);
}
#endif

/*!
    \fn QGLFramebufferObject::~QGLFramebufferObject()

    Destroys the framebuffer object and frees any allocated resources.
*/
QGLFramebufferObject::~QGLFramebufferObject()
{
   Q_D(QGLFramebufferObject);
   QGL_FUNC_CONTEXT;

   delete d->engine;

   if (isValid() && ctx) {
      QGLShareContextScope scope(ctx);
      if (d->texture) {
         glDeleteTextures(1, &d->texture);
      }
      if (d->color_buffer) {
         glDeleteRenderbuffers(1, &d->color_buffer);
      }
      if (d->depth_buffer) {
         glDeleteRenderbuffers(1, &d->depth_buffer);
      }
      if (d->stencil_buffer && d->stencil_buffer != d->depth_buffer) {
         glDeleteRenderbuffers(1, &d->stencil_buffer);
      }
      GLuint fbo = d->fbo();
      glDeleteFramebuffers(1, &fbo);
   }
}

/*!
    \fn bool QGLFramebufferObject::isValid() const

    Returns true if the framebuffer object is valid.

    The framebuffer can become invalid if the initialization process
    fails, the user attaches an invalid buffer to the framebuffer
    object, or a non-power of two width/height is specified as the
    texture size if the texture target is \c{GL_TEXTURE_2D}.
    The non-power of two limitation does not apply if the OpenGL version
    is 2.0 or higher, or if the GL_ARB_texture_non_power_of_two extension
    is present.

    The framebuffer can also become invalid if the QGLContext that
    the framebuffer was created within is destroyed and there are
    no other shared contexts that can take over ownership of the
    framebuffer.
*/
bool QGLFramebufferObject::isValid() const
{
   Q_D(const QGLFramebufferObject);
   return d->valid && d->fbo_guard.context();
}

/*!
    \fn bool QGLFramebufferObject::bind()

    Switches rendering from the default, windowing system provided
    framebuffer to this framebuffer object.
    Returns true upon success, false otherwise.

    \sa release()
*/
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
#ifdef QT_DEBUG
   if (!current ||
         QGLContextPrivate::contextGroup(current) != QGLContextPrivate::contextGroup(ctx)) {
      qWarning("QGLFramebufferObject::bind() called from incompatible context");
   }
#endif
   glBindFramebuffer(GL_FRAMEBUFFER_EXT, d->fbo());
   d->valid = d->checkFramebufferStatus();
   if (d->valid && current) {
      current->d_ptr->current_fbo = d->fbo();
   }
   return d->valid;
}

/*!
    \fn bool QGLFramebufferObject::release()

    Switches rendering back to the default, windowing system provided
    framebuffer.
    Returns true upon success, false otherwise.

    \sa bind()
*/
bool QGLFramebufferObject::release()
{
   if (!isValid()) {
      return false;
   }
   QGL_FUNC_CONTEXT;
   if (!ctx) {
      return false;   // Context no longer exists.
   }

   const QGLContext *current = QGLContext::currentContext();

#ifdef QT_DEBUG
   if (!current ||
         QGLContextPrivate::contextGroup(current) != QGLContextPrivate::contextGroup(ctx)) {
      qWarning("QGLFramebufferObject::release() called from incompatible context");
   }
#endif

   if (current) {
      current->d_ptr->current_fbo = current->d_ptr->default_fbo;
      glBindFramebuffer(GL_FRAMEBUFFER_EXT, current->d_ptr->default_fbo);
   }

   return true;
}

/*!
    \fn GLuint QGLFramebufferObject::texture() const

    Returns the texture id for the texture attached as the default
    rendering target in this framebuffer object. This texture id can
    be bound as a normal texture in your own GL code.

    If a multisample framebuffer object is used then the value returned
    from this function will be invalid.
*/
GLuint QGLFramebufferObject::texture() const
{
   Q_D(const QGLFramebufferObject);
   return d->texture;
}

/*!
    \fn QSize QGLFramebufferObject::size() const

    Returns the size of the texture attached to this framebuffer
    object.
*/
QSize QGLFramebufferObject::size() const
{
   Q_D(const QGLFramebufferObject);
   return d->size;
}

/*!
    Returns the format of this framebuffer object.
*/
QGLFramebufferObjectFormat QGLFramebufferObject::format() const
{
   Q_D(const QGLFramebufferObject);
   return d->format;
}

/*!
    \fn QImage QGLFramebufferObject::toImage() const

    Returns the contents of this framebuffer object as a QImage.
*/
QImage QGLFramebufferObject::toImage() const
{
   Q_D(const QGLFramebufferObject);
   if (!d->valid) {
      return QImage();
   }

   // qt_gl_read_framebuffer doesn't work on a multisample FBO
   if (format().samples() != 0) {
      QGLFramebufferObject temp(size(), QGLFramebufferObjectFormat());

      QRect rect(QPoint(0, 0), size());
      blitFramebuffer(&temp, rect, const_cast<QGLFramebufferObject *>(this), rect);

      return temp.toImage();
   }

   bool wasBound = isBound();
   if (!wasBound) {
      const_cast<QGLFramebufferObject *>(this)->bind();
   }
   QImage image = qt_gl_read_framebuffer(d->size, format().internalTextureFormat() != GL_RGB, true);
   if (!wasBound) {
      const_cast<QGLFramebufferObject *>(this)->release();
   }

   return image;
}

#if !defined(QT_OPENGL_ES_1)
Q_GLOBAL_STATIC(QGLEngineThreadStorage<QGL2PaintEngineEx>, qt_buffer_2_engine)
#endif

#ifndef QT_OPENGL_ES_2
Q_GLOBAL_STATIC(QGLEngineThreadStorage<QOpenGLPaintEngine>, qt_buffer_engine)
#endif

/*! \reimp */
QPaintEngine *QGLFramebufferObject::paintEngine() const
{
   Q_D(const QGLFramebufferObject);
   if (d->engine) {
      return d->engine;
   }

#if !defined(QT_OPENGL_ES_1)
#if !defined (QT_OPENGL_ES_2)
   if (qt_gl_preferGL2Engine()) {
#endif
      QPaintEngine *engine = qt_buffer_2_engine()->engine();
      if (engine->isActive() && engine->paintDevice() != this) {
         d->engine = new QGL2PaintEngineEx;
         return d->engine;
      }
      return engine;
#if !defined (QT_OPENGL_ES_2)
   }
#endif
#endif

#if !defined(QT_OPENGL_ES_2)
   QPaintEngine *engine = qt_buffer_engine()->engine();
   if (engine->isActive() && engine->paintDevice() != this) {
      d->engine = new QOpenGLPaintEngine;
      return d->engine;
   }
   return engine;
#endif
}

/*!
    \fn bool QGLFramebufferObject::bindDefault()
    \internal

    Switches rendering back to the default, windowing system provided
    framebuffer.
    Returns true upon success, false otherwise.

    \sa bind(), release()
*/
bool QGLFramebufferObject::bindDefault()
{
   QGLContext *ctx = const_cast<QGLContext *>(QGLContext::currentContext());

   if (ctx) {
      bool ext_detected = (QGLExtensions::glExtensions() & QGLExtensions::FramebufferObject);
      if (!ext_detected || (ext_detected && !qt_resolve_framebufferobject_extensions(ctx))) {
         return false;
      }

      ctx->d_ptr->current_fbo = ctx->d_ptr->default_fbo;
      glBindFramebuffer(GL_FRAMEBUFFER_EXT, ctx->d_ptr->default_fbo);
#ifdef QT_DEBUG
   } else {
      qWarning("QGLFramebufferObject::bindDefault() called without current context.");
#endif
   }

   return ctx != 0;
}

/*!
    \fn bool QGLFramebufferObject::hasOpenGLFramebufferObjects()

    Returns true if the OpenGL \c{GL_EXT_framebuffer_object} extension
    is present on this system; otherwise returns false.
*/
bool QGLFramebufferObject::hasOpenGLFramebufferObjects()
{
   return (QGLExtensions::glExtensions() & QGLExtensions::FramebufferObject);
}

/*!
    \since 4.4

    Draws the given texture, \a textureId, to the given target rectangle,
    \a target, in OpenGL model space. The \a textureTarget should be a 2D
    texture target.

    The framebuffer object should be bound when calling this function.

    Equivalent to the corresponding QGLContext::drawTexture().
*/
void QGLFramebufferObject::drawTexture(const QRectF &target, GLuint textureId, GLenum textureTarget)
{
   const_cast<QGLContext *>(QGLContext::currentContext())->drawTexture(target, textureId, textureTarget);
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
void QGLFramebufferObject::drawTexture(const QRectF &target, QMacCompatGLuint textureId, QMacCompatGLenum textureTarget)
{
   const_cast<QGLContext *>(QGLContext::currentContext())->drawTexture(target, textureId, textureTarget);
}
#endif

/*!
    \since 4.4

    Draws the given texture, \a textureId, at the given \a point in OpenGL
    model space. The \a textureTarget should be a 2D texture target.

    The framebuffer object should be bound when calling this function.

    Equivalent to the corresponding QGLContext::drawTexture().
*/
void QGLFramebufferObject::drawTexture(const QPointF &point, GLuint textureId, GLenum textureTarget)
{
   const_cast<QGLContext *>(QGLContext::currentContext())->drawTexture(point, textureId, textureTarget);
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
void QGLFramebufferObject::drawTexture(const QPointF &point, QMacCompatGLuint textureId, QMacCompatGLenum textureTarget)
{
   const_cast<QGLContext *>(QGLContext::currentContext())->drawTexture(point, textureId, textureTarget);
}
#endif

/*! \reimp */
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

      default:
         qWarning("QGLFramebufferObject::metric(), Unhandled metric type: %d.\n", metric);
         break;
   }
   return 0;
}

/*!
    \fn GLuint QGLFramebufferObject::handle() const

    Returns the GL framebuffer object handle for this framebuffer
    object (returned by the \c{glGenFrameBuffersEXT()} function). This
    handle can be used to attach new images or buffers to the
    framebuffer. The user is responsible for cleaning up and
    destroying these objects.
*/
GLuint QGLFramebufferObject::handle() const
{
   Q_D(const QGLFramebufferObject);
   return d->fbo();
}

/*! \fn int QGLFramebufferObject::devType() const
    \internal
*/


/*!
    Returns the status of the depth and stencil buffers attached to
    this framebuffer object.
*/

QGLFramebufferObject::Attachment QGLFramebufferObject::attachment() const
{
   Q_D(const QGLFramebufferObject);
   if (d->valid) {
      return d->fbo_attachment;
   }
   return NoAttachment;
}

/*!
    \since 4.5

    Returns true if the framebuffer object is currently bound to a context,
    otherwise false is returned.
*/

bool QGLFramebufferObject::isBound() const
{
   Q_D(const QGLFramebufferObject);
   const QGLContext *current = QGLContext::currentContext();
   return current ? current->d_ptr->current_fbo == d->fbo() : false;
}

/*!
    \fn bool QGLFramebufferObject::hasOpenGLFramebufferBlit()

    \since 4.6

    Returns true if the OpenGL \c{GL_EXT_framebuffer_blit} extension
    is present on this system; otherwise returns false.

    \sa blitFramebuffer()
*/
bool QGLFramebufferObject::hasOpenGLFramebufferBlit()
{
   return (QGLExtensions::glExtensions() & QGLExtensions::FramebufferBlit);
}

/*!
    \since 4.6

    Blits from the \a sourceRect rectangle in the \a source framebuffer
    object to the \a targetRect rectangle in the \a target framebuffer object.

    If \a source or \a target is 0, the default framebuffer will be used
    instead of a framebuffer object as source or target respectively.

    The \a buffers parameter should be a mask consisting of any combination of
    \c GL_COLOR_BUFFER_BIT, \c GL_DEPTH_BUFFER_BIT, and
    \c GL_STENCIL_BUFFER_BIT.  Any buffer type that is not present both
    in the source and target buffers is ignored.

    The \a sourceRect and \a targetRect rectangles may have different sizes;
    in this case \a buffers should not contain \c GL_DEPTH_BUFFER_BIT or
    \c GL_STENCIL_BUFFER_BIT. The \a filter parameter should be set to
    \c GL_LINEAR or \c GL_NEAREST, and specifies whether linear or nearest
    interpolation should be used when scaling is performed.

    If \a source equals \a target a copy is performed within the same buffer.
    Results are undefined if the source and target rectangles overlap and
    have different sizes. The sizes must also be the same if any of the
    framebuffer objects are multisample framebuffers.

    Note that the scissor test will restrict the blit area if enabled.

    This function will have no effect unless hasOpenGLFramebufferBlit() returns
    true.

    \sa hasOpenGLFramebufferBlit()
*/
void QGLFramebufferObject::blitFramebuffer(QGLFramebufferObject *target, const QRect &targetRect,
      QGLFramebufferObject *source, const QRect &sourceRect,
      GLbitfield buffers,
      GLenum filter)
{
   if (!(QGLExtensions::glExtensions() & QGLExtensions::FramebufferBlit)) {
      return;
   }

   const QGLContext *ctx = QGLContext::currentContext();
   if (!ctx) {
      return;
   }

   const int height = ctx->device()->height();

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

   glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, source ? source->handle() : 0);
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, target ? target->handle() : 0);

   glBlitFramebufferEXT(sx0, sy0, sx1, sy1,
                        tx0, ty0, tx1, ty1,
                        buffers, filter);

   glBindFramebuffer(GL_FRAMEBUFFER_EXT, ctx->d_ptr->current_fbo);
}

QT_END_NAMESPACE
