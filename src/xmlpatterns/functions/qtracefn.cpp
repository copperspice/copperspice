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

#include "qcommonsequencetypes_p.h"
#include "qcommonvalues_p.h"
#include "qitemmappingiterator_p.h"
#include "qpatternistlocale_p.h"

#include "qtracefn_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

namespace QPatternist {
class TraceCallback : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<TraceCallback> Ptr;

   inline TraceCallback(const QString &msg) : m_position(0),
      m_msg(msg) {
   }

   /**
    * Performs the actual tracing.
    */
   Item mapToItem(const Item &item,
                  const DynamicContext::Ptr &context) {
      QTextStream out(stderr);
      ++m_position;
      if (m_position == 1) {
         if (item) {
            out << qPrintable(m_msg)
                << " : "
                << qPrintable(item.stringValue());
         } else {
            out << qPrintable(m_msg)
                << " : ("
                << qPrintable(formatType(context->namePool(), CommonSequenceTypes::Empty))
                << ")\n";
            return Item();
         }
      } else {
         out << qPrintable(item.stringValue())
             << '['
             << m_position
             << "]\n";
      }

      return item;
   }

 private:
   xsInteger m_position;
   const QString m_msg;
};
}

Item::Iterator::Ptr TraceFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
   const QString msg(m_operands.last()->evaluateSingleton(context).stringValue());

   return makeItemMappingIterator<Item>(TraceCallback::Ptr(new TraceCallback(msg)),
                                        m_operands.first()->evaluateSequence(context),
                                        context);
}

Item TraceFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const QString msg(m_operands.last()->evaluateSingleton(context).stringValue());
   const Item item(m_operands.first()->evaluateSingleton(context));

   return TraceCallback::Ptr(new TraceCallback(msg))->mapToItem(item, context);
}

SequenceType::Ptr TraceFN::staticType() const
{
   return m_operands.first()->staticType();
}

QT_END_NAMESPACE
