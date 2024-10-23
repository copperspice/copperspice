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

   virtual bool isAvailable(const NamePool::Ptr &np, const QXmlName name, const xsInteger arity);

   virtual FunctionSignature::Hash functionSignatures() const = 0;

   bool hasSignature(const FunctionSignature::Ptr &signature) const;

 protected:
   FunctionFactory() {
   }

   virtual FunctionSignature::Ptr retrieveFunctionSignature(const NamePool::Ptr &np, const QXmlName name) = 0;

 private:
   FunctionFactory(const FunctionFactory &) = delete;
   FunctionFactory &operator=(const FunctionFactory &) = delete;
};

}

#endif
