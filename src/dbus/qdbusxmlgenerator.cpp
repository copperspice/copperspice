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

#include <QtCore/qmetaobject.h>
#include <QtCore/qstringlist.h>

#include "qdbusinterface_p.h"   // for ANNOTATION_NO_WAIT
#include "qdbusabstractadaptor_p.h" // for QCLASSINFO_DBUS_*
#include "qdbusconnection_p.h"  // for the flags
#include "qdbusmetatype_p.h"
#include "qdbusmetatype.h"
#include "qdbusutil_p.h"

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

extern Q_DBUS_EXPORT QString qDBusGenerateMetaObjectXml(QString interface, const QMetaObject *mo,
                                                       const QMetaObject *base, int flags);

static inline QString typeNameToXml(const char *typeName)
{
    // ### copied from qtextdocument.cpp
    // ### move this into QtCore at some point
    QString plain = QLatin1String(typeName);
    QString rich;
    rich.reserve(int(plain.length() * 1.1));
    for (int i = 0; i < plain.length(); ++i) {
        if (plain.at(i) == QLatin1Char('<'))
            rich += QLatin1String("&lt;");
        else if (plain.at(i) == QLatin1Char('>'))
            rich += QLatin1String("&gt;");
        else if (plain.at(i) == QLatin1Char('&'))
            rich += QLatin1String("&amp;");
        else
            rich += plain.at(i);
    }
    return rich;
}

// implement the D-Bus org.freedesktop.DBus.Introspectable interface
// we do that by analysing the metaObject of all the adaptor interfaces

static QString generateInterfaceXml(const QMetaObject *mo, int flags, int methodOffset, int propOffset)
{
    QString retval;

    // start with properties:
    if (flags & (QDBusConnection::ExportScriptableProperties |
                 QDBusConnection::ExportNonScriptableProperties)) {
        for (int i = propOffset; i < mo->propertyCount(); ++i) {
            static const char *accessvalues[] = {0, "read", "write", "readwrite"};

            QMetaProperty mp = mo->property(i);

            if (!((mp.isScriptable() && (flags & QDBusConnection::ExportScriptableProperties)) ||
                  (!mp.isScriptable() && (flags & QDBusConnection::ExportNonScriptableProperties))))
                continue;

            int access = 0;
            if (mp.isReadable())
                access |= 1;
            if (mp.isWritable())
                access |= 2;

            int typeId = qDBusNameToTypeId(mp.typeName());
            if (!typeId)
                continue;
            const char *signature = QDBusMetaType::typeToSignature(typeId);
            if (!signature)
                continue;

            retval += QString::fromLatin1("    <property name=\"%1\" type=\"%2\" access=\"%3\"")
                      .arg(QLatin1String(mp.name()))
                      .arg(QLatin1String(signature))
                      .arg(QLatin1String(accessvalues[access]));

            if (QDBusMetaType::signatureToType(signature) == QVariant::Invalid) {
                const char *typeName = QVariant::typeToName(QVariant::Type(typeId));
                retval += QString::fromLatin1(">\n      <annotation name=\"org.qtproject.QtDBus.QtTypeName\" value=\"%3\"/>\n    </property>\n")
                          .arg(typeNameToXml(typeName));
            } else {
                retval += QLatin1String("/>\n");
            }
        }
    }

    // now add methods:
    for (int i = methodOffset; i < mo->methodCount(); ++i) {
        QMetaMethod mm = mo->method(i);
        QByteArray signature = mm.signature();
        int paren = signature.indexOf('(');

        bool isSignal;
        if (mm.methodType() == QMetaMethod::Signal)
            // adding a signal
            isSignal = true;
        else if (mm.access() == QMetaMethod::Public && (mm.methodType() == QMetaMethod::Slot || mm.methodType() == QMetaMethod::Method))
            isSignal = false;
        else
            continue;           // neither signal nor public slot

        if (isSignal && !(flags & (QDBusConnection::ExportScriptableSignals |
                                   QDBusConnection::ExportNonScriptableSignals)))
            continue;           // we're not exporting any signals
        if (!isSignal && (!(flags & (QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportNonScriptableSlots)) &&
                          !(flags & (QDBusConnection::ExportScriptableInvokables | QDBusConnection::ExportNonScriptableInvokables))))
            continue;           // we're not exporting any slots or invokables

        QString xml = QString::fromLatin1("    <%1 name=\"%2\">\n")
                      .arg(isSignal ? QLatin1String("signal") : QLatin1String("method"))
                      .arg(QLatin1String(signature.left(paren)));

        // check the return type first
        int typeId = qDBusNameToTypeId(mm.typeName());
        if (typeId) {
            const char *typeName = QDBusMetaType::typeToSignature(typeId);
            if (typeName) {
                xml += QString::fromLatin1("      <arg type=\"%1\" direction=\"out\"/>\n")
                       .arg(typeNameToXml(typeName));

                // do we need to describe this argument?
                if (QDBusMetaType::signatureToType(typeName) == QVariant::Invalid)
                    xml += QString::fromLatin1("      <annotation name=\"org.qtproject.QtDBus.QtTypeName.Out0\" value=\"%1\"/>\n")
                        .arg(typeNameToXml(QVariant::typeToName(QVariant::Type(typeId))));
            } else
                continue;
        }
        else if (*mm.typeName())
            continue;           // wasn't a valid type

        QList<QByteArray> names = mm.parameterNames();
        QList<int> types;
        int inputCount = qDBusParametersForMethod(mm, types);
        if (inputCount == -1)
            continue;           // invalid form
        if (isSignal && inputCount + 1 != types.count())
            continue;           // signal with output arguments?
        if (isSignal && types.at(inputCount) == QDBusMetaTypeId::message)
            continue;           // signal with QDBusMessage argument?
        if (isSignal && mm.attributes() & QMetaMethod::Cloned)
            continue;           // cloned signal?

        int j;
        bool isScriptable = mm.attributes() & QMetaMethod::Scriptable;
        for (j = 1; j < types.count(); ++j) {
            // input parameter for a slot or output for a signal
            if (types.at(j) == QDBusMetaTypeId::message) {
                isScriptable = true;
                continue;
            }

            QString name;
            if (!names.at(j - 1).isEmpty())
                name = QString::fromLatin1("name=\"%1\" ").arg(QLatin1String(names.at(j - 1)));

            bool isOutput = isSignal || j > inputCount;

            const char *signature = QDBusMetaType::typeToSignature(types.at(j));
            xml += QString::fromLatin1("      <arg %1type=\"%2\" direction=\"%3\"/>\n")
                   .arg(name)
                   .arg(QLatin1String(signature))
                   .arg(isOutput ? QLatin1String("out") : QLatin1String("in"));

            // do we need to describe this argument?
            if (QDBusMetaType::signatureToType(signature) == QVariant::Invalid) {
                const char *typeName = QVariant::typeToName( QVariant::Type(types.at(j)) );
                xml += QString::fromLatin1("      <annotation name=\"org.qtproject.QtDBus.QtTypeName.%1%2\" value=\"%3\"/>\n")
                       .arg(isOutput ? QLatin1String("Out") : QLatin1String("In"))
                       .arg(isOutput && !isSignal ? j - inputCount : j - 1)
                       .arg(typeNameToXml(typeName));
            }
        }

        int wantedMask;
        const bool isSlot = mm.methodType() == QMetaMethod::Slot;
        if (isScriptable)
            wantedMask = isSignal ? QDBusConnection::ExportScriptableSignals
                                  : isSlot ? QDBusConnection::ExportScriptableSlots : QDBusConnection::ExportScriptableInvokables;
        else
            wantedMask = isSignal ? QDBusConnection::ExportNonScriptableSignals
                                  : isSlot ? QDBusConnection::ExportNonScriptableSlots : QDBusConnection::ExportNonScriptableInvokables;
        if ((flags & wantedMask) != wantedMask)
            continue;

        if (qDBusCheckAsyncTag(mm.tag()))
            // add the no-reply annotation
            xml += QLatin1String("      <annotation name=\"" ANNOTATION_NO_WAIT "\""
                                 " value=\"true\"/>\n");

        retval += xml;
        retval += QString::fromLatin1("    </%1>\n")
                  .arg(isSignal ? QLatin1String("signal") : QLatin1String("method"));
    }

    return retval;
}

