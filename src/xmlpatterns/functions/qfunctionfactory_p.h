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

#ifndef QFunctionFactory_P_H
#define QFunctionFactory_P_H

#include <QHash>
#include <QSharedData>
#include <qexpression_p.h>
#include <qfunctionsignature_p.h>
#include <qprimitives_p.h>
#include <qxmlname.h>

namespace QPatternist {

class FunctionFactory : public QSharedData
{
 public:

   typedef QExplicitlySharedDataPointer<FunctionFactory> Ptr;
   typedef QList<FunctionFactory::Ptr> List;

   virtual ~FunctionFactory();

   virtual Expression::Ptr createFunctionCall(const QXmlName name,
         const Expression::List &arguments,
         const StaticContext::Ptr &context,
         const SourceLocationReflection *const r) = 0;

   /**
    * Determines whether a function with the name @p name and arity @p arity
    * is available. The implementation operates on the result of
    * retrieveFunctionSignature() to determine the result.
    *
    * @param np the NamePool.
    * @param name the name of the function. For example fn:string-join.
    * @param arity the number of arguments the function must have.
    */
   virtual bool isAvailable(const NamePool::Ptr &np, const QXmlName name, const xsInteger arity);

   virtual FunctionSignature::Hash functionSignatures() const = 0;

   /**
    * Determines whether this FunctionFactory contains the function signature
    * @p signature.
    *
    * The implementation uses functionSignatures().
    */
   bool hasSignature(const FunctionSignature::Ptr &signature) const;

 protected:
   /**
    * @short This constructor cannot be removed, because it can't be synthesized, for
    * some reason.
    */
   inline FunctionFactory() {
   }

   /**
    * This is a convenience function for sub-classes. It retrieves the
    * function signature for function with name @p name.
    *
    * According to the specifications are function signatures identified by their
    * name and arity, but currently is the arity not part of the signature.
    *
    * If no function could be found for the given name, @c null is returned.
    */
   virtual FunctionSignature::Ptr retrieveFunctionSignature(const NamePool::Ptr &np, const QXmlName name) = 0;

 private:
   FunctionFactory(const FunctionFactory &) = delete;
   FunctionFactory &operator=(const FunctionFactory &) = delete;
};

}

#endif
