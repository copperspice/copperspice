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

#ifndef AVFVIDEOFRAMERENDERER_H
#define AVFVIDEOFRAMERENDERER_H

#include <QObject>
#include <QImage>
#include <QOpenGLContext>
#include <QSize>

@class AVPlayerLayer;
@class AVPlayerItemVideoOutput;

class QOpenGLContext;
class QOpenGLFramebufferObject;
class QOpenGLShaderProgram;
class QOffscreenSurface;
class QAbstractVideoSurface;

typedef struct __CVBuffer *CVBufferRef;
typedef CVBufferRef CVImageBufferRef;
typedef CVImageBufferRef CVPixelBufferRef;

#if defined(Q_OS_IOS)

typedef struct __CVOpenGLESTextureCache *CVOpenGLESTextureCacheRef;
typedef CVImageBufferRef CVOpenGLESTextureRef;
// helpers to avoid boring if def
typedef CVOpenGLESTextureCacheRef CVOGLTextureCacheRef;
typedef CVOpenGLESTextureRef CVOGLTextureRef;
#define CVOGLTextureGetTarget CVOpenGLESTextureGetTarget
#define CVOGLTextureGetName CVOpenGLESTextureGetName
#define CVOGLTextureCacheCreate CVOpenGLESTextureCacheCreate
#define CVOGLTextureCacheCreateTextureFromImage CVOpenGLESTextureCacheCreateTextureFromImage
#define CVOGLTextureCacheFlush CVOpenGLESTextureCacheFlush

#else

typedef struct __CVOpenGLTextureCache *CVOpenGLTextureCacheRef;
typedef CVImageBufferRef CVOpenGLTextureRef;
// helpers to avoid boring if def
typedef CVOpenGLTextureCacheRef CVOGLTextureCacheRef;
typedef CVOpenGLTextureRef CVOGLTextureRef;
#define CVOGLTextureGetTarget CVOpenGLTextureGetTarget
#define CVOGLTextureGetName CVOpenGLTextureGetName
#define CVOGLTextureCacheCreate CVOpenGLTextureCacheCreate
#define CVOGLTextureCacheCreateTextureFromImage CVOpenGLTextureCacheCreateTextureFromImage
#define CVOGLTextureCacheFlush CVOpenGLTextureCacheFlush
#endif

class AVFVideoFrameRenderer : public QObject
{
 public:
   AVFVideoFrameRenderer(QAbstractVideoSurface *surface, QObject *parent = nullptr);

   virtual ~AVFVideoFrameRenderer();

   void setPlayerLayer(AVPlayerLayer *layer);

   CVOGLTextureRef renderLayerToTexture(AVPlayerLayer *layer);
   QImage renderLayerToImage(AVPlayerLayer *layer);

 private:
   void initRenderer();
   CVPixelBufferRef copyPixelBufferFromLayer(AVPlayerLayer *layer, size_t &width, size_t &height);
   CVOGLTextureRef createCacheTextureFromLayer(AVPlayerLayer *layer, size_t &width, size_t &height);

   QOpenGLContext *m_glContext;
   QOffscreenSurface *m_offscreenSurface;
   QAbstractVideoSurface *m_surface;
   CVOGLTextureCacheRef m_textureCache;
   AVPlayerItemVideoOutput *m_videoOutput;
   bool m_isContextShared;
};

#endif
