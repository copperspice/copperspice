/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

/********************************************************
**  This file is part of the KDE project.
********************************************************/

#ifndef PHONON_AUDIOOUTPUT_H
#define PHONON_AUDIOOUTPUT_H

#include <QtCore/QFile>
#include <phonon/audiooutputinterface.h>

#include "backend.h"

struct IBaseFilter;
struct IBasicAudio;

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace WaveOut
    {
        class AudioOutput : public QObject, public Phonon::AudioOutputInterface
        {
            PHN_CS_OBJECT(AudioOutput)
            CS_INTERFACES(Phonon::AudioOutputInterface)

        public:
            AudioOutput(Backend *back, QObject *parent);
            ~AudioOutput();

            // Attributes Getters:
            qreal volume() const;
            int outputDevice() const;
            void setVolume(qreal newVolume);
            bool setOutputDevice(int newDevice);
            bool setOutputDevice(const AudioOutputDevice & newDevice);
            void setCrossFadingProgress(short currentIndex, qreal progress);
         
            PHN_CS_SIGNAL_1(Public, void audioDeviceFailed())
            PHN_CS_SIGNAL_2(audioDeviceFailed) 
            PHN_CS_SIGNAL_1(Public, void volumeChanged(qreal un_named_arg1))
            PHN_CS_SIGNAL_2(volumeChanged,un_named_arg1) 

        private:
            unsigned int m_volume;

        };
    }
}

QT_END_NAMESPACE

#endif // PHONON_AUDIOOUTPUT_H
