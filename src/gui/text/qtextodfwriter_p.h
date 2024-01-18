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

#ifndef QTEXTODFWRITER_P_H
#define QTEXTODFWRITER_P_H

#include <qglobal.h>
#ifndef QT_NO_TEXTODFWRITER

#include <QXmlStreamWriter>
#include <qset.h>
#include <qstack.h>
#include <qtextdocument_p.h>
#include <qtextdocumentwriter.h>

class QTextDocumentPrivate;
class QTextCursor;
class QTextBlock;
class QIODevice;
class QXmlStreamWriter;
class QTextOdfWriterPrivate;
class QTextBlockFormat;
class QTextCharFormat;
class QTextListFormat;
class QTextFrameFormat;
class QTextTableCellFormat;
class QTextFrame;
class QTextFragment;
class QOutputStrategy;

class QTextOdfWriter
{
 public:
   QTextOdfWriter(const QTextDocument &document, QIODevice *device);
   bool writeAll();

   void setCodec(QTextCodec *codec) {
      m_codec = codec;
   }

   void setCreateArchive(bool on) {
      m_createArchive = on;
   }

   bool createArchive() const {
      return m_createArchive;
   }

   void writeBlock(QXmlStreamWriter &writer, const QTextBlock &block);
   void writeFormats(QXmlStreamWriter &writer, const QSet<int> &formatIds) const;
   void writeBlockFormat(QXmlStreamWriter &writer, QTextBlockFormat format, int formatIndex) const;
   void writeCharacterFormat(QXmlStreamWriter &writer, QTextCharFormat format, int formatIndex) const;
   void writeListFormat(QXmlStreamWriter &writer, QTextListFormat format, int formatIndex) const;
   void writeFrameFormat(QXmlStreamWriter &writer, QTextFrameFormat format, int formatIndex) const;
   void writeTableCellFormat(QXmlStreamWriter &writer, QTextTableCellFormat format, int formatIndex) const;
   void writeFrame(QXmlStreamWriter &writer, const QTextFrame *frame);
   void writeInlineCharacter(QXmlStreamWriter &writer, const QTextFragment &fragment) const;

   const QString officeNS;
   const QString textNS;
   const QString styleNS;
   const QString foNS;
   const QString tableNS;
   const QString drawNS;
   const QString xlinkNS;
   const QString svgNS;

 private:
   const QTextDocument *m_document;
   QIODevice *m_device;

   QOutputStrategy *m_strategy;
   QTextCodec *m_codec;
   bool m_createArchive;

   QStack<QTextList *> m_listStack;
};


#endif // QT_NO_TEXTODFWRITER
#endif // QTEXTODFWRITER_H
