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

#include <avfvideoframerenderer_ios.h>

#include <qabstractvideosurface.h>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOffscreenSurface>

#ifdef QT_DEBUG_AVF
#include <qdebug.h>
#endif

#import <CoreVideo/CVBase.h>
#import <AVFoundation/AVFoundation.h>

AVFVideoFrameRenderer::AVFVideoFrameRenderer(QAbstractVideoSurface *surface, QObject *parent)
   : QObject(parent)
   , m_glContext(0)
   , m_offscreenSurface(0)
   , m_surface(surface)
   , m_textureCache(0)
   , m_videoOutput(0)
   , m_isContextShared(true)
{
}

AVFVideoFrameRenderer::~AVFVideoFrameRenderer()
{
#ifdef QT_DEBUG_AVF
   qDebug() << Q_FUNC_INFO;
#endif

   [m_videoOutput release]; // sending to nil is fine
   if (m_textureCache) {
      CFRelease(m_textureCache);
   }
   delete m_offscreenSurface;
   delete m_glContext;
}

void AVFVideoFrameRenderer::setPlayerLayer(AVPlayerLayer *layer)
{
   (void) layer;

   if (m_videoOutput) {
      [m_videoOutput release];
      m_videoOutput = 0;
      // will be re-created in first call to copyPixelBufferFromLayer
   }
}

CVOGLTextureRef AVFVideoFrameRenderer::renderLayerToTexture(AVPlayerLayer *layer)
{
   initRenderer();

   // If the glContext isn't shared, it doesn't make sense to return a texture for us
   if (!m_isContextShared) {
      return 0;
   }

   size_t dummyWidth = 0, dummyHeight = 0;
   return createCacheTextureFromLayer(layer, dummyWidth, dummyHeight);
}

static NSString *const AVF_PIXEL_FORMAT_KEY = (NSString *)kCVPixelBufferPixelFormatTypeKey;
static NSNumber *const AVF_PIXEL_FORMAT_VALUE = [NSNumber numberWithUnsignedInt: kCVPixelFormatType_32BGRA];
static NSDictionary *const AVF_OUTPUT_SETTINGS = [NSDictionary dictionaryWithObject: AVF_PIXEL_FORMAT_VALUE forKey:
                   AVF_PIXEL_FORMAT_KEY];


CVPixelBufferRef AVFVideoFrameRenderer::copyPixelBufferFromLayer(AVPlayerLayer *layer,
   size_t &width, size_t &height)
{
   //Is layer valid
   if (!layer) {
#ifdef QT_DEBUG_AVF
      qWarning("copyPixelBufferFromLayer: invalid layer");
#endif
      return 0;
   }

   if (!m_videoOutput) {
      m_videoOutput = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes: AVF_OUTPUT_SETTINGS];
      [m_videoOutput setDelegate: nil queue: nil];
      AVPlayerItem *item = [[layer player] currentItem];
      [item addOutput: m_videoOutput];
   }

   CFTimeInterval currentCAFrameTime =  CACurrentMediaTime();
   CMTime currentCMFrameTime =  [m_videoOutput itemTimeForHostTime: currentCAFrameTime];
   // happens when buffering / loading
   if (CMTimeCompare(currentCMFrameTime, kCMTimeZero) < 0) {
      return 0;
   }

   CVPixelBufferRef pixelBuffer = [m_videoOutput copyPixelBufferForItemTime: currentCMFrameTime
                                                         itemTimeForDisplay: nil];
   if (!pixelBuffer) {
#ifdef QT_DEBUG_AVF
      qWarning("copyPixelBufferForItemTime returned nil");
      CMTimeShow(currentCMFrameTime);
#endif
      return 0;
   }

   width = CVPixelBufferGetWidth(pixelBuffer);
   height = CVPixelBufferGetHeight(pixelBuffer);
   return pixelBuffer;
}

