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

#include <qwayland_key_extension_p.h>

namespace QtWaylandClient {

QWaylandKeyExtension::QWaylandKeyExtension(QWaylandDisplay *display, uint32_t id)
   : m_display(nullptr)
{
}

void QWaylandKeyExtension::key_extension_qtkey(struct wl_surface *surface, uint32_t time, uint32_t type,
      uint32_t key, uint32_t modifiers, uint32_t nativeScanCode, uint32_t nativeVirtualKey,
      uint32_t nativeModifiers, const QString &text, uint32_t autorep, uint32_t count)
{
   // pending implementation
}

}
