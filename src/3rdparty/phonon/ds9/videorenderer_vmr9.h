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

#ifndef DS9_VIDEORENDERER_VMR9_H
#define DS9_VIDEORENDERER_VMR9_H

#include "abstractvideorenderer.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VIDEO

namespace Phonon
{
    namespace DS9
    {
        class VideoRendererVMR9 : public AbstractVideoRenderer
        {
        public:
            VideoRendererVMR9(QWidget *target);
            ~VideoRendererVMR9();

            //Implementation from AbstractVideoRenderer
            void repaintCurrentFrame(QWidget *target, const QRect &rect) override;
            void notifyResize(const QSize&, Phonon::VideoWidget::AspectRatio, Phonon::VideoWidget::ScaleMode) override;
            QSize videoSize() const override;
            QImage snapshot() const override;
            void applyMixerSettings(qreal brightness, qreal contrast, qreal m_hue, qreal saturation) override;
            bool isNative() const override;

        private:
            QWidget *m_target;
        };
    }
}

#endif //QT_NO_PHONON_VIDEO

QT_END_NAMESPACE

#endif

