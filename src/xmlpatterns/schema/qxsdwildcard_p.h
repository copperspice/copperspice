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

#ifndef QXsdWildcard_P_H
#define QXsdWildcard_P_H

#include <qxsdterm_p.h>
#include <QSet>

namespace QPatternist {

class XsdWildcard : public XsdTerm
{
 public:
   typedef QExplicitlySharedDataPointer<XsdWildcard> Ptr;

   /**
    * Defines the absent namespace that is used in wildcards.
    */
   static QString absentNamespace();

   /**
    * Describes the <a href="http://www.w3.org/TR/xmlschema11-1/#w-namespace_constraint">namespace constraint</a> of the wildcard.
    */
   class NamespaceConstraint : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<NamespaceConstraint> Ptr;

      /**
       * Describes the variety of the namespace constraint.
       *
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#nc-variety">Variety Definition</a>
       */
      enum Variety {
         Any,         ///< Any namespace is allowed.
         Enumeration, ///< Namespaces in the namespaces set are allowed.
         Not          ///< Namespaces in the namespaces set are not allowed.
      };

      /**
       * Sets the @p variety of the namespace constraint.
       *
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#nc-variety">Variety Definition</a>
       */
      void setVariety(Variety variety);

      /**
       * Returns the variety of the namespace constraint.
       */
      Variety variety() const;

      /**
       * Sets the set of @p namespaces of the namespace constraint.
       */
      void setNamespaces(const QSet<QString> &namespaces);

      /**
       * Returns the set of namespaces of the namespace constraint.
       */
      QSet<QString> namespaces() const;

      /**
       * Sets the set of disallowed @p names of the namespace constraint.
       */
      void setDisallowedNames(const QSet<QString> &names);

      /**
       * Returns the set of disallowed names of the namespace constraint.
       */
      QSet<QString> disallowedNames() const;

    private:
      Variety       m_variety;
      QSet<QString> m_namespaces;
      QSet<QString> m_disallowedNames;
   };

   /**
    * Describes the <a href="http://www.w3.org/TR/xmlschema11-1/#w-process_contents">type of content processing</a> of the wildcard.
    */
   enum ProcessContents {
      Strict,      // There must be a top-level declaration for the item available
                   // or the item must have an xsi:type, and the item must be valid as appropriate.

      Lax,         // If the item has a uniquely determined declaration available, it must be valid with
                   // respect to that definition.

      Skip         // No constraints at all: the item must simply be well-formed XML.
   };

   /**
    * Creates a new wildcard object.
    */
   XsdWildcard();

   /**
    * Returns always @c true, used to avoid dynamic casts.
    */
   bool isWildcard() const override;

   /**
    * Sets the namespace @p constraint of the wildcard.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#w-namespace_constraint">Namespace Constraint Definition</a>
    */
   void setNamespaceConstraint(const NamespaceConstraint::Ptr &constraint);

   /**
    * Returns the namespace constraint of the wildcard.
    */
   NamespaceConstraint::Ptr namespaceConstraint() const;

   /**
    * Sets the process @p contents of the wildcard.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#w-process_contents">Process Contents Definition</a>
    */
   void setProcessContents(ProcessContents contents);

   /**
    * Returns the process contents of the wildcard.
    */
   ProcessContents processContents() const;

 private:
   NamespaceConstraint::Ptr m_namespaceConstraint;
   ProcessContents          m_processContents;
};

}

#endif
