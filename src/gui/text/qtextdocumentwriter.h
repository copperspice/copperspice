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

#ifndef QTEXTDOCUMENTWRITER_H
#define QTEXTDOCUMENTWRITER_H

#include <qstring.h>

class QTextDocumentWriterPrivate;
class QIODevice;
class QByteArray;
class QTextDocument;
class QTextDocumentFragment;

class Q_GUI_EXPORT QTextDocumentWriter
{
 public:
   QTextDocumentWriter();
   QTextDocumentWriter(QIODevice *device, const QByteArray &format);
   explicit QTextDocumentWriter(const QString &fileName, const QByteArray &format = QByteArray());

   QTextDocumentWriter(const QTextDocumentWriter &) = delete;
   QTextDocumentWriter &operator=(const QTextDocumentWriter &) = delete;

   ~QTextDocumentWriter();

   void setFormat (const QByteArray &format);
   QByteArray format () const;

   void setDevice (QIODevice *device);
   QIODevice *device () const;
   void setFileName (const QString &fileName);
   QString fileName () const;

   bool write(const QTextDocument *document);
   bool write(const QTextDocumentFragment &fragment);

#ifndef QT_NO_TEXTCODEC
   void setCodec(QTextCodec *codec);
   QTextCodec *codec() const;
#endif

   static QList<QByteArray> supportedDocumentFormats();

 private:
   QTextDocumentWriterPrivate *d;
};

#endif
