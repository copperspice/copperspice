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

#ifndef QWAYLAND_BRADIENT_DECORATION_H
#define QWAYLAND_BRADIENT_DECORATION_H

#include <qcolor.h>
#include <qmargins.h>
#include <qpainter.h>
#include <qpointf.h>
#include <qrectf.h>
#include <qstatictext.h>

#include <qwayland_abstract_decoration_p.h>
#include <qwayland_inputdevice_p.h>

namespace QtWaylandClient {

enum Button {
   None,
   Close,
   Maximize,
   Minimize
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandBradientDecoration : public QWaylandAbstractDecoration
{
 public:
   QWaylandBradientDecoration();

 protected:
   bool handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,
         Qt::MouseButtons b, Qt::KeyboardModifiers mods) override;

   bool handleTouch(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,
         Qt::TouchPointState state, Qt::KeyboardModifiers mods) override;

   QMargins margins() const override;

   void paint(QPaintDevice *device) override;

 private:
   bool clickButton(Qt::MouseButtons b, Button btn);

   void processMouseTop(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods);
   void processMouseBottom(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods);
   void processMouseLeft(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods);
   void processMouseRight(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods);

   int m_factor;
   Button m_clicking;

   QRectF closeButtonRect() const;
   QRectF maximizeButtonRect() const;
   QRectF minimizeButtonRect() const;

   QColor m_foregroundColor;
   QColor m_backgroundColor;

   QStaticText m_windowTitle;
};

}

#endif
