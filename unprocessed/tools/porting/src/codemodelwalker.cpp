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

#include "codemodelwalker.h"

QT_BEGIN_NAMESPACE
using namespace CodeModel;

void CodeModelWalker::parseScope(CodeModel::Scope *scope)
{
    if(!scope)
        return;

    if(scope->toClassScope())
        parseClassScope(scope->toClassScope());
    if(scope->toNamespaceScope())
        parseNamespaceScope(scope->toNamespaceScope());
    if(scope->toBlockScope())
        parseBlockScope(scope->toBlockScope());


    {
        MemberCollection collection = scope->members();
        MemberCollection::ConstIterator it = collection.constBegin();
        while(it != collection.constEnd())
            parseMember(*it++);
    }
    {
        ScopeCollection collection = scope->scopes();
        ScopeCollection::ConstIterator it = collection.constBegin();
        while(it != collection.constEnd())
            parseScope(*it++);
    }
    {
        NameUseCollection collection = scope->nameUses();
        NameUseCollection::ConstIterator it = collection.constBegin();
        while(it != collection.constEnd())
            parseNameUse(*it++);
    }
}

void CodeModelWalker::parseType(CodeModel::Type *type)
{
    if(!type)
        return;
    if (type->toEnumType())
        parseEnumType(type->toEnumType());
    else if (type->toClassType())
        parseClassType(type->toClassType());
    else if (type->toBuiltinType())
        parseBuiltinType(type->toBuiltinType());
    else if (type->toPointerType())
        parsePointerType(type->toPointerType());
    else if (type->toReferenceType())
        parseReferenceType(type->toReferenceType());
    else if (type->toGenericType())
        parseGenericType(type->toGenericType());
    else if (type->toAliasType())
        parseAliasType(type->toAliasType());
    else if (type->toUnknownType())
        parseUnknownType(type->toUnknownType());
}

void CodeModelWalker::parseMember(CodeModel::Member *member)
{
    if(!member)
        return;

    if (member->toFunctionMember())
        parseFunctionMember(member->toFunctionMember());
    else if (member->toVariableMember())
        parseVariableMember(member->toVariableMember());
    else if (member->toUsingDeclarationMember())
        parseUsingDeclarationMember(member->toUsingDeclarationMember());
    else if (member->toTypeMember())
        parseTypeMember(member->toTypeMember());
}

void CodeModelWalker::parseFunctionMember(CodeModel::FunctionMember *member)
{
    if(!member)
        return;
    if(member->functionBodyScope())
        parseScope(member->functionBodyScope());
}

QT_END_NAMESPACE
