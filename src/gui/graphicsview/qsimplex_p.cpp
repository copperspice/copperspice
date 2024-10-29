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

#include <qsimplex_p.h>

#include <qset.h>
#include <qdebug.h>

#include <stdlib.h>

QSimplex::QSimplex()
   : objective(nullptr), rows(0), columns(0), firstArtificial(0), matrix(nullptr)
{
}

QSimplex::~QSimplex()
{
   clearDataStructures();
}

void QSimplex::clearDataStructures()
{
   if (matrix == nullptr) {
      return;
   }

   // Matrix
   rows = 0;
   columns = 0;
   firstArtificial = 0;
   free(matrix);
   matrix = nullptr;

   // Constraints
   for (int i = 0; i < constraints.size(); ++i) {
      delete constraints[i]->helper.first;
      delete constraints[i]->artificial;
      delete constraints[i];
   }
   constraints.clear();

   // Other
   variables.clear();
   objective = nullptr;
}

bool QSimplex::setConstraints(const QList<QSimplexConstraint *> &newConstraints)
{
   // Reset to initial state
   clearDataStructures();

   if (newConstraints.isEmpty()) {
      return true;   // we are ok with no constraints
   }

   // Make deep copy of constraints. We need this copy because we may change
   // them in the simplification method.
   for (int i = 0; i < newConstraints.size(); ++i) {
      QSimplexConstraint *c = new QSimplexConstraint;
      c->constant = newConstraints[i]->constant;
      c->ratio = newConstraints[i]->ratio;
      c->variables = newConstraints[i]->variables;
      constraints << c;
   }

   // Remove constraints of type Var == K and replace them for their value.
   if (!simplifyConstraints(&constraints)) {
      qWarning("QSimplex::setConstraints() No feasible solution");
      clearDataStructures();
      return false;
   }

   // Set Variables direct mapping.
   // "variables" is a list that provides a stable, indexed list of all variables
   // used in this problem.
   QSet<QSimplexVariable *> variablesSet;

   for (int i = 0; i < constraints.size(); ++i)
      variablesSet += \
         QSet<QSimplexVariable *>::fromList(constraints[i]->variables.keys());
   variables = variablesSet.toList();

   // Set Variables reverse mapping
   // We also need to be able to find the index for a given variable, to do that
   // we store in each variable its index.
   for (int i = 0; i < variables.size(); ++i) {
      // The variable "0" goes at the column "1", etc...
      variables[i]->index = i + 1;
   }

   // Normalize Constraints
   // In this step, we prepare the constraints in two ways:
   // Firstly, we modify all constraints of type "LessOrEqual" or "MoreOrEqual"
   // by the adding slack or surplus variables and making them "Equal" constraints.
   // Secondly, we need every single constraint to have a direct, easy feasible
   // solution. Constraints that have slack variables are already easy to solve,
   // to all the others we add artificial variables.
   //
   // At the end we modify the constraints as follows:
   //  - LessOrEqual: SLACK variable is added.
   //  - Equal: ARTIFICIAL variable is added.
   //  - More or Equal: ARTIFICIAL and SURPLUS variables are added.
   int variableIndex = variables.size();
   QList <QSimplexVariable *> artificialList;

   for (int i = 0; i < constraints.size(); ++i) {
      QSimplexVariable *slack;
      QSimplexVariable *surplus;
      QSimplexVariable *artificial;

      Q_ASSERT(constraints[i]->helper.first == nullptr);
      Q_ASSERT(constraints[i]->artificial == nullptr);

      switch (constraints[i]->ratio) {
         case QSimplexConstraint::LessOrEqual:
            slack = new QSimplexVariable;
            slack->index = ++variableIndex;
            constraints[i]->helper.first = slack;
            constraints[i]->helper.second = 1.0;
            break;

         case QSimplexConstraint::MoreOrEqual:
            surplus = new QSimplexVariable;
            surplus->index = ++variableIndex;
            constraints[i]->helper.first = surplus;
            constraints[i]->helper.second = -1.0;
            [[fallthrough]];

         case QSimplexConstraint::Equal:
            artificial = new QSimplexVariable;
            constraints[i]->artificial = artificial;
            artificialList += constraints[i]->artificial;
            break;
      }
   }

   // All original, slack and surplus have already had its index set
   // at this point. We now set the index of the artificial variables
   // as to ensure they are at the end of the variable list and therefore
   // can be easily removed at the end of this method.
   firstArtificial = variableIndex + 1;
   for (int i = 0; i < artificialList.size(); ++i) {
      artificialList[i]->index = ++variableIndex;
   }
   artificialList.clear();

   // Fill the Simplex matrix

   // One for each variable plus the Basic and BFS columns (first and last)
   columns = variableIndex + 2;
   // One for each constraint plus the objective function
   rows = constraints.size() + 1;

   matrix = (qreal *)malloc(sizeof(qreal) * columns * rows);
   if (!matrix) {
      qWarning("QSimplex::setConstraints() Unable to allocate memory");
      return false;
   }
   for (int i = columns * rows - 1; i >= 0; --i) {
      matrix[i] = 0.0;
   }

   // Fill Matrix
   for (int i = 1; i <= constraints.size(); ++i) {
      QSimplexConstraint *c = constraints[i - 1];

      if (c->artificial) {
         // Will use artificial basic variable
         setValueAt(i, 0, c->artificial->index);
         setValueAt(i, c->artificial->index, 1.0);

         if (c->helper.second != 0.0) {
            // Surplus variable
            setValueAt(i, c->helper.first->index, c->helper.second);
         }
      } else {
         // Slack is used as the basic variable
         Q_ASSERT(c->helper.second == 1.0);
         setValueAt(i, 0, c->helper.first->index);
         setValueAt(i, c->helper.first->index, 1.0);
      }

      QHash<QSimplexVariable *, qreal>::const_iterator iter;
      for (iter = c->variables.constBegin();
         iter != c->variables.constEnd();
         ++iter) {
         setValueAt(i, iter.key()->index, iter.value());
      }

      setValueAt(i, columns - 1, c->constant);
   }

   // Set objective for the first-phase Simplex.
   // Z = -1 * sum_of_artificial_vars
   for (int j = firstArtificial; j < columns - 1; ++j) {
      setValueAt(0, j, 1.0);
   }

   // Maximize our objective (artificial vars go to zero)
   solveMaxHelper();

   // If there is a solution where the sum of all artificial
   // variables is zero, then all of them can be removed and yet
   // we will have a feasible (but not optimal) solution for the
   // original problem.
   // Otherwise, we clean up our structures and report there is
   // no feasible solution.
   if ((valueAt(0, columns - 1) != 0.0) && (qAbs(valueAt(0, columns - 1)) > 0.00001)) {
      qWarning("QSimplex::setConstraints() No feasible solution");
      clearDataStructures();
      return false;
   }

   // Remove artificial variables. We already have a feasible
   // solution for the first problem, thus we don't need them
   // anymore.
   clearColumns(firstArtificial, columns - 2);

   return true;
}

