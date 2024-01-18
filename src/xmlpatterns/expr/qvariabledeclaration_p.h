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

#ifndef QVariableDeclaration_P_H
#define QVariableDeclaration_P_H

#include <QSharedData>
#include <qexpression_p.h>
#include <qpatternistlocale_p.h>
#include <qvariablereference_p.h>

template<typename T> class QStack;

namespace QPatternist {
class VariableDeclaration : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<VariableDeclaration> Ptr;
   typedef QStack<VariableDeclaration::Ptr> Stack;
   typedef QList<VariableDeclaration::Ptr> List;


   typedef QHash<QXmlName, VariableDeclaration::Ptr> Hash;

   enum Type {
      RangeVariable,
      ExpressionVariable,
      FunctionArgument,
      PositionalVariable,
      TemplateParameter,
      GlobalVariable,
      ExternalVariable
   };


   VariableDeclaration(const QXmlName n,
                       const VariableSlotID varSlot,
                       const Type t,
                       const SequenceType::Ptr &seqType) : name(n)
      , slot(varSlot)
      , type(t)
      , sequenceType(seqType)
      , canSourceRewrite(true) {
      Q_ASSERT(!name.isNull());
      Q_ASSERT(t == ExternalVariable || t == TemplateParameter || varSlot > -1);
   }

   inline bool isUsed() const {
      return !references.isEmpty();
   }

   inline const Expression::Ptr &expression() const {
      return m_expression;
   }

   inline void setExpression(const Expression::Ptr &expr) {
      m_expression = expr;
   }

   /**
    * @short Returns how many times this variable is used.
    */
   inline bool usedByMany() const {
      return references.count() > 1;
   }

   /**
    * @short Returns @c true if @p list contains @p lookup.
    */
   static bool contains(const VariableDeclaration::List &list,
                        const QXmlName &lookup);

   const QXmlName                  name;
   const VariableSlotID            slot;
   const Type                      type;

   /**
    * The declared type of the variable. What the value might be, depends
    * on the context which VariableDeclaration is used in. Note that
    * sequenceType is hence not in anyway obligated to the type of
    * expression().
    */
   const SequenceType::Ptr         sequenceType;
   VariableReference::List         references;

   /**
    * @short Whether a reference can rewrite itself to expression().
    *
    * The default value is @c true.
    */
   bool canSourceRewrite;

 private:
   Expression::Ptr m_expression;

   VariableDeclaration(const VariableDeclaration &) = delete;
   VariableDeclaration &operator=(const VariableDeclaration &) = delete;
};

/**
 * @short Formats @p var appropriately for display.
 *
 * @relates VariableDeclaration
 */
static inline QString formatKeyword(const VariableDeclaration::Ptr &var,
                                    const NamePool::Ptr &np)
{
   Q_ASSERT(var);
   Q_ASSERT(np);
   return formatKeyword(np->displayName(var->name));
}

}

#endif
