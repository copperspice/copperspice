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

#ifndef PHONON_OBJECTDESCRIPTION_H
#define PHONON_OBJECTDESCRIPTION_H

#include "phonon_export.h"

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QSharedData>
#include <QtCore/QString>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    class ObjectDescriptionPrivate;

    enum ObjectDescriptionType
    {
        AudioOutputDeviceType,
        EffectType,
        AudioChannelType,
        SubtitleType,
        AudioCaptureDeviceType

        //VideoOutputDeviceType,
        //VideoCaptureDeviceType,
        //AudioCodecType,
        //VideoCodecType,
        //ContainerFormatType,
        //VisualizationType,
    };


class PHONON_EXPORT ObjectDescriptionData : public QSharedData
{
    public:
        /**
         * Returns \c true if this ObjectDescription describes the same
         * as \p otherDescription; otherwise returns \c false.
         */
        bool operator==(const ObjectDescriptionData &otherDescription) const;

        /**
         * Returns the name of the capture source.
         *
         * \return A string that should be presented to the user to
         * choose the capture source.
         */
        QString name() const;

        /**
         * Returns a description of the capture source. This text should
         * make clear what sound source this is, which is sometimes hard
         * to describe or understand from just the name.
         *
         * \return A string describing the capture source.
         */
        QString description() const;

        /**
         * Returns a named property.
         *
         * If the property is not set an invalid value is returned.
         *
         * \see propertyNames()
         */
        QVariant property(const char *name) const;

        /**
         * Returns all names that return valid data when property() is called.
         *
         * \see property()
         */
        QList<QByteArray> propertyNames() const;

        /**
         * Returns \c true if the Tuple is valid (index != -1); otherwise returns
         * \c false.
         */
        bool isValid() const;

        /**
         * A unique identifier for this device/. Used internally
         * to distinguish between the devices/.
         *
         * \return An integer that uniquely identifies every device/
         */
        int index() const;

        static ObjectDescriptionData *fromIndex(ObjectDescriptionType type, int index);

        ~ObjectDescriptionData();

        ObjectDescriptionData(ObjectDescriptionPrivate * = 0);
        ObjectDescriptionData(int index, const QHash<QByteArray, QVariant> &properties);

    protected:
        ObjectDescriptionPrivate *const d;

    private:
        ObjectDescriptionData &operator=(const ObjectDescriptionData &rhs);
};

template<ObjectDescriptionType T> class ObjectDescriptionModel;


template<ObjectDescriptionType T>
class ObjectDescription
{
    public:

        static inline ObjectDescription<T> fromIndex(int index) {
            return ObjectDescription<T>(QExplicitlySharedDataPointer<ObjectDescriptionData>(ObjectDescriptionData::fromIndex(T, index)));
        }

        inline bool operator==(const ObjectDescription &otherDescription) const {
            return *d == *otherDescription.d;
        }

        inline bool operator!=(const ObjectDescription &otherDescription) const {
            return !operator==(otherDescription);
        }

        inline QString name() const
            { return d->name(); }

        inline QString description() const
            { return d->description(); }

        inline QVariant property(const char *name) const
            { return d->property(name); }

        inline QList<QByteArray> propertyNames() const
            { return d->propertyNames(); }

        inline bool isValid() const
            { return d->isValid(); }

        inline int index() const
            { return d->index(); }

        ObjectDescription()
                  : d(new ObjectDescriptionData(0)) {}

        ObjectDescription(int index, const QHash<QByteArray, QVariant> &properties)
                  : d(new ObjectDescriptionData(index, properties)) {}

    protected:
        friend class ObjectDescriptionModel<T>;
        ObjectDescription(const QExplicitlySharedDataPointer<ObjectDescriptionData> &dd) : d(dd) {}
        QExplicitlySharedDataPointer<ObjectDescriptionData> d;
};

template<ObjectDescriptionType T>
inline QDebug operator<<(QDebug s, const ObjectDescription<T> &o)
{
    return s << o.name();
}

typedef ObjectDescription<AudioOutputDeviceType> AudioOutputDevice;

#ifndef QT_NO_PHONON_AUDIOCAPTURE
typedef ObjectDescription<AudioCaptureDeviceType> AudioCaptureDevice;
#endif

#ifndef QT_NO_PHONON_EFFECT
typedef ObjectDescription<EffectType> EffectDescription;
#endif

#ifndef QT_NO_PHONON_MEDIACONTROLLER
typedef ObjectDescription<AudioChannelType> AudioChannelDescription;
typedef ObjectDescription<SubtitleType> SubtitleDescription;
#endif

} //namespace Phonon

QT_END_NAMESPACE

Q_DECLARE_METATYPE(Phonon::AudioOutputDevice)
Q_DECLARE_METATYPE(QList<Phonon::AudioOutputDevice>)

#ifndef QT_NO_PHONON_AUDIOCAPTURE
Q_DECLARE_METATYPE(Phonon::AudioCaptureDevice)
Q_DECLARE_METATYPE(QList<Phonon::AudioCaptureDevice>)
#endif

#ifndef QT_NO_PHONON_EFFECT
Q_DECLARE_METATYPE(QList<Phonon::EffectDescription>)
Q_DECLARE_METATYPE(Phonon::EffectDescription)
#endif

#ifndef QT_NO_PHONON_MEDIACONTROLLER
Q_DECLARE_METATYPE(Phonon::AudioChannelDescription)
Q_DECLARE_METATYPE(Phonon::SubtitleDescription)
Q_DECLARE_METATYPE(QList<Phonon::AudioChannelDescription>)
Q_DECLARE_METATYPE(QList<Phonon::SubtitleDescription>)
#endif

#endif // PHONON_OBJECTDESCRIPTION_H
