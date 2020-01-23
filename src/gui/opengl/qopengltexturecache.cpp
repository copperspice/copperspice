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

#include <qopengltexturecache_p.h>
#include <qmath.h>
#include <qopenglfunctions.h>
#include <qopenglcontext_p.h>
#include <qopenglextensions_p.h>
#include <qimagepixmapcleanuphooks_p.h>
#include <qplatform_pixmap.h>

#ifndef GL_RED
#define GL_RED                            0x1903
#endif

#ifndef GL_RGB10_A2
#define GL_RGB10_A2                       0x8059
#endif

#ifndef GL_BGR
#define GL_BGR 0x80E0
#endif

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#endif

#ifndef GL_UNSIGNED_INT_2_10_10_10_REV
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#endif

class QOpenGLTextureCacheWrapper
{
public:
    QOpenGLTextureCacheWrapper()
    {
        QImagePixmapCleanupHooks::instance()->addPlatformPixmapModificationHook(cleanupTexturesForPixmapData);
        QImagePixmapCleanupHooks::instance()->addPlatformPixmapDestructionHook(cleanupTexturesForPixmapData);
        QImagePixmapCleanupHooks::instance()->addImageHook(cleanupTexturesForCacheKey);
    }

    ~QOpenGLTextureCacheWrapper()
    {
        QImagePixmapCleanupHooks::instance()->removePlatformPixmapModificationHook(cleanupTexturesForPixmapData);
        QImagePixmapCleanupHooks::instance()->removePlatformPixmapDestructionHook(cleanupTexturesForPixmapData);
        QImagePixmapCleanupHooks::instance()->removeImageHook(cleanupTexturesForCacheKey);
    }

    QOpenGLTextureCache *cacheForContext(QOpenGLContext *context) {
        QMutexLocker lock(&m_mutex);
        return m_resource.value<QOpenGLTextureCache>(context);
    }

    static void cleanupTexturesForCacheKey(qint64 key);
    static void cleanupTexturesForPixmapData(QPlatformPixmap *pmd);

private:
    QOpenGLMultiGroupSharedResource m_resource;
    QMutex m_mutex;
};

Q_GLOBAL_STATIC(QOpenGLTextureCacheWrapper, qt_texture_caches)

QOpenGLTextureCache *QOpenGLTextureCache::cacheForContext(QOpenGLContext *context)
{
    return qt_texture_caches()->cacheForContext(context);
}

void QOpenGLTextureCacheWrapper::cleanupTexturesForCacheKey(qint64 key)
{
    QList<QOpenGLSharedResource *> resources = qt_texture_caches()->m_resource.resources();
    for (QList<QOpenGLSharedResource *>::iterator it = resources.begin(); it != resources.end(); ++it)
        static_cast<QOpenGLTextureCache *>(*it)->invalidate(key);
}

void QOpenGLTextureCacheWrapper::cleanupTexturesForPixmapData(QPlatformPixmap *pmd)
{
    cleanupTexturesForCacheKey(pmd->cacheKey());
}

QOpenGLTextureCache::QOpenGLTextureCache(QOpenGLContext *ctx)
    : QOpenGLSharedResource(ctx->shareGroup())
    , m_cache(64 * 1024) // 64 MB cache
{
}

QOpenGLTextureCache::~QOpenGLTextureCache()
{
}

GLuint QOpenGLTextureCache::bindTexture(QOpenGLContext *context, const QPixmap &pixmap, BindOptions options)
{
    if (pixmap.isNull())
        return 0;
    QMutexLocker locker(&m_mutex);
    qint64 key = pixmap.cacheKey();

    // A QPainter is active on the image - take the safe route and replace the texture.
    if (!pixmap.paintingActive()) {
        QOpenGLCachedTexture *entry = m_cache.object(key);
        if (entry && entry->options() == options) {
            context->functions()->glBindTexture(GL_TEXTURE_2D, entry->id());
            return entry->id();
        }
    }

    GLuint id = bindTexture(context, key, pixmap.toImage(), options);
    if (id > 0)
        QImagePixmapCleanupHooks::enableCleanupHooks(pixmap);

    return id;
}

