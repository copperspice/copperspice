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

#ifndef QOPENKODEWINDOW_H
#define QOPENKODEWINDOW_H

#include <QtGui/QPlatformWindow>
#include <QtCore/QVector>

#include <KD/kd.h>

QT_BEGIN_NAMESPACE

class QEGLPlatformContext;
class QPlatformEventLoopIntegration;

class QOpenKODEWindow : public QPlatformWindow
{
public:
    QOpenKODEWindow(QWidget *tlw);
    ~QOpenKODEWindow();

    void setGeometry(const QRect &rect);
    void setVisible(bool visible);
    WId winId() const;

    QPlatformGLContext *glContext() const;

    void raise();
    void lower();

    void processKeyEvents( const KDEvent *event );
    void processMouseEvents( const KDEvent *event );

private:
    struct KDWindow *m_kdWindow;
    EGLNativeWindowType m_eglWindow;
    EGLConfig m_eglConfig;
    QVector<EGLint> m_eglWindowAttrs;
    QVector<EGLint> m_eglContextAttrs;
    EGLenum m_eglApi;
    QEGLPlatformContext *m_platformGlContext;

    bool isFullScreen;
};

QT_END_NAMESPACE

#endif //QOPENKODEWINDOW_H
