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

#ifndef QTEXTTABLE_H
#define QTEXTTABLE_H

#include <qglobal.h>
#include <qobject.h>
#include <qtextobject.h>

class QTextCursor;
class QTextTable;
class QTextTablePrivate;

class Q_GUI_EXPORT QTextTableCell
{
 public:
   QTextTableCell()
      : table(nullptr)
   {
   }

   ~QTextTableCell()
   {
   }

   QTextTableCell(const QTextTableCell &other)
      : table(other.table), fragment(other.fragment)
   {
   }

   QTextTableCell &operator=(const QTextTableCell &other) {
      table    = other.table;
      fragment = other.fragment;

      return *this;
   }

   void setFormat(const QTextCharFormat &format);
   QTextCharFormat format() const;

   int row() const;
   int column() const;

   int rowSpan() const;
   int columnSpan() const;

   inline bool isValid() const {
      return table != nullptr;
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

   QTextTable(const QTextTable &) = delete;
   QTextTable &operator=(const QTextTable &) = delete;

   ~QTextTable();

   void resize(int rows, int columns);
   void insertRows(int index, int numRows);
   void insertColumns(int index, int numColumns);
   void appendRows(int count);
   void appendColumns(int count);
   void removeRows(int index, int numRows);
   void removeColumns(int index, int numColumns);

   void mergeCells(int row, int column, int numRows, int numColumns);
   void mergeCells(const QTextCursor &cursor);
   void splitCell(int row, int column, int numRows, int numColumns);

   int rows() const;
   int columns() const;

   QTextTableCell cellAt(int row, int column) const;
   QTextTableCell cellAt(int position) const;
   QTextTableCell cellAt(const QTextCursor &cursor) const;

   QTextCursor rowStart(const QTextCursor &cursor) const;
   QTextCursor rowEnd(const QTextCursor &cursor) const;

   void setFormat(const QTextTableFormat &format);
   QTextTableFormat format() const {
      return QTextObject::format().toTableFormat();
   }

 private:
   Q_DECLARE_PRIVATE(QTextTable)
   friend class QTextTableCell;
};

#endif
