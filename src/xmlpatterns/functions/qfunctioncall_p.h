/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QFunctionCall_P_H
#define QFunctionCall_P_H

#include <qunlimitedcontainer_p.h>
#include <qfunctionsignature_p.h>
#include <qxpathhelper_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class FunctionCall : public UnlimitedContainer
{
 public:
   typedef QExplicitlySharedDataPointer<FunctionCall> Ptr;

   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;

   virtual void setSignature(const FunctionSignature::Ptr &sign);
   virtual FunctionSignature::Ptr signature() const;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   Expression::Properties properties() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   ID id() const override;

 private:
   FunctionSignature::Ptr m_signature;
};
}

QT_END_NAMESPACE

#endif
