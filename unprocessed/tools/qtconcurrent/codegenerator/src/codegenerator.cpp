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

#include "codegenerator.h"
#include <qdebug.h>
namespace CodeGenerator
{

//Convenience constructor so you can say Item("foo")
Item::Item(const char * const text) : generator(Text(QByteArray(text)).generator) {}

int BaseGenerator::currentCount(GeneratorStack * const stack) const
{
    const int stackSize = stack->count();
    for (int i = stackSize - 1; i >= 0; --i) {
        BaseGenerator const * const generator = stack->at(i);
        switch (generator->type) {
            case RepeaterType: {
                RepeaterGenerator const * const repeater = static_cast<RepeaterGenerator const * const>(generator);
                return repeater->currentRepeat;
            } break;
            case GroupType: {
                GroupGenerator const * const group = static_cast<GroupGenerator const * const>(generator);
                return group->currentRepeat;
            } break;
            default:
            break;
        }
    }
    return -1;
}

int BaseGenerator::repeatCount(GeneratorStack * const stack) const
{
    const int stackSize = stack->count();
    for (int i = stackSize - 1; i >= 0; --i) {
        BaseGenerator const * const generator = stack->at(i);
        switch (generator->type) {
            case RepeaterType: {
                RepeaterGenerator const * const repeater = static_cast<RepeaterGenerator const * const>(generator);
                return repeater->currentRepeat;
            } break;
/*            case GroupType: {
                GroupGenerator const * const group = static_cast<GroupGenerator const * const>(generator);
                return group->currentRepeat;
            } break;
*/  
            default:
            break;
        }
    }
    return -1;
}

QByteArray RepeaterGenerator::generate(GeneratorStack * const stack)
{ 
    GeneratorStacker stacker(stack, this);
    QByteArray generated;
    for (int i = repeatOffset; i < repeatCount + repeatOffset; ++i) {
        currentRepeat = i;
        generated += childGenerator->generate(stack);
    }
    return generated;
};

QByteArray GroupGenerator::generate(GeneratorStack * const stack)
{ 
    const int repeatCount = currentCount(stack);
    GeneratorStacker stacker(stack, this);
    QByteArray generated;

    if (repeatCount > 0)
        generated += prefix->generate(stack);

    for (int i = 1; i <= repeatCount; ++i) {
        currentRepeat = i;
        generated += childGenerator->generate(stack);
        if (i != repeatCount)
            generated += separator->generate(stack);
    }

    if (repeatCount > 0)
        generated += postfix->generate(stack);

    return generated;
};

const Compound operator+(const Item &a, const Item &b)
{
    return Compound(a, b);
}

const Compound operator+(const Item &a, const char * const text)
{
    return Compound(a, Text(text));
}

const Compound operator+(const char * const text, const Item &b)
{
    return Compound(Text(text), b);
}

}
