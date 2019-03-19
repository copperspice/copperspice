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

#ifndef GSTREAMER_X11RENDERER_H
#define GSTREAMER_X11RENDERER_H

// located in gstreamer
#include <videowidget.h>
#include <common.h>

#include <qwidget.h>
#include <gst/gst.h>
#include <qstringfwd.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{

class OverlayWidget;
class X11Renderer : public AbstractRenderer
{
public:
    X11Renderer(VideoWidget *videoWidget);
    ~X11Renderer();

    void handlePaint(QPaintEvent *event) override;
    void aspectRatioChanged(Phonon::VideoWidget::AspectRatio aspectRatio) override;
    void scaleModeChanged(Phonon::VideoWidget::ScaleMode scaleMode) override;
    void movieSizeChanged(const QSize &movieSize) override;
    void handleMediaNodeEvent(const MediaNodeEvent *event) override;
    bool eventFilter(QEvent *) override;
    bool paintsOnWidget() override { return false; }

    bool overlaySet() const { return m_overlaySet; }
    void setOverlay();
    void windowExposed();
    GstElement *createVideoSink();

private:
    OverlayWidget *m_renderWidget;
    bool m_overlaySet;
};

}
} //namespace Phonon::Gstreamer

QT_END_NAMESPACE

#endif // Phonon_GSTREAMER_X11RENDERER_H
