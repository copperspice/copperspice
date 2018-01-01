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

#ifndef QGLWINDOWSURFACE_QWS_P_H
#define QGLWINDOWSURFACE_QWS_P_H

#include <QtCore/qglobal.h>
#include <QPaintDevice>
#include "qwindowsurface_qws_p.h"

QT_BEGIN_NAMESPACE

class QPaintDevice;
class QPoint;
class QRegion;
class QSize;
class QWidget;
class QGLContext;

class QWSGLWindowSurfacePrivate;

class Q_OPENGL_EXPORT QWSGLWindowSurface : public QWSWindowSurface
{
   Q_DECLARE_PRIVATE(QWSGLWindowSurface)

 public:
   QWSGLWindowSurface(QWidget *widget);
   QWSGLWindowSurface();
   ~QWSGLWindowSurface();

   QGLContext *context() const;
   void setContext(QGLContext *context);

 private:
   QWSGLWindowSurfacePrivate *d_ptr;
};


QT_END_NAMESPACE

#endif // QGLWINDOWSURFACE_QWS_P_H
