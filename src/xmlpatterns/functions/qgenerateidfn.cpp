/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qatomicstring_p.h"

#include "qgenerateidfn_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

Item GenerateIDFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const QXmlNodeModelIndex &node = m_operands.first()->evaluateSingleton(context).asNode();

   if (node.isNull()) {
      return AtomicString::fromValue(QString());
   }

   return AtomicString::fromValue(QLatin1Char('T')
                                  + QString::number(qptrdiff(node.model()))
                                  + QString::number(qptrdiff(node.internalPointer()))
                                  + QString::number(node.additionalData()));
}

QT_END_NAMESPACE
