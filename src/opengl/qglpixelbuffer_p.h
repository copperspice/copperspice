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

#ifndef QGLPIXELBUFFER_P_H
#define QGLPIXELBUFFER_P_H

QT_BEGIN_NAMESPACE

QT_BEGIN_INCLUDE_NAMESPACE
#include "QtOpenGL/qglpixelbuffer.h"
#include <qgl_p.h>
#include <qglpaintdevice_p.h>
#include <qglobal.h>

#if defined(Q_WS_X11) && defined(QT_NO_EGL)
#include <GL/glx.h>

// The below is needed to for compilation on HPUX, due to broken GLX
// headers. Some of the systems define GLX_VERSION_1_3 without
// defining the GLXFBConfig structure, which is wrong.
#if defined (Q_OS_HPUX) && defined(QT_DEFINE_GLXFBCONFIG_STRUCT)
typedef unsigned long GLXPbuffer;

struct GLXFBConfig {
   int visualType;
   int transparentType;
   /*    colors are floats scaled to ints */
   int transparentRed, transparentGreen, transparentBlue, transparentAlpha;
   int transparentIndex;

   int visualCaveat;

   int associatedVisualId;
   int screen;

   int drawableType;
   int renderType;

   int maxPbufferWidth, maxPbufferHeight, maxPbufferPixels;
   int optimalPbufferWidth, optimalPbufferHeight;  /* for SGIX_pbuffer */

   int visualSelectGroup;	/* visuals grouped by select priority */

   unsigned int id;

   GLboolean rgbMode;
   GLboolean colorIndexMode;
   GLboolean doubleBufferMode;
   GLboolean stereoMode;
   GLboolean haveAccumBuffer;
   GLboolean haveDepthBuffer;
   GLboolean haveStencilBuffer;

   /* The number of bits present in various buffers */
   GLint accumRedBits, accumGreenBits, accumBlueBits, accumAlphaBits;
   GLint depthBits;
   GLint stencilBits;
   GLint indexBits;
   GLint redBits, greenBits, blueBits, alphaBits;
   GLuint redMask, greenMask, blueMask, alphaMask;

   GLuint multiSampleSize;     /* Number of samples per pixel (0 if no ms) */

   GLuint nMultiSampleBuffers; /* Number of available ms buffers */
   GLint maxAuxBuffers;

   /* frame buffer level */
   GLint level;

   /* color ranges (for SGI_color_range) */
   GLboolean extendedRange;
   GLdouble minRed, maxRed;
   GLdouble minGreen, maxGreen;
   GLdouble minBlue, maxBlue;
   GLdouble minAlpha, maxAlpha;
};

#endif

#elif defined(Q_OS_WIN)
DECLARE_HANDLE(HPBUFFERARB);
#elif !defined(QT_NO_EGL)
#include <qegl_p.h>
#endif

QT_END_INCLUDE_NAMESPACE

class QEglContext;

class QGLPBufferGLPaintDevice : public QGLPaintDevice
{
 public:
   QPaintEngine *paintEngine() const override {
      return pbuf->paintEngine();
   }

   QSize size() const override {
      return pbuf->size();
   }

   QGLContext *context() const override;
   void endPaint() override;
   void setPBuffer(QGLPixelBuffer *pb);

 private:
   QGLPixelBuffer *pbuf;
};

class QGLPixelBufferPrivate
{
   Q_DECLARE_PUBLIC(QGLPixelBuffer)

 public:
   QGLPixelBufferPrivate(QGLPixelBuffer *q) : q_ptr(q), invalid(true), qctx(0), pbuf(0), ctx(0) {

#ifdef Q_OS_WIN
      dc = 0;
#elif defined(Q_OS_MAC)
      share_ctx = 0;
#endif
   }

   bool init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget);
   void common_init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget);
   bool cleanup();

   QGLPixelBuffer *q_ptr;
   bool invalid;
   QGLContext *qctx;
   QGLPBufferGLPaintDevice glDevice;
   QGLFormat format;

   QGLFormat req_format;
   QPointer<QGLWidget> req_shareWidget;
   QSize req_size;

#if defined(Q_WS_X11) && defined(QT_NO_EGL)
   GLXPbuffer pbuf;
   GLXContext ctx;
#elif defined(Q_OS_WIN)
   HDC dc;
   bool has_render_texture : 1;

#if !defined(QT_OPENGL_ES)
   HPBUFFERARB pbuf;
   HGLRC ctx;
#endif

#elif defined(Q_OS_MAC)
   void *pbuf;
   void *ctx;
   void *share_ctx;

#endif

#ifndef QT_NO_EGL
   EGLSurface pbuf;
   QEglContext *ctx;
   int textureFormat;

#elif defined(Q_WS_QPA)
   //stubs
   void *pbuf;
   void *ctx;
#endif
};

QT_END_NAMESPACE

#endif // QGLPIXELBUFFER_P_H
