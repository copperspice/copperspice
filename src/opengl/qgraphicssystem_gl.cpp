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

#include "qgraphicssystem_gl_p.h"
#include <QGraphicsView>
#include "qpixmap_raster_p.h"
#include "qpixmapdata_gl_p.h"
#include "qwindowsurface_gl_p.h"
#include "qgl_p.h"
#include <qwindowsurface_raster_p.h>

#if defined(Q_WS_X11) && !defined(QT_NO_EGL)
#include "qpixmapdata_x11gl_p.h"
#include "qwindowsurface_x11gl_p.h"
#endif

QT_BEGIN_NAMESPACE

extern QGLWidget *qt_gl_getShareWidget();

QPixmapData *QGLGraphicsSystem::createPixmapData(QPixmapData::PixelType type) const
{
   return new QGLPixmapData(type);
}

QWindowSurface *QGLGraphicsSystem::createWindowSurface(QWidget *widget) const
{
#ifdef Q_OS_WIN
   // On Windows the QGLWindowSurface class can't handle
   // drop shadows and native effects, e.g. fading a menu in/out using
   // top level window opacity.
   if (widget->windowType() == Qt::Popup) {
      return new QRasterWindowSurface(widget);
   }
#endif

#if defined(Q_WS_X11) && !defined(QT_NO_EGL)
   if (m_useX11GL && QX11GLPixmapData::hasX11GLPixmaps()) {
      // If the widget is a QGraphicsView which will be re-drawing the entire
      // scene each frame anyway, we should use QGLWindowSurface as this may
      // provide proper buffer flipping, which should be faster than QX11GL's
      // blitting approach:
      QGraphicsView *qgv = qobject_cast<QGraphicsView *>(widget);
      if (qgv && qgv->viewportUpdateMode() == QGraphicsView::FullViewportUpdate) {
         return new QGLWindowSurface(widget);
      } else {
         return new QX11GLWindowSurface(widget);
      }
   }
#endif

   return new QGLWindowSurface(widget);
}

QT_END_NAMESPACE

