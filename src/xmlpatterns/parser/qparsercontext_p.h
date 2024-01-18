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

#ifndef QParserContext_P_H
#define QParserContext_P_H

#include <qflags.h>
#include <qglobal.h>
#include <qhash.h>
#include <qmultihash.h>
#include <qshareddata.h>
#include <qstack.h>
#include <qstringlist.h>
#include <qxmlquery.h>

#include <qbuiltintypes_p.h>
#include <qfunctionsignature_p.h>
#include <qorderby_p.h>
#include <qtemplatemode_p.h>
#include <quserfunctioncallsite_p.h>
#include <quserfunction_p.h>
#include <qvariabledeclaration_p.h>

namespace QPatternist {
class Tokenizer;

class ParserContext : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<ParserContext> Ptr;

   enum PrologDeclaration {
      BoundarySpaceDecl               = 1,
      DefaultCollationDecl            = 2,
      BaseURIDecl                     = 4,
      ConstructionDecl                = 8,
      OrderingModeDecl                = 16,
      EmptyOrderDecl                  = 32,
      CopyNamespacesDecl              = 64,
      DeclareDefaultElementNamespace  = 128,
      DeclareDefaultFunctionNamespace = 256
   };

   typedef QFlags<PrologDeclaration> PrologDeclarations;

   ParserContext(const StaticContext::Ptr &context,
                 const QXmlQuery::QueryLanguage lang,
                 Tokenizer *const tokenizer);

   void finalizePushedVariable(const int amount = 1, const bool shouldPop = true);

   inline VariableSlotID allocatePositionalSlot() {
      ++m_positionSlot;
      return m_positionSlot;
   }

   inline VariableSlotID allocateExpressionSlot() {
      const VariableSlotID retval = m_expressionSlot;
      ++m_expressionSlot;
      return retval;
   }

   inline VariableSlotID allocateGlobalVariableSlot() {
      ++m_globalVariableSlot;
      return m_globalVariableSlot;
   }

   inline bool hasDeclaration(const PrologDeclaration decl) const {
      return m_prologDeclarations.testFlag(decl);
   }

   inline void registerDeclaration(const PrologDeclaration decl) {
      m_prologDeclarations |= decl;
   }

   /**
    * The namespaces declared with <tt>declare namespace</tt>.
    */
   QStringList declaredPrefixes;

   /**
    * This is a temporary stack, used for keeping variables in scope,
    * such as for function arguments & let clauses.
    */
   VariableDeclaration::Stack variables;

   inline bool isXSLT() const {
      return languageAccent == QXmlQuery::XSLT20;
   }

   const StaticContext::Ptr staticContext;
   /**
    * We don't store a Tokenizer::Ptr here, because then we would get a
    * circular referencing between ParserContext and XSLTTokenizer, and
    * hence they would never destruct.
    */
   Tokenizer *const tokenizer;
   const QXmlQuery::QueryLanguage languageAccent;

   /**
    * Only used in the case of XSL-T. Is the name of the initial template
    * to call. If null, no name was provided, and regular template
    * matching should be done.
    */
   QXmlName initialTemplateName;

   /**
    * Used when parsing direct element constructors. It is used
    * for ensuring tags are well-balanced.
    */
   QStack<QXmlName> tagStack;

   Expression::Ptr queryBody;

   /**
    * The user functions declared in the prolog.
    */
   UserFunction::List userFunctions;

   /**
    * Contains all calls to user defined functions.
    */
   UserFunctionCallsite::List userFunctionCallsites;

   /**
    * All variables declared with <tt>declare variable</tt>.
    */
   VariableDeclaration::List declaredVariables;

   inline VariableSlotID currentPositionSlot() const {
      return m_positionSlot;
   }

   inline VariableSlotID currentExpressionSlot() const {
      return m_expressionSlot;
   }

   inline void restoreNodeTestSource() {
      nodeTestSource = BuiltinTypes::element;
   }

   inline VariableSlotID allocateCacheSlot() {
      return ++m_evaluationCacheSlot;
   }

   inline VariableSlotID allocateCacheSlots(const int count) {
      const VariableSlotID retval = m_evaluationCacheSlot + 1;
      m_evaluationCacheSlot += count + 1;
      return retval;
   }

   ItemType::Ptr nodeTestSource;

   QStack<Expression::Ptr> typeswitchSource;

