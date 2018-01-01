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

#ifndef CODEMODELATTRIBUTES_H
#define CODEMODELATTRIBUTES_H

#include "codemodelwalker.h"
#include "translationunit.h"

QT_BEGIN_NAMESPACE

class CodeModelAttributes : public CodeModelWalker
{
public:
    void createAttributes(TranslationUnit translationUnit);
protected:
    void parseNameUse(CodeModel::NameUse *);
    void parseMember(CodeModel::Member *member);
    void parseFunctionMember(CodeModel::FunctionMember *member);

    void createNameTypeAttribute(CodeModel::Member *member);
    void createNameTypeAttribute(CodeModel::NameUse *nameUse);

private:
    void createNameTypeAttributeAtIndex(TokenEngine::TokenAttributes *attributes,
                                        int index, CodeModel::Member *member);
    bool areAttributesEnabled(const TokenEngine::TokenAttributes *attributes) const;

    TranslationUnit  m_translationUnit;
};

QT_END_NAMESPACE

#endif
