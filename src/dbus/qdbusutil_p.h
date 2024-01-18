/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

#ifndef QDBUSUTIL_H
#define QDBUSUTIL_H

#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>
#include <QtDBus/qdbusmacros.h>
#include <QtDBus/qdbuserror.h>

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

namespace QDBusUtil
{
    Q_DBUS_EXPORT bool isValidInterfaceName(const QString &ifaceName);

    Q_DBUS_EXPORT bool isValidUniqueConnectionName(const QString &busName);

    Q_DBUS_EXPORT bool isValidBusName(const QString &busName);

    Q_DBUS_EXPORT bool isValidMemberName(const QString &memberName);

    Q_DBUS_EXPORT bool isValidErrorName(const QString &errorName);

    Q_DBUS_EXPORT bool isValidPartOfObjectPath(const QString &path);

    Q_DBUS_EXPORT bool isValidObjectPath(const QString &path);

    Q_DBUS_EXPORT bool isValidFixedType(int c);

    Q_DBUS_EXPORT bool isValidBasicType(int c);

    Q_DBUS_EXPORT bool isValidSignature(const QString &signature);

    Q_DBUS_EXPORT bool isValidSingleSignature(const QString &signature);

    Q_DBUS_EXPORT QString argumentToString(const QVariant &variant);

    enum AllowEmptyFlag {
        EmptyAllowed,
        EmptyNotAllowed
    };

    inline bool checkInterfaceName(const QString &name, AllowEmptyFlag empty, QDBusError *error)
    {
        if (name.isEmpty()) {
            if (empty == EmptyAllowed) return true;
            *error = QDBusError(QDBusError::InvalidInterface, QLatin1String("Interface name cannot be empty"));
            return false;
        }
        if (isValidInterfaceName(name)) return true;
        *error = QDBusError(QDBusError::InvalidInterface, QString::fromLatin1("Invalid interface class: %1").arg(name));
        return false;
    }

    inline bool checkBusName(const QString &name, AllowEmptyFlag empty, QDBusError *error)
    {
        if (name.isEmpty()) {
            if (empty == EmptyAllowed) return true;
            *error = QDBusError(QDBusError::InvalidService, QLatin1String("Service name cannot be empty"));
            return false;
        }
        if (isValidBusName(name)) return true;
        *error = QDBusError(QDBusError::InvalidService, QString::fromLatin1("Invalid service name: %1").arg(name));
        return false;
    }

    inline bool checkObjectPath(const QString &path, AllowEmptyFlag empty, QDBusError *error)
    {
        if (path.isEmpty()) {
            if (empty == EmptyAllowed) return true;
            *error = QDBusError(QDBusError::InvalidObjectPath, QLatin1String("Object path cannot be empty"));
            return false;
        }
        if (isValidObjectPath(path)) return true;
        *error = QDBusError(QDBusError::InvalidObjectPath, QString::fromLatin1("Invalid object path: %1").arg(path));
        return false;
    }

    inline bool checkMemberName(const QString &name, AllowEmptyFlag empty, QDBusError *error, const char *nameType = 0)
    {
        if (!nameType) nameType = "member";
        if (name.isEmpty()) {
            if (empty == EmptyAllowed) return true;
            *error = QDBusError(QDBusError::InvalidMember, QLatin1String(nameType) + QLatin1String(" name cannot be empty"));
            return false;
        }
        if (isValidMemberName(name)) return true;
        *error = QDBusError(QDBusError::InvalidMember, QString::fromLatin1("Invalid %1 name: %2")
                            .arg(QString::fromLatin1(nameType), name));
        return false;
    }

    inline bool checkErrorName(const QString &name, AllowEmptyFlag empty, QDBusError *error)
    {
        if (name.isEmpty()) {
            if (empty == EmptyAllowed) return true;
            *error = QDBusError(QDBusError::InvalidInterface, QLatin1String("Error name cannot be empty"));
            return false;
        }
        if (isValidErrorName(name)) return true;
        *error = QDBusError(QDBusError::InvalidInterface, QString::fromLatin1("Invalid error name: %1").arg(name));
        return false;
    }
}

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif
