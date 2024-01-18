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

#ifndef QArgumentConverter_P_H
#define QArgumentConverter_P_H

#include <quntypedatomicconverter_p.h>

namespace QPatternist {

class ArgumentConverter : public UntypedAtomicConverter
{
 public:
   ArgumentConverter(const Expression::Ptr &operand,
                     const ItemType::Ptr &reqType);

   Item evaluateSingleton(const DynamicContext::Ptr &) const override;
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   inline Item::Iterator::Ptr mapToSequence(const Item &item, const DynamicContext::Ptr &context) const;

   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;

 private:
   typedef QExplicitlySharedDataPointer<const ArgumentConverter> ConstPtr;

};

}

#endif
