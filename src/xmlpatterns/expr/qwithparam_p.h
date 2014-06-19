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

#ifndef Patternist_TemplateParam_P_H
#define Patternist_TemplateParam_P_H

#include "qfunctionargument_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {
class WithParam : public FunctionArgument
{
 public:
   typedef QExplicitlySharedDataPointer<WithParam> Ptr;
   typedef QHash<QXmlName, Ptr> Hash;

   inline WithParam(const QXmlName name,
                    const SequenceType::Ptr &type,
                    const Expression::Ptr &sourceExpression);

   inline void setSourceExpression(const Expression::Ptr &expr) {
      Q_ASSERT(expr);
      m_sourceExpression = expr;
   }

   inline Expression::Ptr sourceExpression() const {
      return m_sourceExpression;
   }

 private:
   Expression::Ptr m_sourceExpression;
};

WithParam::WithParam(const QXmlName name,
                     const SequenceType::Ptr &type,
                     const Expression::Ptr &sourceExpression) : FunctionArgument(name, type)
   , m_sourceExpression(sourceExpression)
{
   Q_ASSERT(m_sourceExpression);
}

}

QT_END_NAMESPACE

#endif
