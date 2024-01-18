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

#include "qanyuri_p.h"
#include "qbuiltintypes_p.h"
#include "qxpathhelper_p.h"

#include "qqnamevalue_p.h"

using namespace QPatternist;

QNameValue::QNameValue(const NamePool::Ptr &np, const QXmlName name) : m_qName(name)
   , m_namePool(np)
{
   Q_ASSERT(!name.isNull());
   Q_ASSERT(m_namePool);
}

QNameValue::Ptr QNameValue::fromValue(const NamePool::Ptr &np, const QXmlName name)
{
   Q_ASSERT(!name.isNull());
   return QNameValue::Ptr(new QNameValue(np, name));
}

QString QNameValue::stringValue() const
{
   return m_namePool->toLexical(m_qName);
}

ItemType::Ptr QNameValue::type() const
{
   return BuiltinTypes::xsQName;
}
