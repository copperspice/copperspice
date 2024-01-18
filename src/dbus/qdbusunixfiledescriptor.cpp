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

#include "qdbusunixfiledescriptor.h"
#include <QSharedData>

#ifdef Q_OS_UNIX
# include <qcore_unix_p.h>
#endif

class QDBusUnixFileDescriptorPrivate : public QSharedData {
public:
    QDBusUnixFileDescriptorPrivate() : fd(-1) { }
    QDBusUnixFileDescriptorPrivate(const QDBusUnixFileDescriptorPrivate &other)
        : QSharedData(other), fd(-1)
    {  }
    ~QDBusUnixFileDescriptorPrivate();

    QAtomicInt fd;
};

template<> inline
QExplicitlySharedDataPointer<QDBusUnixFileDescriptorPrivate>::~QExplicitlySharedDataPointer()
{ if (d && !d->ref.deref()) delete d; }

QDBusUnixFileDescriptor::QDBusUnixFileDescriptor()
    : d(0)
{
}

QDBusUnixFileDescriptor::QDBusUnixFileDescriptor(int fileDescriptor)
    : d(0)
{
    if (fileDescriptor != -1)
        setFileDescriptor(fileDescriptor);
}

QDBusUnixFileDescriptor::QDBusUnixFileDescriptor(const QDBusUnixFileDescriptor &other)
    : d(other.d)
{
}

QDBusUnixFileDescriptor &QDBusUnixFileDescriptor::operator=(const QDBusUnixFileDescriptor &other)
{
    if (this != &other)
        d.operator=(other.d);
    return *this;
}

QDBusUnixFileDescriptor::~QDBusUnixFileDescriptor()
{
}

bool QDBusUnixFileDescriptor::isValid() const
{
    return d ? d->fd != -1 : false;
}

int QDBusUnixFileDescriptor::fileDescriptor() const
{
    return d ? d->fd.operator int() : -1;
}

// actual implementation
#ifdef Q_OS_UNIX

bool QDBusUnixFileDescriptor::isSupported()
{
    return true;
}

void QDBusUnixFileDescriptor::setFileDescriptor(int fileDescriptor)
{
    if (fileDescriptor != -1)
        giveFileDescriptor(qt_safe_dup(fileDescriptor));
}

void QDBusUnixFileDescriptor::giveFileDescriptor(int fileDescriptor)
{
    // if we are the sole ref, d remains unchanged
    // if detaching happens, d->fd will be -1
    if (d)
        d.detach();
    else
        d = new QDBusUnixFileDescriptorPrivate;

    if (d->fd != -1)
        qt_safe_close(d->fd);

    if (fileDescriptor != -1)
        d->fd = fileDescriptor;
}

/*!
    \internal
    Extracts the Unix file descriptor from the QDBusUnixFileDescriptor object
    and transfers ownership.

    Note: since QDBusUnixFileDescriptor is implicitly shared, this function
    is inherently racy and should be avoided.
*/
int QDBusUnixFileDescriptor::takeFileDescriptor()
{
    if (!d)
        return -1;

    return d->fd.fetchAndStoreRelaxed(-1);
}

QDBusUnixFileDescriptorPrivate::~QDBusUnixFileDescriptorPrivate()
{
    if (fd != -1)
        qt_safe_close(fd);
}

#else
bool QDBusUnixFileDescriptor::isSupported()
{
    return false;
}

void QDBusUnixFileDescriptor::setFileDescriptor(int)
{
}

void QDBusUnixFileDescriptor::giveFileDescriptor(int)
{
}

int QDBusUnixFileDescriptor::takeFileDescriptor()
{
    return -1;
}

QDBusUnixFileDescriptorPrivate::~QDBusUnixFileDescriptorPrivate()
{
}

#endif

QT_END_NAMESPACE
