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

#include <QtGui/QPaintDevice>
#include <QtGui/QWidget>
#include <QtOpenGL/QGLWidget>
#include "qglwindowsurface_qws_p.h"
#include "qpaintengine_opengl_p.h"

QT_BEGIN_NAMESPACE

class QWSGLWindowSurfacePrivate
{
 public:
   QWSGLWindowSurfacePrivate() :
      qglContext(0), ownsContext(false) {}

   QGLContext *qglContext;
   bool ownsContext;
};

/*!
    Constructs an empty QWSGLWindowSurface for the given top-level \a window.
    The window surface is later initialized from chooseContext() and resources for it
    is typically allocated in setGeometry().
*/
QWSGLWindowSurface::QWSGLWindowSurface(QWidget *window)
   : QWSWindowSurface(window),
     d_ptr(new QWSGLWindowSurfacePrivate)
{
}

/*!
    Constructs an empty QWSGLWindowSurface.
*/
QWSGLWindowSurface::QWSGLWindowSurface()
   : d_ptr(new QWSGLWindowSurfacePrivate)
{
}

/*!
    Destroys the QWSGLWindowSurface object and frees any
    allocated resources.
 */
QWSGLWindowSurface::~QWSGLWindowSurface()
{
   Q_D(QWSGLWindowSurface);
   if (d->ownsContext) {
      delete d->qglContext;
   }
   delete d;
}

/*!
    Returns the QGLContext of the window surface.
*/
QGLContext *QWSGLWindowSurface::context() const
{
   Q_D(const QWSGLWindowSurface);
   if (!d->qglContext) {
      QWSGLWindowSurface *that = const_cast<QWSGLWindowSurface *>(this);
      that->setContext(new QGLContext(QGLFormat::defaultFormat()));
      that->d_func()->ownsContext = true;
   }
   return d->qglContext;
}

/*!
    Sets the QGLContext for this window surface to \a context.
*/
void QWSGLWindowSurface::setContext(QGLContext *context)
{
   Q_D(QWSGLWindowSurface);
   if (d->ownsContext) {
      delete d->qglContext;
      d->ownsContext = false;
   }
   d->qglContext = context;
}

QT_END_NAMESPACE
