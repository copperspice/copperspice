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

#ifndef Patternist_UntypedAtomicConverter_P_H
#define Patternist_UntypedAtomicConverter_P_H

#include "qitem_p.h"
#include "qsinglecontainer_p.h"
#include "qcastingplatform_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    class UntypedAtomicConverter : public SingleContainer, public CastingPlatform<UntypedAtomicConverter, true>
    {
    public:
        UntypedAtomicConverter(const Expression::Ptr &operand,
                               const ItemType::Ptr &reqType,
                               const ReportContext::ErrorCode code = ReportContext::FORG0001);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;

        virtual SequenceType::Ptr staticType() const;
        virtual SequenceType::List expectedOperandTypes() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        /**
         * Overridden to call CastingPlatform::typeCheck()
         */
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        inline Item mapToItem(const Item &item,
                              const DynamicContext::Ptr &context) const;

        inline ItemType::Ptr targetType() const
        {
            return m_reqType;
        }

        virtual const SourceLocationReflection *actualReflection() const;

    private:
        typedef QExplicitlySharedDataPointer<const UntypedAtomicConverter> ConstPtr;
        const ItemType::Ptr m_reqType;
    };

    Item UntypedAtomicConverter::mapToItem(const Item &item, const DynamicContext::Ptr &context) const
    {
        return cast(item, context);
    }
}

QT_END_NAMESPACE

#endif
