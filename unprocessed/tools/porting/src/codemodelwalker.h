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

#ifndef CODEMODELWALKER_H
#define CODEMODELWALKER_H

#include "codemodel.h"

QT_BEGIN_NAMESPACE

class CodeModelWalker
{
public:
    virtual ~CodeModelWalker(){};
    virtual void parseScope(CodeModel::Scope *scope);
    virtual void parseClassScope(CodeModel::ClassScope *){};
    virtual void parseNamespaceScope(CodeModel::NamespaceScope *){};
    virtual void parseBlockScope(CodeModel::BlockScope *){};

    virtual void parseType(CodeModel::Type *type);
    virtual void parseEnumType(CodeModel::EnumType *){};
    virtual void parseClassType(CodeModel::ClassType *){};
    virtual void parseUnknownType(CodeModel::UnknownType *){};
    virtual void parseBuiltinType(CodeModel::BuiltinType *){};
    virtual void parsePointerType(CodeModel::PointerType *){};
    virtual void parseReferenceType(CodeModel::ReferenceType *){};
    virtual void parseGenericType(CodeModel::GenericType *){};
    virtual void parseAliasType(CodeModel::AliasType *){};

    virtual void parseMember(CodeModel::Member *member);
    virtual void parseFunctionMember(CodeModel::FunctionMember *);
    virtual void parseVariableMember(CodeModel::VariableMember *){};
    virtual void parseUsingDeclarationMember(CodeModel::UsingDeclarationMember *){};
    virtual void parseTypeMember(CodeModel::TypeMember *){};

    virtual void parseArgument(CodeModel::Argument *){};
    virtual void parseNameUse(CodeModel::NameUse *){};
};

QT_END_NAMESPACE

#endif
