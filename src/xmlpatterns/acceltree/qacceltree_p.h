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
      /* No need to initialize the members. See AccelTreeBuilder. */
      inline BasicNodeData() {
      }

      inline BasicNodeData(const PreNumber aDepth,
                           const PreNumber aParent,
                           const QXmlNodeModelIndex::NodeKind k,
                           const PreNumber s,
                           const QXmlName n = QXmlName()) : m_parent(aParent)
         , m_size(s)
         , m_name(n)
         , m_depth(aDepth)
         , m_kind(k) {
      }

      inline Depth depth() const {
         return m_depth;
      }

      inline PreNumber parent() const {
         return m_parent;
      }

      /**
       * @see AccelTree::size()
       */
      inline PreNumber size() const {
         /* Remember that we use the m_size to signal compression if
          * we're a text node. */
         if (m_kind == QXmlNodeModelIndex::Text) {
            return 0;
         } else {
            return m_size;
         }
      }

      inline void setSize(const PreNumber aSize) {
         m_size = aSize;
      }

      inline QXmlNodeModelIndex::NodeKind kind() const {
         return m_kind;
      }

      inline QXmlName name() const {
         return m_name;
      }

      inline bool isCompressed() const {
         Q_ASSERT_X(m_kind == QXmlNodeModelIndex::Text, Q_FUNC_INFO,
                    "Currently, only text nodes are compressed.");
         /* Note, we don't call size() here, since it has logic for text
          * nodes. */
         return m_size == IsCompressed;
      }

    private:
      /**
       * This is the pre number of the parent.
       */
      PreNumber                       m_parent;

      /**
       * This is the count of children this node has.
       *
       * In the case of a text node, which cannot have children,
       * it is set to IsCompressed, if the content has been the result
       * of CompressedWhitespace::compress(). If it's not compressed,
       * it is zero.
       */
      PreNumber                       m_size;

      /**
       * For text nodes, and less importantly, comments,
       * this variable is not used.
       */
      QXmlName                        m_name;

      Depth                           m_depth;

      /**
       * Technically it is sufficient with 7 bits. However, at least MSVC
       * 2005 miscompiles it such that QXmlNodeModelIndex::Text becomes
       * -64 instead of 64 with hilarious crashes as result.
       *
       * Fortunately this extra bit would be padded anyway.
       */
      QXmlNodeModelIndex::NodeKind    m_kind : 8;
   };

   QUrl baseUri(const QXmlNodeModelIndex &ni) const override;
   QUrl documentUri(const QXmlNodeModelIndex &ni) const override;
   QXmlNodeModelIndex::NodeKind kind(const QXmlNodeModelIndex &ni) const override;
   QXmlNodeModelIndex::DocumentOrder compareOrder(const QXmlNodeModelIndex &ni1,
         const QXmlNodeModelIndex &ni2) const override;

   /**
    * @short Returns the root node.
    *
    * This function does not use @p n, so a default constructed
    * QXmlNodeModelIndex may be passed.
    */
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

   /**
    * The key is the pre number of an element, and the value is a vector
    * containing the namespace declarations being declared on that
    * element. Therefore, it does not reflect the namespaces being in
    * scope for that element. For that, a walk along axis ancestor is
    * necessary.
    */
   QHash<PreNumber, QVector<QXmlName> > namespaces;

   /**
    * Stores data for nodes. The QHash's value is the data of the processing instruction, and the
    * content of a text node or comment.
    */
   QHash<PreNumber, QString> data;

   QVector<BasicNodeData> basicData;
   QHash<PreNumber, QPair<qint64, qint64> > sourcePositions;

   inline QUrl documentUri() const {
      return m_documentURI;
   }

   inline QUrl baseUri() const {
      return m_baseURI;
   }

   /**
    * @short Returns @c true if the node identified by @p pre has child
    * nodes(in the sense of the XDM), but also if it has namespace nodes,
    * or attribute nodes.
    */
   inline bool hasChildren(const PreNumber pre) const {
      return basicData.at(pre).size() > 0;
   }

   /**
    * @short Returns the parent node of @p pre.
    *
    * If @p pre parent doesn't have a parent node, the return value is
    * undefined.
    *
    * @see hasParent()
    */
   inline PreNumber parent(const PreNumber pre) const {
      return basicData.at(pre).parent();
   }

   inline bool hasParent(const PreNumber pre) const {
      return basicData.at(pre).depth() > 0;
   }

   inline bool hasFollowingSibling(const PreNumber pre) const {
      return pre < maximumPreNumber();
   }

   inline PostNumber postNumber(const PreNumber pre) const {
      const BasicNodeData &b = basicData.at(pre);
      return pre + b.size() - b.depth();
   }

   inline QXmlNodeModelIndex::NodeKind kind(const PreNumber pre) const {
      return basicData.at(pre).kind();
   }

   inline PreNumber maximumPreNumber() const {
      return basicData.count() - 1;
   }

   inline PreNumber toPreNumber(const QXmlNodeModelIndex n) const {
      return n.data();
   }

   inline PreNumber size(const PreNumber pre) const {
      Q_ASSERT_X(basicData.at(pre).size() != -1, Q_FUNC_INFO,
                 "The size cannot be -1. That means an uninitialized value is attempted to be used.");
      return basicData.at(pre).size();
   }

   inline Depth depth(const PreNumber pre) const {
      return basicData.at(pre).depth();
   }

   void printStats(const NamePool::Ptr &np) const;

   inline QXmlName name(const PreNumber pre) const {
      return basicData.at(pre).name();
   }

   inline bool isCompressed(const PreNumber pre) const {
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
   /**
    * Returns the source location for the object with the given @p index.
    */
   QSourceLocation sourceLocation(const QXmlNodeModelIndex &index) const;

   /**
    * Copies the children of @p node to @p receiver.
    */
   inline void copyChildren(const QXmlNodeModelIndex &node,
                  QAbstractXmlReceiver *const receiver, const NodeCopySettings &settings) const;

   /**
    * The key is the xml:id value, and the value is the element
    * with that value.
    */
   QHash<QXmlName::LocalNameCode, PreNumber> m_IDs;
};

}

#endif
