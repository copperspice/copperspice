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

#include "qdbuserror.h"

#include <qdebug.h>
#include <qvarlengtharray.h>
#include "qdbus_symbols_p.h"
#include "qdbusmessage.h"
#include "qdbusmessage_p.h"

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

/*
 * Use the following Perl script to generate the error string index list:
===== PERL SCRIPT ====
print "static const char errorMessages_string[] =\n";
$counter = 0;
$i = 0;
while (<STDIN>) {
    chomp;
    print "    \"$_\\0\"\n";
    $sizes[$i++] = $counter;
    $counter += 1 + length $_;
}
print "    \"\\0\";\n\nstatic const int errorMessages_indices[] = {\n    ";
for ($j = 0; $j < $i; ++$j) {
    printf "$sizes[$j], ";
}
print "0\n};\n";
===== PERL SCRIPT ====
 
 * The input data is as follows:
other
org.freedesktop.DBus.Error.Failed
org.freedesktop.DBus.Error.NoMemory
org.freedesktop.DBus.Error.ServiceUnknown
org.freedesktop.DBus.Error.NoReply
org.freedesktop.DBus.Error.BadAddress
org.freedesktop.DBus.Error.NotSupported
org.freedesktop.DBus.Error.LimitsExceeded
org.freedesktop.DBus.Error.AccessDenied
org.freedesktop.DBus.Error.NoServer
org.freedesktop.DBus.Error.Timeout
org.freedesktop.DBus.Error.NoNetwork
org.freedesktop.DBus.Error.AddressInUse
org.freedesktop.DBus.Error.Disconnected
org.freedesktop.DBus.Error.InvalidArgs
org.freedesktop.DBus.Error.UnknownMethod
org.freedesktop.DBus.Error.TimedOut
org.freedesktop.DBus.Error.InvalidSignature
org.freedesktop.DBus.Error.UnknownInterface
com.trolltech.QtDBus.Error.InternalError
org.freedesktop.DBus.Error.UnknownObject
com.trolltech.QtDBus.Error.InvalidService
com.trolltech.QtDBus.Error.InvalidObjectPath
com.trolltech.QtDBus.Error.InvalidInterface
com.trolltech.QtDBus.Error.InvalidMember
*/

// in the same order as KnownErrors!
static const char errorMessages_string[] =
    "other\0"
    "org.freedesktop.DBus.Error.Failed\0"
    "org.freedesktop.DBus.Error.NoMemory\0"
    "org.freedesktop.DBus.Error.ServiceUnknown\0"
    "org.freedesktop.DBus.Error.NoReply\0"
    "org.freedesktop.DBus.Error.BadAddress\0"
    "org.freedesktop.DBus.Error.NotSupported\0"
    "org.freedesktop.DBus.Error.LimitsExceeded\0"
    "org.freedesktop.DBus.Error.AccessDenied\0"
    "org.freedesktop.DBus.Error.NoServer\0"
    "org.freedesktop.DBus.Error.Timeout\0"
    "org.freedesktop.DBus.Error.NoNetwork\0"
    "org.freedesktop.DBus.Error.AddressInUse\0"
    "org.freedesktop.DBus.Error.Disconnected\0"
    "org.freedesktop.DBus.Error.InvalidArgs\0"
    "org.freedesktop.DBus.Error.UnknownMethod\0"
    "org.freedesktop.DBus.Error.TimedOut\0"
    "org.freedesktop.DBus.Error.InvalidSignature\0"
    "org.freedesktop.DBus.Error.UnknownInterface\0"
    "com.trolltech.QtDBus.Error.InternalError\0"
    "org.freedesktop.DBus.Error.UnknownObject\0"
    "com.trolltech.QtDBus.Error.InvalidService\0"
    "com.trolltech.QtDBus.Error.InvalidObjectPath\0"
    "com.trolltech.QtDBus.Error.InvalidInterface\0"
    "com.trolltech.QtDBus.Error.InvalidMember\0"
    "\0";

static const int errorMessages_indices[] = {
       0,    6,   40,   76,  118,  153,  191,  231,
     273,  313,  349,  384,  421,  461,  501,  540,
     581,  617,  661,  705,  746,  787,  829,  874,
     918,    0
};

static const int errorMessages_count = sizeof errorMessages_indices /
                                       sizeof errorMessages_indices[0];

