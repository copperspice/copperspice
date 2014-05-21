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

#ifndef Patternist_TripleContainer_P_H
#define Patternist_TripleContainer_P_H

#include "qexpression_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist
{
 
    class TripleContainer : public Expression
    {
    public:
        virtual Expression::List operands() const;
        virtual void setOperands(const Expression::List &operands);

        virtual bool compressOperands(const StaticContext::Ptr &);

    protected:
        TripleContainer(const Expression::Ptr &operand1,
                        const Expression::Ptr &operand2,
                        const Expression::Ptr &operand3);

        Expression::Ptr m_operand1;
        Expression::Ptr m_operand2;
        Expression::Ptr m_operand3;
    };
}

QT_END_NAMESPACE

#endif
