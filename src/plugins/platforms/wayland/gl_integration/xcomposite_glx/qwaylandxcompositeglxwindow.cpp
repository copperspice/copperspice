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

#include "qwaylandxcompositeglxwindow.h"

#include <QtCore/QDebug>

QWaylandXCompositeGLXWindow::QWaylandXCompositeGLXWindow(QWidget *window, QWaylandXCompositeGLXIntegration *glxIntegration)
    : QWaylandWindow(window)
    , mGlxIntegration(glxIntegration)
    , mContext(0)
{

}

QWaylandWindow::WindowType QWaylandXCompositeGLXWindow::windowType() const
{
    //yeah. this type needs a new name
    return QWaylandWindow::Egl;
}

QPlatformGLContext * QWaylandXCompositeGLXWindow::glContext() const
{
    if (!mContext) {
        qDebug() << "creating glcontext;";
        QWaylandXCompositeGLXWindow *that = const_cast<QWaylandXCompositeGLXWindow *>(this);
        that->mContext = new QWaylandXCompositeGLXContext(mGlxIntegration,that);
    }
    return mContext;
}

void QWaylandXCompositeGLXWindow::setGeometry(const QRect &rect)
{
    QWaylandWindow::setGeometry(rect);

    if (mContext) {
        mContext->geometryChanged();
    }
}
