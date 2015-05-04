/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QArgumentReference_P_H
#define QArgumentReference_P_H

#include <qvariablereference_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class ArgumentReference : public VariableReference
{
 public:
   ArgumentReference(const SequenceType::Ptr &sourceType, const VariableSlotID slot);

   virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
   virtual SequenceType::Ptr staticType() const;
   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
   virtual ID id() const;

 private:
   const SequenceType::Ptr m_type;
};
}

QT_END_NAMESPACE

#endif
