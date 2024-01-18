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

#include <qplatform_offscreensurface.h>
#include <qoffscreensurface.h>
#include <qscreen.h>

class QPlatformOffscreenSurfacePrivate
{
 public:
};

QPlatformOffscreenSurface::QPlatformOffscreenSurface(QOffscreenSurface *offscreenSurface)
   : QPlatformSurface(offscreenSurface)
   , d_ptr(new QPlatformOffscreenSurfacePrivate)
{
}

QPlatformOffscreenSurface::~QPlatformOffscreenSurface()
{
}

QOffscreenSurface *QPlatformOffscreenSurface::offscreenSurface() const
{
   return static_cast<QOffscreenSurface *>(m_surface);
}

/*!
    Returns the platform screen handle corresponding to this QPlatformOffscreenSurface.
*/
QPlatformScreen *QPlatformOffscreenSurface::screen() const
{
   return offscreenSurface()->screen()->handle();
}

/*!
    Returns the actual surface format of the offscreen surface.
*/
QSurfaceFormat QPlatformOffscreenSurface::format() const
{
   return QSurfaceFormat();
}

/*!
    Returns \c true if the platform offscreen surface has been allocated.
*/
bool QPlatformOffscreenSurface::isValid() const
{
   return false;
}

