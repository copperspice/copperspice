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

#ifndef Phonon_AUDIOOUTPUT_H
#define Phonon_AUDIOOUTPUT_H

#include "phonon_export.h"
#include "abstractaudiooutput.h"
#include "phonondefs.h"
#include "phononnamespace.h"
#include "objectdescription.h"

QT_BEGIN_NAMESPACE

class QString;
class AudioOutputAdaptor;

namespace Phonon
{
    class AudioOutputPrivate;
   
    class PHONON_EXPORT AudioOutput : public AbstractAudioOutput
    {
        friend class FactoryPrivate;
        friend class ::AudioOutputAdaptor;

        PHN_CS_OBJECT(AudioOutput)
        K_DECLARE_PRIVATE(AudioOutput)
      
        PHN_CS_PROPERTY_READ(name, name)
        PHN_CS_PROPERTY_WRITE(name, setName)
      
        PHN_CS_PROPERTY_READ(volume, volume)
        PHN_CS_PROPERTY_WRITE(volume, setVolume)
        PHN_CS_PROPERTY_NOTIFY(volume, volumeChanged)

        PHN_CS_PROPERTY_READ(volumeDecibel, volumeDecibel)
        PHN_CS_PROPERTY_WRITE(volumeDecibel, setVolumeDecibel)

        PHN_CS_PROPERTY_READ(outputDevice, outputDevice)
        PHN_CS_PROPERTY_WRITE(outputDevice, cs_setOutputDevice)

        PHN_CS_PROPERTY_READ(muted, isMuted)
        PHN_CS_PROPERTY_WRITE(muted, setMuted)
        PHN_CS_PROPERTY_NOTIFY(muted, mutedChanged)

        public:

            // wrapper for overloaded method
            inline void cs_setOutputDevice(const Phonon::AudioOutputDevice & newAudioOutputDevice);               
            
            explicit AudioOutput(Phonon::Category category, QObject *parent = nullptr);
            explicit AudioOutput(QObject *parent = nullptr);

            QString name() const;
            qreal volume() const;
            qreal volumeDecibel() const;
           
            Phonon::Category category() const;
            AudioOutputDevice outputDevice() const;
            bool isMuted() const;
        
            PHN_CS_SLOT_1(Public, void setName(const QString & newName))
            PHN_CS_SLOT_2(setName) 

            PHN_CS_SLOT_1(Public, void setVolume(qreal newVolume))
            PHN_CS_SLOT_2(setVolume) 

            PHN_CS_SLOT_1(Public, void setVolumeDecibel(qreal newVolumeDecibel))
            PHN_CS_SLOT_2(setVolumeDecibel) 

            PHN_CS_SLOT_1(Public, bool setOutputDevice(const Phonon::AudioOutputDevice & newAudioOutputDevice))
            PHN_CS_SLOT_2(setOutputDevice) 

            PHN_CS_SLOT_1(Public, void setMuted(bool mute))
            PHN_CS_SLOT_2(setMuted)        
           
            PHN_CS_SIGNAL_1(Public, void volumeChanged(qreal newVolume))
            PHN_CS_SIGNAL_2(volumeChanged,newVolume) 
 
            PHN_CS_SIGNAL_1(Public, void mutedChanged(bool un_named_arg1))
            PHN_CS_SIGNAL_2(mutedChanged,un_named_arg1) 

            PHN_CS_SIGNAL_1(Public, void outputDeviceChanged(const Phonon::AudioOutputDevice & newAudioOutputDevice))
            PHN_CS_SIGNAL_2(outputDeviceChanged,newAudioOutputDevice) 

        private:
            PHN_CS_SLOT_1(Private, void _k_volumeChanged(qreal un_named_arg1))
            PHN_CS_SLOT_2(_k_volumeChanged)

            PHN_CS_SLOT_1(Private, void _k_revertFallback())
            PHN_CS_SLOT_2(_k_revertFallback)

            PHN_CS_SLOT_1(Private, void _k_audioDeviceFailed())
            PHN_CS_SLOT_2(_k_audioDeviceFailed)

            PHN_CS_SLOT_1(Private, void _k_deviceListChanged())
            PHN_CS_SLOT_2(_k_deviceListChanged)

            PHN_CS_SLOT_1(Private, void _k_deviceChanged(QString streamUuid,int device))
            PHN_CS_SLOT_2(_k_deviceChanged)        
    };


void AudioOutput::cs_setOutputDevice(const Phonon::AudioOutputDevice & newAudioOutputDevice) 
{   
   setOutputDevice(newAudioOutputDevice); 
}


} //namespace Phonon

QT_END_NAMESPACE

#endif
