/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QEGLWINDOWSURFACE_H
#define QEGLWINDOWSURFACE_H

#include <qeglfsintegration.h>
#include <qeglfswindow.h>
#include <qwindowsurface_p.h>

QT_BEGIN_NAMESPACE

class QEglFSWindowSurface : public QWindowSurface
{
public:
    QEglFSWindowSurface(QEglFSScreen *screen, QWidget *window);
    ~QEglFSWindowSurface() {}

    QPaintDevice *paintDevice() { return m_paintDevice; }
    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);
    void resize(const QSize &size);
private:
    QPaintDevice *m_paintDevice;
};

QT_END_NAMESPACE

#endif // QEGLWINDOWSURFACE_H
