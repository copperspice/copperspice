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

#ifndef QT7_AUDIOOUTPUT_H
#define QT7_AUDIOOUTPUT_H

#include <QtCore/QObject>
#include <phonon/audiooutputinterface.h>
#include <phonon/abstractaudiooutput.h>
#include "medianode.h"
#include "audionode.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class AudioOutputAudioPart : public QObject, AudioNode
    {
        CS_OBJECT(AudioOutputAudioPart)

        public:
            AudioOutputAudioPart();

            void setVolume(float volume);
            float volume();

        protected:
            ComponentDescription getAudioNodeDescription() const;
            void initializeAudioUnit();

        public:
            QT7_CS_SIGNAL_1(Public, void volumeChanged(qreal newVolume))
            QT7_CS_SIGNAL_2(volumeChanged,newVolume) 
            QT7_CS_SIGNAL_1(Public, void audioDeviceFailed())
            QT7_CS_SIGNAL_2(audioDeviceFailed) 

        private:
            friend class AudioOutput;
            qreal m_volume;
            AudioDeviceID m_audioDevice;
            void setAudioDevice(AudioDeviceID device);
    };

    class AudioOutput : public MediaNode, public AudioOutputInterface
    {
        CS_OBJECT(AudioOutput)
        CS_INTERFACES(Phonon::AudioOutputInterface)

        public:
            AudioOutput(QObject *parent = 0);
            ~AudioOutput();

            qreal volume() const;
            void setVolume(qreal);
            int outputDevice() const;
            bool setOutputDevice(int);

        public:
            QT7_CS_SIGNAL_1(Public, void volumeChanged(qreal newVolume))
            QT7_CS_SIGNAL_2(volumeChanged,newVolume) 
            QT7_CS_SIGNAL_1(Public, void audioDeviceFailed())
            QT7_CS_SIGNAL_2(audioDeviceFailed) 

        protected:
            void mediaNodeEvent(const MediaNodeEvent *event);

        private:
            AudioOutputAudioPart *m_audioOutput;
            int m_device;
            bool m_redirectToMovie;
    };

}} //namespace Phonon::QT7

QT_END_NAMESPACE
#endif
