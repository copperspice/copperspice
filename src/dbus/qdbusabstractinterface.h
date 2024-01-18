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

#ifndef QDBUSABSTRACTINTERFACE_H
#define QDBUSABSTRACTINTERFACE_H

#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>

#include <QtDBus/qdbusmessage.h>
#include <QtDBus/qdbusextratypes.h>
#include <QtDBus/qdbusconnection.h>

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QDBusError;
class QDBusPendingCall;
class QDBusAbstractInterfacePrivate;

class Q_DBUS_EXPORT QDBusAbstractInterfaceBase: public QObject
{
public:
    int qt_metacall(QMetaObject::Call, int, void**);
protected:
    QDBusAbstractInterfaceBase(QDBusAbstractInterfacePrivate &dd, QObject *parent);
private:
    Q_DECLARE_PRIVATE(QDBusAbstractInterface)
};

class Q_DBUS_EXPORT QDBusAbstractInterface:
        public QDBusAbstractInterfaceBase

{
    CS_OBJECT(QDBusAbstractInterface)

public:
    virtual ~QDBusAbstractInterface();
    bool isValid() const;

    QDBusConnection connection() const;

    QString service() const;
    QString path() const;
    QString interface() const;

    QDBusError lastError() const;

    void setTimeout(int timeout);
    int timeout() const;

    QDBusMessage call(const QString &method,
                      const QVariant &arg1 = QVariant(),
                      const QVariant &arg2 = QVariant(),
                      const QVariant &arg3 = QVariant(),
                      const QVariant &arg4 = QVariant(),
                      const QVariant &arg5 = QVariant(),
                      const QVariant &arg6 = QVariant(),
                      const QVariant &arg7 = QVariant(),
                      const QVariant &arg8 = QVariant());

    QDBusMessage call(QDBus::CallMode mode,
                      const QString &method,
                      const QVariant &arg1 = QVariant(),
                      const QVariant &arg2 = QVariant(),
                      const QVariant &arg3 = QVariant(),
                      const QVariant &arg4 = QVariant(),
                      const QVariant &arg5 = QVariant(),
                      const QVariant &arg6 = QVariant(),
                      const QVariant &arg7 = QVariant(),
                      const QVariant &arg8 = QVariant());

    QDBusMessage callWithArgumentList(QDBus::CallMode mode,
                                      const QString &method,
                                      const QList<QVariant> &args);

    bool callWithCallback(const QString &method,
                          const QList<QVariant> &args,
                          QObject *receiver, const char *member, const char *errorSlot);
    bool callWithCallback(const QString &method,
                          const QList<QVariant> &args,
                          QObject *receiver, const char *member);

    QDBusPendingCall asyncCall(const QString &method,
                               const QVariant &arg1 = QVariant(),
                               const QVariant &arg2 = QVariant(),
                               const QVariant &arg3 = QVariant(),
                               const QVariant &arg4 = QVariant(),
                               const QVariant &arg5 = QVariant(),
                               const QVariant &arg6 = QVariant(),
                               const QVariant &arg7 = QVariant(),
                               const QVariant &arg8 = QVariant());
    QDBusPendingCall asyncCallWithArgumentList(const QString &method,
                                               const QList<QVariant> &args);

protected:
    QDBusAbstractInterface(const QString &service, const QString &path, const char *interface,
                           const QDBusConnection &connection, QObject *parent);
    QDBusAbstractInterface(QDBusAbstractInterfacePrivate &, QObject *parent);

    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);
    QVariant internalPropGet(const char *propname) const;
    void internalPropSet(const char *propname, const QVariant &value);
    QDBusMessage internalConstCall(QDBus::CallMode mode,
                                   const QString &method,
                                   const QList<QVariant> &args = QList<QVariant>()) const;

private:
    Q_DECLARE_PRIVATE(QDBusAbstractInterface)
    CS_SLOT_1(Private, void _q_serviceOwnerChanged(QString un_named_arg1,QString un_named_arg2,QString un_named_arg3))
    CS_SLOT_2(_q_serviceOwnerChanged)

/*  PRIVATE_SLOT
void _q_serviceOwnerChanged(QString un_named_arg1,QString un_named_arg2,QString un_named_arg3)
{
	Q_D(QDBusAbstractInterface);
	d->_q_serviceOwnerChanged();
}
*/
};

QT_END_NAMESPACE


#endif // QT_NO_DBUS
#endif
