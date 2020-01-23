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

#ifndef QOPENGLBUFFER_H
#define QOPENGLBUFFER_H

#include <qglobal.h>

#ifndef QT_NO_OPENGL

#include <qscopedpointer.h>
#include <qopengl.h>

class QOpenGLBufferPrivate;

class Q_GUI_EXPORT QOpenGLBuffer
{
public:
    enum Type
    {
        VertexBuffer        = 0x8892, // GL_ARRAY_BUFFER
        IndexBuffer         = 0x8893, // GL_ELEMENT_ARRAY_BUFFER
        PixelPackBuffer     = 0x88EB, // GL_PIXEL_PACK_BUFFER
        PixelUnpackBuffer   = 0x88EC  // GL_PIXEL_UNPACK_BUFFER
    };

    QOpenGLBuffer();
    explicit QOpenGLBuffer(QOpenGLBuffer::Type type);
    QOpenGLBuffer(const QOpenGLBuffer &other);
    ~QOpenGLBuffer();

    QOpenGLBuffer &operator=(const QOpenGLBuffer &other);

    enum UsagePattern
    {
        StreamDraw          = 0x88E0, // GL_STREAM_DRAW
        StreamRead          = 0x88E1, // GL_STREAM_READ
        StreamCopy          = 0x88E2, // GL_STREAM_COPY
        StaticDraw          = 0x88E4, // GL_STATIC_DRAW
        StaticRead          = 0x88E5, // GL_STATIC_READ
        StaticCopy          = 0x88E6, // GL_STATIC_COPY
        DynamicDraw         = 0x88E8, // GL_DYNAMIC_DRAW
        DynamicRead         = 0x88E9, // GL_DYNAMIC_READ
        DynamicCopy         = 0x88EA  // GL_DYNAMIC_COPY
    };

    enum Access
    {
        ReadOnly            = 0x88B8, // GL_READ_ONLY
        WriteOnly           = 0x88B9, // GL_WRITE_ONLY
        ReadWrite           = 0x88BA  // GL_READ_WRITE
    };

    enum RangeAccessFlag
    {
        RangeRead             = 0x0001, // GL_MAP_READ_BIT
        RangeWrite            = 0x0002, // GL_MAP_WRITE_BIT
        RangeInvalidate       = 0x0004, // GL_MAP_INVALIDATE_RANGE_BIT
        RangeInvalidateBuffer = 0x0008, // GL_MAP_INVALIDATE_BUFFER_BIT
        RangeFlushExplicit    = 0x0010, // GL_MAP_FLUSH_EXPLICIT_BIT
        RangeUnsynchronized   = 0x0020  // GL_MAP_UNSYNCHRONIZED_BIT
    };
    Q_DECLARE_FLAGS(RangeAccessFlags, RangeAccessFlag)

    QOpenGLBuffer::Type type() const;

    QOpenGLBuffer::UsagePattern usagePattern() const;
    void setUsagePattern(QOpenGLBuffer::UsagePattern value);

    bool create();
    bool isCreated() const;

    void destroy();

    bool bind();
    void release();

    static void release(QOpenGLBuffer::Type type);

    GLuint bufferId() const;

    int size() const;

    bool read(int offset, void *data, int count);
    void write(int offset, const void *data, int count);

    void allocate(const void *data, int count);
    inline void allocate(int count) { allocate(nullptr, count); }

    void *map(QOpenGLBuffer::Access access);
    void *mapRange(int offset, int count, QOpenGLBuffer::RangeAccessFlags access);
    bool unmap();

private:
    QOpenGLBufferPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QOpenGLBuffer)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QOpenGLBuffer::RangeAccessFlags)

#endif // QT_NO_OPENGL

#endif
