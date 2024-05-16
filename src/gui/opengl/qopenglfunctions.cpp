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

#include <qopenglfunctions.h>

#include <qdebug.h>
#include <qopengl_extrafunctions.h>
#include <qplatform_integration.h>

#include <qapplication_p.h>
#include <qopengl_extensions_p.h>
#include <qopengl_p.h>
#include <qopenglcontext_p.h>

#ifdef Q_OS_IOS
#include <dlfcn.h>
#endif

#ifndef GL_FRAMEBUFFER_SRGB_CAPABLE_EXT
#define GL_FRAMEBUFFER_SRGB_CAPABLE_EXT   0x8DBA
#endif

// emerald    Q_LOGGING_CATEGORY(lcGLES3, "qt.opengl.es3")

// Hidden private fields for additional extension data.
struct QOpenGLFunctionsPrivateEx : public QOpenGLExtensionsPrivate, public QOpenGLSharedResource
{
    QOpenGLFunctionsPrivateEx(QOpenGLContext *context)
        : QOpenGLExtensionsPrivate(context)
        , QOpenGLSharedResource(context->shareGroup())
        , m_features(-1)
        , m_extensions(-1)
    {}

    void invalidateResource() override
    {
        m_features = -1;
        m_extensions = -1;
    }

    void freeResource(QOpenGLContext *) override
    {
        // no gl resources to free
    }

    int m_features;
    int m_extensions;
};

static QOpenGLMultiGroupSharedResource *qt_gl_functions_resource()
{
   static QOpenGLMultiGroupSharedResource retval;
   return &retval;
}

static QOpenGLFunctionsPrivateEx *qt_gl_functions(QOpenGLContext *context = nullptr)
{
    if (! context) {
        context = QOpenGLContext::currentContext();
    }

    Q_ASSERT(context);

    QOpenGLFunctionsPrivateEx *funcs =
        qt_gl_functions_resource()->value<QOpenGLFunctionsPrivateEx>(context);

    return funcs;
}

QOpenGLFunctions::QOpenGLFunctions()
    : d_ptr(nullptr)
{
}

QOpenGLFunctions::QOpenGLFunctions(QOpenGLContext *context)
    : d_ptr(nullptr)
{
    if (context && QOpenGLContextGroup::currentContextGroup() == context->shareGroup()) {
        d_ptr = qt_gl_functions(context);
    } else {
        qWarning() << "QOpenGLFunctions created with non-current context";
    }
}

QOpenGLExtensions::QOpenGLExtensions()
{
}

QOpenGLExtensions::QOpenGLExtensions(QOpenGLContext *context)
    : QOpenGLExtraFunctions(context)
{
}

static int qt_gl_resolve_features()
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();

    if (ctx->isOpenGLES()) {
        // OpenGL ES
        int features = QOpenGLFunctions::Multitexture |
            QOpenGLFunctions::Shaders |
            QOpenGLFunctions::Buffers |
            QOpenGLFunctions::Framebuffers |
            QOpenGLFunctions::BlendColor |
            QOpenGLFunctions::BlendEquation |
            QOpenGLFunctions::BlendEquationSeparate |
            QOpenGLFunctions::BlendFuncSeparate |
            QOpenGLFunctions::BlendSubtract |
            QOpenGLFunctions::CompressedTextures |
            QOpenGLFunctions::Multisample |
            QOpenGLFunctions::StencilSeparate;
        QOpenGLExtensionMatcher extensions;
        if (extensions.match("GL_IMG_texture_npot"))
            features |= QOpenGLFunctions::NPOTTextures;
        if (extensions.match("GL_OES_texture_npot"))
            features |= QOpenGLFunctions::NPOTTextures |
                QOpenGLFunctions::NPOTTextureRepeat;
        if (ctx->format().majorVersion() >= 3 || extensions.match("GL_EXT_texture_rg")) {
            // Mesa's GLES implementation (as of 10.6.0) is unable to handle this, even though it provides 3.0.
            const char *renderer = reinterpret_cast<const char *>(ctx->functions()->glGetString(GL_RENDERER));
            if (!(renderer && strstr(renderer, "Mesa")))
                features |= QOpenGLFunctions::TextureRGFormats;
        }
        if (ctx->format().majorVersion() >= 3)
            features |= QOpenGLFunctions::MultipleRenderTargets;
        return features;
    } else {
        // OpenGL
        int features = QOpenGLFunctions::TextureRGFormats;
        QSurfaceFormat format = QOpenGLContext::currentContext()->format();
        QOpenGLExtensionMatcher extensions;

        if (format.majorVersion() >= 3)
            features |= QOpenGLFunctions::Framebuffers | QOpenGLFunctions::MultipleRenderTargets;
        else if (extensions.match("GL_EXT_framebuffer_object") || extensions.match("GL_ARB_framebuffer_object"))
            features |= QOpenGLFunctions::Framebuffers | QOpenGLFunctions::MultipleRenderTargets;

        if (format.majorVersion() >= 2) {
            features |= QOpenGLFunctions::BlendColor |
                QOpenGLFunctions::BlendEquation |
                QOpenGLFunctions::BlendSubtract |
                QOpenGLFunctions::Multitexture |
                QOpenGLFunctions::CompressedTextures |
                QOpenGLFunctions::Multisample |
                QOpenGLFunctions::BlendFuncSeparate |
                QOpenGLFunctions::Buffers |
                QOpenGLFunctions::Shaders |
                QOpenGLFunctions::StencilSeparate |
                QOpenGLFunctions::BlendEquationSeparate |
                QOpenGLFunctions::NPOTTextures |
                QOpenGLFunctions::NPOTTextureRepeat;
        } else {
            // Recognize features by extension name.
            if (extensions.match("GL_ARB_multitexture"))
                features |= QOpenGLFunctions::Multitexture;
            if (extensions.match("GL_ARB_shader_objects"))
                features |= QOpenGLFunctions::Shaders;
            if (extensions.match("GL_EXT_blend_color"))
                features |= QOpenGLFunctions::BlendColor;
            if (extensions.match("GL_EXT_blend_equation_separate"))
                features |= QOpenGLFunctions::BlendEquationSeparate;
            if (extensions.match("GL_EXT_blend_subtract"))
                features |= QOpenGLFunctions::BlendSubtract;
            if (extensions.match("GL_EXT_blend_func_separate"))
                features |= QOpenGLFunctions::BlendFuncSeparate;
            if (extensions.match("GL_ARB_texture_compression"))
                features |= QOpenGLFunctions::CompressedTextures;
            if (extensions.match("GL_ARB_multisample"))
                features |= QOpenGLFunctions::Multisample;
            if (extensions.match("GL_ARB_texture_non_power_of_two"))
                features |= QOpenGLFunctions::NPOTTextures |
                    QOpenGLFunctions::NPOTTextureRepeat;
        }

        const QPair<int, int> version = format.version();
        if (version < qMakePair(3, 0)
            || (version == qMakePair(3, 0) && format.testOption(QSurfaceFormat::DeprecatedFunctions))
            || (version == qMakePair(3, 1) && extensions.match("GL_ARB_compatibility"))
            || (version >= qMakePair(3, 2) && format.profile() == QSurfaceFormat::CompatibilityProfile)) {
            features |= QOpenGLFunctions::FixedFunctionPipeline;
        }
        return features;
    }
}

static int qt_gl_resolve_extensions()
{
    int extensions = 0;
    QOpenGLExtensionMatcher extensionMatcher;
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    QSurfaceFormat format = ctx->format();

    if (extensionMatcher.match("GL_EXT_bgra"))
        extensions |= QOpenGLExtensions::BGRATextureFormat;
    if (extensionMatcher.match("GL_ARB_texture_rectangle"))
        extensions |= QOpenGLExtensions::TextureRectangle;
    if (extensionMatcher.match("GL_ARB_texture_compression"))
        extensions |= QOpenGLExtensions::TextureCompression;
    if (extensionMatcher.match("GL_EXT_texture_compression_s3tc"))
        extensions |= QOpenGLExtensions::DDSTextureCompression;
    if (extensionMatcher.match("GL_OES_compressed_ETC1_RGB8_texture"))
        extensions |= QOpenGLExtensions::ETC1TextureCompression;
    if (extensionMatcher.match("GL_IMG_texture_compression_pvrtc"))
        extensions |= QOpenGLExtensions::PVRTCTextureCompression;
    if (extensionMatcher.match("GL_ARB_texture_mirrored_repeat"))
        extensions |= QOpenGLExtensions::MirroredRepeat;
    if (extensionMatcher.match("GL_EXT_stencil_two_side"))
        extensions |= QOpenGLExtensions::StencilTwoSide;
    if (extensionMatcher.match("GL_EXT_stencil_wrap"))
        extensions |= QOpenGLExtensions::StencilWrap;
    if (extensionMatcher.match("GL_NV_float_buffer"))
        extensions |= QOpenGLExtensions::NVFloatBuffer;
    if (extensionMatcher.match("GL_ARB_pixel_buffer_object"))
        extensions |= QOpenGLExtensions::PixelBufferObject;

    if (ctx->isOpenGLES()) {
        if (format.majorVersion() >= 2)
            extensions |= QOpenGLExtensions::GenerateMipmap;

        if (format.majorVersion() >= 3) {
            extensions |= QOpenGLExtensions::PackedDepthStencil
                | QOpenGLExtensions::Depth24
                | QOpenGLExtensions::ElementIndexUint
                | QOpenGLExtensions::MapBufferRange
                | QOpenGLExtensions::FramebufferBlit
                | QOpenGLExtensions::FramebufferMultisample
                | QOpenGLExtensions::Sized8Formats;
        } else {
            // Recognize features by extension name.
            if (extensionMatcher.match("GL_OES_packed_depth_stencil"))
                extensions |= QOpenGLExtensions::PackedDepthStencil;
            if (extensionMatcher.match("GL_OES_depth24"))
                extensions |= QOpenGLExtensions::Depth24;
            if (extensionMatcher.match("GL_ANGLE_framebuffer_blit"))
                extensions |= QOpenGLExtensions::FramebufferBlit;
            if (extensionMatcher.match("GL_ANGLE_framebuffer_multisample"))
                extensions |= QOpenGLExtensions::FramebufferMultisample;
            if (extensionMatcher.match("GL_NV_framebuffer_blit"))
                extensions |= QOpenGLExtensions::FramebufferBlit;
            if (extensionMatcher.match("GL_NV_framebuffer_multisample"))
                extensions |= QOpenGLExtensions::FramebufferMultisample;
            if (extensionMatcher.match("GL_OES_rgb8_rgba8"))
                extensions |= QOpenGLExtensions::Sized8Formats;
        }

        if (extensionMatcher.match("GL_OES_mapbuffer"))
            extensions |= QOpenGLExtensions::MapBuffer;
        if (extensionMatcher.match("GL_OES_element_index_uint"))
            extensions |= QOpenGLExtensions::ElementIndexUint;
        // We don't match GL_APPLE_texture_format_BGRA8888 here because it has different semantics.
        if (extensionMatcher.match("GL_IMG_texture_format_BGRA8888") || extensionMatcher.match("GL_EXT_texture_format_BGRA8888"))
            extensions |= QOpenGLExtensions::BGRATextureFormat;
        if (extensionMatcher.match("GL_EXT_discard_framebuffer"))
            extensions |= QOpenGLExtensions::DiscardFramebuffer;
        if (extensionMatcher.match("GL_EXT_texture_norm16"))
            extensions |= QOpenGLExtensions::Sized16Formats;
    } else {
        extensions |= QOpenGLExtensions::ElementIndexUint
            | QOpenGLExtensions::MapBuffer
            | QOpenGLExtensions::Sized16Formats;

        if (format.version() >= qMakePair(1, 2))
            extensions |= QOpenGLExtensions::BGRATextureFormat;

        if (format.version() >= qMakePair(1, 4) || extensionMatcher.match("GL_SGIS_generate_mipmap"))
            extensions |= QOpenGLExtensions::GenerateMipmap;

        if (format.majorVersion() >= 3 || extensionMatcher.match("GL_ARB_framebuffer_object")) {
            extensions |= QOpenGLExtensions::FramebufferMultisample
                | QOpenGLExtensions::FramebufferBlit
                | QOpenGLExtensions::PackedDepthStencil
                | QOpenGLExtensions::Sized8Formats;
        } else {
            // Recognize features by extension name.
            if (extensionMatcher.match("GL_EXT_framebuffer_multisample"))
                extensions |= QOpenGLExtensions::FramebufferMultisample;
            if (extensionMatcher.match("GL_EXT_framebuffer_blit"))
                extensions |= QOpenGLExtensions::FramebufferBlit;
            if (extensionMatcher.match("GL_EXT_packed_depth_stencil"))
                extensions |= QOpenGLExtensions::PackedDepthStencil;
        }

        if (format.version() >= qMakePair(3, 2) || extensionMatcher.match("GL_ARB_geometry_shader4"))
            extensions |= QOpenGLExtensions::GeometryShaders;

        if (extensionMatcher.match("GL_ARB_map_buffer_range"))
            extensions |= QOpenGLExtensions::MapBufferRange;

        if (extensionMatcher.match("GL_EXT_framebuffer_sRGB")) {
            GLboolean srgbCapableFramebuffers = false;
            ctx->functions()->glGetBooleanv(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &srgbCapableFramebuffers);
            if (srgbCapableFramebuffers)
                extensions |= QOpenGLExtensions::SRGBFrameBuffer;
        }
    }

    return extensions;
}

QOpenGLFunctions::OpenGLFeatures QOpenGLFunctions::openGLFeatures() const
{
    QOpenGLFunctionsPrivateEx *d = static_cast<QOpenGLFunctionsPrivateEx *>(d_ptr);
    if (!d)
        return Qt::EmptyFlag;

    if (d->m_features == -1)
        d->m_features = qt_gl_resolve_features();
    return QOpenGLFunctions::OpenGLFeatures(d->m_features);
}

bool QOpenGLFunctions::hasOpenGLFeature(QOpenGLFunctions::OpenGLFeature feature) const
{
    QOpenGLFunctionsPrivateEx *d = static_cast<QOpenGLFunctionsPrivateEx *>(d_ptr);
    if (!d)
        return false;

    if (d->m_features == -1)
        d->m_features = qt_gl_resolve_features();

    return (d->m_features & int(feature)) != 0;
}

QOpenGLExtensions::OpenGLExtensions QOpenGLExtensions::openGLExtensions()
{
    QOpenGLFunctionsPrivateEx *d = static_cast<QOpenGLFunctionsPrivateEx *>(d_ptr);
    if (!d)
        return Qt::EmptyFlag;

    if (d->m_extensions == -1)
        d->m_extensions = qt_gl_resolve_extensions();
    return QOpenGLExtensions::OpenGLExtensions(d->m_extensions);
}

bool QOpenGLExtensions::hasOpenGLExtension(QOpenGLExtensions::OpenGLExtension extension) const
{
    QOpenGLFunctionsPrivateEx *d = static_cast<QOpenGLFunctionsPrivateEx *>(d_ptr);
    if (!d)
        return false;
    if (d->m_extensions == -1)
        d->m_extensions = qt_gl_resolve_extensions();
    return (d->m_extensions & int(extension)) != 0;
}

void QOpenGLFunctions::initializeOpenGLFunctions()
{
    d_ptr = qt_gl_functions();
}

namespace {

enum ResolvePolicy
{
    ResolveOES = 0x1,
    ResolveEXT = 0x2,
    ResolveANGLE = 0x4,
    ResolveNV = 0x8
};

template <typename Base, typename FuncType, int Policy, typename ReturnType>
class Resolver
{
public:
    Resolver(FuncType Base::*func, FuncType fallback, const char *name, const char *alternateName = nullptr)
        : funcPointerName(func)
        , fallbackFuncPointer(fallback)
        , funcName(name)
        , alternateFuncName(alternateName)
    {
    }

    ReturnType operator()();

    template <typename P1>
    ReturnType operator()(P1 p1);

    template <typename P1, typename P2>
    ReturnType operator()(P1 p1, P2 p2);

    template <typename P1, typename P2, typename P3>
    ReturnType operator()(P1 p1, P2 p2, P3 p3);

    template <typename P1, typename P2, typename P3, typename P4>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4);

    template <typename P1, typename P2, typename P3, typename P4, typename P5>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11>
    ReturnType operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11);

private:
    FuncType Base::*funcPointerName;
    FuncType fallbackFuncPointer;
    QByteArray funcName;
    QByteArray alternateFuncName;
};

template <typename Base, typename FuncType, int Policy>
class Resolver<Base, FuncType, Policy, void>
{
public:
    Resolver(FuncType Base::*func, FuncType fallback, const char *name, const char *alternateName = nullptr)
        : funcPointerName(func)
        , fallbackFuncPointer(fallback)
        , funcName(name)
        , alternateFuncName(alternateName)
    {
    }

    void operator()();

    template <typename P1>
    void operator()(P1 p1);

    template <typename P1, typename P2>
    void operator()(P1 p1, P2 p2);

    template <typename P1, typename P2, typename P3>
    void operator()(P1 p1, P2 p2, P3 p3);

