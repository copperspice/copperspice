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

#ifndef QTEXTDOCUMENTFRAGMENT_H
#define QTEXTDOCUMENTFRAGMENT_H

#include <qstring.h>

class QTextStream;
class QTextDocument;
class QTextDocumentFragmentPrivate;
class QTextCursor;

class Q_GUI_EXPORT QTextDocumentFragment
{
 public:
   QTextDocumentFragment();
   explicit QTextDocumentFragment(const QTextDocument *document);
   explicit QTextDocumentFragment(const QTextCursor &cursor);
   QTextDocumentFragment(const QTextDocumentFragment &other);
   QTextDocumentFragment &operator=(const QTextDocumentFragment &other);

   ~QTextDocumentFragment();

   bool isEmpty() const;

   QString toPlainText() const;

#ifndef QT_NO_TEXTHTMLPARSER
   QString toHtml(const QByteArray &encoding = QByteArray()) const;
#endif

   static QTextDocumentFragment fromPlainText(const QString &plainText);

#ifndef QT_NO_TEXTHTMLPARSER
   static QTextDocumentFragment fromHtml(const QString &text);
   static QTextDocumentFragment fromHtml(const QString &text, const QTextDocument *resourceProvider);
#endif

 private:
   QTextDocumentFragmentPrivate *d;
   friend class QTextCursor;
   friend class QTextDocumentWriter;
};

#endif
