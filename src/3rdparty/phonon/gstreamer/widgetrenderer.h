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

#ifndef GSTREAMER_WIDGETRENDERER_H
#define GSTREAMER_WIDGETRENDERER_H

#include "videowidget.h"
#include "common.h"

#ifndef QT_NO_OPENGL
#include <QtOpenGL/QGLFormat>
#include <QtOpenGL/QGLWidget>
#endif

#ifndef QT_NO_PHONON_VIDEO
QT_BEGIN_NAMESPACE

class QString;

namespace Phonon
{
namespace Gstreamer
{

class WidgetRenderer : public AbstractRenderer
{
public:
    WidgetRenderer(VideoWidget *videoWidget);

    bool eventFilter(QEvent * event) override;
    void handlePaint(QPaintEvent *paintEvent) override;
    void handleMediaNodeEvent(const MediaNodeEvent *event) override;

    const QImage& currentFrame() const;
    QRect drawFrameRect() const { return m_drawFrameRect; }
    void setNextFrame(const QByteArray &array, int width, int height);
    bool frameIsSet() { return !m_array.isNull(); }
    void clearFrame();

private:
    mutable QImage m_frame;
    QByteArray m_array;
    int m_width;
    int m_height;
    QRect m_drawFrameRect;
};

}
} //namespace Phonon::Gstreamer

QT_END_NAMESPACE
#endif //QT_NO_PHONON_VIDEO
#endif // Phonon_GSTREAMER_WIDGETRENDERER_H
