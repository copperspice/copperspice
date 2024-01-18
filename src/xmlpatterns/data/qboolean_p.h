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

#ifndef QBoolean_P_H
#define QBoolean_P_H

#include <qitem_p.h>

namespace QPatternist {
class Boolean : public AtomicValue
{
 public:
   typedef AtomicValue::Ptr Ptr;

   /**
    * @returns the boolean value this Boolean represents
    */
   static bool evaluateEBV(const Item::Iterator::Ptr &e,
                           const QExplicitlySharedDataPointer<DynamicContext> &);

   static bool evaluateEBV(const Item &first,
                           const Item::Iterator::Ptr &e,
                           const QExplicitlySharedDataPointer<DynamicContext> &);

   static bool evaluateEBV(const Item &item,
                           const QExplicitlySharedDataPointer<DynamicContext> &context);

   QString stringValue() const override;

   /**
    * @returns a Boolean object instantiating @p value. Use True() or False()
    * if you already know what value you need.
    */
   static Boolean::Ptr fromValue(const bool value);

   /**
    * Creates a boolean value from a lexical representation. "true" and "1"
    * becomes @c true, while "false" and "0" becomes @c false.
    */
   static AtomicValue::Ptr fromLexical(const QString &val);

   /**
    * Get the Effective %Boolean Value of this boolean value. For <tt>xs:boolean</tt>, this
    * is simply the value.
    */
   bool evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const override;

   ItemType::Ptr type() const override;

   bool value() const {
      return m_value;
   }

 protected:
   friend class CommonValues;
   Boolean(const bool value);

 private:
   const bool m_value;
};
}

#endif
