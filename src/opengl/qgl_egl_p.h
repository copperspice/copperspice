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

#ifndef QGL_EGL_P_H
#define QGL_EGL_P_H

#include <qegl_p.h>
#include <qeglcontext_p.h>
#include <qeglproperties_p.h>

QT_BEGIN_NAMESPACE

class QGLFormat;

void qt_eglproperties_set_glformat(QEglProperties &props, const QGLFormat &format);
void qt_glformat_from_eglconfig(QGLFormat &format, const EGLConfig config);

QT_END_NAMESPACE

#endif // QGL_EGL_P_H
