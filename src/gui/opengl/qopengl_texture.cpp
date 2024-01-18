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

#include <qopengl_texture.h>
#include <qopengl_texture_p.h>

#include <qcolor.h>
#include <qopenglfunctions.h>
#include <qopenglcontext.h>

#include <qopengl_texturehelper_p.h>
#include <qopenglcontext_p.h>

// this is to work around GL_TEXTURE_WRAP_R_OES which also has 0x8072 as value
#if ! defined(GL_TEXTURE_WRAP_R)
#define GL_TEXTURE_WRAP_R 0x8072
#endif

QOpenGLTexturePrivate::QOpenGLTexturePrivate(QOpenGLTexture::Target textureTarget, QOpenGLTexture *qq)
    : q_ptr(qq), context(nullptr), target(textureTarget), textureId(0), format(QOpenGLTexture::NoFormat),
      formatClass(QOpenGLTexture::NoFormatClass), requestedMipLevels(1), mipLevels(-1),
      layers(1), faces(1), samples(0), fixedSamplePositions(true), baseLevel(0), maxLevel(1000),
      depthStencilMode(QOpenGLTexture::DepthMode),
      comparisonFunction(QOpenGLTexture::CompareLessEqual),
      comparisonMode(QOpenGLTexture::CompareNone),
      minFilter(QOpenGLTexture::Nearest), magFilter(QOpenGLTexture::Nearest),
      maxAnisotropy(1.0f), minLevelOfDetail(-1000.0f), maxLevelOfDetail(1000.0f),
      levelOfDetailBias(0.0f), textureView(false), autoGenerateMipMaps(true),
      storageAllocated(false), texFuncs(nullptr)
{
    dimensions[0] = dimensions[1] = dimensions[2] = 1;

    switch (target) {
       case QOpenGLTexture::Target1D:
           bindingTarget = QOpenGLTexture::BindingTarget1D;
           break;
       case QOpenGLTexture::Target1DArray:
           bindingTarget = QOpenGLTexture::BindingTarget1DArray;
           break;
       case QOpenGLTexture::Target2D:
           bindingTarget = QOpenGLTexture::BindingTarget2D;
           break;
       case QOpenGLTexture::Target2DArray:
           bindingTarget = QOpenGLTexture::BindingTarget2DArray;
           break;
       case QOpenGLTexture::Target3D:
           bindingTarget = QOpenGLTexture::BindingTarget3D;
           break;
       case QOpenGLTexture::TargetCubeMap:
           bindingTarget = QOpenGLTexture::BindingTargetCubeMap;
           faces = 6;
           break;
       case QOpenGLTexture::TargetCubeMapArray:
           bindingTarget = QOpenGLTexture::BindingTargetCubeMapArray;
           faces = 6;
           break;
       case QOpenGLTexture::Target2DMultisample:
           bindingTarget = QOpenGLTexture::BindingTarget2DMultisample;
           break;
       case QOpenGLTexture::Target2DMultisampleArray:
           bindingTarget = QOpenGLTexture::BindingTarget2DMultisampleArray;
           break;
       case QOpenGLTexture::TargetRectangle:
           bindingTarget = QOpenGLTexture::BindingTargetRectangle;
           break;
       case QOpenGLTexture::TargetBuffer:
           bindingTarget = QOpenGLTexture::BindingTargetBuffer;
           break;
    }

    swizzleMask[0] = QOpenGLTexture::RedValue;
    swizzleMask[1] = QOpenGLTexture::GreenValue;
    swizzleMask[2] = QOpenGLTexture::BlueValue;
    swizzleMask[3] = QOpenGLTexture::AlphaValue;

    wrapModes[0] = wrapModes[1] = wrapModes[2] = target == QOpenGLTexture::TargetRectangle
        ? QOpenGLTexture::ClampToEdge : QOpenGLTexture::Repeat;
}

QOpenGLTexturePrivate::~QOpenGLTexturePrivate()
{
    destroy();
}

void QOpenGLTexturePrivate::initializeOpenGLFunctions()
{
    // If we already have a functions object, there is nothing to do
    if (texFuncs)
        return;

    // See if the context already has a suitable resource we can use.
    // If not create a functions object and add it to the context in case
    // others wish to use it too
    texFuncs = context->textureFunctions();
    if (!texFuncs) {
        texFuncs = new QOpenGLTextureHelper(context);
        context->setTextureFunctions(texFuncs);
    }
}

bool QOpenGLTexturePrivate::create()
{
    if (textureId != 0)
        return true;

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        qWarning("Requires a valid current OpenGL context.\n"
                 "Texture has not been created");
        return false;
    }
    context = ctx;

    // Resolve any functions we will need based upon context version and create the texture
    initializeOpenGLFunctions();

    // What features do we have?
    QOpenGLTexture::Feature feature = QOpenGLTexture::ImmutableStorage;
    while (feature != QOpenGLTexture::MaxFeatureFlag) {
        if (QOpenGLTexture::hasFeature(feature))
            features |= feature;
        feature = static_cast<QOpenGLTexture::Feature>(feature << 1);
    }

    texFuncs->glGenTextures(1, &textureId);
    return textureId != 0;
}

void QOpenGLTexturePrivate::destroy()
{
    if (!textureId) {
        // not created or already destroyed
        return;
    }
    QOpenGLContext *currentContext = QOpenGLContext::currentContext();
    if (!currentContext || !QOpenGLContext::areSharing(currentContext, context)) {
        qWarning("Texture is not valid in the current context.\n"
                 "Texture has not been destroyed");
        return;
    }

    texFuncs->glDeleteTextures(1, &textureId);

    context   = nullptr;
    textureId = 0;
    format = QOpenGLTexture::NoFormat;
    formatClass = QOpenGLTexture::NoFormatClass;
    requestedMipLevels = 1;
    mipLevels = -1;
    layers = 1;
    faces = 1;
    samples = 0;
    fixedSamplePositions = true,
    baseLevel = 0;
    maxLevel = 1000;
    depthStencilMode = QOpenGLTexture::DepthMode;
    minFilter = QOpenGLTexture::Nearest;
    magFilter = QOpenGLTexture::Nearest;
    maxAnisotropy = 1.0f;
    minLevelOfDetail = -1000.0f;
    maxLevelOfDetail = 1000.0f;
    levelOfDetailBias = 0.0f;
    textureView = false;
    autoGenerateMipMaps = true;
    storageAllocated = false;
    texFuncs = nullptr;

    swizzleMask[0] = QOpenGLTexture::RedValue;
    swizzleMask[1] = QOpenGLTexture::GreenValue;
    swizzleMask[2] = QOpenGLTexture::BlueValue;
    swizzleMask[3] = QOpenGLTexture::AlphaValue;

    wrapModes[0] = wrapModes[1] = wrapModes[2] = target == QOpenGLTexture::TargetRectangle
        ? QOpenGLTexture::ClampToEdge : QOpenGLTexture::Repeat;
}

void QOpenGLTexturePrivate::bind()
{
    texFuncs->glBindTexture(target, textureId);
}

void QOpenGLTexturePrivate::bind(uint unit, QOpenGLTexture::TextureUnitReset reset)
{
    GLint oldTextureUnit = 0;
    if (reset == QOpenGLTexture::ResetTextureUnit)
        texFuncs->glGetIntegerv(GL_ACTIVE_TEXTURE, &oldTextureUnit);

    texFuncs->glActiveTexture(GL_TEXTURE0 + unit);
    texFuncs->glBindTexture(target, textureId);

    if (reset == QOpenGLTexture::ResetTextureUnit)
        texFuncs->glActiveTexture(GL_TEXTURE0 + oldTextureUnit);
}

void QOpenGLTexturePrivate::release()
{
    texFuncs->glBindTexture(target, 0);
}

void QOpenGLTexturePrivate::release(uint unit, QOpenGLTexture::TextureUnitReset reset)
{
    GLint oldTextureUnit = 0;
    if (reset == QOpenGLTexture::ResetTextureUnit)
        texFuncs->glGetIntegerv(GL_ACTIVE_TEXTURE, &oldTextureUnit);

    texFuncs->glActiveTexture(GL_TEXTURE0 + unit);
    texFuncs->glBindTexture(target, 0);

    if (reset == QOpenGLTexture::ResetTextureUnit)
        texFuncs->glActiveTexture(GL_TEXTURE0 + oldTextureUnit);
}

bool QOpenGLTexturePrivate::isBound() const
{
    GLint boundTextureId = 0;
    texFuncs->glGetIntegerv(bindingTarget, &boundTextureId);
    return (static_cast<GLuint>(boundTextureId) == textureId);
}

bool QOpenGLTexturePrivate::isBound(uint unit) const
{
    GLint oldTextureUnit = 0;
    texFuncs->glGetIntegerv(GL_ACTIVE_TEXTURE, &oldTextureUnit);

    GLint boundTextureId = 0;
    texFuncs->glActiveTexture(GL_TEXTURE0 + unit);
    texFuncs->glGetIntegerv(bindingTarget, &boundTextureId);
    bool result = (static_cast<GLuint>(boundTextureId) == textureId);

    texFuncs->glActiveTexture(GL_TEXTURE0 + oldTextureUnit);
    return result;
}

int QOpenGLTexturePrivate::evaluateMipLevels() const
{
    switch (target) {
    case QOpenGLTexture::Target1D:
    case QOpenGLTexture::Target1DArray:
    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::Target2DArray:
    case QOpenGLTexture::Target3D:
    case QOpenGLTexture::TargetCubeMap:
    case QOpenGLTexture::TargetCubeMapArray:
        return qMin(maximumMipLevelCount(), qMax(1, requestedMipLevels));

    case QOpenGLTexture::TargetRectangle:
    case QOpenGLTexture::Target2DMultisample:
    case QOpenGLTexture::Target2DMultisampleArray:
    case QOpenGLTexture::TargetBuffer:
    default:
        return 1;
    }
}

static bool isSizedTextureFormat(QOpenGLTexture::TextureFormat internalFormat)
{
    switch (internalFormat) {
    case QOpenGLTexture::NoFormat:
        return false;

    case QOpenGLTexture::R8_UNorm:
    case QOpenGLTexture::RG8_UNorm:
    case QOpenGLTexture::RGB8_UNorm:
    case QOpenGLTexture::RGBA8_UNorm:
    case QOpenGLTexture::R16_UNorm:
    case QOpenGLTexture::RG16_UNorm:
    case QOpenGLTexture::RGB16_UNorm:
    case QOpenGLTexture::RGBA16_UNorm:
    case QOpenGLTexture::R8_SNorm:
    case QOpenGLTexture::RG8_SNorm:
    case QOpenGLTexture::RGB8_SNorm:
    case QOpenGLTexture::RGBA8_SNorm:
    case QOpenGLTexture::R16_SNorm:
    case QOpenGLTexture::RG16_SNorm:
    case QOpenGLTexture::RGB16_SNorm:
    case QOpenGLTexture::RGBA16_SNorm:
    case QOpenGLTexture::R8U:
    case QOpenGLTexture::RG8U:
    case QOpenGLTexture::RGB8U:
    case QOpenGLTexture::RGBA8U:
    case QOpenGLTexture::R16U:
    case QOpenGLTexture::RG16U:
    case QOpenGLTexture::RGB16U:
    case QOpenGLTexture::RGBA16U:
    case QOpenGLTexture::R32U:
    case QOpenGLTexture::RG32U:
    case QOpenGLTexture::RGB32U:
    case QOpenGLTexture::RGBA32U:
    case QOpenGLTexture::R8I:
    case QOpenGLTexture::RG8I:
    case QOpenGLTexture::RGB8I:
    case QOpenGLTexture::RGBA8I:
    case QOpenGLTexture::R16I:
    case QOpenGLTexture::RG16I:
    case QOpenGLTexture::RGB16I:
    case QOpenGLTexture::RGBA16I:
    case QOpenGLTexture::R32I:
    case QOpenGLTexture::RG32I:
    case QOpenGLTexture::RGB32I:
    case QOpenGLTexture::RGBA32I:
    case QOpenGLTexture::R16F:
    case QOpenGLTexture::RG16F:
    case QOpenGLTexture::RGB16F:
    case QOpenGLTexture::RGBA16F:
    case QOpenGLTexture::R32F:
    case QOpenGLTexture::RG32F:
    case QOpenGLTexture::RGB32F:
    case QOpenGLTexture::RGBA32F:
    case QOpenGLTexture::RGB9E5:
    case QOpenGLTexture::RG11B10F:
    case QOpenGLTexture::RG3B2:
    case QOpenGLTexture::R5G6B5:
    case QOpenGLTexture::RGB5A1:
    case QOpenGLTexture::RGBA4:
    case QOpenGLTexture::RGB10A2:

    case QOpenGLTexture::D16:
    case QOpenGLTexture::D24:
    case QOpenGLTexture::D32:
    case QOpenGLTexture::D32F:

    case QOpenGLTexture::D24S8:
    case QOpenGLTexture::D32FS8X24:

    case QOpenGLTexture::S8:

    case QOpenGLTexture::RGB_DXT1:
    case QOpenGLTexture::RGBA_DXT1:
    case QOpenGLTexture::RGBA_DXT3:
    case QOpenGLTexture::RGBA_DXT5:
    case QOpenGLTexture::R_ATI1N_UNorm:
    case QOpenGLTexture::R_ATI1N_SNorm:
    case QOpenGLTexture::RG_ATI2N_UNorm:
    case QOpenGLTexture::RG_ATI2N_SNorm:
    case QOpenGLTexture::RGB_BP_UNSIGNED_FLOAT:
    case QOpenGLTexture::RGB_BP_SIGNED_FLOAT:
    case QOpenGLTexture::RGB_BP_UNorm:
    case QOpenGLTexture::SRGB8:
    case QOpenGLTexture::SRGB8_Alpha8:
    case QOpenGLTexture::SRGB_DXT1:
    case QOpenGLTexture::SRGB_Alpha_DXT1:
    case QOpenGLTexture::SRGB_Alpha_DXT3:
    case QOpenGLTexture::SRGB_Alpha_DXT5:
    case QOpenGLTexture::SRGB_BP_UNorm:
    case QOpenGLTexture::R11_EAC_UNorm:
    case QOpenGLTexture::R11_EAC_SNorm:
    case QOpenGLTexture::RG11_EAC_UNorm:
    case QOpenGLTexture::RG11_EAC_SNorm:
    case QOpenGLTexture::RGB8_ETC2:
    case QOpenGLTexture::SRGB8_ETC2:
    case QOpenGLTexture::RGB8_PunchThrough_Alpha1_ETC2:
    case QOpenGLTexture::SRGB8_PunchThrough_Alpha1_ETC2:
    case QOpenGLTexture::RGBA8_ETC2_EAC:
    case QOpenGLTexture::SRGB8_Alpha8_ETC2_EAC:
        return true;

    case QOpenGLTexture::RGB8_ETC1:
        return false;

    case QOpenGLTexture::DepthFormat:
    case QOpenGLTexture::AlphaFormat:

    case QOpenGLTexture::RGBFormat:
    case QOpenGLTexture::RGBAFormat:

    case QOpenGLTexture::LuminanceFormat:

    case QOpenGLTexture::LuminanceAlphaFormat:
        return false;
    }

    // error, may want to throw
    return false;
}

