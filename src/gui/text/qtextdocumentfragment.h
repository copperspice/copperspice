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

#ifndef QTEXTDOCUMENTFRAGMENT_H
#define QTEXTDOCUMENTFRAGMENT_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QTextStream;
class QTextDocument;
class QTextDocumentFragmentPrivate;
class QTextCursor;

class Q_GUI_EXPORT QTextDocumentFragment
{

 public:
   QTextDocumentFragment();
   explicit QTextDocumentFragment(const QTextDocument *document);
   explicit QTextDocumentFragment(const QTextCursor &range);
   QTextDocumentFragment(const QTextDocumentFragment &rhs);
   QTextDocumentFragment &operator=(const QTextDocumentFragment &rhs);
   ~QTextDocumentFragment();

   bool isEmpty() const;

   QString toPlainText() const;
   static QTextDocumentFragment fromPlainText(const QString &plainText);

#ifndef QT_NO_TEXTHTMLPARSER
   QString toHtml(const QByteArray &encoding = QByteArray()) const;

   static QTextDocumentFragment fromHtml(const QString &html);
   static QTextDocumentFragment fromHtml(const QString &html, const QTextDocument *resourceProvider);
#endif

 private:
   QTextDocumentFragmentPrivate *d;
   friend class QTextCursor;
   friend class QTextDocumentWriter;
};

QT_END_NAMESPACE

#endif // QTEXTDOCUMENTFRAGMENT_H
