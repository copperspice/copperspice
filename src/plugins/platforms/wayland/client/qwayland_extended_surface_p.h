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

#ifndef QWAYLAND_EXTENDED_SURFACE_H
#define QWAYLAND_EXTENDED_SURFACE_H

#include <qstring.h>
#include <qvariant.h>

#include <wayland-client.h>
#include <qwayland-surface-extension.h>

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandExtendedSurface : public QtWayland::qt_extended_surface
{
 public:
   QWaylandExtendedSurface(QWaylandWindow *window);
   ~QWaylandExtendedSurface();

   void setContentOrientationMask(Qt::ScreenOrientations mask);

   void updateGenericProperty(const QString &name, const QVariant &value);

   Qt::WindowFlags setWindowFlags(Qt::WindowFlags flags);

 private:
   void extended_surface_onscreen_visibility(int32_t visibility) override;
   void extended_surface_set_generic_property(const QString &name, wl_array *value) override;
   void extended_surface_close() override;

   QWaylandWindow *m_window;
   QVariantMap m_properties;
};

}

#endif
