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

#include <qgl.h>
#include <qgl_p.h>
#include <qopenglextensions_p.h>
#include <qatomic.h>
#include <qglbuffer.h>



class QGLBufferPrivate
{
 public:
   QGLBufferPrivate(QGLBuffer::Type t)
      : ref(1),
        type(t),
        guard(0),
        usagePattern(QGLBuffer::StaticDraw),
        actualUsagePattern(QGLBuffer::StaticDraw),
        funcs(0) {
   }

   QAtomicInt ref;
   QGLBuffer::Type type;
   QGLSharedResourceGuardBase *guard;
   QGLBuffer::UsagePattern usagePattern;
   QGLBuffer::UsagePattern actualUsagePattern;
   QOpenGLExtensions *funcs;
};

/*!
    Constructs a new buffer object of type QGLBuffer::VertexBuffer.

    Note: this constructor just creates the QGLBuffer instance.  The actual
    buffer object in the GL server is not created until create() is called.

    \sa create()
*/
QGLBuffer::QGLBuffer()
   : d_ptr(new QGLBufferPrivate(QGLBuffer::VertexBuffer))
{
}

/*!
    Constructs a new buffer object of \a type.

    Note: this constructor just creates the QGLBuffer instance.  The actual
    buffer object in the GL server is not created until create() is called.

    \sa create()
*/
QGLBuffer::QGLBuffer(QGLBuffer::Type type)
   : d_ptr(new QGLBufferPrivate(type))
{
}

/*!
    Constructs a shallow copy of \a other.

    Note: QGLBuffer does not implement copy-on-write semantics,
    so \a other will be affected whenever the copy is modified.
*/
QGLBuffer::QGLBuffer(const QGLBuffer &other)
   : d_ptr(other.d_ptr)
{
   d_ptr->ref.ref();
}

#define ctx QGLContext::currentContext();

/*!
    Destroys this buffer object, including the storage being
    used in the GL server.
*/
QGLBuffer::~QGLBuffer()
{
   if (!d_ptr->ref.deref()) {
      destroy();
      delete d_ptr;
   }
}

/*!
    Assigns a shallow copy of \a other to this object.

    Note: QGLBuffer does not implement copy-on-write semantics,
    so \a other will be affected whenever the copy is modified.
*/
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

/*!
    Returns the type of buffer represented by this object.
*/
QGLBuffer::Type QGLBuffer::type() const
{
   Q_D(const QGLBuffer);
   return d->type;
}

/*!
    Returns the usage pattern for this buffer object.
    The default value is StaticDraw.

    \sa setUsagePattern()
*/
QGLBuffer::UsagePattern QGLBuffer::usagePattern() const
{
   Q_D(const QGLBuffer);
   return d->usagePattern;
}

/*!
    Sets the usage pattern for this buffer object to \a value.
    This function must be called before allocate() or write().

    \sa usagePattern(), allocate(), write()
*/
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

/*!
    Destroys this buffer object, including the storage being
    used in the GL server.  All references to the buffer will
    become invalid.
*/
void QGLBuffer::destroy()
{
   Q_D(QGLBuffer);
   if (d->guard) {
      d->guard->free();
      d->guard = 0;
   }
}

/*!
    Reads the \a count bytes in this buffer starting at \a offset
    into \a data.  Returns true on success; false if reading from
    the buffer is not supported.  Buffer reading is not supported
    under OpenGL/ES.

    It is assumed that this buffer has been bound to the current context.

    \sa write(), bind()
*/
bool QGLBuffer::read(int offset, void *data, int count)
{
#if ! defined(QT_OPENGL_ES)
   Q_D(QGLBuffer);

   if (!d->funcs->hasOpenGLFeature(QOpenGLFunctions::Buffers) || !d->guard->id()) {
      return false;
   }

   while (d->funcs->glGetError() != GL_NO_ERROR) ; // Clear error state.
   d->funcs->glGetBufferSubData(d->type, offset, count, data);
   return d->funcs->glGetError() == GL_NO_ERROR;

#else

   return false;
#endif
}

