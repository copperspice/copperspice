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

#ifndef PHONON_BACKENDINTERFACE_H
#define PHONON_BACKENDINTERFACE_H

#include "phonon_export.h"
#include "objectdescription.h"
#include <QtCore/QtGlobal>
#include <QtCore/QSet>

QT_BEGIN_NAMESPACE

class QVariant;

namespace Phonon
{

class BackendInterface
{
    public:
        /*
         * \internal
         *
         * Silence gcc's warning.
         */
        virtual ~BackendInterface() {}

        /*
         * Classes that the createObject function has to handle.
         */
        enum Class {
            /*
             * Request to return a %MediaObject object.
             */
            MediaObjectClass,
            /*
             * Request to return a %VolumeFaderEffect object.
             */
            VolumeFaderEffectClass,
            /*
             * Request to return a %AudioOutput object.
             */
            AudioOutputClass,
            /*
             * Request to return a %AudioDataOutput object.
             */
            AudioDataOutputClass,
            /*
             * Request to return a %Visualization object.
             */
            VisualizationClass,
            /*
             * Request to return a %VideoDataOutput object.
             */
            VideoDataOutputClass,
            /*
             * Request to return a %Effect object.
             *
             * Takes an additional int that specifies the effect Id.
             */
            EffectClass,
            /*
             * Request to return a %VideoWidget object.
             */
            VideoWidgetClass
        };

        /*
         * Returns a new instance of the requested class.
         *
         * \param c The requested class.
         * \param parent The parent object.
         * \param args Additional arguments (documented in \ref Class).
         */
        virtual QObject *createObject(Class c, QObject *parent, const QList<QVariant> &args = QList<QVariant>()) = 0;

        /*
         * Returns the unique identifiers for the devices/effects/codecs of the given \p type.
         *
         * \param type see \ref ObjectDescriptionType
         */
        virtual QList<int> objectDescriptionIndexes(ObjectDescriptionType type) const = 0;

        /*
         * Given a unique identifier that was returned from objectDescriptionIndexes this function
         * returns a hash mapping property names to values.
         *
         * The property "name" must always be present. All other properties are optional.
         *
         * List of possible properties:
         * \li \c \b name: The name of the device/effect/codec/...
         * \li \c \b description: A text explaining what this device/effect/codec/... is/can do
         * \li \c \b icon: An icon name (using the freedesktop naming scheme) or a QIcon for this
         * device/effect/codec/...
         * \li \c \b available: A bool telling whether the device is present or unplugged.
         *
         * \param type see \ref ObjectDescriptionType
         * \param index The unique identifier that is returned from objectDescriptionIndexes
         */
        virtual QHash<QByteArray, QVariant> objectDescriptionProperties(ObjectDescriptionType type, int index) const = 0;

        /*
         * When this function is called the nodes given in the parameter list should not lose any
         * signal data when connections are changed.
         */
        virtual bool startConnectionChange(QSet<QObject *>) = 0;

        /*
         * Defines a signal connection between the two given nodes.
         */
        virtual bool connectNodes(QObject *, QObject *) = 0;

        /*
         * Cuts a signal connection between the two given nodes.
         */
        virtual bool disconnectNodes(QObject *, QObject *) = 0;

        /*
         * When this function is called the nodes given in the parameter list may lose
         * signal data when a port is not connected.
         */
        virtual bool endConnectionChange(QSet<QObject *>) = 0;

        /*
         * gets all available mime types
         */
        virtual QStringList availableMimeTypes() const = 0;

};
} // namespace Phonon

CS_DECLARE_INTERFACE(Phonon::BackendInterface, "BackendInterface3.phonon.kde.org")

QT_END_NAMESPACE

#endif // PHONON_BACKENDINTERFACE_H
