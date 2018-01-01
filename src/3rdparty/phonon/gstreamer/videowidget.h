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

#ifndef GSTREAMER_VIDEOWIDGET_H
#define GSTREAMER_VIDEOWIDGET_H

#include <phonon/videowidget.h>
#include <phonon/videowidgetinterface.h>
#include "backend.h"
#include "common.h"
#include "medianode.h"
#include "abstractrenderer.h"
#include "videowidget.h"
#include <gst/gst.h>

#ifndef QT_NO_PHONON_VIDEO

QT_BEGIN_NAMESPACE

class QString;

namespace Phonon
{
namespace Gstreamer
{

class VideoWidget : public QWidget, public Phonon::VideoWidgetInterface, public MediaNode
{
    GSTRM_CS_OBJECT(VideoWidget)
    CS_INTERFACES(Phonon::VideoWidgetInterface, Phonon::Gstreamer::MediaNode)
   
public:
    VideoWidget(Backend *backend, QWidget *parent = nullptr);
    ~VideoWidget();

    void setupVideoBin();
    void paintEvent(QPaintEvent *event) override;
    void mediaNodeEvent(const MediaNodeEvent *event) override;
    void setVisible(bool) override;

    Phonon::VideoWidget::AspectRatio aspectRatio() const override;
    void setAspectRatio(Phonon::VideoWidget::AspectRatio aspectRatio) override;
    Phonon::VideoWidget::ScaleMode scaleMode() const override;
    void setScaleMode(Phonon::VideoWidget::ScaleMode) override;
    qreal brightness() const override;
    void setBrightness(qreal) override;
    qreal contrast() const override;
    void setContrast(qreal) override;
    qreal hue() const override;
    void setHue(qreal) override;
    qreal saturation() const override;
    void setSaturation(qreal) override;
    void setMovieSize(const QSize &size);
    QSize sizeHint() const override;
    QRect scaleToAspect(QRect srcRect, int w, int h) const;
    QRect calculateDrawFrameRect() const;

    GstElement *videoElement() override
    {
        Q_ASSERT(m_videoBin);
        return m_videoBin;
    }

    QSize movieSize() const {
        return m_movieSize;
    }

    bool event(QEvent *) override;

    QWidget *widget() override {
        return this;
    }

protected:
    GstElement *m_videoBin;
    QSize m_movieSize;
    AbstractRenderer *m_renderer;

private:
    Phonon::VideoWidget::AspectRatio m_aspectRatio;
    qreal m_brightness, m_hue, m_contrast, m_saturation;
    Phonon::VideoWidget::ScaleMode m_scaleMode;

    GstElement *m_videoBalance;
    GstElement *m_colorspace;
    GstElement *m_videoplug;
};

}
} //namespace Phonon::Gstreamer

QT_END_NAMESPACE
#endif //QT_NO_PHONON_VIDEO
#endif // Phonon_GSTREAMER_VIDEOWIDGET_H
