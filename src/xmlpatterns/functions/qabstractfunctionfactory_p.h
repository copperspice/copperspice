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

#ifndef Patternist_AbstractFunctionFactory_H
#define Patternist_AbstractFunctionFactory_H

#include "qcommonnamespaces_p.h"
#include "qfunctionfactory_p.h"
#include "qfunctionsignature_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist
{
  class AbstractFunctionFactory : public FunctionFactory
    {
    public:
        virtual Expression::Ptr createFunctionCall(const QXmlName name,
                                                   const Expression::List &arguments,
                                                   const StaticContext::Ptr &context,
                                                   const SourceLocationReflection *const r);

        virtual FunctionSignature::Hash functionSignatures() const;

    protected:

        virtual Expression::Ptr retrieveExpression(const QXmlName name,
                                                   const Expression::List &args,
                                                   const FunctionSignature::Ptr &sign) const = 0;

        inline
        FunctionSignature::Ptr addFunction(const QXmlName::LocalNameCode localName,
                                           const FunctionSignature::Arity minArgs,
                                           const FunctionSignature::Arity maxArgs,
                                           const SequenceType::Ptr &returnType,
                                           const Expression::Properties props)
        {
            return addFunction(localName,
                               minArgs,
                               maxArgs,
                               returnType,
                               Expression::IDIgnorableExpression,
                               props);
        }

        FunctionSignature::Ptr addFunction(const QXmlName::LocalNameCode &localName,
                                           const FunctionSignature::Arity minArgs,
                                           const FunctionSignature::Arity maxArgs,
                                           const SequenceType::Ptr &returnType,
                                           const Expression::ID id = Expression::IDIgnorableExpression,
                                           const Expression::Properties props = Expression::Properties(),
                                           const StandardNamespaces::ID ns = StandardNamespaces::fn)
        {
            const QXmlName name(ns, localName);

            const FunctionSignature::Ptr s(new FunctionSignature(name, minArgs, maxArgs,returnType, props, id));

            m_signatures.insert(name, s);
            return s;
        }

        static inline QXmlName::LocalNameCode argument(const NamePool::Ptr &np, const char *const name)
        {
            return np->allocateLocalName(QLatin1String(name));
        }

        FunctionSignature::Hash m_signatures;

    private:
        void verifyArity(const FunctionSignature::Ptr &sign,
                         const StaticContext::Ptr &context,
                         const xsInteger arity,
                         const SourceLocationReflection *const r) const;

    };
}

QT_END_NAMESPACE

#endif
