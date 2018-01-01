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

#ifndef QT7_VIDEOWIDGET_H
#define QT7_VIDEOWIDGET_H

#include <QtGui/QPaintEngine>
#include <phonon/videowidgetinterface.h>
#include "medianode.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class MediaNodeEvent;
    class VideoRenderWidget;

    class VideoWidget : public MediaNode, public Phonon::VideoWidgetInterface
    {
        QT7_CS_OBJECT(VideoWidget)
        CS_INTERFACES(Phonon::VideoWidgetInterface)

     public:
        VideoWidget(QObject *parent);
        virtual ~VideoWidget();

        Phonon::VideoWidget::AspectRatio aspectRatio() const override;
        void setAspectRatio(Phonon::VideoWidget::AspectRatio aspectRatio) override;
        qreal brightness() const override;
        void setBrightness(qreal) override;
        Phonon::VideoWidget::ScaleMode scaleMode() const override;
        void setScaleMode(Phonon::VideoWidget::ScaleMode scaleMode) override;
        qreal contrast() const override;
        void setContrast(qreal) override;
        qreal hue() const override;
        void setHue(qreal) override;
        qreal saturation() const override;
        void setSaturation(qreal) override;

        QWidget *widget() override;

        void updateVideo(VideoFrame &frame) override;

    protected:
        void mediaNodeEvent(const MediaNodeEvent *event) override;

    private:
        VideoRenderWidget *m_videoRenderWidget;
    };

}} // namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_VIDEOWIDGET_H
