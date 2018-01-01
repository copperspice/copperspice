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

#ifndef QSYNTAXHIGHLIGHTER_H
#define QSYNTAXHIGHLIGHTER_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_SYNTAXHIGHLIGHTER

#include <QtCore/qobject.h>
#include <QtGui/qtextobject.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QTextDocument;
class QSyntaxHighlighterPrivate;
class QTextCharFormat;
class QFont;
class QColor;
class QTextBlockUserData;
class QTextEdit;

class Q_GUI_EXPORT QSyntaxHighlighter : public QObject
{
   GUI_CS_OBJECT(QSyntaxHighlighter)
   Q_DECLARE_PRIVATE(QSyntaxHighlighter)

 public:
   QSyntaxHighlighter(QObject *parent);
   QSyntaxHighlighter(QTextDocument *parent);
   QSyntaxHighlighter(QTextEdit *parent);
   virtual ~QSyntaxHighlighter();

   void setDocument(QTextDocument *doc);
   QTextDocument *document() const;

   GUI_CS_SLOT_1(Public, void rehighlight())
   GUI_CS_SLOT_2(rehighlight)
   GUI_CS_SLOT_1(Public, void rehighlightBlock(const QTextBlock &block))
   GUI_CS_SLOT_2(rehighlightBlock)

 protected:
   virtual void highlightBlock(const QString &text) = 0;

   void setFormat(int start, int count, const QTextCharFormat &format);
   void setFormat(int start, int count, const QColor &color);
   void setFormat(int start, int count, const QFont &font);
   QTextCharFormat format(int pos) const;

   int previousBlockState() const;
   int currentBlockState() const;
   void setCurrentBlockState(int newState);

   void setCurrentBlockUserData(QTextBlockUserData *data);
   QTextBlockUserData *currentBlockUserData() const;

   QTextBlock currentBlock() const;

   QScopedPointer<QSyntaxHighlighterPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QSyntaxHighlighter)

   GUI_CS_SLOT_1(Private, void _q_reformatBlocks(int from, int charsRemoved, int charsAdded))
   GUI_CS_SLOT_2(_q_reformatBlocks)

   GUI_CS_SLOT_1(Private, void _q_delayedRehighlight())
   GUI_CS_SLOT_2(_q_delayedRehighlight)
   
};

QT_END_NAMESPACE

#endif // QT_NO_SYNTAXHIGHLIGHTER

#endif // QSYNTAXHIGHLIGHTER_H
