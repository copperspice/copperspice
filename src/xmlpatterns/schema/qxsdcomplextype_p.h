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

#ifndef QXsdComplexType_P_H
#define QXsdComplexType_P_H

#include <qanytype_p.h>
#include <qxsdassertion_p.h>
#include <qxsdattributeuse_p.h>
#include <qxsdparticle_p.h>
#include <qxsdsimpletype_p.h>
#include <qxsduserschematype_p.h>
#include <qxsdwildcard_p.h>
#include <QSet>

namespace QPatternist {

class XsdComplexType : public XsdUserSchemaType<AnyType>
{
 public:
   typedef QExplicitlySharedDataPointer<XsdComplexType> Ptr;

   class OpenContent : public QSharedData, public XsdAnnotated
   {
    public:
      typedef QExplicitlySharedDataPointer<OpenContent> Ptr;

      enum Mode {
         None,
         Interleave,
         Suffix
      };

      void setMode(Mode mode);
      Mode mode() const;
      void setWildcard(const XsdWildcard::Ptr &wildcard);
      XsdWildcard::Ptr wildcard() const;

    private:
      Mode             m_mode;
      XsdWildcard::Ptr m_wildcard;
   };

   class ContentType : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<ContentType> Ptr;

      enum Variety {
         Empty = 0,       // complex type has no further content.
         Simple,          // complex type has only simple type content (e.g. text, number etc.)
         ElementOnly,     // complex type has further elements or attributes but no text as content.
         Mixed            // complex type has further elements or attributes and text as content.
      };

      void setVariety(Variety variety);
      Variety variety() const;

      void setParticle(const XsdParticle::Ptr &particle);
      XsdParticle::Ptr particle() const;

      void setOpenContent(const OpenContent::Ptr &content);
      OpenContent::Ptr openContent() const;

      void setSimpleType(const AnySimpleType::Ptr &type);
      AnySimpleType::Ptr simpleType() const;

    private:
      Variety            m_variety;
      XsdParticle::Ptr   m_particle;
      OpenContent::Ptr   m_openContent;
      XsdSimpleType::Ptr m_simpleType;
   };


   XsdComplexType();

   ~XsdComplexType() {};

   QString displayName(const NamePool::Ptr &namePool) const override;

   void setWxsSuperType(const SchemaType::Ptr &type);
   SchemaType::Ptr wxsSuperType() const override;

   void setContext(const NamedSchemaComponent::Ptr &component);

   NamedSchemaComponent::Ptr context() const;

   void setDerivationMethod(DerivationMethod method);
   DerivationMethod derivationMethod() const override;

   void setIsAbstract(bool abstract);
   bool isAbstract() const override;

   void setAttributeUses(const XsdAttributeUse::List &uses);
   void addAttributeUse(const XsdAttributeUse::Ptr &use);

   XsdAttributeUse::List attributeUses() const;
   void setAttributeWildcard(const XsdWildcard::Ptr &wildcard);
   XsdWildcard::Ptr attributeWildcard() const;

   TypeCategory category() const override;

   void setContentType(const ContentType::Ptr &type);
   ContentType::Ptr contentType() const;

   void setProhibitedSubstitutions(const BlockingConstraints &substitutions);
   BlockingConstraints prohibitedSubstitutions() const;

   void setAssertions(const XsdAssertion::List &assertions);

   void addAssertion(const XsdAssertion::Ptr &assertion);
   XsdAssertion::List assertions() const;

   bool isDefinedBySchema() const override;

 private:
   SchemaType                *m_superType;
   NamedSchemaComponent      *m_context;
   DerivationMethod          m_derivationMethod;
   bool                      m_isAbstract;
   XsdAttributeUse::List     m_attributeUses;
   XsdWildcard::Ptr          m_attributeWildcard;
   ContentType::Ptr          m_contentType;
   BlockingConstraints       m_prohibitedSubstitutions;
   XsdAssertion::List        m_assertions;
};

}


#endif
