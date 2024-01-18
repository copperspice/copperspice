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
      /**
       * Flags the function as allowing an unlimited amount of arguments.
       */
      UnlimitedArity = -1
   };

   using Ptr   = QExplicitlySharedDataPointer<FunctionSignature>;
   using Hash  = QHash<QXmlName, FunctionSignature::Ptr>;
   using List  = QList<FunctionSignature::Ptr>;

   /**
    * A number which tells the amount of arguments a function has.
    */
   typedef qint16 Arity;

   FunctionSignature(const QXmlName name, const Arity minArgs, const Arity maxArgs, const SequenceType::Ptr &returnType,
                     const Expression::Properties chars = Expression::Properties(),
                     const Expression::ID id = Expression::IDIgnorableExpression);

   void setArguments(const FunctionArgument::List &args);
   FunctionArgument::List arguments() const;

   /**
    * This is a convenience function. Calling this once, is equal to
    * calling setArguments() with a list containing a FunctionsArgument with name @p name
    * and type @p type.
    */
   void appendArgument(const QXmlName::LocalNameCode name, const SequenceType::Ptr &type);

   /**
    * Checks whether @p arity is within the range of allowed count of arguments. For example,
    * when the minimum arguments is 1 and maximum arguments 2, @c false will be returned for
    * passing 0 while @c true will be returned when 2 is passed.
    */
   bool isArityValid(const xsInteger arity) const;

   Arity minimumArguments() const;
   Arity maximumArguments() const;

   /**
    * The return type of this function signature. For example, if the represented function
    * signature is <tt>fn:string() as xs:string</tt>, the return type is <tt>xs:string</tt>.
    */
   SequenceType::Ptr returnType() const;

   /**
    * The properties that the corresponding FunctionCall instance should return in
    * Expression::properties().
    */
   Expression::Properties properties() const;

   /**
    * Determines whether this FunctionSignature is equal to @p other, taking
    * into account XPath's function polymorphism. @p other is equal to this
    * FunctionSignature if their name() instances are equal, and that the maximumArguments()
    * and minimumArguments() arguments of @p other are allowed, as per isArityValid().
    *
    * In other words, this equalness operator can return @c true for different
    * signatures, but it do make sense since a FunctionSignature can represent
    * multiple signatures.
    *
    * @returns @c true if this FunctionSignature is equal to @p other, otherwise @c false
    */
   bool operator==(const FunctionSignature &other) const;

   /**
    * Builds a string representation for this function signature. The syntax
    * used is the one used in the XQuery. It looks like this:
    *
    * <tt>prefix:function-name($parameter-name as parameter-type, ...) as return-type</tt>
    *
    * The prefix used for the name is conventional. For example, for constructor functions
    * is @c xs used.
    *
    * @see <a href="http://www.w3.org/TR/xpath-functions/#func-signatures">XQuery 1.0 and
    * XPath 2.0 Functions and Operators, 1.4 Function Signatures and Descriptions</a>
    */
   QString displayName(const NamePool::Ptr &np) const;

   /**
    * The ID that the corresponding FunctionCall instance should return in
    * Expression::id().
    */
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

/**
 * @short Formats FunctionSignature.
 */
static inline QString formatFunction(const NamePool::Ptr &np, const FunctionSignature::Ptr &func)
{
   return QLatin1String("<span class='XQuery-function'>")  + escape(func->displayName(np)) + QLatin1String("</span>");
}
}

#endif
