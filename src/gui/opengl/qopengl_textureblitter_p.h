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

#ifndef QOPENGLTEXTUREBLITTER_P_H
#define QOPENGLTEXTUREBLITTER_P_H

#include <qopengl.h>
#include <qmatrix3x3.h>

class QOpenGLTextureBlitterPrivate;

class Q_GUI_EXPORT QOpenGLTextureBlitter
{
 public:
    QOpenGLTextureBlitter();

    QOpenGLTextureBlitter(const QOpenGLTextureBlitter &) = delete;
    QOpenGLTextureBlitter &operator=(const QOpenGLTextureBlitter &) = delete;

    ~QOpenGLTextureBlitter();

    enum Origin {
        OriginBottomLeft,
        OriginTopLeft
    };

    bool create();
    bool isCreated() const;
    void destroy();

    bool supportsExternalOESTarget() const;

    void bind(GLenum target = GL_TEXTURE_2D);
    void release();

    void setSwizzleRB(bool swizzle);
    void setOpacity(float opacity);

    void blit(GLuint texture, const QMatrix4x4 &targetTransform, Origin sourceOrigin);
    void blit(GLuint texture, const QMatrix4x4 &targetTransform, const QMatrix3x3 &sourceTransform);

    static QMatrix4x4 targetTransform(const QRectF &target, const QRect &viewport);
    static QMatrix3x3 sourceTransform(const QRectF &subTexture, const QSize &textureSize, Origin origin);

 private:
    Q_DECLARE_PRIVATE(QOpenGLTextureBlitter)
    QScopedPointer<QOpenGLTextureBlitterPrivate> d_ptr;
};

#endif
