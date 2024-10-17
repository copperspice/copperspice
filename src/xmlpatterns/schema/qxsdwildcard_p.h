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

   static QString absentNamespace();

   class NamespaceConstraint : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<NamespaceConstraint> Ptr;

      enum Variety {
         Any,               // Any namespace is allowed.
         Enumeration,       // Namespaces in the namespaces set are allowed.
         Not                // Namespaces in the namespaces set are not allowed.
      };

      void setVariety(Variety variety);
      Variety variety() const;

      void setNamespaces(const QSet<QString> &namespaces);
      QSet<QString> namespaces() const;

      void setDisallowedNames(const QSet<QString> &names);
      QSet<QString> disallowedNames() const;

    private:
      Variety       m_variety;
      QSet<QString> m_namespaces;
      QSet<QString> m_disallowedNames;
   };

   enum ProcessContents {
      Strict,      // There must be a top-level declaration for the item available
                   // or the item must have an xsi:type, and the item must be valid as appropriate.

      Lax,         // If the item has a uniquely determined declaration available, it must be valid with
                   // respect to that definition.

      Skip         // No constraints at all: the item must simply be well-formed XML.
   };

   XsdWildcard();
   bool isWildcard() const override;

   void setNamespaceConstraint(const NamespaceConstraint::Ptr &constraint);
   NamespaceConstraint::Ptr namespaceConstraint() const;

   void setProcessContents(ProcessContents contents);
   ProcessContents processContents() const;

 private:
   NamespaceConstraint::Ptr m_namespaceConstraint;
   ProcessContents          m_processContents;
};

}

#endif
