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

#ifndef PHONON_PULSESUPPORT_H
#define PHONON_PULSESUPPORT_H

#include "phonon_export.h"
#include "phononnamespace.h"
#include "objectdescription.h"
#include <QtCore/QtGlobal>
#include <QtCore/QSet>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    class PHONON_EXPORT PulseSupport : public QObject
    {
        PHN_CS_OBJECT(PulseSupport)

        public:
            static PulseSupport* getInstance();
            static void shutdown();

            bool isActive();
            void enable(bool enabled = true);

            QList<int> objectDescriptionIndexes(ObjectDescriptionType type) const;
            QHash<QByteArray, QVariant> objectDescriptionProperties(ObjectDescriptionType type, int index) const;
            QList<int> objectIndexesByCategory(ObjectDescriptionType type, Category category) const;

            void setOutputDevicePriorityForCategory(Category category, QList<int> order);
            void setCaptureDevicePriorityForCategory(Category category, QList<int> order);

            void setStreamPropList(Category category, QString streamUuid);
            void emitObjectDescriptionChanged(ObjectDescriptionType);
            void emitUsingDevice(QString streamUuid, int device);

            bool setOutputDevice(QString streamUuid, int device);
            bool setCaptureDevice(QString streamUuid, int device);
            void clearStreamCache(QString streamUuid);
        
            PHN_CS_SIGNAL_1(Public, void objectDescriptionChanged(ObjectDescriptionType un_named_arg1))
            PHN_CS_SIGNAL_2(objectDescriptionChanged,un_named_arg1) 
            PHN_CS_SIGNAL_1(Public, void usingDevice(QString streamUuid,int device))
            PHN_CS_SIGNAL_2(usingDevice,streamUuid,device) 

        private:
            PulseSupport();
            ~PulseSupport();

            bool mEnabled;
    };
} // namespace Phonon

QT_END_NAMESPACE

#endif // PHONON_PULSESUPPORT_H
