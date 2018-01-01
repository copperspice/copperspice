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

#ifndef DS9_ABSTRACTVIDEORENDERER_H
#define DS9_ABSTRACTVIDEORENDERER_H

#include "backendnode.h"

#include <phonon/videowidget.h>

QT_BEGIN_NAMESPACE

class QImage;

#ifndef QT_NO_PHONON_VIDEO

namespace Phonon
{
    namespace DS9
    {
        //this is the interface used by the videorenderer from the VideoWidget class
        class AbstractVideoRenderer
        {
        public:
            virtual ~AbstractVideoRenderer();

            virtual void repaintCurrentFrame(QWidget *target, const QRect &rect) = 0;
            virtual void notifyResize(const QSize&, Phonon::VideoWidget::AspectRatio, Phonon::VideoWidget::ScaleMode) = 0;
            virtual void applyMixerSettings(qreal brightness, qreal contrast, qreal m_hue, qreal saturation) = 0;
            
            void setActive(bool);
            bool isActive() const;
            
            virtual bool isNative() const = 0;
            virtual QImage snapshot() const = 0;

            Filter getFilter() const;
            QSize sizeHint() const;

        protected:
            virtual QSize videoSize() const = 0;

            AbstractVideoRenderer();
            void internalNotifyResize(const QSize &size, const QSize &videoSize, 
                Phonon::VideoWidget::AspectRatio aspectRatio, Phonon::VideoWidget::ScaleMode scaleMode);


            Filter m_filter;
            int m_dstX, m_dstY, m_dstWidth, m_dstHeight;
            bool m_active;
        };
    }
}

#endif //QT_NO_PHONON_VIDEO


QT_END_NAMESPACE

#endif
