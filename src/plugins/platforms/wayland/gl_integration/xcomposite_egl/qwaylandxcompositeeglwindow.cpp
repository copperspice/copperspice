/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qwaylandxcompositeeglwindow.h"

#include <QtCore/QDebug>

QWaylandXCompositeEGLWindow::QWaylandXCompositeEGLWindow(QWidget *window, QWaylandXCompositeEGLIntegration *glxIntegration)
    : QWaylandWindow(window)
    , mGlxIntegration(glxIntegration)
    , mContext(0)
{

}

QWaylandWindow::WindowType QWaylandXCompositeEGLWindow::windowType() const
{
    //yeah. this type needs a new name
    return QWaylandWindow::Egl;
}

QPlatformGLContext * QWaylandXCompositeEGLWindow::glContext() const
{
    if (!mContext) {
        qDebug() << "creating glcontext;";
        QWaylandXCompositeEGLWindow *that = const_cast<QWaylandXCompositeEGLWindow *>(this);
        that->mContext = new QWaylandXCompositeEGLContext(mGlxIntegration,that);
    }
    return mContext;
}

void QWaylandXCompositeEGLWindow::setGeometry(const QRect &rect)
{
    QWaylandWindow::setGeometry(rect);

    if (mContext) {
        mContext->geometryChanged();
    }
}
