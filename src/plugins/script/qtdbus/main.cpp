/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "main.h"
#include <QDebug>
#include <QMetaMethod>
#include <QScriptExtensionPlugin>

#ifndef QT_NO_DBUS

QT_USE_NAMESPACE

static QScriptValue setupDBusInterface(QScriptEngine *engine, QDBusAbstractInterface *iface);

static QScriptValue do_dbus_call(QScriptContext *context, QScriptEngine *engine)
{
    int firstArgument = 0;
    QString functionName = context->callee().property(QLatin1String("functionName")).toString();
    if (functionName.isEmpty()) {
        functionName = context->argument(0).toString();
        ++firstArgument;
    }

    QScriptValue thisObject = context->thisObject();
    QDBusAbstractInterface *iface = qobject_cast<QDBusAbstractInterface *>(thisObject.toQObject());
    if (!iface)
        return QScriptValue();

    QDBusMessage msg = QDBusMessage::createMethodCall(iface->service(),
                                                      iface->path(),
                                                      iface->interface(),
                                                      functionName);

    QList<QVariant> args;
    for (int i = firstArgument; i < context->argumentCount(); ++i) {
        args.append(context->argument(i).toVariant());
    }
    msg.setArguments(args);

    msg = iface->connection().call(msg);

    QScriptValue returnValue = engine->nullValue();
    args = msg.arguments();
    if (args.count() != 1)
        return returnValue;

    QVariant variant = args.first();
    if (variant.type() == QVariant::UserType
        && variant.userType() == qMetaTypeId<QDBusObjectPath>()) {
        QDBusObjectPath path = qvariant_cast<QDBusObjectPath>(variant);

        QDBusInterface *returnedIface = new QDBusInterface(iface->service(),
                                                           path.path(),
                                                           /*interface*/QString(),
                                                           iface->connection(),
                                                           engine);
        returnValue = setupDBusInterface(engine, returnedIface);
    } else {
        returnValue = engine->newVariant(variant);
    }

    return returnValue;
}

