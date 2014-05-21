/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef Patternist_Atomizer_P_H
#define Patternist_Atomizer_P_H

#include "qitem_p.h"
#include "qsinglecontainer_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    class Atomizer : public SingleContainer
    {
    public:
        Atomizer(const Expression::Ptr &operand);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;

        virtual SequenceType::Ptr staticType() const;

        virtual SequenceType::List expectedOperandTypes() const;
        virtual const SourceLocationReflection *actualReflection() const;

        /**
         * Makes an early compression, by returning the result of
         * the type checked operand, if the operand has the static type
         * xs:anyAtomicType(no atomization needed).
         */
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        inline Item::Iterator::Ptr mapToSequence(const Item &item,
                                                 const DynamicContext::Ptr &context) const;
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

    private:
        typedef QExplicitlySharedDataPointer<const Atomizer> ConstPtr;
    };
}

QT_END_NAMESPACE

#endif
