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

#include <qopengl_buffer.h>

#include <qopengl.h>
#include <qatomic.h>

#include <qopengl_extensions_p.h>
#include <qopenglcontext_p.h>

class QOpenGLBufferPrivate
{
public:
    QOpenGLBufferPrivate(QOpenGLBuffer::Type t)
        : ref(1), type(t), guard(nullptr), usagePattern(QOpenGLBuffer::StaticDraw),
          actualUsagePattern(QOpenGLBuffer::StaticDraw), funcs(nullptr)
    { }

    QAtomicInt ref;
    QOpenGLBuffer::Type type;
    QOpenGLSharedResourceGuard *guard;
    QOpenGLBuffer::UsagePattern usagePattern;
    QOpenGLBuffer::UsagePattern actualUsagePattern;
    QOpenGLExtensions *funcs;
};

QOpenGLBuffer::QOpenGLBuffer()
    : d_ptr(new QOpenGLBufferPrivate(QOpenGLBuffer::VertexBuffer))
{
}

QOpenGLBuffer::QOpenGLBuffer(QOpenGLBuffer::Type type)
    : d_ptr(new QOpenGLBufferPrivate(type))
{ }

QOpenGLBuffer::QOpenGLBuffer(const QOpenGLBuffer &other)
    : d_ptr(other.d_ptr)
{
    d_ptr->ref.ref();
}

QOpenGLBuffer::~QOpenGLBuffer()
{
    if (! d_ptr->ref.deref()) {
        destroy();
        delete d_ptr;
    }
}

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

QOpenGLBuffer::Type QOpenGLBuffer::type() const
{
    Q_D(const QOpenGLBuffer);
    return d->type;
}

QOpenGLBuffer::UsagePattern QOpenGLBuffer::usagePattern() const
{
    Q_D(const QOpenGLBuffer);
    return d->usagePattern;
}

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

bool QOpenGLBuffer::isCreated() const
{
    Q_D(const QOpenGLBuffer);
    return d->guard && d->guard->id();
}

void QOpenGLBuffer::destroy()
{
    Q_D(QOpenGLBuffer);

    if (d->guard) {
        d->guard->free();
        d->guard = nullptr;
    }

    delete d->funcs;
    d->funcs = nullptr;
}

bool QOpenGLBuffer::read(int offset, void *data, int count)
{
#if ! defined(QT_OPENGL_ES)
    Q_D(QOpenGLBuffer);

    if (! d->funcs->hasOpenGLFeature(QOpenGLFunctions::Buffers) || ! d->guard->id()) {
        return false;
    }

    // Clear error state
    while (d->funcs->glGetError() != GL_NO_ERROR) {
       ;
    }

    d->funcs->glGetBufferSubData(d->type, offset, count, data);

    return d->funcs->glGetError() == GL_NO_ERROR;

#else
    (void) offset;
    (void) data;
    (void) count;

    return false;
#endif
}

void QOpenGLBuffer::write(int offset, const void *data, int count)
{
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (! isCreated()) {
       qDebug("QOpenGLBuffer::write() Buffer was not created");
    }
#endif

    Q_D(QOpenGLBuffer);

    if (d->guard && d->guard->id()) {
        d->funcs->glBufferSubData(d->type, offset, count, data);
    }
}

void QOpenGLBuffer::allocate(const void *data, int count)
{
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (! isCreated()) {
       qDebug("QOpenGLBuffer::allocate() Buffer was not created");
    }
#endif

    Q_D(QOpenGLBuffer);

    if (d->guard && d->guard->id()) {
        d->funcs->glBufferData(d->type, count, data, d->actualUsagePattern);
    }
}

bool QOpenGLBuffer::bind()
{
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (! isCreated()) {
       qDebug("QOpenGLBuffer::bind() Buffer was not created");
    }
#endif

    Q_D(const QOpenGLBuffer);
    GLuint bufferId = d->guard ? d->guard->id() : 0;

    if (bufferId) {
        if (d->guard->group() != QOpenGLContextGroup::currentContextGroup()) {

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
           qDebug("QOpenGLBuffer::bind() Buffer is not valid in the current context");
#endif
           return false;
        }

        d->funcs->glBindBuffer(d->type, bufferId);
        return true;

    } else {
        return false;
    }
}

void QOpenGLBuffer::release()
{
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (! isCreated()) {
       qDebug("QOpenGLBuffer::release() Buffer was not created");
    }
#endif

    Q_D(const QOpenGLBuffer);

    if (d->guard && d->guard->id()) {
        d->funcs->glBindBuffer(d->type, 0);
    }
}

void QOpenGLBuffer::release(QOpenGLBuffer::Type type)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();

    if (ctx) {
        ctx->functions()->glBindBuffer(GLenum(type), 0);
    }
}

GLuint QOpenGLBuffer::bufferId() const
{
    Q_D(const QOpenGLBuffer);
    return d->guard ? d->guard->id() : 0;
}

int QOpenGLBuffer::size() const
{
    Q_D(const QOpenGLBuffer);
    if (!d->guard || !d->guard->id()) {
        return -1;
    }

    GLint value = -1;
    d->funcs->glGetBufferParameteriv(d->type, GL_BUFFER_SIZE, &value);

    return value;
}

void *QOpenGLBuffer::map(QOpenGLBuffer::Access access)
{
    Q_D(QOpenGLBuffer);

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (! isCreated()) {
       qDebug("QOpenGLBuffer::map() Buffer was not created");
    }
#endif

    if (! d->guard || ! d->guard->id()) {
        return nullptr;
    }

    if (d->funcs->hasOpenGLExtension(QOpenGLExtensions::MapBufferRange)) {
        QOpenGLBuffer::RangeAccessFlags rangeAccess = Qt::EmptyFlag;

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

void *QOpenGLBuffer::mapRange(int offset, int count, QOpenGLBuffer::RangeAccessFlags access)
{
    Q_D(QOpenGLBuffer);

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (! isCreated()) {
       qDebug("QOpenGLBuffer::mapRange() Buffer was not created");
    }
#endif

    if (! d->guard || ! d->guard->id()) {
        return nullptr;
    }

    return d->funcs->glMapBufferRange(d->type, offset, count, access);
}

bool QOpenGLBuffer::unmap()
{
    Q_D(QOpenGLBuffer);

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (! isCreated()) {
       qDebug("QOpenGLBuffer::unmap() Buffer was not created");
    }
#endif

    if (! d->guard || !d->guard->id()) {
        return false;
    }

    return d->funcs->glUnmapBuffer(d->type) == GL_TRUE;
}
