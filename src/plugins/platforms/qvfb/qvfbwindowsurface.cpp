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

#include "qvfbwindowsurface.h"
#include "qvfbintegration.h"
#include <QtCore/qdebug.h>
#include <QtGui/qpainter.h>
#include <private/qapplication_p.h>

QT_BEGIN_NAMESPACE

QVFbWindowSurface::QVFbWindowSurface(//QVFbIntegration *graphicsSystem,
                                     QVFbScreen *screen, QWidget *window)
    : QWindowSurface(window),
      mScreen(screen)
{
}

QVFbWindowSurface::~QVFbWindowSurface()
{
}

QPaintDevice *QVFbWindowSurface::paintDevice()
{
    return mScreen->screenImage();
}

void QVFbWindowSurface::flush(QWidget *widget, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(widget);
    Q_UNUSED(offset);

//    QRect rect = geometry();
//    QPoint topLeft = rect.topLeft();

    mScreen->setDirty(region.boundingRect());
}

void QVFbWindowSurface::resize(const QSize&)
{

// any size you like as long as it's full-screen...

    QRect rect(mScreen->availableGeometry());
    QWindowSurface::resize(rect.size());
}


QVFbWindow::QVFbWindow(QVFbScreen *screen, QWidget *window)
    : QPlatformWindow(window),
      mScreen(screen)
{
}


void QVFbWindow::setGeometry(const QRect &)
{

// any size you like as long as it's full-screen...

    QRect rect(mScreen->availableGeometry());
    QWindowSystemInterface::handleGeometryChange(this->widget(), rect);

    QPlatformWindow::setGeometry(rect);
}



QT_END_NAMESPACE
