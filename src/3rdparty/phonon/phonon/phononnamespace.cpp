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

#include "phononnamespace.h"
#include "phononnamespace_p.h"
#include "phonondefs_p.h"

#include "factory_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    /*!
        Returns the version number of Phonon at run-time as a string (for
        example, "4.0.0"). This may be a different version than the
        version the application was compiled against.

        \sa PHONON_VERSION_STR
    */
    const char *phononVersion()
    {
        return PHONON_VERSION_STR;
    }

    QString categoryToString(Category c)
    {
        switch(c)
        {
        case Phonon::NoCategory:
            break;
        case Phonon::NotificationCategory:
            return QCoreApplication::translate("Phonon::", "Notifications");
        case Phonon::MusicCategory:
            return QCoreApplication::translate("Phonon::", "Music");
        case Phonon::VideoCategory:
            return QCoreApplication::translate("Phonon::", "Video");
        case Phonon::CommunicationCategory:
            return QCoreApplication::translate("Phonon::", "Communication");
        case Phonon::GameCategory:
            return QCoreApplication::translate("Phonon::", "Games");
        case Phonon::AccessibilityCategory:
            return QCoreApplication::translate("Phonon::", "Accessibility");
        }
        return QString();
    }
}

static int registerPhononMetaTypes()
{
    qRegisterMetaType<Phonon::State>();
    qRegisterMetaType<Phonon::ErrorType>();
    qRegisterMetaType<Phonon::Category>();

    // need those for QSettings
    qRegisterMetaType<QList<int> >();
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");

    return 0; // something
}

#ifdef Q_CONSTRUCTOR_FUNCTION
Q_CONSTRUCTOR_FUNCTION(registerPhononMetaTypes)
#else
static const int _Phonon_registerMetaTypes = registerPhononMetaTypes();
#endif

QT_END_NAMESPACE

