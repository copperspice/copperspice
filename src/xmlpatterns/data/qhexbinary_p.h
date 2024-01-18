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

#ifndef QHexBinary_P_H
#define QHexBinary_P_H

#include <qbase64binary_p.h>

namespace QPatternist {
class HexBinary : public Base64Binary
{
 public:
   friend class CommonValues;

   typedef AtomicValue::Ptr Ptr;

   QString stringValue() const override;
   ItemType::Ptr type() const override;

   /**
    * Creates a @c xs:hexBinary from the lexical representation @p value.
    */
   static AtomicValue::Ptr fromLexical(const NamePool::Ptr &np, const QString &value);

   /**
    * Creates an instance representing @p value.
    */
   static HexBinary::Ptr fromValue(const QByteArray &data);

 protected:
   HexBinary(const QByteArray &val);

 private:

   static inline qint8 fromHex(const QChar &c);
};
}

#endif
