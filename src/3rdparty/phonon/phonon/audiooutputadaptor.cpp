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

#include "audiooutputadaptor_p.h"
#include "audiooutput.h"
#include <QtCore/QArgument>
#include <QtCore/QByteRef>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include "phononnamespace_p.h"
#include "objectdescription.h"

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

namespace Phonon
{

AudioOutputAdaptor::AudioOutputAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

AudioOutputAdaptor::~AudioOutputAdaptor()
{
    // destructor
}

double AudioOutputAdaptor::volume() const
{
    // get the value of property volume
    return qvariant_cast<qreal>(parent()->property("volume"));
}

void AudioOutputAdaptor::setVolume(double value)
{
    // set the value of property volume
    parent()->setProperty("volume", QVariant::fromValue(static_cast<qreal>(value)));
}

bool AudioOutputAdaptor::muted() const
{
    return parent()->property("muted").toBool();
}

void AudioOutputAdaptor::setMuted(bool value)
{
    parent()->setProperty("muted", value);
}

QString AudioOutputAdaptor::category()
{
    // handle method call org.kde.Phonon.AudioOutput.category
    return Phonon::categoryToString(static_cast<Phonon::AudioOutput *>(parent())->category());
}

QString AudioOutputAdaptor::name()
{
    // handle method call org.kde.Phonon.AudioOutput.name
    QString name;
    //QMetaObject::invokeMethod(parent(), "name", Q_RETURN_ARG(QString, name));

    // Alternative:
    name = static_cast<Phonon::AudioOutput *>(parent())->name();
    return name;
}

int AudioOutputAdaptor::outputDeviceIndex() const
{
    return static_cast<Phonon::AudioOutput *>(parent())->outputDevice().index();
}

void AudioOutputAdaptor::setOutputDeviceIndex(int newAudioOutputDeviceIndex)
{
    static_cast<Phonon::AudioOutput *>(parent())
        ->setOutputDevice(Phonon::AudioOutputDevice::fromIndex(newAudioOutputDeviceIndex));
}

} // namespace Phonon

QT_END_NAMESPACE

#endif
