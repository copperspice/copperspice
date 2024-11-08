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

#ifndef QAccelTree_P_H
#define QAccelTree_P_H

#include <qhash.h>
#include <qurl.h>
#include <qvector.h>
#include <qxmlname.h>

#include <qitem_p.h>
#include <qnamepool_p.h>

namespace QPatternist {

template<bool> class AccelTreeBuilder;

class AccelTree : public QAbstractXmlNodeModel
{
   friend class AccelTreePrivate;

 public:
   using QAbstractXmlNodeModel::createIndex;

   typedef QExplicitlySharedDataPointer<AccelTree> Ptr;
   typedef qint32 PreNumber;
   typedef PreNumber PostNumber;
   typedef qint8 Depth;

   AccelTree(const QUrl &docURI, const QUrl &bURI);

   class BasicNodeData
   {
    public:
      // No need to initialize the members. See AccelTreeBuilder.
      BasicNodeData() {
      }

      BasicNodeData(const PreNumber aDepth, const PreNumber aParent, const QXmlNodeModelIndex::NodeKind k,
            const PreNumber s, const QXmlName n = QXmlName())
         : m_parent(aParent), m_size(s), m_name(n), m_depth(aDepth), m_kind(k) {
      }

      Depth depth() const {
         return m_depth;
      }

      PreNumber parent() const {
         return m_parent;
      }

      PreNumber size() const {
         // Remember that we use the m_size to signal compression if we're a text node.
         if (m_kind == QXmlNodeModelIndex::Text) {
            return 0;
         } else {
            return m_size;
         }
      }

      void setSize(const PreNumber aSize) {
         m_size = aSize;
      }

      QXmlNodeModelIndex::NodeKind kind() const {
         return m_kind;
      }

      QXmlName name() const {
         return m_name;
      }

      bool isCompressed() const {
         Q_ASSERT_X(m_kind == QXmlNodeModelIndex::Text, Q_FUNC_INFO, "Only text nodes are compressed.");

         // we do not call size() here since it has logic for text nodes
         return m_size == IsCompressed;
      }

    private:

      PreNumber m_parent;
      PreNumber m_size;
      QXmlName m_name;

      Depth m_depth;

      QXmlNodeModelIndex::NodeKind m_kind : 8;
   };

   QUrl baseUri(const QXmlNodeModelIndex &ni) const override;
   QUrl documentUri(const QXmlNodeModelIndex &ni) const override;
   QXmlNodeModelIndex::NodeKind kind(const QXmlNodeModelIndex &ni) const override;
   QXmlNodeModelIndex::DocumentOrder compareOrder(const QXmlNodeModelIndex &ni1,
         const QXmlNodeModelIndex &ni2) const override;

   QXmlNodeModelIndex root(const QXmlNodeModelIndex &n) const override;

   virtual QXmlNodeModelIndex parent(const QXmlNodeModelIndex &ni) const;

   QXmlNodeModelIndex::Iterator::Ptr iterate(const QXmlNodeModelIndex &ni,
         QXmlNodeModelIndex::Axis axis) const override;

   QXmlName name(const QXmlNodeModelIndex &ni) const override;
   QVector<QXmlName> namespaceBindings(const QXmlNodeModelIndex &n) const override;
   void sendNamespaces(const QXmlNodeModelIndex &n, QAbstractXmlReceiver *const receiver) const override;
   QString stringValue(const QXmlNodeModelIndex &n) const override;
   QVariant typedValue(const QXmlNodeModelIndex &n) const override;
   Item::Iterator::Ptr sequencedTypedValue(const QXmlNodeModelIndex &n) const override;
   ItemType::Ptr type(const QXmlNodeModelIndex &ni) const override;
   QXmlNodeModelIndex elementById(const QXmlName &id) const override;
   QVector<QXmlNodeModelIndex> nodesByIdref(const QXmlName &idref) const override;

   void copyNodeTo(const QXmlNodeModelIndex &node, QAbstractXmlReceiver *const receiver,
                  const NodeCopySettings &settings) const override;

   friend class AccelTreeBuilder<false>;
   friend class AccelTreeBuilder<true>;

   enum Constants {
      IsCompressed = 1
   };

   QHash<PreNumber, QVector<QXmlName> > namespaces;
   QHash<PreNumber, QString> data;

   QVector<BasicNodeData> basicData;
   QHash<PreNumber, QPair<qint64, qint64> > sourcePositions;

   QUrl documentUri() const {
      return m_documentURI;
   }

   QUrl baseUri() const {
      return m_baseURI;
   }

   bool hasChildren(const PreNumber pre) const {
      return basicData.at(pre).size() > 0;
   }

   PreNumber parent(const PreNumber pre) const {
      return basicData.at(pre).parent();
   }

   bool hasParent(const PreNumber pre) const {
      return basicData.at(pre).depth() > 0;
   }

   bool hasFollowingSibling(const PreNumber pre) const {
      return pre < maximumPreNumber();
   }

   PostNumber postNumber(const PreNumber pre) const {
      const BasicNodeData &b = basicData.at(pre);
      return pre + b.size() - b.depth();
   }

   QXmlNodeModelIndex::NodeKind kind(const PreNumber pre) const {
      return basicData.at(pre).kind();
   }

   PreNumber maximumPreNumber() const {
      return basicData.count() - 1;
   }

   PreNumber toPreNumber(const QXmlNodeModelIndex n) const {
      return n.data();
   }

   PreNumber size(const PreNumber pre) const {
      Q_ASSERT_X(basicData.at(pre).size() != -1, Q_FUNC_INFO,
                 "The size cannot be -1. That means an uninitialized value is attempted to be used.");
      return basicData.at(pre).size();
   }

   Depth depth(const PreNumber pre) const {
      return basicData.at(pre).depth();
   }

   void printStats(const NamePool::Ptr &np) const;

   QXmlName name(const PreNumber pre) const {
      return basicData.at(pre).name();
   }

   bool isCompressed(const PreNumber pre) const {
      return basicData.at(pre).isCompressed();
   }

   static inline bool hasPrefix(const QVector<QXmlName> &nbs, const QXmlName::PrefixCode prefix);

   QUrl m_documentURI;
   QUrl m_baseURI;

 protected:
   QXmlNodeModelIndex nextFromSimpleAxis(QAbstractXmlNodeModel::SimpleAxis,
         const QXmlNodeModelIndex &) const override;

   QVector<QXmlNodeModelIndex> attributes(const QXmlNodeModelIndex &element) const override;

 private:
   QSourceLocation sourceLocation(const QXmlNodeModelIndex &index) const;

   inline void copyChildren(const QXmlNodeModelIndex &node,
         QAbstractXmlReceiver *const receiver, const NodeCopySettings &settings) const;

   QHash<QXmlName::LocalNameCode, PreNumber> m_IDs;
};

}

#endif