    template <typename P1, typename P2, typename P3, typename P4>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4);

    template <typename P1, typename P2, typename P3, typename P4, typename P5>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10);

    template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11>
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11);

private:
    FuncType Base::*funcPointerName;
    FuncType fallbackFuncPointer;
    QByteArray funcName;
    QByteArray alternateFuncName;
};

#define RESOLVER_COMMON \
    QOpenGLContext *context = QOpenGLContext::currentContext(); \
    Base *funcs = qt_gl_functions(context); \
 \
    FuncType old = funcs->*funcPointerName; \
 \
    funcs->*funcPointerName = (FuncType)context->getProcAddress(funcName); \
 \
    if ((Policy & ResolveOES) && !(funcs->*funcPointerName)) \
        funcs->*funcPointerName = (FuncType)context->getProcAddress(funcName + "OES"); \
 \
    if (!(funcs->*funcPointerName)) \
        funcs->*funcPointerName = (FuncType)context->getProcAddress(funcName + "ARB"); \
 \
    if ((Policy & ResolveEXT) && !(funcs->*funcPointerName)) \
        funcs->*funcPointerName = (FuncType)context->getProcAddress(funcName + "EXT"); \
 \
    if ((Policy & ResolveANGLE) && !(funcs->*funcPointerName)) \
        funcs->*funcPointerName = (FuncType)context->getProcAddress(funcName + "ANGLE"); \
 \
    if ((Policy & ResolveNV) && !(funcs->*funcPointerName)) \
        funcs->*funcPointerName = (FuncType)context->getProcAddress(funcName + "NV"); \
 \
    if (!alternateFuncName.isEmpty() && !(funcs->*funcPointerName)) { \
        funcs->*funcPointerName = (FuncType)context->getProcAddress(alternateFuncName); \
 \
        if ((Policy & ResolveOES) && !(funcs->*funcPointerName)) \
            funcs->*funcPointerName = (FuncType)context->getProcAddress(alternateFuncName + "OES"); \
 \
        if (!(funcs->*funcPointerName)) \
            funcs->*funcPointerName = (FuncType)context->getProcAddress(alternateFuncName + "ARB"); \
 \
        if ((Policy & ResolveEXT) && !(funcs->*funcPointerName)) \
            funcs->*funcPointerName = (FuncType)context->getProcAddress(alternateFuncName + "EXT"); \
 \
        if ((Policy & ResolveANGLE) && !(funcs->*funcPointerName)) \
            funcs->*funcPointerName = (FuncType)context->getProcAddress(funcName + "ANGLE"); \
 \
        if ((Policy & ResolveNV) && !(funcs->*funcPointerName)) \
            funcs->*funcPointerName = (FuncType)context->getProcAddress(funcName + "NV"); \
    }

#define RESOLVER_COMMON_NON_VOID \
    RESOLVER_COMMON \
 \
    if (!(funcs->*funcPointerName)) { \
        if (fallbackFuncPointer) { \
            funcs->*funcPointerName = fallbackFuncPointer; \
        } else { \
            funcs->*funcPointerName = old; \
            return ReturnType(); \
        } \
    }

#define RESOLVER_COMMON_VOID \
    RESOLVER_COMMON \
 \
    if (!(funcs->*funcPointerName)) { \
        if (fallbackFuncPointer) { \
            funcs->*funcPointerName = fallbackFuncPointer; \
        } else { \
            funcs->*funcPointerName = old; \
            return; \
        } \
    }

template <typename Base, typename FuncType, int Policy, typename ReturnType>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()()
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)();
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4, typename P5>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4, p5);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

template <typename Base, typename FuncType, int Policy, typename ReturnType> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
ReturnType Resolver<Base, FuncType, Policy, ReturnType>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10)
{
    RESOLVER_COMMON_NON_VOID

    return (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
}

template <typename Base, typename FuncType, int Policy>
void Resolver<Base, FuncType, Policy, void>::operator()()
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)();
}

template <typename Base, typename FuncType, int Policy> template <typename P1>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
}

template <typename Base, typename FuncType, int Policy> template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11>
void Resolver<Base, FuncType, Policy, void>::operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11)
{
    RESOLVER_COMMON_VOID

    (funcs->*funcPointerName)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}

template <typename ReturnType, int Policy, typename Base, typename FuncType>
Resolver<Base, FuncType, Policy, ReturnType> functionResolverWithFallback(FuncType Base::*func, FuncType fallback, const char *name,
         const char *alternate = nullptr)
{
    return Resolver<Base, FuncType, Policy, ReturnType>(func, fallback, name, alternate);
}

template <typename ReturnType, int Policy, typename Base, typename FuncType>
Resolver<Base, FuncType, Policy, ReturnType> functionResolver(FuncType Base::*func, const char *name, const char *alternate = nullptr)
{
    return Resolver<Base, FuncType, Policy, ReturnType>(func, nullptr, name, alternate);
}

} // namespace

#define RESOLVE_FUNC(RETURN_TYPE, POLICY, NAME) \
    return functionResolver<RETURN_TYPE, POLICY>(&QOpenGLExtensionsPrivate::NAME, "gl" #NAME)

#define RESOLVE_FUNC_VOID(POLICY, NAME) \
    functionResolver<void, POLICY>(&QOpenGLExtensionsPrivate::NAME, "gl" #NAME)

#define RESOLVE_FUNC_SPECIAL(RETURN_TYPE, POLICY, NAME) \
    return functionResolverWithFallback<RETURN_TYPE, POLICY>(&QOpenGLExtensionsPrivate::NAME, qopenglfSpecial##NAME, "gl" #NAME)

#define RESOLVE_FUNC_SPECIAL_VOID(POLICY, NAME) \
    functionResolverWithFallback<void, POLICY>(&QOpenGLExtensionsPrivate::NAME, qopenglfSpecial##NAME, "gl" #NAME)

#define RESOLVE_FUNC_WITH_ALTERNATE(RETURN_TYPE, POLICY, NAME, ALTERNATE) \
    return functionResolver<RETURN_TYPE, POLICY>(&QOpenGLExtensionsPrivate::NAME, "gl" #NAME, "gl" #ALTERNATE)

#define RESOLVE_FUNC_VOID_WITH_ALTERNATE(POLICY, NAME, ALTERNATE) \
    functionResolver<void, POLICY>(&QOpenGLExtensionsPrivate::NAME, "gl" #NAME, "gl" #ALTERNATE)

#ifndef QT_OPENGL_ES_2

// GLES2 + OpenGL1 common subset. These are normally not resolvable,
// but the underlying platform code may hide this limitation.

static void QOPENGLF_APIENTRY qopenglfResolveBindTexture(GLenum target, GLuint texture)
{
    RESOLVE_FUNC_VOID(0, BindTexture)(target, texture);
}

static void QOPENGLF_APIENTRY qopenglfResolveBlendFunc(GLenum sfactor, GLenum dfactor)
{
    RESOLVE_FUNC_VOID(0, BlendFunc)(sfactor, dfactor);
}

static void QOPENGLF_APIENTRY qopenglfResolveClear(GLbitfield mask)
{
    RESOLVE_FUNC_VOID(0, Clear)(mask);
}

static void QOPENGLF_APIENTRY qopenglfResolveClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    RESOLVE_FUNC_VOID(0, ClearColor)(red, green, blue, alpha);
}

static void QOPENGLF_APIENTRY qopenglfResolveClearDepthf(GLclampf depth)
{
    if (QOpenGLContext::currentContext()->isOpenGLES()) {
        RESOLVE_FUNC_VOID(0, ClearDepthf)(depth);
    } else {
        RESOLVE_FUNC_VOID(0, ClearDepth)((GLdouble) depth);
    }
}

static void QOPENGLF_APIENTRY qopenglfResolveClearStencil(GLint s)
{
    RESOLVE_FUNC_VOID(0, ClearStencil)(s);
}

static void QOPENGLF_APIENTRY qopenglfResolveColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    RESOLVE_FUNC_VOID(0, ColorMask)(red, green, blue, alpha);
}

static void QOPENGLF_APIENTRY qopenglfResolveCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    RESOLVE_FUNC_VOID(0, CopyTexImage2D)(target, level, internalformat, x, y, width, height, border);
}

static void QOPENGLF_APIENTRY qopenglfResolveCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    RESOLVE_FUNC_VOID(0, CopyTexSubImage2D)(target, level, xoffset, yoffset, x, y, width, height);
}

static void QOPENGLF_APIENTRY qopenglfResolveCullFace(GLenum mode)
{
    RESOLVE_FUNC_VOID(0, CullFace)(mode);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteTextures(GLsizei n, const GLuint* textures)
{
    RESOLVE_FUNC_VOID(0, DeleteTextures)(n, textures);
}

static void QOPENGLF_APIENTRY qopenglfResolveDepthFunc(GLenum func)
{
    RESOLVE_FUNC_VOID(0, DepthFunc)(func);
}

static void QOPENGLF_APIENTRY qopenglfResolveDepthMask(GLboolean flag)
{
    RESOLVE_FUNC_VOID(0, DepthMask)(flag);
}

static void QOPENGLF_APIENTRY qopenglfResolveDepthRangef(GLclampf zNear, GLclampf zFar)
{
    if (QOpenGLContext::currentContext()->isOpenGLES()) {
        RESOLVE_FUNC_VOID(0, DepthRangef)(zNear, zFar);
    } else {
        RESOLVE_FUNC_VOID(0, DepthRange)((GLdouble) zNear, (GLdouble) zFar);
    }
}

static void QOPENGLF_APIENTRY qopenglfResolveDisable(GLenum cap)
{
    RESOLVE_FUNC_VOID(0, Disable)(cap);
}

static void QOPENGLF_APIENTRY qopenglfResolveDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    RESOLVE_FUNC_VOID(0, DrawArrays)(mode, first, count);
}

static void QOPENGLF_APIENTRY qopenglfResolveDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    RESOLVE_FUNC_VOID(0, DrawElements)(mode, count, type, indices);
}

static void QOPENGLF_APIENTRY qopenglfResolveEnable(GLenum cap)
{
    RESOLVE_FUNC_VOID(0, Enable)(cap);
}

static void QOPENGLF_APIENTRY qopenglfResolveFinish()
{
    RESOLVE_FUNC_VOID(0, Finish)();
}

static void QOPENGLF_APIENTRY qopenglfResolveFlush()
{
    RESOLVE_FUNC_VOID(0, Flush)();
}

static void QOPENGLF_APIENTRY qopenglfResolveFrontFace(GLenum mode)
{
    RESOLVE_FUNC_VOID(0, FrontFace)(mode);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenTextures(GLsizei n, GLuint* textures)
{
    RESOLVE_FUNC_VOID(0, GenTextures)(n, textures);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetBooleanv(GLenum pname, GLboolean* params)
{
    RESOLVE_FUNC_VOID(0, GetBooleanv)(pname, params);
}

static GLenum QOPENGLF_APIENTRY qopenglfResolveGetError()
{
    RESOLVE_FUNC(GLenum, 0, GetError)();
}

static void QOPENGLF_APIENTRY qopenglfResolveGetFloatv(GLenum pname, GLfloat* params)
{
    RESOLVE_FUNC_VOID(0, GetFloatv)(pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetIntegerv(GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID(0, GetIntegerv)(pname, params);
}

static const GLubyte * QOPENGLF_APIENTRY qopenglfResolveGetString(GLenum name)
{
    RESOLVE_FUNC(const GLubyte *, 0, GetString)(name);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
    RESOLVE_FUNC_VOID(0, GetTexParameterfv)(target, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID(0, GetTexParameteriv)(target, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveHint(GLenum target, GLenum mode)
{
    RESOLVE_FUNC_VOID(0, Hint)(target, mode);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsEnabled(GLenum cap)
{
    RESOLVE_FUNC(GLboolean, 0, IsEnabled)(cap);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsTexture(GLuint texture)
{
    RESOLVE_FUNC(GLboolean, 0, IsTexture)(texture);
}

static void QOPENGLF_APIENTRY qopenglfResolveLineWidth(GLfloat width)
{
    RESOLVE_FUNC_VOID(0, LineWidth)(width);
}

static void QOPENGLF_APIENTRY qopenglfResolvePixelStorei(GLenum pname, GLint param)
{
    RESOLVE_FUNC_VOID(0, PixelStorei)(pname, param);
}

static void QOPENGLF_APIENTRY qopenglfResolvePolygonOffset(GLfloat factor, GLfloat units)
{
    RESOLVE_FUNC_VOID(0, PolygonOffset)(factor, units);
}

static void QOPENGLF_APIENTRY qopenglfResolveReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
    RESOLVE_FUNC_VOID(0, ReadPixels)(x, y, width, height, format, type, pixels);
}

static void QOPENGLF_APIENTRY qopenglfResolveScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    RESOLVE_FUNC_VOID(0, Scissor)(x, y, width, height);
}

static void QOPENGLF_APIENTRY qopenglfResolveStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    RESOLVE_FUNC_VOID(0, StencilFunc)(func, ref, mask);
}

static void QOPENGLF_APIENTRY qopenglfResolveStencilMask(GLuint mask)
{
    RESOLVE_FUNC_VOID(0, StencilMask)(mask);
}

static void QOPENGLF_APIENTRY qopenglfResolveStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    RESOLVE_FUNC_VOID(0, StencilOp)(fail, zfail, zpass);
}

static void QOPENGLF_APIENTRY qopenglfResolveTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    RESOLVE_FUNC_VOID(0, TexImage2D)(target, level, internalformat, width, height, border, format, type, pixels);
}

static void QOPENGLF_APIENTRY qopenglfResolveTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    RESOLVE_FUNC_VOID(0, TexParameterf)(target, pname, param);
}

static void QOPENGLF_APIENTRY qopenglfResolveTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
    RESOLVE_FUNC_VOID(0, TexParameterfv)(target, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveTexParameteri(GLenum target, GLenum pname, GLint param)
{
    RESOLVE_FUNC_VOID(0, TexParameteri)(target, pname, param);
}

static void QOPENGLF_APIENTRY qopenglfResolveTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
    RESOLVE_FUNC_VOID(0, TexParameteriv)(target, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    RESOLVE_FUNC_VOID(0, TexSubImage2D)(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

static void QOPENGLF_APIENTRY qopenglfResolveViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    RESOLVE_FUNC_VOID(0, Viewport)(x, y, width, height);
}

// GL(ES)2

static void QOPENGLF_APIENTRY qopenglfResolveActiveTexture(GLenum texture)
{
    RESOLVE_FUNC_VOID(0, ActiveTexture)(texture);
}

static void QOPENGLF_APIENTRY qopenglfResolveAttachShader(GLuint program, GLuint shader)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, AttachShader, AttachObject)(program, shader);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindAttribLocation(GLuint program, GLuint index, const char* name)
{
    RESOLVE_FUNC_VOID(0, BindAttribLocation)(program, index, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindBuffer(GLenum target, GLuint buffer)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BindBuffer)(target, buffer);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindFramebuffer(GLenum target, GLuint framebuffer)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BindFramebuffer)(target, framebuffer);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BindRenderbuffer)(target, renderbuffer);
}

static void QOPENGLF_APIENTRY qopenglfResolveBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BlendColor)(red, green, blue, alpha);
}

static void QOPENGLF_APIENTRY qopenglfResolveBlendEquation(GLenum mode)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BlendEquation)(mode);
}

static void QOPENGLF_APIENTRY qopenglfResolveBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BlendEquationSeparate)(modeRGB, modeAlpha);
}

static void QOPENGLF_APIENTRY qopenglfResolveBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BlendFuncSeparate)(srcRGB, dstRGB, srcAlpha, dstAlpha);
}

static void QOPENGLF_APIENTRY qopenglfResolveBufferData(GLenum target, qopengl_GLsizeiptr size, const void* data, GLenum usage)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BufferData)(target, size, data, usage);
}

static void QOPENGLF_APIENTRY qopenglfResolveBufferSubData(GLenum target, qopengl_GLintptr offset, qopengl_GLsizeiptr size, const void* data)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, BufferSubData)(target, offset, size, data);
}

static GLenum QOPENGLF_APIENTRY qopenglfResolveCheckFramebufferStatus(GLenum target)
{
    RESOLVE_FUNC(GLenum, ResolveOES | ResolveEXT, CheckFramebufferStatus)(target);
}

static void QOPENGLF_APIENTRY qopenglfResolveCompileShader(GLuint shader)
{
    RESOLVE_FUNC_VOID(0, CompileShader)(shader);
}

static void QOPENGLF_APIENTRY qopenglfResolveCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, CompressedTexImage2D)(target, level, internalformat, width, height, border, imageSize, data);
}