static bool isTextureTargetMultisample(QOpenGLTexture::Target target)
{
    switch (target) {
    case QOpenGLTexture::Target1D:
    case QOpenGLTexture::Target1DArray:
    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::Target2DArray:
    case QOpenGLTexture::Target3D:
    case QOpenGLTexture::TargetCubeMap:
    case QOpenGLTexture::TargetCubeMapArray:
        return false;

    case QOpenGLTexture::Target2DMultisample:
    case QOpenGLTexture::Target2DMultisampleArray:
        return true;

    case QOpenGLTexture::TargetRectangle:
    case QOpenGLTexture::TargetBuffer:
        return false;
    }

    // error, may want to throw

    return false;
}

bool QOpenGLTexturePrivate::isUsingImmutableStorage() const
{
    // Use immutable storage whenever possible, falling back to mutable
    // Note that if multisample textures are not supported at all, we'll still fail into
    // the mutable storage allocation
    return isSizedTextureFormat(format)
            && (isTextureTargetMultisample(target)
                ? features.testFlag(QOpenGLTexture::ImmutableMultisampleStorage)
                : features.testFlag(QOpenGLTexture::ImmutableStorage));
}

void QOpenGLTexturePrivate::allocateStorage(QOpenGLTexture::PixelFormat pixelFormat, QOpenGLTexture::PixelType pixelType)
{
    // Resolve the actual number of mipmap levels we can use
    mipLevels = evaluateMipLevels();

    if (isUsingImmutableStorage())
        allocateImmutableStorage();
    else
        allocateMutableStorage(pixelFormat, pixelType);
}

static QOpenGLTexture::PixelFormat pixelFormatCompatibleWithInternalFormat(QOpenGLTexture::TextureFormat internalFormat)
{
    switch (internalFormat) {
    case QOpenGLTexture::NoFormat:
        return QOpenGLTexture::NoSourceFormat;

    case QOpenGLTexture::R8_UNorm:
        return QOpenGLTexture::Red;

    case QOpenGLTexture::RG8_UNorm:
        return QOpenGLTexture::RG;

    case QOpenGLTexture::RGB8_UNorm:
        return QOpenGLTexture::RGB;

    case QOpenGLTexture::RGBA8_UNorm:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::R16_UNorm:
        return QOpenGLTexture::Red;

    case QOpenGLTexture::RG16_UNorm:
        return QOpenGLTexture::RG;

    case QOpenGLTexture::RGB16_UNorm:
        return QOpenGLTexture::RGB;

    case QOpenGLTexture::RGBA16_UNorm:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::R8_SNorm:
        return QOpenGLTexture::Red;

    case QOpenGLTexture::RG8_SNorm:
        return QOpenGLTexture::RG;

    case QOpenGLTexture::RGB8_SNorm:
        return QOpenGLTexture::RGB;

    case QOpenGLTexture::RGBA8_SNorm:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::R16_SNorm:
        return QOpenGLTexture::Red;

    case QOpenGLTexture::RG16_SNorm:
        return QOpenGLTexture::RG;

    case QOpenGLTexture::RGB16_SNorm:
        return QOpenGLTexture::RGB;

    case QOpenGLTexture::RGBA16_SNorm:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::R8U:
        return QOpenGLTexture::Red_Integer;

    case QOpenGLTexture::RG8U:
        return QOpenGLTexture::RG_Integer;

    case QOpenGLTexture::RGB8U:
        return QOpenGLTexture::RGB_Integer;

    case QOpenGLTexture::RGBA8U:
        return QOpenGLTexture::RGBA_Integer;

    case QOpenGLTexture::R16U:
        return QOpenGLTexture::Red_Integer;

    case QOpenGLTexture::RG16U:
        return QOpenGLTexture::RG_Integer;

    case QOpenGLTexture::RGB16U:
        return QOpenGLTexture::RGB_Integer;

    case QOpenGLTexture::RGBA16U:
        return QOpenGLTexture::RGBA_Integer;

    case QOpenGLTexture::R32U:
        return QOpenGLTexture::Red_Integer;

    case QOpenGLTexture::RG32U:
        return QOpenGLTexture::RG_Integer;

    case QOpenGLTexture::RGB32U:
        return QOpenGLTexture::RGB_Integer;

    case QOpenGLTexture::RGBA32U:
        return QOpenGLTexture::RGBA_Integer;

    case QOpenGLTexture::R8I:
        return QOpenGLTexture::Red_Integer;

    case QOpenGLTexture::RG8I:
        return QOpenGLTexture::RG_Integer;

    case QOpenGLTexture::RGB8I:
        return QOpenGLTexture::RGB_Integer;

    case QOpenGLTexture::RGBA8I:
        return QOpenGLTexture::RGBA_Integer;

    case QOpenGLTexture::R16I:
        return QOpenGLTexture::Red_Integer;

    case QOpenGLTexture::RG16I:
        return QOpenGLTexture::RG_Integer;

    case QOpenGLTexture::RGB16I:
        return QOpenGLTexture::RGB_Integer;

    case QOpenGLTexture::RGBA16I:
        return QOpenGLTexture::RGBA_Integer;

    case QOpenGLTexture::R32I:
        return QOpenGLTexture::Red_Integer;

    case QOpenGLTexture::RG32I:
        return QOpenGLTexture::RG_Integer;

    case QOpenGLTexture::RGB32I:
        return QOpenGLTexture::RGB_Integer;

    case QOpenGLTexture::RGBA32I:
        return QOpenGLTexture::RGBA_Integer;

    case QOpenGLTexture::R16F:
        return QOpenGLTexture::Red;

    case QOpenGLTexture::RG16F:
        return QOpenGLTexture::RG;

    case QOpenGLTexture::RGB16F:
        return QOpenGLTexture::RGB;

    case QOpenGLTexture::RGBA16F:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::R32F:
        return QOpenGLTexture::Red;

    case QOpenGLTexture::RG32F:
        return QOpenGLTexture::RG;

    case QOpenGLTexture::RGB32F:
        return QOpenGLTexture::RGB;

    case QOpenGLTexture::RGBA32F:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::RGB9E5:
        return QOpenGLTexture::RGB;

    case QOpenGLTexture::RG11B10F:
        return QOpenGLTexture::RGB;

    case QOpenGLTexture::RG3B2:
        return QOpenGLTexture::RGB;

    case QOpenGLTexture::R5G6B5:
        return QOpenGLTexture::RGB;

    case QOpenGLTexture::RGB5A1:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::RGBA4:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::RGB10A2:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::D16:
    case QOpenGLTexture::D24:
    case QOpenGLTexture::D32:
    case QOpenGLTexture::D32F:
        return QOpenGLTexture::Depth;

    case QOpenGLTexture::D24S8:
    case QOpenGLTexture::D32FS8X24:
        return QOpenGLTexture::DepthStencil;

    case QOpenGLTexture::S8:
        return QOpenGLTexture::Stencil;

    case QOpenGLTexture::RGB_DXT1:
    case QOpenGLTexture::RGBA_DXT1:
    case QOpenGLTexture::RGBA_DXT3:
    case QOpenGLTexture::RGBA_DXT5:
    case QOpenGLTexture::R_ATI1N_UNorm:
    case QOpenGLTexture::R_ATI1N_SNorm:
    case QOpenGLTexture::RG_ATI2N_UNorm:
    case QOpenGLTexture::RG_ATI2N_SNorm:
    case QOpenGLTexture::RGB_BP_UNSIGNED_FLOAT:
    case QOpenGLTexture::RGB_BP_SIGNED_FLOAT:
    case QOpenGLTexture::RGB_BP_UNorm:
    case QOpenGLTexture::SRGB8:
    case QOpenGLTexture::SRGB8_Alpha8:
    case QOpenGLTexture::SRGB_DXT1:
    case QOpenGLTexture::SRGB_Alpha_DXT1:
    case QOpenGLTexture::SRGB_Alpha_DXT3:
    case QOpenGLTexture::SRGB_Alpha_DXT5:
    case QOpenGLTexture::SRGB_BP_UNorm:
    case QOpenGLTexture::RGB8_ETC1:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::R11_EAC_UNorm:
    case QOpenGLTexture::R11_EAC_SNorm:
        return QOpenGLTexture::Red;

    case QOpenGLTexture::RG11_EAC_UNorm:
    case QOpenGLTexture::RG11_EAC_SNorm:
        return QOpenGLTexture::RG;

    case QOpenGLTexture::RGB8_ETC2:
    case QOpenGLTexture::SRGB8_ETC2:
        return QOpenGLTexture::RGB;

    case QOpenGLTexture::RGB8_PunchThrough_Alpha1_ETC2:
    case QOpenGLTexture::SRGB8_PunchThrough_Alpha1_ETC2:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::RGBA8_ETC2_EAC:
    case QOpenGLTexture::SRGB8_Alpha8_ETC2_EAC:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::DepthFormat:
        return QOpenGLTexture::Depth;

    case QOpenGLTexture::AlphaFormat:
        return QOpenGLTexture::Alpha;

    case QOpenGLTexture::RGBFormat:
    case QOpenGLTexture::RGBAFormat:
        return QOpenGLTexture::RGBA;

    case QOpenGLTexture::LuminanceFormat:
        return QOpenGLTexture::Luminance;

    case QOpenGLTexture::LuminanceAlphaFormat:
        return QOpenGLTexture::LuminanceAlpha;
    }

    // error, may want to throw

    return QOpenGLTexture::NoSourceFormat;
}

