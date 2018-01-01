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

#ifndef RPPTREEWALKER_H
#define RPPTREEWALKER_H

#include "rpp.h"

QT_BEGIN_NAMESPACE

namespace Rpp {

class RppTreeWalker
{
public:
    virtual ~RppTreeWalker(){};
    virtual void evaluateItem(const Item *item);
    virtual void evaluateItemComposite(const ItemComposite *itemComposite);
    virtual void evaluateSource(const Source *source);
    virtual void evaluateDirective(const Directive *directive);
    virtual void evaluateIfSection(const IfSection *ifSection);
    virtual void evaluateConditionalDirective(const ConditionalDirective *conditionalDirective);

    virtual void evaluateText(const Text *textLine) {Q_UNUSED(textLine);}
    virtual void evaluateEmptyDirective(const EmptyDirective *directive) {Q_UNUSED(directive);}
    virtual void evaluateErrorDirective(const ErrorDirective *directive) {Q_UNUSED(directive);}
    virtual void evaluatePragmaDirective(const PragmaDirective *directive) {Q_UNUSED(directive);}
    virtual void evaluateIncludeDirective(const IncludeDirective *directive) {Q_UNUSED(directive);}
    virtual void evaluateDefineDirective(const DefineDirective *directive) {Q_UNUSED(directive);}
    virtual void evaluateUndefDirective(const UndefDirective *directive) {Q_UNUSED(directive);}
    virtual void evaluateLineDirective(const LineDirective *directive) {Q_UNUSED(directive);}
    virtual void evaluateNonDirective(const NonDirective *directive) {Q_UNUSED(directive);}

    virtual void evaluateIfdefDirective(const IfdefDirective *directive);
    virtual void evaluateIfndefDirective(const IfndefDirective *directive);
    virtual void evaluateIfDirective(const IfDirective *directive);
    virtual void evaluateElifDirective(const ElifDirective *directive);
    virtual void evaluateElseDirective(const ElseDirective *directive);

    virtual void evaluateEndifDirective(const EndifDirective *directive) {Q_UNUSED(directive);}
};

}

QT_END_NAMESPACE

#endif
