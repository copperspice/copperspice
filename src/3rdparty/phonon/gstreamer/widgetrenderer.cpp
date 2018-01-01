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

/********************************************************
**  This file is part of the KDE project.
********************************************************/

#include <QtGui/QPainter>
#include <gst/gst.h>
#include "common.h"
#include "message.h"
#include "mediaobject.h"
#include "qwidgetvideosink.h"
#include "widgetrenderer.h"
#include "qrgb.h"

// support old OpenGL installations (1.2)
// assume that if TEXTURE0 isn't defined, none are
#ifndef GL_TEXTURE0
# define GL_TEXTURE0    0x84C0
# define GL_TEXTURE1    0x84C1
# define GL_TEXTURE2    0x84C2
#endif

#ifndef QT_NO_PHONON_VIDEO
QT_BEGIN_NAMESPACE

static void frameRendered()
{
    static QString displayFps = qgetenv("PHONON_GST_FPS");
    if (displayFps.isEmpty())
        return;

    static int frames = 0;
    static QTime lastTime = QTime::currentTime();
    QTime time = QTime::currentTime();

    int delta = lastTime.msecsTo(time);
    if (delta > 2000) {
        printf("FPS: %f\n", 1000.0 * frames / qreal(delta));
        lastTime = time;
        frames = 0;
    }

    ++frames;
}

namespace Phonon
{
namespace Gstreamer
{

WidgetRenderer::WidgetRenderer(VideoWidget *videoWidget)
        : AbstractRenderer(videoWidget)
        , m_width(0)
        , m_height(0)
{
    videoWidget->backend()->logMessage("Creating QWidget renderer");
    if ((m_videoSink = GST_ELEMENT(g_object_new(get_type_RGB(), NULL)))) {
        gst_object_ref (GST_OBJECT (m_videoSink)); //Take ownership
        gst_object_sink (GST_OBJECT (m_videoSink));
        
        QWidgetVideoSinkBase*  sink = reinterpret_cast<QWidgetVideoSinkBase*>(m_videoSink);
        // Let the videosink know which widget to direct frame updates to
        sink->renderWidget = videoWidget;
    }

    // Clear the background with black by default
    QPalette palette;
    palette.setColor(QPalette::Background, Qt::black);
    m_videoWidget->setPalette(palette);
    m_videoWidget->setAutoFillBackground(true);
    m_videoWidget->setAttribute(Qt::WA_NoSystemBackground, false);
    m_videoWidget->setAttribute(Qt::WA_PaintOnScreen, false);
}

void WidgetRenderer::setNextFrame(const QByteArray &array, int w, int h)
{
    if (m_videoWidget->root()->state() == Phonon::LoadingState)
        return;

    m_frame = QImage(); 
    {
        m_frame = QImage((uchar *)array.constData(), w, h, QImage::Format_RGB32);
    }

    m_array = array;
    m_width = w;
    m_height = h;

    m_videoWidget->update();
}

void WidgetRenderer::handleMediaNodeEvent(const MediaNodeEvent *event)
{
    switch (event->type()) {
    case MediaNodeEvent::SourceChanged:
    {
        clearFrame();
        break;
    }
    default:
        break;
    }
}

void WidgetRenderer::clearFrame()
{
    m_frame = QImage();
    m_array = QByteArray();
    m_videoWidget->update();
}

const QImage &WidgetRenderer::currentFrame() const
{
    return m_frame;
}

void WidgetRenderer::handlePaint(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(m_videoWidget);
    m_drawFrameRect = m_videoWidget->calculateDrawFrameRect();
    painter.drawImage(drawFrameRect(), currentFrame());
    frameRendered();
}

bool WidgetRenderer::eventFilter(QEvent * event)
{
    if (event->type() == QEvent::User) {
        NewFrameEvent *frameEvent= static_cast <NewFrameEvent *>(event);
        setNextFrame(frameEvent->frame, frameEvent->width, frameEvent->height);
        return true;
    }
    return false;
}

}
} //namespace Phonon::Gstreamer

QT_END_NAMESPACE
#endif //QT_NO_PHONON_VIDEO
