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

#include "codemodel.h"

#include <QList>
#include <QByteArray>
#include <QtDebug>

QT_BEGIN_NAMESPACE

namespace CodeModel {

BuiltinType BuiltinType::Bool("bool", 0 );
BuiltinType BuiltinType::Void("void", 0 );
BuiltinType BuiltinType::Char("char", 0 );
BuiltinType BuiltinType::Short("short", 0 );
BuiltinType BuiltinType::Int("int", 0 );
BuiltinType BuiltinType::Long("long", 0 );
BuiltinType BuiltinType::Double("double", 0 );
BuiltinType BuiltinType::Float("float", 0 );
BuiltinType BuiltinType::Unsigned("unsigned", 0 );
BuiltinType BuiltinType::Signed("signed", 0 );

void Scope::addScope(Scope *scope)
{
    scope->setParent(this);
    m_scopes.add(scope);
}

void Scope::addType(Type *type)
{
    if (ClassType *klass = type->toClassType())
        klass->setParent(this);
    m_types.add(type);
}

void Scope::addMember(Member *member)
{
    member->setParent(this);
    m_members.add(member);
}

void Scope::addNameUse(NameUse *nameUse)
{
    nameUse->setParent(this);
    m_nameUses.add(nameUse);
}

} //namepsace CodeModel

QT_END_NAMESPACE
