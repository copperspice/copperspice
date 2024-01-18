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

#ifndef QXsdModelGroup_P_H
#define QXsdModelGroup_P_H

#include <qxsdparticle_p.h>
#include <qxsdterm_p.h>

template<typename N> class QList;

namespace QPatternist {

class XsdModelGroup : public XsdTerm
{
 public:
   typedef QExplicitlySharedDataPointer<XsdModelGroup> Ptr;
   typedef QList<XsdModelGroup::Ptr> List;

   /**
    * Describes the <a href="http://www.w3.org/TR/xmlschema11-1/#mg-compositor">compositor</a> of the model group.
    */
   enum ModelCompositor {
      SequenceCompositor,     ///< The model group is a sequence.
      ChoiceCompositor,       ///< The model group is a choice.
      AllCompositor           ///< The model group contains elements only.
   };

   /**
    * Creates a new model group object.
    */
   XsdModelGroup();

   /**
    * Returns always @c true, used to avoid dynamic casts.
    */
   bool isModelGroup() const override;

   /**
    * Sets the @p compositor of the model group.
    *
    * @see ModelCompositor
    */
   void setCompositor(ModelCompositor compositor);

   /**
    * Returns the compositor of the model group.
    */
   ModelCompositor compositor() const;

   /**
    * Sets the list of @p particles of the model group.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#mg-particles">Particles Definition</a>
    */
   void setParticles(const XsdParticle::List &particles);

   /**
    * Returns the list of particles of the model group.
    */
   XsdParticle::List particles() const;

 private:
   ModelCompositor   m_compositor;
   XsdParticle::List m_particles;
};

}

#endif
