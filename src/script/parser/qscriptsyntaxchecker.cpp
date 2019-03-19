/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qscriptsyntaxchecker_p.h"
#include "qscriptlexer_p.h"
#include "qscriptparser_p.h"

namespace QScript {

SyntaxChecker::SyntaxChecker():
   tos(0), stack_size(0), state_stack(0)
{
}

SyntaxChecker::~SyntaxChecker()
{
   if (stack_size) {
      qFree(state_stack);
   }
}

bool SyntaxChecker::automatic(QScript::Lexer *lexer, int token) const
{
   return token == T_RBRACE || token == 0 || lexer->prevTerminator();
}

SyntaxChecker::Result SyntaxChecker::checkSyntax(const QString &code)
{
   const int INITIAL_STATE = 0;
   QScript::Lexer lexer (/*engine=*/ 0);
   lexer.setCode(code, /*lineNo*/ 1);

   int yytoken = -1;
   int saved_yytoken = -1;
   QString error_message;
   int error_lineno = -1;
   int error_column = -1;
   State checkerState = Valid;

   reallocateStack();

   tos = 0;
   state_stack[++tos] = INITIAL_STATE;

   while (true) {
      const int state = state_stack [tos];
      if (yytoken == -1 && - TERMINAL_COUNT != action_index [state]) {
         if (saved_yytoken == -1) {
            yytoken = lexer.lex();
         } else {
            yytoken = saved_yytoken;
            saved_yytoken = -1;
         }
      }

      int act = t_action (state, yytoken);

      if (act == ACCEPT_STATE) {
         if (lexer.error() == QScript::Lexer::UnclosedComment) {
            checkerState = Intermediate;
         } else {
            checkerState = Valid;
         }
         break;

      } else if (act > 0) {
         if (++tos == stack_size) {
            reallocateStack();
         }

         state_stack [tos] = act;
         yytoken = -1;
      }

      else if (act < 0) {
         int r = - act - 1;

         tos -= rhs [r];
         act = state_stack [tos++];

         if ((r == Q_SCRIPT_REGEXPLITERAL_RULE1)
               || (r == Q_SCRIPT_REGEXPLITERAL_RULE2)) {
            // Skip the rest of the RegExp literal
            bool rx = lexer.scanRegExp();
            if (!rx) {
               checkerState = Intermediate;
               break;
            }
         }

         state_stack [tos] = nt_action (act, lhs [r] - TERMINAL_COUNT);
      }

      else {
         if (saved_yytoken == -1 && automatic (&lexer, yytoken) && t_action (state, T_AUTOMATIC_SEMICOLON) > 0) {
            saved_yytoken = yytoken;
            yytoken = T_SEMICOLON;
            continue;
         }

         else if ((state == INITIAL_STATE) && (yytoken == 0)) {
            // accept empty input
            yytoken = T_SEMICOLON;
            continue;
         }

         int ers = state;
         int shifts = 0;
         int reduces = 0;
         int expected_tokens [3];

         for (int tk = 0; tk < TERMINAL_COUNT; ++tk) {
            int k = t_action (ers, tk);

            if (! k) {
               continue;
            } else if (k < 0) {
               ++reduces;

            } else if (spell [tk]) {
               if (shifts < 3) {
                  expected_tokens[shifts] = tk;
               }
               ++shifts;
            }
         }

         error_message.clear ();
         if (shifts && shifts < 3) {
            bool first = true;

            for (int s = 0; s < shifts; ++s) {
               if (first) {
                  error_message += "Expected ";
               } else {
                  error_message += ", ";
               }

               first = false;
               error_message += '`';
               error_message += QString::fromLatin1(spell[expected_tokens[s]]);
               error_message += '\'';
            }
         }

         if (error_message.isEmpty()) {
            error_message = lexer.errorMessage();
         }

         error_lineno = lexer.startLineNo();
         error_column = lexer.startColumnNo();
         checkerState = Error;
         break;
      }
   }

   if (checkerState == Error) {
      if (lexer.error() == QScript::Lexer::UnclosedComment) {
         checkerState = Intermediate;

      } else if (yytoken == 0) {
         checkerState = Intermediate;
      }
   }

   return Result(checkerState, error_lineno, error_column, error_message);
}

} // namespace QScript


