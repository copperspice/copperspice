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

#ifndef QDBUSBINDING_H
#define QDBUSBINDING_H

#include <QtDBus/QtDBus>
#include <QtScript/qscriptable.h>
#include <QtScript/qscriptengine.h>

#ifndef QT_NO_DBUS

class QDBusConnectionConstructor : public QObject,
                                   public QScriptable
{
    Q_OBJECT
    Q_PROPERTY(QScriptValue sessionBus READ sessionBus)
    Q_PROPERTY(QScriptValue systemBus READ systemBus)

public:
    QDBusConnectionConstructor(QScriptEngine *engine, QScriptValue extensionObject);

    QScriptValue sessionBus() const;
    QScriptValue systemBus() const;

public Q_SLOTS:
    QObject *qscript_call(const QString &name);

    void disconnectFromBus(const QString &name);
    QDBusConnection connectToBus(const QString &address, const QString &name);
    QDBusConnection connectToBus(QDBusConnection::BusType type, const QString &name);
};

class QScriptDBusConnection : public QObject,
                              public QScriptable
{
    Q_OBJECT
    Q_PROPERTY(QString baseService READ baseService)
    Q_PROPERTY(bool isConnected READ isConnected)
    Q_PROPERTY(QScriptValue dbusInterface READ dbusInterface)
public:
    QScriptDBusConnection(const QDBusConnection &conn, QObject *parent);

    inline QString baseService() const { return connection.baseService(); }
    inline bool isConnected() const { return connection.isConnected(); }
    QScriptValue dbusInterface() const;

    inline QDBusConnection dbusConnection() const { return connection; }

public Q_SLOTS:
    inline bool send(const QDBusMessage &message) const
    { return connection.send(message); }
    inline QDBusMessage call(const QDBusMessage &message, int callMode = QDBus::Block, int timeout = -1) const
    { return connection.call(message, QDBus::CallMode(callMode), timeout); }

    inline bool registerService(const QString &serviceName)
    { return connection.registerService(serviceName); }
    inline bool unregisterService(const QString &serviceName)
    { return connection.unregisterService(serviceName); }

    inline QDBusError lastError() const
    { return connection.lastError(); }

    inline void unregisterObject(const QString &path, QDBusConnection::UnregisterMode mode = QDBusConnection::UnregisterNode)
    { return connection.unregisterObject(path, mode); }
    inline QObject *objectRegisteredAt(const QString &path) const
    { return connection.objectRegisteredAt(path); }

#if 0
    bool callWithCallback(const QDBusMessage &message, QObject *receiver,
                          const char *slot, int timeout = -1) const;

    bool connect(const QString &service, const QString &path, const QString &interface,
                 const QString &name, QObject *receiver, const char *slot);
    bool disconnect(const QString &service, const QString &path, const QString &interface,
                    const QString &name, QObject *receiver, const char *slot);

    bool connect(const QString &service, const QString &path, const QString &interface,
                 const QString &name, const QString& signature,
                 QObject *receiver, const char *slot);
    bool disconnect(const QString &service, const QString &path, const QString &interface,
                    const QString &name, const QString& signature,
                    QObject *receiver, const char *slot);

    bool registerObject(const QString &path, QObject *object,
                        RegisterOptions options = ExportAdaptors);

#endif

private:
    QDBusConnection connection;
};

Q_DECLARE_METATYPE(QScriptDBusConnection*)

class QScriptDBusInterfaceConstructor : public QObject,
                                        public QScriptable
{
    Q_OBJECT
public:
    QScriptDBusInterfaceConstructor(QScriptEngine *engine, QScriptValue extensionObject);

public Q_SLOTS:
    QScriptValue qscript_call(const QString &service, const QString &path, const QString &interface = QString(),
                              const QScriptValue &conn = QScriptValue());
};

Q_DECLARE_METATYPE(QDBusMessage)

class QScriptDBusMessageConstructor : public QObject, public QScriptable
{
    Q_OBJECT
    Q_ENUMS(MessageType)
public:
    enum MessageType {
        InvalidMessage = QDBusMessage::InvalidMessage,
        MethodCallMessage = QDBusMessage::MethodCallMessage,
        ReplyMessage = QDBusMessage::ReplyMessage,
        ErrorMessage = QDBusMessage::ErrorMessage,
        SignalMessage = QDBusMessage::SignalMessage
    };

    QScriptDBusMessageConstructor(QScriptEngine *engine, QScriptValue extensionObject);

    QScriptValue protoType() const { return proto; }

public Q_SLOTS:
    QDBusMessage createSignal(const QString &path, const QString &interface, const QString &name);
    QDBusMessage createMethodCall(const QString &destination, const QString &path, const QString &interface, const QString &method);
    QDBusMessage createError(const QString &name, const QString &msg);

public:
    static QScriptValue createReply(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue createErrorReply(QScriptContext *context, QScriptEngine *engine);

private:
    QScriptValue proto;
};

#endif // QT_NO_DBUS
#endif // QDBUSBINDING_H
