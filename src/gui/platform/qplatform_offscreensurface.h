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

#ifndef QPLATFORM_OFFSCREENSURFACE_H
#define QPLATFORM_OFFSCREENSURFACE_H

#include <qplatform_surface.h>
#include <qscopedpointer.h>

class QOffscreenSurface;
class QPlatformScreen;

class QPlatformOffscreenSurfacePrivate;

class Q_GUI_EXPORT QPlatformOffscreenSurface : public QPlatformSurface
{
 public:
   explicit QPlatformOffscreenSurface(QOffscreenSurface *offscreenSurface);

   QPlatformOffscreenSurface(const QPlatformOffscreenSurface &) = delete;
   QPlatformOffscreenSurface &operator=(const QPlatformOffscreenSurface &) = delete;

   virtual ~QPlatformOffscreenSurface();

   QOffscreenSurface *offscreenSurface() const;

   QPlatformScreen *screen() const;

   QSurfaceFormat format() const override;
   virtual bool isValid() const;

 protected:
   QScopedPointer<QPlatformOffscreenSurfacePrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QPlatformOffscreenSurface)
};

#endif
