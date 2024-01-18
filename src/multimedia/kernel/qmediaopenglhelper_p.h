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

#ifndef QMEDIAOPENGLHELPER_P_H
#define QMEDIAOPENGLHELPER_P_H

#include <QOpenGLContext>

#if defined(Q_OS_WIN) && (defined(QT_OPENGL_ES_2) || defined(QT_OPENGL_DYNAMIC))
#include <EGL/egl.h>
#endif

class QMediaOpenGLHelper
{
 public:
   static bool isANGLE();
};

inline bool QMediaOpenGLHelper::isANGLE()
{
   bool isANGLE = false;

# if defined(Q_OS_WIN) && (defined(QT_OPENGL_ES_2) || defined(QT_OPENGL_DYNAMIC))
   if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES) {
      // Although unlikely, technically LibGLES could mean a non-ANGLE EGL/GLES2 implementation too.
      // Verify that it is indeed ANGLE.

#  ifdef QT_OPENGL_ES_2_ANGLE_STATIC
      // ANGLE linked-in statically.
      isANGLE = true;

#  else
      // configured with either -opengl es2 or -opengl desktop.
      HMODULE eglHandle = LoadLibraryW(L"libEGL.dll");

      if (eglHandle) {
         typedef EGLDisplay (EGLAPIENTRYP EglGetDisplay)(EGLNativeDisplayType display_id);
         typedef EGLBoolean (EGLAPIENTRYP EglInitialize)(EGLDisplay dpy, EGLint * major, EGLint * minor);
         typedef const char *(EGLAPIENTRYP EglQueryString)(EGLDisplay dpy, EGLint name);

         EglGetDisplay eglGetDisplay = (EglGetDisplay) GetProcAddress(eglHandle, "eglGetDisplay");
         EglInitialize eglInitialize = (EglInitialize) GetProcAddress(eglHandle, "eglInitialize");
         EglQueryString eglQueryString = (EglQueryString) GetProcAddress(eglHandle, "eglQueryString");

         if (eglGetDisplay && eglInitialize && eglQueryString) {
            // EGL may not be initialized at this stage.
            EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            eglInitialize(dpy, 0, 0);
            const char *vendorStr = eglQueryString(dpy, EGL_VERSION);
            isANGLE = vendorStr && strstr(vendorStr, "ANGLE");
         }
      }
#  endif // QT_OPENGL_ES_2_ANGLE_STATIC

   }

# endif // Q_OS_WIN && (QT_OPENGL_ES_2 || QT_OPENGL_DYNAMIC)

   return isANGLE;
}

#endif
