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

#include <qdocumentprojector_p.h>

using namespace QPatternist;

DocumentProjector::DocumentProjector(const ProjectedExpression::Vector &paths, QAbstractXmlReceiver *const receiver)
   : m_paths(paths), m_pathCount(paths.count()), m_action(ProjectedExpression::Move),
     m_nodesInProcess(0), m_receiver(receiver)
{
   Q_ASSERT_X(paths.count() > 0, Q_FUNC_INFO,
              "Using DocumentProjector with no paths is undefined behavior.");
   Q_ASSERT(m_receiver);
}

void DocumentProjector::startElement(const QXmlName &name)
{
   switch (m_action) {
      case ProjectedExpression::KeepSubtree: {
         m_receiver->startElement(name);
      }
      [[fallthrough]];

      case ProjectedExpression::Skip: {
         ++m_nodesInProcess;
         return;
      }

      default: {
         Q_ASSERT_X(m_action == ProjectedExpression::Move, Q_FUNC_INFO,
                    "Not to supposed to receive Keep because "
                    "endElement() should always end the Keep state.");

         for (int i = 0; i < m_pathCount; ++i) {
            m_action = m_paths.at(i)->actionForElement(name, m_paths[i]);

            switch (m_action) {
               case ProjectedExpression::Keep: {
                  m_action = ProjectedExpression::Keep;
                  continue;
               }

               case ProjectedExpression::KeepSubtree: {
                  // Ok, at least one path wanted this node. Pass it on, * and exit.
                  m_receiver->startElement(name);
                  ++m_nodesInProcess;
                  return;
               }

               case ProjectedExpression::Skip: {
                  /* This particular path doesn't need it, but
                   * some other path might, so continue looping. */
                  continue;
               }

               case ProjectedExpression::Move:
                  Q_ASSERT_X(false, Q_FUNC_INFO, "Move is not valid.");
            }
         }

         ++m_nodesInProcess;

         if (m_action == ProjectedExpression::Keep) {
            m_receiver->startElement(name);
         } else {
            Q_ASSERT(m_action == ProjectedExpression::Skip);
         }
      }
   }
}

void DocumentProjector::endElement()
{
   if (m_action == ProjectedExpression::Keep) {
      Q_ASSERT(m_nodesInProcess == 1);

      m_receiver->endElement();

      /* We have now kept the single node, and now wants to skip
       * all its children. */
      m_action = ProjectedExpression::Skip;
      m_nodesInProcess = 0;

   } else if (m_action == ProjectedExpression::KeepSubtree) {
      m_receiver->endElement();
      --m_nodesInProcess;

      if (m_nodesInProcess == 0) {
         /* We have now skipped all the children, let's do
          * a new path analysis. */
         m_action = ProjectedExpression::Move;
      }

   } else {
      Q_ASSERT_X(m_action == ProjectedExpression::Skip, Q_FUNC_INFO,
                 "We're not supposed to be in a Move action here.");
      /* We skip calling m_receiver's endElement() here since we're
       * skipping. */
      Q_ASSERT(m_nodesInProcess > 0);
      --m_nodesInProcess;

      if (m_nodesInProcess == 0) {
         /* Ok, we've skipped them all, let's do something
          * new -- let's Move on to the next path! */
         m_action = ProjectedExpression::Move;
      }
   }
}

void DocumentProjector::attribute(const QXmlName &name, QStringView value)
{
   (void) name;
   (void) value;
}

void DocumentProjector::namespaceBinding(const QXmlName &nb)
{
   (void) nb;
}

void DocumentProjector::comment(const QString &value)
{
   Q_ASSERT_X(! value.contains(QString("--")), Q_FUNC_INFO,
              "Invalid input, caller is responsible to supply valid input.");
   (void) value;
}

void DocumentProjector::characters(QStringView value)
{
   (void) value;
}

void DocumentProjector::processingInstruction(const QXmlName &name, const QString &value)
{
   Q_ASSERT_X(! value.contains(QString("?>")), Q_FUNC_INFO,
              "Invalid input, caller is responsible to supply valid input.");

   (void) name;
   (void) value;
}

void DocumentProjector::item(const Item &outputItem)
{
   (void) outputItem;
}

void DocumentProjector::startDocument()
{
}

void DocumentProjector::endDocument()
{
}