static QOpenGLTexture::PixelType pixelTypeCompatibleWithInternalFormat(QOpenGLTexture::TextureFormat internalFormat)
{
    switch (internalFormat) {
    case QOpenGLTexture::NoFormat:
        return QOpenGLTexture::NoPixelType;

    case QOpenGLTexture::R8_UNorm:
    case QOpenGLTexture::RG8_UNorm:
    case QOpenGLTexture::RGB8_UNorm:
    case QOpenGLTexture::RGBA8_UNorm:
    case QOpenGLTexture::R16_UNorm:
    case QOpenGLTexture::RG16_UNorm:
    case QOpenGLTexture::RGB16_UNorm:
    case QOpenGLTexture::RGBA16_UNorm:
        return QOpenGLTexture::UInt8;

    case QOpenGLTexture::R8_SNorm:
    case QOpenGLTexture::RG8_SNorm:
    case QOpenGLTexture::RGB8_SNorm:
    case QOpenGLTexture::RGBA8_SNorm:
    case QOpenGLTexture::R16_SNorm:
    case QOpenGLTexture::RG16_SNorm:
    case QOpenGLTexture::RGB16_SNorm:
    case QOpenGLTexture::RGBA16_SNorm:
        return QOpenGLTexture::Int8;

    case QOpenGLTexture::R8U:
    case QOpenGLTexture::RG8U:
    case QOpenGLTexture::RGB8U:
    case QOpenGLTexture::RGBA8U:
    case QOpenGLTexture::R16U:
    case QOpenGLTexture::RG16U:
    case QOpenGLTexture::RGB16U:
    case QOpenGLTexture::RGBA16U:
    case QOpenGLTexture::R32U:
    case QOpenGLTexture::RG32U:
    case QOpenGLTexture::RGB32U:
    case QOpenGLTexture::RGBA32U:
        return QOpenGLTexture::UInt8;

    case QOpenGLTexture::R8I:
    case QOpenGLTexture::RG8I:
    case QOpenGLTexture::RGB8I:
    case QOpenGLTexture::RGBA8I:
    case QOpenGLTexture::R16I:
    case QOpenGLTexture::RG16I:
    case QOpenGLTexture::RGB16I:
    case QOpenGLTexture::RGBA16I:
    case QOpenGLTexture::R32I:
    case QOpenGLTexture::RG32I:
    case QOpenGLTexture::RGB32I:
    case QOpenGLTexture::RGBA32I:
        return QOpenGLTexture::Int8;

    case QOpenGLTexture::R16F:
    case QOpenGLTexture::RG16F:
    case QOpenGLTexture::RGB16F:
    case QOpenGLTexture::RGBA16F:
        return QOpenGLTexture::Float16;

    case QOpenGLTexture::R32F:
    case QOpenGLTexture::RG32F:
    case QOpenGLTexture::RGB32F:
    case QOpenGLTexture::RGBA32F:
        return QOpenGLTexture::Float32;

    case QOpenGLTexture::RGB9E5:
        return QOpenGLTexture::UInt16_RGB5A1_Rev;

    case QOpenGLTexture::RG11B10F:
        return QOpenGLTexture::UInt32_RG11B10F;

    case QOpenGLTexture::RG3B2:
        return QOpenGLTexture::UInt8_RG3B2;

    case QOpenGLTexture::R5G6B5:
        return QOpenGLTexture::UInt16_R5G6B5;

    case QOpenGLTexture::RGB5A1:
        return QOpenGLTexture::UInt16_RGB5A1;

    case QOpenGLTexture::RGBA4:
        return QOpenGLTexture::UInt16_RGBA4;

    case QOpenGLTexture::RGB10A2:
        return QOpenGLTexture::UInt32_RGB10A2;

    case QOpenGLTexture::D16:
        return QOpenGLTexture::UInt16;

    case QOpenGLTexture::D24:
    case QOpenGLTexture::D32:
        return QOpenGLTexture::UInt32;

    case QOpenGLTexture::D32F:
        return QOpenGLTexture::Float32;

    case QOpenGLTexture::D24S8:
        return QOpenGLTexture::UInt32_D24S8;

    case QOpenGLTexture::D32FS8X24:
        return QOpenGLTexture::Float32_D32_UInt32_S8_X24;

    case QOpenGLTexture::S8:
        return QOpenGLTexture::UInt8;

    case QOpenGLTexture::RGB_DXT1:
    case QOpenGLTexture::RGBA_DXT1:
    case QOpenGLTexture::RGBA_DXT3:
    case QOpenGLTexture::RGBA_DXT5:
    case QOpenGLTexture::R_ATI1N_UNorm:
    case QOpenGLTexture::R_ATI1N_SNorm:
    case QOpenGLTexture::RG_ATI2N_UNorm:
    case QOpenGLTexture::RG_ATI2N_SNorm:
    case QOpenGLTexture::RGB_BP_UNSIGNED_FLOAT:
    case QOpenGLTexture::RGB_BP_SIGNED_FLOAT:
    case QOpenGLTexture::RGB_BP_UNorm:
    case QOpenGLTexture::SRGB8:
    case QOpenGLTexture::SRGB8_Alpha8:
    case QOpenGLTexture::SRGB_DXT1:
    case QOpenGLTexture::SRGB_Alpha_DXT1:
    case QOpenGLTexture::SRGB_Alpha_DXT3:
    case QOpenGLTexture::SRGB_Alpha_DXT5:
    case QOpenGLTexture::SRGB_BP_UNorm:
    case QOpenGLTexture::R11_EAC_UNorm:
    case QOpenGLTexture::R11_EAC_SNorm:
    case QOpenGLTexture::RG11_EAC_UNorm:
    case QOpenGLTexture::RG11_EAC_SNorm:
    case QOpenGLTexture::RGB8_ETC2:
    case QOpenGLTexture::SRGB8_ETC2:
    case QOpenGLTexture::RGB8_PunchThrough_Alpha1_ETC2:
    case QOpenGLTexture::SRGB8_PunchThrough_Alpha1_ETC2:
    case QOpenGLTexture::RGBA8_ETC2_EAC:
    case QOpenGLTexture::SRGB8_Alpha8_ETC2_EAC:
    case QOpenGLTexture::RGB8_ETC1:
        return QOpenGLTexture::UInt8;

    case QOpenGLTexture::DepthFormat:
        return QOpenGLTexture::UInt32;

    case QOpenGLTexture::AlphaFormat:
    case QOpenGLTexture::RGBFormat:
    case QOpenGLTexture::RGBAFormat:
    case QOpenGLTexture::LuminanceFormat:
    case QOpenGLTexture::LuminanceAlphaFormat:
        return QOpenGLTexture::UInt8;
    }

    // error, may want to throw

    return QOpenGLTexture::NoPixelType;
}

static bool isCompressedFormat(QOpenGLTexture::TextureFormat internalFormat)
{
    switch (internalFormat) {
    case QOpenGLTexture::NoFormat:

    case QOpenGLTexture::R8_UNorm:
    case QOpenGLTexture::RG8_UNorm:
    case QOpenGLTexture::RGB8_UNorm:
    case QOpenGLTexture::RGBA8_UNorm:
    case QOpenGLTexture::R16_UNorm:
    case QOpenGLTexture::RG16_UNorm:
    case QOpenGLTexture::RGB16_UNorm:
    case QOpenGLTexture::RGBA16_UNorm:
    case QOpenGLTexture::R8_SNorm:
    case QOpenGLTexture::RG8_SNorm:
    case QOpenGLTexture::RGB8_SNorm:
    case QOpenGLTexture::RGBA8_SNorm:
    case QOpenGLTexture::R16_SNorm:
    case QOpenGLTexture::RG16_SNorm:
    case QOpenGLTexture::RGB16_SNorm:
    case QOpenGLTexture::RGBA16_SNorm:
    case QOpenGLTexture::R8U:
    case QOpenGLTexture::RG8U:
    case QOpenGLTexture::RGB8U:
    case QOpenGLTexture::RGBA8U:
    case QOpenGLTexture::R16U:
    case QOpenGLTexture::RG16U:
    case QOpenGLTexture::RGB16U:
    case QOpenGLTexture::RGBA16U:
    case QOpenGLTexture::R32U:
    case QOpenGLTexture::RG32U:
    case QOpenGLTexture::RGB32U:
    case QOpenGLTexture::RGBA32U:
    case QOpenGLTexture::R8I:
    case QOpenGLTexture::RG8I:
    case QOpenGLTexture::RGB8I:
    case QOpenGLTexture::RGBA8I:
    case QOpenGLTexture::R16I:
    case QOpenGLTexture::RG16I:
    case QOpenGLTexture::RGB16I:
    case QOpenGLTexture::RGBA16I:
    case QOpenGLTexture::R32I:
    case QOpenGLTexture::RG32I:
    case QOpenGLTexture::RGB32I:
    case QOpenGLTexture::RGBA32I:
    case QOpenGLTexture::R16F:
    case QOpenGLTexture::RG16F:
    case QOpenGLTexture::RGB16F:
    case QOpenGLTexture::RGBA16F:
    case QOpenGLTexture::R32F:
    case QOpenGLTexture::RG32F:
    case QOpenGLTexture::RGB32F:
    case QOpenGLTexture::RGBA32F:
    case QOpenGLTexture::RGB9E5:
    case QOpenGLTexture::RG11B10F:
    case QOpenGLTexture::RG3B2:
    case QOpenGLTexture::R5G6B5:
    case QOpenGLTexture::RGB5A1:
    case QOpenGLTexture::RGBA4:
    case QOpenGLTexture::RGB10A2:

    case QOpenGLTexture::D16:
    case QOpenGLTexture::D24:
    case QOpenGLTexture::D32:
    case QOpenGLTexture::D32F:

    case QOpenGLTexture::D24S8:
    case QOpenGLTexture::D32FS8X24:

    case QOpenGLTexture::S8:
        return false;

    case QOpenGLTexture::RGB_DXT1:
    case QOpenGLTexture::RGBA_DXT1:
    case QOpenGLTexture::RGBA_DXT3:
    case QOpenGLTexture::RGBA_DXT5:
    case QOpenGLTexture::R_ATI1N_UNorm:
    case QOpenGLTexture::R_ATI1N_SNorm:
    case QOpenGLTexture::RG_ATI2N_UNorm:
    case QOpenGLTexture::RG_ATI2N_SNorm:
    case QOpenGLTexture::RGB_BP_UNSIGNED_FLOAT:
    case QOpenGLTexture::RGB_BP_SIGNED_FLOAT:
    case QOpenGLTexture::RGB_BP_UNorm:
    case QOpenGLTexture::SRGB8:
    case QOpenGLTexture::SRGB8_Alpha8:
    case QOpenGLTexture::SRGB_DXT1:
    case QOpenGLTexture::SRGB_Alpha_DXT1:
    case QOpenGLTexture::SRGB_Alpha_DXT3:
    case QOpenGLTexture::SRGB_Alpha_DXT5:
    case QOpenGLTexture::SRGB_BP_UNorm:
    case QOpenGLTexture::R11_EAC_UNorm:
    case QOpenGLTexture::R11_EAC_SNorm:
    case QOpenGLTexture::RG11_EAC_UNorm:
    case QOpenGLTexture::RG11_EAC_SNorm:
    case QOpenGLTexture::RGB8_ETC2:
    case QOpenGLTexture::SRGB8_ETC2:
    case QOpenGLTexture::RGB8_PunchThrough_Alpha1_ETC2:
    case QOpenGLTexture::SRGB8_PunchThrough_Alpha1_ETC2:
    case QOpenGLTexture::RGBA8_ETC2_EAC:
    case QOpenGLTexture::SRGB8_Alpha8_ETC2_EAC:
    case QOpenGLTexture::RGB8_ETC1:
        return true;

    case QOpenGLTexture::DepthFormat:
    case QOpenGLTexture::AlphaFormat:
    case QOpenGLTexture::RGBFormat:
    case QOpenGLTexture::RGBAFormat:
    case QOpenGLTexture::LuminanceFormat:
    case QOpenGLTexture::LuminanceAlphaFormat:
        return false;
    }

    // error, may want to throw

    return false;
}

void QOpenGLTexturePrivate::allocateMutableStorage(QOpenGLTexture::PixelFormat pixelFormat, QOpenGLTexture::PixelType pixelType)
{
    // There is no way to allocate mutable storage for compressed textures in in
    // versions older than OpenGL 3.1 and OpenGL ES 3.0, because the older specs
    // do not mandate accepting null data pointers for glCompressedTexImage*D,
    // unlike glTexImage*D (which in turn does not accept compressed formats).
    if (isCompressedFormat(format)) {
        storageAllocated = true;
        return;
    }

    switch (target) {
    case QOpenGLTexture::TargetBuffer:
        // Buffer textures get their storage from an external OpenGL buffer
        qWarning("Buffer textures do not allocate storage");
        return;

    case QOpenGLTexture::Target1D:
        if (features.testFlag(QOpenGLTexture::Texture1D)) {
            for (int level = 0; level < mipLevels; ++level)
                texFuncs->glTextureImage1D(textureId, target, bindingTarget, level, format,
                      mipLevelSize(level, dimensions[0]), 0, pixelFormat, pixelType, nullptr);

        } else {
            qWarning("1D textures are not supported");
            return;
        }
        break;

    case QOpenGLTexture::Target1DArray:
        if (features.testFlag(QOpenGLTexture::Texture1D)
                && features.testFlag(QOpenGLTexture::TextureArrays)) {
            for (int level = 0; level < mipLevels; ++level)
                texFuncs->glTextureImage2D(textureId, target, bindingTarget, level, format,
                      mipLevelSize(level, dimensions[0]), layers, 0, pixelFormat, pixelType, nullptr);

        } else {
            qWarning("1D array textures are not supported");
            return;
        }
        break;

    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::TargetRectangle:
        for (int level = 0; level < mipLevels; ++level)
            texFuncs->glTextureImage2D(textureId, target, bindingTarget, level, format,
                      mipLevelSize(level, dimensions[0]), mipLevelSize(level, dimensions[1]), 0,
                      pixelFormat, pixelType, nullptr);
        break;

    case QOpenGLTexture::TargetCubeMap: {
        // Cubemaps are the odd one out. We have to allocate storage for each
        // face and miplevel using the special cubemap face targets rather than
        // GL_TARGET_CUBEMAP.
        const QOpenGLTexture::CubeMapFace faceTargets[] = {
            QOpenGLTexture::CubeMapPositiveX, QOpenGLTexture::CubeMapNegativeX,
            QOpenGLTexture::CubeMapPositiveY, QOpenGLTexture::CubeMapNegativeY,
            QOpenGLTexture::CubeMapPositiveZ, QOpenGLTexture::CubeMapNegativeZ
        };

        for (int faceTarget = 0; faceTarget < 6; ++faceTarget) {
            for (int level = 0; level < mipLevels; ++level) {
                texFuncs->glTextureImage2D(textureId, faceTargets[faceTarget], bindingTarget,
                      level, format, mipLevelSize(level, dimensions[0]), mipLevelSize(level, dimensions[1]), 0,
                      pixelFormat, pixelType, nullptr);
            }
        }
        break;
    }

    case QOpenGLTexture::Target2DArray:
        if (features.testFlag(QOpenGLTexture::TextureArrays)) {
            for (int level = 0; level < mipLevels; ++level)
                texFuncs->glTextureImage3D(textureId, target, bindingTarget, level, format,
                      mipLevelSize(level, dimensions[0]), mipLevelSize(level, dimensions[1]), layers, 0,
                      pixelFormat, pixelType, nullptr);
        } else {
            qWarning("Array textures are not supported");
            return;
        }
        break;

    case QOpenGLTexture::TargetCubeMapArray:
        // Cubemap arrays must specify number of layer-faces (6 * layers) as depth parameter
        if (features.testFlag(QOpenGLTexture::TextureCubeMapArrays)) {
            for (int level = 0; level < mipLevels; ++level)
                texFuncs->glTextureImage3D(textureId, target, bindingTarget, level, format,
                      mipLevelSize(level, dimensions[0]), mipLevelSize(level, dimensions[1]),
                      6 * layers, 0, pixelFormat, pixelType, nullptr);

        } else {
            qWarning("Cubemap Array textures are not supported");
            return;
        }
        break;

    case QOpenGLTexture::Target3D:
        if (features.testFlag(QOpenGLTexture::Texture3D)) {
            for (int level = 0; level < mipLevels; ++level)
                texFuncs->glTextureImage3D(textureId, target, bindingTarget, level, format,
                      mipLevelSize(level, dimensions[0]), mipLevelSize(level, dimensions[1]),
                      mipLevelSize(level, dimensions[2]), 0, pixelFormat, pixelType, nullptr);

        } else {
            qWarning("3D textures are not supported");
            return;
        }
        break;

    case QOpenGLTexture::Target2DMultisample:
        if (features.testFlag(QOpenGLTexture::TextureMultisample)) {
            texFuncs->glTextureImage2DMultisample(textureId, target, bindingTarget, samples, format,
                                                  dimensions[0], dimensions[1],
                                                  fixedSamplePositions);
        } else {
            qWarning("Multisample textures are not supported");
            return;
        }
        break;

    case QOpenGLTexture::Target2DMultisampleArray:
        if (features.testFlag(QOpenGLTexture::TextureMultisample)
                && features.testFlag(QOpenGLTexture::TextureArrays)) {
            texFuncs->glTextureImage3DMultisample(textureId, target, bindingTarget, samples, format,
                                                  dimensions[0], dimensions[1], layers,
                                                  fixedSamplePositions);
        } else {
            qWarning("Multisample array textures are not supported");
            return;
        }
        break;
    }

    storageAllocated = true;
}

