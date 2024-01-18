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

#include "qanyuri_p.h"
#include "qboolean_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonvalues_p.h"
#include "qliteral_p.h"
#include "qitem_p.h"
#include "qqnamevalue_p.h"
#include "qatomicstring_p.h"
#include "qaccessorfns_p.h"

using namespace QPatternist;

Item NodeNameFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item item(m_operands.first()->evaluateSingleton(context));

   if (item) {
      const QXmlName name(item.asNode().name());

      if (name.isNull()) {
         return Item();
      } else {
         return toItem(QNameValue::fromValue(context->namePool(), name));
      }
   } else {
      return Item();
   }
}

Item NilledFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item item(m_operands.first()->evaluateSingleton(context));

   if (item && item.asNode().kind() == QXmlNodeModelIndex::Element) {
      /* We have no access to the PSVI -- always return false. */
      return CommonValues::BooleanFalse;
   } else {
      return Item();
   }
}

Item StringFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item item(m_operands.first()->evaluateSingleton(context));

   if (item) {
      return AtomicString::fromValue(item.stringValue());
   } else {
      return CommonValues::EmptyString;
   }
}

Expression::Ptr StringFN::typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType)
{
   const Expression::Ptr me(FunctionCall::typeCheck(context, reqType));
   if (me != this) {
      return me;
   }

   if (BuiltinTypes::xsString->xdtTypeMatches(m_operands.first()->staticType()->itemType())) {
      return m_operands.first();   /* No need for string(), it's already a string. */
   } else {
      return me;
   }
}

Item BaseURIFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item node(m_operands.first()->evaluateSingleton(context));

   if (node) {
      const QUrl base(node.asNode().baseUri());

      if (base.isEmpty()) {
         return Item();
      } else if (base.isValid()) {
         Q_ASSERT_X(!base.isRelative(), Q_FUNC_INFO,
                    "The base URI must be absolute.");
         return toItem(AnyURI::fromValue(base));
      } else {
         return Item();
      }
   } else {
      return Item();
   }
}

Item DocumentURIFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item node(m_operands.first()->evaluateSingleton(context));

   if (node) {
      const QUrl documentURI(node.asNode().documentUri());

      if (documentURI.isValid()) {
         if (documentURI.isEmpty()) {
            return Item();
         } else {
            Q_ASSERT_X(!documentURI.isRelative(), Q_FUNC_INFO,
                       "The document URI must be absolute.");
            return toItem(AnyURI::fromValue(documentURI));
         }
      } else {
         return Item();
      }
   } else {
      return Item();
   }
}
