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

#ifndef DS9_VIDEORENDERER_DEFAULT_H
#define DS9_VIDEORENDERER_DEFAULT_H

#include "abstractvideorenderer.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VIDEO

namespace Phonon
{
    namespace DS9
    {
        class VideoRendererDefault : public AbstractVideoRenderer
        {
        public:
            VideoRendererDefault(QWidget *target);
            ~VideoRendererDefault();

            //Implementation from AbstractVideoRenderer
            void repaintCurrentFrame(QWidget *target, const QRect &rect);
            void notifyResize(const QSize&, Phonon::VideoWidget::AspectRatio, Phonon::VideoWidget::ScaleMode);
            QSize videoSize() const;
            QImage snapshot() const;
            void applyMixerSettings(qreal brightness, qreal contrast, qreal m_hue, qreal saturation);
            bool isNative() const;
        private:
            QWidget *m_target;
        };
    }
}

#endif //QT_NO_PHONON_VIDEO

QT_END_NAMESPACE

#endif