void QOpenGLTexturePrivate::allocateImmutableStorage()
{
    switch (target) {
    case QOpenGLTexture::TargetBuffer:
        // Buffer textures get their storage from an external OpenGL buffer
        qWarning("Buffer textures do not allocate storage");
        return;

    case QOpenGLTexture::Target1D:
        if (features.testFlag(QOpenGLTexture::Texture1D)) {
            texFuncs->glTextureStorage1D(textureId, target, bindingTarget, mipLevels, format,
                                         dimensions[0]);
        } else {
            qWarning("1D textures are not supported");
            return;
        }
        break;

    case QOpenGLTexture::Target1DArray:
        if (features.testFlag(QOpenGLTexture::Texture1D)
                && features.testFlag(QOpenGLTexture::TextureArrays)) {
            texFuncs->glTextureStorage2D(textureId, target, bindingTarget, mipLevels, format,
                                         dimensions[0], layers);
        } else {
            qWarning("1D array textures are not supported");
            return;
        }
        break;

    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::TargetCubeMap:
    case QOpenGLTexture::TargetRectangle:
        texFuncs->glTextureStorage2D(textureId, target, bindingTarget, mipLevels, format,
                                     dimensions[0], dimensions[1]);
        break;

    case QOpenGLTexture::Target2DArray:
        if (features.testFlag(QOpenGLTexture::TextureArrays)) {
            texFuncs->glTextureStorage3D(textureId, target, bindingTarget, mipLevels, format,
                                         dimensions[0], dimensions[1], layers);
        } else {
            qWarning("Array textures are not supported");
            return;
        }
        break;

    case QOpenGLTexture::TargetCubeMapArray:
        // Cubemap arrays must specify number of layer-faces (6 * layers) as depth parameter
        if (features.testFlag(QOpenGLTexture::TextureCubeMapArrays)) {
            texFuncs->glTextureStorage3D(textureId, target, bindingTarget, mipLevels, format,
                                         dimensions[0], dimensions[1], 6 * layers);
        } else {
            qWarning("Cubemap Array textures are not supported");
            return;
        }
        break;

    case QOpenGLTexture::Target3D:
        if (features.testFlag(QOpenGLTexture::Texture3D)) {
            texFuncs->glTextureStorage3D(textureId, target, bindingTarget, mipLevels, format,
                                         dimensions[0], dimensions[1], dimensions[2]);
        } else {
            qWarning("3D textures are not supported");
            return;
        }
        break;

    case QOpenGLTexture::Target2DMultisample:
        if (features.testFlag(QOpenGLTexture::ImmutableMultisampleStorage)) {
            texFuncs->glTextureStorage2DMultisample(textureId, target, bindingTarget, samples, format,
                                                    dimensions[0], dimensions[1],
                                                    fixedSamplePositions);
        } else {
            qWarning("Multisample textures are not supported");
            return;
        }
        break;

    case QOpenGLTexture::Target2DMultisampleArray:
        if (features.testFlag(QOpenGLTexture::ImmutableMultisampleStorage)
                && features.testFlag(QOpenGLTexture::TextureArrays)) {
            texFuncs->glTextureStorage3DMultisample(textureId, target, bindingTarget, samples, format,
                                                    dimensions[0], dimensions[1], layers,
                                                    fixedSamplePositions);
        } else {
            qWarning("Multisample array textures are not supported");
            return;
        }
        break;
    }

    storageAllocated = true;
}

void QOpenGLTexturePrivate::setData(int mipLevel, int layer, QOpenGLTexture::CubeMapFace cubeFace,
                                    QOpenGLTexture::PixelFormat sourceFormat, QOpenGLTexture::PixelType sourceType,
                                    const void *data, const QOpenGLPixelTransferOptions * const options)
{
    switch (target) {
    case QOpenGLTexture::Target1D:
        (void) layer;
        (void) cubeFace;
        texFuncs->glTextureSubImage1D(textureId, target, bindingTarget, mipLevel,
                                      0, mipLevelSize( mipLevel, dimensions[0] ),
                                      sourceFormat, sourceType, data, options);
        break;

    case QOpenGLTexture::Target1DArray:
        (void) cubeFace;
        texFuncs->glTextureSubImage2D(textureId, target, bindingTarget, mipLevel,
                                      0, layer,
                                      mipLevelSize(mipLevel, dimensions[0]),
                                      1,
                                      sourceFormat, sourceType, data, options);
        break;

    case QOpenGLTexture::Target2D:
        (void) layer;
        (void) cubeFace;
        texFuncs->glTextureSubImage2D(textureId, target, bindingTarget, mipLevel,
                                      0, 0,
                                      mipLevelSize(mipLevel, dimensions[0]),
                                      mipLevelSize(mipLevel, dimensions[1]),
                                      sourceFormat, sourceType, data, options);
        break;

    case QOpenGLTexture::Target2DArray:
        (void) cubeFace;
        texFuncs->glTextureSubImage3D(textureId, target, bindingTarget, mipLevel,
                                      0, 0, layer,
                                      mipLevelSize(mipLevel, dimensions[0]),
                                      mipLevelSize(mipLevel, dimensions[1]),
                                      1,
                                      sourceFormat, sourceType, data, options);
        break;

    case QOpenGLTexture::Target3D:
        (void) cubeFace;
        texFuncs->glTextureSubImage3D(textureId, target, bindingTarget, mipLevel,
                                      0, 0, layer,
                                      mipLevelSize(mipLevel, dimensions[0]),
                                      mipLevelSize(mipLevel, dimensions[1]),
                                      mipLevelSize(mipLevel, dimensions[2]),
                                      sourceFormat, sourceType, data, options);
        break;

    case QOpenGLTexture::TargetCubeMap:
        (void) layer;
        texFuncs->glTextureSubImage2D(textureId, cubeFace, bindingTarget, mipLevel,
                                      0, 0,
                                      mipLevelSize(mipLevel, dimensions[0]),
                                      mipLevelSize(mipLevel, dimensions[1]),
                                      sourceFormat, sourceType, data, options);
        break;

    case QOpenGLTexture::TargetCubeMapArray: {
        int faceIndex = cubeFace - QOpenGLTexture::CubeMapPositiveX;
        int layerFace = 6 * layer + faceIndex;
        texFuncs->glTextureSubImage3D(textureId, target, bindingTarget, mipLevel,
                                      0, 0, layerFace,
                                      mipLevelSize(mipLevel, dimensions[0]),
                                      mipLevelSize(mipLevel, dimensions[1]),
                                      1,
                                      sourceFormat, sourceType, data, options);
        break;
    }

    case QOpenGLTexture::TargetRectangle:
        (void) mipLevel;
        (void) layer;
        (void) cubeFace;
        texFuncs->glTextureSubImage2D(textureId, target, bindingTarget, 0,
                                      0, 0,
                                      dimensions[0],
                                      dimensions[1],
                                      sourceFormat, sourceType, data, options);
        break;

    case QOpenGLTexture::Target2DMultisample:
    case QOpenGLTexture::Target2DMultisampleArray:
    case QOpenGLTexture::TargetBuffer:
        // We don't upload pixel data for these targets
        qWarning("QOpenGLTexture::setData(): Texture target does not support pixel data upload");
        break;
    }

    // If requested perform automatic mip map generation
    if (mipLevel == 0 && autoGenerateMipMaps && mipLevels > 1) {
        Q_Q(QOpenGLTexture);
        q->generateMipMaps();
    }
}

void QOpenGLTexturePrivate::setCompressedData(int mipLevel, int layer, QOpenGLTexture::CubeMapFace cubeFace,
                                              int dataSize, const void *data,
                                              const QOpenGLPixelTransferOptions * const options)
{
    if (!isCompressedFormat(format)) {
        qWarning("Cannot set compressed data for non-compressed format 0x%x", format);
        return;
    }

    const bool needsFullSpec = !isUsingImmutableStorage(); // was allocateStorage() a no-op?

    switch (target) {
    case QOpenGLTexture::Target1D:
        (void) layer;
        (void) cubeFace;

        if (needsFullSpec) {
            texFuncs->glCompressedTextureImage1D(textureId, target, bindingTarget, mipLevel,
                                                 format,
                                                 mipLevelSize(mipLevel, dimensions[0]),
                                                 0, dataSize, data, options);
        } else {
            texFuncs->glCompressedTextureSubImage1D(textureId, target, bindingTarget, mipLevel,
                                                    0, mipLevelSize( mipLevel, dimensions[0] ),
                                                    format, dataSize, data, options);
        }
        break;

    case QOpenGLTexture::Target1DArray:
        (void) cubeFace;

        if (!needsFullSpec) {
            texFuncs->glCompressedTextureSubImage2D(textureId, target, bindingTarget, mipLevel,
                                                    0, layer,
                                                    mipLevelSize(mipLevel, dimensions[0]),
                                                    1,
                                                    format, dataSize, data, options);
        }
        break;

    case QOpenGLTexture::Target2D:
        (void) layer;
        (void) cubeFace;

        if (needsFullSpec) {
            texFuncs->glCompressedTextureImage2D(textureId, target, bindingTarget, mipLevel,
                                                 format,
                                                 mipLevelSize(mipLevel, dimensions[0]),
                                                 mipLevelSize(mipLevel, dimensions[1]),
                                                 0, dataSize, data, options);
        } else {
            texFuncs->glCompressedTextureSubImage2D(textureId, target, bindingTarget, mipLevel,
                                                    0, 0,
                                                    mipLevelSize(mipLevel, dimensions[0]),
                                                    mipLevelSize(mipLevel, dimensions[1]),
                                                    format, dataSize, data, options);
        }
        break;

    case QOpenGLTexture::Target2DArray:
        (void) cubeFace;

        if (!needsFullSpec) {
            texFuncs->glCompressedTextureSubImage3D(textureId, target, bindingTarget, mipLevel,
                                                    0, 0, layer,
                                                    mipLevelSize(mipLevel, dimensions[0]),
                                                    mipLevelSize(mipLevel, dimensions[1]),
                                                    1,
                                                    format, dataSize, data, options);
        }
        break;

    case QOpenGLTexture::Target3D:
        (void) cubeFace;

        if (needsFullSpec) {
            texFuncs->glCompressedTextureImage3D(textureId, target, bindingTarget, mipLevel,
                                                 format,
                                                 mipLevelSize(mipLevel, dimensions[0]),
                                                 mipLevelSize(mipLevel, dimensions[1]),
                                                 mipLevelSize(mipLevel, dimensions[2]),
                                                 0, dataSize, data, options);
        } else {
            texFuncs->glCompressedTextureSubImage3D(textureId, target, bindingTarget, mipLevel,
                                                    0, 0, layer,
                                                    mipLevelSize(mipLevel, dimensions[0]),
                                                    mipLevelSize(mipLevel, dimensions[1]),
                                                    mipLevelSize(mipLevel, dimensions[2]),
                                                    format, dataSize, data, options);
        }
        break;

    case QOpenGLTexture::TargetCubeMap:
        (void) layer;

        if (needsFullSpec) {
            texFuncs->glCompressedTextureImage2D(textureId, cubeFace, bindingTarget, mipLevel,
                                                 format,
                                                 mipLevelSize(mipLevel, dimensions[0]),
                                                 mipLevelSize(mipLevel, dimensions[1]),
                                                 0, dataSize, data, options);
        } else {
            texFuncs->glCompressedTextureSubImage2D(textureId, cubeFace, bindingTarget, mipLevel,
                                                    0, 0,
                                                    mipLevelSize(mipLevel, dimensions[0]),
                                                    mipLevelSize(mipLevel, dimensions[1]),
                                                    format, dataSize, data, options);
        }
        break;

    case QOpenGLTexture::TargetCubeMapArray: {
        int faceIndex = cubeFace - QOpenGLTexture::CubeMapPositiveX;
        int layerFace = 6 * layer + faceIndex;
        if (!needsFullSpec) {
            texFuncs->glCompressedTextureSubImage3D(textureId, target, bindingTarget, mipLevel,
                                                    0, 0, layerFace,
                                                    mipLevelSize(mipLevel, dimensions[0]),
                                                    mipLevelSize(mipLevel, dimensions[1]),
                                                    1,
                                                    format, dataSize, data, options);
        }
        break;
    }

    case QOpenGLTexture::TargetRectangle:
    case QOpenGLTexture::Target2DMultisample:
    case QOpenGLTexture::Target2DMultisampleArray:
    case QOpenGLTexture::TargetBuffer:
        // We don't upload pixel data for these targets
        qWarning("QOpenGLTexture::setCompressedData(): Texture target does not support pixel data upload");
        break;
    }

    // If requested perform automatic mip map generation
    if (mipLevel == 0 && autoGenerateMipMaps && mipLevels > 1) {
        Q_Q(QOpenGLTexture);
        q->generateMipMaps();
    }
}

