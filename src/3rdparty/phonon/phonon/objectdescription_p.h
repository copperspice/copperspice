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

#ifndef PHONON_OBJECTDESCRIPTION_P_H
#define PHONON_OBJECTDESCRIPTION_P_H

#include <QtCore/QByteRef>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include "phononnamespace_p.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
    class ObjectDescriptionPrivate
    {
        public:
            ObjectDescriptionPrivate(int _index, const QHash<QByteArray, QVariant> &_properties)
                : index(_index),
                name(_properties["name"].toString()),
                description(_properties["description"].toString()),
                properties(_properties)
            {
            }

            bool operator==(const ObjectDescriptionPrivate &rhs) const
            {
                if (index == rhs.index && (name != rhs.name || description != rhs.description))
                    pError() << "Same index (" << index <<
                        "), but different name/description. This is a bug in the Phonon backend.";
                return index == rhs.index;// && name == rhs.name && description == rhs.description;
            }

            int index;
            QString name, description;
            QHash<QByteArray, QVariant> properties;
    };
} // namespace Phonon

QT_END_NAMESPACE

#endif // PHONON_OBJECTDESCRIPTION_P_H
