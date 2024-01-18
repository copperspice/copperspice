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

#ifndef QSURFACE_H
#define QSURFACE_H

#include <qnamespace.h>
#include <qsurfaceformat.h>
#include <qsize.h>

class QPlatformSurface;
class QSurfacePrivate;

class Q_GUI_EXPORT QSurface
{
 public:
   enum SurfaceClass {
      Window,
      Offscreen
   };

   enum SurfaceType {
      RasterSurface,
      OpenGLSurface,
      RasterGLSurface,
      VulkanSurface
   };

   virtual ~QSurface();

   SurfaceClass surfaceClass() const;

   virtual QSurfaceFormat format() const = 0;
   virtual QPlatformSurface *surfaceHandle() const = 0;

   virtual SurfaceType surfaceType() const = 0;
   bool supportsOpenGL() const;

   virtual QSize size() const = 0;

 protected:
   explicit QSurface(SurfaceClass type);

   SurfaceClass m_type;

   QSurfacePrivate *m_reserved;
};

#endif
