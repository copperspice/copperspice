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

#ifndef QTEXTTABLE_H
#define QTEXTTABLE_H

#include <QtCore/qglobal.h>
#include <QtCore/qobject.h>
#include <QtGui/qtextobject.h>


QT_BEGIN_NAMESPACE

class QTextCursor;
class QTextTable;
class QTextTablePrivate;

class Q_GUI_EXPORT QTextTableCell
{

 public:
   QTextTableCell() : table(0) {}
   ~QTextTableCell() {}
   QTextTableCell(const QTextTableCell &o) : table(o.table), fragment(o.fragment) {}
   QTextTableCell &operator=(const QTextTableCell &o) {
      table = o.table;
      fragment = o.fragment;
      return *this;
   }

   void setFormat(const QTextCharFormat &format);
   QTextCharFormat format() const;

   int row() const;
   int column() const;

   int rowSpan() const;
   int columnSpan() const;

   inline bool isValid() const {
      return table != 0;
   }

   QTextCursor firstCursorPosition() const;
   QTextCursor lastCursorPosition() const;
   int firstPosition() const;
   int lastPosition() const;

   inline bool operator==(const QTextTableCell &other) const {
      return table == other.table && fragment == other.fragment;
   }
   inline bool operator!=(const QTextTableCell &other) const {
      return !operator==(other);
   }

   QTextFrame::iterator begin() const;
   QTextFrame::iterator end() const;

   int tableCellFormatIndex() const;

 private:
   friend class QTextTable;
   QTextTableCell(const QTextTable *t, int f)
      : table(t), fragment(f) {}

   const QTextTable *table;
   int fragment;
};

class Q_GUI_EXPORT QTextTable : public QTextFrame
{
   GUI_CS_OBJECT(QTextTable)

 public:
   explicit QTextTable(QTextDocument *doc);
   ~QTextTable();

   void resize(int rows, int cols);
   void insertRows(int pos, int num);
   void insertColumns(int pos, int num);
   void appendRows(int count);
   void appendColumns(int count);
   void removeRows(int pos, int num);
   void removeColumns(int pos, int num);

   void mergeCells(int row, int col, int numRows, int numCols);
   void mergeCells(const QTextCursor &cursor);
   void splitCell(int row, int col, int numRows, int numCols);

   int rows() const;
   int columns() const;

   QTextTableCell cellAt(int row, int col) const;
   QTextTableCell cellAt(int position) const;
   QTextTableCell cellAt(const QTextCursor &c) const;

   QTextCursor rowStart(const QTextCursor &c) const;
   QTextCursor rowEnd(const QTextCursor &c) const;

   void setFormat(const QTextTableFormat &format);
   QTextTableFormat format() const {
      return QTextObject::format().toTableFormat();
   }

 private:
   Q_DISABLE_COPY(QTextTable)
   Q_DECLARE_PRIVATE(QTextTable)
   friend class QTextTableCell;
};

QT_END_NAMESPACE

#endif // QTEXTTABLE_H
