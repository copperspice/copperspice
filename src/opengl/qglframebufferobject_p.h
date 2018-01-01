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

#ifndef QGLFRAMEBUFFEROBJECT_P_H
#define QGLFRAMEBUFFEROBJECT_P_H

QT_BEGIN_NAMESPACE

QT_BEGIN_INCLUDE_NAMESPACE

#include <qglframebufferobject.h>
#include <qglpaintdevice_p.h>
#include <qgl_p.h>

QT_END_INCLUDE_NAMESPACE

#ifndef QT_OPENGL_ES
#define DEFAULT_FORMAT GL_RGBA8
#else
#define DEFAULT_FORMAT GL_RGBA
#endif

class QGLFramebufferObjectFormatPrivate
{
 public:
   QGLFramebufferObjectFormatPrivate()
      : ref(1),
        samples(0),
        attachment(QGLFramebufferObject::NoAttachment),
        target(GL_TEXTURE_2D),
        internal_format(DEFAULT_FORMAT),
        mipmap(false) {
   }
   QGLFramebufferObjectFormatPrivate
   (const QGLFramebufferObjectFormatPrivate *other)
      : ref(1),
        samples(other->samples),
        attachment(other->attachment),
        target(other->target),
        internal_format(other->internal_format),
        mipmap(other->mipmap) {
   }
   bool equals(const QGLFramebufferObjectFormatPrivate *other) {
      return samples == other->samples &&
             attachment == other->attachment &&
             target == other->target &&
             internal_format == other->internal_format &&
             mipmap == other->mipmap;
   }

   QAtomicInt ref;
   int samples;
   QGLFramebufferObject::Attachment attachment;
   GLenum target;
   GLenum internal_format;
   uint mipmap : 1;
};

class QGLFBOGLPaintDevice : public QGLPaintDevice
{
 public:
   QPaintEngine *paintEngine() const override {
      return fbo->paintEngine();
   }

   QSize size() const override {
      return fbo->size();
   }

   QGLContext *context() const override;

   QGLFormat format() const override {
      return fboFormat;
   }

   bool alphaRequested() const override {
      return reqAlpha;
   }

   void setFBO(QGLFramebufferObject *f,
               QGLFramebufferObject::Attachment attachment);

 private:
   QGLFramebufferObject *fbo;
   QGLFormat fboFormat;
   bool wasBound;
   bool reqAlpha;
};

class QGLFramebufferObjectPrivate
{
 public:
   QGLFramebufferObjectPrivate() : fbo_guard(0), texture(0), depth_buffer(0), stencil_buffer(0)
      , color_buffer(0), valid(false), engine(0) {}
   ~QGLFramebufferObjectPrivate() {}

   void init(QGLFramebufferObject *q, const QSize &sz,
             QGLFramebufferObject::Attachment attachment,
             GLenum internal_format, GLenum texture_target,
             GLint samples = 0, bool mipmap = false);
   bool checkFramebufferStatus() const;
   QGLSharedResourceGuard fbo_guard;
   GLuint texture;
   GLuint depth_buffer;
   GLuint stencil_buffer;
   GLuint color_buffer;
   GLenum target;
   QSize size;
   QGLFramebufferObjectFormat format;
   uint valid : 1;
   QGLFramebufferObject::Attachment fbo_attachment;
   mutable QPaintEngine *engine;
   QGLFBOGLPaintDevice glDevice;

   inline GLuint fbo() const {
      return fbo_guard.id();
   }
};


QT_END_NAMESPACE

#endif // QGLFRAMEBUFFEROBJECT_P_H
