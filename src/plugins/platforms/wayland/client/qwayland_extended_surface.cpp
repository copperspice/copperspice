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

#include <qwayland_extended_surface_p.h>

#include <qapplication.h>
#include <qplatform_nativeinterface.h>
#include <qwindowsysteminterface.h>

namespace QtWaylandClient {

QWaylandExtendedSurface::QWaylandExtendedSurface(QWaylandWindow *window)
{
}

QWaylandExtendedSurface::~QWaylandExtendedSurface()
{
   qt_extended_surface_destroy(object());
}

void QWaylandExtendedSurface::updateGenericProperty(const QString &name, const QVariant &value)
{
   QByteArray byteValue;

   QDataStream ds(&byteValue, QIODevice::WriteOnly);
   ds << value;

   update_generic_property(name, byteValue);
}

void QWaylandExtendedSurface::setContentOrientationMask(Qt::ScreenOrientations mask)
{
   int32_t wlmask = 0;

   if (mask & Qt::PrimaryOrientation) {
      wlmask |= QT_EXTENDED_SURFACE_ORIENTATION_PRIMARYORIENTATION;
   }

   if (mask & Qt::PortraitOrientation) {
      wlmask |= QT_EXTENDED_SURFACE_ORIENTATION_PORTRAITORIENTATION;
   }

   if (mask & Qt::LandscapeOrientation) {
      wlmask |= QT_EXTENDED_SURFACE_ORIENTATION_LANDSCAPEORIENTATION;
   }

   if (mask & Qt::InvertedPortraitOrientation) {
      wlmask |= QT_EXTENDED_SURFACE_ORIENTATION_INVERTEDPORTRAITORIENTATION;
   }

   if (mask & Qt::InvertedLandscapeOrientation) {
      wlmask |= QT_EXTENDED_SURFACE_ORIENTATION_INVERTEDLANDSCAPEORIENTATION;
   }

   set_content_orientation_mask(wlmask);
}

void QWaylandExtendedSurface::extended_surface_onscreen_visibility(int32_t visibility)
{
   // pending implementation
}

void QWaylandExtendedSurface::extended_surface_set_generic_property(const QString &name, wl_array *value)
{
   QByteArray data = QByteArray::fromRawData(static_cast<char *>(value->data), value->size);

   QVariant variantValue;
   QDataStream ds(data);
   ds >> variantValue;

   // pending implementation
}

void QWaylandExtendedSurface::extended_surface_close()
{
   // pending implementation
}

Qt::WindowFlags QWaylandExtendedSurface::setWindowFlags(Qt::WindowFlags flags)
{
   uint wlFlags = 0;

   if (flags & Qt::WindowStaysOnTopHint) {
      wlFlags |= QT_EXTENDED_SURFACE_WINDOWFLAG_STAYSONTOP;
   }

   if (flags & Qt::WindowOverridesSystemGestures) {
      wlFlags |= QT_EXTENDED_SURFACE_WINDOWFLAG_OVERRIDESSYSTEMGESTURES;
   }

   if (flags & Qt::BypassWindowManagerHint) {
      wlFlags |= QT_EXTENDED_SURFACE_WINDOWFLAG_BYPASSWINDOWMANAGER;
   }

   set_window_flags(wlFlags);

   return flags & (Qt::WindowStaysOnTopHint | Qt::WindowOverridesSystemGestures | Qt::BypassWindowManagerHint);
}

}
