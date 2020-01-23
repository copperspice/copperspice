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

#include <qopengl.h>
#include <qopenglcontext_p.h>
#include <qatomic.h>
#include <qopenglbuffer.h>
#include <qopenglextensions_p.h>

class QOpenGLBufferPrivate
{
public:
    QOpenGLBufferPrivate(QOpenGLBuffer::Type t)
        : ref(1),
          type(t),
          guard(0),
          usagePattern(QOpenGLBuffer::StaticDraw),
          actualUsagePattern(QOpenGLBuffer::StaticDraw),
          funcs(0)
    {
    }

    QAtomicInt ref;
    QOpenGLBuffer::Type type;
    QOpenGLSharedResourceGuard *guard;
    QOpenGLBuffer::UsagePattern usagePattern;
    QOpenGLBuffer::UsagePattern actualUsagePattern;
    QOpenGLExtensions *funcs;
};

/*!
    Constructs a new buffer object of type QOpenGLBuffer::VertexBuffer.

    Note: this constructor just creates the QOpenGLBuffer instance.  The actual
    buffer object in the OpenGL server is not created until create() is called.

    \sa create()
*/
QOpenGLBuffer::QOpenGLBuffer()
    : d_ptr(new QOpenGLBufferPrivate(QOpenGLBuffer::VertexBuffer))
{
}

/*!
    Constructs a new buffer object of \a type.

    Note: this constructor just creates the QOpenGLBuffer instance.  The actual
    buffer object in the OpenGL server is not created until create() is called.

    \sa create()
*/
QOpenGLBuffer::QOpenGLBuffer(QOpenGLBuffer::Type type)
    : d_ptr(new QOpenGLBufferPrivate(type))
{
}

/*!
    Constructs a shallow copy of \a other.

    Note: QOpenGLBuffer does not implement copy-on-write semantics,
    so \a other will be affected whenever the copy is modified.
*/
QOpenGLBuffer::QOpenGLBuffer(const QOpenGLBuffer &other)
    : d_ptr(other.d_ptr)
{
    d_ptr->ref.ref();
}

/*!
    Destroys this buffer object, including the storage being
    used in the OpenGL server.
*/
QOpenGLBuffer::~QOpenGLBuffer()
{
    if (!d_ptr->ref.deref()) {
        destroy();
        delete d_ptr;
    }
}

/*!
    Assigns a shallow copy of \a other to this object.

    Note: QOpenGLBuffer does not implement copy-on-write semantics,
    so \a other will be affected whenever the copy is modified.
*/
QOpenGLBuffer &QOpenGLBuffer::operator=(const QOpenGLBuffer &other)
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
QOpenGLBuffer::Type QOpenGLBuffer::type() const
{
    Q_D(const QOpenGLBuffer);
    return d->type;
}

/*!
    Returns the usage pattern for this buffer object.
    The default value is StaticDraw.

    \sa setUsagePattern()
*/
QOpenGLBuffer::UsagePattern QOpenGLBuffer::usagePattern() const
{
    Q_D(const QOpenGLBuffer);
    return d->usagePattern;
}

/*!
    Sets the usage pattern for this buffer object to \a value.
    This function must be called before allocate() or write().

    \sa usagePattern(), allocate(), write()
*/
void QOpenGLBuffer::setUsagePattern(QOpenGLBuffer::UsagePattern value)
{
    Q_D(QOpenGLBuffer);
    d->usagePattern = d->actualUsagePattern = value;
}

namespace {
    void freeBufferFunc(QOpenGLFunctions *funcs, GLuint id)
    {
        funcs->glDeleteBuffers(1, &id);
    }
}

/*!
    Creates the buffer object in the OpenGL server.  Returns \c true if
    the object was created; false otherwise.

    This function must be called with a current QOpenGLContext.
    The buffer will be bound to and can only be used in
    that context (or any other context that is shared with it).

    This function will return false if the OpenGL implementation
    does not support buffers, or there is no current QOpenGLContext.

    \sa isCreated(), allocate(), write(), destroy()
*/
bool QOpenGLBuffer::create()
{
    Q_D(QOpenGLBuffer);
    if (d->guard && d->guard->id())
        return true;
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (ctx) {
        delete d->funcs;
        d->funcs = new QOpenGLExtensions(ctx);
        GLuint bufferId = 0;
        d->funcs->glGenBuffers(1, &bufferId);
        if (bufferId) {
            if (d->guard)
                d->guard->free();

            d->guard = new QOpenGLSharedResourceGuard(ctx, bufferId, freeBufferFunc);
            return true;
        }
    }
    return false;
}

