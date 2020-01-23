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

#include <qopenglgradientcache_p.h>
#include <qdrawhelper_p.h>
#include <qopenglcontext_p.h>
#include <qrgba64_p.h>
#include <qmutex.h>
#include <qopenglfunctions.h>
#include <qopenglextensions_p.h>

#ifndef GL_RGBA16
#define GL_RGBA16   0x805B
#endif

QT_BEGIN_NAMESPACE

class QOpenGL2GradientCacheWrapper
{
public:
    QOpenGL2GradientCache *cacheForContext(QOpenGLContext *context) {
        QMutexLocker lock(&m_mutex);
        return m_resource.value<QOpenGL2GradientCache>(context);
    }

private:
    QOpenGLMultiGroupSharedResource m_resource;
    QMutex m_mutex;
};

Q_GLOBAL_STATIC(QOpenGL2GradientCacheWrapper, qt_gradient_caches)

QOpenGL2GradientCache::QOpenGL2GradientCache(QOpenGLContext *ctx)
    : QOpenGLSharedResource(ctx->shareGroup())
{
}

QOpenGL2GradientCache::~QOpenGL2GradientCache()
{
    cache.clear();
}

QOpenGL2GradientCache *QOpenGL2GradientCache::cacheForContext(QOpenGLContext *context)
{
    return qt_gradient_caches()->cacheForContext(context);
}

void QOpenGL2GradientCache::invalidateResource()
{
    QMutexLocker lock(&m_mutex);
    cache.clear();
}

void QOpenGL2GradientCache::freeResource(QOpenGLContext *)
{
    cleanCache();
}

void QOpenGL2GradientCache::cleanCache()
{
    QMutexLocker lock(&m_mutex);
    QOpenGLGradientColorTableHash::const_iterator it = cache.constBegin();
    QOpenGLFunctions *funcs = QOpenGLContext::currentContext()->functions();
    for (; it != cache.constEnd(); ++it) {
        const CacheInfo &cache_info = it.value();
        funcs->glDeleteTextures(1, &cache_info.texId);
    }
    cache.clear();
}

GLuint QOpenGL2GradientCache::getBuffer(const QGradient &gradient, qreal opacity)
{
    quint64 hash_val = 0;

    QGradientStops stops = gradient.stops();
    for (int i = 0; i < stops.size() && i <= 2; i++)
        hash_val += stops[i].second.rgba();

    const QMutexLocker lock(&m_mutex);
    QOpenGLGradientColorTableHash::const_iterator it = cache.constFind(hash_val);

    if (it == cache.constEnd())
        return addCacheElement(hash_val, gradient, opacity);
    else {
        do {
            const CacheInfo &cache_info = it.value();
            if (cache_info.stops == stops && cache_info.opacity == opacity
                && cache_info.interpolationMode == gradient.interpolationMode())
            {
                return cache_info.texId;
            }
            ++it;
        } while (it != cache.constEnd() && it.key() == hash_val);
        // an exact match for these stops and opacity was not found, create new cache
        return addCacheElement(hash_val, gradient, opacity);
    }
}


GLuint QOpenGL2GradientCache::addCacheElement(quint64 hash_val, const QGradient &gradient, qreal opacity)
{
    QOpenGLFunctions *funcs = QOpenGLContext::currentContext()->functions();
    if (cache.size() == maxCacheSize()) {
        int elem_to_remove = qrand() % maxCacheSize();
        quint64 key = cache.keys()[elem_to_remove];

        // need to call glDeleteTextures on each removed cache entry:
        QOpenGLGradientColorTableHash::const_iterator it = cache.constFind(key);
        do {
            funcs->glDeleteTextures(1, &it.value().texId);
        } while (++it != cache.constEnd() && it.key() == key);
        cache.remove(key); // may remove more than 1, but OK
    }

    CacheInfo cache_entry(gradient.stops(), opacity, gradient.interpolationMode());
    funcs->glGenTextures(1, &cache_entry.texId);
    funcs->glBindTexture(GL_TEXTURE_2D, cache_entry.texId);
    if (static_cast<QOpenGLExtensions *>(funcs)->hasOpenGLExtension(QOpenGLExtensions::Sized16Formats)) {
        QRgba64 buffer[1024];
        generateGradientColorTable(gradient, buffer, paletteSize(), opacity);
        funcs->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, paletteSize(), 1,
                            0, GL_RGBA, GL_UNSIGNED_SHORT, buffer);
    } else {
        uint buffer[1024];
        generateGradientColorTable(gradient, buffer, paletteSize(), opacity);
        funcs->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, paletteSize(), 1,
                            0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    }
    return cache.insert(hash_val, cache_entry).value().texId;
}


