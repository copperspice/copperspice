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

#ifndef QXSLTTokenizer_P_H
#define QXSLTTokenizer_P_H

#include <qqueue.h>
#include <qstack.h>
#include <qurl.h>

#include <qmaintainingreader_p.h>
#include <qreportcontext_p.h>
#include <qtokenizer_p.h>
#include <qxslttokenlookup_p.h>

namespace QPatternist {

class SingleTokenContainer : public TokenSource
{
 public:
   inline SingleTokenContainer(const Tokenizer::Token &token, const YYLTYPE &location);

   Tokenizer::Token nextToken(YYLTYPE *const sourceLocator) override;

 private:
   const Tokenizer::Token m_token;
   const YYLTYPE          m_location;
   bool                   m_hasDelivered;
};

SingleTokenContainer::SingleTokenContainer(const Tokenizer::Token &token,
      const YYLTYPE &location) : m_token(token), m_location(location), m_hasDelivered(false)
{
}

class XSLTTokenizer : public Tokenizer, private MaintainingReader<XSLTTokenLookup>
{
 public:
   XSLTTokenizer(QIODevice *const queryDevice, const QUrl &location, const ReportContext::Ptr &context,
                 const NamePool::Ptr &np);

   Token nextToken(YYLTYPE *const sourceLocator) override;

   int commenceScanOnly() override;
   void resumeTokenizationFrom(const int position) override;
   void setParserContext(const ParserContext::Ptr &parseInfo) override;

   QUrl documentURI() const override {
      return queryURI();
   }

 protected:
   bool isAnyAttributeAllowed() const override;

 private:
   inline void validateElement() const;

   YYLTYPE currentSourceLocator() const;

   enum State {
      OutsideDocumentElement,
      InsideStylesheetModule,
      InsideSequenceConstructor
   };

   enum VariableType {
      FunctionParameter,
      GlobalParameter,
      TemplateParameter,
      VariableDeclaration,
      VariableInstruction,
      WithParamVariable
   };

   void queueNamespaceDeclarations(TokenSource::Queue *const ts, QStack<Token> *const target,
                  const bool isDeclaration = false);

   inline void queueToken(const Token &token, TokenSource::Queue *const ts);
   void queueEmptySequence(TokenSource::Queue *const to);
   void queueSequenceType(const QString &expr);

   void queueSimpleContentConstructor(const ReportContext::ErrorCode code,
                  const bool emptynessAllowed, TokenSource::Queue *const to, const bool selectOnlyFirst = false);

   void queueAVT(const QString &expr, TokenSource::Queue *const to);

   void hasWrittenExpression(bool &beacon);
   void commencingExpression(bool &hasWrittenExpression, TokenSource::Queue *const to);

   void outsideDocumentElement();
   void insideChoose(TokenSource::Queue *const to);
   void insideFunction();

   bool attributeYesNo(const QString &localName) const;

   void parseFallbacksOnly();
   bool isStylesheetElement() const;

   bool isElement(const NodeName &name) const;

   void queueTextConstructor(QString &chars, bool &hasWrittenExpression, TokenSource::Queue *const to);

   void insideStylesheetModule();
   void insideTemplate();

   void queueExpression(const QString &expr, TokenSource::Queue *const to, const bool wrapWithParantheses = true);
   void skipBodyOfParam(const ReportContext::ErrorCode code);

   void queueParams(const NodeName parentName, TokenSource::Queue *const to);

   void queueWithParams(const NodeName parentName,
                        TokenSource::Queue *const to,
                        const bool initialAdvance = true);

   void queueVariableDeclaration(const VariableType variableType,
                                 TokenSource::Queue *const to);

   bool skipSubTree(const bool exitOnContent = false);

   bool queueSelectOrSequenceConstructor(const ReportContext::ErrorCode code,
                                         const bool emptynessAllowed,
                                         TokenSource::Queue *const to,
                                         const QXmlStreamAttributes *const atts = nullptr,
                                         const bool queueEmptyOnEmpty = true);

   bool insideSequenceConstructor(TokenSource::Queue *const to,
                                  const bool initialAdvance = true,
                                  const bool queueEmptyOnEmpty = true);

   bool insideSequenceConstructor(TokenSource::Queue *const to,
                                  QStack<Token> &queueOnExit,
                                  const bool initialAdvance = true,
                                  const bool queueEmptyOnEmpty = true);

   void insideAttributeSet();
   void pushState(const State nextState);
   void leaveState();

   void handleStandardAttributes(const bool isXSLTElement);

   inline void queueOnExit(QStack<Token> &source,
                           TokenSource::Queue *const destination);

   void handleValidationAttributes(const bool isLRE) const;

   void unexpectedContent(const ReportContext::ErrorCode code = ReportContext::XTSE0010) const;

   void checkForParseError() const;

   inline void startStorageOfCurrent(TokenSource::Queue *const to);
   inline void endStorageOfCurrent(TokenSource::Queue *const to);

   void handleXSLTVersion(TokenSource::Queue *const to,
                          QStack<Token> *const queueOnExit,
                          const bool isXSLTElement,
                          const QXmlStreamAttributes *atts = nullptr,
                          const bool generateCode = true,
                          const bool setGlobalVersion = false);

   void handleXMLBase(TokenSource::Queue *const to,
                      QStack<Token> *const queueOnExit,
                      const bool isInstruction = true,
                      const QXmlStreamAttributes *atts = nullptr);


   QString readElementText();

   void queueSorting(const bool oneSortRequired, TokenSource::Queue *const to, const bool speciallyTreatWhitespace = false);

   static ElementDescription<XSLTTokenLookup>::Hash createElementDescriptions();
   static QHash<QString, int> createValidationAlternatives();
   static QSet<NodeName> createStandardAttributes();

   bool readToggleAttribute(const QString &attributeName, const QString &isTrue, const QString &isFalse,
                            const QXmlStreamAttributes *const atts = nullptr) const;

   int readAlternativeAttribute(const QHash<QString, int> &alternatives, const QXmlStreamAttribute &attr) const;

   inline bool whitespaceToSkip() const;

   const QUrl                                  m_location;
   const NamePool::Ptr                         m_namePool;
   QStack<State>                               m_state;
   TokenSource::Queue                          m_tokenSource;

   enum ProcessMode {
      BackwardsCompatible,
      ForwardCompatible,
      NormalProcessing
   };

   QStack<ProcessMode> m_processingMode;

   inline bool isXSLT() const;

   const QHash<QString, int>  m_validationAlternatives;
   ParserContext::Ptr m_parseInfo;
};

}

#endif