QString qDBusGenerateMetaObjectXml(QString interface, const QMetaObject *mo,
                                   const QMetaObject *base, int flags)
{
    if (interface.isEmpty())
        // generate the interface name from the meta object
        interface = qDBusInterfaceFromMetaObject(mo);

    QString xml;
    int idx = mo->indexOfClassInfo(QCLASSINFO_DBUS_INTROSPECTION);
    if (idx >= mo->classInfoOffset())
        return QString::fromUtf8(mo->classInfo(idx).value());
    else
        xml = generateInterfaceXml(mo, flags, base->methodCount(), base->propertyCount());

    if (xml.isEmpty())
        return QString();       // don't add an empty interface
    return QString::fromLatin1("  <interface name=\"%1\">\n%2  </interface>\n")
        .arg(interface, xml);
}
#if 0
QString qDBusGenerateMetaObjectXml(QString interface, const QMetaObject *mo, const QMetaObject *base,
                                   int flags)
{
    if (interface.isEmpty()) {
        // generate the interface name from the meta object
        int idx = mo->indexOfClassInfo(QCLASSINFO_DBUS_INTERFACE);
        if (idx >= mo->classInfoOffset()) {
            interface = QLatin1String(mo->classInfo(idx).value());
        } else {
            interface = QLatin1String(mo->className());
            interface.replace(QLatin1String("::"), QLatin1String("."));

            if (interface.startsWith(QLatin1String("QDBus"))) {
                interface.prepend(QLatin1String("org.qtproject.QtDBus."));
            } else if (interface.startsWith(QLatin1Char('Q')) &&
                       interface.length() >= 2 && interface.at(1).isUpper()) {
                // assume it's Qt
                interface.prepend(QLatin1String("org.qtproject.Qt."));
            } else if (!QCoreApplication::instance()||
                       QCoreApplication::instance()->applicationName().isEmpty()) {
                interface.prepend(QLatin1String("local."));
            } else {
                interface.prepend(QLatin1Char('.')).prepend(QCoreApplication::instance()->applicationName());
                QStringList domainName =
                    QCoreApplication::instance()->organizationDomain().split(QLatin1Char('.'),
                                                                             QString::SkipEmptyParts);
                if (domainName.isEmpty())
                    interface.prepend(QLatin1String("local."));
                else
                    for (int i = 0; i < domainName.count(); ++i)
                        interface.prepend(QLatin1Char('.')).prepend(domainName.at(i));
            }
        }
    }

    QString xml;
    int idx = mo->indexOfClassInfo(QCLASSINFO_DBUS_INTROSPECTION);
    if (idx >= mo->classInfoOffset())
        return QString::fromUtf8(mo->classInfo(idx).value());
    else
        xml = generateInterfaceXml(mo, flags, base->methodCount(), base->propertyCount());

    if (xml.isEmpty())
        return QString();       // don't add an empty interface
    return QString::fromLatin1("  <interface name=\"%1\">\n%2  </interface>\n")
        .arg(interface, xml);
}

#endif

QT_END_NAMESPACE

#endif // QT_NO_DBUS
