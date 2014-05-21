----------------------------------------------------------------------------
--
-- Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
-- All rights reserved.
-- Contact: Nokia Corporation (qt-info@nokia.com)
--
-- This file is part of the QtCore module of the Qt Toolkit.
--
-- $QT_BEGIN_LICENSE:LGPL$
-- GNU Lesser General Public License Usage
-- This file may be used under the terms of the GNU Lesser General Public
-- License version 2.1 as published by the Free Software Foundation and
-- appearing in the file LICENSE.LGPL included in the packaging of this
-- file. Please review the following information to ensure the GNU Lesser
-- General Public License version 2.1 requirements will be met:
-- http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
--
-- In addition, as a special exception, Nokia gives you certain additional
-- rights. These rights are described in the Nokia Qt LGPL Exception
-- version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
--
-- GNU General Public License Usage
-- Alternatively, this file may be used under the terms of the GNU General
-- Public License version 3.0 as published by the Free Software Foundation
-- and appearing in the file LICENSE.GPL included in the packaging of this
-- file. Please review the following information to ensure the GNU General
-- Public License version 3.0 requirements will be met:
-- http://www.gnu.org/copyleft/gpl.html.
--
-- Other Usage
-- Alternatively, this file may be used in accordance with the terms and
-- conditions contained in a signed written agreement between you and Nokia.
--
--
--
--
--
-- $QT_END_LICENSE$
--
----------------------------------------------------------------------------

%parser calc_grammar
%decl calc_parser.h
%impl calc_parser.cpp

%token_prefix Token_
%token number
%token lparen
%token rparen
%token plus
%token minus

%start Goal

/:
#ifndef CALC_PARSER_H
#define CALC_PARSER_H

#include "qparser.h"
#include "calc_grammar_p.h"

class CalcParser: public QParser<CalcParser, $table>
{
public:
  int nextToken();
  void consumeRule(int ruleno);
};

#endif // CALC_PARSER_H
:/





/.
#include "calc_parser.h"

#include <QtDebug>
#include <cstdlib>

void CalcParser::consumeRule(int ruleno)
  {
    switch (ruleno) {
./

Goal: Expression ;
/.
case $rule_number:
  qDebug() << "value:" << sym(1);
  break;
./

PrimaryExpression: number ;
PrimaryExpression: lparen Expression rparen ;
/.
case $rule_number:
  sym(1) = sym (2);
  break;
./

Expression: PrimaryExpression ;

Expression: Expression plus PrimaryExpression;
/.
case $rule_number:
  sym(1) += sym (3);
  break;
./

Expression: Expression minus PrimaryExpression;
/.
case $rule_number:
  sym(1) -= sym (3);
  break;
./



/.
    } // switch
}

#include <cstdio>

int main()
{
  CalcParser p;

  if (p.parse())
    printf("ok\n");
}
./
