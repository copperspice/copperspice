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

#ifndef QEGLCONTEXT_P_H
#define QEGLCONTEXT_P_H

#include <QtCore/qsize.h>
#include <QtGui/qimage.h>
#include <qegl_p.h>
#include <qeglproperties_p.h>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QEglContext
{
 public:
   QEglContext();
   ~QEglContext();

   bool isValid() const;
   bool isCurrent() const;
   bool isSharing() const {
      return sharing;
   }

   QEgl::API api() const {
      return apiType;
   }
   void setApi(QEgl::API api) {
      apiType = api;
   }

   bool chooseConfig(const QEglProperties &properties, QEgl::PixelFormatMatch match = QEgl::ExactPixelFormat);
   bool createContext(QEglContext *shareContext = 0, const QEglProperties *properties = 0);
   void destroyContext();
   EGLSurface createSurface(QPaintDevice *device, const QEglProperties *properties = 0);
   void destroySurface(EGLSurface surface);

   bool makeCurrent(EGLSurface surface);
   bool doneCurrent();
   bool lazyDoneCurrent();
   bool swapBuffers(EGLSurface surface);
   bool swapBuffersRegion2NOK(EGLSurface surface, const QRegion *region);

   int  configAttrib(int name) const;

   EGLContext context() const {
      return ctx;
   }
   void setContext(EGLContext context) {
      ctx = context;
      ownsContext = false;
   }

   EGLDisplay display() {
      return QEgl::display();
   }

   EGLConfig config() const {
      return cfg;
   }
   void setConfig(EGLConfig config) {
      cfg = config;
   }

 private:
   QEgl::API apiType;
   EGLContext ctx;
   EGLConfig cfg;
   EGLSurface currentSurface;
   bool current;
   bool ownsContext;
   bool sharing;
   bool apiChanged;

   static QEglContext *currentContext(QEgl::API api);
   static void setCurrentContext(QEgl::API api, QEglContext *context);

   friend class QMeeGoGraphicsSystem;
   friend class QMeeGoPixmapData;
};

QT_END_NAMESPACE

#endif // QEGLCONTEXT_P_H
