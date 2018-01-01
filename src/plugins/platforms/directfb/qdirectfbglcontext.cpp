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

#include "qdirectfbglcontext.h"

#include <directfbgl.h>

#include <QDebug>

QT_BEGIN_NAMESPACE

QDirectFbGLContext::QDirectFbGLContext(IDirectFBGL *glContext)
    : m_dfbGlContext(glContext)
{
    DFBResult result;
    DFBGLAttributes glAttribs;
    result = m_dfbGlContext->GetAttributes(glContext, &glAttribs);
    if (result == DFB_OK) {
        m_windowFormat.setDepthBufferSize(glAttribs.depth_size);
        m_windowFormat.setStencilBufferSize(glAttribs.stencil_size);

        m_windowFormat.setRedBufferSize(glAttribs.red_size);
        m_windowFormat.setGreenBufferSize(glAttribs.green_size);
        m_windowFormat.setBlueBufferSize(glAttribs.blue_size);
        m_windowFormat.setAlphaBufferSize(glAttribs.alpha_size);

        m_windowFormat.setAccumBufferSize(glAttribs.accum_red_size);
        m_windowFormat.setAlpha(glAttribs.accum_alpha_size);

        m_windowFormat.setDoubleBuffer(glAttribs.double_buffer);
        m_windowFormat.setStereo(glAttribs.stereo);
    }
}

void QDirectFbGLContext::makeCurrent()
{
    QPlatformGLContext::makeCurrent();
    m_dfbGlContext->Lock(m_dfbGlContext);
}

void QDirectFbGLContext::doneCurrent()
{
    QPlatformGLContext::doneCurrent();
    m_dfbGlContext->Unlock(m_dfbGlContext);
}

void *QDirectFbGLContext::getProcAddress(const QString &procName)
{
    void *proc;
    DFBResult result = m_dfbGlContext->GetProcAddress(m_dfbGlContext,qPrintable(procName),&proc);
    if (result == DFB_OK)
        return proc;
    return 0;
}

void QDirectFbGLContext::swapBuffers()
{
//    m_dfbGlContext->Unlock(m_dfbGlContext); //maybe not in doneCurrent()
    qDebug() << "Swap buffers";
}

QPlatformWindowFormat QDirectFbGLContext::platformWindowFormat() const
{
    return m_windowFormat;
}

QT_END_NAMESPACE
