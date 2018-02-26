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

#ifndef QASN1ELEMENT_P_H
#define QASN1ELEMENT_P_H

#include <qdatetime.h>
#include <qmap.h>

#define RSA_ENCRYPTION_OID QByteArrayLiteral("1.2.840.113549.1.1.1")
#define DSA_ENCRYPTION_OID QByteArrayLiteral("1.2.840.10040.4.1")
#define EC_ENCRYPTION_OID QByteArrayLiteral("1.2.840.10045.2.1")

class  QAsn1Element
{
 public:
    enum ElementType {
        // universal
        BooleanType = 0x01,
        IntegerType  = 0x02,
        BitStringType  = 0x03,
        OctetStringType = 0x04,
        NullType = 0x05,
        ObjectIdentifierType = 0x06,
        Utf8StringType = 0x0c,
        PrintableStringType = 0x13,
        TeletexStringType = 0x14,
        UtcTimeType = 0x17,
        GeneralizedTimeType = 0x18,
        SequenceType = 0x30,
        SetType = 0x31,

        // GeneralNameTypes
        Rfc822NameType = 0x81,
        DnsNameType = 0x82,
        UniformResourceIdentifierType = 0x86,

        // context specific
        Context0Type = 0xA0,
        Context1Type = 0xA1,
        Context3Type = 0xA3
    };

    explicit QAsn1Element(quint8 type = 0, const QByteArray &value = QByteArray());

    bool read(QDataStream &data);
    bool read(const QByteArray &data);
    void write(QDataStream &data) const;

    static QAsn1Element fromBool(bool val);
    static QAsn1Element fromInteger(unsigned int val);
    static QAsn1Element fromVector(const QVector<QAsn1Element> &items);
    static QAsn1Element fromObjectId(const QByteArray &id);

    bool toBool(bool *ok = 0) const;
    QDateTime toDateTime() const;
    QMultiMap<QByteArray, QString> toInfo() const;
    qint64 toInteger(bool *ok = 0) const;
    QVector<QAsn1Element> toVector() const;
    QByteArray toObjectId() const;
    QByteArray toObjectName() const;
    QString toString() const;

    quint8 type() const { return mType; }
    QByteArray value() const { return mValue; }

    friend inline bool operator==(const QAsn1Element &, const QAsn1Element &);
    friend inline bool operator!=(const QAsn1Element &, const QAsn1Element &);

 private:
    quint8 mType;
    QByteArray mValue;
};
Q_DECLARE_TYPEINFO(QAsn1Element, Q_MOVABLE_TYPE);

inline bool operator==(const QAsn1Element &e1, const QAsn1Element &e2) {
   return e1.mType == e2.mType && e1.mValue == e2.mValue;
}

inline bool operator!=(const QAsn1Element &e1, const QAsn1Element &e2) {
   return e1.mType != e2.mType || e1.mValue != e2.mValue;
}

#endif
