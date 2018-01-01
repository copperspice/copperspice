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

#ifndef QXCBWINDOWSURFACE_H
#define QXCBWINDOWSURFACE_H

#include <qwindowsurface_p.h>
#include <xcb/xcb.h>
#include "qxcbobject.h"

class QXcbShmImage;

class QXcbWindowSurface : public QXcbObject, public QWindowSurface
{
public:
    QXcbWindowSurface(QWidget *widget, bool setDefaultSurface = true);
    ~QXcbWindowSurface();

    QPaintDevice *paintDevice();
    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);
    void resize(const QSize &size);
    bool scroll(const QRegion &area, int dx, int dy);

    void beginPaint(const QRegion &);
    void endPaint(const QRegion &);

private:
    QXcbShmImage *m_image;
    bool m_syncingResize;
};

#endif
