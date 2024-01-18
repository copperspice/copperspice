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

#ifndef QXsdParticle_P_H
#define QXsdParticle_P_H

#include <qlist.h>

#include <qnamedschemacomponent_p.h>
#include <qxsdterm_p.h>

namespace QPatternist {

class XsdParticle : public NamedSchemaComponent
{
 public:
   typedef QExplicitlySharedDataPointer<XsdParticle> Ptr;
   typedef QList<XsdParticle::Ptr> List;

   /**
    * Creates a new particle object.
    */
   XsdParticle();

   /**
    * Sets the minimum @p occurrence of the particle.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#p-min_occurs">Minimum Occurrence Definition</a>
    */
   void setMinimumOccurs(unsigned int occurrence);

   /**
    * Returns the minimum occurrence of the particle.
    */
   unsigned int minimumOccurs() const;

   /**
    * Sets the maximum @p occurrence of the particle.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#p-max_occurs">Maximum Occurrence Definition</a>
    */
   void setMaximumOccurs(unsigned int occurrence);

   /**
    * Returns the maximum occurrence of the particle.
    *
    * @note This value has only a meaning if maximumOccursUnbounded is @c false.
    */
   unsigned int maximumOccurs() const;

   /**
    * Sets whether the maximum occurrence of the particle is unbounded.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#p-max_occurs">Maximum Occurrence Definition</a>
    */
   void setMaximumOccursUnbounded(bool unbounded);

   /**
    * Returns whether the maximum occurrence of the particle is unbounded.
    */
   bool maximumOccursUnbounded() const;

   /**
    * Sets the @p term of the particle.
    *
    * The term can be an element, a model group or an element wildcard.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#p-term">Term Definition</a>
    */
   void setTerm(const XsdTerm::Ptr &term);

   /**
    * Returns the term of the particle.
    */
   XsdTerm::Ptr term() const;

 private:
   unsigned int m_minimumOccurs;
   unsigned int m_maximumOccurs;
   bool         m_maximumOccursUnbounded;
   XsdTerm::Ptr m_term;
};

}

#endif
