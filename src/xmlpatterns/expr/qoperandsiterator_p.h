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

#ifndef QOperandsIterator_P_H
#define QOperandsIterator_P_H

#include <QPair>
#include <QStack>

#include <qexpression_p.h>

namespace QPatternist {
class OperandsIterator
{
   typedef QPair<Expression::List, int> Level;

 public:
   enum TreatParent {
      ExcludeParent,
      IncludeParent
   };

   OperandsIterator(const Expression::Ptr &start, const TreatParent treatParent) {
      Q_ASSERT(start);

      if (treatParent == IncludeParent) {
         Expression::List l;
         l.append(start);
         m_exprs.push(qMakePair(l, -1));
      }

      m_exprs.push(qMakePair(start->operands(), -1));
   }

   Expression::Ptr next() {
      if (m_exprs.isEmpty()) {
         return Expression::Ptr();
      }

      Level &lvl = m_exprs.top();
      ++lvl.second;

      if (lvl.second == lvl.first.size()) {
         // Resume iteration above current position
         m_exprs.pop();

         if (m_exprs.isEmpty()) {
            return Expression::Ptr();
         }

         while (true) {
            Level &previous = m_exprs.top();
            ++previous.second;

            if (previous.second < previous.first.count()) {
               Expression::Ptr op = previous.first.at(previous.second);
               m_exprs.push(qMakePair(op->operands(), -1));
               return op;

            } else {
               // We have already reached the end of this level.
               m_exprs.pop();
               if (m_exprs.isEmpty()) {
                  return Expression::Ptr();
               }
            }
         }

      } else {
         Expression::Ptr op = lvl.first.at(lvl.second);
         m_exprs.push(qMakePair(op->operands(), -1));
         return op;
      }
   }

   Expression::Ptr skipOperands() {
      if (m_exprs.isEmpty()) {
         return Expression::Ptr();
      }

      Level &lvl = m_exprs.top();
      ++lvl.second;

      if (lvl.second == lvl.first.size()) {
         m_exprs.pop();
      }

      return next();
   }

 private:
   OperandsIterator(const OperandsIterator &) = delete;
   OperandsIterator &operator=(const OperandsIterator &) = delete;

   QStack<Level> m_exprs;
};

}

#endif
