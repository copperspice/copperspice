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

#ifndef QABSTRACTOPENGLTEXTURE_P_H
#define QABSTRACTOPENGLTEXTURE_P_H

#ifndef QT_NO_OPENGL

#include <qopengl.h>
#include <qopengl_texture.h>

#include <cmath>

namespace {

inline double qLog2(const double x)
{
    return std::log(x) / std::log(2.0);
}
}

class QOpenGLContext;
class QOpenGLTextureHelper;

class QOpenGLTexturePrivate
{
public:
    QOpenGLTexturePrivate(QOpenGLTexture::Target textureTarget,
                          QOpenGLTexture *qq);
    ~QOpenGLTexturePrivate();

    Q_DECLARE_PUBLIC(QOpenGLTexture)

    void resetFuncs(QOpenGLTextureHelper *funcs);
    void initializeOpenGLFunctions();

    bool create();
    void destroy();

    void bind();
    void bind(uint unit, QOpenGLTexture::TextureUnitReset reset = QOpenGLTexture::DontResetTextureUnit);
    void release();
    void release(uint unit, QOpenGLTexture::TextureUnitReset reset = QOpenGLTexture::DontResetTextureUnit);
    bool isBound() const;
    bool isBound(uint unit) const;

    void allocateStorage(QOpenGLTexture::PixelFormat pixelFormat, QOpenGLTexture::PixelType pixelType);
    void allocateMutableStorage(QOpenGLTexture::PixelFormat pixelFormat, QOpenGLTexture::PixelType pixelType);
    void allocateImmutableStorage();
    void setData(int mipLevel, int layer, QOpenGLTexture::CubeMapFace cubeFace,
                 QOpenGLTexture::PixelFormat sourceFormat, QOpenGLTexture::PixelType sourceType,
                 const void *data, const QOpenGLPixelTransferOptions * const options);
    void setCompressedData(int mipLevel, int layer, QOpenGLTexture::CubeMapFace cubeFace,
                           int dataSize, const void *data,
                           const QOpenGLPixelTransferOptions * const options);

    void setWrapMode(QOpenGLTexture::WrapMode mode);
    void setWrapMode(QOpenGLTexture::CoordinateDirection direction, QOpenGLTexture::WrapMode mode);
    QOpenGLTexture::WrapMode wrapMode(QOpenGLTexture::CoordinateDirection direction) const;

    QOpenGLTexture *createTextureView(QOpenGLTexture::Target target, QOpenGLTexture::TextureFormat viewFormat,
                                      int minimumMipmapLevel, int maximumMipmapLevel,
                                      int minimumLayer, int maximumLayer) const;

    int evaluateMipLevels() const;

    inline int maximumMipLevelCount() const
    {
        return 1 + std::floor(qLog2(qMax(dimensions[0], qMax(dimensions[1], dimensions[2]))));
    }

    static inline int mipLevelSize(int mipLevel, int baseLevelSize)
    {
        return std::floor(double(qMax(1, baseLevelSize >> mipLevel)));
    }

    bool isUsingImmutableStorage() const;

    QOpenGLTexture *q_ptr;
    QOpenGLContext *context;
    QOpenGLTexture::Target target;
    QOpenGLTexture::BindingTarget bindingTarget;
    GLuint textureId;
    QOpenGLTexture::TextureFormat format;
    QOpenGLTexture::TextureFormatClass formatClass;
    int dimensions[3];
    int requestedMipLevels;
    int mipLevels;
    int layers;
    int faces;

    int samples;
    bool fixedSamplePositions;

    int baseLevel;
    int maxLevel;

    QOpenGLTexture::SwizzleValue swizzleMask[4];
    QOpenGLTexture::DepthStencilMode depthStencilMode;
    QOpenGLTexture::ComparisonFunction comparisonFunction;
    QOpenGLTexture::ComparisonMode comparisonMode;

    QOpenGLTexture::Filter minFilter;
    QOpenGLTexture::Filter magFilter;
    float maxAnisotropy;
    QOpenGLTexture::WrapMode wrapModes[3];
    QVariantList borderColor;
    float minLevelOfDetail;
    float maxLevelOfDetail;
    float levelOfDetailBias;

    bool textureView;
    bool autoGenerateMipMaps;
    bool storageAllocated;

    QOpenGLTextureHelper *texFuncs;

    QOpenGLTexture::Features features;
};

#undef Q_CALL_MEMBER_FUNCTION

#endif // QT_NO_OPENGL

#endif // QABSTRACTOPENGLTEXTURE_P_H
