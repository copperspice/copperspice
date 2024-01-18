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

   /**
    * @short Describes the open content object of a complex type.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ct-open_content">Open Content Definition</a>
    */
   class OpenContent : public QSharedData, public XsdAnnotated
   {
    public:
      typedef QExplicitlySharedDataPointer<OpenContent> Ptr;

      /**
       * Describes the mode of the open content.
       *
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#oc-mode">Mode Definition</a>
       */
      enum Mode {
         None,
         Interleave,
         Suffix
      };

      /**
       * Sets the @p mode of the open content.
       *
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#oc-mode">Mode Definition</a>
       */
      void setMode(Mode mode);

      /**
       * Returns the mode of the open content.
       */
      Mode mode() const;

      /**
       * Sets the @p wildcard of the open content.
       *
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#oc-wildcard">Wildcard Definition</a>
       */
      void setWildcard(const XsdWildcard::Ptr &wildcard);

      /**
       * Returns the wildcard of the open content.
       */
      XsdWildcard::Ptr wildcard() const;

    private:
      Mode             m_mode;
      XsdWildcard::Ptr m_wildcard;
   };

   /**
    * @short Describes the content type of a complex type.
    */
   class ContentType : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<ContentType> Ptr;

      /**
       * Describes the variety of the content type.
       */
      enum Variety {
         Empty = 0,    ///< The complex type has no further content.
         Simple,       ///< The complex type has only simple type content (e.g. text, number etc.)
         ElementOnly,  ///< The complex type has further elements or attributes but no text as content.
         Mixed         ///< The complex type has further elements or attributes and text as content.
      };

      /**
       * Sets the @p variety of the content type.
       *
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ct-variety">Variety Definition</a>
       */
      void setVariety(Variety variety);

      /**
       * Returns the variety of the content type.
       */
      Variety variety() const;

      /**
       * Sets the @p particle object of the content type.
       *
       * The content type has only a particle object if
       * its variety is ElementOnly or Mixed.
       *
       * @see XsdParticle
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ct-particle">Particle Declaration</a>
       */
      void setParticle(const XsdParticle::Ptr &particle);

      /**
       * Returns the particle object of the content type,
       * or an empty pointer if its variety is neither
       * ElementOnly nor Mixed.
       */
      XsdParticle::Ptr particle() const;

      /**
       * Sets the open @p content object of the content type.
       *
       * The content type has only an open content object if
       * its variety is ElementOnly or Mixed.
       *
       * @see OpenContent
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ct-open_content">Open Content Declaration</a>
       */
      void setOpenContent(const OpenContent::Ptr &content);

      /**
       * Returns the open content object of the content type,
       * or an empty pointer if its variety is neither
       * ElementOnly nor Mixed.
       */
      OpenContent::Ptr openContent() const;

      /**
       * Sets the simple @p type object of the content type.
       *
       * The content type has only a simple type object if
       * its variety is Simple.
       *
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ct-simple_type_definition">Simple Type Definition</a>
       */
      void setSimpleType(const AnySimpleType::Ptr &type);

      /**
       * Returns the simple type object of the content type,
       * or an empty pointer if its variety is not Simple.
       */
      AnySimpleType::Ptr simpleType() const;

    private:
      Variety            m_variety;
      XsdParticle::Ptr   m_particle;
      OpenContent::Ptr   m_openContent;
      XsdSimpleType::Ptr m_simpleType;
   };


   /**
    * Creates a complex type object with empty content.
    */
   XsdComplexType();

   /**
    * Destroys the complex type object.
    */
   ~XsdComplexType() {};

   /**
    * Returns the display name of the complex type.
    *
    * The display name can be used to show the type name
    * to the user.
    *
    * @param namePool The name pool where the type name is stored in.
    */
   QString displayName(const NamePool::Ptr &namePool) const override;

   /**
    * Sets the base type of the complex type.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ctd-base_type_definition">Base Type Definition</a>
    */
   void setWxsSuperType(const SchemaType::Ptr &type);

   /**
    * Returns the base type of the complex type.
    */
   SchemaType::Ptr wxsSuperType() const override;

   /**
    * Sets the context @p component of the complex type.
    *
    * The component is either an element declaration or a complex type definition.
    */
   void setContext(const NamedSchemaComponent::Ptr &component);

   /**
    * Returns the context component of the complex type.
    */
   NamedSchemaComponent::Ptr context() const;

   /**
    * Sets the derivation @p method of the complex type.
    *
    * The derivation method depends on whether the complex
    * type object has an extension or restriction object as child.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ctd-derivation_method">Derivation Method Definition</a>
    * @see DerivationMethod
    */
   void setDerivationMethod(DerivationMethod method);

   /**
    * Returns the derivation method of the complex type.
    */
   DerivationMethod derivationMethod() const override;

   /**
    * Sets whether the complex type is @p abstract.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ctd-abstract">Abstract Definition</a>
    */
   void setIsAbstract(bool abstract);

   /**
    * Returns whether the complex type is abstract.
    */
   bool isAbstract() const override;

   /**
    * Sets the list of all attribute @p uses of the complex type.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ctd-attribute_uses">Attribute Uses Declaration</a>
    */
   void setAttributeUses(const XsdAttributeUse::List &uses);

   /**
    * Adds a new attribute @p use to the complex type.
    */
   void addAttributeUse(const XsdAttributeUse::Ptr &use);

   /**
    * Returns the list of all attribute uses of the complex type.
    */
   XsdAttributeUse::List attributeUses() const;

   /**
    * Sets the attribute @p wildcard of the complex type.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ctd-attribute_wildcard">Attribute Wildcard Declaration</a>
    */
   void setAttributeWildcard(const XsdWildcard::Ptr &wildcard);

   /**
    * Returns the attribute wildcard of the complex type.
    */
   XsdWildcard::Ptr attributeWildcard() const;

   /**
    * Always returns SchemaType::ComplexType
    */
   TypeCategory category() const override;

   /**
    * Sets the content @p type of the complex type.
    *
    * @see ContentType
    */
   void setContentType(const ContentType::Ptr &type);

   /**
    * Returns the content type of the complex type.
    */
   ContentType::Ptr contentType() const;

   /**
    * Sets the prohibited @p substitutions of the complex type.
    *
    * Only ExtensionConstraint and RestrictionConstraint are allowed.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ctd-prohibited_substitutions">Prohibited Substitutions Definition</a>
    */
   void setProhibitedSubstitutions(const BlockingConstraints &substitutions);

   /**
    * Returns the prohibited substitutions of the complex type.
    */
   BlockingConstraints prohibitedSubstitutions() const;

   /**
    * Sets the @p assertions of the complex type.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ctd-assertions">Assertions Definition</a>
    */
   void setAssertions(const XsdAssertion::List &assertions);

   /**
    * Adds an @p assertion to the complex type.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ctd-assertions">Assertions Definition</a>
    */
   void addAssertion(const XsdAssertion::Ptr &assertion);

   /**
    * Returns the assertions of the complex type.
    */
   XsdAssertion::List assertions() const;

   /**
    * Always returns @c true.
    */
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
