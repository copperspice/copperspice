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

#include <qglbuffer.h>

#include <qgl.h>
#include <qatomic.h>

#include <qgl_p.h>
#include <qopengl_extensions_p.h>

class QGLBufferPrivate
{
 public:
   QGLBufferPrivate(QGLBuffer::Type t)
      : ref(1), type(t), guard(nullptr), usagePattern(QGLBuffer::StaticDraw),
        actualUsagePattern(QGLBuffer::StaticDraw), funcs(nullptr) {
   }

   QAtomicInt ref;
   QGLBuffer::Type type;
   QGLSharedResourceGuardBase *guard;
   QGLBuffer::UsagePattern usagePattern;
   QGLBuffer::UsagePattern actualUsagePattern;
   QOpenGLExtensions *funcs;
};

QGLBuffer::QGLBuffer()
   : d_ptr(new QGLBufferPrivate(QGLBuffer::VertexBuffer))
{
}

QGLBuffer::QGLBuffer(QGLBuffer::Type type)
   : d_ptr(new QGLBufferPrivate(type))
{
}

QGLBuffer::QGLBuffer(const QGLBuffer &other)
   : d_ptr(other.d_ptr)
{
   d_ptr->ref.ref();
}

#define ctx QGLContext::currentContext();

QGLBuffer::~QGLBuffer()
{
   if (!d_ptr->ref.deref()) {
      destroy();
      delete d_ptr;
   }
}

QGLBuffer &QGLBuffer::operator=(const QGLBuffer &other)
{
   if (d_ptr != other.d_ptr) {
      other.d_ptr->ref.ref();
      if (!d_ptr->ref.deref()) {
         destroy();
         delete d_ptr;
      }
      d_ptr = other.d_ptr;
   }
   return *this;
}

QGLBuffer::Type QGLBuffer::type() const
{
   Q_D(const QGLBuffer);
   return d->type;
}

QGLBuffer::UsagePattern QGLBuffer::usagePattern() const
{
   Q_D(const QGLBuffer);
   return d->usagePattern;
}

void QGLBuffer::setUsagePattern(QGLBuffer::UsagePattern value)
{
   Q_D(QGLBuffer);
   d->usagePattern = d->actualUsagePattern = value;
}

#undef ctx

namespace {
void freeBufferFunc(QGLContext *ctx, GLuint id)
{
   Q_ASSERT(ctx);
   ctx->contextHandle()->functions()->glDeleteBuffers(1, &id);
}
}

bool QGLBuffer::create()
{
   Q_D(QGLBuffer);

   if (d->guard && d->guard->id()) {
      return true;
   }

   QGLContext *ctx = const_cast<QGLContext *>(QGLContext::currentContext());

   if (ctx) {
      delete d->funcs;
      d->funcs = new QOpenGLExtensions(ctx->contextHandle());
      if (!d->funcs->hasOpenGLFeature(QOpenGLFunctions::Buffers)) {
         return false;
      }

      GLuint bufferId = 0;
      d->funcs->glGenBuffers(1, &bufferId);
      if (bufferId) {
         if (d->guard) {
            d->guard->free();
         }

         d->guard = createSharedResourceGuard(ctx, bufferId, freeBufferFunc);
         return true;
      }
   }
   return false;
}

#define ctx QGLContext::currentContext()

/*!
    Returns true if this buffer has been created; false otherwise.

    \sa create(), destroy()
*/
bool QGLBuffer::isCreated() const
{
   Q_D(const QGLBuffer);
   return d->guard && d->guard->id();
}

void QGLBuffer::destroy()
{
   Q_D(QGLBuffer);
   if (d->guard) {
      d->guard->free();
      d->guard = nullptr;
   }
}