void QSimplex::solveMaxHelper()
{
   reducedRowEchelon();
   while (iterate()) ;
}

void QSimplex::setObjective(QSimplexConstraint *newObjective)
{
   objective = newObjective;
}

void QSimplex::clearRow(int rowIndex)
{
   qreal *item = matrix + rowIndex * columns;
   for (int i = 0; i < columns; ++i) {
      item[i] = 0.0;
   }
}

void QSimplex::clearColumns(int first, int last)
{
   for (int i = 0; i < rows; ++i) {
      qreal *row = matrix + i * columns;
      for (int j = first; j <= last; ++j) {
         row[j] = 0.0;
      }
   }
}

void QSimplex::combineRows(int toIndex, int fromIndex, qreal factor)
{
   if (!factor) {
      return;
   }

   qreal *from = matrix + fromIndex * columns;
   qreal *to = matrix + toIndex * columns;

   for (int j = 1; j < columns; ++j) {
      qreal value = from[j];

      // skip to[j] = to[j] + factor*0.0
      if (value == 0.0) {
         continue;
      }

      to[j] += factor * value;

      // ### Avoid Numerical errors
      if (qAbs(to[j]) < 0.0000000001) {
         to[j] = 0.0;
      }
   }
}

int QSimplex::findPivotColumn()
{
   qreal min = 0;
   int minIndex = -1;

   for (int j = 0; j < columns - 1; ++j) {
      if (valueAt(0, j) < min) {
         min = valueAt(0, j);
         minIndex = j;
      }
   }

   return minIndex;
}

int QSimplex::pivotRowForColumn(int column)
{
   qreal min = qreal(999999999999.0); // ###
   int minIndex = -1;

   for (int i = 1; i < rows; ++i) {
      qreal divisor = valueAt(i, column);
      if (divisor <= 0) {
         continue;
      }

      qreal quotient = valueAt(i, columns - 1) / divisor;
      if (quotient < min) {
         min = quotient;
         minIndex = i;
      } else if ((quotient == min) && (valueAt(i, 0) > valueAt(minIndex, 0))) {
         minIndex = i;
      }
   }

   return minIndex;
}

void QSimplex::reducedRowEchelon()
{
   for (int i = 1; i < rows; ++i) {
      int factorInObjectiveRow = valueAt(i, 0);
      combineRows(0, i, -1 * valueAt(0, factorInObjectiveRow));
   }
}