GLuint QOpenGLTextureCache::bindTexture(QOpenGLContext *context, const QImage &image, BindOptions options)
{
    if (image.isNull())
        return 0;
    QMutexLocker locker(&m_mutex);
    qint64 key = image.cacheKey();

    // A QPainter is active on the image - take the safe route and replace the texture.
    if (!image.paintingActive()) {
        QOpenGLCachedTexture *entry = m_cache.object(key);
        if (entry && entry->options() == options) {
            context->functions()->glBindTexture(GL_TEXTURE_2D, entry->id());
            return entry->id();
        }
    }

    QImage img = image;
    if (!context->functions()->hasOpenGLFeature(QOpenGLFunctions::NPOTTextures)) {
        // Scale the pixmap if needed. GL textures needs to have the
        // dimensions 2^n+2(border) x 2^m+2(border), unless we're using GL
        // 2.0 or use the GL_TEXTURE_RECTANGLE texture target
        int tx_w = qNextPowerOfTwo(image.width() - 1);
        int tx_h = qNextPowerOfTwo(image.height() - 1);
        if (tx_w != image.width() || tx_h != image.height()) {
            img = img.scaled(tx_w, tx_h);
        }
    }

    GLuint id = bindTexture(context, key, img, options);
    if (id > 0)
        QImagePixmapCleanupHooks::enableCleanupHooks(image);

    return id;
}

