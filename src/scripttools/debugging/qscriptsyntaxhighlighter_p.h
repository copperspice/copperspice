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

#ifndef QSCRIPTSYNTAXHIGHLIGHTER_P_H
#define QSCRIPTSYNTAXHIGHLIGHTER_P_H


#include <QtCore/qglobal.h>

#ifndef QT_NO_SYNTAXHIGHLIGHTER

#include <QtGui/qsyntaxhighlighter.h>

#include <QtGui/qtextformat.h>

QT_BEGIN_NAMESPACE

class QScriptSyntaxHighlighter : public QSyntaxHighlighter
{
 public:
   QScriptSyntaxHighlighter(QTextDocument *document = 0);
   ~QScriptSyntaxHighlighter();

 protected:
   void highlightBlock(const QString &text);

 private:
   void highlightWord(int currentPos, const QString &buffer);

   enum ScriptFormats {
      ScriptTextFormat, ScriptNumberFormat,
      ScriptStringFormat, ScriptTypeFormat,
      ScriptKeywordFormat, ScriptPreprocessorFormat,
      ScriptLabelFormat, ScriptCommentFormat,
      NumScriptFormats
   };
   QTextCharFormat m_formats[NumScriptFormats];

 private:
   Q_DISABLE_COPY(QScriptSyntaxHighlighter)
};

QT_END_NAMESPACE

#endif // QT_NO_SYNTAXHIGHLIGHTER

#endif
