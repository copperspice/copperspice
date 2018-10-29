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

#ifndef QPLATFORMGLCONTEXT_QPA_H
#define QPLATFORMGLCONTEXT_QPA_H

#include <QtCore/qnamespace.h>
#include <QtGui/QPlatformWindowFormat>

QT_BEGIN_NAMESPACE

class QPlatformGLContextPrivate;

class Q_OPENGL_EXPORT QPlatformGLContext
{
   Q_DECLARE_PRIVATE(QPlatformGLContext);

 public:
   explicit QPlatformGLContext();
   virtual ~QPlatformGLContext();

   virtual void makeCurrent();
   virtual void doneCurrent();
   virtual void swapBuffers() = 0;
   virtual void *getProcAddress(const QString &procName) = 0;

   virtual QPlatformWindowFormat platformWindowFormat() const = 0;

   const static QPlatformGLContext *currentContext();

 protected:
   QScopedPointer<QPlatformGLContextPrivate> d_ptr;

 private:
   //hack to make it work with QGLContext::CurrentContext
   friend class QGLContext;
   friend class QWidgetPrivate;
   void *qGLContextHandle() const;
   void setQGLContextHandle(void *handle, void (*qGLContextDeleteFunction)(void *));
   void deleteQGLContext();
   Q_DISABLE_COPY(QPlatformGLContext);
};

QT_END_NAMESPACE

#endif // QPLATFORM_GL_INTEGRATION_P_H
