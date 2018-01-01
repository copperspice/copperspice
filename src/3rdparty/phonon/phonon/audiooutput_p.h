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

#ifndef PHONON_AUDIOOUTPUT_P_H
#define PHONON_AUDIOOUTPUT_P_H

#include "audiooutput.h"
#include "abstractaudiooutput_p.h"
#include "platform_p.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
class AudioOutputAdaptor;

class AudioOutputPrivate : public AbstractAudioOutputPrivate
{
    Q_DECLARE_PUBLIC(AudioOutput)
    PHONON_PRIVATECLASS
    public:
        inline static AudioOutputPrivate *cast(MediaNodePrivate *x)
        {
            if (x && x->castId == MediaNodePrivate::AudioOutputType) {
                return static_cast<AudioOutputPrivate *>(x);
            }
            return 0;
        }
        void init(Phonon::Category c);
        QString getStreamUuid();


    protected:
        AudioOutputPrivate(CastId castId = MediaNodePrivate::AudioOutputType)
            : AbstractAudioOutputPrivate(castId),
            name(Platform::applicationName()),
            volume(Platform::loadVolume(name)),
#ifndef QT_NO_DBUS
            adaptor(0),
#endif
            deviceBeforeFallback(-1),
            outputDeviceOverridden(false),
            forceMove(false),
            muted(false)
        {
        }

        ~AudioOutputPrivate();

        enum DeviceChangeType {
            FallbackChange,
            HigherPreferenceChange,
            SoundSystemChange
        };
        void handleAutomaticDeviceChange(const AudioOutputDevice &newDev, DeviceChangeType type);

        void _k_volumeChanged(qreal);
        void _k_revertFallback();
        void _k_audioDeviceFailed();
        void _k_deviceListChanged();
        void _k_deviceChanged(QString streamUuid, int deviceIndex);

    private:
        QString name;
        Phonon::AudioOutputDevice device;
        qreal volume;
        QString streamUuid;
#ifndef QT_NO_DBUS
        Phonon::AudioOutputAdaptor *adaptor;
#endif
        Category category;
        int deviceBeforeFallback;
        bool outputDeviceOverridden;
        bool forceMove;
        bool muted;
};
} //namespace Phonon

QT_END_NAMESPACE

#endif
