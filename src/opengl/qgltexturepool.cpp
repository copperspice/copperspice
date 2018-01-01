/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
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

#include "qgltexturepool_p.h"
#include "qpixmapdata_gl_p.h"
#include "qgl_p.h"

QT_BEGIN_NAMESPACE

Q_OPENGL_EXPORT extern QGLWidget *qt_gl_share_widget();

static QGLTexturePool *qt_gl_texture_pool = 0;

class QGLTexturePoolPrivate
{
 public:
   QGLTexturePoolPrivate() : lruFirst(0), lruLast(0) {}

   QGLTexture *lruFirst;
   QGLTexture *lruLast;
};

QGLTexturePool::QGLTexturePool()
   : d_ptr(new QGLTexturePoolPrivate())
{
}

QGLTexturePool::~QGLTexturePool()
{
}

QGLTexturePool *QGLTexturePool::instance()
{
   if (!qt_gl_texture_pool) {
      qt_gl_texture_pool = new QGLTexturePool();
   }
   return qt_gl_texture_pool;
}

GLuint QGLTexturePool::createTexture(GLenum target,
                                     GLint level,
                                     GLint internalformat,
                                     GLsizei width,
                                     GLsizei height,
                                     GLenum format,
                                     GLenum type,
                                     QGLTexture *texture)
{
   GLuint tex;
   glGenTextures(1, &tex);
   glBindTexture(target, tex);
   do {
      glTexImage2D(target, level, internalformat, width, height, 0, format, type, 0);
      GLenum error = glGetError();
      if (error == GL_NO_ERROR) {
         if (texture) {
            moveToHeadOfLRU(texture);
         }
         return tex;
      } else if (error != GL_OUT_OF_MEMORY) {
         qWarning("QGLTexturePool: cannot create temporary texture because of invalid params");
         return 0;
      }
   } while (reclaimSpace(internalformat, width, height, format, type, texture));
   qWarning("QGLTexturePool: cannot reclaim sufficient space for a %dx%d texture",
            width, height);
   return 0;
}

bool QGLTexturePool::createPermanentTexture(GLuint tex,
      GLenum target,
      GLint level,
      GLint internalformat,
      GLsizei width,
      GLsizei height,
      GLenum format,
      GLenum type,
      const GLvoid *data)
{
   glBindTexture(target, tex);
   do {
      glTexImage2D(target, level, internalformat, width, height, 0, format, type, data);

      GLenum error = glGetError();
      if (error == GL_NO_ERROR) {
         return true;
      } else if (error != GL_OUT_OF_MEMORY) {
         qWarning("QGLTexturePool: cannot create permanent texture because of invalid params");
         return false;
      }
   } while (reclaimSpace(internalformat, width, height, format, type, 0));
   qWarning("QGLTexturePool: cannot reclaim sufficient space for a %dx%d texture",
            width, height);
   return 0;
}

void QGLTexturePool::useTexture(QGLTexture *texture)
{
   moveToHeadOfLRU(texture);
   texture->inTexturePool = true;
}

void QGLTexturePool::detachTexture(QGLTexture *texture)
{
   removeFromLRU(texture);
   texture->inTexturePool = false;
}

bool QGLTexturePool::reclaimSpace(GLint internalformat,
                                  GLsizei width,
                                  GLsizei height,
                                  GLenum format,
                                  GLenum type,
                                  QGLTexture *texture)
{
   Q_UNUSED(internalformat);   // For future use in picking the best texture to eject.
   Q_UNUSED(width);
   Q_UNUSED(height);
   Q_UNUSED(format);
   Q_UNUSED(type);

   bool succeeded = false;
   bool wasInLRU = false;
   if (texture) {
      wasInLRU = texture->inLRU;
      moveToHeadOfLRU(texture);
   }

   QGLTexture *lrutexture = textureLRU();
   if (lrutexture && lrutexture != texture) {
      if (lrutexture->boundPixmap) {
         lrutexture->boundPixmap->reclaimTexture();
      } else {
         QGLTextureCache::instance()->remove(lrutexture->boundKey);
      }
      succeeded = true;
   }

   if (texture && !wasInLRU) {
      removeFromLRU(texture);
   }

   return succeeded;
}

void QGLTexturePool::hibernate()
{
   Q_D(QGLTexturePool);
   QGLTexture *texture = d->lruLast;
   while (texture) {
      QGLTexture *prevLRU = texture->prevLRU;
      texture->inTexturePool = false;
      texture->inLRU = false;
      texture->nextLRU = 0;
      texture->prevLRU = 0;
      if (texture->boundPixmap) {
         texture->boundPixmap->hibernate();
      } else {
         QGLTextureCache::instance()->remove(texture->boundKey);
      }
      texture = prevLRU;
   }
   d->lruFirst = 0;
   d->lruLast = 0;
}

void QGLTexturePool::moveToHeadOfLRU(QGLTexture *texture)
{
   Q_D(QGLTexturePool);
   if (texture->inLRU) {
      if (!texture->prevLRU) {
         return;   // Already at the head of the list.
      }
      removeFromLRU(texture);
   }
   texture->inLRU = true;
   texture->nextLRU = d->lruFirst;
   texture->prevLRU = 0;
   if (d->lruFirst) {
      d->lruFirst->prevLRU = texture;
   } else {
      d->lruLast = texture;
   }
   d->lruFirst = texture;
}

void QGLTexturePool::removeFromLRU(QGLTexture *texture)
{
   Q_D(QGLTexturePool);
   if (!texture->inLRU) {
      return;
   }
   if (texture->nextLRU) {
      texture->nextLRU->prevLRU = texture->prevLRU;
   } else {
      d->lruLast = texture->prevLRU;
   }
   if (texture->prevLRU) {
      texture->prevLRU->nextLRU = texture->nextLRU;
   } else {
      d->lruFirst = texture->nextLRU;
   }
   texture->inLRU = false;
}

QGLTexture *QGLTexturePool::textureLRU()
{
   Q_D(QGLTexturePool);
   return d->lruLast;
}

QT_END_NAMESPACE