   /**
    * The library module namespace set with <tt>declare module</tt>.
    */
   QXmlName::NamespaceCode moduleNamespace;

   /**
    * When a direct element constructor is processed, resolvers are
    * created in order to carry the namespace declarations. In such case,
    * the old resolver is pushed here.
    */
   QStack<NamespaceResolver::Ptr> resolvers;

   /**
    * This is used for handling the following obscene case:
    *
    * - <tt>\<e\>{1}{1}\<\/e\></tt> produce <tt>\<e\>11\</e\></tt>
    * - <tt>\<e\>{1, 1}\<\/e\></tt> produce <tt>\<e\>1 1\</e\></tt>
    *
    * This boolean tracks whether the previous reduction inside element
    * content was done with an enclosed expression.
    */
   bool isPreviousEnclosedExpr;

   int elementConstructorDepth;

   QStack<bool> scanOnlyStack;

   QStack<OrderBy::Stability> orderStability;

   /**
    * Whether any prolog declaration that must occur after the first
    * group has been encountered.
    */
   bool hasSecondPrologPart;

   bool preserveNamespacesMode;
   bool inheritNamespacesMode;

   /**
    * Contains all named templates. Since named templates
    * can also have rules, each body may also be in templateRules.
    */
   QHash<QXmlName, Template::Ptr>  namedTemplates;
   QVector<Expression::Ptr>        templateCalls;

   /**
    * If we're in XSL-T, and a variable reference is encountered
    * which isn't in-scope, it's added to this hash since a global
    * variable declaration may appear later on.
    *
    * We use a multi hash, since we can encounter several references to
    * the same variable before it's declared.
    */
   QMultiHash<QXmlName, Expression::Ptr> unresolvedVariableReferences;

   /**
    *
    * Contains the encountered template rules, as opposed
    * to named templates.
    *
    * The key is the name of the template mode. If it's a default
    * constructed value, it's the default mode.
    *
    * Since templates rules may also be named, each body may also be in
    * namedTemplates.
    *
    * To be specific, the values are not the templates, the values are
    * modes, and the TemplateMode contains the patterns and bodies.
    */
   QHash<QXmlName, TemplateMode::Ptr>  templateRules;

   TemplateMode::Ptr modeFor(const QXmlName &modeName) {
      /* #current is not a mode, so it cannot contain templates. #current
       * specifies how to look up templates wrt. mode. This check helps
       * code that calls us, asking for the mode it needs to lookup in.
       */
      if (modeName == QXmlName(StandardNamespaces::InternalXSLT, StandardLocalNames::current)) {
         return TemplateMode::Ptr();
      }

      TemplateMode::Ptr &mode = templateRules[modeName];

      if (!mode) {
         mode = TemplateMode::Ptr(new TemplateMode(modeName));
      }

      Q_ASSERT(templateRules[modeName]);
      return mode;
   }

   inline TemplatePattern::ID allocateTemplateID() {
      ++m_currentTemplateID;
      return m_currentTemplateID;
   }

   VariableDeclaration::List templateParameters;
   WithParam::Hash templateWithParams;

   inline void templateParametersHandled() {
      finalizePushedVariable(templateParameters.count());
      templateParameters.clear();
   }

   inline void templateWithParametersHandled() {
      templateWithParams.clear();
   }

   inline bool isParsingWithParam() const {
      return m_isParsingWithParam.top();
   }

   void startParsingWithParam() {
      m_isParsingWithParam.push(true);
   }

   void endParsingWithParam() {
      m_isParsingWithParam.pop();
   }

   bool isParsingPattern;
   ImportPrecedence currentImportPrecedence;

   bool isFirstTemplate() const {
      return m_currentTemplateID == InitialTemplateID;
   }

   /**
    * Whether we're processing XSL-T 1.0 code.
    */
   QStack<bool> isBackwardsCompat;

 private:
   enum {
      InitialTemplateID = -1
   };

   VariableSlotID                      m_evaluationCacheSlot;
   VariableSlotID                      m_expressionSlot;
   VariableSlotID                      m_positionSlot;
   PrologDeclarations                  m_prologDeclarations;
   VariableSlotID                      m_globalVariableSlot;
   TemplatePattern::ID                 m_currentTemplateID;

   QStack<bool>                        m_isParsingWithParam;

   ParserContext(const ParserContext &) = delete;
   ParserContext &operator=(const ParserContext &) = delete;
};
}

#endif
