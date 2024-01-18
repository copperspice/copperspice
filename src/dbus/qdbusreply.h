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

#ifndef QDBUSREPLY_H
#define QDBUSREPLY_H

#include <QtCore/qglobal.h>
#include <QtCore/qvariant.h>

#include <QtDBus/qdbusmacros.h>
#include <QtDBus/qdbusmessage.h>
#include <QtDBus/qdbuserror.h>
#include <QtDBus/qdbusextratypes.h>
#include <QtDBus/qdbuspendingreply.h>

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

Q_DBUS_EXPORT void qDBusReplyFill(const QDBusMessage &reply, QDBusError &error, QVariant &data);

template<typename T>
class QDBusReply
{
    typedef T Type;
public:
    inline QDBusReply(const QDBusMessage &reply)
    {
        *this = reply;
    }
    inline QDBusReply& operator=(const QDBusMessage &reply)
    {
        QVariant data(qMetaTypeId<Type>(), reinterpret_cast<void*>(0));
        qDBusReplyFill(reply, m_error, data);
        m_data = qvariant_cast<Type>(data);
        return *this;
    }

    inline QDBusReply(const QDBusPendingCall &pcall)
    {
        *this = pcall;
    }
    inline QDBusReply &operator=(const QDBusPendingCall &pcall)
    {
        QDBusPendingCall other(pcall);
        other.waitForFinished();
        return *this = other.reply();
    }
    inline QDBusReply(const QDBusPendingReply<T> &reply)
    {
        *this = static_cast<QDBusPendingCall>(reply);
    }

    inline QDBusReply(const QDBusError &dbusError = QDBusError())
        : m_error(dbusError), m_data(Type())
    {
    }
    inline QDBusReply& operator=(const QDBusError& dbusError)
    {
        m_error = dbusError;
        m_data = Type();
        return *this;
    }

    inline QDBusReply& operator=(const QDBusReply& other)
    {
        m_error = other.m_error;
        m_data = other.m_data;
        return *this;
    }

    inline bool isValid() const { return !m_error.isValid(); }

    inline const QDBusError& error() { return m_error; }

    inline Type value() const
    {
        return m_data;
    }

    inline operator Type () const
    {
        return m_data;
    }

private:
    QDBusError m_error;
    Type m_data;
};


// specialize for QVariant:
template<> inline QDBusReply<QVariant>&
QDBusReply<QVariant>::operator=(const QDBusMessage &reply)
{
    void *null = 0;
    QVariant data(qMetaTypeId<QDBusVariant>(), null);
    qDBusReplyFill(reply, m_error, data);
    m_data = qvariant_cast<QDBusVariant>(data).variant();
    return *this;
}

// specialize for void:
template<>
class QDBusReply<void>
{
public:
    inline QDBusReply(const QDBusMessage &reply)
        : m_error(reply)
    {
    }
    inline QDBusReply& operator=(const QDBusMessage &reply)
    {
        m_error = reply;
        return *this;
    }
    inline QDBusReply(const QDBusError &dbusError = QDBusError())
        : m_error(dbusError)
    {
    }
    inline QDBusReply(const QDBusPendingCall &pcall)
    {
        *this = pcall;
    }
    inline QDBusReply &operator=(const QDBusPendingCall &pcall)
    {
        QDBusPendingCall other(pcall);
        other.waitForFinished();
        return *this = other.reply();
    }
    inline QDBusReply& operator=(const QDBusError& dbusError)
    {
        m_error = dbusError;
        return *this;
    }

    inline QDBusReply& operator=(const QDBusReply& other)
    {
        m_error = other.m_error;
        return *this;
    }

    inline bool isValid() const { return !m_error.isValid(); }

    inline const QDBusError& error() { return m_error; }

private:
    QDBusError m_error;
};


QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif
