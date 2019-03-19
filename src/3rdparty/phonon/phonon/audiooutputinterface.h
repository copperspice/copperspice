/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>
* Copyright (C) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef PHONON_AUDIOOUTPUTINTERFACE_H
#define PHONON_AUDIOOUTPUTINTERFACE_H

#include "phononnamespace.h"
#include "objectdescription.h"
#include "phonondefs.h"
#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE

namespace Phonon
{

class AudioOutputInterface40
{
    public:
        virtual ~AudioOutputInterface40() {}

        /**
         * Returns the current software volume.
         *
         * A value of 0.0 means muted, 1.0 means unchanged, 2.0 means double voltage (i.e. all
         * samples are multiplied by 2).
         */
        virtual qreal volume() const = 0;
        /**
         * Sets the new current software volume.
         *
         * A value of 0.0 means muted, 1.0 means unchanged, 2.0 means double voltage (i.e. all
         * samples are multiplied by 2).
         *
         * Every time the volume in the backend changes it should emit volumeChanged(qreal), also
         * inside this function.
         */
        virtual void setVolume(qreal) = 0;

        /**
         * Returns the index of the device that is used. The index is the number returned from
         * BackendInterface::objectDescriptionIndexes(AudioOutputDeviceType).
         */
        virtual int outputDevice() const = 0;
        /**
         * \deprecated
         *
         * Requests to change the current output device to the one identified by the passed index.
         *
         * The index is the number returned from
         * BackendInterface::objectDescriptionIndexes(AudioOutputDeviceType).
         *
         * \returns \c true if the requested device works and is used after this call.
         * \returns \c false if something failed and the device is not used after this call.
         */
        virtual bool setOutputDevice(int) = 0;
};

class AudioOutputInterface42 : public AudioOutputInterface40
{
    public:
        /**
         * Requests to change the current output device.
         *
         * \returns \c true if the requested device works and is used after this call.
         * \returns \c false if something failed and the device is not used after this call.
         */
        virtual bool setOutputDevice(const Phonon::AudioOutputDevice &) = 0;

        using AudioOutputInterface40::setOutputDevice;

        // Helper function for backends to get a list of (driver, handle) pairs for
        // AudioOutputDevice objects that are listed by the platform plugin.

        PHONON_EXPORT QList<QPair<QByteArray, QString> > deviceAccessListFor(const Phonon::AudioOutputDevice &) const;
};

} // namespace Phonon

#ifdef PHONON_BACKEND_VERSION_4_2
namespace Phonon { typedef AudioOutputInterface42 AudioOutputInterface; }
CS_DECLARE_INTERFACE(Phonon::AudioOutputInterface40, "AudioOutputInterface2.phonon.kde.org")
CS_DECLARE_INTERFACE(Phonon::AudioOutputInterface,   "3AudioOutputInterface.phonon.kde.org")
#else
namespace Phonon { typedef AudioOutputInterface40 AudioOutputInterface; }
CS_DECLARE_INTERFACE(Phonon::AudioOutputInterface,   "AudioOutputInterface2.phonon.kde.org")
CS_DECLARE_INTERFACE(Phonon::AudioOutputInterface42, "3AudioOutputInterface.phonon.kde.org")
#endif

QT_END_NAMESPACE

#endif // PHONON_AUDIOOUTPUTINTERFACE_H
