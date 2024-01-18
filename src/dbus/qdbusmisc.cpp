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

#include <string.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qvariant.h>
#include <QtCore/qmetaobject.h>

#include "qdbusutil_p.h"
#include "qdbusconnection_p.h"
#include "qdbusmetatype_p.h"
#include "qdbusabstractadaptor_p.h" // for QCLASSINFO_DBUS_*

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

bool qDBusCheckAsyncTag(const char *tag)
{
    static const char noReplyTag[] = "Q_NOREPLY";
    if (!tag || !*tag)
        return false;

    const char *p = strstr(tag, noReplyTag);
    if (p != NULL &&
        (p == tag || *(p-1) == ' ') &&
        (p[sizeof noReplyTag - 1] == '\0' || p[sizeof noReplyTag - 1] == ' '))
        return true;

    return false;
}

int qDBusNameToTypeId(const char *name)
{
    int id = static_cast<int>( QVariant::nameToType(name) );
    if (id == QVariant::UserType)
        id = QMetaType::type(name);
    return id;
}

QString qDBusInterfaceFromMetaObject(const QMetaObject *mo)
{
    QString interface;

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

    return interface;
}

bool qDBusInterfaceInObject(QObject *obj, const QString &interface_name)
{
    const QMetaObject *mo = obj->metaObject();
    for ( ; mo != &QObject::staticMetaObject; mo = mo->superClass())
        if (interface_name == qDBusInterfaceFromMetaObject(mo))
            return true;
    return false;
}

// calculates the metatypes for the method
// the slot must have the parameters in the following form:
//  - zero or more value or const-ref parameters of any kind
//  - zero or one const ref of QDBusMessage
//  - zero or more non-const ref parameters
// No parameter may be a template.
// this function returns -1 if the parameters don't match the above form
// this function returns the number of *input* parameters, including the QDBusMessage one if any
// this function does not check the return type, so metaTypes[0] is always 0 and always present
// metaTypes.count() >= retval + 1 in all cases
//
// sig must be the normalised signature for the method
int qDBusParametersForMethod(const QMetaMethod &mm, QList<int>& metaTypes)
{
    QDBusMetaTypeId::init();

    QList<QByteArray> parameterTypes = mm.parameterTypes();
    metaTypes.clear();

    metaTypes.append(0);        // return type
    int inputCount = 0;
    bool seenMessage = false;
    QList<QByteArray>::ConstIterator it = parameterTypes.constBegin();
    QList<QByteArray>::ConstIterator end = parameterTypes.constEnd();
    for ( ; it != end; ++it) {
        const QByteArray &type = *it;
        if (type.endsWith('*')) {
            //qWarning("Could not parse the method '%s'", mm.signature());
            // pointer?
            return -1;
        }

        if (type.endsWith('&')) {
            QByteArray basictype = type;
            basictype.truncate(type.length() - 1);

            int id = qDBusNameToTypeId(basictype);
            if (id == 0) {
                //qWarning("Could not parse the method '%s'", mm.signature());
                // invalid type in method parameter list
                return -1;
            } else if (QDBusMetaType::typeToSignature(id) == 0)
                return -1;

            metaTypes.append( id );
            seenMessage = true; // it cannot appear anymore anyways
            continue;
        }

        if (seenMessage) {      // && !type.endsWith('&')
            //qWarning("Could not parse the method '%s'", mm.signature());
            // non-output parameters after message or after output params
            return -1;          // not allowed
        }

        int id = qDBusNameToTypeId(type);
        if (id == 0) {
            //qWarning("Could not parse the method '%s'", mm.signature());
            // invalid type in method parameter list
            return -1;
        }

        if (id == QDBusMetaTypeId::message)
            seenMessage = true;
        else if (QDBusMetaType::typeToSignature(id) == 0)
            return -1;

        metaTypes.append(id);
        ++inputCount;
    }

    return inputCount;
}

QT_END_NAMESPACE

#endif // QT_NO_DBUS
