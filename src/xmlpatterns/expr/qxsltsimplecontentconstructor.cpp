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

#include "qatomicstring_p.h"
#include "qcommonsequencetypes_p.h"

#include "qxsltsimplecontentconstructor_p.h"

using namespace QPatternist;

XSLTSimpleContentConstructor::XSLTSimpleContentConstructor(const Expression::Ptr &source) : SimpleContentConstructor(
      source)
{
}

QString XSLTSimpleContentConstructor::processItem(const Item &item,
      bool &discard,
      bool &isText)
{
   if (item.isNode()) {
      isText = (item.asNode().kind() == QXmlNodeModelIndex::Text);

      if (isText) {
         const QString value(item.stringValue());

         /* "1. Zero-length text nodes in the sequence are discarded." */
         discard = value.isEmpty();
         return value;
      } else {
         Item::Iterator::Ptr it(item.sequencedTypedValue()); /* Atomic values. */
         Item next(it->next());
         QString result;

         if (next) {
            result = next.stringValue();
         }

         next = it->next();

         while (next) {
            result += next.stringValue();
            result += QLatin1Char(' ');
            next = it->next();
         }

         return result;
      }
   } else {
      discard = false;
      isText = false;
      return item.stringValue();
   }
}

Item XSLTSimpleContentConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));

   Item next(it->next());
   QString result;

   bool previousIsText = false;
   bool discard = false;

   if (next) {
      const QString unit(processItem(next, discard, previousIsText));

      if (!discard) {
         result = unit;
      }

      next = it->next();
   } else {
      return Item();
   }

   while (next) {
      bool currentIsText = false;
      const QString unit(processItem(next, discard, currentIsText));

      if (!discard) {
         /* "Adjacent text nodes in the sequence are merged into a single text
          * node." */
         if (previousIsText && currentIsText)
            ;
         else {
            result += QLatin1Char(' ');
         }

         result += unit;
      }

      next = it->next();
      previousIsText = currentIsText;
   }

   return AtomicString::fromValue(result);
}

SequenceType::List XSLTSimpleContentConstructor::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

SequenceType::Ptr XSLTSimpleContentConstructor::staticType() const
{
   return CommonSequenceTypes::ZeroOrOneString;
}
