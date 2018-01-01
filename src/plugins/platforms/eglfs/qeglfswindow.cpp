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

#include "qeglfswindow.h"

#include <QtGui/QWindowSystemInterface>

QT_BEGIN_NAMESPACE

QEglFSWindow::QEglFSWindow(QWidget *w, QEglFSScreen *screen)
    : QPlatformWindow(w), m_screen(screen)
{
    static int serialNo = 0;
    m_winid  = ++serialNo;
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglWindow %p: %p %p 0x%x\n", this, w, screen, uint(m_winid));
#endif
}


void QEglFSWindow::setGeometry(const QRect &)
{
    // We only support full-screen windows
    QRect rect(m_screen->availableGeometry());
    QWindowSystemInterface::handleGeometryChange(this->widget(), rect);

    // Since toplevels are fullscreen, propegate the screen size back to the widget
    widget()->setGeometry(rect);

    QPlatformWindow::setGeometry(rect);
}

WId QEglFSWindow::winId() const
{
    return m_winid;
}



QPlatformGLContext *QEglFSWindow::glContext() const
{
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglWindow::glContext %p\n", m_screen->platformContext());
#endif
    Q_ASSERT(m_screen);
     return m_screen->platformContext();
}

QT_END_NAMESPACE
