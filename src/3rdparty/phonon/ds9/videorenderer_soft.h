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

#ifndef DS9_VIDEORENDERER_SOFT_H
#define DS9_VIDEORENDERER_SOFT_H

#include "abstractvideorenderer.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VIDEO

namespace Phonon
{
    namespace DS9
    {
        class VideoRendererSoftFilter;
        //this class is used to render evrything in software (like in the Graphics View)
        class VideoRendererSoft : public AbstractVideoRenderer, 
            public QObject //this is used to receive events
        {
        public:
            VideoRendererSoft(QWidget *);
            ~VideoRendererSoft();

            //Implementation from AbstractVideoRenderer
            void repaintCurrentFrame(QWidget *target, const QRect &rect) override;
            void notifyResize(const QSize&, Phonon::VideoWidget::AspectRatio, Phonon::VideoWidget::ScaleMode) override;
            QSize videoSize() const override;
            void applyMixerSettings(qreal brightness, qreal contrast, qreal hue, qreal saturation) override;
            bool isNative() const override;

            QImage snapshot() const override;
            void setSnapshot(const QImage &); 

        protected:
            bool event(QEvent *) override;

        private:
            VideoRendererSoftFilter *m_renderer;
            QTransform m_transform;
            QRect m_videoRect; //rectangle where the video is displayed
            QWidget *m_target;
        };
    }
}

#endif //QT_NO_PHONON_VIDEO

QT_END_NAMESPACE

#endif