static void QOPENGLF_APIENTRY qopenglfResolveCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, CompressedTexSubImage2D)(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

static GLuint QOPENGLF_APIENTRY qopenglfResolveCreateProgram()
{
    RESOLVE_FUNC_WITH_ALTERNATE(GLuint, 0, CreateProgram, CreateProgramObject)();
}

static GLuint QOPENGLF_APIENTRY qopenglfResolveCreateShader(GLenum type)
{
    RESOLVE_FUNC_WITH_ALTERNATE(GLuint, 0, CreateShader, CreateShaderObject)(type);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteBuffers(GLsizei n, const GLuint* buffers)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, DeleteBuffers)(n, buffers);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, DeleteFramebuffers)(n, framebuffers);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteProgram(GLuint program)
{
    RESOLVE_FUNC_VOID(0, DeleteProgram)(program);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, DeleteRenderbuffers)(n, renderbuffers);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteShader(GLuint shader)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, DeleteShader, DeleteObject)(shader);
}

static void QOPENGLF_APIENTRY qopenglfResolveDetachShader(GLuint program, GLuint shader)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, DetachShader, DetachObject)(program, shader);
}

static void QOPENGLF_APIENTRY qopenglfResolveDisableVertexAttribArray(GLuint index)
{
    RESOLVE_FUNC_VOID(0, DisableVertexAttribArray)(index);
}

static void QOPENGLF_APIENTRY qopenglfResolveEnableVertexAttribArray(GLuint index)
{
    RESOLVE_FUNC_VOID(0, EnableVertexAttribArray)(index);
}

static void QOPENGLF_APIENTRY qopenglfResolveFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, FramebufferRenderbuffer)(target, attachment, renderbuffertarget, renderbuffer);
}

static void QOPENGLF_APIENTRY qopenglfResolveFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, FramebufferTexture2D)(target, attachment, textarget, texture, level);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenBuffers(GLsizei n, GLuint* buffers)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GenBuffers)(n, buffers);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenerateMipmap(GLenum target)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GenerateMipmap)(target);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GenFramebuffers)(n, framebuffers);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GenRenderbuffers)(n, renderbuffers);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name)
{
    RESOLVE_FUNC_VOID(0, GetActiveAttrib)(program, index, bufsize, length, size, type, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name)
{
    RESOLVE_FUNC_VOID(0, GetActiveUniform)(program, index, bufsize, length, size, type, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, GetAttachedShaders, GetAttachedObjects)(program, maxcount, count, shaders);
}

static GLint QOPENGLF_APIENTRY qopenglfResolveGetAttribLocation(GLuint program, const char* name)
{
    RESOLVE_FUNC(GLint, 0, GetAttribLocation)(program, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GetBufferParameteriv)(target, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GetFramebufferAttachmentParameteriv)(target, attachment, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, GetProgramiv, GetObjectParameteriv)(program, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, char* infolog)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, GetProgramInfoLog, GetInfoLog)(program, bufsize, length, infolog);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, GetRenderbufferParameteriv)(target, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, GetShaderiv, GetObjectParameteriv)(shader, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog)
{
    RESOLVE_FUNC_VOID_WITH_ALTERNATE(0, GetShaderInfoLog, GetInfoLog)(shader, bufsize, length, infolog);
}

static void QOPENGLF_APIENTRY qopenglfSpecialGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    (void) shadertype;
    (void) precisiontype;

    range[0] = range[1] = precision[0] = 0;
}

