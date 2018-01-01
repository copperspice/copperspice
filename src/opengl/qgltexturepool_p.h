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

#ifndef QGLTEXTUREPOOL_P_H
#define QGLTEXTUREPOOL_P_H

#include "qgl.h"
#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

class QGLTexture;
class QGLTexturePoolPrivate;

class QGLTexturePool
{
 public:
   QGLTexturePool();
   virtual ~QGLTexturePool();

   static QGLTexturePool *instance();

   // Create a new texture with the specified parameters and associate
   // it with "texture".  The QGLTexture will be notified when the
   // texture needs to be reclaimed by the pool.
   //
   // This function will call reclaimSpace() when texture creation fails.
   GLuint createTexture(GLenum target,
                        GLint level,
                        GLint internalformat,
                        GLsizei width,
                        GLsizei height,
                        GLenum format,
                        GLenum type,
                        QGLTexture *texture);

   // Create a permanent texture with the specified parameters.
   // If there is insufficient space for the texture,
   // then this function will call reclaimSpace() and try again.
   //
   // The caller is responsible for calling glDeleteTextures()
   // when it no longer needs the texture, as the texture is not
   // recorded in the texture pool.
   bool createPermanentTexture(GLuint texture,
                               GLenum target,
                               GLint level,
                               GLint internalformat,
                               GLsizei width,
                               GLsizei height,
                               GLenum format,
                               GLenum type,
                               const GLvoid *data);

   // Notify the pool that a QGLTexture object is using
   // an texture again.  This allows the pool to move the texture
   // within a least-recently-used list of QGLTexture objects.
   void useTexture(QGLTexture *texture);

   // Notify the pool that the texture associated with a
   // QGLTexture is being detached from the pool.  The caller
   // will become responsible for calling glDeleteTextures().
   void detachTexture(QGLTexture *texture);

   // Reclaim space for an image allocation with the specified parameters.
   // Returns true if space was reclaimed, or false if there is no
   // further space that can be reclaimed.  The "texture" parameter
   // indicates the texture that is trying to obtain space which should
   // not itself be reclaimed.
   bool reclaimSpace(GLint internalformat,
                     GLsizei width,
                     GLsizei height,
                     GLenum format,
                     GLenum type,
                     QGLTexture *data);

   // Hibernate the texture pool because the context is about to be
   // destroyed.  All textures left in the pool should be released.
   void hibernate();

 protected:
   // Helper functions for managing the LRU list of QGLTexture objects.
   void moveToHeadOfLRU(QGLTexture *texture);
   void removeFromLRU(QGLTexture *texture);
   QGLTexture *textureLRU();

 private:
   QScopedPointer<QGLTexturePoolPrivate> d_ptr;

   Q_DECLARE_PRIVATE(QGLTexturePool)
   Q_DISABLE_COPY(QGLTexturePool)
};

QT_END_NAMESPACE

#endif