/*!
    Replaces the \a count bytes of this buffer starting at \a offset
    with the contents of \a data.  Any other bytes in the buffer
    will be left unmodified.

    It is assumed that create() has been called on this buffer and that
    it has been bound to the current context.

    \sa create(), read(), allocate()
*/
void QGLBuffer::write(int offset, const void *data, int count)
{
#ifndef QT_NO_DEBUG
   if (!isCreated()) {
      qWarning("QGLBuffer::allocate(): buffer not created");
   }
#endif

   Q_D(QGLBuffer);
   if (d->guard && d->guard->id()) {
      d->funcs->glBufferSubData(d->type, offset, count, data);
   }
}


void QGLBuffer::allocate(const void *data, int count)
{
#ifndef QT_NO_DEBUG
   if (!isCreated()) {
      qWarning("QGLBuffer::allocate(): buffer not created");
   }
#endif

   Q_D(QGLBuffer);
   if (d->guard && d->guard->id()) {
      d->funcs->glBufferData(d->type, count, data, d->actualUsagePattern);
   }
}

bool QGLBuffer::bind()
{
#ifndef QT_NO_DEBUG
   if (!isCreated()) {
      qWarning("QGLBuffer::bind(): buffer not created");
   }

#endif
   Q_D(const QGLBuffer);
   GLuint bufferId = d->guard ? d->guard->id() : 0;

   if (bufferId) {
      if (d->guard->group() != QOpenGLContextGroup::currentContextGroup()) {

#ifndef QT_NO_DEBUG
         qWarning("QGLBuffer::bind: buffer is not valid in the current context");
#endif
         return false;
      }
      d->funcs->glBindBuffer(d->type, bufferId);
      return true;
   } else {
      return false;
   }
}

/*!
    Releases the buffer associated with this object from the
    current GL context.

    This function must be called with the same QGLContext current
    as when bind() was called on the buffer.

    \sa bind()
*/
void QGLBuffer::release()
{
#ifndef QT_NO_DEBUG
   if (!isCreated()) {
      qWarning("QGLBuffer::release(): buffer not created");
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

/*!
    Returns the GL identifier associated with this buffer; zero if
    the buffer has not been created.

    \sa isCreated()
*/
GLuint QGLBuffer::bufferId() const
{
   Q_D(const QGLBuffer);
   return d->guard ? d->guard->id() : 0;
}

#ifndef GL_BUFFER_SIZE
#define GL_BUFFER_SIZE 0x8764
#endif

/*!
    Returns the size of the data in this buffer, for reading operations.
    Returns -1 if fetching the buffer size is not supported, or the
    buffer has not been created.

    It is assumed that this buffer has been bound to the current context.

    \sa isCreated(), bind()
*/
int QGLBuffer::size() const
{
   Q_D(const QGLBuffer);
   if (!d->guard || !d->guard->id()) {
      return -1;
   }

   GLint value = -1;
   d->funcs->glGetBufferParameteriv(d->type, GL_BUFFER_SIZE, &value);
   return value;
}

/*!
    Maps the contents of this buffer into the application's memory
    space and returns a pointer to it.  Returns null if memory
    mapping is not possible.  The \a access parameter indicates the
    type of access to be performed.

    It is assumed that create() has been called on this buffer and that
    it has been bound to the current context.

    This function is only supported under OpenGL/ES if the
    \c{GL_OES_mapbuffer} extension is present.

    \sa unmap(), create(), bind()
*/
void *QGLBuffer::map(QGLBuffer::Access access)
{
   Q_D(QGLBuffer);

#ifndef QT_NO_DEBUG
   if (!isCreated()) {
      qWarning("QGLBuffer::map(): buffer not created");
   }

#endif
   if (!d->guard || !d->guard->id()) {
      return 0;
   }
   return d->funcs->glMapBuffer(d->type, access);
}

/*!
    Unmaps the buffer after it was mapped into the application's
    memory space with a previous call to map().  Returns true if
    the unmap succeeded; false otherwise.

    It is assumed that this buffer has been bound to the current context,
    and that it was previously mapped with map().

    This function is only supported under OpenGL/ES if the
    \c{GL_OES_mapbuffer} extension is present.

    \sa map()
*/
bool QGLBuffer::unmap()
{
   Q_D(QGLBuffer);
#ifndef QT_NO_DEBUG
   if (!isCreated()) {
      qWarning("QGLBuffer::unmap(): buffer not created");
   }
#endif

   if (!d->guard || !d->guard->id()) {
      return false;
   }
   return d->funcs->glUnmapBuffer(d->type) == GL_TRUE;
}

