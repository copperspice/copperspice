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

#ifndef QWithParam_P_H
#define QWithParam_P_H

#include <qfunctionargument_p.h>

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

#endif
