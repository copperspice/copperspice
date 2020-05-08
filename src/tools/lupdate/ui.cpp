/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "lupdate.h"

#include <translator.h>

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QString>

#include <QXmlAttributes>
#include <QXmlDefaultHandler>
#include <QXmlLocator>
#include <QXmlParseException>

class LU
{
   Q_DECLARE_TR_FUNCTIONS(LUpdate)
};

class UiReader : public QXmlDefaultHandler
{
 public:
   UiReader(Translator &translator, ConversionData &cd)
      : m_translator(translator), m_cd(cd), m_lineNumber(-1), m_isTrString(false),
        m_needUtf8(translator.codecName() != "UTF-8") {
   }

   bool startElement(const QString &namespaceURI, const QString &localName,
      const QString &qName, const QXmlAttributes &atts);

   bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
   bool characters(const QString &ch);
   bool fatalError(const QXmlParseException &exception);

   void setDocumentLocator(QXmlLocator *locator) {
      m_locator = locator;
   }

 private:
   void flush();

   Translator &m_translator;
   ConversionData &m_cd;
   QString m_context;
   QString m_source;
   QString m_comment;
   QString m_extracomment;
   QXmlLocator *m_locator;

   QString m_accum;
   int m_lineNumber;
   bool m_isTrString;
   bool m_needUtf8;
};

bool UiReader::startElement(const QString &namespaceURI, const QString &localName,
            const QString &qName, const QXmlAttributes &atts)
{
   Q_UNUSED(namespaceURI);
   Q_UNUSED(localName);

   if (qName == "string") {
      flush();

      if (atts.value("notr").isEmpty() || atts.value("notr") != "true") {
         m_isTrString = true;

         m_comment      = atts.value("comment");
         m_extracomment = atts.value("extracomment");

         if (! m_cd.m_noUiLines) {
            m_lineNumber = m_locator->lineNumber();
         }

      } else {
         m_isTrString = false;
      }
   }

   m_accum.clear();

   return true;
}

bool UiReader::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
   (void) namespaceURI;
   (void) localName;

   m_accum.replace("\r\n", "\n");

   if (qName == "class") {
      // UI "header"

      if (m_context.isEmpty()) {
         m_context = m_accum;
      }

   } else if (qName == "string" && m_isTrString) {
      m_source = m_accum;

   } else if (qName == "comment") {
      m_comment = m_accum;
      flush();

   } else {
      flush();
   }

   return true;
}

bool UiReader::characters(const QString &ch)
{
   m_accum += ch;

   return true;
}

bool UiReader::fatalError(const QXmlParseException &exception)
{
   QString msg = LU::tr("XML error: Parse error at line %1, column %2 (%3).")
      .formatArg(exception.lineNumber()).formatArg(exception.columnNumber()).formatArg(exception.message());

   m_cd.appendError(msg);

   return false;
}

void UiReader::flush()
{
   if (! m_context.isEmpty() && !m_source.isEmpty()) {

      TranslatorMessage msg(m_context, m_source, m_comment, QString(),
                  m_cd.m_sourceFileName, m_lineNumber, QStringList());

      msg.setExtraComment(m_extracomment);

      if (m_needUtf8 && msg.needs8Bit()) {
         msg.setUtf8(true);
      }
      m_translator.extend(msg);
   }

   m_source.clear();
   m_comment.clear();
   m_extracomment.clear();
}

bool loadUI(Translator &translator, const QString &filename, ConversionData &cd)
{
   cd.m_sourceFileName = filename;
   QFile file(filename);

   if (! file.open(QIODevice::ReadOnly)) {
      cd.appendError(LU::tr("Unable to open %1: %2").formatArgs(filename, file.errorString()));
      return false;
   }

   QXmlInputSource in(&file);
   QXmlSimpleReader reader;
   reader.setFeature("http://xml.org/sax/features/namespaces", false);
   reader.setFeature("http://xml.org/sax/features/namespace-prefixes", true);
   reader.setFeature("http://copperspice.com/xml/features/report-whitespace-only-CharData", false);

   UiReader handler(translator, cd);
   reader.setContentHandler(&handler);
   reader.setErrorHandler(&handler);

   bool result = reader.parse(in);

   if (! result) {
      cd.appendError(LU::tr("Parse error in UI file"));
   }

   reader.setContentHandler(0);
   reader.setErrorHandler(0);

   return result;
}
