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

#ifndef QProjectedExpression_P_H
#define QProjectedExpression_P_H

#include <qitem_p.h>

namespace QPatternist {

class ProjectedExpression
{
 public:
   typedef ProjectedExpression *Ptr;
   typedef QVector<ProjectedExpression::Ptr> Vector;

   virtual ~ProjectedExpression()
   { }

   enum Action {
      Move = 0,
      Skip = 1,
      Keep = 2,
      KeepSubtree = 4 | Keep
   };

   virtual Action actionForElement(const QXmlName name, ProjectedExpression::Ptr &next) const {
      (void) name;
      (void) next;

      return Skip;
   }
};

class ProjectedNodeTest
{
 public:
   typedef ProjectedNodeTest *Ptr;

   virtual ~ProjectedNodeTest()
   { }

   virtual bool isMatch(const QXmlNodeModelIndex::NodeKind kind) const {
      (void) kind;
      return false;
   }
};

class ProjectedStep : public ProjectedExpression
{
 public:
   ProjectedStep(const ProjectedNodeTest::Ptr test, const QXmlNodeModelIndex::Axis axis)
      : m_test(test)
   {
      (void) axis;

      Q_ASSERT(m_test);
   }

   Action actionForElement(const QXmlName name, ProjectedExpression::Ptr &next) const override {
      (void) name;
      (void) next;

      // TODO
      return Skip;
   }

 private:
   const ProjectedNodeTest::Ptr m_test;
};

class ProjectedPath : public ProjectedExpression
{
 public:
   ProjectedPath(const ProjectedExpression::Ptr left, const ProjectedExpression::Ptr right)
      : m_left(left), m_right(right)
   {
      Q_ASSERT(m_left);
      Q_ASSERT(m_right);
   }

   Action actionForElement(const QXmlName name, ProjectedExpression::Ptr &next) const override {
      ProjectedExpression::Ptr &candidateNext = next;
      const Action a = m_left->actionForElement(name, candidateNext);

      if (a != Skip) {
         /* The test accepted it, so let's replace us with the new step. */
         next = candidateNext;
      }

      return a;
   }

 private:
   const ProjectedExpression::Ptr  m_left;
   const ProjectedExpression::Ptr  m_right;
};

}

#endif
