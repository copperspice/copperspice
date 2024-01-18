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

#include "qscriptxmlparser_p.h"

#include <QtCore/qstringlist.h>
#include <QtCore/qxmlstream.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

static void tokenUntil(QXmlStreamReader &reader, QXmlStreamReader::TokenType target,
                       QList<int> &lineNumbers)
{
   int level = 0;
   while (!reader.atEnd()) {
      QXmlStreamReader::TokenType t = reader.readNext();
      if ((t == target) && (level == 0)) {
         return;
      }
      if (t == QXmlStreamReader::StartElement) {
         ++level;
         QString line = reader.attributes().value(QLatin1String("line")).toString();
         if (!line.isEmpty()) {
            lineNumbers.append(line.toInt());
         }
      } else if (t == QXmlStreamReader::EndElement) {
         --level;
      }
   }
   //    Q_ASSERT_X(false, "QScriptXmlParser", "premature end of file");
}

QScriptXmlParser::Result QScriptXmlParser::parse(const QString &xml)
{
   QMap<QString, int> functionsInfo;
   QList<int> lineNumbers;
   QXmlStreamReader reader(xml);
   reader.readNext(); // StartDocument
   reader.readNext(); // <program>
   reader.readNext(); // <source-elements>
   while (reader.readNext() == QXmlStreamReader::StartElement) {
      //        qDebug() << reader.name().toString();
      int line = reader.attributes().value(QLatin1String("line")).toString().toInt();
      lineNumbers.append(line);
      if (reader.name() == QLatin1String("function-declaration")) {
         // extract the line number, name and formal parameters
         reader.readNext(); // <name>
         reader.readNext(); // Characters
         QString name = reader.text().toString();
         reader.readNext(); // </name>
         reader.readNext(); // <formal-parameter-list>
         QStringList formalParameters;
         while (reader.readNext() == QXmlStreamReader::StartElement) {
            reader.readNext(); // Characters
            formalParameters.append(reader.text().toString());
            reader.readNext(); // </identifier>
         }
         reader.readNext(); // <function-body>
         tokenUntil(reader, QXmlStreamReader::EndElement, lineNumbers);

         QString signature;
         signature.append(name);
         signature.append(QLatin1Char('('));
         for (int i = 0; i < formalParameters.size(); ++i) {
            if (i > 0) {
               signature.append(QLatin1String(", "));
            }
            signature.append(formalParameters.at(i));
         }
         signature.append(QLatin1Char(')'));
         functionsInfo.insert(signature, line);
      } else if (reader.name() == QLatin1String("expression-statement")) {
         reader.readNext();
         if ((reader.name() == QLatin1String("binary-expression"))
               && reader.attributes().value(QLatin1String("op")) == QLatin1String("=")) {
            // try to match a statement of the form Foo.prototype.bar = function() { ... }
            // this can be generalized...
            QString first, second, third;
            reader.readNext(); // LHS
            if (reader.name() == QLatin1String("field-member-expression")) {
               reader.readNext();
               if (reader.name() == QLatin1String("field-member-expression")) {
                  reader.readNext();
                  if (reader.name() == QLatin1String("identifier")) {
                     reader.readNext();
                     first = reader.text().toString();
                  }
                  tokenUntil(reader, QXmlStreamReader::EndElement, lineNumbers);
                  reader.readNext();
                  if (reader.name() == QLatin1String("identifier")) {
                     reader.readNext();
                     second = reader.text().toString();
                  }
                  tokenUntil(reader, QXmlStreamReader::EndElement, lineNumbers);
               } else if (reader.name() == QLatin1String("identifier")) {
                  reader.readNext();
                  first = reader.text().toString();
               }
               tokenUntil(reader, QXmlStreamReader::EndElement, lineNumbers);
               reader.readNext();
               if (reader.name() == QLatin1String("identifier")) {
                  reader.readNext();
                  if (second.isEmpty()) {
                     second = reader.text().toString();
                  } else {
                     third = reader.text().toString();
                  }
               }
               tokenUntil(reader, QXmlStreamReader::EndElement, lineNumbers);
            }
            tokenUntil(reader, QXmlStreamReader::EndElement, lineNumbers);
            reader.readNext(); // RHS
            if (reader.name() == QLatin1String("function-expression")) {
               if (!first.isEmpty()) {
                  QString signature = first;
                  if (!second.isEmpty()) {
                     signature.append(QLatin1Char('.'));
                     signature.append(second);
                     if (!third.isEmpty()) {
                        signature.append(QLatin1Char('.'));
                        signature.append(third);
                     }
                  }
                  signature.append(QLatin1String("()"));
                  functionsInfo.insert(signature, line);
               }
            }
            tokenUntil(reader, QXmlStreamReader::EndElement, lineNumbers);
         }
         tokenUntil(reader, QXmlStreamReader::EndElement, lineNumbers);
      }
      tokenUntil(reader, QXmlStreamReader::EndElement, lineNumbers);
   }
   reader.readNext(); // </source-elements>
   reader.readNext(); // </program>
   reader.readNext(); // EndDocument
   Q_ASSERT(reader.atEnd());
   return Result(functionsInfo, lineNumbers.toSet());
}

QT_END_NAMESPACE