void QOpenGLTexturePrivate::setWrapMode(QOpenGLTexture::WrapMode mode)
{
    switch (target) {
    case QOpenGLTexture::Target1D:
    case QOpenGLTexture::Target1DArray:
    case QOpenGLTexture::TargetBuffer:
        wrapModes[0] = mode;
        texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_S, mode);
        break;

    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::Target2DArray:
    case QOpenGLTexture::TargetCubeMap:
    case QOpenGLTexture::TargetCubeMapArray:
    case QOpenGLTexture::Target2DMultisample:
    case QOpenGLTexture::Target2DMultisampleArray:
    case QOpenGLTexture::TargetRectangle:
        wrapModes[0] = wrapModes[1] = mode;
        texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_S, mode);
        texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_T, mode);
        break;

    case QOpenGLTexture::Target3D:
        wrapModes[0] = wrapModes[1] = wrapModes[2] = mode;
        texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_S, mode);
        texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_T, mode);
        texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_R, mode);
        break;
    }
}

void QOpenGLTexturePrivate::setWrapMode(QOpenGLTexture::CoordinateDirection direction, QOpenGLTexture::WrapMode mode)
{
    switch (target) {
    case QOpenGLTexture::Target1D:
    case QOpenGLTexture::Target1DArray:
    case QOpenGLTexture::TargetBuffer:
        switch (direction) {
        case QOpenGLTexture::DirectionS:
            wrapModes[0] = mode;
            texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_S, mode);
            break;

        case QOpenGLTexture::DirectionT:
        case QOpenGLTexture::DirectionR:
            qWarning("QOpenGLTexture::setWrapMode() direction not valid for this texture target");
            break;
        }
        break;

    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::Target2DArray:
    case QOpenGLTexture::TargetCubeMap:
    case QOpenGLTexture::TargetCubeMapArray:
    case QOpenGLTexture::Target2DMultisample:
    case QOpenGLTexture::Target2DMultisampleArray:
    case QOpenGLTexture::TargetRectangle:
        switch (direction) {
        case QOpenGLTexture::DirectionS:
            wrapModes[0] = mode;
            texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_S, mode);
            break;

        case QOpenGLTexture::DirectionT:
            wrapModes[1] = mode;
            texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_T, mode);
            break;

        case QOpenGLTexture::DirectionR:
            qWarning("QOpenGLTexture::setWrapMode() direction not valid for this texture target");
            break;
        }
        break;

    case QOpenGLTexture::Target3D:
        switch (direction) {
        case QOpenGLTexture::DirectionS:
            wrapModes[0] = mode;
            texFuncs->glTextureParameteri(textureId, target, bindingTarget, direction, mode);
            break;

        case QOpenGLTexture::DirectionT:
            wrapModes[1] = mode;
            texFuncs->glTextureParameteri(textureId, target, bindingTarget, direction, mode);
            break;

        case QOpenGLTexture::DirectionR:
            wrapModes[2] = mode;
            texFuncs->glTextureParameteri(textureId, target, bindingTarget, direction, mode);
            break;
        }
        break;
    }
}

QOpenGLTexture::WrapMode QOpenGLTexturePrivate::wrapMode(QOpenGLTexture::CoordinateDirection direction) const
{
    switch (target) {
    case QOpenGLTexture::Target1D:
    case QOpenGLTexture::Target1DArray:
    case QOpenGLTexture::TargetBuffer:
        switch (direction) {
        case QOpenGLTexture::DirectionS:
            return wrapModes[0];

        case QOpenGLTexture::DirectionT:
        case QOpenGLTexture::DirectionR:
            qWarning("QOpenGLTexture::wrapMode() direction not valid for this texture target");
            return QOpenGLTexture::Repeat;
        }
        break;

    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::Target2DArray:
    case QOpenGLTexture::TargetCubeMap:
    case QOpenGLTexture::TargetCubeMapArray:
    case QOpenGLTexture::Target2DMultisample:
    case QOpenGLTexture::Target2DMultisampleArray:
    case QOpenGLTexture::TargetRectangle:
        switch (direction) {
        case QOpenGLTexture::DirectionS:
            return wrapModes[0];

        case QOpenGLTexture::DirectionT:
            return wrapModes[1];

        case QOpenGLTexture::DirectionR:
            qWarning("QOpenGLTexture::wrapMode() direction not valid for this texture target");
            return QOpenGLTexture::Repeat;
        }
        break;

    case QOpenGLTexture::Target3D:
        switch (direction) {
        case QOpenGLTexture::DirectionS:
            return wrapModes[0];

        case QOpenGLTexture::DirectionT:
            return wrapModes[1];

        case QOpenGLTexture::DirectionR:
            return wrapModes[2];
        }
        break;
    }
    // Should never get here
    Q_ASSERT(false);
    return QOpenGLTexture::Repeat;
}

QOpenGLTexture *QOpenGLTexturePrivate::createTextureView(QOpenGLTexture::Target viewTarget,
                                                         QOpenGLTexture::TextureFormat viewFormat,
                                                         int minimumMipmapLevel, int maximumMipmapLevel,
                                                         int minimumLayer, int maximumLayer) const
{
    // Do sanity checks - see http://www.opengl.org/wiki/GLAPI/glTextureView

    // Check the targets are compatible
    bool viewTargetCompatible = false;
    switch (target) {
    case QOpenGLTexture::Target1D:
    case QOpenGLTexture::Target1DArray:
        viewTargetCompatible = (viewTarget == QOpenGLTexture::Target1D
                             || viewTarget == QOpenGLTexture::Target1DArray);
        break;


    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::Target2DArray:
        viewTargetCompatible = (viewTarget == QOpenGLTexture::Target2D
                             || viewTarget == QOpenGLTexture::Target2DArray);
        break;

    case QOpenGLTexture::Target3D:
        viewTargetCompatible = (viewTarget == QOpenGLTexture::Target3D);
        break;

    case QOpenGLTexture::TargetCubeMap:
    case QOpenGLTexture::TargetCubeMapArray:
        viewTargetCompatible = (viewTarget == QOpenGLTexture::TargetCubeMap
                             || viewTarget == QOpenGLTexture::Target2D
                             || viewTarget == QOpenGLTexture::Target2DArray
                             || viewTarget == QOpenGLTexture::TargetCubeMapArray);
        break;

    case QOpenGLTexture::Target2DMultisample:
    case QOpenGLTexture::Target2DMultisampleArray:
        viewTargetCompatible = (viewTarget == QOpenGLTexture::Target2DMultisample
                             || viewTarget == QOpenGLTexture::Target2DMultisampleArray);
        break;

    case QOpenGLTexture::TargetRectangle:
        viewTargetCompatible = (viewTarget == QOpenGLTexture::TargetRectangle);
        break;

    case QOpenGLTexture::TargetBuffer:
        // Cannot be used with texture views
        break;
    }

    if (!viewTargetCompatible) {
        qWarning("QOpenGLTexture::createTextureView(): Incompatible source and view targets");
        return nullptr;
    }

    // Check the formats are compatible
    bool viewFormatCompatible = false;
    switch (formatClass) {
    case QOpenGLTexture::NoFormatClass:
        break;

    case QOpenGLTexture::FormatClass_128Bit:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RGBA32F
                             || viewFormat == QOpenGLTexture::RGBA32U
                             || viewFormat == QOpenGLTexture::RGBA32I);
        break;

    case QOpenGLTexture::FormatClass_96Bit:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RGB32F
                             || viewFormat == QOpenGLTexture::RGB32U
                             || viewFormat == QOpenGLTexture::RGB32I);
        break;

    case QOpenGLTexture::FormatClass_64Bit:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RGBA16F
                             || viewFormat == QOpenGLTexture::RG32F
                             || viewFormat == QOpenGLTexture::RGBA16U
                             || viewFormat == QOpenGLTexture::RG32U
                             || viewFormat == QOpenGLTexture::RGBA16I
                             || viewFormat == QOpenGLTexture::RG32I
                             || viewFormat == QOpenGLTexture::RGBA16_UNorm
                             || viewFormat == QOpenGLTexture::RGBA16_SNorm);
        break;

    case QOpenGLTexture::FormatClass_48Bit:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RGB16_UNorm
                             || viewFormat == QOpenGLTexture::RGB16_SNorm
                             || viewFormat == QOpenGLTexture::RGB16F
                             || viewFormat == QOpenGLTexture::RGB16U
                             || viewFormat == QOpenGLTexture::RGB16I);
        break;

    case QOpenGLTexture::FormatClass_32Bit:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RG16F
                             || viewFormat == QOpenGLTexture::RG11B10F
                             || viewFormat == QOpenGLTexture::R32F
                             || viewFormat == QOpenGLTexture::RGB10A2
                             || viewFormat == QOpenGLTexture::RGBA8U
                             || viewFormat == QOpenGLTexture::RG16U
                             || viewFormat == QOpenGLTexture::R32U
                             || viewFormat == QOpenGLTexture::RGBA8I
                             || viewFormat == QOpenGLTexture::RG16I
                             || viewFormat == QOpenGLTexture::R32I
                             || viewFormat == QOpenGLTexture::RGBA8_UNorm
                             || viewFormat == QOpenGLTexture::RG16_UNorm
                             || viewFormat == QOpenGLTexture::RGBA8_SNorm
                             || viewFormat == QOpenGLTexture::RG16_SNorm
                             || viewFormat == QOpenGLTexture::SRGB8_Alpha8
                             || viewFormat == QOpenGLTexture::RGB9E5);
        break;

    case QOpenGLTexture::FormatClass_24Bit:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RGB8_UNorm
                             || viewFormat == QOpenGLTexture::RGB8_SNorm
                             || viewFormat == QOpenGLTexture::SRGB8
                             || viewFormat == QOpenGLTexture::RGB8U
                             || viewFormat == QOpenGLTexture::RGB8I);
        break;

    case QOpenGLTexture::FormatClass_16Bit:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::R16F
                             || viewFormat == QOpenGLTexture::RG8U
                             || viewFormat == QOpenGLTexture::R16U
                             || viewFormat == QOpenGLTexture::RG8I
                             || viewFormat == QOpenGLTexture::R16I
                             || viewFormat == QOpenGLTexture::RG8_UNorm
                             || viewFormat == QOpenGLTexture::R16_UNorm
                             || viewFormat == QOpenGLTexture::RG8_SNorm
                             || viewFormat == QOpenGLTexture::R16_SNorm);
        break;

    case QOpenGLTexture::FormatClass_8Bit:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::R8U
                             || viewFormat == QOpenGLTexture::R8I
                             || viewFormat == QOpenGLTexture::R8_UNorm
                             || viewFormat == QOpenGLTexture::R8_SNorm);
        break;

    case QOpenGLTexture::FormatClass_RGTC1_R:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::R_ATI1N_UNorm
                             || viewFormat == QOpenGLTexture::R_ATI1N_SNorm);
        break;

    case QOpenGLTexture::FormatClass_RGTC2_RG:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RG_ATI2N_UNorm
                             || viewFormat == QOpenGLTexture::RG_ATI2N_SNorm);
        break;

    case QOpenGLTexture::FormatClass_BPTC_Unorm:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RGB_BP_UNorm
                             || viewFormat == QOpenGLTexture::SRGB_BP_UNorm);
        break;

    case QOpenGLTexture::FormatClass_BPTC_Float:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RGB_BP_UNSIGNED_FLOAT
                             || viewFormat == QOpenGLTexture::RGB_BP_SIGNED_FLOAT);
        break;

    case QOpenGLTexture::FormatClass_S3TC_DXT1_RGB:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RGB_DXT1
                             || viewFormat == QOpenGLTexture::SRGB_DXT1);
        break;

    case QOpenGLTexture::FormatClass_S3TC_DXT1_RGBA:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RGBA_DXT1
                             || viewFormat == QOpenGLTexture::SRGB_Alpha_DXT1);
        break;

    case QOpenGLTexture::FormatClass_S3TC_DXT3_RGBA:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RGBA_DXT3
                             || viewFormat == QOpenGLTexture::SRGB_Alpha_DXT3);
        break;

    case QOpenGLTexture::FormatClass_S3TC_DXT5_RGBA:
        viewFormatCompatible = (viewFormat == QOpenGLTexture::RGBA_DXT5
                             || viewFormat == QOpenGLTexture::SRGB_Alpha_DXT5);
        break;

    case QOpenGLTexture::FormatClass_Unique:
        viewFormatCompatible = (viewFormat == format);
        break;
    }

    if (!viewFormatCompatible) {
        qWarning("QOpenGLTexture::createTextureView(): Incompatible source and view formats");
        return nullptr;
    }


    // Create a view
    QOpenGLTexture *view = new QOpenGLTexture(viewTarget);
    view->setFormat(viewFormat);
    view->create();
    view->d_ptr->textureView = true;
    texFuncs->glTextureView(view->textureId(), viewTarget, textureId, viewFormat,
                            minimumMipmapLevel, maximumMipmapLevel - minimumMipmapLevel + 1,
                            minimumLayer, maximumLayer - minimumLayer + 1);
    return view;
}

QOpenGLTexture::QOpenGLTexture(Target target)
    : d_ptr(new QOpenGLTexturePrivate(target, this))
{
}

QOpenGLTexture::QOpenGLTexture(const QImage& image, MipMapGeneration genMipMaps)
    : d_ptr(new QOpenGLTexturePrivate(QOpenGLTexture::Target2D, this))
{
    setData(image, genMipMaps);
}