bool QSimplex::iterate()
{
   // Find Pivot column
   int pivotColumn = findPivotColumn();
   if (pivotColumn == -1) {
      return false;
   }

   // Find Pivot row for column
   int pivotRow = pivotRowForColumn(pivotColumn);
   if (pivotRow == -1) {
      qWarning("QSimplex::iterate() Unbounded problem");
      return false;
   }

   // Normalize Pivot Row
   qreal pivot = valueAt(pivotRow, pivotColumn);
   if (pivot != 1.0) {
      combineRows(pivotRow, pivotRow, (qreal(1.0) - pivot) / pivot);
   }

   // Update other rows
   for (int row = 0; row < rows; ++row) {
      if (row == pivotRow) {
         continue;
      }

      combineRows(row, pivotRow, -1 * valueAt(row, pivotColumn));
   }

   // Update first column
   setValueAt(pivotRow, 0, pivotColumn);

   return true;
}

qreal QSimplex::solver(SolverFactor factor)
{
   // Remove old objective
   clearRow(0);

   // Set new objective in the first row of the simplex matrix
   qreal resultOffset = 0;
   QHash<QSimplexVariable *, qreal>::const_iterator iter;

   for (iter = objective->variables.constBegin();
      iter != objective->variables.constEnd();
      ++iter) {

      // Check if the variable was removed in the simplification process.
      // If so, we save its offset to the objective function and skip adding
      // it to the matrix.
      if (iter.key()->index == -1) {
         resultOffset += iter.value() * iter.key()->result;
         continue;
      }

      setValueAt(0, iter.key()->index, -1 * factor * iter.value());
   }

   solveMaxHelper();
   collectResults();

#if defined(CS_SHOW_DEBUG_GUI_GRAPHICSVIEW)
   for (int i = 0; i < constraints.size(); ++i) {
      Q_ASSERT(constraints[i]->isSatisfied());
   }
#endif

   // Return the value calculated by the simplex plus the value of the
   // fixed variables.
   return (factor * valueAt(0, columns - 1)) + resultOffset;
}

qreal QSimplex::solveMin()
{
   return solver(Minimum);
}

qreal QSimplex::solveMax()
{
   return solver(Maximum);
}

void QSimplex::collectResults()
{
   // All variables are zero unless overridden below.

   // ### Is this really needed? Is there any chance that an
   // important variable remains as non-basic at the end of simplex?
   for (int i = 0; i < variables.size(); ++i) {
      variables[i]->result = 0;
   }

   // Basic variables
   // Update the variable indicated in the first column with the value in the last column.
   for (int i = 1; i < rows; ++i) {
      int index = valueAt(i, 0) - 1;
      if (index < variables.size()) {
         variables[index]->result = valueAt(i, columns - 1);
      }
   }
}

bool QSimplex::simplifyConstraints(QList<QSimplexConstraint *> *constraints)
{
   QHash<QSimplexVariable *, qreal> results;   // List of single-valued variables
   bool modified = true;                       // Any chance more optimization exists?

   while (modified) {
      modified = false;

      // For all constraints
      QList<QSimplexConstraint *>::iterator iter = constraints->begin();
      while (iter != constraints->end()) {
         QSimplexConstraint *c = *iter;
         if ((c->ratio == QSimplexConstraint::Equal) && (c->variables.count() == 1)) {
            // Check whether this is a constraint of type Var == K
            // If so, save its value to "results".
            QSimplexVariable *variable = c->variables.constBegin().key();
            qreal result = c->constant / c->variables.value(variable);

            results.insert(variable, result);
            variable->result = result;
            variable->index = -1;
            modified = true;

         }

         // Replace known values among their variables
         QHash<QSimplexVariable *, qreal>::const_iterator r;
         for (r = results.constBegin(); r != results.constEnd(); ++r) {
            if (c->variables.contains(r.key())) {
               c->constant -= r.value() * c->variables.take(r.key());
               modified = true;
            }
         }

         // Keep it normalized
         if (c->constant < 0) {
            c->invert();
         }

         if (c->variables.isEmpty()) {
            // If constraint became empty due to substitution, delete it.
            if (c->isSatisfied() == false)
               // We must ensure that the constraint soon to be deleted would not
               // make the problem unfeasible if left behind. If that's the case,
               // we return false so the simplex solver can properly report that.
            {
               return false;
            }

            delete c;
            iter = constraints->erase(iter);
         } else {
            ++iter;
         }
      }
   }

   return true;
}

void QSimplexConstraint::invert()
{
   constant = -constant;
   ratio = Ratio(2 - ratio);

   QHash<QSimplexVariable *, qreal>::iterator iter;
   for (iter = variables.begin(); iter != variables.end(); ++iter) {
      iter.value() = -iter.value();
   }
}


