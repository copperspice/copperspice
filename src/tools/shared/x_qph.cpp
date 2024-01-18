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

#include "translator.h"

#include <QByteArray>
#include <QDebug>
#include <QTextCodec>
#include <QTextStream>

#include <QXmlStreamReader>
#include <QXmlStreamAttribute>

QT_BEGIN_NAMESPACE

class QPHReader : public QXmlStreamReader
{
 public:
   QPHReader(QIODevice &dev)
      : QXmlStreamReader(&dev) {
   }

   // the "real thing"
   bool read(Translator &translator);

 private:
   bool isWhiteSpace() const {
      return isCharacters() && text().toString().trimmed().isEmpty();
   }

   enum DataField { NoField, SourceField, TargetField, DefinitionField };
   DataField m_currentField;
   QString m_currentSource;
   QString m_currentTarget;
   QString m_currentDefinition;
};

bool QPHReader::read(Translator &translator)
{
   m_currentField = NoField;
   QString result;
   while (!atEnd()) {
      readNext();
      if (isStartElement()) {
         if (name() == QLatin1String("source")) {
            m_currentField = SourceField;
         } else if (name() == QLatin1String("target")) {
            m_currentField = TargetField;
         } else if (name() == QLatin1String("definition")) {
            m_currentField = DefinitionField;
         } else {
            m_currentField = NoField;
            if (name() == QLatin1String("QPH")) {
               QXmlStreamAttributes atts = attributes();
               translator.setLanguageCode(atts.value(QLatin1String("language")).toString());
               translator.setSourceLanguageCode(atts.value(QLatin1String("sourcelanguage")).toString());
            }
         }
      } else if (isWhiteSpace()) {
         // ignore these
      } else if (isCharacters()) {
         if (m_currentField == SourceField) {
            m_currentSource += text();
         } else if (m_currentField == TargetField) {
            m_currentTarget += text();
         } else if (m_currentField == DefinitionField) {
            m_currentDefinition += text();
         }
      } else if (isEndElement() && name() == QLatin1String("phrase")) {
         m_currentTarget.replace(QChar(Translator::TextVariantSeparator),
                                 QChar(Translator::BinaryVariantSeparator));
         TranslatorMessage msg;
         msg.setSourceText(m_currentSource);
         msg.setTranslation(m_currentTarget);
         msg.setComment(m_currentDefinition);
         translator.append(msg);
         m_currentSource.clear();
         m_currentTarget.clear();
         m_currentDefinition.clear();
      }
   }
   return true;
}

static bool loadQPH(Translator &translator, QIODevice &dev, ConversionData &)
{
   translator.setLocationsType(Translator::NoLocations);
   QPHReader reader(dev);
   return reader.read(translator);
}

static QString protect(const QString &str)
{
   QString result;

   for (QChar c : str) {

      switch (c.unicode()) {
         case '\"':
            result += QLatin1String("&quot;");
            break;

         case '&':
            result += QLatin1String("&amp;");
            break;

         case '>':
            result += QLatin1String("&gt;");
            break;

         case '<':
            result += QLatin1String("&lt;");
            break;

         case '\'':
            result += QLatin1String("&apos;");
            break;

         default:
            if (c < 0x20 && c != '\r' && c != '\n' && c != '\t') {
               result += QString("&#%1;").formatArg(c);

            } else {
               // this also covers surrogates
               result += c;
            }
      }
   }

   return result;
}

static bool saveQPH(const Translator &translator, QIODevice &dev, ConversionData &)
{
   QTextStream t(&dev);
   t.setCodec(QTextCodec::codecForName("UTF-8"));
   t << "<!DOCTYPE QPH>\n<QPH";

   QString languageCode = translator.languageCode();

   if (!languageCode.isEmpty() && languageCode != QLatin1String("C")) {
      t << " language=\"" << languageCode << "\"";
   }

   languageCode = translator.sourceLanguageCode();
   if (!languageCode.isEmpty() && languageCode != QLatin1String("C")) {
      t << " sourcelanguage=\"" << languageCode << "\"";
   }
   t << ">\n";

   for (const TranslatorMessage &msg : translator.messages()) {
      t << "<phrase>\n";
      t << "    <source>" << protect(msg.sourceText()) << "</source>\n";

      QString str = msg.translations().join(QLatin1String("@"));
      str.replace(QChar(Translator::BinaryVariantSeparator),
                  QChar(Translator::TextVariantSeparator));

      t << "    <target>" << protect(str)
        << "</target>\n";

      if (!msg.comment().isEmpty()) {
         t << "    <definition>" << protect(msg.comment()) << "</definition>\n";
      }
      t << "</phrase>\n";
   }
   t << "</QPH>\n";
   return true;
}

int initQPH()
{
   Translator::FileFormat format;

   format.extension = QLatin1String("qph");
   format.description = QObject::tr("Qt Linguist 'Phrase Book'");
   format.fileType = Translator::FileFormat::TranslationSource;
   format.priority = 0;
   format.loader = &loadQPH;
   format.saver = &saveQPH;
   Translator::registerFileFormat(format);

   return 1;
}

Q_CONSTRUCTOR_FUNCTION(initQPH)

QT_END_NAMESPACE
