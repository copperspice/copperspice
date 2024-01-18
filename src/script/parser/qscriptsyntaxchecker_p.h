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

#ifndef QSCRIPTSYNTAXCHECKER_P_H
#define QSCRIPTSYNTAXCHECKER_P_H

#include <qstring.h>
#include <qscriptgrammar_p.h>

namespace QScript {

class Lexer;

class SyntaxChecker: protected QScriptGrammar
{
 public:
   enum State {
      Error,
      Intermediate,
      Valid,
   };

   struct Result {
      Result(State s, int ln, int col, const QString &msg)
         : state(s), errorLineNumber(ln), errorColumnNumber(col),
           errorMessage(msg) {}
      State state;
      int errorLineNumber;
      int errorColumnNumber;
      QString errorMessage;
   };

   SyntaxChecker();
   ~SyntaxChecker();

   Result checkSyntax(const QString &code);

 protected:
   bool automatic(QScript::Lexer *lexer, int token) const;
   inline void reallocateStack();

 protected:
   int tos;
   int stack_size;
   int *state_stack;
};

inline void SyntaxChecker::reallocateStack()
{
   if (! stack_size) {
      stack_size = 128;
   } else {
      stack_size <<= 1;
   }

   state_stack = reinterpret_cast<int *> (qRealloc(state_stack, stack_size * sizeof(int)));
}

} // namespace QScript

#endif
