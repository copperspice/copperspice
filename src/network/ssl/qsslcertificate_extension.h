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

#ifndef QSSLCERTIFICATE_EXTENSION_H
#define QSSLCERTIFICATE_EXTENSION_H

#include <qnamespace.h>
#include <qshareddata.h>
#include <qstring.h>
#include <qvariant.h>

#ifdef QT_SSL

class QSslCertificateExtensionPrivate;

class Q_NETWORK_EXPORT QSslCertificateExtension
{
 public:
    QSslCertificateExtension();
    QSslCertificateExtension(const QSslCertificateExtension &other);

    QSslCertificateExtension &operator=(QSslCertificateExtension &&other) {
      swap(other);
      return *this;
    }

    QSslCertificateExtension &operator=(const QSslCertificateExtension &other);
    ~QSslCertificateExtension();

    void swap(QSslCertificateExtension &other) {
      qSwap(d, other.d);
    }

    QString oid() const;
    QString name() const;
    QVariant value() const;
    bool isCritical() const;

    bool isSupported() const;

 private:
    friend class QSslCertificatePrivate;
    QSharedDataPointer<QSslCertificateExtensionPrivate> d;
};

#endif

#endif
