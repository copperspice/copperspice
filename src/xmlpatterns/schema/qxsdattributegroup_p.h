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

#ifndef QXsdAttributeGroup_P_H
#define QXsdAttributeGroup_P_H

#include <qlist.h>

#include <qxsdannotated_p.h>
#include <qxsdattributeuse_p.h>
#include <qxsdwildcard_p.h>

namespace QPatternist {

class XsdAttributeGroup : public NamedSchemaComponent, public XsdAnnotated
{
 public:
   typedef QExplicitlySharedDataPointer<XsdAttributeGroup> Ptr;
   typedef QList<XsdAttributeGroup::Ptr> List;

   /**
    * Sets the list of attribute @p uses that are defined in the attribute group.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#agd-attribute_uses">Attribute Uses</a>
    */
   void setAttributeUses(const XsdAttributeUse::List &uses);

   /**
    * Adds a new attribute @p use to the attribute group.
    */
   void addAttributeUse(const XsdAttributeUse::Ptr &use);

   /**
    * Returns the list of all attribute uses of the attribute group.
    */
   XsdAttributeUse::List attributeUses() const;

   /**
    * Sets the attribute @p wildcard of the attribute group.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#agd-attribute_wildcard">Attribute Wildcard</a>
    */
   void setWildcard(const XsdWildcard::Ptr &wildcard);

   /**
    * Returns the attribute wildcard of the attribute group.
    */
   XsdWildcard::Ptr wildcard() const;

 private:
   XsdAttributeUse::List m_attributeUses;
   XsdWildcard::Ptr      m_wildcard;
};

}

#endif
