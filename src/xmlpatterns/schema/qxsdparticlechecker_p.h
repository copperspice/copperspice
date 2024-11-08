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

#ifndef QXsdParticleChecker_P_H
#define QXsdParticleChecker_P_H

#include <qxsdelement_p.h>
#include <qxsdparticle_p.h>
#include <qxsdschemacontext_p.h>
#include <qxsdwildcard_p.h>

namespace QPatternist {

class XsdParticleChecker
{
 public:
   static bool hasDuplicatedElements(const XsdParticle::Ptr &particle, const NamePool::Ptr &namePool,
         XsdElement::Ptr &conflictingElement);

   static bool isUPAConform(const XsdParticle::Ptr &particle, const NamePool::Ptr &namePool);

   static bool isUPAConformXsdAll(const XsdParticle::Ptr &particle, const NamePool::Ptr &namePool);

   static bool subsumes(const XsdParticle::Ptr &particle, const XsdParticle::Ptr &derivedParticle,
         const XsdSchemaContext::Ptr &context, QString &errorMsg);
};

}

#endif
