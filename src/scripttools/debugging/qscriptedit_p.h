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

#ifndef QSCRIPTEDIT_P_H
#define QSCRIPTEDIT_P_H

#include <QtGui/qplaintextedit.h>
#include <QtCore/qhash.h>
#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE

class QScriptEditExtraArea;

class QScriptEdit : public QPlainTextEdit
{
   SCRIPT_T_CS_OBJECT(QScriptEdit)
 public:
   QScriptEdit(QWidget *parent = nullptr);
   ~QScriptEdit();

   int baseLineNumber() const;
   void setBaseLineNumber(int base);

   int executionLineNumber() const;
   void setExecutionLineNumber(int lineNumber, bool error);
   void setExecutableLineNumbers(const QSet<int> &lineNumbers);
   bool isExecutableLine(int lineNumber) const;

   int currentLineNumber() const;
   void gotoLine(int lineNumber);

   void setBreakpoint(int lineNumber);
   void setBreakpointEnabled(int lineNumber, bool enable);
   void deleteBreakpoint(int lineNumber);

   int extraAreaWidth() const;

 public:
   CS_SIGNAL_1(Public, void breakpointToggleRequest(int lineNumber, bool on))
   CS_SIGNAL_2(breakpointToggleRequest, lineNumber, on)
   CS_SIGNAL_1(Public, void breakpointEnableRequest(int lineNumber, bool enable))
   CS_SIGNAL_2(breakpointEnableRequest, lineNumber, enable)

 protected:
   void paintEvent(QPaintEvent *e);
   void resizeEvent(QResizeEvent *e);

   void extraAreaPaintEvent(QPaintEvent *e);
   void extraAreaMouseEvent(QMouseEvent *e);
   bool extraAreaEvent(QEvent *e);

 private :
   CS_SLOT_1(Private, void updateExtraAreaWidth())
   CS_SLOT_2(updateExtraAreaWidth)
   CS_SLOT_1(Private, void updateExtraArea(const QRect &un_named_arg1, int un_named_arg2))
   CS_SLOT_2(updateExtraArea)
   CS_SLOT_1(Private, void highlightCurrentLine())
   CS_SLOT_2(highlightCurrentLine)

 private:
   QTextEdit::ExtraSelection currentLineSelection() const;
   QTextEdit::ExtraSelection currentExecutionLineSelection() const;
   void updateExtraSelections();

 private:
   QScriptEditExtraArea *m_extraArea;
   int m_baseLineNumber;
   int m_executionLineNumber;
   QSet<int> m_executableLineNumbers;
   bool m_executionLineNumberHasError;
   int m_extraAreaToggleBlockNumber;

   struct BreakpointData {
      BreakpointData() : enabled(true) {}
      bool enabled;
   };
   QHash<int, BreakpointData> m_breakpoints;

 private:
   Q_DISABLE_COPY(QScriptEdit)
   friend class QScriptEditExtraArea;
};

QT_END_NAMESPACE

#endif
