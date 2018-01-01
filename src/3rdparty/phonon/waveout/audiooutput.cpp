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

#include "audiooutput.h"
#include "mediaobject.h"

#include <QtCore/QVector>

#include <cmath>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace WaveOut
    {
        AudioOutput::AudioOutput(Backend *, QObject *parent)
        {
            setParent(parent);
            m_volume = 0xffff;
        }

        AudioOutput::~AudioOutput()
        {
        }

        int AudioOutput::outputDevice() const
        {
            return 0;
        }

        void AudioOutput::setVolume(qreal newVolume)
        {
            m_volume = newVolume;
            emit volumeChanged(newVolume);
        }

        void AudioOutput::setCrossFadingProgress(short currentIndex, qreal progress)
        {
            Q_UNUSED(currentIndex);
            Q_UNUSED(progress);
        }

        bool AudioOutput::setOutputDevice(const AudioOutputDevice & newDevice)
        {
            return setOutputDevice(newDevice.index());
        }

        qreal AudioOutput::volume() const
        {
            return m_volume;
        }

        bool AudioOutput::setOutputDevice(int newDevice)
        {
         
            return (newDevice == 0);
        }

    }
}

QT_END_NAMESPACE

