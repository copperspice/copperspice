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

#include <cglconvenience_p.h>

#include <qglobal.h>
#include <qcore_mac_p.h>
#include <qvector.h>

#include <Cocoa/Cocoa.h>

void (*qcgl_getProcAddress(const QByteArray &procName))()
{
   CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
         CFSTR("/System/Library/Frameworks/OpenGL.framework"), kCFURLPOSIXPathStyle, false);

   CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, url);
   CFStringRef procNameCF = QCFString::toCFStringRef(QString::fromLatin1(procName.constData()));
   void *proc = CFBundleGetFunctionPointerForName(bundle, procNameCF);
   CFRelease(url);
   CFRelease(bundle);
   CFRelease(procNameCF);
   return (void (*) ())proc;
}

// Match up with createNSOpenGLPixelFormat below!
QSurfaceFormat qcgl_surfaceFormat()
{
   QSurfaceFormat format;
   format.setRenderableType(QSurfaceFormat::OpenGL);
   format.setRedBufferSize(8);
   format.setGreenBufferSize(8);
   format.setBlueBufferSize(8);
   format.setAlphaBufferSize(8);
   /*
       format.setDepthBufferSize(24);
       format.setAccumBufferSize(0);
       format.setStencilBufferSize(8);
       format.setSampleBuffers(false);
       format.setSamples(1);
       format.setDepth(true);
       format.setRgba(true);
       format.setAlpha(true);
       format.setAccum(false);
       format.setStencil(true);
       format.setStereo(false);
       format.setDirectRendering(false);
   */
   return format;
}

void *qcgl_createNSOpenGLPixelFormat(const QSurfaceFormat &format)
{

   QVector<NSOpenGLPixelFormatAttribute> attrs;

   if (format.swapBehavior() == QSurfaceFormat::DoubleBuffer || format.swapBehavior() == QSurfaceFormat::DefaultSwapBehavior) {
      attrs.append(NSOpenGLPFADoubleBuffer);
   }

   else if (format.swapBehavior() == QSurfaceFormat::TripleBuffer) {
      attrs.append(NSOpenGLPFATripleBuffer);
   }

   if (format.profile() == QSurfaceFormat::CoreProfile
      && ((format.majorVersion() == 3 && format.minorVersion() >= 2) || format.majorVersion() > 3)) {

      attrs << NSOpenGLPFAOpenGLProfile;
      attrs << NSOpenGLProfileVersion3_2Core;

   } else {
      attrs << NSOpenGLPFAOpenGLProfile;
      attrs << NSOpenGLProfileVersionLegacy;
   }

   if (format.depthBufferSize() > 0) {
      attrs <<  NSOpenGLPFADepthSize << format.depthBufferSize();
   }

   if (format.stencilBufferSize() > 0) {
      attrs << NSOpenGLPFAStencilSize << format.stencilBufferSize();
   }

   if (format.alphaBufferSize() > 0) {
      attrs << NSOpenGLPFAAlphaSize << format.alphaBufferSize();
   }

   if ((format.redBufferSize() > 0) &&
      (format.greenBufferSize() > 0) &&
      (format.blueBufferSize() > 0)) {

      const int colorSize = format.redBufferSize() + format.greenBufferSize() + format.blueBufferSize();
      attrs << NSOpenGLPFAColorSize << colorSize << NSOpenGLPFAMinimumPolicy;
   }

   if (format.samples() > 0) {
      attrs << NSOpenGLPFAMultisample
         << NSOpenGLPFASampleBuffers << (NSOpenGLPixelFormatAttribute) 1
         << NSOpenGLPFASamples << (NSOpenGLPixelFormatAttribute) format.samples();
   }

   if (format.stereo()) {
      attrs << NSOpenGLPFAStereo;
   }

   attrs << NSOpenGLPFAAllowOfflineRenderers;

   QByteArray useLayer = qgetenv("QT_MAC_WANTS_LAYER");
   if (!useLayer.isEmpty() && useLayer.toInt() > 0) {
      // Disable the software rendering fallback. This makes compositing
      // OpenGL and raster NSViews using Core Animation layers possible.
      attrs << NSOpenGLPFANoRecovery;
   }

   attrs << 0;

   NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes: attrs.constData()];
   return pixelFormat;
}
