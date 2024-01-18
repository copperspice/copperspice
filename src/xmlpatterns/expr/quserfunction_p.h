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

#ifndef QUserFunction_P_H
#define QUserFunction_P_H

template<typename T>
class QList;

#include <QSharedData>

#include <qexpression_p.h>
#include <qfunctionsignature_p.h>
#include <qvariabledeclaration_p.h>

namespace QPatternist {
class UserFunction : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<UserFunction> Ptr;
   typedef QList<UserFunction::Ptr> List;

   /**
    * If @p slotOffset is -1, it means this function has no arguments.
    */
   UserFunction(const FunctionSignature::Ptr &signature, const Expression::Ptr &body,
                const VariableSlotID slotOffset, const VariableDeclaration::List &varDecls);

   inline const Expression::Ptr &body() const;
   inline void setBody(const Expression::Ptr &newBody);
   inline FunctionSignature::Ptr signature() const;
   inline VariableSlotID expressionSlotOffset() const;
   inline VariableDeclaration::List argumentDeclarations() const;

 private:
   const FunctionSignature::Ptr    m_signature;
   Expression::Ptr                 m_body;
   const VariableSlotID            m_slotOffset;
   const VariableDeclaration::List m_argumentDeclarations;
};

inline const Expression::Ptr &UserFunction::body() const
{
   return m_body;
}

inline FunctionSignature::Ptr UserFunction::signature() const
{
   return m_signature;
}

inline VariableSlotID UserFunction::expressionSlotOffset() const
{
   return m_slotOffset;
}

inline VariableDeclaration::List UserFunction::argumentDeclarations() const
{
   return m_argumentDeclarations;
}

void UserFunction::setBody(const Expression::Ptr &newBody)
{
   m_body = newBody;
}

}

#endif
