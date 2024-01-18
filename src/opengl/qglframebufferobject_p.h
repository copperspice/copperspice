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

#ifndef QGL_FRAMEBUFFEROBJECT_P_H
#define QGL_FRAMEBUFFEROBJECT_P_H

#include <qglframebufferobject.h>

#include <qglpaintdevice_p.h>
#include <qgl_p.h>
#include <qopengl_extensions_p.h>

class QGLFramebufferObjectFormatPrivate
{
 public:
   QGLFramebufferObjectFormatPrivate()
      : ref(1), samples(0), attachment(QGLFramebufferObject::NoAttachment),
        target(GL_TEXTURE_2D), mipmap(false) {

#ifndef QT_OPENGL_ES_2
      QOpenGLContext *ctx = QOpenGLContext::currentContext();
      const bool isES = ctx ? ctx->isOpenGLES() : QOpenGLContext::openGLModuleType() != QOpenGLContext::LibGL;
      internal_format = isES ? GL_RGBA : GL_RGBA8;
#else
      internal_format = GL_RGBA;
#endif
   }

   QGLFramebufferObjectFormatPrivate(const QGLFramebufferObjectFormatPrivate *other)
      : ref(1), samples(other->samples), attachment(other->attachment), target(other->target),
        internal_format(other->internal_format), mipmap(other->mipmap) {
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
   bool reqAlpha;
};

class QGLFramebufferObjectPrivate
{
 public:
   QGLFramebufferObjectPrivate()
      : fbo_guard(nullptr), texture_guard(nullptr), depth_buffer_guard(nullptr),
        stencil_buffer_guard(nullptr), color_buffer_guard(nullptr), valid(false), engine(nullptr)
   {
   }

   ~QGLFramebufferObjectPrivate() { }

   void init(QGLFramebufferObject *q, const QSize &sz,
      QGLFramebufferObject::Attachment attachment,
      GLenum internal_format, GLenum texture_target,
      GLint samples = 0, bool mipmap = false);

   bool checkFramebufferStatus() const;

   QGLSharedResourceGuardBase *fbo_guard;
   QGLSharedResourceGuardBase *texture_guard;
   QGLSharedResourceGuardBase *depth_buffer_guard;
   QGLSharedResourceGuardBase *stencil_buffer_guard;
   QGLSharedResourceGuardBase *color_buffer_guard;

   GLenum target;
   QSize size;
   QGLFramebufferObjectFormat format;
   uint valid : 1;
   QGLFramebufferObject::Attachment fbo_attachment;
   mutable QPaintEngine *engine;
   QGLFBOGLPaintDevice glDevice;
   QOpenGLExtensions funcs;

   GLuint fbo() const {
      return fbo_guard ? fbo_guard->id() : 0;
   }
};

#endif
