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

#include <QDebug>
#include "codegenerator.h"
using namespace CodeGenerator;

int main()
{
    // The code generator works on items. Each item has a generate() function:
    Item item("");
    qDebug() << item.generate(); // produces "".
    
    // There are several Item subclasses. Text items contains a text string which they
    // reproduce when generate is called:
    Text text(" Hi there");
    qDebug() << text.generate(); // produces " Hi there".
    
    // Items can be concatenated:
    Item sentence = text + Text(" Bye there") ;
    qDebug() << sentence.generate(); // produces "Hi there Bye there".
    // (Internally, this creates a tree of items, and generate is called recursively 
    // for items that have children.)

    // Repeater items repeat their content when generate is called:
    Repeater repeater = text;
    repeater.setRepeatCount(3);
    qDebug() << repeater.generate(); // produces "Hi there Hi there Hi there".
    
    // Counters evaluate to the current repeat index.
    Repeater repeater2 = text + Counter();
    repeater2.setRepeatCount(3);
    qDebug() << repeater2.generate(); // produces "Hi there0 Hi there1 Hi there2".

    // Groups provide sub-groups which are repeated according to the current repeat index.
    // Counters inside Groups evaluate to the local repeat index for the Group.
    Group arguments("Arg" + Counter()  + " arg" + Counter());
    Repeater function("void foo(" + arguments + ");\n");
    function.setRepeatCount(3);
    qDebug() << function.generate();

    // Produces:
    // void foo(Arg1 arg1);
    // void foo(Arg1 arg1, Arg2 arg2);
    // void foo(Arg1 arg1, Arg2 arg2, Arg3 arg3);
}