static QScriptValue setupDBusInterface(QScriptEngine *engine, QDBusAbstractInterface *iface)
{
    QScriptValue v = engine->newQObject(iface);

    if (!qobject_cast<QDBusConnectionInterface *>(iface)) {
        const QMetaObject *mo = iface->metaObject();
        for (int i = 0; i < mo->methodCount(); ++i) {
            const QMetaMethod method = mo->method(i);
            const QByteArray signature = method.signature();
            //qDebug() << "signature" << signature;
            int parenIndex = signature.indexOf('(');
            if (parenIndex == -1)
                continue;
            const QByteArray name = signature.left(parenIndex);
            if (name.isEmpty())
                continue;

            // don't try to override properties
            if (mo->indexOfProperty(name) != -1)
                continue;

            QScriptValue callWrapper = engine->newFunction(do_dbus_call);
            const QString nameString = QString::fromLatin1(name);
            callWrapper.setProperty(QLatin1String("functionName"), QScriptValue(engine, nameString));
            v.setProperty(nameString, callWrapper);
        }
    }

    v.setProperty(QLatin1String("service"), QScriptValue(engine, iface->service()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("path"), QScriptValue(engine, iface->path()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("interface"), QScriptValue(engine, iface->interface()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("isValid"), QScriptValue(engine, iface->isValid()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("connection"), engine->newQObject(new QScriptDBusConnection(iface->connection(), engine)), QScriptValue::ReadOnly);

    return v;
}

QDBusConnectionConstructor::QDBusConnectionConstructor(QScriptEngine *engine, QScriptValue extensionObject)
    : QObject(engine)
{
    QScriptValue ctor = engine->newQObject(this);

    QScriptValue proto = engine->newQMetaObject(&QDBusConnection::staticMetaObject);
    proto.setPrototype(engine->globalObject().property(QLatin1String("Function")).property(QLatin1String("prototype")));
    ctor.setProperty(QLatin1String("prototype"), proto);

    extensionObject.setProperty(QLatin1String("QDBusConnection"), ctor);
}

QScriptValue QDBusConnectionConstructor::sessionBus() const
{
    return engine()->newQObject(new QScriptDBusConnection(QDBusConnection::sessionBus(), engine()));
}

QScriptValue QDBusConnectionConstructor::systemBus() const
{
    return engine()->newQObject(new QScriptDBusConnection(QDBusConnection::systemBus(), engine()));
}

QObject *QDBusConnectionConstructor::qscript_call(const QString &name)
{
    return new QScriptDBusConnection(QDBusConnection(name), this);
}

void QDBusConnectionConstructor::disconnectFromBus(const QString &name)
{
    QDBusConnection::disconnectFromBus(name);
}

QDBusConnection QDBusConnectionConstructor::connectToBus(const QString &address, const QString &name)
{
    return QDBusConnection::connectToBus(address, name);
}

QDBusConnection QDBusConnectionConstructor::connectToBus(QDBusConnection::BusType type, const QString &name)
{
    return QDBusConnection::connectToBus(type, name);
}

QScriptDBusConnection::QScriptDBusConnection(const QDBusConnection &conn, QObject *parent)
    : QObject(parent), connection(conn)
{
}

QScriptValue QScriptDBusConnection::dbusInterface() const
{
    QDBusConnectionInterface *iface = connection.interface();
    if (!iface)
        return engine()->nullValue();
    return setupDBusInterface(engine(), iface);
}

QScriptDBusInterfaceConstructor::QScriptDBusInterfaceConstructor(QScriptEngine *engine, QScriptValue extensionObject)
{
    QScriptValue ctorValue = engine->newQObject(this);
    QScriptValue klass = engine->newQMetaObject(metaObject(), ctorValue);
    extensionObject.setProperty(QLatin1String("QDBusInterface"), klass);
}

QScriptValue QScriptDBusInterfaceConstructor::qscript_call(const QString &service, const QString &path, const QString &interface,
                                                           const QScriptValue &conn)
{
    QDBusConnection connection = QDBusConnection::sessionBus();

    QScriptDBusConnection *connWrapper = qobject_cast<QScriptDBusConnection *>(conn.toQObject());
    if (connWrapper)
        connection = connWrapper->dbusConnection();

    return setupDBusInterface(engine(), new QDBusInterface(service, path, interface, connection, engine()));
}

QScriptDBusMessageConstructor::QScriptDBusMessageConstructor(QScriptEngine *engine, QScriptValue extensionObject)
    : QObject(engine)
{
    proto = engine->newQMetaObject(metaObject(), engine->newQObject(this));

    proto.setProperty(QLatin1String("createReply"), engine->newFunction(createReply));
    proto.setProperty(QLatin1String("createErrorReply"), engine->newFunction(createErrorReply));

    extensionObject.setProperty(QLatin1String("QDBusMessage"), proto);
    engine->setDefaultPrototype(qMetaTypeId<QDBusMessage>(), proto);
}

QDBusMessage QScriptDBusMessageConstructor::createSignal(const QString &path, const QString &interface, const QString &name)
{
    return QDBusMessage::createSignal(path, interface, name);
}

QDBusMessage QScriptDBusMessageConstructor::createMethodCall(const QString &destination, const QString &path, const QString &interface, const QString &method)
{
    return QDBusMessage::createMethodCall(destination, path, interface, method);
}

QDBusMessage QScriptDBusMessageConstructor::createError(const QString &name, const QString &msg)
{
    return QDBusMessage::createError(name, msg);
}

static QScriptValue messageToScriptValue(QScriptEngine *engine, const QDBusMessage &message)
{
    QScriptValue v = engine->newVariant(QVariant::fromValue(message));
    v.setProperty(QLatin1String("service"), QScriptValue(engine, message.service()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("path"), QScriptValue(engine, message.path()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("interface"), QScriptValue(engine, message.interface()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("member"), QScriptValue(engine, message.member()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("type"), QScriptValue(engine, message.type()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("signature"), QScriptValue(engine, message.signature()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("isReplyRequired"), QScriptValue(engine, message.isReplyRequired()), QScriptValue::ReadOnly);

    v.setProperty(QLatin1String("delayedReply"), QScriptValue(engine, message.isDelayedReply()));
    QScriptValue argValue = engine->newArray();
    const QList<QVariant> args = message.arguments();
    for (int i = 0; i < args.count(); ++i)
        argValue.setProperty(QScriptValue(engine, i).toString(),
                             engine->newVariant(args.at(i)));

    v.setProperty(QLatin1String("arguments"), argValue);

    return v;
}

static void scriptValueToMessage(const QScriptValue &value, QDBusMessage &message)
{
    QVariant v = value.toVariant();
    message = qvariant_cast<QDBusMessage>(v);
    message.setDelayedReply(value.property(QLatin1String("delayedReply")).toBoolean());

    QList<QVariant> args;
    quint32 len = value.property(QLatin1String("length")).toUInt32();
    for (quint32 i = 0; i < len; ++i) {
        QScriptValue item = value.property(i);
        args.append(item.toVariant());
    }
    message.setArguments(args);
}

QScriptValue QScriptDBusMessageConstructor::createReply(QScriptContext *context, QScriptEngine *engine)
{
    QDBusMessage msg;
    scriptValueToMessage(context->thisObject(), msg);

    QList<QVariant> args;
    for (int i = 0; i < context->argumentCount(); ++i) {
        QScriptValue value = context->argument(i);
        args.append(value.toVariant());
    }

    return messageToScriptValue(engine, msg.createReply(args));
}

QScriptValue QScriptDBusMessageConstructor::createErrorReply(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2)
        return engine->nullValue();

    QDBusMessage msg;
    scriptValueToMessage(context->thisObject(), msg);

    QString name = context->argument(0).toString();
    QString errMsg = context->argument(1).toString();
    return messageToScriptValue(engine, msg.createErrorReply(name, errMsg));
}

template <typename T>
QScriptValue qDBusReplyToScriptValue(QScriptEngine *eng, const QDBusReply<T> &reply)
{
    return QScriptValue(eng, reply.value());
}

template <>
QScriptValue qDBusReplyToScriptValue(QScriptEngine *eng, const QDBusReply<QStringList> &reply)
{
    QScriptValue v = eng->newArray();
    const QStringList &lst = reply.value();
    for (int i = 0; i < lst.count(); ++i)
        v.setProperty(i, QScriptValue(eng, lst.at(i)));
    return v;
}

template <typename T>
void qDBusReplyFromScriptValue(const QScriptValue &, QDBusReply<T> &)
{
    // never called
}

QScriptValue qDBusErrorToScriptValue(QScriptEngine *engine, const QDBusError &error)
{
    QScriptValue v = engine->newObject();
    v.setProperty(QLatin1String("type"), QScriptValue(engine, error.type()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("name"), QScriptValue(engine, error.name()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("message"), QScriptValue(engine, error.message()), QScriptValue::ReadOnly);
    v.setProperty(QLatin1String("isValid"), QScriptValue(engine, error.isValid()), QScriptValue::ReadOnly);
    return v;
}

void scriptValueToQDBusError(const QScriptValue &value, QDBusError &error)
{
    Q_UNUSED(value)
    Q_UNUSED(error)
    // never called
}

Q_DECLARE_METATYPE(QDBusReply<QString>)
Q_DECLARE_METATYPE(QDBusReply<QStringList>)
Q_DECLARE_METATYPE(QDBusReply<uint>)
Q_DECLARE_METATYPE(QDBusReply<bool>)
Q_DECLARE_METATYPE(QDBusReply<QDBusConnectionInterface::RegisterServiceReply>)
Q_DECLARE_METATYPE(QDBusError)

class QtDBusScriptPlugin : public QScriptExtensionPlugin
{
public:
    QStringList keys() const;
    void initialize(const QString &key, QScriptEngine *engine);
};

QStringList QtDBusScriptPlugin::keys() const
{
    return QStringList(QLatin1String("qt.dbus"));
}

void QtDBusScriptPlugin::initialize(const QString &key, QScriptEngine *engine)
{
    if (key != QLatin1String("qt.dbus")) {
        Q_ASSERT_X(false, "initialize", qPrintable(key));
        return;
    }

    QScriptValue extensionObject = engine->globalObject();

    qScriptRegisterMetaType<QDBusReply<QString> >(engine, qDBusReplyToScriptValue, qDBusReplyFromScriptValue);
    qScriptRegisterMetaType<QDBusReply<QStringList> >(engine, qDBusReplyToScriptValue, qDBusReplyFromScriptValue);
    qScriptRegisterMetaType<QDBusReply<uint> >(engine, qDBusReplyToScriptValue, qDBusReplyFromScriptValue);
    qScriptRegisterMetaType<QDBusReply<bool> >(engine, qDBusReplyToScriptValue, qDBusReplyFromScriptValue);
    qScriptRegisterMetaType<QDBusReply<QDBusConnectionInterface::RegisterServiceReply> >(engine, qDBusReplyToScriptValue, qDBusReplyFromScriptValue);
    qScriptRegisterMetaType<QDBusMessage>(engine, messageToScriptValue, scriptValueToMessage);
    qScriptRegisterMetaType<QDBusError>(engine, qDBusErrorToScriptValue, scriptValueToQDBusError);

    QScriptValue connIfaceProto = engine->newQMetaObject(&QDBusConnectionInterface::staticMetaObject, engine->nullValue());
    extensionObject.setProperty(QLatin1String("QDBusConnectionInterface"), connIfaceProto);

    QScriptValue qdbus = engine->newObject();
    qdbus.setProperty(QLatin1String("NoBlock"), QScriptValue(engine, QDBus::NoBlock));
    qdbus.setProperty(QLatin1String("Block"), QScriptValue(engine, QDBus::Block));
    qdbus.setProperty(QLatin1String("BlockWithGui"), QScriptValue(engine, QDBus::BlockWithGui));
    qdbus.setProperty(QLatin1String("AutoDetect"), QScriptValue(engine, QDBus::AutoDetect));
    engine->globalObject().setProperty(QLatin1String("QDBus"), qdbus);

    (void)new QDBusConnectionConstructor(engine, extensionObject);
    (void)new QScriptDBusInterfaceConstructor(engine, extensionObject);
    (void)new QScriptDBusMessageConstructor(engine, extensionObject);
}


Q_EXPORT_STATIC_PLUGIN(QtDBusScriptPlugin)
Q_EXPORT_PLUGIN2(qtscriptdbus, QtDBusScriptPlugin)

#endif // QT_NO_DBUS
