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

#include <sourcecodeview.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextblock.h>
#include <qtextcharformat.h>
#include <qtextcursor.h>
#include <qtextstream.h>

SourceCodeView::SourceCodeView(QWidget *parent)
   : QPlainTextEdit(parent), m_isActive(true), m_lineNumToLoad(0)
{
   setReadOnly(true);
}

void SourceCodeView::setSourceContext(const QString &fileName, const int lineNum)
{
   m_fileToLoad.clear();
   setToolTip(fileName);

   if (fileName.isEmpty()) {
      clear();
      m_currentFileName.clear();
      appendHtml(tr("<i>Source code not available</i>"));
      return;
   }

   if (m_isActive) {
      showSourceCode(fileName, lineNum);
   } else {
      m_fileToLoad = fileName;
      m_lineNumToLoad = lineNum;
   }
}

void SourceCodeView::setActivated(bool activated)
{
   m_isActive = activated;
   if (activated && ! m_fileToLoad.isEmpty()) {
      showSourceCode(m_fileToLoad, m_lineNumToLoad);
      m_fileToLoad.clear();
   }
}

void SourceCodeView::showSourceCode(const QString &absFileName, const int lineNum)
{
   QString fileText = fileHash.value(absFileName);

   if (fileText.isEmpty()) {
      // File not in hash
      m_currentFileName.clear();

      // Assume fileName is relative to directory
      QFile file(absFileName);

      if (! file.exists()) {
         clear();
         appendHtml(tr("<i>File %1 not available</i>").formatArg(absFileName));
         return;
      }

      if (! file.open(QIODevice::ReadOnly | QIODevice::Text)) {
         clear();
         appendHtml(tr("<i>File %1 not readable</i>").formatArg(absFileName));
         return;
      }

      fileText = QString::fromUtf8(file.readAll());
      fileHash.insert(absFileName, fileText);
   }


   if (m_currentFileName != absFileName) {
      setPlainText(fileText);
      m_currentFileName = absFileName;
   }

   QTextCursor cursor = textCursor();
   cursor.setPosition(document()->findBlockByNumber(lineNum - 1).position());
   setTextCursor(cursor);
   centerCursor();

   cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
   cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);

   QTextEdit::ExtraSelection selectedLine;
   selectedLine.cursor = cursor;

   // Define custom color for line selection
   const QColor fg = palette().color(QPalette::Highlight);
   const QColor bg = palette().color(QPalette::Base);

   QColor col;
   const qreal ratio = 0.25;
   col.setRedF(fg.redF() * ratio + bg.redF() * (1 - ratio));
   col.setGreenF(fg.greenF() * ratio + bg.greenF() * (1 - ratio));
   col.setBlueF(fg.blueF() * ratio + bg.blueF() * (1 - ratio));

   selectedLine.format.setBackground(col);
   selectedLine.format.setProperty(QTextFormat::FullWidthSelection, true);
   setExtraSelections(QList<QTextEdit::ExtraSelection>() << selectedLine);
}

