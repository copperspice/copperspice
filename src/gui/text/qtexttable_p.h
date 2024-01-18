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

#ifndef QTEXTTABLE_P_H
#define QTEXTTABLE_P_H

#include <qtextobject_p.h>
#include <qtextdocument_p.h>

class QTextTablePrivate : public QTextFramePrivate
{
   Q_DECLARE_PUBLIC(QTextTable)

 public:
   QTextTablePrivate(QTextDocument *document)
      : QTextFramePrivate(document), grid(nullptr), nRows(0), nCols(0), dirty(true),
        blockFragmentUpdates(false)
   {
   }

   ~QTextTablePrivate();

   static QTextTable *createTable(QTextDocumentPrivate *, int pos, int rows, int cols,
      const QTextTableFormat &tableFormat);

   void fragmentAdded(QChar type, uint fragment)  override;
   void fragmentRemoved(QChar type, uint fragment)  override;

   void update() const;

   int findCellIndex(int fragment) const;

   QList<int> cells;

   // symmetric to cells array and maps to indecs in grid,
   // used for fast-lookup for row/column by fragment
   mutable QVector<int> cellIndices;
   mutable int *grid;
   mutable int nRows;
   mutable int nCols;
   mutable bool dirty;
   bool blockFragmentUpdates;
};


#endif // QTEXTTABLE_P_H
