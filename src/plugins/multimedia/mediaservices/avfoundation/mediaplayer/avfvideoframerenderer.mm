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

#include <avfvideoframerenderer.h>

#include <qabstractvideosurface.h>
#include <qopengl_framebufferobject.h>
#include <qwindow.h>

#ifdef QT_DEBUG_AVF
#include <qdebug.h>
#endif

#import <CoreVideo/CVBase.h>
#import <AVFoundation/AVFoundation.h>

AVFVideoFrameRenderer::AVFVideoFrameRenderer(QAbstractVideoSurface *surface, QObject *parent)
   : QObject(parent), m_videoLayerRenderer(nullptr), m_surface(surface), m_offscreenSurface(nullptr),
     m_glContext(nullptr), m_currentBuffer(1), m_isContextShared(true)
{
   m_fbo[0] = nullptr;
   m_fbo[1] = nullptr;
}

AVFVideoFrameRenderer::~AVFVideoFrameRenderer()
{
#ifdef QT_DEBUG_AVF
   qDebug() << Q_FUNC_INFO;
#endif

   [m_videoLayerRenderer release];
   delete m_fbo[0];
   delete m_fbo[1];
   delete m_offscreenSurface;
   delete m_glContext;
}

GLuint AVFVideoFrameRenderer::renderLayerToTexture(AVPlayerLayer *layer)
{
   //Is layer valid
   if (!layer) {
      return 0;
   }

   //If the glContext isn't shared, it doesn't make sense to return a texture for us
   if (m_offscreenSurface && !m_isContextShared) {
      return 0;
   }

   QOpenGLFramebufferObject *fbo = initRenderer(layer);

   if (!fbo) {
      return 0;
   }

   renderLayerToFBO(layer, fbo);
   if (m_glContext) {
      m_glContext->doneCurrent();
   }

   return fbo->texture();
}

QImage AVFVideoFrameRenderer::renderLayerToImage(AVPlayerLayer *layer)
{
   //Is layer valid
   if (!layer) {
      return QImage();
   }

   QOpenGLFramebufferObject *fbo = initRenderer(layer);

   if (!fbo) {
      return QImage();
   }

   renderLayerToFBO(layer, fbo);
   QImage fboImage = fbo->toImage();
   if (m_glContext) {
      m_glContext->doneCurrent();
   }

   return fboImage;
}

QOpenGLFramebufferObject *AVFVideoFrameRenderer::initRenderer(AVPlayerLayer *layer)
{

   //Get size from AVPlayerLayer
   m_targetSize = QSize(layer.bounds.size.width, layer.bounds.size.height);

   //Make sure we have an OpenGL context to make current
   if (!QOpenGLContext::currentContext() && !m_glContext) {
      //Create Hidden QWindow surface to create context in this thread
      m_offscreenSurface = new QWindow();
      m_offscreenSurface->setSurfaceType(QWindow::OpenGLSurface);
      //Needs geometry to be a valid surface, but size is not important
      m_offscreenSurface->setGeometry(0, 0, 1, 1);
      m_offscreenSurface->create();

      //Create OpenGL context and set share context from surface
      QOpenGLContext *shareContext = nullptr;

      if (m_surface) {
         //QOpenGLContext *renderThreadContext = 0;
         shareContext = qobject_cast<QOpenGLContext *>(m_surface->property("GLContext").value<QObject *>());
      }
      m_glContext = new QOpenGLContext();
      m_glContext->setFormat(m_offscreenSurface->requestedFormat());

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
         qWarning("failed to create QOpenGLContext");
         return nullptr;
      }
   }

   //Need current context
   if (m_glContext) {
      m_glContext->makeCurrent(m_offscreenSurface);
   }

   //Create the CARenderer if needed
   if (!m_videoLayerRenderer) {
      m_videoLayerRenderer = [CARenderer rendererWithCGLContext: CGLGetCurrentContext() options: nil];
      [m_videoLayerRenderer retain];
   }

   //Set/Change render source if needed
   if (m_videoLayerRenderer.layer != layer) {
      m_videoLayerRenderer.layer = layer;
      m_videoLayerRenderer.bounds = layer.bounds;
   }

   //Do we have FBO's already?
   if ((!m_fbo[0] && !m_fbo[0]) || (m_fbo[0]->size() != m_targetSize)) {
      delete m_fbo[0];
      delete m_fbo[1];
      m_fbo[0] = new QOpenGLFramebufferObject(m_targetSize);
      m_fbo[1] = new QOpenGLFramebufferObject(m_targetSize);
   }

   //Switch buffer target
   m_currentBuffer = !m_currentBuffer;
   return m_fbo[m_currentBuffer];
}

void AVFVideoFrameRenderer::renderLayerToFBO(AVPlayerLayer *layer, QOpenGLFramebufferObject *fbo)
{
   //Start Rendering
   //NOTE: This rendering method will NOT work on iOS as there is no CARenderer in iOS
   if (!fbo->bind()) {
      qWarning("AVFVideoRender FBO failed to bind");
      return;
   }

   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);

   glViewport(0, 0, m_targetSize.width(), m_targetSize.height());

   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();

   //Render to FBO with inverted Y
   glOrtho(0.0, m_targetSize.width(), 0.0, m_targetSize.height(), 0.0, 1.0);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   [m_videoLayerRenderer beginFrameAtTime: CACurrentMediaTime() timeStamp: nullptr];
   [m_videoLayerRenderer addUpdateRect: layer.bounds];
   [m_videoLayerRenderer render];
   [m_videoLayerRenderer endFrame];

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();

   glFinish(); //Rendering needs to be done before passing texture to video frame

   fbo->release();
}
