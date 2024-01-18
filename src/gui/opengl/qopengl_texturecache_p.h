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

#ifndef QOPENGLTEXTURECACHE_P_H
#define QOPENGLTEXTURECACHE_P_H

#include <qhash.h>
#include <qobject.h>
#include <qcache.h>
#include <qmutex.h>

#include <qopenglcontext_p.h>

class QOpenGLCachedTexture;

class Q_GUI_EXPORT QOpenGLTextureCache : public QOpenGLSharedResource
{
public:
   static QOpenGLTextureCache *cacheForContext(QOpenGLContext *context);

   QOpenGLTextureCache(QOpenGLContext *);
   ~QOpenGLTextureCache();

   enum BindOption {
      NoBindOption                  = 0x0000,
      PremultipliedAlphaBindOption  = 0x0001,
      UseRedFor8BitBindOption       = 0x0002,
   };
   using BindOptions = QFlags<BindOption>;

   GLuint bindTexture(QOpenGLContext *context, const QPixmap &pixmap,
               QOpenGLTextureCache::BindOptions options = PremultipliedAlphaBindOption);

   GLuint bindTexture(QOpenGLContext *context, const QImage &image,
               QOpenGLTextureCache::BindOptions options = PremultipliedAlphaBindOption);

   void invalidate(qint64 key);

   void invalidateResource() override;
   void freeResource(QOpenGLContext *ctx) override;

private:
   GLuint bindTexture(QOpenGLContext *context, qint64 key, const QImage &image, QOpenGLTextureCache::BindOptions options);

   QMutex m_mutex;
   QCache<quint64, QOpenGLCachedTexture> m_cache;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QOpenGLTextureCache::BindOptions)

class QOpenGLCachedTexture
{
public:
   QOpenGLCachedTexture(GLuint id, QOpenGLTextureCache::BindOptions options, QOpenGLContext *context);
   ~QOpenGLCachedTexture() { m_resource->free(); }

   GLuint id() const {
      return m_resource->id();
   }

   QOpenGLTextureCache::BindOptions options() const {
      return m_options;
   }

private:
   QOpenGLSharedResourceGuard *m_resource;
   QOpenGLTextureCache::BindOptions m_options;
};

#endif

