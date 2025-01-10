/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QBase64Binary_P_H
#define QBase64Binary_P_H

#include <qitem_p.h>

namespace QPatternist {

class Base64Binary : public AtomicValue
{
 public:
   friend class CommonValues;

   typedef AtomicValue::Ptr Ptr;

   static AtomicValue::Ptr fromLexical(const QString &value);
   static Base64Binary::Ptr fromValue(const QByteArray &data);

   QString stringValue() const override;
   ItemType::Ptr type() const override;

   const QByteArray &asByteArray() const {
      return m_value;
   }

 protected:
   Base64Binary(const QByteArray &val);

   const QByteArray m_value;

 private:
   static void base64Decode(const QByteArray &in, QByteArray &out, bool &ok);
   static const char Base64DecMap[128];
};

}

#endif
