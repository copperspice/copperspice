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

#ifndef QDBUSUNIXFILEDESCRIPTOR_H
#define QDBUSUNIXFILEDESCRIPTOR_H

#include <QtCore/QSharedDataPointer>
#include <QtDBus/qdbusmacros.h>

#ifndef QT_NO_DBUS
# include <utility>

QT_BEGIN_NAMESPACE

class QDBusUnixFileDescriptorPrivate;
template<> QExplicitlySharedDataPointer<QDBusUnixFileDescriptorPrivate>::~QExplicitlySharedDataPointer();

class Q_DBUS_EXPORT QDBusUnixFileDescriptor
{
public:
    QDBusUnixFileDescriptor();
    explicit QDBusUnixFileDescriptor(int fileDescriptor);
    QDBusUnixFileDescriptor(const QDBusUnixFileDescriptor &other);
    QDBusUnixFileDescriptor &operator=(const QDBusUnixFileDescriptor &other);
    ~QDBusUnixFileDescriptor();

    bool isValid() const;

    int fileDescriptor() const;
    void setFileDescriptor(int fileDescriptor);

    void giveFileDescriptor(int fileDescriptor);
    int takeFileDescriptor();

    static bool isSupported();

    QDBusUnixFileDescriptor(QDBusUnixFileDescriptor &&other) : d(static_cast<Data &&>(other.d))
    { }
    inline QDBusUnixFileDescriptor &operator=(QDBusUnixFileDescriptor &&other)
    { d.swap(other.d); return *this; }

protected:
    typedef QExplicitlySharedDataPointer<QDBusUnixFileDescriptorPrivate>  Data;
    Data d;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDBusUnixFileDescriptor)
Q_DECLARE_METATYPE(QList<QDBusUnixFileDescriptor>)

#endif // QT_NO_DBUS
#endif // QDBUSUNIXFILEDESCRIPTOR_H
