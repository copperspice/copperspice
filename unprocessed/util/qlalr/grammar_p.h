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

#ifndef GRAMMAR_P_H
#define GRAMMAR_P_H

class grammar
{
public:
  enum {
    EOF_SYMBOL = 0,
    COLON = 16,
    DECL = 19,
    DECL_FILE = 3,
    ERROR = 21,
    EXPECT = 4,
    EXPECT_RR = 5,
    ID = 1,
    IMPL = 20,
    IMPL_FILE = 6,
    LEFT = 7,
    MERGED_OUTPUT = 8,
    NONASSOC = 9,
    OR = 17,
    PARSER = 10,
    PREC = 11,
    RIGHT = 12,
    SEMICOLON = 18,
    START = 13,
    STRING_LITERAL = 2,
    TOKEN = 14,
    TOKEN_PREFIX = 15,

    ACCEPT_STATE = 68,
    RULE_COUNT = 45,
    STATE_COUNT = 69,
    TERMINAL_COUNT = 22,
    NON_TERMINAL_COUNT = 24,

    GOTO_INDEX_OFFSET = 69,
    GOTO_INFO_OFFSET = 76,
    GOTO_CHECK_OFFSET = 76
  };

  static const char  *const spell [];
  static const int            lhs [];
  static const int            rhs [];
  static const int   goto_default [];
  static const int action_default [];
  static const int   action_index [];
  static const int    action_info [];
  static const int   action_check [];

  inline int nt_action (int state, int nt) const
  {
    const int *const goto_index = &action_index [GOTO_INDEX_OFFSET];
    const int *const goto_check = &action_check [GOTO_CHECK_OFFSET];

    const int yyn = goto_index [state] + nt;

    if (yyn < 0 || goto_check [yyn] != nt)
      return goto_default [nt];

    const int *const goto_info = &action_info [GOTO_INFO_OFFSET];
    return goto_info [yyn];
  }

  inline int t_action (int state, int token) const
  {
    const int yyn = action_index [state] + token;

    if (yyn < 0 || action_check [yyn] != token)
      return - action_default [state];

    return action_info [yyn];
  }
};


#endif // GRAMMAR_P_H