QOpenGLTexture::~QOpenGLTexture()
{
}

QOpenGLTexture::Target QOpenGLTexture::target() const
{
    Q_D(const QOpenGLTexture);
    return d->target;
}

bool QOpenGLTexture::create()
{
    Q_D(QOpenGLTexture);
    return d->create();
}

void QOpenGLTexture::destroy()
{
    Q_D(QOpenGLTexture);
    return d->destroy();
}

bool QOpenGLTexture::isCreated() const
{
    Q_D(const QOpenGLTexture);
    return d->textureId != 0;
}

GLuint QOpenGLTexture::textureId() const
{
    Q_D(const QOpenGLTexture);
    return d->textureId;
}

void QOpenGLTexture::bind()
{
    Q_D(QOpenGLTexture);
    Q_ASSERT(d->textureId);
    d->bind();
}

void QOpenGLTexture::bind(uint unit, TextureUnitReset reset)
{
    Q_D(QOpenGLTexture);
    Q_ASSERT(d->textureId);
    d->bind(unit, reset);
}

void QOpenGLTexture::release()
{
    Q_D(QOpenGLTexture);
    d->release();
}

void QOpenGLTexture::release(uint unit, TextureUnitReset reset)
{
    Q_D(QOpenGLTexture);
    d->release(unit, reset);
}

bool QOpenGLTexture::isBound() const
{
    Q_D(const QOpenGLTexture);
    Q_ASSERT(d->textureId);
    return d->isBound();
}

bool QOpenGLTexture::isBound(uint unit)
{
    Q_D(const QOpenGLTexture);
    Q_ASSERT(d->textureId);
    return d->isBound(unit);
}

GLuint QOpenGLTexture::boundTextureId(BindingTarget target)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        qWarning("QOpenGLTexture::boundTextureId() requires a valid current context");
        return 0;
    }

    GLint textureId = 0;
    ctx->functions()->glGetIntegerv(target, &textureId);
    return static_cast<GLuint>(textureId);
}

GLuint QOpenGLTexture::boundTextureId(uint unit, BindingTarget target)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        qWarning("QOpenGLTexture::boundTextureId() requires a valid current context");
        return 0;
    }

    QOpenGLFunctions *funcs = ctx->functions();
    funcs->initializeOpenGLFunctions();

    GLint oldTextureUnit = 0;
    funcs->glGetIntegerv(GL_ACTIVE_TEXTURE, &oldTextureUnit);

    funcs->glActiveTexture(unit);
    GLint textureId = 0;
    funcs->glGetIntegerv(target, &textureId);
    funcs->glActiveTexture(oldTextureUnit);

    return static_cast<GLuint>(textureId);
}

void QOpenGLTexture::setFormat(TextureFormat format)
{
    Q_D(QOpenGLTexture);
    d->create();
    if (isStorageAllocated()) {
        qWarning("QOpenGLTexture::setFormat(): Cannot change format once storage has been allocated");
        return;
    }

    d->format = format;

    switch (format) {
    case NoFormat:
        d->formatClass = NoFormatClass;
        break;

    case RGBA32F:
    case RGBA32U:
    case RGBA32I:
        d->formatClass = FormatClass_128Bit;
        break;

    case RGB32F:
    case RGB32U:
    case RGB32I:
        d->formatClass = FormatClass_96Bit;
        break;

    case RGBA16F:
    case RG32F:
    case RGBA16U:
    case RG32U:
    case RGBA16I:
    case RG32I:
    case RGBA16_UNorm:
    case RGBA16_SNorm:
        d->formatClass = FormatClass_64Bit;
        break;

    case RGB16_UNorm:
    case RGB16_SNorm:
    case RGB16F:
    case RGB16U:
    case RGB16I:
        d->formatClass = FormatClass_48Bit;
        break;

    case RG16F:
    case RG11B10F:
    case R32F:
    case RGB10A2:
    case RGBA8U:
    case RG16U:
    case R32U:
    case RGBA8I:
    case RG16I:
    case R32I:
    case RGBA8_UNorm:
    case RG16_UNorm:
    case RGBA8_SNorm:
    case RG16_SNorm:
    case SRGB8_Alpha8:
    case RGB9E5:
        d->formatClass = FormatClass_32Bit;
        break;

    case RGB8_UNorm:
    case RGB8_SNorm:
    case SRGB8:
    case RGB8U:
    case RGB8I:
        d->formatClass = FormatClass_24Bit;
        break;

    case R16F:
    case RG8U:
    case R16U:
    case RG8I:
    case R16I:
    case RG8_UNorm:
    case R16_UNorm:
    case RG8_SNorm:
    case R16_SNorm:
        d->formatClass = FormatClass_16Bit;
        break;

    case R8U:
    case R8I:
    case R8_UNorm:
    case R8_SNorm:
        d->formatClass = FormatClass_8Bit;
        break;

    case R_ATI1N_UNorm:
    case R_ATI1N_SNorm:
        d->formatClass = FormatClass_RGTC1_R;
        break;

    case RG_ATI2N_UNorm:
    case RG_ATI2N_SNorm:
        d->formatClass = FormatClass_RGTC2_RG;
        break;

    case RGB_BP_UNorm:
    case SRGB_BP_UNorm:
        d->formatClass = FormatClass_BPTC_Unorm;
        break;

    case RGB_BP_UNSIGNED_FLOAT:
    case RGB_BP_SIGNED_FLOAT:
        d->formatClass = FormatClass_BPTC_Float;
        break;

    case RGB_DXT1:
    case SRGB_DXT1:
        d->formatClass = FormatClass_S3TC_DXT1_RGB;
        break;

    case RGBA_DXT1:
    case SRGB_Alpha_DXT1:
        d->formatClass = FormatClass_S3TC_DXT1_RGBA;
        break;

    case RGBA_DXT3:
    case SRGB_Alpha_DXT3:
        d->formatClass = FormatClass_S3TC_DXT3_RGBA;
        break;

    case RGBA_DXT5:
    case SRGB_Alpha_DXT5:
        d->formatClass = FormatClass_S3TC_DXT5_RGBA;
        break;

    case QOpenGLTexture::R11_EAC_UNorm:
    case QOpenGLTexture::R11_EAC_SNorm:
    case QOpenGLTexture::RG11_EAC_UNorm:
    case QOpenGLTexture::RG11_EAC_SNorm:
    case QOpenGLTexture::RGB8_ETC2:
    case QOpenGLTexture::SRGB8_ETC2:
    case QOpenGLTexture::RGB8_PunchThrough_Alpha1_ETC2:
    case QOpenGLTexture::SRGB8_PunchThrough_Alpha1_ETC2:
    case QOpenGLTexture::RGBA8_ETC2_EAC:
    case QOpenGLTexture::SRGB8_Alpha8_ETC2_EAC:
    case QOpenGLTexture::RGB8_ETC1:
    case RG3B2:
    case R5G6B5:
    case RGB5A1:
    case RGBA4:
    case D16:
    case D24:
    case D24S8:
    case D32:
    case D32F:
    case D32FS8X24:
    case S8:
    case DepthFormat:
    case AlphaFormat:
    case RGBFormat:
    case RGBAFormat:
    case LuminanceFormat:
    case LuminanceAlphaFormat:
        d->formatClass = FormatClass_Unique;
        break;
    }
}

QOpenGLTexture::TextureFormat QOpenGLTexture::format() const
{
    Q_D(const QOpenGLTexture);
    return d->format;
}

void QOpenGLTexture::setSize(int width, int height, int depth)
{
    Q_D(QOpenGLTexture);
    d->create();
    if (isStorageAllocated()) {
        qWarning("Cannot resize a texture that already has storage allocated.\n"
                 "To do so, destroy() the texture and then create() and setSize()");
        return;
    }

    switch (d->target) {
    case QOpenGLTexture::Target1D:
    case QOpenGLTexture::Target1DArray:
    case QOpenGLTexture::TargetBuffer:
        d->dimensions[0] = width;
        (void) height;
        (void) depth;
        break;

    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::Target2DArray:
    case QOpenGLTexture::TargetRectangle:
    case QOpenGLTexture::Target2DMultisample:
    case QOpenGLTexture::Target2DMultisampleArray:
        d->dimensions[0] = width;
        d->dimensions[1] = height;
        (void) depth;
        break;

    case QOpenGLTexture::TargetCubeMap:
    case QOpenGLTexture::TargetCubeMapArray:
        if (width != height) {
           qWarning("QAbstractOpenGLTexture::setSize(): Cube map textures must be square");
        }
        d->dimensions[0] = d->dimensions[1] = width;
        (void) depth;
        break;

    case QOpenGLTexture::Target3D:
        d->dimensions[0] = width;
        d->dimensions[1] = height;
        d->dimensions[2] = depth;
        break;
    }
}

int QOpenGLTexture::width() const
{
    Q_D(const QOpenGLTexture);
    return d->dimensions[0];
}

int QOpenGLTexture::height() const
{
    Q_D(const QOpenGLTexture);
    return d->dimensions[1];
}

int QOpenGLTexture::depth() const
{
    Q_D(const QOpenGLTexture);
    return d->dimensions[2];
}

void QOpenGLTexture::setMipLevels(int levels)
{
    Q_D(QOpenGLTexture);
    d->create();

    if (isStorageAllocated()) {
        qWarning("Cannot set mip levels on a texture that already has storage allocated.\n"
                 "To do so, destroy() the texture and then create() and setMipLevels()");
        return;
    }

    switch (d->target) {
    case QOpenGLTexture::Target1D:
    case QOpenGLTexture::Target1DArray:
    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::Target2DArray:
    case QOpenGLTexture::TargetCubeMap:
    case QOpenGLTexture::TargetCubeMapArray:
    case QOpenGLTexture::Target3D:
        d->requestedMipLevels = levels;
        break;

    case QOpenGLTexture::TargetBuffer:
    case QOpenGLTexture::TargetRectangle:
    case QOpenGLTexture::Target2DMultisample:
    case QOpenGLTexture::Target2DMultisampleArray:
        qWarning("QAbstractOpenGLTexture::setMipLevels(): This texture target does not support mipmaps");
        break;
    }
}

int QOpenGLTexture::mipLevels() const
{
    Q_D(const QOpenGLTexture);
    return isStorageAllocated() ? d->mipLevels : d->requestedMipLevels;
}

int QOpenGLTexture::maximumMipLevels() const
{
    Q_D(const QOpenGLTexture);
    return d->maximumMipLevelCount();
}

void QOpenGLTexture::setLayers(int layers)
{
    Q_D(QOpenGLTexture);
    d->create();
    if (isStorageAllocated()) {
        qWarning("Cannot set layers on a texture that already has storage allocated.\n"
                 "To do so, destroy() the texture and then create() and setLayers()");
        return;
    }

    switch (d->target) {
    case QOpenGLTexture::Target1DArray:
    case QOpenGLTexture::Target2DArray:
    case QOpenGLTexture::TargetCubeMapArray:
    case QOpenGLTexture::Target2DMultisampleArray:
        d->layers = layers;
        break;

    case QOpenGLTexture::Target1D:
    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::Target3D:
    case QOpenGLTexture::TargetCubeMap:
    case QOpenGLTexture::TargetBuffer:
    case QOpenGLTexture::TargetRectangle:
    case QOpenGLTexture::Target2DMultisample:
        qWarning("Texture target does not support array layers");
        break;
    }
}

int QOpenGLTexture::layers() const
{
    Q_D(const QOpenGLTexture);
    return d->layers;
}

int QOpenGLTexture::faces() const
{
    Q_D(const QOpenGLTexture);
    return d->faces;
}

void QOpenGLTexture::setSamples(int samples)
{
    Q_D(QOpenGLTexture);
    d->create();
    if (isStorageAllocated()) {
        qWarning("Cannot set sample count on a texture that already has storage allocated.\n"
                 "To do so, destroy() the texture and then create() and setSamples()");
        return;
    }

    switch (d->target) {
    case QOpenGLTexture::Target2DMultisample:
    case QOpenGLTexture::Target2DMultisampleArray:
        d->samples = samples;
        break;

    case QOpenGLTexture::Target1D:
    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::Target3D:
    case QOpenGLTexture::Target1DArray:
    case QOpenGLTexture::Target2DArray:
    case QOpenGLTexture::TargetCubeMap:
    case QOpenGLTexture::TargetCubeMapArray:
    case QOpenGLTexture::TargetBuffer:
    case QOpenGLTexture::TargetRectangle:

        qWarning("Texture target does not support multisampling");
        break;
    }
}

int QOpenGLTexture::samples() const
{
    Q_D(const QOpenGLTexture);
    return d->samples;
}

void QOpenGLTexture::setFixedSamplePositions(bool fixed)
{
    Q_D(QOpenGLTexture);
    d->create();
    if (isStorageAllocated()) {
        qWarning("Cannot set sample positions on a texture that already has storage allocated.\n"
                 "To do so, destroy() the texture and then create() and setFixedSamplePositions()");
        return;
    }

    switch (d->target) {
    case QOpenGLTexture::Target2DMultisample:
    case QOpenGLTexture::Target2DMultisampleArray:
        d->fixedSamplePositions = fixed;
        break;

    case QOpenGLTexture::Target1D:
    case QOpenGLTexture::Target2D:
    case QOpenGLTexture::Target3D:
    case QOpenGLTexture::Target1DArray:
    case QOpenGLTexture::Target2DArray:
    case QOpenGLTexture::TargetCubeMap:
    case QOpenGLTexture::TargetCubeMapArray:
    case QOpenGLTexture::TargetBuffer:
    case QOpenGLTexture::TargetRectangle:

        qWarning("Texture target does not support multisampling");
        break;
    }
}

