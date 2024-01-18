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

#ifndef QAtomicString_P_H
#define QAtomicString_P_H

#include <QUrl>
#include <qitem_p.h>

namespace QPatternist {

class AtomicString : public AtomicValue
{
 public:
   friend class CommonValues;

   typedef AtomicValue::Ptr Ptr;

   /**
    * Creates an instance representing @p value.
    *
    * @note This function does not remove the string literal escaping allowed in XPath 2.0
    */
   static AtomicString::Ptr fromValue(const QString &value);

   static inline AtomicString::Ptr fromValue(const QUrl &value) {
      return fromValue(value.toString());
   }

   /**
    * Get the Effective %Boolean Value of this string. A zero-length
    * string has an effective boolean value of @c false, in all other cases @c true.
    *
    * @returns @c false if the contained string has a zero-length, otherwise @c true.
    */
   bool evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const override;

   /**
    * The string value of a AtomicString instance is the value space.
    */
   QString stringValue() const override;

   ItemType::Ptr type() const override;

 protected:
   friend class StringComparator;
   friend class CompareFN;
   AtomicString(const QString &value);
   const QString m_value;
};
}

#endif
