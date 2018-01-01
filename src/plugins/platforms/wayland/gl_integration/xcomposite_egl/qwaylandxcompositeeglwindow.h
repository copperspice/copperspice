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

#ifndef QWAYLANDXCOMPOSITEEGLWINDOW_H
#define QWAYLANDXCOMPOSITEEGLWINDOW_H

#include "qwaylandwindow.h"
#include "qwaylandxcompositeeglintegration.h"
#include "qwaylandxcompositeeglcontext.h"

class QWaylandXCompositeEGLWindow : public QWaylandWindow
{
public:
    QWaylandXCompositeEGLWindow(QWidget *window, QWaylandXCompositeEGLIntegration *glxIntegration);
    WindowType windowType() const;

    QPlatformGLContext *glContext() const;

    void setGeometry(const QRect &rect);

private:
    QWaylandXCompositeEGLIntegration *mGlxIntegration;
    QWaylandXCompositeEGLContext *mContext;

};

#endif // QWAYLANDXCOMPOSITEEGLWINDOW_H
