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

#ifndef QEGLWINDOW_H
#define QEGLWINDOW_H

#include "qeglfsintegration.h"
#include "qeglfsscreen.h"

#include <QPlatformWindow>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QEglFSWindow : public QPlatformWindow
{
public:
    QEglFSWindow(QWidget *w, QEglFSScreen *screen);

    void setGeometry(const QRect &);
    WId winId() const;

    QPlatformGLContext *glContext() const;

private:
    QEglFSScreen *m_screen;
    WId m_winid;
};
QT_END_NAMESPACE
#endif // QEGLWINDOW_H
