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

#ifndef GSTREAMER_ABSTRACTRENDERER_H
#define GSTREAMER_ABSTRACTRENDERER_H

#include "backend.h"
#include "common.h"
#include "medianode.h"
#include <phonon/videowidget.h>

#ifndef QT_NO_PHONON_VIDEO

QT_BEGIN_NAMESPACE

class QString;
namespace Phonon
{
namespace Gstreamer
{

class VideoWidget;

class AbstractRenderer
{
public:
    AbstractRenderer(VideoWidget *video) :
          m_videoWidget(video)
        , m_videoSink(0) { }
    virtual ~AbstractRenderer();
    virtual GstElement *videoSink() {return m_videoSink;}
    virtual void aspectRatioChanged(Phonon::VideoWidget::AspectRatio aspectRatio);
    virtual void scaleModeChanged(Phonon::VideoWidget::ScaleMode scaleMode);
    virtual void movieSizeChanged(const QSize &movieSize);
    virtual void handleMediaNodeEvent(const MediaNodeEvent *event) = 0;
    virtual bool eventFilter(QEvent *) = 0;
    virtual void handlePaint(QPaintEvent *) {}
    virtual bool paintsOnWidget() { return true; } // Controls overlays

protected:
    VideoWidget *m_videoWidget;
    GstElement *m_videoSink;
};

}
} //namespace Phonon::Gstreamer

QT_END_NAMESPACE
#endif //QT_NO_PHONON_VIDEO
#endif // Phonon_GSTREAMER_ABSTRACTRENDERER_H
