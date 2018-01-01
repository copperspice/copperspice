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

#ifndef PHONON_VOLUMEFADEREFFECT_H
#define PHONON_VOLUMEFADEREFFECT_H

#include "phonon_export.h"
#include "effect.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VOLUMEFADEREFFECT

namespace Phonon
{
    class VolumeFaderEffectPrivate;
   
    class PHONON_EXPORT VolumeFaderEffect : public Effect
    {
        PHN_CS_OBJECT(VolumeFaderEffect)
        K_DECLARE_PRIVATE(VolumeFaderEffect)
        PHONON_HEIR(VolumeFaderEffect)

        PHN_CS_ENUM(FadeCurve)
      
        PHN_CS_PROPERTY_READ(volume, volume)
        PHN_CS_PROPERTY_WRITE(volume, setVolume)
       
        PHN_CS_PROPERTY_READ(volumeDecibel, volumeDecibel)
        PHN_CS_PROPERTY_WRITE(volumeDecibel, setVolumeDecibel)
       
        PHN_CS_PROPERTY_READ(fadeCurve, fadeCurve)
        PHN_CS_PROPERTY_WRITE(fadeCurve, setFadeCurve)

        public:
            /**
             * Determines the curve of the volume change.
             */
            enum FadeCurve {                
                Fade3Decibel,               
                Fade6Decibel,               
                Fade9Decibel,               
                Fade12Decibel
            };

            float volume() const;
            double volumeDecibel() const;
            FadeCurve fadeCurve() const;
        
            PHN_CS_SLOT_1(Public, void fadeIn(int fadeTime))
            PHN_CS_SLOT_2(fadeIn) 
         
            PHN_CS_SLOT_1(Public, void fadeOut(int fadeTime))
            PHN_CS_SLOT_2(fadeOut) 

            PHN_CS_SLOT_1(Public, void setVolume(float volume))
            PHN_CS_SLOT_2(setVolume) 
            
            PHN_CS_SLOT_1(Public, void setVolumeDecibel(double volumeDecibel))
            PHN_CS_SLOT_2(setVolumeDecibel) 

            PHN_CS_SLOT_1(Public, void setFadeCurve(FadeCurve curve))
            PHN_CS_SLOT_2(setFadeCurve) 

            PHN_CS_SLOT_1(Public, void fadeTo(float volume,int fadeTime))
            PHN_CS_SLOT_2(fadeTo) 
    };

} //namespace Phonon

#endif //QT_NO_PHONON_VOLUMEFADEREFFECT

QT_END_NAMESPACE

#endif