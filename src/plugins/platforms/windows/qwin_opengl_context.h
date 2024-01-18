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

#ifndef QWINDOWSOPENGLCONTEXT_H
#define QWINDOWSOPENGLCONTEXT_H

#include <qopenglcontext.h>
#include <qplatform_openglcontext.h>

#ifndef QT_NO_OPENGL

class QWindowsOpenGLContext;

class QWindowsStaticOpenGLContext
{
 public:
   static QWindowsStaticOpenGLContext *create();
   virtual ~QWindowsStaticOpenGLContext() { }

   virtual QWindowsOpenGLContext *createContext(QOpenGLContext *context) = 0;
   virtual void *moduleHandle() const = 0;
   virtual QOpenGLContext::OpenGLModuleType moduleType() const = 0;
   virtual bool supportsThreadedOpenGL() const {
      return false;
   }

   // If the windowing system interface needs explicitly created window surfaces (like EGL),
   // reimplement these.
   virtual void *createWindowSurface(void * /*nativeWindow*/, void * /*nativeConfig*/, int * /*err*/) {
      return nullptr;
   }

   virtual void destroyWindowSurface(void * /*nativeSurface*/) {
   }

 private:
   static QWindowsStaticOpenGLContext *doCreate();
};

class QWindowsOpenGLContext : public QPlatformOpenGLContext
{
 public:
   virtual ~QWindowsOpenGLContext() { }

   // Returns the native context handle (e.g. HGLRC for WGL, EGLContext for EGL).
   virtual void *nativeContext() const = 0;

   // These should be implemented only for some winsys interfaces, for example EGL.
   // For others, like WGL, they are not relevant.
   virtual void *nativeDisplay() const {
      return nullptr;
   }

   virtual void *nativeConfig() const {
      return nullptr;
   }
};

#endif // QT_NO_OPENGL

#endif