bool QOpenGLTexture::isFixedSamplePositions() const
{
    Q_D(const QOpenGLTexture);
    return d->fixedSamplePositions;
}

void QOpenGLTexture::allocateStorage()
{
    Q_D(QOpenGLTexture);
    if (d->create()) {
        const QOpenGLTexture::PixelFormat pixelFormat = pixelFormatCompatibleWithInternalFormat(d->format);
        const QOpenGLTexture::PixelType pixelType = pixelTypeCompatibleWithInternalFormat(d->format);
        d->allocateStorage(pixelFormat, pixelType);
    }
}

void QOpenGLTexture::allocateStorage(QOpenGLTexture::PixelFormat pixelFormat, QOpenGLTexture::PixelType pixelType)
{
    Q_D(QOpenGLTexture);
    if (d->create())
        d->allocateStorage(pixelFormat, pixelType);
}

bool QOpenGLTexture::isStorageAllocated() const
{
    Q_D(const QOpenGLTexture);
    return d->storageAllocated;
}

QOpenGLTexture *QOpenGLTexture::createTextureView(Target target,
      TextureFormat viewFormat, int minimumMipmapLevel, int maximumMipmapLevel,
      int minimumLayer, int maximumLayer) const
{
    Q_D(const QOpenGLTexture);

    if (!isStorageAllocated()) {
        qWarning("Cannot set create a texture view of a texture that does not have storage allocated.");
        return nullptr;
    }

    Q_ASSERT(maximumMipmapLevel >= minimumMipmapLevel);
    Q_ASSERT(maximumLayer >= minimumLayer);

    return d->createTextureView(target, viewFormat, minimumMipmapLevel, maximumMipmapLevel,
       minimumLayer, maximumLayer);
}

bool QOpenGLTexture::isTextureView() const
{
    Q_D(const QOpenGLTexture);
    Q_ASSERT(d->textureId);
    return d->textureView;
}

void QOpenGLTexture::setData(int mipLevel, int layer, CubeMapFace cubeFace,
   PixelFormat sourceFormat, PixelType sourceType,
   const void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(QOpenGLTexture);
    Q_ASSERT(d->textureId);
    if (!isStorageAllocated()) {
        qWarning("Cannot set data on a texture that does not have storage allocated.\n"
                 "To do so call allocateStorage() before this function");
        return;
    }
    d->setData(mipLevel, layer, cubeFace, sourceFormat, sourceType, data, options);
}

void QOpenGLTexture::setData(int mipLevel, int layer,
   PixelFormat sourceFormat, PixelType sourceType,
   const void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(QOpenGLTexture);
    Q_ASSERT(d->textureId);
    d->setData(mipLevel, layer, QOpenGLTexture::CubeMapPositiveX, sourceFormat, sourceType, data, options);
}

void QOpenGLTexture::setData(int mipLevel,
   PixelFormat sourceFormat, PixelType sourceType,
   const void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(QOpenGLTexture);
    Q_ASSERT(d->textureId);
    d->setData(mipLevel, 0, QOpenGLTexture::CubeMapPositiveX, sourceFormat, sourceType, data, options);
}

void QOpenGLTexture::setData(PixelFormat sourceFormat, PixelType sourceType,
   const void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(QOpenGLTexture);
    Q_ASSERT(d->textureId);
    d->setData(0, 0, QOpenGLTexture::CubeMapPositiveX, sourceFormat, sourceType, data, options);
}

void QOpenGLTexture::setData(const QImage& image, MipMapGeneration genMipMaps)
{
    QOpenGLContext *context = QOpenGLContext::currentContext();
    if (!context) {
        qWarning("QOpenGLTexture::setData() requires a valid current context");
        return;
    }

    if (image.isNull()) {
        qWarning("QOpenGLTexture::setData() tried to set a null image");
        return;
    }

    if (context->isOpenGLES() && context->format().majorVersion() < 3)
        setFormat(QOpenGLTexture::RGBAFormat);
    else
        setFormat(QOpenGLTexture::RGBA8_UNorm);

    setSize(image.width(), image.height());
    setMipLevels(genMipMaps == GenerateMipMaps ? maximumMipLevels() : 1);
    allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);

    // Upload pixel data and generate mipmaps
    QImage glImage = image.convertToFormat(QImage::Format_RGBA8888);
    QOpenGLPixelTransferOptions uploadOptions;
    uploadOptions.setAlignment(1);
    setData(0, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, glImage.constBits(), &uploadOptions);
}

void QOpenGLTexture::setCompressedData(int mipLevel, int layer, CubeMapFace cubeFace,
   int dataSize, const void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(QOpenGLTexture);
    Q_ASSERT(d->textureId);

    if (!isStorageAllocated()) {
        qWarning("Cannot set data on a texture that does not have storage allocated.\n"
                 "To do so call allocateStorage() before this function");
        return;
    }

    d->setCompressedData(mipLevel, layer, cubeFace, dataSize, data, options);
}

void QOpenGLTexture::setCompressedData(int mipLevel, int layer, int dataSize, const void *data,
   const QOpenGLPixelTransferOptions * const options)
{
    Q_D(QOpenGLTexture);
    Q_ASSERT(d->textureId);
    d->setCompressedData(mipLevel, layer, QOpenGLTexture::CubeMapPositiveX, dataSize, data, options);
}

void QOpenGLTexture::setCompressedData(int mipLevel, int dataSize, const void *data,
                                       const QOpenGLPixelTransferOptions * const options)
{
    Q_D(QOpenGLTexture);
    Q_ASSERT(d->textureId);
    d->setCompressedData(mipLevel, 0, QOpenGLTexture::CubeMapPositiveX, dataSize, data, options);
}

void QOpenGLTexture::setCompressedData(int dataSize, const void *data,
                                       const QOpenGLPixelTransferOptions * const options)
{
    Q_D(QOpenGLTexture);
    Q_ASSERT(d->textureId);
    d->setCompressedData(0, 0, QOpenGLTexture::CubeMapPositiveX, dataSize, data, options);
}

bool QOpenGLTexture::hasFeature(Feature feature)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (! ctx) {
        qWarning("QOpenGLTexture::hasFeature() requires a valid current context");
        return false;
    }

    QSurfaceFormat f = ctx->format();

    bool supported = false;

#if !defined(QT_OPENGL_ES_2)
    if (!ctx->isOpenGLES()) {
        switch (feature) {

        case ImmutableMultisampleStorage:
            supported = f.version() >= qMakePair(4, 3)
                    || ctx->hasExtension("GL_ARB_texture_storage_multisample");
            break;

        case TextureBuffer:
            supported = f.version() >= qMakePair(3, 0)
                    || ctx->hasExtension("GL_ARB_texture_buffer_object");
            break;

        case StencilTexturing:
            supported = f.version() >= qMakePair(4, 3)
                    || ctx->hasExtension("GL_ARB_stencil_texturing");
            break;

        case ImmutableStorage:
            supported = f.version() >= qMakePair(4, 2)
                    || ctx->hasExtension("GL_ARB_texture_storage")
                    || ctx->hasExtension("GL_EXT_texture_storage");
            break;

        case TextureCubeMapArrays:
            supported = f.version() >= qMakePair(4, 0)
                    || ctx->hasExtension("ARB_texture_cube_map_array");
            break;

        case Swizzle:
            supported = f.version() >= qMakePair(3, 3)
                    || ctx->hasExtension("GL_ARB_texture_swizzle");
            break;

        case TextureMultisample:
            supported = f.version() >= qMakePair(3, 2)
                    || ctx->hasExtension("GL_ARB_texture_multisample");
            break;

        case TextureArrays:
            supported = f.version() >= qMakePair(3, 0)
                    || ctx->hasExtension("GL_EXT_texture_array");
            break;

        case TextureRectangle:
            supported = f.version() >= qMakePair(2, 1)
                    || ctx->hasExtension("ARB_texture_rectangle");
            break;

        case Texture3D:
            supported = f.version() >= qMakePair(1, 3);
            break;

        case AnisotropicFiltering:
            supported = ctx->hasExtension("GL_EXT_texture_filter_anisotropic");
            break;

        case NPOTTextures:
        case NPOTTextureRepeat:
            supported = ctx->hasExtension("GL_ARB_texture_non_power_of_two");
            break;

        case Texture1D:
            supported = f.version() >= qMakePair(1, 1);
            break;

        case TextureComparisonOperators:
            // GL 1.4 and GL_ARB_shadow alone support only LEQUAL and GEQUAL;
            // since we're talking about history anyhow avoid to be extra pedantic
            // in the feature set, and simply claim supported if we have the full set of operators
            // (which has been added into 1.5 / GL_EXT_shadow_funcs).

            supported = f.version() >= qMakePair(1, 5) || (ctx->hasExtension("GL_ARB_shadow")
                        && ctx->hasExtension("GL_EXT_shadow_funcs"));
            break;

        case TextureMipMapLevel:
            supported = f.version() >= qMakePair(1, 2);
            break;

        case MaxFeatureFlag:
            break;
        }
    }

    if (ctx->isOpenGLES())
#endif
    {
        const char *renderer = reinterpret_cast<const char *>(ctx->functions()->glGetString(GL_RENDERER));
        switch (feature) {
        case ImmutableStorage:
            supported = (f.version() >= qMakePair(3, 0) || ctx->hasExtension("EXT_texture_storage"))
                && !(renderer && strstr(renderer, "Mali")); // do not use on Mali: QTBUG-45106
            break;

        case ImmutableMultisampleStorage:
            supported = f.version() >= qMakePair(3, 1);
            break;

        case TextureRectangle:
            break;

        case TextureArrays:
            supported = f.version() >= qMakePair(3, 0);
            break;

        case Texture3D:
            supported = f.version() >= qMakePair(3, 0)
                    || ctx->hasExtension("GL_OES_texture_3D");
            break;

        case TextureMultisample:
            supported = f.version() >= qMakePair(3, 1);
            break;

        case TextureBuffer:
            break;

        case TextureCubeMapArrays:
            break;

        case Swizzle:
            supported = f.version() >= qMakePair(3, 0);
            break;

        case StencilTexturing:
            break;

        case AnisotropicFiltering:
            supported = ctx->hasExtension("GL_EXT_texture_filter_anisotropic");
            break;

        case NPOTTextures:
        case NPOTTextureRepeat:
            supported = f.version() >= qMakePair(3,0)
                    || ctx->hasExtension("GL_OES_texture_npot")
                    || ctx->hasExtension("GL_ARB_texture_non_power_of_two");
            break;

        case Texture1D:
            break;

        case TextureComparisonOperators:
            supported = f.version() >= qMakePair(3, 0)
                    || ctx->hasExtension("GL_EXT_shadow_samplers");
            break;

        case TextureMipMapLevel:
            supported = f.version() >= qMakePair(3, 0);
            break;

        case MaxFeatureFlag:
            break;
        }
    }

    return supported;
}

void QOpenGLTexture::setMipBaseLevel(int baseLevel)
{
    Q_D(QOpenGLTexture);
    d->create();

    if (!d->features.testFlag(TextureMipMapLevel)) {
        qWarning("QOpenGLTexture::setMipBaseLevel: requires OpenGL >= 1.2 or OpenGL ES >= 3.0");
        return;
    }

    Q_ASSERT(d->textureId);
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(baseLevel <= d->maxLevel);

    d->baseLevel = baseLevel;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_BASE_LEVEL, baseLevel);
}

int QOpenGLTexture::mipBaseLevel() const
{
    Q_D(const QOpenGLTexture);
    return d->baseLevel;
}

void QOpenGLTexture::setMipMaxLevel(int maxLevel)
{
    Q_D(QOpenGLTexture);
    d->create();
    if (!d->features.testFlag(TextureMipMapLevel)) {
        qWarning("QOpenGLTexture::setMipMaxLevel: requires OpenGL >= 1.2 or OpenGL ES >= 3.0");
        return;
    }
    Q_ASSERT(d->textureId);
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->baseLevel <= maxLevel);
    d->maxLevel = maxLevel;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAX_LEVEL, maxLevel);
}

int QOpenGLTexture::mipMaxLevel() const
{
    Q_D(const QOpenGLTexture);
    return d->maxLevel;
}

void QOpenGLTexture::setMipLevelRange(int baseLevel, int maxLevel)
{
    Q_D(QOpenGLTexture);
    d->create();
    if (!d->features.testFlag(TextureMipMapLevel)) {
        qWarning("QOpenGLTexture::setMipLevelRange: requires OpenGL >= 1.2 or OpenGL ES >= 3.0");
        return;
    }
    Q_ASSERT(d->textureId);
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(baseLevel <= maxLevel);
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_BASE_LEVEL, baseLevel);
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAX_LEVEL, maxLevel);
}

QPair<int, int> QOpenGLTexture::mipLevelRange() const
{
    Q_D(const QOpenGLTexture);
    return qMakePair(d->baseLevel, d->maxLevel);
}

void QOpenGLTexture::setAutoMipMapGenerationEnabled(bool enabled)
{
    Q_D(QOpenGLTexture);
    d->autoGenerateMipMaps = enabled;
}

bool QOpenGLTexture::isAutoMipMapGenerationEnabled() const
{
    Q_D(const QOpenGLTexture);
    return d->autoGenerateMipMaps;
}

void QOpenGLTexture::generateMipMaps()
{
    Q_D(QOpenGLTexture);
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    if (isCompressedFormat(d->format)) {
        if (QOpenGLContext *ctx = QOpenGLContext::currentContext())
            if (ctx->isOpenGLES() && ctx->format().majorVersion() < 3)
                return;
    }
    d->texFuncs->glGenerateTextureMipmap(d->textureId, d->target, d->bindingTarget);
}

