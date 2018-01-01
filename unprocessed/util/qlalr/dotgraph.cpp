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
#include "dotgraph.h"

DotGraph::DotGraph(QTextStream &o):
  out (o)
{
}

void DotGraph::operator () (Automaton *aut)
{
  Grammar *g = aut->_M_grammar;

  out << "digraph {" << endl << endl;

  out << "subgraph Includes {" << endl;
  for (Automaton::IncludesGraph::iterator incl = Automaton::IncludesGraph::begin_nodes ();
       incl != Automaton::IncludesGraph::end_nodes (); ++incl)
    {
      for (Automaton::IncludesGraph::edge_iterator edge = incl->begin (); edge != incl->end (); ++edge)
        {
          out << "\t\"(" << aut->id (incl->data.state) << ", " << incl->data.nt << ")\"";
          out << "\t->\t";
          out << "\"(" << aut->id ((*edge)->data.state) << ", " << (*edge)->data.nt << ")\"\t";
          out << "[label=\"" << incl->data.state->follows [incl->data.nt] << "\"]";
          out << endl;
        }
    }
  out << "}" << endl << endl;


  out << "subgraph LRA {" << endl;
  //out << "node [shape=record];" << endl << endl;

  for (StatePointer q = aut->states.begin (); q != aut->states.end (); ++q)
    {
      int state = aut->id (q);

      out << "\t" << state << "\t[shape=record,label=\"{";

      out << "<0> State " << state;

      int index = 1;
      for (ItemPointer item = q->kernel.begin (); item != q->kernel.end (); ++item)
        out << "| <" << index++ << "> " << *item;

      out << "}\"]" << endl;

      for (Bundle::iterator a = q->bundle.begin (); a != q->bundle.end (); ++a)
        {
          const char *clr = g->isTerminal (a.key ()) ? "blue" : "red";
          out << "\t" << state << "\t->\t" << aut->id (*a) << "\t[color=\"" << clr << "\",label=\"" << a.key () << "\"]" << endl;
        }
      out << endl;
    }

  out << "}" << endl;
  out << endl << endl << "}" << endl;
}
