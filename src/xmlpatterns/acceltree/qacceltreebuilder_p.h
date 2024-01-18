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

#ifndef QAccelTreeBuilder_P_H
#define QAccelTreeBuilder_P_H

#include <qdebug.h>
#include <qset.h>
#include <qstack.h>
#include <qstringfwd.h>

#include <qxmlutils_p.h>
#include <qacceltree_p.h>
#include <qbuiltintypes_p.h>
#include <qcompressedwhitespace_p.h>
#include <qnamepool_p.h>
#include <qnodebuilder_p.h>
#include <qreportcontext_p.h>
#include <qsourcelocationreflection_p.h>
#include <qpatternistlocale_p.h>

namespace QPatternist {

template<bool FromDocument>
class AccelTreeBuilder : public NodeBuilder, public SourceLocationReflection
{
 public:
   typedef QExplicitlySharedDataPointer<AccelTreeBuilder> Ptr;

   /**
    * Describes the memory relevant features the builder shall support.
    */
   enum Feature {
      NoneFeature,                ///< No special features are enabled.
      SourceLocationsFeature = 1  ///< The accel tree builder will store source locations for each start element.
   };
   using Features = QFlags<Feature>;

   /**
    * @param context may be @c null.
    */
   AccelTreeBuilder(const QUrl &docURI, const QUrl &baseURI, const NamePool::Ptr &np,
                  ReportContext *const context, Features features = NoneFeature);

   void startDocument() override;
   void endDocument() override;
   void startElement(const QXmlName &name) override;
   void startElement(const QXmlName &name, qint64 line, qint64 column);
   void endElement() override;
   void attribute(const QXmlName &name, QStringView value) override;
   void characters(QStringView ch) override;
   void whitespaceOnly(QStringView ch) override;
   void processingInstruction(const QXmlName &target, const QString &data) override;
   void namespaceBinding(const QXmlName &nb) override;
   void comment(const QString &content) override;
   void item(const Item &it) override;

   QAbstractXmlNodeModel::Ptr builtDocument() override;
   NodeBuilder::Ptr create(const QUrl &baseURI) const override;
   void startOfSequence() override;
   void endOfSequence() override;

   inline AccelTree::Ptr builtDocument() const {
      return m_document;
   }

   void atomicValue(const QVariant &value) override;

   const SourceLocationReflection *actualReflection() const override;
   QSourceLocation sourceLocation() const override;

 private:
   inline void startStructure();

   inline AccelTree::PreNumber currentDepth() const {
      return m_ancestors.count() - 1;
   }

   inline AccelTree::PreNumber currentParent() const {
      return m_ancestors.isEmpty() ? -1 : m_ancestors.top();
   }

   enum Constants {
      DefaultNodeStackSize = 10,
      SizeIsEmpty = 0
   };

   AccelTree::PreNumber            m_preNumber;
   bool                            m_isPreviousAtomic;
   bool                            m_hasCharacters;

   /**
    * Whether m_characters has been run through CompressedWhitespace::compress().
    */
   bool                            m_isCharactersCompressed;
   QString                         m_characters;
   NamePool::Ptr                   m_namePool;
   AccelTree::Ptr                  m_document;
   QStack<AccelTree::PreNumber>    m_ancestors;
   QStack<AccelTree::PreNumber>    m_size;

   /** If we have already commenced a document, we don't want to
    * add more document nodes. We keep track of them with this
    * counter, which ensures that startDocument() and endDocument() are skipped consistently.
   */
   AccelTree::PreNumber            m_skippedDocumentNodes;

   /**
    * All attribute values goes through this set such that we store only
    * one QString for identical attribute values.
    */
   QSet<QString>                   m_attributeCompress;
   const QUrl                      m_documentURI;

   /**
    * We do not store a reference pointer here because then we get a
    * circular reference with GenericDynamicContext, when it stores us as a member.
    */
   ReportContext *const            m_context;
   Features                        m_features;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AccelTreeBuilder<true>::Features)
Q_DECLARE_OPERATORS_FOR_FLAGS(AccelTreeBuilder<false>::Features)

#include "qacceltreebuilder.cpp"

}

#endif
