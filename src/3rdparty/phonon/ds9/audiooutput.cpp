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

#include "audiooutput.h"
#include "mediaobject.h"

#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {
        AudioOutput::AudioOutput(Backend *back, QObject *parent)
            : BackendNode(parent), m_currentIndex(0), m_crossfadeProgress(1.),
              m_device(-1), m_backend(back), m_volume(0.)
        {
        }

        AudioOutput::~AudioOutput()
        {
        }

        int AudioOutput::outputDevice() const
        {
            return m_device;
        }

        static const qreal log10over20 = qreal(0.1151292546497022842); // ln(10) / 20

        void AudioOutput::setVolume(qreal newVolume)
        {
            for(int i = 0; i < FILTER_COUNT; ++i) {
                ComPointer<IBasicAudio> audio(m_filters[i], IID_IBasicAudio);

                if (audio) {
                    const qreal currentVolume = newVolume * (m_currentIndex == i ? m_crossfadeProgress : 1-m_crossfadeProgress);
                    const qreal newDbVolume = (qMax(0., 1.-::log(::pow(currentVolume, -log10over20)))-1.) * 10000;
                    audio->put_Volume(qRound(newDbVolume));
                }
            }

            if (m_volume != newVolume) {
                m_volume = newVolume;
                emit volumeChanged(newVolume);
            }
        }

        void AudioOutput::setCrossFadingProgress(short currentIndex, qreal progress)
        {
            m_crossfadeProgress = progress;
            m_currentIndex = currentIndex;
            setVolume(m_volume);
        }

        bool AudioOutput::setOutputDevice(const AudioOutputDevice & newDevice)
        {
            //stub implementation
            return setOutputDevice(newDevice.index());
        }

        qreal AudioOutput::volume() const
        {
            return m_volume;
        }

        bool AudioOutput::setOutputDevice(int newDevice)
        {

            if (newDevice == m_device) {
                return true;
            }

            // free the previous one if it was already set
            for(int i = 0; i < FILTER_COUNT; ++i) {
                const Filter &oldFilter = m_filters[i];

                Filter newFilter = m_backend->getAudioOutputFilter(newDevice);

                if (m_mediaObject && oldFilter && newFilter) {
                    m_mediaObject->switchFilters(i, oldFilter, newFilter);
                }

                m_filters[i] = newFilter;
            }

            m_device = newDevice;
            setVolume(m_volume);
            return true;
        }
    }
}

QT_END_NAMESPACE
