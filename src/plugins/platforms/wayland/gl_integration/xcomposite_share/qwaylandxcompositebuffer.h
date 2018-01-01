/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QWAYLANDXCOMPOSITEBUFFER_H
#define QWAYLANDXCOMPOSITEBUFFER_H

#include "qwaylandbuffer.h"

#include "wayland-xcomposite-client-protocol.h"

class QWaylandXCompositeBuffer : public QWaylandBuffer
{
public:
    QWaylandXCompositeBuffer(struct wl_xcomposite *xcomposite,
                             uint32_t window,
                             const QSize &size,
                             struct wl_visual *visual);

    QSize size() const;
private:
    QSize mSize;
};

#endif // QWAYLANDXCOMPOSITEBUFFER_H