GLuint QOpenGLTextureCache::bindTexture(QOpenGLContext *context, qint64 key, const QImage &image, BindOptions options)
{
    GLuint id;
    QOpenGLFunctions *funcs = context->functions();
    funcs->glGenTextures(1, &id);
    funcs->glBindTexture(GL_TEXTURE_2D, id);

    QImage tx;
    GLenum externalFormat;
    GLenum internalFormat;
    GLuint pixelType;
    QImage::Format targetFormat = QImage::Format_Invalid;
    const bool isOpenGL12orBetter = !context->isOpenGLES() && (context->format().majorVersion() >= 2 || context->format().minorVersion() >= 2);

    switch (image.format()) {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        if (isOpenGL12orBetter) {
            externalFormat = GL_BGRA;
            internalFormat = GL_RGBA;
            pixelType = GL_UNSIGNED_INT_8_8_8_8_REV;
        } else  {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
            // Without GL_UNSIGNED_INT_8_8_8_8_REV, BGRA only matches ARGB on little endian.
            break;
#endif
            if (static_cast<QOpenGLExtensions*>(context->functions())->hasOpenGLExtension(QOpenGLExtensions::BGRATextureFormat)) {
                // GL_EXT_bgra or GL_EXT_texture_format_BGRA8888 extensions.
                if (context->isOpenGLES()) {
                    // The GL_EXT_texture_format_BGRA8888 extension requires the internal format to match the external.
                    externalFormat = internalFormat = GL_BGRA;
                } else {
                    // OpenGL BGRA/BGR format is not allowed as an internal format
                    externalFormat = GL_BGRA;
                    internalFormat = GL_RGBA;
                }
                pixelType = GL_UNSIGNED_BYTE;
            } else if (context->isOpenGLES() && context->hasExtension("GL_APPLE_texture_format_BGRA8888")) {
                // Is only allowed as an external format like OpenGL.
                externalFormat = GL_BGRA;
                internalFormat = GL_RGBA;
                pixelType = GL_UNSIGNED_BYTE;
            } else {
                // No support for direct ARGB32 upload.
                break;
            }
        }
        targetFormat = image.format();
        break;
    case QImage::Format_BGR30:
    case QImage::Format_A2BGR30_Premultiplied:
        if (isOpenGL12orBetter || (context->isOpenGLES() && context->format().majorVersion() >= 3)) {
            pixelType = GL_UNSIGNED_INT_2_10_10_10_REV;
            externalFormat = GL_RGBA;
            internalFormat = GL_RGB10_A2;
            targetFormat =  image.format();
        }
        break;
    case QImage::Format_RGB30:
    case QImage::Format_A2RGB30_Premultiplied:
        if (isOpenGL12orBetter) {
            pixelType = GL_UNSIGNED_INT_2_10_10_10_REV;
            externalFormat = GL_BGRA;
            internalFormat = GL_RGB10_A2;
            targetFormat = image.format();
        } else if (context->isOpenGLES() && context->format().majorVersion() >= 3) {
            pixelType = GL_UNSIGNED_INT_2_10_10_10_REV;
            externalFormat = GL_RGBA;
            internalFormat = GL_RGB10_A2;
            targetFormat = QImage::Format_A2BGR30_Premultiplied;
        }
        break;
    case QImage::Format_RGB444:
    case QImage::Format_RGB555:
    case QImage::Format_RGB16:
        if (isOpenGL12orBetter || context->isOpenGLES()) {
            externalFormat = internalFormat = GL_RGB;
            pixelType = GL_UNSIGNED_SHORT_5_6_5;
            targetFormat = QImage::Format_RGB16;
        }
        break;
    case QImage::Format_RGB666:
    case QImage::Format_RGB888:
        externalFormat = internalFormat = GL_RGB;
        pixelType = GL_UNSIGNED_BYTE;
        targetFormat = QImage::Format_RGB888;
        break;
    case QImage::Format_RGBX8888:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
        externalFormat = internalFormat = GL_RGBA;
        pixelType = GL_UNSIGNED_BYTE;
        targetFormat = image.format();
        break;
    case QImage::Format_Indexed8:
        if (options & UseRedFor8BitBindOption) {
            externalFormat = internalFormat = GL_RED;
            pixelType = GL_UNSIGNED_BYTE;
            targetFormat = image.format();
        }
        break;
    case QImage::Format_Alpha8:
        if (options & UseRedFor8BitBindOption) {
            externalFormat = internalFormat = GL_RED;
            pixelType = GL_UNSIGNED_BYTE;
            targetFormat = image.format();
        } else if (context->isOpenGLES() || context->format().profile() != QSurfaceFormat::CoreProfile) {
            externalFormat = internalFormat = GL_ALPHA;
            pixelType = GL_UNSIGNED_BYTE;
            targetFormat = image.format();
        }
        break;
    case QImage::Format_Grayscale8:
        if (options & UseRedFor8BitBindOption) {
            externalFormat = internalFormat = GL_RED;
            pixelType = GL_UNSIGNED_BYTE;
            targetFormat = image.format();
        } else if (context->isOpenGLES() || context->format().profile() != QSurfaceFormat::CoreProfile) {
            externalFormat = internalFormat = GL_LUMINANCE;
            pixelType = GL_UNSIGNED_BYTE;
            targetFormat = image.format();
        }
        break;
    default:
        break;
    }

    if (targetFormat == QImage::Format_Invalid) {
        externalFormat = internalFormat = GL_RGBA;
        pixelType = GL_UNSIGNED_BYTE;
        if (!image.hasAlphaChannel())
            targetFormat = QImage::Format_RGBX8888;
        else
            targetFormat = QImage::Format_RGBA8888;
    }

    if (options & PremultipliedAlphaBindOption) {
        if (targetFormat == QImage::Format_ARGB32)
            targetFormat = QImage::Format_ARGB32_Premultiplied;
        else if (targetFormat == QImage::Format_RGBA8888)
            targetFormat = QImage::Format_RGBA8888_Premultiplied;
    } else {
        if (targetFormat == QImage::Format_ARGB32_Premultiplied)
            targetFormat = QImage::Format_ARGB32;
        else if (targetFormat == QImage::Format_RGBA8888_Premultiplied)
            targetFormat = QImage::Format_RGBA8888;
    }

    if (image.format() != targetFormat)
        tx = image.convertToFormat(targetFormat);
    else
        tx = image;

    funcs->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, tx.width(), tx.height(), 0, externalFormat, pixelType, const_cast<const QImage &>(tx).bits());

    int cost = tx.width() * tx.height() * tx.depth() / (1024 * 8);
    m_cache.insert(key, new QOpenGLCachedTexture(id, options, context), cost);

    return id;
}

void QOpenGLTextureCache::invalidate(qint64 key)
{
    QMutexLocker locker(&m_mutex);
    m_cache.remove(key);
}

void QOpenGLTextureCache::invalidateResource()
{
    m_cache.clear();
}

void QOpenGLTextureCache::freeResource(QOpenGLContext *)
{
    Q_ASSERT(false); // the texture cache lives until the context group disappears
}

static void freeTexture(QOpenGLFunctions *funcs, GLuint id)
{
    funcs->glDeleteTextures(1, &id);
}

QOpenGLCachedTexture::QOpenGLCachedTexture(GLuint id, int options, QOpenGLContext *context) : m_options(options)
{
    m_resource = new QOpenGLSharedResourceGuard(context, id, freeTexture);
}

