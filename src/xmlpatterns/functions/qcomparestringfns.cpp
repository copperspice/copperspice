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

#include "qcommonnamespaces_p.h"

#include "qboolean_p.h"
#include "qcommonvalues_p.h"
#include "qinteger_p.h"
#include "qatomicstring_p.h"

#include "qcomparestringfns_p.h"

using namespace QPatternist;

Item CodepointEqualFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item op1(m_operands.first()->evaluateSingleton(context));
   if (!op1) {
      return Item();
   }

   const Item op2(m_operands.last()->evaluateSingleton(context));
   if (!op2) {
      return Item();
   }

   if (caseSensitivity() == Qt::CaseSensitive) {
      return Boolean::fromValue(op1.stringValue() == op2.stringValue());
   } else {
      const QString s1(op1.stringValue());
      const QString s2(op2.stringValue());

      return Boolean::fromValue(s1.length() == s2.length() &&
                                s1.startsWith(s2, Qt::CaseInsensitive));
   }
}

Item CompareFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item op1(m_operands.first()->evaluateSingleton(context));
   if (!op1) {
      return Item();
   }

   const Item op2(m_operands.at(1)->evaluateSingleton(context));
   if (!op2) {
      return Item();
   }

   const int retval = caseSensitivity() == Qt::CaseSensitive
                      ? op1.stringValue().compare(op2.stringValue())
                      : op1.stringValue().toLower().compare(op2.stringValue().toLower());

   if (retval > 0) {
      return CommonValues::IntegerOne;
   } else if (retval < 0) {
      return CommonValues::IntegerOneNegative;
   } else {
      Q_ASSERT(retval == 0);
      return CommonValues::IntegerZero;
   }
}
