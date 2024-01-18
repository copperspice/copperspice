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

#ifndef QOPENGLFRAMEBUFFEROBJECT_H
#define QOPENGLFRAMEBUFFEROBJECT_H

#include <qglobal.h>

#ifndef QT_NO_OPENGL

#include <qopengl.h>
#include <qpaintdevice.h>
#include <qscopedpointer.h>

class QOpenGLFramebufferObjectPrivate;
class QOpenGLFramebufferObjectFormat;

class Q_GUI_EXPORT QOpenGLFramebufferObject
{
 public:
    enum Attachment {
        NoAttachment,
        CombinedDepthStencil,
        Depth
    };

    explicit QOpenGLFramebufferObject(const QSize &size, GLenum target = GL_TEXTURE_2D);
    QOpenGLFramebufferObject(int width, int height, GLenum target = GL_TEXTURE_2D);

    QOpenGLFramebufferObject(const QSize &size, Attachment attachment,
                         GLenum target = GL_TEXTURE_2D, GLenum internalFormat = 0);
    QOpenGLFramebufferObject(int width, int height, Attachment attachment,
                         GLenum target = GL_TEXTURE_2D, GLenum internalFormat = 0);

    QOpenGLFramebufferObject(const QSize &size, const QOpenGLFramebufferObjectFormat &format);
    QOpenGLFramebufferObject(int width, int height, const QOpenGLFramebufferObjectFormat &format);

    QOpenGLFramebufferObject(const QOpenGLFramebufferObject &) = delete;
    QOpenGLFramebufferObject &operator=(const QOpenGLFramebufferObject &) = delete;

    virtual ~QOpenGLFramebufferObject();

    void addColorAttachment(const QSize &size, GLenum internalFormat = 0);
    void addColorAttachment(int width, int height, GLenum internalFormat = 0);

    QOpenGLFramebufferObjectFormat format() const;

    bool isValid() const;
    bool isBound() const;
    bool bind();
    bool release();

    int width() const { return size().width(); }
    int height() const { return size().height(); }

    GLuint texture() const;
    QVector<GLuint> textures() const;

    GLuint takeTexture();
    GLuint takeTexture(int colorAttachmentIndex);

    QSize size() const;
    QVector<QSize> sizes() const;

    QImage toImage() const;
    QImage toImage(bool flipped) const;
    QImage toImage(bool flipped, int colorAttachmentIndex) const;

    Attachment attachment() const;
    void setAttachment(Attachment attachment);

    GLuint handle() const;

    static bool bindDefault();

    static bool hasOpenGLFramebufferObjects();

    static bool hasOpenGLFramebufferBlit();
    static void blitFramebuffer(QOpenGLFramebufferObject *target, const QRect &targetRect,
                                QOpenGLFramebufferObject *source, const QRect &sourceRect,
                                GLbitfield buffers,
                                GLenum filter,
                                int readColorAttachmentIndex,
                                int drawColorAttachmentIndex);
    static void blitFramebuffer(QOpenGLFramebufferObject *target, const QRect &targetRect,
                                QOpenGLFramebufferObject *source, const QRect &sourceRect,
                                GLbitfield buffers = GL_COLOR_BUFFER_BIT,
                                GLenum filter = GL_NEAREST);
    static void blitFramebuffer(QOpenGLFramebufferObject *target,
                                QOpenGLFramebufferObject *source,
                                GLbitfield buffers = GL_COLOR_BUFFER_BIT,
                                GLenum filter = GL_NEAREST);

 private:
    Q_DECLARE_PRIVATE(QOpenGLFramebufferObject)

    QScopedPointer<QOpenGLFramebufferObjectPrivate> d_ptr;
    friend class QOpenGLPaintDevice;
    friend class QOpenGLFBOGLPaintDevice;
};

class QOpenGLFramebufferObjectFormatPrivate;

class Q_GUI_EXPORT QOpenGLFramebufferObjectFormat
{
 public:
    QOpenGLFramebufferObjectFormat();
    QOpenGLFramebufferObjectFormat(const QOpenGLFramebufferObjectFormat &other);
    QOpenGLFramebufferObjectFormat &operator=(const QOpenGLFramebufferObjectFormat &other);

    ~QOpenGLFramebufferObjectFormat();

    void setSamples(int samples);
    int samples() const;

    void setMipmap(bool enabled);
    bool mipmap() const;

    void setAttachment(QOpenGLFramebufferObject::Attachment attachment);
    QOpenGLFramebufferObject::Attachment attachment() const;

    void setTextureTarget(GLenum target);
    GLenum textureTarget() const;

    void setInternalTextureFormat(GLenum internalTextureFormat);
    GLenum internalTextureFormat() const;

    bool operator==(const QOpenGLFramebufferObjectFormat& other) const;
    bool operator!=(const QOpenGLFramebufferObjectFormat& other) const;

private:
    QOpenGLFramebufferObjectFormatPrivate *d;

    void detach();
};

#endif // QT_NO_OPENGL

#endif // QOPENGLFRAMEBUFFEROBJECT_H