void QOpenGLTexture::generateMipMaps(int baseLevel, bool resetBaseLevel)
{
    Q_D(QOpenGLTexture);
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);

    if (isCompressedFormat(d->format)) {
        if (QOpenGLContext *ctx = QOpenGLContext::currentContext())
            if (ctx->isOpenGLES() && ctx->format().majorVersion() < 3)
                return;
    }

    int oldBaseLevel = 0;

    if (resetBaseLevel) {
        oldBaseLevel = mipBaseLevel();
    }

    setMipBaseLevel(baseLevel);
    d->texFuncs->glGenerateTextureMipmap(d->textureId, d->target, d->bindingTarget);

    if (resetBaseLevel) {
        setMipBaseLevel(oldBaseLevel);
    }
}

void QOpenGLTexture::setSwizzleMask(SwizzleComponent component, SwizzleValue value)
{
#if !defined(Q_OS_DARWIN) && !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(QOpenGLTexture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        if (!d->features.testFlag(Swizzle)) {
            qWarning("QOpenGLTexture::setSwizzleMask() requires OpenGL >= 3.3");
            return;
        }
        d->swizzleMask[component - SwizzleRed] = value;
        d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, component, value);
        return;
    }
#else
    (void) component;
    (void) value;
#endif

    qWarning("QOpenGLTexture: Texture swizzling is not supported");
}

void QOpenGLTexture::setSwizzleMask(SwizzleValue r, SwizzleValue g, SwizzleValue b, SwizzleValue a)
{
#if ! defined(Q_OS_DARWIN) && !defined(QT_OPENGL_ES_2)
    if (! QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(QOpenGLTexture);
        d->create();

        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);

        if (! d->features.testFlag(Swizzle)) {
            qWarning("QOpenGLTexture::setSwizzleMask() requires OpenGL >= 3.3");
            return;
        }

        GLint swizzleMask[] = {GLint(r), GLint(g), GLint(b), GLint(a)};
        d->swizzleMask[0] = r;
        d->swizzleMask[1] = g;
        d->swizzleMask[2] = b;
        d->swizzleMask[3] = a;
        d->texFuncs->glTextureParameteriv(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        return;
    }
#else
    (void) r;
    (void) g;
    (void) b;
    (void) a;
#endif
    qWarning("QOpenGLTexture: Texture swizzling is not supported");
}

QOpenGLTexture::SwizzleValue QOpenGLTexture::swizzleMask(SwizzleComponent component) const
{
    Q_D(const QOpenGLTexture);
    return d->swizzleMask[component - SwizzleRed];
}

void QOpenGLTexture::setDepthStencilMode(QOpenGLTexture::DepthStencilMode mode)
{
#if !defined(Q_OS_DARWIN) && !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(QOpenGLTexture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        if (!d->features.testFlag(StencilTexturing)) {
            qWarning("QOpenGLTexture::setDepthStencilMode() requires OpenGL >= 4.3 or GL_ARB_stencil_texturing");
            return;
        }
        d->depthStencilMode = mode;
        d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_DEPTH_STENCIL_TEXTURE_MODE, mode);
        return;
    }
#else
    (void) mode;
#endif

    qWarning("QOpenGLTexture: DepthStencil Mode is not supported");
}

QOpenGLTexture::DepthStencilMode QOpenGLTexture::depthStencilMode() const
{
    Q_D(const QOpenGLTexture);
    return d->depthStencilMode;
}

void QOpenGLTexture::setComparisonFunction(QOpenGLTexture::ComparisonFunction function)
{
    Q_D(QOpenGLTexture);
    d->create();
    if (!d->features.testFlag(TextureComparisonOperators)) {
        qWarning("QOpenGLTexture::setComparisonFunction: requires OpenGL >= 1.5 or OpenGL ES >= 3.0");
        return;
    }
    d->comparisonFunction = function;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_COMPARE_FUNC, function);
}

QOpenGLTexture::ComparisonFunction QOpenGLTexture::comparisonFunction() const
{
    Q_D(const QOpenGLTexture);
    return d->comparisonFunction;
}

void QOpenGLTexture::setComparisonMode(QOpenGLTexture::ComparisonMode mode)
{
    Q_D(QOpenGLTexture);
    d->create();
    if (!d->features.testFlag(TextureComparisonOperators)) {
        qWarning("QOpenGLTexture::setComparisonMode: requires OpenGL >= 1.5 or OpenGL ES >= 3.0");
        return;
    }
    d->comparisonMode = mode;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_COMPARE_MODE, mode);
}

QOpenGLTexture::ComparisonMode QOpenGLTexture::comparisonMode() const
{
    Q_D(const QOpenGLTexture);
    return d->comparisonMode;
}

void QOpenGLTexture::setMinificationFilter(QOpenGLTexture::Filter filter)
{
    Q_D(QOpenGLTexture);
    d->create();
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    d->minFilter = filter;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MIN_FILTER, filter);
}

QOpenGLTexture::Filter QOpenGLTexture::minificationFilter() const
{
    Q_D(const QOpenGLTexture);
    return d->minFilter;
}

void QOpenGLTexture::setMagnificationFilter(QOpenGLTexture::Filter filter)
{
    Q_D(QOpenGLTexture);
    d->create();
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    d->magFilter = filter;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAG_FILTER, filter);
}

QOpenGLTexture::Filter QOpenGLTexture::magnificationFilter() const
{
    Q_D(const QOpenGLTexture);
    return d->magFilter;
}

void QOpenGLTexture::setMinMagFilters(QOpenGLTexture::Filter minificationFilter,
                                      QOpenGLTexture::Filter magnificationFilter)
{
    Q_D(QOpenGLTexture);
    d->create();
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    d->minFilter = minificationFilter;
    d->magFilter = magnificationFilter;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MIN_FILTER, minificationFilter);
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAG_FILTER, magnificationFilter);
}

QPair<QOpenGLTexture::Filter, QOpenGLTexture::Filter> QOpenGLTexture::minMagFilters() const
{
    Q_D(const QOpenGLTexture);
    return QPair<QOpenGLTexture::Filter, QOpenGLTexture::Filter>(d->minFilter, d->magFilter);
}

void QOpenGLTexture::setMaximumAnisotropy(float anisotropy)
{
    Q_D(QOpenGLTexture);
    d->create();
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    if (!d->features.testFlag(AnisotropicFiltering)) {
        qWarning("QOpenGLTexture::setMaximumAnisotropy() requires GL_EXT_texture_filter_anisotropic");
        return;
    }
    d->maxAnisotropy = anisotropy;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
}

float QOpenGLTexture::maximumAnisotropy() const
{
    Q_D(const QOpenGLTexture);
    return d->maxAnisotropy;
}

void QOpenGLTexture::setWrapMode(QOpenGLTexture::WrapMode mode)
{
    Q_D(QOpenGLTexture);
    d->create();
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    d->setWrapMode(mode);
}

void QOpenGLTexture::setWrapMode(QOpenGLTexture::CoordinateDirection direction, QOpenGLTexture::WrapMode mode)
{
    Q_D(QOpenGLTexture);
    d->create();
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    d->setWrapMode(direction, mode);
}

QOpenGLTexture::WrapMode QOpenGLTexture::wrapMode(QOpenGLTexture::CoordinateDirection direction) const
{
    Q_D(const QOpenGLTexture);
    return d->wrapMode(direction);
}

void QOpenGLTexture::setBorderColor(QColor color)
{
#if ! defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(QOpenGLTexture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        float values[4];
        values[0] = color.redF();
        values[1] = color.greenF();
        values[2] = color.blueF();
        values[3] = color.alphaF();
        d->borderColor.clear();
        for (int i = 0; i < 4; ++i)
            d->borderColor.append(QVariant(values[i]));
        d->texFuncs->glTextureParameterfv(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_BORDER_COLOR, values);
        return;
    }
#else
    (void) color;
#endif
    qWarning("QOpenGLTexture: Border color is not supported");
}

void QOpenGLTexture::setBorderColor(float r, float g, float b, float a)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(QOpenGLTexture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        float values[4];
        values[0] = r;
        values[1] = g;
        values[2] = b;
        values[3] = a;
        d->borderColor.clear();
        for (int i = 0; i < 4; ++i)
            d->borderColor.append(QVariant(values[i]));
        d->texFuncs->glTextureParameterfv(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_BORDER_COLOR, values);
        return;
    }
#else
    (void) r;
    (void) g;
    (void) b;
    (void) a;
#endif
    qWarning("QOpenGLTexture: Border color is not supported");
}

void QOpenGLTexture::setBorderColor(int r, int g, int b, int a)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(QOpenGLTexture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        int values[4];
        values[0] = r;
        values[1] = g;
        values[2] = b;
        values[3] = a;
        d->borderColor.clear();
        for (int i = 0; i < 4; ++i)
            d->borderColor.append(QVariant(values[i]));
        d->texFuncs->glTextureParameteriv(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_BORDER_COLOR, values);
        return;
    }
#else
    (void) r;
    (void) g;
    (void) b;
    (void) a;
#endif
    qWarning("QOpenGLTexture: Border color is not supported");

    // TODO Handle case of using glTextureParameterIiv() based on format
}

void QOpenGLTexture::setBorderColor(uint r, uint g, uint b, uint a)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(QOpenGLTexture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        int values[4];
        values[0] = int(r);
        values[1] = int(g);
        values[2] = int(b);
        values[3] = int(a);
        d->borderColor.clear();

        for (int i = 0; i < 4; ++i) {
            d->borderColor.append(QVariant(values[i]));
        }

        d->texFuncs->glTextureParameteriv(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_BORDER_COLOR, values);

        return;
    }
#else
    (void) r;
    (void) g;
    (void) b;
    (void) a;
#endif

    qWarning("QOpenGLTexture: Border color is not supported");

    // TODO Handle case of using glTextureParameterIuiv() based on format
}

QColor QOpenGLTexture::borderColor() const
{
    Q_D(const QOpenGLTexture);
    QColor c(0.0f, 0.0f, 0.0f, 0.0f);
    if (!d->borderColor.isEmpty()) {
        c.setRedF(d->borderColor.at(0).toFloat());
        c.setGreenF(d->borderColor.at(1).toFloat());
        c.setBlueF(d->borderColor.at(2).toFloat());
        c.setAlphaF(d->borderColor.at(3).toFloat());
    }
    return c;
}

void QOpenGLTexture::borderColor(float *border) const
{
    Q_D(const QOpenGLTexture);
    Q_ASSERT(border);
    if (d->borderColor.isEmpty()) {
        for (int i = 0; i < 4; ++i)
            border[i] = 0.0f;
    } else {
        for (int i = 0; i < 4; ++i)
            border[i] = d->borderColor.at(i).toFloat();
    }
}

void QOpenGLTexture::borderColor(int *border) const
{
    Q_D(const QOpenGLTexture);
    Q_ASSERT(border);
    if (d->borderColor.isEmpty()) {
        for (int i = 0; i < 4; ++i)
            border[i] = 0;
    } else {
        for (int i = 0; i < 4; ++i)
            border[i] = d->borderColor.at(i).toInt();
    }
}

void QOpenGLTexture::borderColor(unsigned int *border) const
{
    Q_D(const QOpenGLTexture);
    Q_ASSERT(border);
    if (d->borderColor.isEmpty()) {
        for (int i = 0; i < 4; ++i)
            border[i] = 0;
    } else {
        for (int i = 0; i < 4; ++i)
            border[i] = d->borderColor.at(i).toUInt();
    }
}

void QOpenGLTexture::setMinimumLevelOfDetail(float value)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(QOpenGLTexture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        Q_ASSERT(value < d->maxLevelOfDetail);
        d->minLevelOfDetail = value;
        d->texFuncs->glTextureParameterf(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MIN_LOD, value);
        return;
    }
#else
    (void) value;
#endif

    qWarning("QOpenGLTexture: Detail level is not supported");
}

float QOpenGLTexture::minimumLevelOfDetail() const
{
    Q_D(const QOpenGLTexture);
    return d->minLevelOfDetail;
}

void QOpenGLTexture::setMaximumLevelOfDetail(float value)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(QOpenGLTexture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        Q_ASSERT(value > d->minLevelOfDetail);
        d->maxLevelOfDetail = value;
        d->texFuncs->glTextureParameterf(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAX_LOD, value);
        return;
    }
#else
    (void) value;
#endif

    qWarning("QOpenGLTexture: Detail level is not supported");
}

float QOpenGLTexture::maximumLevelOfDetail() const
{
    Q_D(const QOpenGLTexture);
    return d->maxLevelOfDetail;
}

void QOpenGLTexture::setLevelOfDetailRange(float min, float max)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(QOpenGLTexture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        Q_ASSERT(min < max);
        d->minLevelOfDetail = min;
        d->maxLevelOfDetail = max;
        d->texFuncs->glTextureParameterf(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MIN_LOD, min);
        d->texFuncs->glTextureParameterf(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAX_LOD, max);
        return;
    }
#else
    (void) min;
    (void) max;
#endif

    qWarning("QOpenGLTexture: Detail level is not supported");
}

QPair<float, float> QOpenGLTexture::levelOfDetailRange() const
{
    Q_D(const QOpenGLTexture);
    return qMakePair(d->minLevelOfDetail, d->maxLevelOfDetail);
}
void QOpenGLTexture::setLevelofDetailBias(float bias)
{
#if ! defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(QOpenGLTexture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        d->levelOfDetailBias = bias;
        d->texFuncs->glTextureParameterf(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_LOD_BIAS, bias);
        return;
    }
#else
    (void) bias;
#endif
    qWarning("QOpenGLTexture: Detail level is not supported");
}

float QOpenGLTexture::levelofDetailBias() const
{
    Q_D(const QOpenGLTexture);
    return d->levelOfDetailBias;
}
