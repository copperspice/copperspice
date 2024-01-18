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

#include <qsslcertificate_extension.h>
#include <qsslcertificate_extension_p.h>

QSslCertificateExtension::QSslCertificateExtension()
   : d(new QSslCertificateExtensionPrivate)
{
}

QSslCertificateExtension::QSslCertificateExtension(const QSslCertificateExtension &other)
   : d(other.d)
{
}

QSslCertificateExtension::~QSslCertificateExtension()
{
}

QSslCertificateExtension &QSslCertificateExtension::operator=(const QSslCertificateExtension &other)
{
   d = other.d;
   return *this;
}

QString QSslCertificateExtension::oid() const
{
   return d->oid;
}

QString QSslCertificateExtension::name() const
{
   return d->name;
}

QVariant QSslCertificateExtension::value() const
{
   return d->value;
}

bool QSslCertificateExtension::isCritical() const
{
   return d->critical;
}

bool QSslCertificateExtension::isSupported() const
{
   return d->supported;
}
