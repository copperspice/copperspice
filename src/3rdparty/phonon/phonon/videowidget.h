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

#ifndef Phonon_VIDEOWIDGET_H
#define Phonon_VIDEOWIDGET_H

#include "phonon_export.h"
#include "phonondefs.h"
#include "abstractvideooutput.h"
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QString;

#ifndef QT_NO_PHONON_VIDEO

namespace Phonon
{
class AbstractVideoOutput;
    class VideoWidgetPrivate;
    class PHONON_EXPORT VideoWidget : public QWidget, public Phonon::AbstractVideoOutput
    {
        PHN_CS_OBJECT(VideoWidget)
        K_DECLARE_PRIVATE(VideoWidget)
                
        PHN_CS_ENUM(AspectRatio)
        PHN_CS_ENUM(ScaleMode)
        
        PHN_CS_PROPERTY_READ(fullScreen, isFullScreen)
        PHN_CS_PROPERTY_WRITE(fullScreen, setFullScreen)
       
        PHN_CS_PROPERTY_READ(aspectRatio, aspectRatio)
        PHN_CS_PROPERTY_WRITE(aspectRatio, setAspectRatio)
       
        PHN_CS_PROPERTY_READ(scaleMode, scaleMode)
        PHN_CS_PROPERTY_WRITE(scaleMode, setScaleMode)
       
        PHN_CS_PROPERTY_READ(brightness, brightness)
        PHN_CS_PROPERTY_WRITE(brightness, setBrightness)
       
        PHN_CS_PROPERTY_READ(contrast, contrast)
        PHN_CS_PROPERTY_WRITE(contrast, setContrast)
       
        PHN_CS_PROPERTY_READ(hue, hue)
        PHN_CS_PROPERTY_WRITE(hue, setHue)
       
        PHN_CS_PROPERTY_READ(saturation, saturation)
        PHN_CS_PROPERTY_WRITE(saturation, setSaturation)

        public:
           
            enum AspectRatio
            {              
                AspectRatioAuto = 0,               
                AspectRatioWidget = 1,                
                AspectRatio4_3 = 2,                
                AspectRatio16_9 = 3
            };

            enum ScaleMode {
                FitInView = 0,
                ScaleAndCrop = 1
            };
           
            VideoWidget(QWidget *parent = nullptr);

            AspectRatio aspectRatio() const;
            ScaleMode scaleMode() const;

            qreal brightness() const;
            qreal contrast() const;
            qreal hue() const;
            qreal saturation() const;
            QImage snapshot() const;

            //TODO: bar colors property
        public :
            PHN_CS_SLOT_1(Public, void setFullScreen(bool fullscreen))
            PHN_CS_SLOT_2(setFullScreen) 

            PHN_CS_SLOT_1(Public, void exitFullScreen())
            PHN_CS_SLOT_2(exitFullScreen) 
           
            PHN_CS_SLOT_1(Public, void enterFullScreen())
            PHN_CS_SLOT_2(enterFullScreen) 

            PHN_CS_SLOT_1(Public, void setAspectRatio(AspectRatio un_named_arg1))
            PHN_CS_SLOT_2(setAspectRatio) 

            PHN_CS_SLOT_1(Public, void setScaleMode(ScaleMode un_named_arg1))
            PHN_CS_SLOT_2(setScaleMode) 

            PHN_CS_SLOT_1(Public, void setBrightness(qreal value))
            PHN_CS_SLOT_2(setBrightness) 
            PHN_CS_SLOT_1(Public, void setContrast(qreal value))
            PHN_CS_SLOT_2(setContrast) 
            PHN_CS_SLOT_1(Public, void setHue(qreal value))
            PHN_CS_SLOT_2(setHue) 
            PHN_CS_SLOT_1(Public, void setSaturation(qreal value))
            PHN_CS_SLOT_2(setSaturation) 

        protected:
            VideoWidget(VideoWidgetPrivate &d, QWidget *parent);

            void mouseMoveEvent(QMouseEvent *) override;
            bool event(QEvent *) override;
    };

} //namespace Phonon

#endif //QT_NO_PHONON_VIDEO

QT_END_NAMESPACE

#endif
