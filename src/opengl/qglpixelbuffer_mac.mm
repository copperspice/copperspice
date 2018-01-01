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

#include "qglpixelbuffer.h"
#include "qglpixelbuffer_p.h"

#include <qimage.h>
#include <qgl_p.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

#ifndef GL_TEXTURE_RECTANGLE_EXT
#define GL_TEXTURE_RECTANGLE_EXT 0x84F5
#endif

static int nearest_gl_texture_size(int v)
{
   int n = 0, last = 0;
   for (int s = 0; s < 32; ++s) {
      if (((v >> s) & 1) == 1) {
         ++n;
         last = s;
      }
   }
   if (n > 1) {
      return 1 << (last + 1);
   }
   return 1 << last;
}

bool QGLPixelBufferPrivate::init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
{
   Q_Q(QGLPixelBuffer);

   // create a dummy context
   QGLContext context(f, q);
   context.create(shareWidget ? shareWidget->context() : 0);

   if (context.isSharing()) {
      share_ctx = shareWidget->context()->d_func()->cx;
   }

   // steal the NSOpenGLContext and update the format
   ctx = context.d_func()->cx;
   context.d_func()->cx = 0;
   // d->cx will be set to ctx later in
   // QGLPixelBufferPrivate::common_init, so we need to retain it:
   [static_cast<NSOpenGLContext *>(ctx) retain];

   format = context.format();

   GLenum target = GL_TEXTURE_2D;

   if ((QGLExtensions::glExtensions() & QGLExtensions::TextureRectangle)
         && (size.width() != nearest_gl_texture_size(size.width())
             || size.height() != nearest_gl_texture_size(size.height()))) {
      target = GL_TEXTURE_RECTANGLE_EXT;
   }

   pbuf = [[NSOpenGLPixelBuffer alloc] initWithTextureTarget: target
           textureInternalFormat: GL_RGBA
           textureMaxMipMapLevel: 0
           pixelsWide: size.width()
           pixelsHigh: size.height()];
   if (!pbuf) {
      qWarning("QGLPixelBuffer: Cannot create a pbuffer");
      return false;
   }

   [static_cast<NSOpenGLContext *>(ctx) setPixelBuffer: static_cast<NSOpenGLPixelBuffer *>(pbuf)
    cubeMapFace: 0
    mipMapLevel: 0
    currentVirtualScreen: 0];
   return true;
}

bool QGLPixelBufferPrivate::cleanup()
{
   [static_cast<NSOpenGLPixelBuffer *>(pbuf) release];
   pbuf = 0;
   [static_cast<NSOpenGLContext *>(ctx) release];
   ctx = 0;

   return true;
}

bool QGLPixelBuffer::bindToDynamicTexture(GLuint texture_id)
{
   Q_D(QGLPixelBuffer);

   if (d->invalid || !d->share_ctx) {
      return false;
   }

   NSOpenGLContext *oldContext = [NSOpenGLContext currentContext];
   if (d->share_ctx != oldContext) {
      [static_cast<NSOpenGLContext *>(d->share_ctx) makeCurrentContext];
   }
   glBindTexture(GL_TEXTURE_2D, texture_id);
   [static_cast<NSOpenGLContext *>(d->share_ctx)
    setTextureImageToPixelBuffer: static_cast<NSOpenGLPixelBuffer *>(d->pbuf)
    colorBuffer: GL_FRONT];
   if (oldContext && oldContext != d->share_ctx) {
      [oldContext makeCurrentContext];
   }
   return true;

}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
bool QGLPixelBuffer::bindToDynamicTexture(QMacCompatGLuint texture_id)
{
   return bindToDynamicTexture(GLuint(texture_id));
}
#endif

void QGLPixelBuffer::releaseFromDynamicTexture()
{
}

GLuint QGLPixelBuffer::generateDynamicTexture() const
{
   Q_D(const QGLPixelBuffer);

   NSOpenGLContext *oldContext = [NSOpenGLContext currentContext];
   if (d->share_ctx != oldContext) {
      [static_cast<NSOpenGLContext *>(d->share_ctx) makeCurrentContext];
   }
   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   if (oldContext && oldContext != d->share_ctx) {
      [oldContext makeCurrentContext];
   }

   return texture;
}

bool QGLPixelBuffer::hasOpenGLPbuffers()
{
   return true;
}

QT_END_NAMESPACE
