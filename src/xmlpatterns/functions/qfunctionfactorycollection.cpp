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

#include "qbasictypesfactory_p.h"
#include "qconstructorfunctionsfactory_p.h"
#include "qfunctioncall_p.h"
#include "qxpath10corefunctions_p.h"
#include "qxpath20corefunctions_p.h"
#include "qxslt20corefunctions_p.h"

#include "qfunctionfactorycollection_p.h"

using namespace QPatternist;

Expression::Ptr FunctionFactoryCollection::createFunctionCall(const QXmlName name,
      const Expression::List &arguments,
      const StaticContext::Ptr &context,
      const SourceLocationReflection *const r)
{
   const_iterator it;
   const_iterator e(constEnd());
   Expression::Ptr function;

   for (it = constBegin(); it != e; ++it) {
      function = (*it)->createFunctionCall(name, arguments, context, r);

      if (function) {
         break;
      }
   }

   return function;
}

bool FunctionFactoryCollection::isAvailable(const NamePool::Ptr &np, const QXmlName name, const xsInteger arity)
{
   const_iterator it;
   const_iterator e(constEnd());

   for (it = constBegin(); it != e; ++it)
      if ((*it)->isAvailable(np, name, arity)) {
         return true;
      }

   return false;
}

FunctionSignature::Hash FunctionFactoryCollection::functionSignatures() const
{
   /* We simply grab the function signatures for each library, and
    * put them all in one list. */

   const const_iterator e(constEnd());
   FunctionSignature::Hash result;

   for (const_iterator it(constBegin()); it != e; ++it) {
      auto tmp = (*it)->functionSignatures();

      const FunctionSignature::Hash::const_iterator e2(tmp.constEnd());
      FunctionSignature::Hash::const_iterator sit(tmp.constBegin());

      for (; sit != e2; ++sit) {
         result.insert(sit.key(), sit.value());
      }
   }

   return result;
}

FunctionSignature::Ptr FunctionFactoryCollection::retrieveFunctionSignature(const NamePool::Ptr &, const QXmlName name)
{
   return functionSignatures().value(name);
}

FunctionFactory::Ptr FunctionFactoryCollection::xpath10Factory()
{
   /* We don't use a global static for caching this, because AbstractFunctionFactory
    * stores state specific to the NamePool, when being used. */
   return  FunctionFactory::Ptr(new XPath10CoreFunctions());
}

FunctionFactory::Ptr FunctionFactoryCollection::xpath20Factory(const NamePool::Ptr &np)
{
   /* We don't use a global static for caching this, because AbstractFunctionFactory
    * stores state specific to the NamePool, when being used. */
   const FunctionFactoryCollection::Ptr fact(new FunctionFactoryCollection());
   fact->append(xpath10Factory());
   fact->append(FunctionFactory::Ptr(new XPath20CoreFunctions()));
   fact->append(FunctionFactory::Ptr(
                   new ConstructorFunctionsFactory(np, BasicTypesFactory::self(np))));
   return fact;
}

FunctionFactory::Ptr FunctionFactoryCollection::xslt20Factory(const NamePool::Ptr &np)
{
   const FunctionFactory::Ptr retval(xpath20Factory(np));
   static_cast<FunctionFactoryCollection *>(retval.data())->append(FunctionFactory::Ptr(new XSLT20CoreFunctions()));
   return retval;
}
