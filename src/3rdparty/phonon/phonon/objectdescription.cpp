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

#include "objectdescription.h"
#include "objectdescription_p.h"

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include "factory_p.h"
#include "backendinterface.h"
#include "platformplugin.h"
#include "pulsesupport.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{

ObjectDescriptionData::ObjectDescriptionData(int index, const QHash<QByteArray, QVariant> &properties)
    : d(new ObjectDescriptionPrivate(index, properties))
{
}

ObjectDescriptionData::ObjectDescriptionData(ObjectDescriptionPrivate *dd)
    : d(dd)
{
}

ObjectDescriptionData::~ObjectDescriptionData()
{
    delete d;
}

bool ObjectDescriptionData::operator==(const ObjectDescriptionData &otherDescription) const
{
    if (!isValid()) {
        return !otherDescription.isValid();
    }
    if (!otherDescription.isValid()) {
        return false;
    }
    return *d == *otherDescription.d;
}

int ObjectDescriptionData::index() const
{
    if (! isValid()) {
        return -1;
    }

    return d->index;
}

QString ObjectDescriptionData::name() const
{
    if (! isValid()) {
        return QString();
    }
    return d->name;
}

QString ObjectDescriptionData::description() const
{
    if (! isValid()) {
        return QString();
    }
    return d->description;
}

QVariant ObjectDescriptionData::property(const char *name) const
{
    if (! isValid()) {
        return QVariant();
    }
    return d->properties.value(name);
}

QList<QByteArray> ObjectDescriptionData::propertyNames() const
{
    if (! isValid()) {
        return QList<QByteArray>();
    }
    return d->properties.keys();
}

bool ObjectDescriptionData::isValid() const
{
    return d != nullptr;
}

ObjectDescriptionData *ObjectDescriptionData::fromIndex(ObjectDescriptionType type, int index)
{
    bool is_audio_device = (AudioOutputDeviceType == type || AudioCaptureDeviceType == type);

    PulseSupport *pulse = PulseSupport::getInstance();

    if (is_audio_device && pulse->isActive()) {
        QList<int> indexes = pulse->objectDescriptionIndexes(type);

        if (indexes.contains(index)) {
            QHash<QByteArray, QVariant> properties = pulse->objectDescriptionProperties(type, index);
            return new ObjectDescriptionData(index, properties);
        }

    } else {
        BackendInterface *iface = qobject_cast<BackendInterface *>(Factory::backend());

        // prefer to get the ObjectDescriptionData from the platform plugin for audio devices
#ifndef QT_NO_PHONON_PLATFORMPLUGIN

        if (is_audio_device) {
            PlatformPlugin *platformPlugin = Factory::platformPlugin();

            if (platformPlugin) {
                QList<int> indexes = platformPlugin->objectDescriptionIndexes(type);

                if (indexes.contains(index)) {
                    QHash<QByteArray, QVariant> properties = platformPlugin->objectDescriptionProperties(type, index);
                    return new ObjectDescriptionData(index, properties);
                }
            }
        }
#endif

        if (iface) {
            QList<int> indexes = iface->objectDescriptionIndexes(type);

            if (indexes.contains(index)) {
                QHash<QByteArray, QVariant> properties = iface->objectDescriptionProperties(type, index);
                return new ObjectDescriptionData(index, properties);
            }
        }
    }

    return new ObjectDescriptionData(nullptr);       // invalid
}

} //namespace Phonon

QT_END_NAMESPACE
