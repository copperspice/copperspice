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

#include <lupdate.h>
#include <translator.h>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qfile.h>
#include <qstring.h>
#include <qxmlattributes.h>
#include <qxmldefaulthandler.h>
#include <qxmllocator.h>
#include <qxmlparseexception.h>

class UiReader : public QXmlDefaultHandler
{
 public:
   UiReader(Translator &translator, ConversionData &cd)
      : m_translator(translator), m_cd(cd), m_lineNumber(-1), m_isTrString(false), m_insideStringList(false) {
   }

   bool startElement(const QString &namespaceURI, const QString &localName,
         const QString &qName, const QXmlAttributes &atts) override;

   bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
   bool characters(const QString &ch) override;
   bool fatalError(const QXmlParseException &exception) override;

   void setDocumentLocator(QXmlLocator *locator) override {
      m_locator = locator;
   }

 private:
   void flush();
   void readTranslationAttributes(const QXmlAttributes &atts);

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
   bool m_insideStringList;
};

bool UiReader::startElement(const QString &namespaceURI, const QString &localName,
                            const QString &qName, const QXmlAttributes &atts)
{
   (void) namespaceURI;
   (void) localName;

   if (qName == "string") {
      flush();

      if (! m_insideStringList) {
         readTranslationAttributes(atts);
      }

   } else if (qName == "stringlist") {
      flush();
      m_insideStringList = true;
      readTranslationAttributes(atts);

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
   } else if (qName == "stringlist") {
      m_insideStringList = false;

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
   QString msg = QString("XML error: Parse error at line %1, column %2 (%3).")
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
      m_translator.extend(msg, m_cd);
   }

   m_source.clear();

   if (! m_insideStringList) {
      m_comment.clear();
      m_extracomment.clear();
   }
}

void UiReader::readTranslationAttributes(const QXmlAttributes &atts)
{
   const QString notr = atts.value("notr");

   if (notr.isEmpty() || notr != "true") {
      m_isTrString = true;
      m_comment = atts.value("comment");
      m_extracomment = atts.value("extracomment");

      if (! m_cd.m_noUiLines) {
         m_lineNumber = m_locator->lineNumber();
      }

   } else {
      m_isTrString = false;
   }
}
bool loadUI(Translator &translator, const QString &filename, ConversionData &cd)
{
   cd.m_sourceFileName = filename;
   QFile file(filename);

   if (! file.open(QIODevice::ReadOnly)) {
      cd.appendError(QString("Unable to open %1: %2").formatArgs(filename, file.errorString()));
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
      cd.appendError("Parse error in UI file");
   }

   reader.setContentHandler(nullptr);
   reader.setErrorHandler(nullptr);

   return result;
}
