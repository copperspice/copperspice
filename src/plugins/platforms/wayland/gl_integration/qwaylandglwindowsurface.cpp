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

#include <qwaylandglwindowsurface.h>
#include <qwaylanddisplay.h>
#include <qwaylandwindow.h>
#include <qwaylandscreen.h>
#include <QtOpenGL/QGLFramebufferObject>
#include <QtOpenGL/QGLContext>
#include <qglengineshadermanager_p.h>

QT_BEGIN_NAMESPACE

static void drawTexture(const QRectF &rect, GLuint tex_id, const QSize &texSize, const QRectF &br)
{
#if !defined(QT_OPENGL_ES_2)
    QGLContext *ctx = const_cast<QGLContext *>(QGLContext::currentContext());
#endif
    const GLenum target = GL_TEXTURE_2D;
    QRectF src = br.isEmpty()
        ? QRectF(QPointF(), texSize)
        : QRectF(QPointF(br.x(), texSize.height() - br.bottom()), br.size());

    if (target == GL_TEXTURE_2D) {
        qreal width = texSize.width();
        qreal height = texSize.height();

        src.setLeft(src.left() / width);
        src.setRight(src.right() / width);
        src.setTop(src.top() / height);
        src.setBottom(src.bottom() / height);
    }

    const GLfloat tx1 = src.left();
    const GLfloat tx2 = src.right();
    const GLfloat ty1 = src.top();
    const GLfloat ty2 = src.bottom();

    GLfloat texCoordArray[4*2] = {
        tx1, ty2, tx2, ty2, tx2, ty1, tx1, ty1
    };

    GLfloat vertexArray[4*2];
    vertexArray[0] = rect.left(); vertexArray[1] = rect.top();
    vertexArray[2] = rect.right(); vertexArray[3] = rect.top();
    vertexArray[4] = rect.right(); vertexArray[5] = rect.bottom();
    vertexArray[6] = rect.left(); vertexArray[7] = rect.bottom();

    glVertexAttribPointer(QT_VERTEX_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, vertexArray);
    glVertexAttribPointer(QT_TEXTURE_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, texCoordArray);

    glBindTexture(target, tex_id);

    glEnableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
    glEnableVertexAttribArray(QT_TEXTURE_COORDS_ATTR);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
    glDisableVertexAttribArray(QT_TEXTURE_COORDS_ATTR);

    glBindTexture(target, 0);
}

static void blitTexture(QGLContext *ctx, GLuint texture, const QSize &viewport, const QSize &texSize, const QRect &targetRect, const QRect &sourceRect)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glViewport(0, 0, viewport.width(), viewport.height());

    QGLShaderProgram *blitProgram =
        QGLEngineSharedShaders::shadersForContext(ctx)->blitProgram();
    blitProgram->bind();
    blitProgram->setUniformValue("imageTexture", 0 /*QT_IMAGE_TEXTURE_UNIT*/);

    // The shader manager's blit program does not multiply the
    // vertices by the pmv matrix, so we need to do the effect
    // of the orthographic projection here ourselves.
    QRectF r;
    qreal w = viewport.width();
    qreal h = viewport.height();
    r.setLeft((targetRect.left() / w) * 2.0f - 1.0f);
    if (targetRect.right() == (viewport.width() - 1))
        r.setRight(1.0f);
    else
        r.setRight((targetRect.right() / w) * 2.0f - 1.0f);
    r.setBottom((targetRect.top() / h) * 2.0f - 1.0f);
    if (targetRect.bottom() == (viewport.height() - 1))
        r.setTop(1.0f);
    else
        r.setTop((targetRect.bottom() / w) * 2.0f - 1.0f);

    drawTexture(r, texture, texSize, sourceRect);
}

QWaylandGLWindowSurface::QWaylandGLWindowSurface(QWidget *window)
    : QWindowSurface(window)
    , mDisplay(QWaylandScreen::waylandScreenFromWidget(window)->display())
    , mPaintDevice(0)
{

}

QWaylandGLWindowSurface::~QWaylandGLWindowSurface()
{
    delete mPaintDevice;
}

QPaintDevice *QWaylandGLWindowSurface::paintDevice()
{
    return mPaintDevice;
}

void QWaylandGLWindowSurface::beginPaint(const QRegion &)
{
    window()->platformWindow()->glContext()->makeCurrent();
    glClearColor(0,0,0,0xff);
    glClear(GL_COLOR_BUFFER_BIT);
}

void QWaylandGLWindowSurface::flush(QWidget *widget, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(offset);
    Q_UNUSED(region);
    QWaylandWindow *ww = (QWaylandWindow *) widget->platformWindow();

    if (mPaintDevice->isBound())
        mPaintDevice->release();

    QRect rect(0,0,size().width(),size().height());
    QGLContext *ctx = QGLContext::fromPlatformGLContext(ww->glContext());
    blitTexture(ctx,mPaintDevice->texture(),size(),mPaintDevice->size(),rect,rect);
    ww->glContext()->swapBuffers();
}

void QWaylandGLWindowSurface::resize(const QSize &size)
{
    QWindowSurface::resize(size);
    window()->platformWindow()->glContext()->makeCurrent();
    delete mPaintDevice;
    mPaintDevice = new QGLFramebufferObject(size,QGLFramebufferObject::CombinedDepthStencil);
}

QT_END_NAMESPACE
