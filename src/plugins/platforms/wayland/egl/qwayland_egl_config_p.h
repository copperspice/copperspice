/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QWAYLAND_EGL_CONFIG_H
#define QWAYLAND_EGL_CONFIG_H

#include <qsizef.h>
#include <qsurfaceformat.h>
#include <qvector.h>

#include <EGL/egl.h>

bool q_reduceConfigAttributes(QVector<EGLint> *configAttributes);
bool q_hasEglExtension(EGLDisplay display, const char *extensionName);
void q_printEglConfig(EGLDisplay display, EGLConfig config);

QVector<EGLint> q_createConfigAttributesFromFormat(const QSurfaceFormat &format);

EGLConfig q_configFromGLFormat(EGLDisplay display, const QSurfaceFormat &format,
      bool highestPixelFormat = false, int surfaceType = EGL_WINDOW_BIT);

QSurfaceFormat q_glFormatFromConfig(EGLDisplay display, const EGLConfig config,
      const QSurfaceFormat &referenceFormat = QSurfaceFormat());

QSizeF q_physicalScreenSizeFromFb(int framebufferDevice, const QSize &screenSize = QSize());
QSize  q_screenSizeFromFb(int framebufferDevice);
int    q_screenDepthFromFb(int framebufferDevice);
qreal  q_refreshRateFromFb(int framebufferDevice);

class QEglConfigChooser
{
 public:
   QEglConfigChooser(EGLDisplay display);
   virtual ~QEglConfigChooser();

   EGLConfig chooseConfig();

   EGLDisplay display() const {
      return m_display;
   }

   bool ignoreColorChannels() const {
      return m_ignore;
   }

   void setIgnoreColorChannels(bool ignore) {
      m_ignore = ignore;
   }

   void setSurfaceFormat(const QSurfaceFormat &format) {
      m_format = format;
   }

   void setSurfaceType(EGLint surfaceType) {
      m_surfaceType = surfaceType;
   }

   QSurfaceFormat surfaceFormat() const {
      return m_format;
   }

   EGLint surfaceType() const {
      return m_surfaceType;
   }

 protected:
   virtual bool filterConfig(EGLConfig config) const;

   bool m_ignore;

   int m_confAttrRed;
   int m_confAttrGreen;
   int m_confAttrBlue;
   int m_confAttrAlpha;

   EGLDisplay m_display;
   EGLint m_surfaceType;
   QSurfaceFormat m_format;
};

#endif
