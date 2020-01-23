/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "qdbusreply.h"
#include "qdbusmetatype.h"
#include "qdbusmetatype_p.h"
#include <QDebug>

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

/*!
    \class QDBusReply
    \inmodule QtDBus
    \since 4.2

    \brief The QDBusReply class stores the reply for a method call to a remote object.

    A QDBusReply object is a subset of the QDBusMessage object that represents a method call's
    reply. It contains only the first output argument or the error code and is used by
    QDBusInterface-derived classes to allow returning the error code as the function's return
    argument.

    It can be used in the following manner:
    \snippet doc/src/snippets/code/src_qdbus_qdbusreply.cpp 0

    If the remote method call cannot fail, you can skip the error checking:
    \snippet doc/src/snippets/code/src_qdbus_qdbusreply.cpp 1

    However, if it does fail under those conditions, the value returned by QDBusReply::value() is
    a default-constructed value. It may be indistinguishable from a valid return value.

    QDBusReply objects are used for remote calls that have no output
    arguments or return values (i.e., they have a "void" return
    type). Use the isValid() function to test if the reply succeeded.

    \sa QDBusMessage, QDBusInterface
*/

/*!
    \fn QDBusReply::QDBusReply(const QDBusMessage &reply)
    Automatically construct a QDBusReply object from the reply message \a reply, extracting the
    first return value from it if it is a success reply.
*/

/*!
    \fn QDBusReply::QDBusReply(const QDBusPendingReply<T> &reply)
    Constructs a QDBusReply object from the pending reply message, \a reply.
*/

/*!
    \fn QDBusReply::QDBusReply(const QDBusPendingCall &pcall)
    Automatically construct a QDBusReply object from the asynchronous
    pending call \a pcall. If the call isn't finished yet, QDBusReply
    will call QDBusPendingCall::waitForFinished(), which is a blocking
    operation.

    If the return types patch, QDBusReply will extract the first
    return argument from the reply.
*/

/*!
    \fn QDBusReply::QDBusReply(const QDBusError &error)
    Constructs an error reply from the D-Bus error code given by \a error.
*/

/*!
    \fn QDBusReply::operator=(const QDBusReply &other)
    Makes this object be a copy of the object \a other.
*/

/*!
    \fn QDBusReply::operator=(const QDBusError &error)
    Sets this object to contain the error code given by \a error. You
    can later access it with error().
*/

/*!
    \fn QDBusReply::operator=(const QDBusMessage &message)

    Makes this object contain the reply specified by message \a
    message. If \a message is an error message, this function will
    copy the error code and message into this object

    If \a message is a standard reply message and contains at least
    one parameter, it will be copied into this object, as long as it
    is of the correct type. If it's not of the same type as this
    QDBusError object, this function will instead set an error code
    indicating a type mismatch.
*/

/*!
    \fn QDBusReply::operator=(const QDBusPendingCall &pcall)

    Makes this object contain the reply specified by the pending
    asynchronous call \a pcall. If the call is not finished yet, this
    function will call QDBusPendingCall::waitForFinished() to block
    until the reply arrives.

    If \a pcall finishes with an error message, this function will
    copy the error code and message into this object

    If \a pcall finished with a standard reply message and contains at
    least one parameter, it will be copied into this object, as long
    as it is of the correct type. If it's not of the same type as this
    QDBusError object, this function will instead set an error code
    indicating a type mismatch.
*/

/*!
    \fn bool QDBusReply::isValid() const

    Returns true if no error occurred; otherwise, returns false.

    \sa error()
*/

/*!
    \fn QDBusReply::error()

    Returns the error code that was returned from the remote function call. If the remote call did
    not return an error (i.e., if it succeeded), then the QDBusError object that is returned will
    not be a valid error code (QDBusError::isValid() will return false).

    \sa isValid()
*/

/*!
    \fn QDBusReply::value() const
    Returns the remote function's calls return value. If the remote call returned with an error,
    the return value of this function is undefined and may be undistinguishable from a valid return
    value.

    This function is not available if the remote call returns \c void.
*/

/*!
    \fn QDBusReply::operator Type() const
    Returns the same as value().

    This function is not available if the remote call returns \c void.
*/

/*!
    \internal
    Fills in the QDBusReply data \a error and \a data from the reply message \a reply.
*/
void qDBusReplyFill(const QDBusMessage &reply, QDBusError &error, QVariant &data)
{
    error = reply;

    if (error.isValid()) {
        data = QVariant();      // clear it
        return;
    }

    if (reply.arguments().count() >= 1 && reply.arguments().at(0).userType() == data.userType()) {
        data = reply.arguments().at(0);
        return;
    }

    const char *expectedSignature = QDBusMetaType::typeToSignature(data.userType());
    const char *receivedType = 0;
    QByteArray receivedSignature;

    if (reply.arguments().count() >= 1) {
        if (reply.arguments().at(0).userType() == QDBusMetaTypeId::argument) {
            // compare signatures instead
            QDBusArgument arg = qvariant_cast<QDBusArgument>(reply.arguments().at(0));
            receivedSignature = arg.currentSignature().toLatin1();
            if (receivedSignature == expectedSignature) {
                // matched. Demarshall it
                QDBusMetaType::demarshall(arg, data.userType(), data.data());
                return;
            }
        } else {
            // not an argument and doesn't match?
            int type = reply.arguments().at(0).userType();
            receivedType = QVariant::typeToName(QVariant::Type(type));
            receivedSignature = QDBusMetaType::typeToSignature(type);
        }
    }

    // error
    if (receivedSignature.isEmpty())
        receivedSignature = "no signature";
    QString errorMsg;
    if (receivedType) {
        errorMsg = QString::fromLatin1("Unexpected reply signature: got \"%1\" (%4), "
                                         "expected \"%2\" (%3)")
                   .arg(QLatin1String(receivedSignature),
                        QLatin1String(expectedSignature),
                        QLatin1String(data.typeName()),
                        QLatin1String(receivedType));
    } else {
        errorMsg = QString::fromLatin1("Unexpected reply signature: got \"%1\", "
                                         "expected \"%2\" (%3)")
                   .arg(QLatin1String(receivedSignature),
                        QLatin1String(expectedSignature),
                        QLatin1String(data.typeName()));
    }

    error = QDBusError(QDBusError::InvalidSignature, errorMsg);
    data = QVariant();      // clear it
}

QT_END_NAMESPACE

#endif // QT_NO_DBUS
