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
        QT7_CS_OBJECT(AudioOutputAudioPart)

        public:
            AudioOutputAudioPart();

            void setVolume(float volume);
            float volume();

            QT7_CS_SIGNAL_1(Public, void volumeChanged(qreal newVolume))
            QT7_CS_SIGNAL_2(volumeChanged,newVolume) 
            QT7_CS_SIGNAL_1(Public, void audioDeviceFailed())
            QT7_CS_SIGNAL_2(audioDeviceFailed) 

        protected:
            ComponentDescription getAudioNodeDescription() const override;
            void initializeAudioUnit() override; 
          
        private:
            friend class AudioOutput;
            qreal m_volume;
            AudioDeviceID m_audioDevice;
            void setAudioDevice(AudioDeviceID device);
    };

    class AudioOutput : public MediaNode, public AudioOutputInterface
    {
        QT7_CS_OBJECT(AudioOutput)
        CS_INTERFACES(Phonon::AudioOutputInterface)

        public:
            AudioOutput(QObject *parent = nullptr);
            ~AudioOutput();

            qreal volume() const override;
            void setVolume(qreal) override;
            int outputDevice() const override;
            bool setOutputDevice(int) override;
       
            QT7_CS_SIGNAL_1(Public, void volumeChanged(qreal newVolume))
            QT7_CS_SIGNAL_2(volumeChanged,newVolume) 
            QT7_CS_SIGNAL_1(Public, void audioDeviceFailed())
            QT7_CS_SIGNAL_2(audioDeviceFailed) 

        protected:
            void mediaNodeEvent(const MediaNodeEvent *event) override;

        private:
            AudioOutputAudioPart *m_audioOutput;
            int m_device;
            bool m_redirectToMovie;
    };

}} //namespace Phonon::QT7

QT_END_NAMESPACE
#endif
