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

   VariableSlotID allocatePositionalSlot() {
      ++m_positionSlot;
      return m_positionSlot;
   }

   VariableSlotID allocateExpressionSlot() {
      const VariableSlotID retval = m_expressionSlot;
      ++m_expressionSlot;
      return retval;
   }

   VariableSlotID allocateGlobalVariableSlot() {
      ++m_globalVariableSlot;
      return m_globalVariableSlot;
   }

   bool hasDeclaration(const PrologDeclaration decl) const {
      return m_prologDeclarations.testFlag(decl);
   }

   void registerDeclaration(const PrologDeclaration decl) {
      m_prologDeclarations |= decl;
   }

   QStringList declaredPrefixes;

   VariableDeclaration::Stack variables;

   bool isXSLT() const {
      return languageAccent == QXmlQuery::XSLT20;
   }

   const StaticContext::Ptr staticContext;

   Tokenizer *const tokenizer;
   const QXmlQuery::QueryLanguage languageAccent;

   QXmlName initialTemplateName;

   QStack<QXmlName> tagStack;

   Expression::Ptr queryBody;

   UserFunction::List userFunctions;
   UserFunctionCallsite::List userFunctionCallsites;

   VariableDeclaration::List declaredVariables;

   VariableSlotID currentPositionSlot() const {
      return m_positionSlot;
   }

   VariableSlotID currentExpressionSlot() const {
      return m_expressionSlot;
   }

   void restoreNodeTestSource() {
      nodeTestSource = BuiltinTypes::element;
   }

   VariableSlotID allocateCacheSlot() {
      return ++m_evaluationCacheSlot;
   }

   VariableSlotID allocateCacheSlots(const int count) {
      const VariableSlotID retval = m_evaluationCacheSlot + 1;
      m_evaluationCacheSlot += count + 1;
      return retval;
   }

   ItemType::Ptr nodeTestSource;

   QStack<Expression::Ptr> typeswitchSource;

   QXmlName::NamespaceCode moduleNamespace;

   QStack<NamespaceResolver::Ptr> resolvers;

   bool isPreviousEnclosedExpr;
   int elementConstructorDepth;

   QStack<bool> scanOnlyStack;
   QStack<OrderBy::Stability> orderStability;

   bool hasSecondPrologPart;

   bool preserveNamespacesMode;
   bool inheritNamespacesMode;

   QHash<QXmlName, Template::Ptr>  namedTemplates;
   QVector<Expression::Ptr>        templateCalls;

   QMultiHash<QXmlName, Expression::Ptr> unresolvedVariableReferences;

   QHash<QXmlName, TemplateMode::Ptr>  templateRules;

   TemplateMode::Ptr modeFor(const QXmlName &modeName) {
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

   TemplatePattern::ID allocateTemplateID() {
      ++m_currentTemplateID;
      return m_currentTemplateID;
   }

   VariableDeclaration::List templateParameters;
   WithParam::Hash templateWithParams;

   void templateParametersHandled() {
      finalizePushedVariable(templateParameters.count());
      templateParameters.clear();
   }

   void templateWithParametersHandled() {
      templateWithParams.clear();
   }

   bool isParsingWithParam() const {
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
