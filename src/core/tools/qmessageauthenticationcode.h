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

#ifndef QMESSAGE_AUTHENTICATION_CODE_H
#define QMESSAGE_AUTHENTICATION_CODE_H

#include <qcryptographichash.h>

class QMessageAuthenticationCodePrivate;
class QIODevice;

class Q_CORE_EXPORT QMessageAuthenticationCode
{
public:
    explicit QMessageAuthenticationCode(QCryptographicHash::Algorithm method, const QByteArray &key = QByteArray());
    ~QMessageAuthenticationCode();

    void reset();

    void setKey(const QByteArray &key);

    void addData(const char *data, int length);
    void addData(const QByteArray &data);
    bool addData(QIODevice *device);

    QByteArray result() const;

    static QByteArray hash(const QByteArray &message, const QByteArray &key, QCryptographicHash::Algorithm method);

private:
    Q_DISABLE_COPY(QMessageAuthenticationCode)
    QMessageAuthenticationCodePrivate *d;
};

#endif
