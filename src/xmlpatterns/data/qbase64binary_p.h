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

#ifndef QBase64Binary_P_H
#define QBase64Binary_P_H

#include <qitem_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class Base64Binary : public AtomicValue
{
 public:
   friend class CommonValues;

   typedef AtomicValue::Ptr Ptr;

   /**
    * Creates an instance representing @p value.
    */
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
   /**
    * @short Assumes @p in is a lexical representation of @c xs:base64Binary, and
    * converts it to the binary data set in @p out.
    *
    * If @p instr is invalid Base64 content, @p ok is set to
    * false, and the returned QByteArray has an undefined value.
    *
    *  We cannot use QByteArray::fromBase64() because it doesn't do the
    *  necessary validation that we need to properly implement W3C XML
    *  Schema's xs:base64Binary type.
    */
   static void base64Decode(const QByteArray &in, QByteArray &out, bool &ok);

   static const char Base64DecMap[128];
};
}

QT_END_NAMESPACE

#endif
