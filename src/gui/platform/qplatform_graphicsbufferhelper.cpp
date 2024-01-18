/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
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

#include <qplatform_graphicsbuffer.h>

#include <qplatform_graphicsbufferhelper.h>
#include <qdebug.h>
#include <qopengl.h>
#include <QImage>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#ifndef GL_RGB10_A2
#define GL_RGB10_A2                       0x8059
#endif

#ifndef GL_UNSIGNED_INT_2_10_10_10_REV
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#endif

bool QPlatformGraphicsBufferHelper::lockAndBindToTexture(QPlatformGraphicsBuffer *graphicsBuffer,
   bool *swizzle, bool *premultiplied, const QRect &rect)
{
   if (graphicsBuffer->lock(QPlatformGraphicsBuffer::TextureAccess)) {
      if (! graphicsBuffer->bindToTexture(rect)) {
         qWarning("QPlatformGraphicsBufferHelper::lockAndBindToTexture() Failed to bind graphicsbuffer to texture");
         return false;
      }

      if (swizzle) {
         *swizzle = false;
      }

      if (premultiplied) {
         *premultiplied = false;
      }

   } else if (graphicsBuffer->lock(QPlatformGraphicsBuffer::SWReadAccess)) {
      if (! bindSWToTexture(graphicsBuffer, swizzle, premultiplied, rect)) {
         qWarning("QPlatformGraphicsBufferHelper::lockAndBindToTexture() Failed to bind SW graphicsbuffer to texture");
         return false;
      }

   } else {
      qWarning("QPlatformGraphicsBufferHelper::lockAndBindToTexture() Failed to lock");
      return false;
   }

   return true;
}

bool QPlatformGraphicsBufferHelper::bindSWToTexture(const QPlatformGraphicsBuffer *graphicsBuffer,
   bool *swizzleRandB, bool *premultipliedB, const QRect &subRect)
{
#ifndef QT_NO_OPENGL
   QOpenGLContext *ctx = QOpenGLContext::currentContext();
   if (!ctx) {
      return false;
   }

   if (!(graphicsBuffer->isLocked() & QPlatformGraphicsBuffer::SWReadAccess)) {
      return false;
   }

   QSize size = graphicsBuffer->size();

   Q_ASSERT(subRect.isEmpty() || QRect(QPoint(0, 0), size).contains(subRect));

   GLenum internalFormat = GL_RGBA;
   GLuint pixelType = GL_UNSIGNED_BYTE;

   bool needsConversion = false;
   bool swizzle = false;
   bool premultiplied = false;
   QImage::Format imageformat = QImage::toImageFormat(graphicsBuffer->format());
   QImage image(graphicsBuffer->data(), size.width(), size.height(), graphicsBuffer->bytesPerLine(), imageformat);

   if (graphicsBuffer->bytesPerLine() != (size.width() * 4)) {
      needsConversion = true;

   } else {
      switch (imageformat) {
         case QImage::Format_ARGB32_Premultiplied:
            premultiplied = true;
            [[fallthrough]];

         case QImage::Format_RGB32:
         case QImage::Format_ARGB32:
            swizzle = true;
            break;
         case QImage::Format_RGBA8888_Premultiplied:
            premultiplied = true;
            [[fallthrough]];

         case QImage::Format_RGBX8888:
         case QImage::Format_RGBA8888:
            break;
         case QImage::Format_BGR30:
         case QImage::Format_A2BGR30_Premultiplied:
            if (!ctx->isOpenGLES() || ctx->format().majorVersion() >= 3) {
               pixelType = GL_UNSIGNED_INT_2_10_10_10_REV;
               internalFormat = GL_RGB10_A2;
               premultiplied = true;
            } else {
               needsConversion = true;
            }
            break;

         case QImage::Format_RGB30:
         case QImage::Format_A2RGB30_Premultiplied:
            if (!ctx->isOpenGLES() || ctx->format().majorVersion() >= 3) {
               pixelType = GL_UNSIGNED_INT_2_10_10_10_REV;
               internalFormat = GL_RGB10_A2;
               premultiplied = true;
               swizzle = true;
            } else {
               needsConversion = true;
            }
            break;

         default:
            needsConversion = true;
            break;
      }
   }
   if (needsConversion) {
      image = image.convertToFormat(QImage::Format_RGBA8888);
   }

   QOpenGLFunctions *funcs = ctx->functions();

   QRect rect = subRect;
   if (rect.isNull() || rect == QRect(QPoint(0, 0), size)) {
      funcs->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.width(), size.height(), 0, GL_RGBA, pixelType, image.constBits());
   } else {
#ifndef QT_OPENGL_ES_2
      if (!ctx->isOpenGLES()) {
         funcs->glPixelStorei(GL_UNPACK_ROW_LENGTH, image.width());
         funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x(), rect.y(), rect.width(), rect.height(), GL_RGBA, pixelType,
            image.constScanLine(rect.y()) + rect.x() * 4);
         funcs->glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
      } else
#endif
      {
         // if the rect is wide enough it's cheaper to just
         // extend it instead of doing an image copy
         if (rect.width() >= size.width() / 2) {
            rect.setX(0);
            rect.setWidth(size.width());
         }

         // if the sub-rect is full-width we can pass the image data directly to
         // OpenGL instead of copying, since there's no gap between scanlines

         if (rect.width() == size.width()) {
            funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, rect.y(), rect.width(), rect.height(), GL_RGBA, pixelType,
               image.constScanLine(rect.y()));
         } else {
            funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x(), rect.y(), rect.width(), rect.height(), GL_RGBA, pixelType,
               image.copy(rect).constBits());
         }
      }
   }

   if (swizzleRandB) {
      *swizzleRandB = swizzle;
   }

   if (premultipliedB) {
      *premultipliedB = premultiplied;
   }

   return true;

#else
   (void) graphicsBuffer;
   (void) swizzleRandB;
   (void) premultipliedB;
   (void) subRect;

   return false;

#endif // QT_NO_OPENGL
}

