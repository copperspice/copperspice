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

#ifndef QExternalVariableLoader_P_H
#define QExternalVariableLoader_P_H

#include <qitem_p.h>
#include <qsequencetype_p.h>
#include <qxmlname.h>

namespace QPatternist {
class DynamicContext;

class ExternalVariableLoader : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<ExternalVariableLoader> Ptr;
   inline ExternalVariableLoader() {}

   virtual ~ExternalVariableLoader();

   virtual SequenceType::Ptr announceExternalVariable(const QXmlName name, const SequenceType::Ptr &declaredType);
   virtual Item::Iterator::Ptr evaluateSequence(const QXmlName name,
         const QExplicitlySharedDataPointer<DynamicContext> &context);
   virtual Item evaluateSingleton(const QXmlName name, const QExplicitlySharedDataPointer<DynamicContext> &context);
   virtual bool evaluateEBV(const QXmlName name, const QExplicitlySharedDataPointer<DynamicContext> &context);
};
}

#endif
