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

#ifndef QAggregateFNs_P_H
#define QAggregateFNs_P_H

#include <qaggregator_p.h>
#include <qatomiccomparator_p.h>
#include <qatomicmathematician_p.h>
#include <qcomparisonplatform_p.h>

namespace QPatternist {

class CountFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;
   Expression::Ptr compress(const StaticContext::Ptr &context) override;
};

class AddingAggregate : public FunctionCall
{
 public:
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

 protected:
   AtomicMathematician::Ptr m_mather;
};

class AvgFN : public AddingAggregate
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   SequenceType::Ptr staticType() const override;

 private:
   AtomicMathematician::Ptr m_adder;
   AtomicMathematician::Ptr m_divider;
};

class SumFN : public AddingAggregate
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;
   SequenceType::Ptr staticType() const override;
};

}

#endif
