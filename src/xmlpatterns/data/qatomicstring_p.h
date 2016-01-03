/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QAtomicString_P_H
#define QAtomicString_P_H

#include <QUrl>
#include <qitem_p.h>

QT_BEGIN_NAMESPACE

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
   virtual bool evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const;

   /**
    * The string value of a AtomicString instance is the value space.
    */
   virtual QString stringValue() const;

   virtual ItemType::Ptr type() const;

 protected:
   friend class StringComparator;
   friend class CompareFN;
   AtomicString(const QString &value);
   const QString m_value;
};
}

QT_END_NAMESPACE


#endif
