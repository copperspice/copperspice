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

// Copyright (C) 2014 Robin Burchell <robin.burchell@viroteck.net>

#include <qwayland_bradient_decoration.h>

namespace QtWaylandClient {

QWaylandBradientDecoration::QWaylandBradientDecoration()
   : QWaylandAbstractDecoration(), m_clicking(Button::None), m_factor(1)
{
   // pending implementation
}

QMargins QWaylandBradientDecoration::margins() const
{
   return QMargins(3 * m_factor, 30 * m_factor, 3 * m_factor, 3 * m_factor);
}

void QWaylandBradientDecoration::paint(QPaintDevice *device)
{
   // pending implementation
}

bool QWaylandBradientDecoration::handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,
      Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
   // pending implementation

   return false;
}

bool QWaylandBradientDecoration::handleTouch(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,
      Qt::TouchPointState state, Qt::KeyboardModifiers mods)
{
   // pending implementation
   return false;
}

}
