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

#ifndef QXsdTerm_P_H
#define QXsdTerm_P_H

#include <qnamedschemacomponent_p.h>
#include <qxsdannotated_p.h>

namespace QPatternist {

class XsdTerm : public NamedSchemaComponent, public XsdAnnotated
{
 public:
   typedef QExplicitlySharedDataPointer<XsdTerm> Ptr;

   /**
    * Returns @c true if the term is an element, @c false otherwise.
    */
   virtual bool isElement() const;

   /**
    * Returns @c true if the term is a model group (group tag), @c false otherwise.
    */
   virtual bool isModelGroup() const;

   /**
    * Returns @c true if the term is a wildcard (any tag), @c false otherwise.
    */
   virtual bool isWildcard() const;

   /**
    * Returns @c true if the term is a reference, @c false otherwise.
    *
    * @note The reference term is only used internally as helper during type resolving.
    */
   virtual bool isReference() const;

 protected:
   /**
    * This constructor only exists to ensure this class is subclassed.
    */
   inline XsdTerm() {};
};

}

#endif
