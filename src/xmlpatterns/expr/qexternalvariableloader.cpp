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

#include "qboolean_p.h"
#include "qcommonvalues_p.h"
#include "qdynamiccontext_p.h"

#include "qexternalvariableloader_p.h"

using namespace QPatternist;

ExternalVariableLoader::~ExternalVariableLoader()
{
}

SequenceType::Ptr ExternalVariableLoader::announceExternalVariable(const QXmlName name,
      const SequenceType::Ptr &declaredType)
{
   Q_ASSERT(!name.isNull());
   Q_ASSERT(declaredType);

   (void) name;
   (void) declaredType;

   return SequenceType::Ptr();
}

Item::Iterator::Ptr ExternalVariableLoader::evaluateSequence(const QXmlName name,
      const DynamicContext::Ptr &context)
{
   Q_ASSERT(!name.isNull());
   const Item item(evaluateSingleton(name, context));

   if (item) {
      return makeSingletonIterator(item);
   } else {
      return CommonValues::emptyIterator;
   }
}

Item ExternalVariableLoader::evaluateSingleton(const QXmlName name,
      const DynamicContext::Ptr &context)
{
   Q_ASSERT(!name.isNull());
   return Boolean::fromValue(evaluateEBV(name, context));
}

bool ExternalVariableLoader::evaluateEBV(const QXmlName name,
      const DynamicContext::Ptr &context)
{
   Q_ASSERT(!name.isNull());
   return Boolean::evaluateEBV(evaluateSequence(name, context), context);
}

