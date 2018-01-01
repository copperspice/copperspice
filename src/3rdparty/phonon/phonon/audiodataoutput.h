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

#ifndef Phonon_AUDIODATAOUTPUT_H
#define Phonon_AUDIODATAOUTPUT_H

#include "phonon_export.h"
#include "abstractaudiooutput.h"
#include "phonondefs.h"
#include <QtCore/qcontainerfwd.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    class AudioDataOutputPrivate;

    class PHONON_EXPORT AudioDataOutput : public AbstractAudioOutput
    {
        PHN_CS_OBJECT(AudioDataOutput)
        K_DECLARE_PRIVATE(AudioDataOutput)

        PHN_CS_ENUM(Channel)

        PHN_CS_PROPERTY_READ(dataSize, dataSize)
        PHN_CS_PROPERTY_WRITE(dataSize, setDataSize)

        PHONON_HEIR(AudioDataOutput)

        public:

            enum Channel
            {
                LeftChannel,
                RightChannel,
                CenterChannel,
                LeftSurroundChannel,
                RightSurroundChannel,
                SubwooferChannel
            };

            int dataSize() const;
            int sampleRate() const;

            PHN_CS_SLOT_1(Public, void setDataSize(int size))
            PHN_CS_SLOT_2(setDataSize)

            PHN_CS_SIGNAL_1(Public, void dataReady(const QMap <Phonon::AudioDataOutput::Channel, QVector <qint16>> & data))
            PHN_CS_SIGNAL_2(dataReady,data)

            PHN_CS_SIGNAL_1(Public, void endOfMedia(int remainingSamples))
            PHN_CS_SIGNAL_2(endOfMedia,remainingSamples)
    };
} // namespace Phonon

QT_END_NAMESPACE

#endif