//TODO: Let GL generate the texture using an FBO
void QOpenGL2GradientCache::generateGradientColorTable(const QGradient& gradient, QRgba64 *colorTable, int size, qreal opacity) const
{
    int pos = 0;
    QGradientStops s = gradient.stops();
    QVector<QRgba64> colors(s.size());

    for (int i = 0; i < s.size(); ++i)
        colors[i] = s[i].second.rgba64();

    bool colorInterpolation = (gradient.interpolationMode() == QGradient::ColorInterpolation);

    uint alpha = qRound(opacity * 256);
    QRgba64 current_color = combineAlpha256(colors[0], alpha);
    qreal incr = 1.0 / qreal(size);
    qreal fpos = 1.5 * incr;
    colorTable[pos++] = qPremultiply(current_color);

    while (fpos <= s.first().first) {
        colorTable[pos] = colorTable[pos - 1];
        pos++;
        fpos += incr;
    }

    if (colorInterpolation)
        current_color = qPremultiply(current_color);

    for (int i = 0; i < s.size() - 1; ++i) {
        qreal delta = 1/(s[i+1].first - s[i].first);
        QRgba64 next_color = combineAlpha256(colors[i+1], alpha);
        if (colorInterpolation)
            next_color = qPremultiply(next_color);

        while (fpos < s[i+1].first && pos < size) {
            int dist = int(256 * ((fpos - s[i].first) * delta));
            int idist = 256 - dist;
            if (colorInterpolation)
                colorTable[pos] = interpolate256(current_color, idist, next_color, dist);
            else
                colorTable[pos] = qPremultiply(interpolate256(current_color, idist, next_color, dist));
            ++pos;
            fpos += incr;
        }
        current_color = next_color;
    }

    Q_ASSERT(s.size() > 0);

    QRgba64 last_color = qPremultiply(combineAlpha256(colors[s.size() - 1], alpha));
    for (;pos < size; ++pos)
        colorTable[pos] = last_color;

    // Make sure the last color stop is represented at the end of the table
    colorTable[size-1] = last_color;
}

void QOpenGL2GradientCache::generateGradientColorTable(const QGradient& gradient, uint *colorTable, int size, qreal opacity) const
{
    int pos = 0;
    QGradientStops s = gradient.stops();
    QVector<uint> colors(s.size());

    for (int i = 0; i < s.size(); ++i)
        colors[i] = s[i].second.rgba(); // Qt LIES! It returns ARGB (on little-endian AND on big-endian)

    bool colorInterpolation = (gradient.interpolationMode() == QGradient::ColorInterpolation);

    uint alpha = qRound(opacity * 256);
    uint current_color = ARGB_COMBINE_ALPHA(colors[0], alpha);
    qreal incr = 1.0 / qreal(size);
    qreal fpos = 1.5 * incr;
    colorTable[pos++] = ARGB2RGBA(qPremultiply(current_color));

    while (fpos <= s.first().first) {
        colorTable[pos] = colorTable[pos - 1];
        pos++;
        fpos += incr;
    }

    if (colorInterpolation)
        current_color = qPremultiply(current_color);

    for (int i = 0; i < s.size() - 1; ++i) {
        qreal delta = 1/(s[i+1].first - s[i].first);
        uint next_color = ARGB_COMBINE_ALPHA(colors[i+1], alpha);
        if (colorInterpolation)
            next_color = qPremultiply(next_color);

        while (fpos < s[i+1].first && pos < size) {
            int dist = int(256 * ((fpos - s[i].first) * delta));
            int idist = 256 - dist;
            if (colorInterpolation)
                colorTable[pos] = ARGB2RGBA(INTERPOLATE_PIXEL_256(current_color, idist, next_color, dist));
            else
                colorTable[pos] = ARGB2RGBA(qPremultiply(INTERPOLATE_PIXEL_256(current_color, idist, next_color, dist)));
            ++pos;
            fpos += incr;
        }
        current_color = next_color;
    }

    Q_ASSERT(s.size() > 0);

    uint last_color = ARGB2RGBA(qPremultiply(ARGB_COMBINE_ALPHA(colors[s.size() - 1], alpha)));
    for (;pos < size; ++pos)
        colorTable[pos] = last_color;

    // Make sure the last color stop is represented at the end of the table
    colorTable[size-1] = last_color;
}

QT_END_NAMESPACE
