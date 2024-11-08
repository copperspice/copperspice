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

#ifndef QFunctionSignature_P_H
#define QFunctionSignature_P_H

#include <QSharedData>

#include <qcalltargetdescription_p.h>
#include <qexpression_p.h>
#include <qfunctionargument_p.h>
#include <qpatternistlocale_p.h>
#include <qprimitives_p.h>
#include <qcontainerfwd.h>

namespace QPatternist {

class FunctionSignature : public CallTargetDescription
{
 public:
   enum {
      UnlimitedArity = -1
   };

   using Ptr   = QExplicitlySharedDataPointer<FunctionSignature>;
   using Hash  = QHash<QXmlName, FunctionSignature::Ptr>;
   using List  = QList<FunctionSignature::Ptr>;

   typedef qint16 Arity;

   FunctionSignature(const QXmlName name, const Arity minArgs, const Arity maxArgs, const SequenceType::Ptr &returnType,
                     const Expression::Properties chars = Expression::Properties(),
                     const Expression::ID id = Expression::IDIgnorableExpression);

   void setArguments(const FunctionArgument::List &args);
   FunctionArgument::List arguments() const;

   void appendArgument(const QXmlName::LocalNameCode name, const SequenceType::Ptr &type);
   bool isArityValid(const xsInteger arity) const;

   Arity minimumArguments() const;
   Arity maximumArguments() const;

   SequenceType::Ptr returnType() const;

   Expression::Properties properties() const;

   bool operator==(const FunctionSignature &other) const;

   QString displayName(const NamePool::Ptr &np) const;

   Expression::ID id() const;

 private:
   FunctionSignature(const FunctionSignature &) = delete;
   FunctionSignature &operator=(const FunctionSignature &) = delete;

   const Arity                     m_minArgs;
   const Arity                     m_maxArgs;
   const SequenceType::Ptr         m_returnType;
   FunctionArgument::List          m_arguments;
   const Expression::Properties    m_props;
   const Expression::ID            m_id;
};

static inline QString formatFunction(const NamePool::Ptr &np, const FunctionSignature::Ptr &func)
{
   return QLatin1String("<span class='XQuery-function'>")  + escape(func->displayName(np)) + QLatin1String("</span>");
}

}

#endif