/*!
    Returns \c true if this buffer has been created; false otherwise.

    \sa create(), destroy()
*/
bool QOpenGLBuffer::isCreated() const
{
    Q_D(const QOpenGLBuffer);
    return d->guard && d->guard->id();
}

/*!
    Destroys this buffer object, including the storage being
    used in the OpenGL server.  All references to the buffer will
    become invalid.
*/
void QOpenGLBuffer::destroy()
{
    Q_D(QOpenGLBuffer);
    if (d->guard) {
        d->guard->free();
        d->guard = 0;
    }
    delete d->funcs;
    d->funcs = 0;
}

/*!
    Reads the \a count bytes in this buffer starting at \a offset
    into \a data.  Returns \c true on success; false if reading from
    the buffer is not supported.  Buffer reading is not supported
    under OpenGL/ES.

    It is assumed that this buffer has been bound to the current context.

    \sa write(), bind()
*/
bool QOpenGLBuffer::read(int offset, void *data, int count)
{
#if !defined(QT_OPENGL_ES)
    Q_D(QOpenGLBuffer);
    if (!d->funcs->hasOpenGLFeature(QOpenGLFunctions::Buffers) || !d->guard->id())
        return false;
    while (d->funcs->glGetError() != GL_NO_ERROR) ; // Clear error state.
    d->funcs->glGetBufferSubData(d->type, offset, count, data);
    return d->funcs->glGetError() == GL_NO_ERROR;
#else
    Q_UNUSED(offset);
    Q_UNUSED(data);
    Q_UNUSED(count);
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
void QOpenGLBuffer::write(int offset, const void *data, int count)
{
#ifndef QT_NO_DEBUG
    if (!isCreated())
        qWarning("QOpenGLBuffer::allocate(): buffer not created");
#endif
    Q_D(QOpenGLBuffer);
    if (d->guard && d->guard->id())
        d->funcs->glBufferSubData(d->type, offset, count, data);
}

/*!
    Allocates \a count bytes of space to the buffer, initialized to
    the contents of \a data.  Any previous contents will be removed.

    It is assumed that create() has been called on this buffer and that
    it has been bound to the current context.

    \sa create(), read(), write()
*/
void QOpenGLBuffer::allocate(const void *data, int count)
{
#ifndef QT_NO_DEBUG
    if (!isCreated())
        qWarning("QOpenGLBuffer::allocate(): buffer not created");
#endif
    Q_D(QOpenGLBuffer);
    if (d->guard && d->guard->id())
        d->funcs->glBufferData(d->type, count, data, d->actualUsagePattern);
}

/*!
    \fn void QOpenGLBuffer::allocate(int count)
    \overload

    Allocates \a count bytes of space to the buffer.  Any previous
    contents will be removed.

    It is assumed that create() has been called on this buffer and that
    it has been bound to the current context.

    \sa create(), write()
*/

/*!
    Binds the buffer associated with this object to the current
    OpenGL context.  Returns \c false if binding was not possible, usually because
    type() is not supported on this OpenGL implementation.

    The buffer must be bound to the same QOpenGLContext current when create()
    was called, or to another QOpenGLContext that is sharing with it.
    Otherwise, false will be returned from this function.

    \sa release(), create()
*/
bool QOpenGLBuffer::bind()
{
#ifndef QT_NO_DEBUG
    if (!isCreated())
        qWarning("QOpenGLBuffer::bind(): buffer not created");
#endif
    Q_D(const QOpenGLBuffer);
    GLuint bufferId = d->guard ? d->guard->id() : 0;
    if (bufferId) {
        if (d->guard->group() != QOpenGLContextGroup::currentContextGroup()) {
#ifndef QT_NO_DEBUG
            qWarning("QOpenGLBuffer::bind: buffer is not valid in the current context");
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
    current OpenGL context.

    This function must be called with the same QOpenGLContext current
    as when bind() was called on the buffer.

    \sa bind()
*/
void QOpenGLBuffer::release()
{
#ifndef QT_NO_DEBUG
    if (!isCreated())
        qWarning("QOpenGLBuffer::release(): buffer not created");
#endif
    Q_D(const QOpenGLBuffer);
    if (d->guard && d->guard->id())
        d->funcs->glBindBuffer(d->type, 0);
}

/*!
    Releases the buffer associated with \a type in the current
    QOpenGLContext.

    This function is a direct call to \c{glBindBuffer(type, 0)}
    for use when the caller does not know which QOpenGLBuffer has
    been bound to the context but wants to make sure that it
    is released.

    \code
    QOpenGLBuffer::release(QOpenGLBuffer::VertexBuffer);
    \endcode
*/
void QOpenGLBuffer::release(QOpenGLBuffer::Type type)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (ctx)
        ctx->functions()->glBindBuffer(GLenum(type), 0);
}

/*!
    Returns the OpenGL identifier associated with this buffer; zero if
    the buffer has not been created.

    \sa isCreated()
*/
GLuint QOpenGLBuffer::bufferId() const
{
    Q_D(const QOpenGLBuffer);
    return d->guard ? d->guard->id() : 0;
}

/*!
    Returns the size of the data in this buffer, for reading operations.
    Returns -1 if fetching the buffer size is not supported, or the
    buffer has not been created.

    It is assumed that this buffer has been bound to the current context.

    \sa isCreated(), bind()
*/
int QOpenGLBuffer::size() const
{
    Q_D(const QOpenGLBuffer);
    if (!d->guard || !d->guard->id())
        return -1;
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

    \note This function is only supported under OpenGL ES 2.0 or
    earlier if the \c GL_OES_mapbuffer extension is present.

    \note On OpenGL ES 3.0 and newer, or, in case if desktop OpenGL,
    if \c GL_ARB_map_buffer_range is supported, this function uses
    \c glMapBufferRange instead of \c glMapBuffer.

    \sa unmap(), create(), bind(), mapRange()
*/
void *QOpenGLBuffer::map(QOpenGLBuffer::Access access)
{
    Q_D(QOpenGLBuffer);
#ifndef QT_NO_DEBUG
    if (!isCreated())
        qWarning("QOpenGLBuffer::map(): buffer not created");
#endif
    if (!d->guard || !d->guard->id())
        return 0;
    if (d->funcs->hasOpenGLExtension(QOpenGLExtensions::MapBufferRange)) {
        QOpenGLBuffer::RangeAccessFlags rangeAccess = 0;
        switch (access) {
        case QOpenGLBuffer::ReadOnly:
            rangeAccess = QOpenGLBuffer::RangeRead;
            break;
        case QOpenGLBuffer::WriteOnly:
            rangeAccess = QOpenGLBuffer::RangeWrite;
            break;
        case QOpenGLBuffer::ReadWrite:
            rangeAccess = QOpenGLBuffer::RangeRead | QOpenGLBuffer::RangeWrite;
            break;
        }
        return d->funcs->glMapBufferRange(d->type, 0, size(), rangeAccess);
    } else {
        return d->funcs->glMapBuffer(d->type, access);
    }
}

/*!
    Maps the range specified by \a offset and \a count of the contents
    of this buffer into the application's memory space and returns a
    pointer to it.  Returns null if memory mapping is not possible.
    The \a access parameter specifies a combination of access flags.

    It is assumed that create() has been called on this buffer and that
    it has been bound to the current context.

    \note This function is not available on OpenGL ES 2.0 and earlier.

    \sa unmap(), create(), bind()
 */
void *QOpenGLBuffer::mapRange(int offset, int count, QOpenGLBuffer::RangeAccessFlags access)
{
    Q_D(QOpenGLBuffer);
#ifndef QT_NO_DEBUG
    if (!isCreated())
        qWarning("QOpenGLBuffer::mapRange(): buffer not created");
#endif
    if (!d->guard || !d->guard->id())
        return 0;
    return d->funcs->glMapBufferRange(d->type, offset, count, access);
}

/*!
    Unmaps the buffer after it was mapped into the application's
    memory space with a previous call to map().  Returns \c true if
    the unmap succeeded; false otherwise.

    It is assumed that this buffer has been bound to the current context,
    and that it was previously mapped with map().

    \note This function is only supported under OpenGL ES 2.0 and
    earlier if the \c{GL_OES_mapbuffer} extension is present.

    \sa map()
*/
bool QOpenGLBuffer::unmap()
{
    Q_D(QOpenGLBuffer);
#ifndef QT_NO_DEBUG
    if (!isCreated())
        qWarning("QOpenGLBuffer::unmap(): buffer not created");
#endif
    if (!d->guard || !d->guard->id())
        return false;
    return d->funcs->glUnmapBuffer(d->type) == GL_TRUE;
}