static void QOPENGLF_APIENTRY qopenglfResolveGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    RESOLVE_FUNC_SPECIAL_VOID(ResolveOES | ResolveEXT, GetShaderPrecisionFormat)(shadertype, precisiontype, range, precision);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, char* source)
{
    RESOLVE_FUNC_VOID(0, GetShaderSource)(shader, bufsize, length, source);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetUniformfv(GLuint program, GLint location, GLfloat* params)
{
    RESOLVE_FUNC_VOID(0, GetUniformfv)(program, location, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetUniformiv(GLuint program, GLint location, GLint* params)
{
    RESOLVE_FUNC_VOID(0, GetUniformiv)(program, location, params);
}

static GLint QOPENGLF_APIENTRY qopenglfResolveGetUniformLocation(GLuint program, const char* name)
{
    RESOLVE_FUNC(GLint, 0, GetUniformLocation)(program, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
    RESOLVE_FUNC_VOID(0, GetVertexAttribfv)(index, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
    RESOLVE_FUNC_VOID(0, GetVertexAttribiv)(index, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer)
{
    RESOLVE_FUNC_VOID(0, GetVertexAttribPointerv)(index, pname, pointer);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsBuffer(GLuint buffer)
{
    RESOLVE_FUNC(GLboolean, ResolveOES | ResolveEXT, IsBuffer)(buffer);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsFramebuffer(GLuint framebuffer)
{
    RESOLVE_FUNC(GLboolean, ResolveOES | ResolveEXT, IsFramebuffer)(framebuffer);
}

static GLboolean QOPENGLF_APIENTRY qopenglfSpecialIsProgram(GLuint program)
{
    return program != 0;
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsProgram(GLuint program)
{
    RESOLVE_FUNC_SPECIAL(GLboolean, 0, IsProgram)(program);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsRenderbuffer(GLuint renderbuffer)
{
    RESOLVE_FUNC(GLboolean, ResolveOES | ResolveEXT, IsRenderbuffer)(renderbuffer);
}

static GLboolean QOPENGLF_APIENTRY qopenglfSpecialIsShader(GLuint shader)
{
    return shader != 0;
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsShader(GLuint shader)
{
    RESOLVE_FUNC_SPECIAL(GLboolean, 0, IsShader)(shader);
}

static void QOPENGLF_APIENTRY qopenglfResolveLinkProgram(GLuint program)
{
    RESOLVE_FUNC_VOID(0, LinkProgram)(program);
}

static void QOPENGLF_APIENTRY qopenglfSpecialReleaseShaderCompiler()
{
}

static void QOPENGLF_APIENTRY qopenglfResolveReleaseShaderCompiler()
{
    RESOLVE_FUNC_SPECIAL_VOID(0, ReleaseShaderCompiler)();
}

static void QOPENGLF_APIENTRY qopenglfResolveRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, RenderbufferStorage)(target, internalformat, width, height);
}

static void QOPENGLF_APIENTRY qopenglfResolveSampleCoverage(GLclampf value, GLboolean invert)
{
    RESOLVE_FUNC_VOID(ResolveOES | ResolveEXT, SampleCoverage)(value, invert);
}

static void QOPENGLF_APIENTRY qopenglfResolveShaderBinary(GLint n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLint length)
{
    RESOLVE_FUNC_VOID(0, ShaderBinary)(n, shaders, binaryformat, binary, length);
}

static void QOPENGLF_APIENTRY qopenglfResolveShaderSource(GLuint shader, GLsizei count, const char** string, const GLint* length)
{
    RESOLVE_FUNC_VOID(0, ShaderSource)(shader, count, string, length);
}

static void QOPENGLF_APIENTRY qopenglfResolveStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    RESOLVE_FUNC_VOID(ResolveEXT, StencilFuncSeparate)(face, func, ref, mask);
}

static void QOPENGLF_APIENTRY qopenglfResolveStencilMaskSeparate(GLenum face, GLuint mask)
{
    RESOLVE_FUNC_VOID(ResolveEXT, StencilMaskSeparate)(face, mask);
}

static void QOPENGLF_APIENTRY qopenglfResolveStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    RESOLVE_FUNC_VOID(ResolveEXT, StencilOpSeparate)(face, fail, zfail, zpass);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform1f(GLint location, GLfloat x)
{
    RESOLVE_FUNC_VOID(0, Uniform1f)(location, x);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
    RESOLVE_FUNC_VOID(0, Uniform1fv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform1i(GLint location, GLint x)
{
    RESOLVE_FUNC_VOID(0, Uniform1i)(location, x);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform1iv(GLint location, GLsizei count, const GLint* v)
{
    RESOLVE_FUNC_VOID(0, Uniform1iv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform2f(GLint location, GLfloat x, GLfloat y)
{
    RESOLVE_FUNC_VOID(0, Uniform2f)(location, x, y);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform2fv(GLint location, GLsizei count, const GLfloat* v)
{
    RESOLVE_FUNC_VOID(0, Uniform2fv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform2i(GLint location, GLint x, GLint y)
{
    RESOLVE_FUNC_VOID(0, Uniform2i)(location, x, y);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform2iv(GLint location, GLsizei count, const GLint* v)
{
    RESOLVE_FUNC_VOID(0, Uniform2iv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    RESOLVE_FUNC_VOID(0, Uniform3f)(location, x, y, z);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform3fv(GLint location, GLsizei count, const GLfloat* v)
{
    RESOLVE_FUNC_VOID(0, Uniform3fv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform3i(GLint location, GLint x, GLint y, GLint z)
{
    RESOLVE_FUNC_VOID(0, Uniform3i)(location, x, y, z);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform3iv(GLint location, GLsizei count, const GLint* v)
{
    RESOLVE_FUNC_VOID(0, Uniform3iv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    RESOLVE_FUNC_VOID(0, Uniform4f)(location, x, y, z, w);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform4fv(GLint location, GLsizei count, const GLfloat* v)
{
    RESOLVE_FUNC_VOID(0, Uniform4fv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
    RESOLVE_FUNC_VOID(0, Uniform4i)(location, x, y, z, w);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform4iv(GLint location, GLsizei count, const GLint* v)
{
    RESOLVE_FUNC_VOID(0, Uniform4iv)(location, count, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    RESOLVE_FUNC_VOID(0, UniformMatrix2fv)(location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    RESOLVE_FUNC_VOID(0, UniformMatrix3fv)(location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    RESOLVE_FUNC_VOID(0, UniformMatrix4fv)(location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUseProgram(GLuint program)
{
    RESOLVE_FUNC_VOID(0, UseProgram)(program);
}

static void QOPENGLF_APIENTRY qopenglfResolveValidateProgram(GLuint program)
{
    RESOLVE_FUNC_VOID(0, ValidateProgram)(program);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib1f(GLuint indx, GLfloat x)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib1f)(indx, x);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib1fv(GLuint indx, const GLfloat* values)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib1fv)(indx, values);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib2f)(indx, x, y);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib2fv(GLuint indx, const GLfloat* values)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib2fv)(indx, values);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib3f)(indx, x, y, z);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib3fv(GLuint indx, const GLfloat* values)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib3fv)(indx, values);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib4f)(indx, x, y, z, w);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttrib4fv(GLuint indx, const GLfloat* values)
{
    RESOLVE_FUNC_VOID(0, VertexAttrib4fv)(indx, values);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr)
{
    RESOLVE_FUNC_VOID(0, VertexAttribPointer)(indx, size, type, normalized, stride, ptr);
}

#endif // !QT_OPENGL_ES_2

// Extensions not standard in any ES version

static GLvoid *QOPENGLF_APIENTRY qopenglfResolveMapBuffer(GLenum target, GLenum access)
{
    // It is possible that GL_OES_map_buffer is present, but then having to
    // differentiate between glUnmapBufferOES and glUnmapBuffer causes extra
    // headache. QOpenGLBuffer::map() will handle this automatically, while direct
    // calls are better off with migrating to the standard glMapBufferRange.
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (ctx->isOpenGLES() && ctx->format().majorVersion() >= 3) {
        qWarning("QOpenGLFunctions: glMapBuffer is not available in OpenGL ES 3.0 and up. Use glMapBufferRange instead.");
        return nullptr;
    } else {
        RESOLVE_FUNC(GLvoid *, ResolveOES, MapBuffer)(target, access);
    }
}

static void QOPENGLF_APIENTRY qopenglfResolveGetBufferSubData(GLenum target, qopengl_GLintptr offset, qopengl_GLsizeiptr size, GLvoid *data)
{
    RESOLVE_FUNC_VOID(ResolveEXT, GetBufferSubData)
        (target, offset, size, data);
}

static void QOPENGLF_APIENTRY qopenglfResolveDiscardFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    RESOLVE_FUNC_VOID(ResolveEXT, DiscardFramebuffer)(target, numAttachments, attachments);
}

#if !defined(QT_OPENGL_ES_2) && !defined(QT_OPENGL_DYNAMIC)
// Special translation functions for ES-specific calls on desktop GL

static void QOPENGLF_APIENTRY qopenglfTranslateClearDepthf(GLclampf depth)
{
    ::glClearDepth(depth);
}

static void QOPENGLF_APIENTRY qopenglfTranslateDepthRangef(GLclampf zNear, GLclampf zFar)
{
    ::glDepthRange(zNear, zFar);
}
#endif // !ES && !DYNAMIC

QOpenGLFunctionsPrivate::QOpenGLFunctionsPrivate(QOpenGLContext *)
{
    /* Assign a pointer to an above defined static function
     * which on first call resolves the function from the current
     * context, assigns it to the member variable and executes it
     * (see Resolver template) */
#ifndef QT_OPENGL_ES_2
    // The GL1 functions may not be queriable via getProcAddress().
    if (QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::AllGLFunctionsQueryable)) {
        // The platform plugin supports resolving these.
        BindTexture = qopenglfResolveBindTexture;
        BlendFunc = qopenglfResolveBlendFunc;
        Clear = qopenglfResolveClear;
        ClearColor = qopenglfResolveClearColor;
        ClearDepthf = qopenglfResolveClearDepthf;
        ClearStencil = qopenglfResolveClearStencil;
        ColorMask = qopenglfResolveColorMask;
        CopyTexImage2D = qopenglfResolveCopyTexImage2D;
        CopyTexSubImage2D = qopenglfResolveCopyTexSubImage2D;
        CullFace = qopenglfResolveCullFace;
        DeleteTextures = qopenglfResolveDeleteTextures;
        DepthFunc = qopenglfResolveDepthFunc;
        DepthMask = qopenglfResolveDepthMask;
        DepthRangef = qopenglfResolveDepthRangef;
        Disable = qopenglfResolveDisable;
        DrawArrays = qopenglfResolveDrawArrays;
        DrawElements = qopenglfResolveDrawElements;
        Enable = qopenglfResolveEnable;
        Finish = qopenglfResolveFinish;
        Flush = qopenglfResolveFlush;
        FrontFace = qopenglfResolveFrontFace;
        GenTextures = qopenglfResolveGenTextures;
        GetBooleanv = qopenglfResolveGetBooleanv;
        GetError = qopenglfResolveGetError;
        GetFloatv = qopenglfResolveGetFloatv;
        GetIntegerv = qopenglfResolveGetIntegerv;
        GetString = qopenglfResolveGetString;
        GetTexParameterfv = qopenglfResolveGetTexParameterfv;
        GetTexParameteriv = qopenglfResolveGetTexParameteriv;
        Hint = qopenglfResolveHint;
        IsEnabled = qopenglfResolveIsEnabled;
        IsTexture = qopenglfResolveIsTexture;
        LineWidth = qopenglfResolveLineWidth;
        PixelStorei = qopenglfResolvePixelStorei;
        PolygonOffset = qopenglfResolvePolygonOffset;
        ReadPixels = qopenglfResolveReadPixels;
        Scissor = qopenglfResolveScissor;
        StencilFunc = qopenglfResolveStencilFunc;
        StencilMask = qopenglfResolveStencilMask;
        StencilOp = qopenglfResolveStencilOp;
        TexImage2D = qopenglfResolveTexImage2D;
        TexParameterf = qopenglfResolveTexParameterf;
        TexParameterfv = qopenglfResolveTexParameterfv;
        TexParameteri = qopenglfResolveTexParameteri;
        TexParameteriv = qopenglfResolveTexParameteriv;
        TexSubImage2D = qopenglfResolveTexSubImage2D;
        Viewport = qopenglfResolveViewport;

    } else {

#ifndef QT_OPENGL_DYNAMIC
        // Use the functions directly. This requires linking QtGui to an OpenGL implementation.
        BindTexture = ::glBindTexture;
        BlendFunc = ::glBlendFunc;
        Clear = ::glClear;
        ClearColor = ::glClearColor;
        ClearDepthf = qopenglfTranslateClearDepthf;
        ClearStencil = ::glClearStencil;
        ColorMask = ::glColorMask;
        CopyTexImage2D = ::glCopyTexImage2D;
        CopyTexSubImage2D = ::glCopyTexSubImage2D;
        CullFace = ::glCullFace;
        DeleteTextures = ::glDeleteTextures;
        DepthFunc = ::glDepthFunc;
        DepthMask = ::glDepthMask;
        DepthRangef = qopenglfTranslateDepthRangef;
        Disable = ::glDisable;
        DrawArrays = ::glDrawArrays;
        DrawElements = ::glDrawElements;
        Enable = ::glEnable;
        Finish = ::glFinish;
        Flush = ::glFlush;
        FrontFace = ::glFrontFace;
        GenTextures = ::glGenTextures;
        GetBooleanv = ::glGetBooleanv;
        GetError = ::glGetError;
        GetFloatv = ::glGetFloatv;
        GetIntegerv = ::glGetIntegerv;
        GetString = ::glGetString;
        GetTexParameterfv = ::glGetTexParameterfv;
        GetTexParameteriv = ::glGetTexParameteriv;
        Hint = ::glHint;
        IsEnabled = ::glIsEnabled;
        IsTexture = ::glIsTexture;
        LineWidth = ::glLineWidth;
        PixelStorei = ::glPixelStorei;
        PolygonOffset = ::glPolygonOffset;
        ReadPixels = ::glReadPixels;
        Scissor = ::glScissor;
        StencilFunc = ::glStencilFunc;
        StencilMask = ::glStencilMask;
        StencilOp = ::glStencilOp;
        TexImage2D = glTexImage2D;
        TexParameterf = ::glTexParameterf;
        TexParameterfv = ::glTexParameterfv;
        TexParameteri = ::glTexParameteri;
        TexParameteriv = ::glTexParameteriv;
        TexSubImage2D = ::glTexSubImage2D;
        Viewport = ::glViewport;
#else // QT_OPENGL_DYNAMIC

        // This should not happen.
        qFatal("QOpenGLFunctions: Dynamic OpenGL builds do not support platforms with insufficient function resolving capabilities");
#endif
    }

    ActiveTexture = qopenglfResolveActiveTexture;
    AttachShader = qopenglfResolveAttachShader;
    BindAttribLocation = qopenglfResolveBindAttribLocation;
    BindBuffer = qopenglfResolveBindBuffer;
    BindFramebuffer = qopenglfResolveBindFramebuffer;
    BindRenderbuffer = qopenglfResolveBindRenderbuffer;
    BlendColor = qopenglfResolveBlendColor;
    BlendEquation = qopenglfResolveBlendEquation;
    BlendEquationSeparate = qopenglfResolveBlendEquationSeparate;
    BlendFuncSeparate = qopenglfResolveBlendFuncSeparate;
    BufferData = qopenglfResolveBufferData;
    BufferSubData = qopenglfResolveBufferSubData;
    CheckFramebufferStatus = qopenglfResolveCheckFramebufferStatus;
    CompileShader = qopenglfResolveCompileShader;
    CompressedTexImage2D = qopenglfResolveCompressedTexImage2D;
    CompressedTexSubImage2D = qopenglfResolveCompressedTexSubImage2D;
    CreateProgram = qopenglfResolveCreateProgram;
    CreateShader = qopenglfResolveCreateShader;
    DeleteBuffers = qopenglfResolveDeleteBuffers;
    DeleteFramebuffers = qopenglfResolveDeleteFramebuffers;
    DeleteProgram = qopenglfResolveDeleteProgram;
    DeleteRenderbuffers = qopenglfResolveDeleteRenderbuffers;
    DeleteShader = qopenglfResolveDeleteShader;
    DetachShader = qopenglfResolveDetachShader;
    DisableVertexAttribArray = qopenglfResolveDisableVertexAttribArray;
    EnableVertexAttribArray = qopenglfResolveEnableVertexAttribArray;
    FramebufferRenderbuffer = qopenglfResolveFramebufferRenderbuffer;
    FramebufferTexture2D = qopenglfResolveFramebufferTexture2D;
    GenBuffers = qopenglfResolveGenBuffers;
    GenerateMipmap = qopenglfResolveGenerateMipmap;
    GenFramebuffers = qopenglfResolveGenFramebuffers;
    GenRenderbuffers = qopenglfResolveGenRenderbuffers;
    GetActiveAttrib = qopenglfResolveGetActiveAttrib;
    GetActiveUniform = qopenglfResolveGetActiveUniform;
    GetAttachedShaders = qopenglfResolveGetAttachedShaders;
    GetAttribLocation = qopenglfResolveGetAttribLocation;
    GetBufferParameteriv = qopenglfResolveGetBufferParameteriv;
    GetFramebufferAttachmentParameteriv = qopenglfResolveGetFramebufferAttachmentParameteriv;
    GetProgramiv = qopenglfResolveGetProgramiv;
    GetProgramInfoLog = qopenglfResolveGetProgramInfoLog;
    GetRenderbufferParameteriv = qopenglfResolveGetRenderbufferParameteriv;
    GetShaderiv = qopenglfResolveGetShaderiv;
    GetShaderInfoLog = qopenglfResolveGetShaderInfoLog;
    GetShaderPrecisionFormat = qopenglfResolveGetShaderPrecisionFormat;
    GetShaderSource = qopenglfResolveGetShaderSource;
    GetUniformfv = qopenglfResolveGetUniformfv;
    GetUniformiv = qopenglfResolveGetUniformiv;
    GetUniformLocation = qopenglfResolveGetUniformLocation;
    GetVertexAttribfv = qopenglfResolveGetVertexAttribfv;
    GetVertexAttribiv = qopenglfResolveGetVertexAttribiv;
    GetVertexAttribPointerv = qopenglfResolveGetVertexAttribPointerv;
    IsBuffer = qopenglfResolveIsBuffer;
    IsFramebuffer = qopenglfResolveIsFramebuffer;
    IsProgram = qopenglfResolveIsProgram;
    IsRenderbuffer = qopenglfResolveIsRenderbuffer;
    IsShader = qopenglfResolveIsShader;
    LinkProgram = qopenglfResolveLinkProgram;
    ReleaseShaderCompiler = qopenglfResolveReleaseShaderCompiler;
    RenderbufferStorage = qopenglfResolveRenderbufferStorage;
    SampleCoverage = qopenglfResolveSampleCoverage;
    ShaderBinary = qopenglfResolveShaderBinary;
    ShaderSource = qopenglfResolveShaderSource;
    StencilFuncSeparate = qopenglfResolveStencilFuncSeparate;
    StencilMaskSeparate = qopenglfResolveStencilMaskSeparate;
    StencilOpSeparate = qopenglfResolveStencilOpSeparate;
    Uniform1f = qopenglfResolveUniform1f;
    Uniform1fv = qopenglfResolveUniform1fv;
    Uniform1i = qopenglfResolveUniform1i;
    Uniform1iv = qopenglfResolveUniform1iv;
    Uniform2f = qopenglfResolveUniform2f;
    Uniform2fv = qopenglfResolveUniform2fv;
    Uniform2i = qopenglfResolveUniform2i;
    Uniform2iv = qopenglfResolveUniform2iv;
    Uniform3f = qopenglfResolveUniform3f;
    Uniform3fv = qopenglfResolveUniform3fv;
    Uniform3i = qopenglfResolveUniform3i;
    Uniform3iv = qopenglfResolveUniform3iv;
    Uniform4f = qopenglfResolveUniform4f;
    Uniform4fv = qopenglfResolveUniform4fv;
    Uniform4i = qopenglfResolveUniform4i;
    Uniform4iv = qopenglfResolveUniform4iv;
    UniformMatrix2fv = qopenglfResolveUniformMatrix2fv;
    UniformMatrix3fv = qopenglfResolveUniformMatrix3fv;
    UniformMatrix4fv = qopenglfResolveUniformMatrix4fv;
    UseProgram = qopenglfResolveUseProgram;
    ValidateProgram = qopenglfResolveValidateProgram;
    VertexAttrib1f = qopenglfResolveVertexAttrib1f;
    VertexAttrib1fv = qopenglfResolveVertexAttrib1fv;
    VertexAttrib2f = qopenglfResolveVertexAttrib2f;
    VertexAttrib2fv = qopenglfResolveVertexAttrib2fv;
    VertexAttrib3f = qopenglfResolveVertexAttrib3f;
    VertexAttrib3fv = qopenglfResolveVertexAttrib3fv;
    VertexAttrib4f = qopenglfResolveVertexAttrib4f;
    VertexAttrib4fv = qopenglfResolveVertexAttrib4fv;
    VertexAttribPointer = qopenglfResolveVertexAttribPointer;
#endif // !QT_OPENGL_ES_2
}

// Functions part of the OpenGL ES 3.0+ standard need special handling. These, just like
// the 2.0 functions, are not guaranteed to be resolvable via eglGetProcAddress or
// similar. (we cannot count on EGL_KHR_(client_)get_all_proc_addresses being available)

// Calling them directly is, unlike the 2.0 functions, not feasible because one may build
// the binaries on a GLES3-capable system and then deploy on a GLES2-only system that does
// not have these symbols, and vice versa. Until ES3 becomes universally available, they
// have to be dlsym'ed.

static QOpenGLES3Helper *qgles3Helper()
{
   static QOpenGLES3Helper retval;
   return &retval;
}

bool QOpenGLES3Helper::init()
{
# ifdef Q_OS_WIN
   m_gl.setFileName("libGLESv2");

# else
   m_gl.setFileNameAndVersion("GLESv2", 2);

# endif

   return m_gl.load();
}

QOpenGLES3Helper::FP_Void QOpenGLES3Helper::resolve(const QString &name)
{
   return FP_Void(m_gl.resolve(name));
}

QOpenGLES3Helper::QOpenGLES3Helper()
{
    m_supportedVersion = qMakePair(2, 0);

    if (init()) {
        const QPair<int, int> contextVersion = QOpenGLContext::currentContext()->format().version();

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
        qDebug("Resolving OpenGL ES 3.0 entry points");
#endif

        BeginQuery = (void (QOPENGLF_APIENTRYP) (GLenum, GLuint)) resolve("glBeginQuery");
        BeginTransformFeedback = (void (QOPENGLF_APIENTRYP) (GLenum)) resolve("glBeginTransformFeedback");
        BindBufferBase = (void (QOPENGLF_APIENTRYP) (GLenum, GLuint, GLuint)) resolve("glBindBufferBase");
        BindBufferRange = (void (QOPENGLF_APIENTRYP) (GLenum, GLuint, GLuint, GLintptr, GLsizeiptr)) resolve("glBindBufferRange");
        BindSampler = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint)) resolve("glBindSampler");
        BindTransformFeedback = (void (QOPENGLF_APIENTRYP) (GLenum, GLuint)) resolve("glBindTransformFeedback");
        BindVertexArray = (void (QOPENGLF_APIENTRYP) (GLuint)) resolve("glBindVertexArray");
        BlitFramebuffer = (void (QOPENGLF_APIENTRYP) (GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum)) resolve("glBlitFramebuffer");
        ClearBufferfi = (void (QOPENGLF_APIENTRYP) (GLenum, GLint, GLfloat, GLint)) resolve("glClearBufferfi");
        ClearBufferfv = (void (QOPENGLF_APIENTRYP) (GLenum, GLint, const GLfloat *)) resolve("glClearBufferfv");
        ClearBufferiv = (void (QOPENGLF_APIENTRYP) (GLenum, GLint, const GLint *)) resolve("glClearBufferiv");
        ClearBufferuiv = (void (QOPENGLF_APIENTRYP) (GLenum, GLint, const GLuint *)) resolve("glClearBufferuiv");
        ClientWaitSync = (GLenum (QOPENGLF_APIENTRYP) (GLsync, GLbitfield, GLuint64)) resolve("glClientWaitSync");
        CompressedTexImage3D = (void (QOPENGLF_APIENTRYP) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void *)) resolve("glCompressedTexImage3D");
        CompressedTexSubImage3D = (void (QOPENGLF_APIENTRYP) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void *)) resolve("glCompressedTexSubImage3D");
        CopyBufferSubData = (void (QOPENGLF_APIENTRYP) (GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr)) resolve("glCopyBufferSubData");
        CopyTexSubImage3D = (void (QOPENGLF_APIENTRYP) (GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei)) resolve("glCopyTexSubImage3D");
        DeleteQueries = (void (QOPENGLF_APIENTRYP) (GLsizei, const GLuint *)) resolve("glDeleteQueries");
        DeleteSamplers = (void (QOPENGLF_APIENTRYP) (GLsizei, const GLuint *)) resolve("glDeleteSamplers");
        DeleteSync = (void (QOPENGLF_APIENTRYP) (GLsync)) resolve("glDeleteSync");
        DeleteTransformFeedbacks = (void (QOPENGLF_APIENTRYP) (GLsizei, const GLuint *)) resolve("glDeleteTransformFeedbacks");
        DeleteVertexArrays = (void (QOPENGLF_APIENTRYP) (GLsizei, const GLuint *)) resolve("glDeleteVertexArrays");
        DrawArraysInstanced = (void (QOPENGLF_APIENTRYP) (GLenum, GLint, GLsizei, GLsizei)) resolve("glDrawArraysInstanced");
        DrawBuffers = (void (QOPENGLF_APIENTRYP) (GLsizei, const GLenum *)) resolve("glDrawBuffers");
        DrawElementsInstanced = (void (QOPENGLF_APIENTRYP) (GLenum, GLsizei, GLenum, const void *, GLsizei)) resolve("glDrawElementsInstanced");
        DrawRangeElements = (void (QOPENGLF_APIENTRYP) (GLenum, GLuint, GLuint, GLsizei, GLenum, const void *)) resolve("glDrawRangeElements");
        EndQuery = (void (QOPENGLF_APIENTRYP) (GLenum)) resolve("glEndQuery");
        EndTransformFeedback = (void (QOPENGLF_APIENTRYP) ()) resolve("glEndTransformFeedback");
        FenceSync = (GLsync (QOPENGLF_APIENTRYP) (GLenum, GLbitfield)) resolve("glFenceSync");
        FlushMappedBufferRange = (void (QOPENGLF_APIENTRYP) (GLenum, GLintptr, GLsizeiptr)) resolve("glFlushMappedBufferRange");
        FramebufferTextureLayer = (void (QOPENGLF_APIENTRYP) (GLenum, GLenum, GLuint, GLint, GLint)) resolve("glFramebufferTextureLayer");
        GenQueries = (void (QOPENGLF_APIENTRYP) (GLsizei, GLuint*)) resolve("glGenQueries");
        GenSamplers = (void (QOPENGLF_APIENTRYP) (GLsizei, GLuint*)) resolve("glGenSamplers");
        GenTransformFeedbacks = (void (QOPENGLF_APIENTRYP) (GLsizei, GLuint*)) resolve("glGenTransformFeedbacks");
        GenVertexArrays = (void (QOPENGLF_APIENTRYP) (GLsizei, GLuint*)) resolve("glGenVertexArrays");
        GetActiveUniformBlockName = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint, GLsizei, GLsizei*, GLchar*)) resolve("glGetActiveUniformBlockName");
        GetActiveUniformBlockiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint, GLenum, GLint*)) resolve("glGetActiveUniformBlockiv");
        GetActiveUniformsiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLsizei, const GLuint *, GLenum, GLint*)) resolve("glGetActiveUniformsiv");
        GetBufferParameteri64v = (void (QOPENGLF_APIENTRYP) (GLenum, GLenum, GLint64*)) resolve("glGetBufferParameteri64v");
        GetBufferPointerv = (void (QOPENGLF_APIENTRYP) (GLenum, GLenum, void **)) resolve("glGetBufferPointerv");
        GetFragDataLocation = (GLint (QOPENGLF_APIENTRYP) (GLuint, const GLchar *)) resolve("glGetFragDataLocation");
        GetInteger64i_v = (void (QOPENGLF_APIENTRYP) (GLenum, GLuint, GLint64*)) resolve("glGetInteger64i_v");
        GetInteger64v = (void (QOPENGLF_APIENTRYP) (GLenum, GLint64*)) resolve("glGetInteger64v");
        GetIntegeri_v = (void (QOPENGLF_APIENTRYP) (GLenum, GLuint, GLint*)) resolve("glGetIntegeri_v");
        GetInternalformativ = (void (QOPENGLF_APIENTRYP) (GLenum, GLenum, GLenum, GLsizei, GLint*)) resolve("glGetInternalformativ");
        GetProgramBinary = (void (QOPENGLF_APIENTRYP) (GLuint, GLsizei, GLsizei*, GLenum*, void *)) resolve("glGetProgramBinary");
        GetQueryObjectuiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, GLuint*)) resolve("glGetQueryObjectuiv");
        GetQueryiv = (void (QOPENGLF_APIENTRYP) (GLenum, GLenum, GLint*)) resolve("glGetQueryiv");
        GetSamplerParameterfv = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, GLfloat*)) resolve("glGetSamplerParameterfv");
        GetSamplerParameteriv = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, GLint*)) resolve("glGetSamplerParameteriv");
        GetStringi = (const GLubyte * (QOPENGLF_APIENTRYP) (GLenum, GLuint)) resolve("glGetStringi");
        GetSynciv = (void (QOPENGLF_APIENTRYP) (GLsync, GLenum, GLsizei, GLsizei*, GLint*)) resolve("glGetSynciv");
        GetTransformFeedbackVarying = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*)) resolve("glGetTransformFeedbackVarying");
        GetUniformBlockIndex = (GLuint (QOPENGLF_APIENTRYP) (GLuint, const GLchar *)) resolve("glGetUniformBlockIndex");
        GetUniformIndices = (void (QOPENGLF_APIENTRYP) (GLuint, GLsizei, const GLchar *const*, GLuint*)) resolve("glGetUniformIndices");
        GetUniformuiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLuint*)) resolve("glGetUniformuiv");
        GetVertexAttribIiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, GLint*)) resolve("glGetVertexAttribIiv");
        GetVertexAttribIuiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, GLuint*)) resolve("glGetVertexAttribIuiv");
        InvalidateFramebuffer = (void (QOPENGLF_APIENTRYP) (GLenum, GLsizei, const GLenum *)) resolve("glInvalidateFramebuffer");
        InvalidateSubFramebuffer = (void (QOPENGLF_APIENTRYP) (GLenum, GLsizei, const GLenum *, GLint, GLint, GLsizei, GLsizei)) resolve("glInvalidateSubFramebuffer");
        IsQuery = (GLboolean (QOPENGLF_APIENTRYP) (GLuint)) resolve("glIsQuery");
        IsSampler = (GLboolean (QOPENGLF_APIENTRYP) (GLuint)) resolve("glIsSampler");
        IsSync = (GLboolean (QOPENGLF_APIENTRYP) (GLsync)) resolve("glIsSync");
        IsTransformFeedback = (GLboolean (QOPENGLF_APIENTRYP) (GLuint)) resolve("glIsTransformFeedback");
        IsVertexArray = (GLboolean (QOPENGLF_APIENTRYP) (GLuint)) resolve("glIsVertexArray");
        MapBufferRange = (void * (QOPENGLF_APIENTRYP) (GLenum, GLintptr, GLsizeiptr, GLbitfield)) resolve("glMapBufferRange");
        PauseTransformFeedback = (void (QOPENGLF_APIENTRYP) ()) resolve("glPauseTransformFeedback");
        ProgramBinary = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, const void *, GLsizei)) resolve("glProgramBinary");
        ProgramParameteri = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, GLint)) resolve("glProgramParameteri");
        ReadBuffer = (void (QOPENGLF_APIENTRYP) (GLenum)) resolve("glReadBuffer");
        RenderbufferStorageMultisample = (void (QOPENGLF_APIENTRYP) (GLenum, GLsizei, GLenum, GLsizei, GLsizei)) resolve("glRenderbufferStorageMultisample");
        ResumeTransformFeedback = (void (QOPENGLF_APIENTRYP) ()) resolve("glResumeTransformFeedback");
        SamplerParameterf = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, GLfloat)) resolve("glSamplerParameterf");
        SamplerParameterfv = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, const GLfloat *)) resolve("glSamplerParameterfv");
        SamplerParameteri = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, GLint)) resolve("glSamplerParameteri");
        SamplerParameteriv = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, const GLint *)) resolve("glSamplerParameteriv");
        TexImage3D = (void (QOPENGLF_APIENTRYP) (GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *)) resolve("glTexImage3D");
        TexStorage2D = (void (QOPENGLF_APIENTRYP) (GLenum, GLsizei, GLenum, GLsizei, GLsizei)) resolve("glTexStorage2D");
        TexStorage3D = (void (QOPENGLF_APIENTRYP) (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei)) resolve("glTexStorage3D");
        TexSubImage3D = (void (QOPENGLF_APIENTRYP) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *)) resolve("glTexSubImage3D");
        TransformFeedbackVaryings = (void (QOPENGLF_APIENTRYP) (GLuint, GLsizei, const GLchar *const*, GLenum)) resolve("glTransformFeedbackVaryings");
        Uniform1ui = (void (QOPENGLF_APIENTRYP) (GLint, GLuint)) resolve("glUniform1ui");
        Uniform1uiv = (void (QOPENGLF_APIENTRYP) (GLint, GLsizei, const GLuint *)) resolve("glUniform1uiv");
        Uniform2ui = (void (QOPENGLF_APIENTRYP) (GLint, GLuint, GLuint)) resolve("glUniform2ui");
        Uniform2uiv = (void (QOPENGLF_APIENTRYP) (GLint, GLsizei, const GLuint *)) resolve("glUniform2uiv");
        Uniform3ui = (void (QOPENGLF_APIENTRYP) (GLint, GLuint, GLuint, GLuint)) resolve("glUniform3ui");
        Uniform3uiv = (void (QOPENGLF_APIENTRYP) (GLint, GLsizei, const GLuint *)) resolve("glUniform3uiv");
        Uniform4ui = (void (QOPENGLF_APIENTRYP) (GLint, GLuint, GLuint, GLuint, GLuint)) resolve("glUniform4ui");
        Uniform4uiv = (void (QOPENGLF_APIENTRYP) (GLint, GLsizei, const GLuint *)) resolve("glUniform4uiv");
        UniformBlockBinding = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint, GLuint)) resolve("glUniformBlockBinding");
        UniformMatrix2x3fv = (void (QOPENGLF_APIENTRYP) (GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glUniformMatrix2x3fv");
        UniformMatrix2x4fv = (void (QOPENGLF_APIENTRYP) (GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glUniformMatrix2x4fv");
        UniformMatrix3x2fv = (void (QOPENGLF_APIENTRYP) (GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glUniformMatrix3x2fv");
        UniformMatrix3x4fv = (void (QOPENGLF_APIENTRYP) (GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glUniformMatrix3x4fv");
        UniformMatrix4x2fv = (void (QOPENGLF_APIENTRYP) (GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glUniformMatrix4x2fv");
        UniformMatrix4x3fv = (void (QOPENGLF_APIENTRYP) (GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glUniformMatrix4x3fv");
        UnmapBuffer = (GLboolean (QOPENGLF_APIENTRYP) (GLenum)) resolve("glUnmapBuffer");
        VertexAttribDivisor = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint)) resolve("glVertexAttribDivisor");
        VertexAttribI4i = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLint, GLint, GLint)) resolve("glVertexAttribI4i");
        VertexAttribI4iv = (void (QOPENGLF_APIENTRYP) (GLuint, const GLint *)) resolve("glVertexAttribI4iv");
        VertexAttribI4ui = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint, GLuint, GLuint, GLuint)) resolve("glVertexAttribI4ui");
        VertexAttribI4uiv = (void (QOPENGLF_APIENTRYP) (GLuint, const GLuint *)) resolve("glVertexAttribI4uiv");
        VertexAttribIPointer = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLenum, GLsizei, const void *)) resolve("glVertexAttribIPointer");
        WaitSync = (void (QOPENGLF_APIENTRYP) (GLsync, GLbitfield, GLuint64)) resolve("glWaitSync");

        if (!BeginQuery || !BlitFramebuffer || !GenTransformFeedbacks || !GenVertexArrays || !MapBufferRange
            || !RenderbufferStorageMultisample || !TexStorage2D || !WaitSync) {
            qWarning("OpenGL ES 3.0 entry points not found. This is odd because the driver returned a context of version %d.%d",
                     contextVersion.first, contextVersion.second);
            return;
        }
        m_supportedVersion = qMakePair(3, 0);

        if (contextVersion >= qMakePair(3, 1)) {
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
            qDebug("Resolving OpenGL ES 3.1 entry points");
#endif

            ActiveShaderProgram = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint)) resolve("glActiveShaderProgram");
            BindImageTexture = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint, GLint, GLboolean, GLint, GLenum, GLenum)) resolve("glBindImageTexture");
            BindProgramPipeline = (void (QOPENGLF_APIENTRYP) (GLuint)) resolve("glBindProgramPipeline");
            BindVertexBuffer = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint, GLintptr, GLsizei)) resolve("glBindVertexBuffer");
            CreateShaderProgramv = (GLuint (QOPENGLF_APIENTRYP) (GLenum, GLsizei, const GLchar *const*)) resolve("glCreateShaderProgramv");
            DeleteProgramPipelines = (void (QOPENGLF_APIENTRYP) (GLsizei, const GLuint *)) resolve("glDeleteProgramPipelines");
            DispatchCompute = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint, GLuint)) resolve("glDispatchCompute");
            DispatchComputeIndirect = (void (QOPENGLF_APIENTRYP) (GLintptr)) resolve("glDispatchComputeIndirect");
            DrawArraysIndirect = (void (QOPENGLF_APIENTRYP) (GLenum, const void *)) resolve("glDrawArraysIndirect");
            DrawElementsIndirect = (void (QOPENGLF_APIENTRYP) (GLenum, GLenum, const void *)) resolve("glDrawElementsIndirect");
            FramebufferParameteri = (void (QOPENGLF_APIENTRYP) (GLenum, GLenum, GLint)) resolve("glFramebufferParameteri");
            GenProgramPipelines = (void (QOPENGLF_APIENTRYP) (GLsizei, GLuint*)) resolve("glGenProgramPipelines");
            GetBooleani_v = (void (QOPENGLF_APIENTRYP) (GLenum, GLuint, GLboolean*)) resolve("glGetBooleani_v");
            GetFramebufferParameteriv = (void (QOPENGLF_APIENTRYP) (GLenum, GLenum, GLint*)) resolve("glGetFramebufferParameteriv");
            GetMultisamplefv = (void (QOPENGLF_APIENTRYP) (GLenum, GLuint, GLfloat*)) resolve("glGetMultisamplefv");
            GetProgramInterfaceiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, GLenum, GLint*)) resolve("glGetProgramInterfaceiv");
            GetProgramPipelineInfoLog = (void (QOPENGLF_APIENTRYP) (GLuint, GLsizei, GLsizei*, GLchar*)) resolve("glGetProgramPipelineInfoLog");
            GetProgramPipelineiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, GLint*)) resolve("glGetProgramPipelineiv");
            GetProgramResourceIndex = (GLuint (QOPENGLF_APIENTRYP) (GLuint, GLenum, const GLchar *)) resolve("glGetProgramResourceIndex");
            GetProgramResourceLocation = (GLint (QOPENGLF_APIENTRYP) (GLuint, GLenum, const GLchar *)) resolve("glGetProgramResourceLocation");
            GetProgramResourceName = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, GLuint, GLsizei, GLsizei*, GLchar*)) resolve("glGetProgramResourceName");
            GetProgramResourceiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLenum, GLuint, GLsizei, const GLenum *, GLsizei, GLsizei*, GLint*)) resolve("glGetProgramResourceiv");
            GetTexLevelParameterfv = (void (QOPENGLF_APIENTRYP) (GLenum, GLint, GLenum, GLfloat*)) resolve("glGetTexLevelParameterfv");
            GetTexLevelParameteriv = (void (QOPENGLF_APIENTRYP) (GLenum, GLint, GLenum, GLint*)) resolve("glGetTexLevelParameteriv");
            IsProgramPipeline = (GLboolean (QOPENGLF_APIENTRYP) (GLuint)) resolve("glIsProgramPipeline");
            MemoryBarrierFunc = (void (QOPENGLF_APIENTRYP) (GLbitfield)) resolve("glMemoryBarrier");
            MemoryBarrierByRegion = (void (QOPENGLF_APIENTRYP) (GLbitfield)) resolve("glMemoryBarrierByRegion");
            ProgramUniform1f = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLfloat)) resolve("glProgramUniform1f");
            ProgramUniform1fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, const GLfloat *)) resolve("glProgramUniform1fv");
            ProgramUniform1i = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLint)) resolve("glProgramUniform1i");
            ProgramUniform1iv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, const GLint *)) resolve("glProgramUniform1iv");
            ProgramUniform1ui = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLuint)) resolve("glProgramUniform1ui");
            ProgramUniform1uiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, const GLuint *)) resolve("glProgramUniform1uiv");
            ProgramUniform2f = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLfloat, GLfloat)) resolve("glProgramUniform2f");
            ProgramUniform2fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, const GLfloat *)) resolve("glProgramUniform2fv");
            ProgramUniform2i = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLint, GLint)) resolve("glProgramUniform2i");
            ProgramUniform2iv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, const GLint *)) resolve("glProgramUniform2iv");
            ProgramUniform2ui = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLuint, GLuint)) resolve("glProgramUniform2ui");
            ProgramUniform2uiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, const GLuint *)) resolve("glProgramUniform2uiv");
            ProgramUniform3f = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLfloat, GLfloat, GLfloat)) resolve("glProgramUniform3f");
            ProgramUniform3fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, const GLfloat *)) resolve("glProgramUniform3fv");
            ProgramUniform3i = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLint, GLint, GLint)) resolve("glProgramUniform3i");
            ProgramUniform3iv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, const GLint *)) resolve("glProgramUniform3iv");
            ProgramUniform3ui = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLuint, GLuint, GLuint)) resolve("glProgramUniform3ui");
            ProgramUniform3uiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, const GLuint *)) resolve("glProgramUniform3uiv");
            ProgramUniform4f = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLfloat, GLfloat, GLfloat, GLfloat)) resolve("glProgramUniform4f");
            ProgramUniform4fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, const GLfloat *)) resolve("glProgramUniform4fv");
            ProgramUniform4i = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLint, GLint, GLint, GLint)) resolve("glProgramUniform4i");
            ProgramUniform4iv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, const GLint *)) resolve("glProgramUniform4iv");
            ProgramUniform4ui = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLuint, GLuint, GLuint, GLuint)) resolve("glProgramUniform4ui");
            ProgramUniform4uiv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, const GLuint *)) resolve("glProgramUniform4uiv");
            ProgramUniformMatrix2fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glProgramUniformMatrix2fv");
            ProgramUniformMatrix2x3fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glProgramUniformMatrix2x3fv");
            ProgramUniformMatrix2x4fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glProgramUniformMatrix2x4fv");
            ProgramUniformMatrix3fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glProgramUniformMatrix3fv");
            ProgramUniformMatrix3x2fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glProgramUniformMatrix3x2fv");
            ProgramUniformMatrix3x4fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glProgramUniformMatrix3x4fv");
            ProgramUniformMatrix4fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glProgramUniformMatrix4fv");
            ProgramUniformMatrix4x2fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glProgramUniformMatrix4x2fv");
            ProgramUniformMatrix4x3fv = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLsizei, GLboolean, const GLfloat *)) resolve("glProgramUniformMatrix4x3fv");
            SampleMaski = (void (QOPENGLF_APIENTRYP) (GLuint, GLbitfield)) resolve("glSampleMaski");
            TexStorage2DMultisample = (void (QOPENGLF_APIENTRYP) (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean)) resolve("glTexStorage2DMultisample");
            UseProgramStages = (void (QOPENGLF_APIENTRYP) (GLuint, GLbitfield, GLuint)) resolve("glUseProgramStages");
            ValidateProgramPipeline = (void (QOPENGLF_APIENTRYP) (GLuint)) resolve("glValidateProgramPipeline");
            VertexAttribBinding = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint)) resolve("glVertexAttribBinding");
            VertexAttribFormat = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLenum, GLboolean, GLuint)) resolve("glVertexAttribFormat");
            VertexAttribIFormat = (void (QOPENGLF_APIENTRYP) (GLuint, GLint, GLenum, GLuint)) resolve("glVertexAttribIFormat");
            VertexBindingDivisor = (void (QOPENGLF_APIENTRYP) (GLuint, GLuint)) resolve("glVertexBindingDivisor");

            if (!ActiveShaderProgram || !BindImageTexture || !DispatchCompute || !DrawArraysIndirect
                || !GenProgramPipelines || !MemoryBarrierFunc) {
                qWarning("OpenGL ES 3.1 entry points not found. This is odd because the driver returned a context of version %d.%d",
                         contextVersion.first, contextVersion.second);
                return;
            }
            m_supportedVersion = qMakePair(3, 1);
        }
    } else {
        qFatal("Failed to load libGLESv2");
    }
}

