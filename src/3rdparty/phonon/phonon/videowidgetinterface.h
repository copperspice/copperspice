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
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#ifndef PHONON_VIDEOWIDGETINTERFACE_H
#define PHONON_VIDEOWIDGETINTERFACE_H

#include "videowidget.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VIDEO

namespace Phonon
{
class VideoWidgetInterface
{
    public:
        virtual ~VideoWidgetInterface() {}

        virtual Phonon::VideoWidget::AspectRatio aspectRatio() const = 0;
        virtual void setAspectRatio(Phonon::VideoWidget::AspectRatio) = 0;
        virtual qreal brightness() const = 0;
        virtual void setBrightness(qreal) = 0;
        virtual Phonon::VideoWidget::ScaleMode scaleMode() const = 0;
        virtual void setScaleMode(Phonon::VideoWidget::ScaleMode) = 0;
        virtual qreal contrast() const = 0;
        virtual void setContrast(qreal) = 0;
        virtual qreal hue() const = 0;
        virtual void setHue(qreal) = 0;
        virtual qreal saturation() const = 0;
        virtual void setSaturation(qreal) = 0;
        virtual QWidget *widget() = 0;
//X        virtual int overlayCapabilities() const = 0;
//X        virtual bool createOverlay(QWidget *widget, int type) = 0;
};

class VideoWidgetInterface44 : public VideoWidgetInterface
{
    public:
        virtual QImage snapshot() const = 0;
};
}

#ifdef PHONON_BACKEND_VERSION_4_4
namespace Phonon { typedef VideoWidgetInterface44 VideoWidgetInterfaceLatest; }
#else
namespace Phonon { typedef VideoWidgetInterface VideoWidgetInterfaceLatest; }
#endif

CS_DECLARE_INTERFACE(Phonon::VideoWidgetInterface44, "VideoWidgetInterface44.phonon.kde.org")
CS_DECLARE_INTERFACE(Phonon::VideoWidgetInterface, "VideoWidgetInterface3.phonon.kde.org")

#endif //QT_NO_PHONON_VIDEO

QT_END_NAMESPACE

#endif // PHONON_VIDEOWIDGETINTERFACE_H
