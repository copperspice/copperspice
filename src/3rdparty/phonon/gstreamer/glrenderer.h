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

#ifndef GSTREAMER_GLRENDERER_H
#define GSTREAMER_GLRENDERER_H

#include "videowidget.h"
#include "common.h"

#ifndef QT_NO_OPENGL

#include <QtOpenGL/QGLFormat>
#include <QtOpenGL/QGLWidget>

#ifndef QT_OPENGL_ES

QT_BEGIN_NAMESPACE

class QString;

namespace Phonon
{
namespace Gstreamer
{
class GLRenderWidgetImplementation;

class GLRenderer : public AbstractRenderer
{
public:
    GLRenderer(VideoWidget *control);
    ~GLRenderer();
    void handleMediaNodeEvent(const MediaNodeEvent *event);
    bool eventFilter(QEvent * event);
    bool paintsOnWidget() { return false; }
private:
    GLRenderWidgetImplementation *m_glWindow;
};

class GLRenderWidgetImplementation : public QGLWidget
{
    GSTRM_CS_OBJECT(GLRenderWidgetImplementation)

    // ARB_fragment_program
    typedef void (*_glProgramStringARB) (GLenum, GLenum, GLsizei, const GLvoid *);
    typedef void (*_glBindProgramARB) (GLenum, GLuint);
    typedef void (*_glDeleteProgramsARB) (GLsizei, const GLuint *);
    typedef void (*_glGenProgramsARB) (GLsizei, GLuint *);
    typedef void (*_glActiveTexture) (GLenum);
public:
    GLRenderWidgetImplementation(VideoWidget *control, const QGLFormat &format);
    void paintEvent(QPaintEvent *event);
    GstElement *createVideoSink();
    void updateTexture(const QByteArray &array, int width, int height);
    bool hasYUVSupport() const;
    const QImage& currentFrame() const;
    QRect drawFrameRect() const { return m_drawFrameRect; }
    bool frameIsSet() const { return !m_array.isNull(); }
    void setNextFrame(const QByteArray &array, int width, int height);
    void clearFrame();
private:
    _glProgramStringARB glProgramStringARB;
    _glBindProgramARB glBindProgramARB;
    _glDeleteProgramsARB glDeleteProgramsARB;
    _glGenProgramsARB glGenProgramsARB;
    _glActiveTexture glActiveTexture;

    mutable QImage m_frame;
    QByteArray m_array;
    int m_width;
    int m_height;
    QRect m_drawFrameRect;
    GLuint m_texture[3];

    bool m_hasPrograms;
    GLuint m_program;
    bool m_yuvSupport;
    VideoWidget *m_videoWidget;
};

}
} //namespace Phonon::Gstreamer

QT_END_NAMESPACE

#endif //QT_OPENGL_ES
#endif // QT_NO_OPENGL

#endif // Phonon_GSTREAMER_GLRENDERER_H