bool QGLBuffer::read(int offset, void *data, int count)
{
#if ! defined(QT_OPENGL_ES)
   Q_D(QGLBuffer);

   if (! d->funcs->hasOpenGLFeature(QOpenGLFunctions::Buffers) || !d->guard->id()) {
      return false;
   }

   while (d->funcs->glGetError() != GL_NO_ERROR) ; // Clear error state.
   d->funcs->glGetBufferSubData(d->type, offset, count, data);
   return d->funcs->glGetError() == GL_NO_ERROR;

#else
   return false;

#endif
}

void QGLBuffer::write(int offset, const void *data, int count)
{
#if defined(CS_SHOW_DEBUG_OPENGL)
   if (! isCreated()) {
      qDebug("QGLBuffer::allocate() Buffer was not created");
   }
#endif

   Q_D(QGLBuffer);
   if (d->guard && d->guard->id()) {
      d->funcs->glBufferSubData(d->type, offset, count, data);
   }
}

void QGLBuffer::allocate(const void *data, int count)
{
#if defined(CS_SHOW_DEBUG_OPENGL)
   if (! isCreated()) {
      qDebug("QGLBuffer::allocate() Buffer was not created");
   }
#endif

   Q_D(QGLBuffer);
   if (d->guard && d->guard->id()) {
      d->funcs->glBufferData(d->type, count, data, d->actualUsagePattern);
   }
}

bool QGLBuffer::bind()
{
#if defined(CS_SHOW_DEBUG_OPENGL)
   if (! isCreated()) {
      qDebug("QGLBuffer::bind() Buffer was not created");
   }
#endif

   Q_D(const QGLBuffer);
   GLuint bufferId = d->guard ? d->guard->id() : 0;

   if (bufferId) {
      if (d->guard->group() != QOpenGLContextGroup::currentContextGroup()) {

#if defined(CS_SHOW_DEBUG_OPENGL)
         qDebug("QGLBuffer::bind() Buffer is not valid in the current context");
#endif
         return false;
      }

      d->funcs->glBindBuffer(d->type, bufferId);
      return true;

   } else {
      return false;
   }
}

void QGLBuffer::release()
{
#if defined(CS_SHOW_DEBUG_OPENGL)
   if (! isCreated()) {
      qDebug("QGLBuffer::release() Buffer was not created");
   }
#endif

   Q_D(const QGLBuffer);

   if (d->guard && d->guard->id()) {
      d->funcs->glBindBuffer(d->type, 0);
   }
}

#undef ctx

void QGLBuffer::release(QGLBuffer::Type type)
{
   if (QOpenGLContext *ctx = QOpenGLContext::currentContext()) {
      ctx->functions()->glBindBuffer(GLenum(type), 0);
   }
}

#define ctx QGLContext::currentContext()

GLuint QGLBuffer::bufferId() const
{
   Q_D(const QGLBuffer);
   return d->guard ? d->guard->id() : 0;
}

#ifndef GL_BUFFER_SIZE
#define GL_BUFFER_SIZE 0x8764
#endif

int QGLBuffer::size() const
{
   Q_D(const QGLBuffer);
   if (! d->guard || !d->guard->id()) {
      return -1;
   }

   GLint value = -1;
   d->funcs->glGetBufferParameteriv(d->type, GL_BUFFER_SIZE, &value);
   return value;
}

void *QGLBuffer::map(QGLBuffer::Access access)
{
   Q_D(QGLBuffer);

#if defined(CS_SHOW_DEBUG_OPENGL)
   if (! isCreated()) {
      qDebug("QGLBuffer::map() Buffer was not created");
   }
#endif

   if (! d->guard || !d->guard->id()) {
      return nullptr;
   }

   return d->funcs->glMapBuffer(d->type, access);
}

bool QGLBuffer::unmap()
{
   Q_D(QGLBuffer);

#if defined(CS_SHOW_DEBUG_OPENGL)
   if (! isCreated()) {
      qDebug("QGLBuffer::unmap() Buffer was not created");
   }
#endif

   if (! d->guard || ! d->guard->id()) {
      return false;
   }

   return d->funcs->glUnmapBuffer(d->type) == GL_TRUE;
}
