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

#include "textreplacement.h"

QT_BEGIN_NAMESPACE

bool TextReplacements::insert(QByteArray newText, int insertPosition, int currentLenght)
{
    //bubble sort the new replacement into the list
    int i;
    for(i=0; i<textReplacementList.size(); ++i) {
        if (insertPosition == textReplacementList.at(i).insertPosition)
            return false; // multiple replacements on the same insertPosition is not allowed.
        if(insertPosition < textReplacementList.at(i).insertPosition)
            break;  //we found the right position
    }
    //++i;
  //  cout << "inserting new text " << newText.constData() << endl;
    // %s at %d overwriting %d bytes at list pos %d\n", newText.constData(), insertPosition, currentLenght, i);
    TextReplacement rep;
    rep.newText=newText;
    rep.insertPosition=insertPosition;
    rep.currentLenght=currentLenght;

    textReplacementList.insert(i, rep);
    return true;
}

void TextReplacements::clear()
{
    textReplacementList.clear();
}

QByteArray TextReplacements::apply(QByteArray current)
{
    QByteArray newData=current;
    int i;
    int replacementOffset=0;

    for(i=0; i<textReplacementList.size(); ++i) {
        TextReplacement rep=textReplacementList.at(i);
        //printf("applying new text %s insert at %d overwriting %d bytes \n", rep.newText.constData(), rep.insertPosition, rep.currentLenght);
        newData.remove(rep.insertPosition+replacementOffset, rep.currentLenght);
        newData.insert(rep.insertPosition+replacementOffset, rep.newText);

        //modify all remaining replacements if we change the document length
        replacementOffset+=(rep.newText.size() - rep.currentLenght);
    }

    return newData;
}

TextReplacements &TextReplacements::operator+=(const TextReplacements &other)
{
    foreach(TextReplacement rep, other.replacements()) {
        insert(rep.newText, rep.insertPosition, rep.currentLenght);
    }
    return *this;
}

QT_END_NAMESPACE
