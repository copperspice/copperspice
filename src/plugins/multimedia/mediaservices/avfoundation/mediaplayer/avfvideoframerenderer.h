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

@class CARenderer;
@class AVPlayerLayer;

class QOpenGLFramebufferObject;
class QWindow;
class QOpenGLContext;
class QAbstractVideoSurface;

class AVFVideoFrameRenderer : public QObject
{
 public:
   AVFVideoFrameRenderer(QAbstractVideoSurface *surface, QObject *parent = nullptr);

   virtual ~AVFVideoFrameRenderer();

   GLuint renderLayerToTexture(AVPlayerLayer *layer);
   QImage renderLayerToImage(AVPlayerLayer *layer);

 private:
   QOpenGLFramebufferObject *initRenderer(AVPlayerLayer *layer);
   void renderLayerToFBO(AVPlayerLayer *layer, QOpenGLFramebufferObject *fbo);

   CARenderer *m_videoLayerRenderer;
   QAbstractVideoSurface *m_surface;
   QOpenGLFramebufferObject *m_fbo[2];
   QWindow *m_offscreenSurface;
   QOpenGLContext *m_glContext;
   QSize m_targetSize;

   uint m_currentBuffer;
   bool m_isContextShared;
};

#endif