// GLES 3.0 and 3.1

// Checks for true OpenGL ES 3.x. OpenGL with GL_ARB_ES3_compatibility
// does not count because there the plain resolvers work anyhow.
static inline bool isES3(int minor)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();

    const bool libMatches = QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES;
    const bool contextMatches = ctx->isOpenGLES() && ctx->format().version() >= qMakePair(3, minor);

    // Resolving happens whenever qgles3Helper() is called first. So do it only
    // when the driver gives a 3.0+ context.
    if (libMatches && contextMatches)
        return qgles3Helper()->supportedVersion() >= qMakePair(3, minor);

    return false;
}

// Go through the dlsym-based helper for real ES 3, resolve using
// wglGetProcAddress or similar when on plain OpenGL.

static void QOPENGLF_APIENTRY qopenglfResolveBeginQuery(GLenum target, GLuint id)
{
    if (isES3(0))
        qgles3Helper()->BeginQuery(target, id);
    else
        RESOLVE_FUNC_VOID(0, BeginQuery)(target, id);
}

static void QOPENGLF_APIENTRY qopenglfResolveBeginTransformFeedback(GLenum primitiveMode)
{
    if (isES3(0))
        qgles3Helper()->BeginTransformFeedback(primitiveMode);
    else
        RESOLVE_FUNC_VOID(0, BeginTransformFeedback)(primitiveMode);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    if (isES3(0))
        qgles3Helper()->BindBufferBase(target, index, buffer);
    else
        RESOLVE_FUNC_VOID(0, BindBufferBase)(target, index, buffer);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    if (isES3(0))
        qgles3Helper()->BindBufferRange(target, index, buffer, offset, size);
    else
        RESOLVE_FUNC_VOID(0, BindBufferRange)(target, index, buffer, offset, size);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindSampler(GLuint unit, GLuint sampler)
{
    if (isES3(0))
        qgles3Helper()->BindSampler(unit, sampler);
    else
        RESOLVE_FUNC_VOID(0, BindSampler)(unit, sampler);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindTransformFeedback(GLenum target, GLuint id)
{
    if (isES3(0))
        qgles3Helper()->BindTransformFeedback(target, id);
    else
        RESOLVE_FUNC_VOID(0, BindTransformFeedback)(target, id);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindVertexArray(GLuint array)
{
    if (isES3(0))
        qgles3Helper()->BindVertexArray(array);
    else
        RESOLVE_FUNC_VOID(0, BindVertexArray)(array);
}

static void QOPENGLF_APIENTRY qopenglfResolveBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    if (isES3(0))
        qgles3Helper()->BlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    else
        RESOLVE_FUNC_VOID(ResolveEXT | ResolveANGLE | ResolveNV, BlitFramebuffer)
            (srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

static void QOPENGLF_APIENTRY qopenglfResolveClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    if (isES3(0))
        qgles3Helper()->ClearBufferfi(buffer, drawbuffer, depth, stencil);
    else
        RESOLVE_FUNC_VOID(0, ClearBufferfi)(buffer, drawbuffer, depth, stencil);
}

static void QOPENGLF_APIENTRY qopenglfResolveClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat * value)
{
    if (isES3(0))
        qgles3Helper()->ClearBufferfv(buffer, drawbuffer, value);
    else
        RESOLVE_FUNC_VOID(0, ClearBufferfv)(buffer, drawbuffer, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint * value)
{
    if (isES3(0))
        qgles3Helper()->ClearBufferiv(buffer, drawbuffer, value);
    else
        RESOLVE_FUNC_VOID(0, ClearBufferiv)(buffer, drawbuffer, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint * value)
{
    if (isES3(0))
        qgles3Helper()->ClearBufferuiv(buffer, drawbuffer, value);
    else
        RESOLVE_FUNC_VOID(0, ClearBufferuiv)(buffer, drawbuffer, value);
}

static GLenum QOPENGLF_APIENTRY qopenglfResolveClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    if (isES3(0))
        return qgles3Helper()->ClientWaitSync(sync, flags, timeout);
    else
        RESOLVE_FUNC(GLenum, 0, ClientWaitSync)(sync, flags, timeout);
}

static void QOPENGLF_APIENTRY qopenglfResolveCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void * data)
{
    if (isES3(0))
        qgles3Helper()->CompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
    else
        RESOLVE_FUNC_VOID(0, CompressedTexImage3D)(target, level, internalformat, width, height, depth, border, imageSize, data);
}

static void QOPENGLF_APIENTRY qopenglfResolveCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void * data)
{
    if (isES3(0))
        qgles3Helper()->CompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
    else
        RESOLVE_FUNC_VOID(0, CompressedTexSubImage3D)(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

static void QOPENGLF_APIENTRY qopenglfResolveCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    if (isES3(0))
        qgles3Helper()->CopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
    else
        RESOLVE_FUNC_VOID(0, CopyBufferSubData)(readTarget, writeTarget, readOffset, writeOffset, size);
}

static void QOPENGLF_APIENTRY qopenglfResolveCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (isES3(0))
        qgles3Helper()->CopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
    else
        RESOLVE_FUNC_VOID(0, CopyTexSubImage3D)(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteQueries(GLsizei n, const GLuint * ids)
{
    if (isES3(0))
        qgles3Helper()->DeleteQueries(n, ids);
    else
        RESOLVE_FUNC_VOID(0, DeleteQueries)(n, ids);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteSamplers(GLsizei count, const GLuint * samplers)
{
    if (isES3(0))
        qgles3Helper()->DeleteSamplers(count, samplers);
    else
        RESOLVE_FUNC_VOID(0, DeleteSamplers)(count, samplers);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteSync(GLsync sync)
{
    if (isES3(0))
        qgles3Helper()->DeleteSync(sync);
    else
        RESOLVE_FUNC_VOID(0, DeleteSync)(sync);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteTransformFeedbacks(GLsizei n, const GLuint * ids)
{
    if (isES3(0))
        qgles3Helper()->DeleteTransformFeedbacks(n, ids);
    else
        RESOLVE_FUNC_VOID(0, DeleteTransformFeedbacks)(n, ids);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteVertexArrays(GLsizei n, const GLuint * arrays)
{
    if (isES3(0))
        qgles3Helper()->DeleteVertexArrays(n, arrays);
    else
        RESOLVE_FUNC_VOID(0, DeleteVertexArrays)(n, arrays);
}

static void QOPENGLF_APIENTRY qopenglfResolveDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
    if (isES3(0))
        qgles3Helper()->DrawArraysInstanced(mode, first, count, instancecount);
    else
        RESOLVE_FUNC_VOID(0, DrawArraysInstanced)(mode, first, count, instancecount);
}

static void QOPENGLF_APIENTRY qopenglfResolveDrawBuffers(GLsizei n, const GLenum * bufs)
{
    if (isES3(0))
        qgles3Helper()->DrawBuffers(n, bufs);
    else
        RESOLVE_FUNC_VOID(0, DrawBuffers)(n, bufs);
}

static void QOPENGLF_APIENTRY qopenglfResolveDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei instancecount)
{
    if (isES3(0))
        qgles3Helper()->DrawElementsInstanced(mode, count, type, indices, instancecount);
    else
        RESOLVE_FUNC_VOID(0, DrawElementsInstanced)(mode, count, type, indices, instancecount);
}

static void QOPENGLF_APIENTRY qopenglfResolveDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void * indices)
{
    if (isES3(0))
        qgles3Helper()->DrawRangeElements(mode, start, end, count, type, indices);
    else
        RESOLVE_FUNC_VOID(0, DrawRangeElements)(mode, start, end, count, type, indices);
}

static void QOPENGLF_APIENTRY qopenglfResolveEndQuery(GLenum target)
{
    if (isES3(0))
        qgles3Helper()->EndQuery(target);
    else
        RESOLVE_FUNC_VOID(0, EndQuery)(target);
}

static void QOPENGLF_APIENTRY qopenglfResolveEndTransformFeedback()
{
    if (isES3(0))
        qgles3Helper()->EndTransformFeedback();
    else
        RESOLVE_FUNC_VOID(0, EndTransformFeedback)();
}

static GLsync QOPENGLF_APIENTRY qopenglfResolveFenceSync(GLenum condition, GLbitfield flags)
{
    if (isES3(0))
        return qgles3Helper()->FenceSync(condition, flags);
    else
        RESOLVE_FUNC(GLsync, 0, FenceSync)(condition, flags);
}

static void QOPENGLF_APIENTRY qopenglfResolveFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
    if (isES3(0))
        qgles3Helper()->FlushMappedBufferRange(target, offset, length);
    else
        RESOLVE_FUNC_VOID(0, FlushMappedBufferRange)(target, offset, length);
}

static void QOPENGLF_APIENTRY qopenglfResolveFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    if (isES3(0))
        qgles3Helper()->FramebufferTextureLayer(target, attachment, texture, level, layer);
    else
        RESOLVE_FUNC_VOID(0, FramebufferTextureLayer)(target, attachment, texture, level, layer);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenQueries(GLsizei n, GLuint* ids)
{
    if (isES3(0))
        qgles3Helper()->GenQueries(n, ids);
    else
        RESOLVE_FUNC_VOID(0, GenQueries)(n, ids);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenSamplers(GLsizei count, GLuint* samplers)
{
    if (isES3(0))
        qgles3Helper()->GenSamplers(count, samplers);
    else
        RESOLVE_FUNC_VOID(0, GenSamplers)(count, samplers);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenTransformFeedbacks(GLsizei n, GLuint* ids)
{
    if (isES3(0))
        qgles3Helper()->GenTransformFeedbacks(n, ids);
    else
        RESOLVE_FUNC_VOID(0, GenTransformFeedbacks)(n, ids);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenVertexArrays(GLsizei n, GLuint* arrays)
{
    if (isES3(0))
        qgles3Helper()->GenVertexArrays(n, arrays);
    else
        RESOLVE_FUNC_VOID(0, GenVertexArrays)(n, arrays);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
    if (isES3(0))
        qgles3Helper()->GetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
    else
        RESOLVE_FUNC_VOID(0, GetActiveUniformBlockName)(program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
    if (isES3(0))
        qgles3Helper()->GetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetActiveUniformBlockiv)(program, uniformBlockIndex, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname, GLint* params)
{
    if (isES3(0))
        qgles3Helper()->GetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetActiveUniformsiv)(program, uniformCount, uniformIndices, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params)
{
    if (isES3(0))
        qgles3Helper()->GetBufferParameteri64v(target, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetBufferParameteri64v)(target, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetBufferPointerv(GLenum target, GLenum pname, void ** params)
{
    if (isES3(0))
        qgles3Helper()->GetBufferPointerv(target, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetBufferPointerv)(target, pname, params);
}

static GLint QOPENGLF_APIENTRY qopenglfResolveGetFragDataLocation(GLuint program, const GLchar * name)
{
    if (isES3(0))
        return qgles3Helper()->GetFragDataLocation(program, name);
    else
        RESOLVE_FUNC(GLint, 0, GetFragDataLocation)(program, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetInteger64i_v(GLenum target, GLuint index, GLint64* data)
{
    if (isES3(0))
        qgles3Helper()->GetInteger64i_v(target, index, data);
    else
        RESOLVE_FUNC_VOID(0, GetInteger64i_v)(target, index, data);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetInteger64v(GLenum pname, GLint64* data)
{
    if (isES3(0))
        qgles3Helper()->GetInteger64v(pname, data);
    else
        RESOLVE_FUNC_VOID(0, GetInteger64v)(pname, data);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetIntegeri_v(GLenum target, GLuint index, GLint* data)
{
    if (isES3(0))
        qgles3Helper()->GetIntegeri_v(target, index, data);
    else
        RESOLVE_FUNC_VOID(0, GetIntegeri_v)(target, index, data);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
    if (isES3(0))
        qgles3Helper()->GetInternalformativ(target, internalformat, pname, bufSize, params);
    else
        RESOLVE_FUNC_VOID(0, GetInternalformativ)(target, internalformat, pname, bufSize, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void * binary)
{
    if (isES3(0))
        qgles3Helper()->GetProgramBinary(program, bufSize, length, binaryFormat, binary);
    else
        RESOLVE_FUNC_VOID(0, GetProgramBinary)(program, bufSize, length, binaryFormat, binary);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params)
{
    if (isES3(0))
        qgles3Helper()->GetQueryObjectuiv(id, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetQueryObjectuiv)(id, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetQueryiv(GLenum target, GLenum pname, GLint* params)
{
    if (isES3(0))
        qgles3Helper()->GetQueryiv(target, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetQueryiv)(target, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params)
{
    if (isES3(0))
        qgles3Helper()->GetSamplerParameterfv(sampler, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetSamplerParameterfv)(sampler, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params)
{
    if (isES3(0))
        qgles3Helper()->GetSamplerParameteriv(sampler, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetSamplerParameteriv)(sampler, pname, params);
}

static const GLubyte * QOPENGLF_APIENTRY qopenglfResolveGetStringi(GLenum name, GLuint index)
{
    if (isES3(0))
        return qgles3Helper()->GetStringi(name, index);
    else
        RESOLVE_FUNC(const GLubyte *, 0, GetStringi)(name, index);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
    if (isES3(0))
        qgles3Helper()->GetSynciv(sync, pname, bufSize, length, values);
    else
        RESOLVE_FUNC_VOID(0, GetSynciv)(sync, pname, bufSize, length, values);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
    if (isES3(0))
        qgles3Helper()->GetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
    else
        RESOLVE_FUNC_VOID(0, GetTransformFeedbackVarying)(program, index, bufSize, length, size, type, name);
}

static GLuint QOPENGLF_APIENTRY qopenglfResolveGetUniformBlockIndex(GLuint program, const GLchar * uniformBlockName)
{
    if (isES3(0))
        return qgles3Helper()->GetUniformBlockIndex(program, uniformBlockName);
    else
        RESOLVE_FUNC(GLuint, 0, GetUniformBlockIndex)(program, uniformBlockName);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const* uniformNames, GLuint* uniformIndices)
{
    if (isES3(0))
        qgles3Helper()->GetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
    else
        RESOLVE_FUNC_VOID(0, GetUniformIndices)(program, uniformCount, uniformNames, uniformIndices);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetUniformuiv(GLuint program, GLint location, GLuint* params)
{
    if (isES3(0))
        qgles3Helper()->GetUniformuiv(program, location, params);
    else
        RESOLVE_FUNC_VOID(0, GetUniformuiv)(program, location, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetVertexAttribIiv(GLuint index, GLenum pname, GLint* params)
{
    if (isES3(0))
        qgles3Helper()->GetVertexAttribIiv(index, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetVertexAttribIiv)(index, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params)
{
    if (isES3(0))
        qgles3Helper()->GetVertexAttribIuiv(index, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetVertexAttribIuiv)(index, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum * attachments)
{
    if (isES3(0))
        qgles3Helper()->InvalidateFramebuffer(target, numAttachments, attachments);
    else
        RESOLVE_FUNC_VOID(0, InvalidateFramebuffer)(target, numAttachments, attachments);
}

static void QOPENGLF_APIENTRY qopenglfResolveInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum * attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (isES3(0))
        qgles3Helper()->InvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
    else
        RESOLVE_FUNC_VOID(0, InvalidateSubFramebuffer)(target, numAttachments, attachments, x, y, width, height);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsQuery(GLuint id)
{
    if (isES3(0))
        return qgles3Helper()->IsQuery(id);
    else
        RESOLVE_FUNC(GLboolean, 0, IsQuery)(id);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsSampler(GLuint sampler)
{
    if (isES3(0))
        return qgles3Helper()->IsSampler(sampler);
    else
        RESOLVE_FUNC(GLboolean, 0, IsSampler)(sampler);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsSync(GLsync sync)
{
    if (isES3(0))
        return qgles3Helper()->IsSync(sync);
    else
        RESOLVE_FUNC(GLboolean, 0, IsSync)(sync);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsTransformFeedback(GLuint id)
{
    if (isES3(0))
        return qgles3Helper()->IsTransformFeedback(id);
    else
        RESOLVE_FUNC(GLboolean, 0, IsTransformFeedback)(id);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsVertexArray(GLuint array)
{
    if (isES3(0))
        return qgles3Helper()->IsVertexArray(array);
    else
        RESOLVE_FUNC(GLboolean, 0, IsVertexArray)(array);
}

static void * QOPENGLF_APIENTRY qopenglfResolveMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    if (isES3(0))
        return qgles3Helper()->MapBufferRange(target, offset, length, access);
    else
        RESOLVE_FUNC(void *, 0, MapBufferRange)(target, offset, length, access);
}

static void QOPENGLF_APIENTRY qopenglfResolvePauseTransformFeedback()
{
    if (isES3(0))
        qgles3Helper()->PauseTransformFeedback();
    else
        RESOLVE_FUNC_VOID(0, PauseTransformFeedback)();
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramBinary(GLuint program, GLenum binaryFormat, const void * binary, GLsizei length)
{
    if (isES3(0))
        qgles3Helper()->ProgramBinary(program, binaryFormat, binary, length);
    else
        RESOLVE_FUNC_VOID(0, ProgramBinary)(program, binaryFormat, binary, length);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramParameteri(GLuint program, GLenum pname, GLint value)
{
    if (isES3(0))
        qgles3Helper()->ProgramParameteri(program, pname, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramParameteri)(program, pname, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveReadBuffer(GLenum src)
{
    if (isES3(0))
        qgles3Helper()->ReadBuffer(src);
    else
        RESOLVE_FUNC_VOID(0, ReadBuffer)(src);
}

static void QOPENGLF_APIENTRY qopenglfResolveRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    if (isES3(0))
        qgles3Helper()->RenderbufferStorageMultisample(target, samples, internalformat, width, height);
    else
        RESOLVE_FUNC_VOID(ResolveEXT | ResolveANGLE | ResolveNV, RenderbufferStorageMultisample)
            (target, samples, internalformat, width, height);
}

static void QOPENGLF_APIENTRY qopenglfResolveResumeTransformFeedback()
{
    if (isES3(0))
        qgles3Helper()->ResumeTransformFeedback();
    else
        RESOLVE_FUNC_VOID(0, ResumeTransformFeedback)();
}

static void QOPENGLF_APIENTRY qopenglfResolveSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
    if (isES3(0))
        qgles3Helper()->SamplerParameterf(sampler, pname, param);
    else
        RESOLVE_FUNC_VOID(0, SamplerParameterf)(sampler, pname, param);
}

static void QOPENGLF_APIENTRY qopenglfResolveSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat * param)
{
    if (isES3(0))
        qgles3Helper()->SamplerParameterfv(sampler, pname, param);
    else
        RESOLVE_FUNC_VOID(0, SamplerParameterfv)(sampler, pname, param);
}

static void QOPENGLF_APIENTRY qopenglfResolveSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
    if (isES3(0))
        qgles3Helper()->SamplerParameteri(sampler, pname, param);
    else
        RESOLVE_FUNC_VOID(0, SamplerParameteri)(sampler, pname, param);
}

static void QOPENGLF_APIENTRY qopenglfResolveSamplerParameteriv(GLuint sampler, GLenum pname, const GLint * param)
{
    if (isES3(0))
        qgles3Helper()->SamplerParameteriv(sampler, pname, param);
    else
        RESOLVE_FUNC_VOID(0, SamplerParameteriv)(sampler, pname, param);
}

static void QOPENGLF_APIENTRY qopenglfResolveTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void * pixels)
{
    if (isES3(0))
        qgles3Helper()->TexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
    else
        RESOLVE_FUNC_VOID(0, TexImage3D)(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

static void QOPENGLF_APIENTRY qopenglfResolveTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    if (isES3(0))
        qgles3Helper()->TexStorage2D(target, levels, internalformat, width, height);
    else
        RESOLVE_FUNC_VOID(0, TexStorage2D)(target, levels, internalformat, width, height);
}

static void QOPENGLF_APIENTRY qopenglfResolveTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    if (isES3(0))
        qgles3Helper()->TexStorage3D(target, levels, internalformat, width, height, depth);
    else
        RESOLVE_FUNC_VOID(0, TexStorage3D)(target, levels, internalformat, width, height, depth);
}

static void QOPENGLF_APIENTRY qopenglfResolveTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void * pixels)
{
    if (isES3(0))
        qgles3Helper()->TexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    else
        RESOLVE_FUNC_VOID(0, TexSubImage3D)(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

static void QOPENGLF_APIENTRY qopenglfResolveTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const* varyings, GLenum bufferMode)
{
    if (isES3(0))
        qgles3Helper()->TransformFeedbackVaryings(program, count, varyings, bufferMode);
    else
        RESOLVE_FUNC_VOID(0, TransformFeedbackVaryings)(program, count, varyings, bufferMode);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform1ui(GLint location, GLuint v0)
{
    if (isES3(0))
        qgles3Helper()->Uniform1ui(location, v0);
    else
        RESOLVE_FUNC_VOID(0, Uniform1ui)(location, v0);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform1uiv(GLint location, GLsizei count, const GLuint * value)
{
    if (isES3(0))
        qgles3Helper()->Uniform1uiv(location, count, value);
    else
        RESOLVE_FUNC_VOID(0, Uniform1uiv)(location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform2ui(GLint location, GLuint v0, GLuint v1)
{
    if (isES3(0))
        qgles3Helper()->Uniform2ui(location, v0, v1);
    else
        RESOLVE_FUNC_VOID(0, Uniform2ui)(location, v0, v1);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform2uiv(GLint location, GLsizei count, const GLuint * value)
{
    if (isES3(0))
        qgles3Helper()->Uniform2uiv(location, count, value);
    else
        RESOLVE_FUNC_VOID(0, Uniform2uiv)(location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    if (isES3(0))
        qgles3Helper()->Uniform3ui(location, v0, v1, v2);
    else
        RESOLVE_FUNC_VOID(0, Uniform3ui)(location, v0, v1, v2);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform3uiv(GLint location, GLsizei count, const GLuint * value)
{
    if (isES3(0))
        qgles3Helper()->Uniform3uiv(location, count, value);
    else
        RESOLVE_FUNC_VOID(0, Uniform3uiv)(location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    if (isES3(0))
        qgles3Helper()->Uniform4ui(location, v0, v1, v2, v3);
    else
        RESOLVE_FUNC_VOID(0, Uniform4ui)(location, v0, v1, v2, v3);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniform4uiv(GLint location, GLsizei count, const GLuint * value)
{
    if (isES3(0))
        qgles3Helper()->Uniform4uiv(location, count, value);
    else
        RESOLVE_FUNC_VOID(0, Uniform4uiv)(location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    if (isES3(0))
        qgles3Helper()->UniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
    else
        RESOLVE_FUNC_VOID(0, UniformBlockBinding)(program, uniformBlockIndex, uniformBlockBinding);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(0))
        qgles3Helper()->UniformMatrix2x3fv(location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, UniformMatrix2x3fv)(location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(0))
        qgles3Helper()->UniformMatrix2x4fv(location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, UniformMatrix2x4fv)(location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(0))
        qgles3Helper()->UniformMatrix3x2fv(location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, UniformMatrix3x2fv)(location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(0))
        qgles3Helper()->UniformMatrix3x4fv(location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, UniformMatrix3x4fv)(location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(0))
        qgles3Helper()->UniformMatrix4x2fv(location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, UniformMatrix4x2fv)(location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(0))
        qgles3Helper()->UniformMatrix4x3fv(location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, UniformMatrix4x3fv)(location, count, transpose, value);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveUnmapBuffer(GLenum target)
{
    if (isES3(0))
        return qgles3Helper()->UnmapBuffer(target);
    else
        RESOLVE_FUNC(GLboolean, ResolveOES, UnmapBuffer)(target);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttribDivisor(GLuint index, GLuint divisor)
{
    if (isES3(0))
        qgles3Helper()->VertexAttribDivisor(index, divisor);
    else
        RESOLVE_FUNC_VOID(0, VertexAttribDivisor)(index, divisor);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    if (isES3(0))
        qgles3Helper()->VertexAttribI4i(index, x, y, z, w);
    else
        RESOLVE_FUNC_VOID(0, VertexAttribI4i)(index, x, y, z, w);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttribI4iv(GLuint index, const GLint * v)
{
    if (isES3(0))
        qgles3Helper()->VertexAttribI4iv(index, v);
    else
        RESOLVE_FUNC_VOID(0, VertexAttribI4iv)(index, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    if (isES3(0))
        qgles3Helper()->VertexAttribI4ui(index, x, y, z, w);
    else
        RESOLVE_FUNC_VOID(0, VertexAttribI4ui)(index, x, y, z, w);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttribI4uiv(GLuint index, const GLuint * v)
{
    if (isES3(0))
        qgles3Helper()->VertexAttribI4uiv(index, v);
    else
        RESOLVE_FUNC_VOID(0, VertexAttribI4uiv)(index, v);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void * pointer)
{
    if (isES3(0))
        qgles3Helper()->VertexAttribIPointer(index, size, type, stride, pointer);
    else
        RESOLVE_FUNC_VOID(0, VertexAttribIPointer)(index, size, type, stride, pointer);
}

static void QOPENGLF_APIENTRY qopenglfResolveWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    if (isES3(0))
        qgles3Helper()->WaitSync(sync, flags, timeout);
    else
        RESOLVE_FUNC_VOID(0, WaitSync)(sync, flags, timeout);
}

static void QOPENGLF_APIENTRY qopenglfResolveActiveShaderProgram(GLuint pipeline, GLuint program)
{
    if (isES3(1))
        qgles3Helper()->ActiveShaderProgram(pipeline, program);
    else
        RESOLVE_FUNC_VOID(0, ActiveShaderProgram)(pipeline, program);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    if (isES3(1))
        qgles3Helper()->BindImageTexture(unit, texture, level, layered, layer, access, format);
    else
        RESOLVE_FUNC_VOID(0, BindImageTexture)(unit, texture, level, layered, layer, access, format);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindProgramPipeline(GLuint pipeline)
{
    if (isES3(1))
        qgles3Helper()->BindProgramPipeline(pipeline);
    else
        RESOLVE_FUNC_VOID(0, BindProgramPipeline)(pipeline);
}

static void QOPENGLF_APIENTRY qopenglfResolveBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    if (isES3(1))
        qgles3Helper()->BindVertexBuffer(bindingindex, buffer, offset, stride);
    else
        RESOLVE_FUNC_VOID(0, BindVertexBuffer)(bindingindex, buffer, offset, stride);
}

static GLuint QOPENGLF_APIENTRY qopenglfResolveCreateShaderProgramv(GLenum type, GLsizei count, const GLchar *const* strings)
{
    if (isES3(1))
        return qgles3Helper()->CreateShaderProgramv(type, count, strings);
    else
        RESOLVE_FUNC(GLuint, 0, CreateShaderProgramv)(type, count, strings);
}

static void QOPENGLF_APIENTRY qopenglfResolveDeleteProgramPipelines(GLsizei n, const GLuint * pipelines)
{
    if (isES3(1))
        qgles3Helper()->DeleteProgramPipelines(n, pipelines);
    else
        RESOLVE_FUNC_VOID(0, DeleteProgramPipelines)(n, pipelines);
}

static void QOPENGLF_APIENTRY qopenglfResolveDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    if (isES3(1))
        qgles3Helper()->DispatchCompute(num_groups_x, num_groups_y, num_groups_z);
    else
        RESOLVE_FUNC_VOID(0, DispatchCompute)(num_groups_x, num_groups_y, num_groups_z);
}

static void QOPENGLF_APIENTRY qopenglfResolveDispatchComputeIndirect(GLintptr indirect)
{
    if (isES3(1))
        qgles3Helper()->DispatchComputeIndirect(indirect);
    else
        RESOLVE_FUNC_VOID(0, DispatchComputeIndirect)(indirect);
}

static void QOPENGLF_APIENTRY qopenglfResolveDrawArraysIndirect(GLenum mode, const void * indirect)
{
    if (isES3(1))
        qgles3Helper()->DrawArraysIndirect(mode, indirect);
    else
        RESOLVE_FUNC_VOID(0, DrawArraysIndirect)(mode, indirect);
}

static void QOPENGLF_APIENTRY qopenglfResolveDrawElementsIndirect(GLenum mode, GLenum type, const void * indirect)
{
    if (isES3(1))
        qgles3Helper()->DrawElementsIndirect(mode, type, indirect);
    else
        RESOLVE_FUNC_VOID(0, DrawElementsIndirect)(mode, type, indirect);
}

static void QOPENGLF_APIENTRY qopenglfResolveFramebufferParameteri(GLenum target, GLenum pname, GLint param)
{
    if (isES3(1))
        qgles3Helper()->FramebufferParameteri(target, pname, param);
    else
        RESOLVE_FUNC_VOID(0, FramebufferParameteri)(target, pname, param);
}

static void QOPENGLF_APIENTRY qopenglfResolveGenProgramPipelines(GLsizei n, GLuint* pipelines)
{
    if (isES3(1))
        qgles3Helper()->GenProgramPipelines(n, pipelines);
    else
        RESOLVE_FUNC_VOID(0, GenProgramPipelines)(n, pipelines);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetBooleani_v(GLenum target, GLuint index, GLboolean* data)
{
    if (isES3(1))
        qgles3Helper()->GetBooleani_v(target, index, data);
    else
        RESOLVE_FUNC_VOID(0, GetBooleani_v)(target, index, data);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetFramebufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    if (isES3(1))
        qgles3Helper()->GetFramebufferParameteriv(target, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetFramebufferParameteriv)(target, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetMultisamplefv(GLenum pname, GLuint index, GLfloat* val)
{
    if (isES3(1))
        qgles3Helper()->GetMultisamplefv(pname, index, val);
    else
        RESOLVE_FUNC_VOID(0, GetMultisamplefv)(pname, index, val);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint* params)
{
    if (isES3(1))
        qgles3Helper()->GetProgramInterfaceiv(program, programInterface, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetProgramInterfaceiv)(program, programInterface, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    if (isES3(1))
        qgles3Helper()->GetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
    else
        RESOLVE_FUNC_VOID(0, GetProgramPipelineInfoLog)(pipeline, bufSize, length, infoLog);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint* params)
{
    if (isES3(1))
        qgles3Helper()->GetProgramPipelineiv(pipeline, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetProgramPipelineiv)(pipeline, pname, params);
}

static GLuint QOPENGLF_APIENTRY qopenglfResolveGetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar * name)
{
    if (isES3(1))
        return qgles3Helper()->GetProgramResourceIndex(program, programInterface, name);
    else
        RESOLVE_FUNC(GLuint, 0, GetProgramResourceIndex)(program, programInterface, name);
}

static GLint QOPENGLF_APIENTRY qopenglfResolveGetProgramResourceLocation(GLuint program, GLenum programInterface, const GLchar * name)
{
    if (isES3(1))
        return qgles3Helper()->GetProgramResourceLocation(program, programInterface, name);
    else
        RESOLVE_FUNC(GLint, 0, GetProgramResourceLocation)(program, programInterface, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name)
{
    if (isES3(1))
        qgles3Helper()->GetProgramResourceName(program, programInterface, index, bufSize, length, name);
    else
        RESOLVE_FUNC_VOID(0, GetProgramResourceName)(program, programInterface, index, bufSize, length, name);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum * props, GLsizei bufSize, GLsizei* length, GLint* params)
{
    if (isES3(1))
        qgles3Helper()->GetProgramResourceiv(program, programInterface, index, propCount, props, bufSize, length, params);
    else
        RESOLVE_FUNC_VOID(0, GetProgramResourceiv)(program, programInterface, index, propCount, props, bufSize, length, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params)
{
    if (isES3(1))
        qgles3Helper()->GetTexLevelParameterfv(target, level, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetTexLevelParameterfv)(target, level, pname, params);
}

static void QOPENGLF_APIENTRY qopenglfResolveGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params)
{
    if (isES3(1))
        qgles3Helper()->GetTexLevelParameteriv(target, level, pname, params);
    else
        RESOLVE_FUNC_VOID(0, GetTexLevelParameteriv)(target, level, pname, params);
}

static GLboolean QOPENGLF_APIENTRY qopenglfResolveIsProgramPipeline(GLuint pipeline)
{
    if (isES3(1))
        return qgles3Helper()->IsProgramPipeline(pipeline);
    else
        RESOLVE_FUNC(GLboolean, 0, IsProgramPipeline)(pipeline);
}

static void QOPENGLF_APIENTRY qopenglfResolveMemoryBarrier(GLbitfield barriers)
{
    if (isES3(1))
        qgles3Helper()->MemoryBarrierFunc(barriers);
    else
        RESOLVE_FUNC_VOID(0, MemoryBarrierFunc)(barriers);
}

static void QOPENGLF_APIENTRY qopenglfResolveMemoryBarrierByRegion(GLbitfield barriers)
{
    if (isES3(1))
        qgles3Helper()->MemoryBarrierByRegion(barriers);
    else
        RESOLVE_FUNC_VOID(0, MemoryBarrierByRegion)(barriers);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform1f(GLuint program, GLint location, GLfloat v0)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform1f(program, location, v0);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform1f)(program, location, v0);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform1fv(program, location, count, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform1fv)(program, location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform1i(GLuint program, GLint location, GLint v0)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform1i(program, location, v0);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform1i)(program, location, v0);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform1iv(program, location, count, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform1iv)(program, location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform1ui(GLuint program, GLint location, GLuint v0)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform1ui(program, location, v0);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform1ui)(program, location, v0);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform1uiv(program, location, count, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform1uiv)(program, location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform2f(program, location, v0, v1);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform2f)(program, location, v0, v1);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform2fv(program, location, count, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform2fv)(program, location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform2i(program, location, v0, v1);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform2i)(program, location, v0, v1);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform2iv(program, location, count, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform2iv)(program, location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform2ui(program, location, v0, v1);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform2ui)(program, location, v0, v1);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform2uiv(program, location, count, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform2uiv)(program, location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform3f(program, location, v0, v1, v2);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform3f)(program, location, v0, v1, v2);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform3fv(program, location, count, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform3fv)(program, location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform3i(program, location, v0, v1, v2);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform3i)(program, location, v0, v1, v2);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform3iv(program, location, count, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform3iv)(program, location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform3ui(program, location, v0, v1, v2);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform3ui)(program, location, v0, v1, v2);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform3uiv(program, location, count, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform3uiv)(program, location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform4f(program, location, v0, v1, v2, v3);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform4f)(program, location, v0, v1, v2, v3);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform4fv(program, location, count, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform4fv)(program, location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform4i(program, location, v0, v1, v2, v3);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform4i)(program, location, v0, v1, v2, v3);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform4iv(program, location, count, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform4iv)(program, location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform4ui(program, location, v0, v1, v2, v3);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform4ui)(program, location, v0, v1, v2, v3);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniform4uiv(program, location, count, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniform4uiv)(program, location, count, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniformMatrix2fv(program, location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniformMatrix2fv)(program, location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniformMatrix2x3fv(program, location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniformMatrix2x3fv)(program, location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniformMatrix2x4fv(program, location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniformMatrix2x4fv)(program, location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniformMatrix3fv(program, location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniformMatrix3fv)(program, location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniformMatrix3x2fv(program, location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniformMatrix3x2fv)(program, location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniformMatrix3x4fv(program, location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniformMatrix3x4fv)(program, location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniformMatrix4fv(program, location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniformMatrix4fv)(program, location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniformMatrix4x2fv(program, location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniformMatrix4x2fv)(program, location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (isES3(1))
        qgles3Helper()->ProgramUniformMatrix4x3fv(program, location, count, transpose, value);
    else
        RESOLVE_FUNC_VOID(0, ProgramUniformMatrix4x3fv)(program, location, count, transpose, value);
}

static void QOPENGLF_APIENTRY qopenglfResolveSampleMaski(GLuint maskNumber, GLbitfield mask)
{
    if (isES3(1))
        qgles3Helper()->SampleMaski(maskNumber, mask);
    else
        RESOLVE_FUNC_VOID(0, SampleMaski)(maskNumber, mask);
}

static void QOPENGLF_APIENTRY qopenglfResolveTexStorage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    if (isES3(1))
        qgles3Helper()->TexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
    else
        RESOLVE_FUNC_VOID(0, TexStorage2DMultisample)(target, samples, internalformat, width, height, fixedsamplelocations);
}

static void QOPENGLF_APIENTRY qopenglfResolveUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program)
{
    if (isES3(1))
        qgles3Helper()->UseProgramStages(pipeline, stages, program);
    else
        RESOLVE_FUNC_VOID(0, UseProgramStages)(pipeline, stages, program);
}

static void QOPENGLF_APIENTRY qopenglfResolveValidateProgramPipeline(GLuint pipeline)
{
    if (isES3(1))
        qgles3Helper()->ValidateProgramPipeline(pipeline);
    else
        RESOLVE_FUNC_VOID(0, ValidateProgramPipeline)(pipeline);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttribBinding(GLuint attribindex, GLuint bindingindex)
{
    if (isES3(1))
        qgles3Helper()->VertexAttribBinding(attribindex, bindingindex);
    else
        RESOLVE_FUNC_VOID(0, VertexAttribBinding)(attribindex, bindingindex);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    if (isES3(1))
        qgles3Helper()->VertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
    else
        RESOLVE_FUNC_VOID(0, VertexAttribFormat)(attribindex, size, type, normalized, relativeoffset);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    if (isES3(1))
        qgles3Helper()->VertexAttribIFormat(attribindex, size, type, relativeoffset);
    else
        RESOLVE_FUNC_VOID(0, VertexAttribIFormat)(attribindex, size, type, relativeoffset);
}

static void QOPENGLF_APIENTRY qopenglfResolveVertexBindingDivisor(GLuint bindingindex, GLuint divisor)
{
    if (isES3(1))
        qgles3Helper()->VertexBindingDivisor(bindingindex, divisor);
    else
        RESOLVE_FUNC_VOID(0, VertexBindingDivisor)(bindingindex, divisor);
}

/*!
    Constructs a default function resolver. The resolver cannot be used until
    \l {QOpenGLFunctions::}{initializeOpenGLFunctions()} is called to specify
    the context.
*/
QOpenGLExtraFunctions::QOpenGLExtraFunctions()
{
}

/*!
    Constructs a function resolver for context. If \a context is null, then
    the resolver will be created for the current QOpenGLContext.

    The context or another context in the group must be current.

    An object constructed in this way can only be used with context and other
    contexts that share with it. Use \l {QOpenGLFunctions::}
    {initializeOpenGLFunctions()} to change the object's context association.
*/
QOpenGLExtraFunctions::QOpenGLExtraFunctions(QOpenGLContext *context)
    : QOpenGLFunctions(context)
{
}

QOpenGLExtraFunctionsPrivate::QOpenGLExtraFunctionsPrivate(QOpenGLContext *ctx)
    : QOpenGLFunctionsPrivate(ctx)
{
    ReadBuffer = qopenglfResolveReadBuffer;
    DrawRangeElements = qopenglfResolveDrawRangeElements;
    TexImage3D = qopenglfResolveTexImage3D;
    TexSubImage3D = qopenglfResolveTexSubImage3D;
    CopyTexSubImage3D = qopenglfResolveCopyTexSubImage3D;
    CompressedTexImage3D = qopenglfResolveCompressedTexImage3D;
    CompressedTexSubImage3D = qopenglfResolveCompressedTexSubImage3D;
    GenQueries = qopenglfResolveGenQueries;
    DeleteQueries = qopenglfResolveDeleteQueries;
    IsQuery = qopenglfResolveIsQuery;
    BeginQuery = qopenglfResolveBeginQuery;
    EndQuery = qopenglfResolveEndQuery;
    GetQueryiv = qopenglfResolveGetQueryiv;
    GetQueryObjectuiv = qopenglfResolveGetQueryObjectuiv;
    UnmapBuffer = qopenglfResolveUnmapBuffer;
    GetBufferPointerv = qopenglfResolveGetBufferPointerv;
    DrawBuffers = qopenglfResolveDrawBuffers;
    UniformMatrix2x3fv = qopenglfResolveUniformMatrix2x3fv;
    UniformMatrix3x2fv = qopenglfResolveUniformMatrix3x2fv;
    UniformMatrix2x4fv = qopenglfResolveUniformMatrix2x4fv;
    UniformMatrix4x2fv = qopenglfResolveUniformMatrix4x2fv;
    UniformMatrix3x4fv = qopenglfResolveUniformMatrix3x4fv;
    UniformMatrix4x3fv = qopenglfResolveUniformMatrix4x3fv;
    BlitFramebuffer = qopenglfResolveBlitFramebuffer;
    RenderbufferStorageMultisample = qopenglfResolveRenderbufferStorageMultisample;
    FramebufferTextureLayer = qopenglfResolveFramebufferTextureLayer;
    MapBufferRange = qopenglfResolveMapBufferRange;
    FlushMappedBufferRange = qopenglfResolveFlushMappedBufferRange;
    BindVertexArray = qopenglfResolveBindVertexArray;
    DeleteVertexArrays = qopenglfResolveDeleteVertexArrays;
    GenVertexArrays = qopenglfResolveGenVertexArrays;
    IsVertexArray = qopenglfResolveIsVertexArray;
    GetIntegeri_v = qopenglfResolveGetIntegeri_v;
    BeginTransformFeedback = qopenglfResolveBeginTransformFeedback;
    EndTransformFeedback = qopenglfResolveEndTransformFeedback;
    BindBufferRange = qopenglfResolveBindBufferRange;
    BindBufferBase = qopenglfResolveBindBufferBase;
    TransformFeedbackVaryings = qopenglfResolveTransformFeedbackVaryings;
    GetTransformFeedbackVarying = qopenglfResolveGetTransformFeedbackVarying;
    VertexAttribIPointer = qopenglfResolveVertexAttribIPointer;
    GetVertexAttribIiv = qopenglfResolveGetVertexAttribIiv;
    GetVertexAttribIuiv = qopenglfResolveGetVertexAttribIuiv;
    VertexAttribI4i = qopenglfResolveVertexAttribI4i;
    VertexAttribI4ui = qopenglfResolveVertexAttribI4ui;
    VertexAttribI4iv = qopenglfResolveVertexAttribI4iv;
    VertexAttribI4uiv = qopenglfResolveVertexAttribI4uiv;
    GetUniformuiv = qopenglfResolveGetUniformuiv;
    GetFragDataLocation = qopenglfResolveGetFragDataLocation;
    Uniform1ui = qopenglfResolveUniform1ui;
    Uniform2ui = qopenglfResolveUniform2ui;
    Uniform3ui = qopenglfResolveUniform3ui;
    Uniform4ui = qopenglfResolveUniform4ui;
    Uniform1uiv = qopenglfResolveUniform1uiv;
    Uniform2uiv = qopenglfResolveUniform2uiv;
    Uniform3uiv = qopenglfResolveUniform3uiv;
    Uniform4uiv = qopenglfResolveUniform4uiv;
    ClearBufferiv = qopenglfResolveClearBufferiv;
    ClearBufferuiv = qopenglfResolveClearBufferuiv;
    ClearBufferfv = qopenglfResolveClearBufferfv;
    ClearBufferfi = qopenglfResolveClearBufferfi;
    GetStringi = qopenglfResolveGetStringi;
    CopyBufferSubData = qopenglfResolveCopyBufferSubData;
    GetUniformIndices = qopenglfResolveGetUniformIndices;
    GetActiveUniformsiv = qopenglfResolveGetActiveUniformsiv;
    GetUniformBlockIndex = qopenglfResolveGetUniformBlockIndex;
    GetActiveUniformBlockiv = qopenglfResolveGetActiveUniformBlockiv;
    GetActiveUniformBlockName = qopenglfResolveGetActiveUniformBlockName;
    UniformBlockBinding = qopenglfResolveUniformBlockBinding;
    DrawArraysInstanced = qopenglfResolveDrawArraysInstanced;
    DrawElementsInstanced = qopenglfResolveDrawElementsInstanced;
    FenceSync = qopenglfResolveFenceSync;
    IsSync = qopenglfResolveIsSync;
    DeleteSync = qopenglfResolveDeleteSync;
    ClientWaitSync = qopenglfResolveClientWaitSync;
    WaitSync = qopenglfResolveWaitSync;
    GetInteger64v = qopenglfResolveGetInteger64v;
    GetSynciv = qopenglfResolveGetSynciv;
    GetInteger64i_v = qopenglfResolveGetInteger64i_v;
    GetBufferParameteri64v = qopenglfResolveGetBufferParameteri64v;
    GenSamplers = qopenglfResolveGenSamplers;
    DeleteSamplers = qopenglfResolveDeleteSamplers;
    IsSampler = qopenglfResolveIsSampler;
    BindSampler = qopenglfResolveBindSampler;
    SamplerParameteri = qopenglfResolveSamplerParameteri;
    SamplerParameteriv = qopenglfResolveSamplerParameteriv;
    SamplerParameterf = qopenglfResolveSamplerParameterf;
    SamplerParameterfv = qopenglfResolveSamplerParameterfv;
    GetSamplerParameteriv = qopenglfResolveGetSamplerParameteriv;
    GetSamplerParameterfv = qopenglfResolveGetSamplerParameterfv;
    VertexAttribDivisor = qopenglfResolveVertexAttribDivisor;
    BindTransformFeedback = qopenglfResolveBindTransformFeedback;
    DeleteTransformFeedbacks = qopenglfResolveDeleteTransformFeedbacks;
    GenTransformFeedbacks = qopenglfResolveGenTransformFeedbacks;
    IsTransformFeedback = qopenglfResolveIsTransformFeedback;
    PauseTransformFeedback = qopenglfResolvePauseTransformFeedback;
    ResumeTransformFeedback = qopenglfResolveResumeTransformFeedback;
    GetProgramBinary = qopenglfResolveGetProgramBinary;
    ProgramBinary = qopenglfResolveProgramBinary;
    ProgramParameteri = qopenglfResolveProgramParameteri;
    InvalidateFramebuffer = qopenglfResolveInvalidateFramebuffer;
    InvalidateSubFramebuffer = qopenglfResolveInvalidateSubFramebuffer;
    TexStorage2D = qopenglfResolveTexStorage2D;
    TexStorage3D = qopenglfResolveTexStorage3D;
    GetInternalformativ = qopenglfResolveGetInternalformativ;

    DispatchCompute = qopenglfResolveDispatchCompute;
    DispatchComputeIndirect = qopenglfResolveDispatchComputeIndirect;
    DrawArraysIndirect = qopenglfResolveDrawArraysIndirect;
    DrawElementsIndirect = qopenglfResolveDrawElementsIndirect;
    FramebufferParameteri = qopenglfResolveFramebufferParameteri;
    GetFramebufferParameteriv = qopenglfResolveGetFramebufferParameteriv;
    GetProgramInterfaceiv = qopenglfResolveGetProgramInterfaceiv;
    GetProgramResourceIndex = qopenglfResolveGetProgramResourceIndex;
    GetProgramResourceName = qopenglfResolveGetProgramResourceName;
    GetProgramResourceiv = qopenglfResolveGetProgramResourceiv;
    GetProgramResourceLocation = qopenglfResolveGetProgramResourceLocation;
    UseProgramStages = qopenglfResolveUseProgramStages;
    ActiveShaderProgram = qopenglfResolveActiveShaderProgram;
    CreateShaderProgramv = qopenglfResolveCreateShaderProgramv;
    BindProgramPipeline = qopenglfResolveBindProgramPipeline;
    DeleteProgramPipelines = qopenglfResolveDeleteProgramPipelines;
    GenProgramPipelines = qopenglfResolveGenProgramPipelines;
    IsProgramPipeline = qopenglfResolveIsProgramPipeline;
    GetProgramPipelineiv = qopenglfResolveGetProgramPipelineiv;
    ProgramUniform1i = qopenglfResolveProgramUniform1i;
    ProgramUniform2i = qopenglfResolveProgramUniform2i;
    ProgramUniform3i = qopenglfResolveProgramUniform3i;
    ProgramUniform4i = qopenglfResolveProgramUniform4i;
    ProgramUniform1ui = qopenglfResolveProgramUniform1ui;
    ProgramUniform2ui = qopenglfResolveProgramUniform2ui;
    ProgramUniform3ui = qopenglfResolveProgramUniform3ui;
    ProgramUniform4ui = qopenglfResolveProgramUniform4ui;
    ProgramUniform1f = qopenglfResolveProgramUniform1f;
    ProgramUniform2f = qopenglfResolveProgramUniform2f;
    ProgramUniform3f = qopenglfResolveProgramUniform3f;
    ProgramUniform4f = qopenglfResolveProgramUniform4f;
    ProgramUniform1iv = qopenglfResolveProgramUniform1iv;
    ProgramUniform2iv = qopenglfResolveProgramUniform2iv;
    ProgramUniform3iv = qopenglfResolveProgramUniform3iv;
    ProgramUniform4iv = qopenglfResolveProgramUniform4iv;
    ProgramUniform1uiv = qopenglfResolveProgramUniform1uiv;
    ProgramUniform2uiv = qopenglfResolveProgramUniform2uiv;
    ProgramUniform3uiv = qopenglfResolveProgramUniform3uiv;
    ProgramUniform4uiv = qopenglfResolveProgramUniform4uiv;
    ProgramUniform1fv = qopenglfResolveProgramUniform1fv;
    ProgramUniform2fv = qopenglfResolveProgramUniform2fv;
    ProgramUniform3fv = qopenglfResolveProgramUniform3fv;
    ProgramUniform4fv = qopenglfResolveProgramUniform4fv;
    ProgramUniformMatrix2fv = qopenglfResolveProgramUniformMatrix2fv;
    ProgramUniformMatrix3fv = qopenglfResolveProgramUniformMatrix3fv;
    ProgramUniformMatrix4fv = qopenglfResolveProgramUniformMatrix4fv;
    ProgramUniformMatrix2x3fv = qopenglfResolveProgramUniformMatrix2x3fv;
    ProgramUniformMatrix3x2fv = qopenglfResolveProgramUniformMatrix3x2fv;
    ProgramUniformMatrix2x4fv = qopenglfResolveProgramUniformMatrix2x4fv;
    ProgramUniformMatrix4x2fv = qopenglfResolveProgramUniformMatrix4x2fv;
    ProgramUniformMatrix3x4fv = qopenglfResolveProgramUniformMatrix3x4fv;
    ProgramUniformMatrix4x3fv = qopenglfResolveProgramUniformMatrix4x3fv;
    ValidateProgramPipeline = qopenglfResolveValidateProgramPipeline;
    GetProgramPipelineInfoLog = qopenglfResolveGetProgramPipelineInfoLog;
    BindImageTexture = qopenglfResolveBindImageTexture;
    GetBooleani_v = qopenglfResolveGetBooleani_v;
    MemoryBarrierFunc = qopenglfResolveMemoryBarrier;
    MemoryBarrierByRegion = qopenglfResolveMemoryBarrierByRegion;
    TexStorage2DMultisample = qopenglfResolveTexStorage2DMultisample;
    GetMultisamplefv = qopenglfResolveGetMultisamplefv;
    SampleMaski = qopenglfResolveSampleMaski;
    GetTexLevelParameteriv = qopenglfResolveGetTexLevelParameteriv;
    GetTexLevelParameterfv = qopenglfResolveGetTexLevelParameterfv;
    BindVertexBuffer = qopenglfResolveBindVertexBuffer;
    VertexAttribFormat = qopenglfResolveVertexAttribFormat;
    VertexAttribIFormat = qopenglfResolveVertexAttribIFormat;
    VertexAttribBinding = qopenglfResolveVertexAttribBinding;
    VertexBindingDivisor = qopenglfResolveVertexBindingDivisor;
}

QOpenGLExtensionsPrivate::QOpenGLExtensionsPrivate(QOpenGLContext *ctx)
    : QOpenGLExtraFunctionsPrivate(ctx),
      flushVendorChecked(false)
{
    MapBuffer = qopenglfResolveMapBuffer;
    GetBufferSubData = qopenglfResolveGetBufferSubData;
    DiscardFramebuffer = qopenglfResolveDiscardFramebuffer;
}

QOpenGLES3Helper *QOpenGLExtensions::gles3Helper()
{
    return qgles3Helper();
}

void QOpenGLExtensions::flushShared()
{
    Q_D(QOpenGLExtensions);

    if (!d->flushVendorChecked) {
        d->flushVendorChecked = true;
        // It is not quite clear if glFlush() is sufficient to synchronize access to
        // resources between sharing contexts in the same thread. On most platforms this
        // is enough (e.g. iOS explicitly documents it), while certain drivers only work
        // properly when doing glFinish().
        d->flushIsSufficientToSyncContexts = false; // default to false, not guaranteed by the spec
        const char *vendor = (const char *) glGetString(GL_VENDOR);
        if (vendor) {
            static const char *const flushEnough[] = { "Apple", "ATI", "Intel", "NVIDIA" };
            for (size_t i = 0; i < sizeof(flushEnough) / sizeof(const char *); ++i) {
                if (strstr(vendor, flushEnough[i])) {
                    d->flushIsSufficientToSyncContexts = true;
                    break;
                }
            }
        }
    }

    if (d->flushIsSufficientToSyncContexts)
        glFlush();
    else
        glFinish();
}