static inline const char *get(QDBusError::ErrorType code)
{
    int intcode = qBound(0, int(code) - int(QDBusError::Other), errorMessages_count);
    return errorMessages_string + errorMessages_indices[intcode];
}

static inline QDBusError::ErrorType get(const char *name)
{
    if (!name || !*name)
        return QDBusError::NoError;
    for (int i = 0; i < errorMessages_count; ++i)
        if (strcmp(name, errorMessages_string + errorMessages_indices[i]) == 0)
            return QDBusError::ErrorType(i + int(QDBusError::Other));
    return QDBusError::Other;
}

/*!
    \class QDBusError
    \inmodule QtDBus
    \since 4.2

    \brief The QDBusError class represents an error received from the
    D-Bus bus or from remote applications found in the bus.

    When dealing with the D-Bus bus service or with remote
    applications over D-Bus, a number of error conditions can
    happen. This error conditions are sometimes signalled by a
    returned error value or by a QDBusError.

    C++ and Java exceptions are a valid analogy for D-Bus errors:
    instead of returning normally with a return value, remote
    applications and the bus may decide to throw an error
    condition. However, the QtDBus implementation does not use the C++
    exception-throwing mechanism, so you will receive QDBusErrors in
    the return reply (see QDBusReply::error()).

    QDBusError objects are used to inspect the error name and message
    as received from the bus and remote applications. You should not
    create such objects yourself to signal error conditions when
    called from D-Bus: instead, use QDBusMessage::createError() and
    QDBusConnection::send().

    \sa QDBusConnection::send(), QDBusMessage, QDBusReply
*/


/*!
    \internal
    Constructs a QDBusError from a DBusError structure.
*/
QDBusError::QDBusError(const DBusError *error)
    : code(NoError)
{
    if (!error || !q_dbus_error_is_set(error))
        return;

    code = ::get(error->name);
    msg = QString::fromUtf8(error->message);
    nm = QString::fromUtf8(error->name);
}

/*!
    \internal
    Constructs a QDBusError from a QDBusMessage.
*/
QDBusError::QDBusError(const QDBusMessage &qdmsg)
    : code(NoError)
{
    if (qdmsg.type() != QDBusMessage::ErrorMessage)
        return;

    code = ::get(qdmsg.errorName().toUtf8().constData());
    nm = qdmsg.errorName();
    msg = qdmsg.errorMessage();
}

/*!
    \internal
    Constructs a QDBusError from a well-known error code
*/
QDBusError::QDBusError(ErrorType error, const QString &mess)
    : code(error)
{
    nm = QLatin1String(::get(error));
    msg = mess;
}

/*!
    \internal
    Constructs a QDBusError from another QDBusError object
*/
QDBusError::QDBusError(const QDBusError &other)
        : code(other.code), msg(other.msg), nm(other.nm)
{
}

/*!
  \internal
  Assignment operator
*/

QDBusError &QDBusError::operator=(const QDBusError &other)
{
    code = other.code;
    msg = other.msg;
    nm = other.nm;
    return *this;
}

/*!
    Returns this error's ErrorType.

    \sa ErrorType
*/

QDBusError::ErrorType QDBusError::type() const
{
    return code;
}

/*!
    Returns this error's name. Error names are similar to D-Bus Interface names, like
    \c org.freedesktop.DBus.InvalidArgs.

    \sa type()
*/

QString QDBusError::name() const
{
    return nm;
}

/*!
    Returns the message that the callee associated with this error. Error messages are
    implementation defined and usually contain a human-readable error code, though this does not
    mean it is suitable for your end-users.
*/

QString QDBusError::message() const
{
    return msg;
}

/*!
    Returns true if this is a valid error condition (i.e., if there was an error),
    otherwise false.
*/

bool QDBusError::isValid() const
{
    return (code != NoError);
}

/*!
    \since 4.3
    Returns the error name associated with error condition \a error.
*/
QString QDBusError::errorString(ErrorType error)
{
    return QLatin1String(::get(error));
}

QDebug operator<<(QDebug dbg, const QDBusError &msg)
{
    dbg.nospace() << "QDBusError(" << msg.name() << ", " << msg.message() << ')';
    return dbg.space();
}

QT_END_NAMESPACE

#endif // QT_NO_DBUS