CVOGLTextureRef AVFVideoFrameRenderer::createCacheTextureFromLayer(AVPlayerLayer *layer,
   size_t &width, size_t &height)
{
   CVPixelBufferRef pixelBuffer = copyPixelBufferFromLayer(layer, width, height);

   if (!pixelBuffer) {
      return 0;
   }

   CVOGLTextureCacheFlush(m_textureCache, 0);

   CVOGLTextureRef texture = 0;
   CVReturn err = CVOGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault, m_textureCache, pixelBuffer, NULL,
         GL_TEXTURE_2D, GL_RGBA,
         (GLsizei) width, (GLsizei) height,
         GL_BGRA, GL_UNSIGNED_BYTE, 0,
         &texture);

   if (!texture || err) {
#ifdef QT_DEBUG_AVF
      qWarning("CVOGLTextureCacheCreateTextureFromImage failed (error: %d)", err);
#endif
   }

   CVPixelBufferRelease(pixelBuffer);

   return texture;
}

QImage AVFVideoFrameRenderer::renderLayerToImage(AVPlayerLayer *layer)
{
   size_t width = 0;
   size_t height = 0;
   CVPixelBufferRef pixelBuffer = copyPixelBufferFromLayer(layer, width, height);

   if (!pixelBuffer) {
      return QImage();
   }

   OSType pixelFormat = CVPixelBufferGetPixelFormatType(pixelBuffer);
   if (pixelFormat != kCVPixelFormatType_32BGRA) {
#ifdef QT_DEBUG_AVF
      qWarning("CVPixelBuffer format is not BGRA32 (got: %d)", static_cast<quint32>(pixelFormat));
#endif
      return QImage();
   }

   CVPixelBufferLockBaseAddress(pixelBuffer, 0);
   char *data = (char *)CVPixelBufferGetBaseAddress(pixelBuffer);
   size_t stride = CVPixelBufferGetBytesPerRow(pixelBuffer);

   // format here is not relevant, only using for storage
   QImage img = QImage(width, height, QImage::Format_ARGB32);
   for (size_t j = 0; j < height; j++) {
      memcpy(img.scanLine(j), data, width * 4);
      data += stride;
   }

   CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
   CVPixelBufferRelease(pixelBuffer);
   return img;
}

void AVFVideoFrameRenderer::initRenderer()
{
   // even for using a texture directly, we need to be able to make a context current,
   // so we need an offscreen, and we shouldn't assume we can make the surface context
   // current on that offscreen, so use our own (sharing with it). Slightly
   // excessive but no performance penalty and makes the QImage path easier to maintain

   //Make sure we have an OpenGL context to make current
   if (!m_glContext) {
      //Create OpenGL context and set share context from surface
      QOpenGLContext *shareContext = 0;
      if (m_surface) {
         shareContext = qobject_cast<QOpenGLContext *>(m_surface->property("GLContext").value<QObject *>());
      }

      m_glContext = new QOpenGLContext();
      if (shareContext) {
         m_glContext->setShareContext(shareContext);
         m_isContextShared = true;
      } else {
#ifdef QT_DEBUG_AVF
         qWarning("failed to get Render Thread context");
#endif
         m_isContextShared = false;
      }
      if (!m_glContext->create()) {
#ifdef QT_DEBUG_AVF
         qWarning("failed to create QOpenGLContext");
#endif
         return;
      }
   }

   if (!m_offscreenSurface) {
      m_offscreenSurface = new QOffscreenSurface();
      m_offscreenSurface->setFormat(m_glContext->format());
      m_offscreenSurface->create();
   }

   //Need current context
   m_glContext->makeCurrent(m_offscreenSurface);

   if (!m_textureCache) {
      //  Create a new open gl texture cache
      CVReturn err = CVOGLTextureCacheCreate(kCFAllocatorDefault, NULL,
            [EAGLContext currentContext],
            NULL, &m_textureCache);
      if (err) {
#ifdef QT_DEBUG_AVF
         qWarning("Error at CVOGLTextureCacheCreate %d", err);
#endif
      }
   }

}
