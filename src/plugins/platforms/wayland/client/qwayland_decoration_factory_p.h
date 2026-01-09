/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

// Copyright (C) 2014 Robin Burchell <robin.burchell@viroteck.net>

#ifndef QWAYLAND_DECORATION_FACTORY_H
#define QWAYLAND_DECORATION_FACTORY_H

#include <qstringlist.h>

namespace QtWaylandClient {

class QWaylandAbstractDecoration;

class Q_WAYLAND_CLIENT_EXPORT QWaylandDecorationFactory
{
 public:
   static QStringList keys(const QString &pluginPath = QString());
   static QWaylandAbstractDecoration *create(const QString &name, const QStringList &args, const QString &pluginPath = QString());
};

}

#endif