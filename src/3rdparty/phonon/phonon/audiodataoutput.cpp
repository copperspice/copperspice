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

#include "audiodataoutput.h"
#include "audiodataoutput_p.h"
#include "factory_p.h"

#define PHONON_CLASSNAME AudioDataOutput

QT_BEGIN_NAMESPACE

namespace Phonon
{

PHONON_HEIR_IMPL(AbstractAudioOutput)

PHONON_GETTER(int, dataSize, d->dataSize)
PHONON_GETTER(int, sampleRate, -1)
PHONON_SETTER(setDataSize, dataSize, int)

bool AudioDataOutputPrivate::aboutToDeleteBackendObject()
{
    Q_ASSERT(m_backendObject);
    pBACKEND_GET(int, dataSize, "dataSize");

    return AbstractAudioOutputPrivate::aboutToDeleteBackendObject();
}

void AudioDataOutputPrivate::setupBackendObject()
{
    Q_Q(AudioDataOutput);
    Q_ASSERT(m_backendObject);
    AbstractAudioOutputPrivate::setupBackendObject();

    // set up attributes
    pBACKEND_CALL1("setDataSize", int, dataSize);

    qRegisterMetaType<QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> > >("QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >");

    QObject::connect(m_backendObject,
            SIGNAL(dataReady(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> > &)),
            q, SLOT(dataReady(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> > &)));

    QObject::connect(m_backendObject, SIGNAL(endOfMedia(int)), q, SLOT(endOfMedia(int)));
}

} // namespace Phonon

QT_END_NAMESPACE

#undef PHONON_CLASSNAME
