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

#ifndef QDIRECTFBWINDOW_H
#define QDIRECTFBWINDOW_H

#include <QPlatformWindow>

#include "qdirectfbconvenience.h"
#include "qdirectfbinput.h"

QT_BEGIN_NAMESPACE

class QDirectFbWindow : public QPlatformWindow
{
public:
    QDirectFbWindow(QWidget *tlw, QDirectFbInput *inputhandler);
    ~QDirectFbWindow();

    void setGeometry(const QRect &rect);
    void setOpacity(qreal level);

    void setVisible(bool visible);

    Qt::WindowFlags setWindowFlags(Qt::WindowFlags flags);
    bool setKeyboardGrabEnabled(bool grab);
    bool setMouseGrabEnabled(bool grab);
    void raise();
    void lower();
    WId winId() const;

    IDirectFBWindow *dfbWindow() const;

    // helper to get access to DirectFB types
    IDirectFBSurface *dfbSurface();

private:
    QDirectFBPointer<IDirectFBSurface> m_dfbSurface;
    QDirectFBPointer<IDirectFBWindow> m_dfbWindow;
    QDirectFbInput *m_inputHandler;
};

QT_END_NAMESPACE

#endif // QDIRECTFBWINDOW_H
