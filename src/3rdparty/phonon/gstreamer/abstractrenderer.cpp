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

#include "abstractrenderer.h"

#ifndef QT_NO_PHONON_VIDEO
QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{


AbstractRenderer::~AbstractRenderer()
{
    if (m_videoSink) {
        gst_object_unref (GST_OBJECT (m_videoSink)); //Take ownership
        m_videoSink = 0;
    }
}

void AbstractRenderer::aspectRatioChanged(Phonon::VideoWidget::AspectRatio aspectRatio)
{
    Q_UNUSED(aspectRatio);
}

void AbstractRenderer::scaleModeChanged(Phonon::VideoWidget::ScaleMode scaleMode)
{
    Q_UNUSED(scaleMode);
}

void AbstractRenderer::movieSizeChanged(const QSize &size)
{
    Q_UNUSED(size);
}

}
} //namespace Phonon::Gstreamer

QT_END_NAMESPACE
#endif //QT_NO_PHONON_VIDEO

