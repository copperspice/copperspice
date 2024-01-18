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

#ifndef QFunctionFactoryCollection_P_H
#define QFunctionFactoryCollection_P_H

#include <qfunctionfactory_p.h>

namespace QPatternist {

class FunctionFactoryCollection: public FunctionFactory, public FunctionFactory::List
{
 public:

   typedef QExplicitlySharedDataPointer<FunctionFactoryCollection> Ptr;

   /**
    * Creates a function call node.
    */
   Expression::Ptr createFunctionCall(const QXmlName, const Expression::List &arguments,
                  const StaticContext::Ptr &context, const SourceLocationReflection *const r) override;

   bool isAvailable(const NamePool::Ptr &np, const QXmlName name, const xsInteger arity) override;

   FunctionSignature::Hash functionSignatures() const override;

   FunctionSignature::Ptr retrieveFunctionSignature(const NamePool::Ptr &np, const QXmlName name) override;

   /**
    * @return a FunctionFactory containing all core functions and constructor
    * functions required for XPath 2.. The functions specified for XQuery 1.0
    * are the same as for XPath 2.0 so this FunctionFactory work for XQuery
    * as well.
    */
   static FunctionFactory::Ptr xpath20Factory(const NamePool::Ptr &np);

   /**
    * @return a FunctionFactory containing all core functions required for XPath 1.0.
    */
   static FunctionFactory::Ptr xpath10Factory();

   /**
    * @return a FunctionFactory containing all core functions required for XSL-T 2.0
    * functions.
    */
   static FunctionFactory::Ptr xslt20Factory(const NamePool::Ptr &np);
};

}

#endif
