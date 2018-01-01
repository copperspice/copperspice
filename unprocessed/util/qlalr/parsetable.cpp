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

#include <QtCore/QTextStream>
#include "lalr.h"
#include "parsetable.h"

ParseTable::ParseTable (QTextStream &o):
  out (o)
{
}

void ParseTable::operator () (Automaton *aut)
{
  Grammar *g = aut->_M_grammar;

  int rindex = 1;
  for (RulePointer rule = g->rules.begin (); rule != g->rules.end (); ++rule)
    out << rindex++ << ")\t" << *rule << endl;
  out << endl << endl;

  int index = 0;
  for (StatePointer state = aut->states.begin (); state != aut->states.end (); ++state)
    {
      out << "state " << index++ << endl << endl;

      for (ItemPointer item = state->kernel.begin (); item != state->kernel.end (); ++item)
        {
          out << " *  " << *item;

          if (item->dot == item->end_rhs ())
            out << " " << aut->lookaheads [item];

          out << endl;
        }

      bool first = true;
      for (Bundle::iterator arrow = state->bundle.begin (); arrow != state->bundle.end (); ++arrow)
        {
          if (! g->isTerminal (arrow.key ()))
            continue;

          if (first)
            out << endl;

          first = false;

          out << "    " << *arrow.key () << " shift, and go to state " << std::distance (aut->states.begin (), *arrow) << endl;
        }

      first = true;
      for (ItemPointer item = state->closure.begin (); item != state->closure.end (); ++item)
        {
          if (item->dot != item->end_rhs () || item->rule == state->defaultReduce)
            continue;

          if (first)
            out << endl;

          first = false;

          foreach (Name la, aut->lookaheads.value (item))
            out << "    " << *la << " reduce using rule " << aut->id (item->rule) << " (" << *item->rule->lhs << ")" << endl;
        }

      first = true;
      for (Bundle::iterator arrow = state->bundle.begin (); arrow != state->bundle.end (); ++arrow)
        {
          if (! g->isNonTerminal (arrow.key ()))
            continue;

          if (first)
            out << endl;

          first = false;

          out << "    " << *arrow.key () << " go to state " << std::distance (aut->states.begin (), *arrow) << endl;
        }

      if (state->defaultReduce != g->rules.end ())
        {
          out << endl
              << "    $default reduce using rule " << aut->id (state->defaultReduce) << " (" << *state->defaultReduce->lhs << ")" << endl;
        }

      out << endl;
    }
}
