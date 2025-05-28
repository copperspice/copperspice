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

// Copyright (C) 2014 Jolla Ltd

#ifndef QWAYLAND_SHELL_INTEGRATION_H
#define QWAYLAND_SHELL_INTEGRATION_H

#include <qglobal.h>

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandShellSurface;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandShellIntegration
{
 public:
   QWaylandShellIntegration() = default;

   virtual ~QWaylandShellIntegration()
   { }

   virtual bool initialize(QWaylandDisplay *display) = 0;
   virtual QWaylandShellSurface *createShellSurface(QWaylandWindow *window) = 0;
   virtual void handleKeyboardFocusChanged(QWaylandWindow *newFocus, QWaylandWindow *oldFocus) = 0;
};

}

#endif
