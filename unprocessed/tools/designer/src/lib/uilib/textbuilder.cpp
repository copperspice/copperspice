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

#include "textbuilder_p.h"
#include "ui4_p.h"
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal {
#endif

QTextBuilder::QTextBuilder()
{

}

QTextBuilder::~QTextBuilder()
{

}

QVariant QTextBuilder::loadText(const DomProperty *property) const
{
    if (property->kind() == DomProperty::String)
        return property->elementString()->text();
    return QVariant();
}

QVariant QTextBuilder::toNativeValue(const QVariant &value) const
{
    return value;
}

DomProperty *QTextBuilder::saveText(const QVariant &value) const
{
    Q_UNUSED(value)
    return 0;
}

#ifdef QFORMINTERNAL_NAMESPACE
} // namespace QFormInternal
#endif

QT_END_NAMESPACE
