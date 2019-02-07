/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QOPENGLGRADIENTCACHE_P_H
#define QOPENGLGRADIENTCACHE_P_H

#include <QMultiHash>
#include <QObject>
#include <qopenglcontext_p.h>
#include <qmutex.h>
#include <QGradient>
#include <qrgba64.h>

QT_BEGIN_NAMESPACE

class QOpenGL2GradientCache : public QOpenGLSharedResource
{
    struct CacheInfo
    {
        inline CacheInfo(QGradientStops s, qreal op, QGradient::InterpolationMode mode) :
            stops(std::move(s)), opacity(op), interpolationMode(mode) {}

        GLuint texId;
        QGradientStops stops;
        qreal opacity;
        QGradient::InterpolationMode interpolationMode;
    };

    typedef QMultiHash<quint64, CacheInfo> QOpenGLGradientColorTableHash;

public:
    static QOpenGL2GradientCache *cacheForContext(QOpenGLContext *context);

    QOpenGL2GradientCache(QOpenGLContext *);
    ~QOpenGL2GradientCache();

    GLuint getBuffer(const QGradient &gradient, qreal opacity);
    inline int paletteSize() const { return 1024; }

    void invalidateResource() override;
    void freeResource(QOpenGLContext *ctx) override;

private:
    inline int maxCacheSize() const { return 60; }
    inline void generateGradientColorTable(const QGradient& gradient,
                                           QRgba64 *colorTable,
                                           int size, qreal opacity) const;
    inline void generateGradientColorTable(const QGradient& gradient,
                                           uint *colorTable,
                                           int size, qreal opacity) const;
    GLuint addCacheElement(quint64 hash_val, const QGradient &gradient, qreal opacity);
    void cleanCache();

    QOpenGLGradientColorTableHash cache;
    QMutex m_mutex;
};

QT_END_NAMESPACE

#endif // QOPENGLGRADIENTCACHE_P_H
