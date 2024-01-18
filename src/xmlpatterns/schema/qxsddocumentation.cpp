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

#include "qxsddocumentation_p.h"

using namespace QPatternist;

XsdDocumentation::XsdDocumentation()
{
}

XsdDocumentation::~XsdDocumentation()
{
}

void XsdDocumentation::setSource(const AnyURI::Ptr &source)
{
   m_source = source;
}

AnyURI::Ptr XsdDocumentation::source() const
{
   return m_source;
}

void XsdDocumentation::setLanguage(const DerivedString<TypeLanguage>::Ptr &language)
{
   m_language = language;
}

DerivedString<TypeLanguage>::Ptr XsdDocumentation::language() const
{
   return m_language;
}

void XsdDocumentation::setContent(const QString &content)
{
   m_content = content;
}

QString XsdDocumentation::content() const
{
   return m_content;
}
